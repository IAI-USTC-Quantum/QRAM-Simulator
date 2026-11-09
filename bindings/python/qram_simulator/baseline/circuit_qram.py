"""Gate-level (circuit-level) qubit-architecture QRAM simulator.

An independent re-implementation of the qubit-architecture QRAM loading circuit
(the symbolic sparse-tree engine of ``qram_qubit::QRAMCircuit``) as an explicit
quantum circuit, simulated gate by gate on the UnifiedQuantum (``uniqc``) QuTiP
density-matrix backend.  Serves as the semi-quantitative baseline against
:class:`qram_simulator.QRAMCircuitQubit` (see ``qram_simulator.baseline.compare``).

Why only 1-2 tree layers
------------------------
The density matrix lives on ``addr + data + 2*(2^addr - 1)`` qubits
(9 qubits for addr=2/data=1); addr=3 already needs 25 qubits (2^50 amplitudes).
With ``addr_size = 1`` every step has ``layer_entangle_max == 0``, so Pauli
noise on the active front vanishes. Whole-tree amplitude damping still acts;
the one-layer case is not a noise-free bridge when damping is nonzero.

Encoding (identical to ``Experiments/QRAM/ChannelCorrespondence``)
------------------------------------------------------------------
- address register: ``addr_size`` qubits, address bit ``b`` = qubit ``b``;
- data bus: ``data_size`` qubits starting at ``addr_size``;
- routing-tree node ``v`` (heap numbering, ``v in [0, 2^addr - 1)``): two
  qubits -- ``a = addr + data + 2v`` (routing/addr slot) and
  ``d = addr + data + 2v + 1`` (data slot); the tree is at ``|0...0>`` ground.
- noise position convention: ``pos = 2v + lr`` with ``lr = 0 -> a, 1 -> d``.

Gate translation (bitwise-verified against the C++ engine by the
ChannelCorrespondence experiment, Stage 0/1)
------------------------------------------------
- ``FirstCopy{l}``   -> CNOT(addr[addr-1-l], node0.d)   (MSB-first)
- ``CopyIn{d}``      -> H on all bus qubits when d == 0, SWAP(bus[d], node0.d)
- ``CopyOut{d}``     -> SWAP(bus[d], node0.d), H on all bus qubits when d == last
- ``SwapInternal{l}``-> SWAP(a, d) on every node of layer l
- ``ControlSwap{l}``  -> per node v of layer l: SWAP(d_v, d_2v+1) controlled by
  a_v == 0 and SWAP(d_v, d_2v+2) controlled by a_v == 1
- ``FetchData{k}``   -> U1(pi) on each leaf d_v controlled by (a_v == lr) for
  every memory cell whose bit k is set (phase-kickback fetch)

The gate sequence and the per-step entangled-layer counts are pulled from the
C++ ``TimeStep`` scheduler through ``qram_simulator._core`` (single source of
truth) -- this module never re-derives the schedule.

Noise model (channel level, mirrors the C++ trajectory sampler)
---------------------------------------------------------------
Noise channels are placed **only on the routing-tree qubits** (never on the
address register or the bus).  After the gates of each time slice with
entangled-layer count ``l``, a channel acts on every active position
``[0, 2*(2^l - 1))`` with marginal probability ``p`` -- the channel-level
counterpart of the C++ per-trajectory sampler (``nerror ~ Binomial(N, p)``
placed uniformly on the N active positions, hence each position is hit with
probability exactly ``p``).  Depolarizing applies the ``{I, X, Z, Y}``
unitary mixture (all unitary after the Y-flip fix, so the textbook TP channel
and the faithful mixture coincide). Damping uses fixed standard Kraus matrices
on every tree qubit, matching the current exact joint trajectory sampler.
Its trajectory norm and the density trace are one. ``mode="legacy"`` retains
the retired state-dependent, non-TP mirror for archived comparisons only;
``mode="nojump"`` retains only K0 and reports its sub-normalized weight.
"""

from __future__ import annotations

import math
from typing import Dict, List, Sequence, Tuple

import numpy as np

from .._core import ARCH_QUBIT, OperationType, TimeStep

__all__ = ["CircuitQRAMQubit", "QubitQRAMEncoding"]

# bit-phase-flip operator of the C++ qubit architecture (run_bitphaseflip):
# |0> -> -|1>, |1> -> |0>  (ZX = iY, unitary)
_QUBIT_M = np.array([[0.0, 1.0], [-1.0, 0.0]], dtype=complex)

# a gate: (op, targets, ctrls[, theta]); ctrls = ((qubit, polarity), ...)
Gate = Tuple[str, Tuple[int, ...], Tuple[Tuple[int, int], ...], float]


def _require_uniqc():
    """Import the uniqc QuTiP density-matrix backend lazily with a helpful error."""
    try:
        from uniqc.simulator.qutip_sim_impl import DensityOperatorSimulatorQutip
    except ImportError as exc:  # pragma: no cover - environment guard
        raise ImportError(
            "The circuit-level baseline requires the UnifiedQuantum (uniqc) "
            "framework and QuTiP. Install them with: "
            "pip install 'qram-simulator[baseline]'"
        ) from exc
    return DensityOperatorSimulatorQutip


# The uniqc backend is created once per simulate() call; the basis-index bit
# order exposed by the actual SDK is probed once and cached; do not assume
# that a raw SDK density matrix uses QuTiP's usual qubit0-first tensor order.
_BIT_ORDER_CACHE: Dict[str, str] = {}


def _backend_bit_order(backend_cls) -> str:
    """Probe whether qubit 0 is the LSB or the MSB of the density-matrix index."""
    key = backend_cls.__module__ + "." + backend_cls.__qualname__
    if key not in _BIT_ORDER_CACHE:
        be = backend_cls()
        be.init_n_qubit(2)
        be.x(0)
        diag = np.asarray(be.density_matrix.diag()).ravel().real
        _BIT_ORDER_CACHE[key] = "msb" if int(np.argmax(diag)) == 0b10 else "lsb"
    return _BIT_ORDER_CACHE[key]


class QubitQRAMEncoding:
    """Qubit-index layout of the encoded QRAM circuit."""

    def __init__(self, addr_size: int, data_size: int):
        self.addr_size = addr_size
        self.data_size = data_size

    @property
    def num_qubits(self) -> int:
        return self.addr_size + self.data_size + 2 * (2**self.addr_size - 1)

    @property
    def num_nodes(self) -> int:
        return 2**self.addr_size - 1

    def addr_qubit(self, bit: int) -> int:
        return bit

    def bus_qubit(self, digit: int) -> int:
        return self.addr_size + digit

    def node_a(self, v: int) -> int:
        return self.addr_size + self.data_size + 2 * v

    def node_d(self, v: int) -> int:
        return self.addr_size + self.data_size + 2 * v + 1

    def noise_qubit(self, pos: int) -> int:
        """Tree qubit of flat noise position ``pos = 2v + lr`` (lr: 0 -> a, 1 -> d)."""
        v, lr = divmod(pos, 2)
        return self.node_a(v) if lr == 0 else self.node_d(v)

    @property
    def address_qubits(self) -> List[int]:
        return list(range(self.addr_size))

    @property
    def bus_qubits(self) -> List[int]:
        return [self.bus_qubit(d) for d in range(self.data_size)]

    @property
    def tree_qubits(self) -> List[int]:
        return [q for v in range(self.num_nodes) for q in (self.node_a(v), self.node_d(v))]

    def layer_nodes(self, layer: int) -> List[int]:
        return list(range(2**layer - 1, 2 ** (layer + 1) - 1))


class CircuitQRAMQubit:
    """Circuit-level baseline of the qubit-architecture QRAM loading circuit.

    Parameters
    ----------
    addr_size : int
        Address width = tree depth. Only 1 or 2 is supported (density-matrix
        size limit); addr=1 only exercises the noise-free bridge (plus the
        whole-tree part of Damping).
    data_size : int
        Bits per memory cell (1 or 2).
    memory : sequence of int
        Data tree, length 2**addr_size, every entry < 2**data_size (the C++
        fidelity XORs raw values; out-of-range entries are silently scored 0).
    depolarizing, damping : float
        Noise probabilities of the two supported channels, mirroring
        ``QRAMCircuitQubit.set_noise_models``. Applied on tree qubits only.
    input_mode : {"zerobus", "uniform"}
        "zerobus" (default): uniform superposition of addresses, bus = 0 --
        the discriminating input; "uniform": uniform address and bus.
    """

    def __init__(
        self,
        addr_size: int,
        data_size: int,
        memory: Sequence[int],
        *,
        depolarizing: float = 0.0,
        damping: float = 0.0,
        bit_flip: float = 0.0,
        phase_flip: float = 0.0,
        input_mode: str = "zerobus",
    ):
        if addr_size not in (1, 2):
            raise ValueError(
                "circuit-level baseline supports addr_size 1 or 2 "
                f"(got {addr_size}: {addr_size + data_size + 2 * (2**addr_size - 1)} "
                "qubits exceed the density-matrix budget at addr_size >= 3)"
            )
        if data_size < 1 or data_size > 2:
            raise ValueError(f"data_size must be 1 or 2 (got {data_size})")
        if input_mode not in ("zerobus", "uniform"):
            raise ValueError(f"input_mode must be 'zerobus' or 'uniform' (got {input_mode})")
        for p, name in (
            (depolarizing, "depolarizing"),
            (damping, "damping"),
            (bit_flip, "bit_flip"),
            (phase_flip, "phase_flip"),
        ):
            if not 0.0 <= p <= 1.0 or (name == "damping" and p >= 1.0):
                raise ValueError(
                    f"{name} probability must lie in [0, 1] (Damping requires gamma < 1)"
                )
        memory = list(memory)
        if len(memory) != 2**addr_size:
            raise ValueError(f"memory must have 2**addr_size = {2**addr_size} cells")
        for value in memory:
            if not 0 <= value < 2**data_size:
                raise ValueError(
                    f"memory value {value} does not fit into data_size = {data_size} bits"
                )

        self.addr_size = addr_size
        self.data_size = data_size
        self.memory = memory
        self.depolarizing = float(depolarizing)
        self.damping = float(damping)
        self.bit_flip = float(bit_flip)
        self.phase_flip = float(phase_flip)
        self.input_mode = input_mode
        self.encoding = QubitQRAMEncoding(addr_size, data_size)

        # Gate schedule + per-step entangled-layer counts come from the C++
        # TimeStep scheduler through the binding -- the single source of truth.
        ts = TimeStep(addr_size, data_size)
        slices = ts.generate({}, ARCH_QUBIT)
        self._steps: List[Tuple[List[Gate], int]] = []
        for index, pack in enumerate(slices.time_slices):
            step = index + 1
            gates: List[Gate] = []
            for op in pack.operations:
                gates.extend(self._translate_op(op, step))
            self._steps.append((gates, ts.layer_entangle_max(step)))

    # ------------------------------------------------------------------
    # schedule translation
    # ------------------------------------------------------------------

    def _translate_op(self, op, step: int) -> List[Gate]:
        enc = self.encoding
        targets = list(op.targets)
        t = op.type
        if t == OperationType.FirstCopy:
            # MSB-first address copy: layer l reads address bit addr_size-1-l
            bit = enc.addr_qubit(self.addr_size - 1 - targets[0])
            return [("CNOT", (bit, enc.node_d(0)), (), math.nan)]
        if t == OperationType.CopyIn:
            gates: List[Gate] = []
            if targets[0] == 0:
                gates.extend(
                    ("H", (enc.bus_qubit(d),), (), math.nan) for d in range(self.data_size)
                )
            gates.append(("SWAP", (enc.bus_qubit(targets[0]), enc.node_d(0)), (), math.nan))
            return gates
        if t == OperationType.CopyOut:
            gates = [("SWAP", (enc.bus_qubit(targets[0]), enc.node_d(0)), (), math.nan)]
            if targets[0] == self.data_size - 1:
                gates.extend(
                    ("H", (enc.bus_qubit(d),), (), math.nan) for d in range(self.data_size)
                )
            return gates
        if t == OperationType.SwapInternal:
            # SWAP(a, d) on every node of the layer (identity on ground nodes)
            return [
                ("SWAP", (enc.node_a(v), enc.node_d(v)), (), math.nan)
                for v in enc.layer_nodes(targets[0])
            ]
        if t == OperationType.ControlSwap:
            gates = []
            for v in enc.layer_nodes(targets[0]):
                gates.append(
                    (
                        "SWAP",
                        (enc.node_d(v), enc.node_d(2 * v + 1)),
                        ((enc.node_a(v), 0),),
                        math.nan,
                    )
                )
                gates.append(
                    (
                        "SWAP",
                        (enc.node_d(v), enc.node_d(2 * v + 2)),
                        ((enc.node_a(v), 1),),
                        math.nan,
                    )
                )
            return gates
        if t == OperationType.FetchData:
            # phase kickback: -1 phase on the leaf cell selected by (a, memory)
            digit = targets[0]
            lower = 2 ** (self.addr_size - 1) - 1
            gates = []
            for v in range(lower, 2**self.addr_size - 1):
                off = v - lower
                if (self.memory[2 * off] >> digit) & 1:
                    gates.append(("U1", (enc.node_d(v),), ((enc.node_a(v), 0),), math.pi))
                if (self.memory[2 * off + 1] >> digit) & 1:
                    gates.append(("U1", (enc.node_d(v),), ((enc.node_a(v), 1),), math.pi))
            return gates
        raise ValueError(f"unexpected logical operation {op.to_string()} at step {step}")

    # ------------------------------------------------------------------
    # backend driving
    # ------------------------------------------------------------------

    def _apply_gate(self, be, gate: Gate) -> None:
        op, targets, ctrls, theta = gate
        negatives = [q for q, value in ctrls if value == 0]
        positives = [q for q, value in ctrls if value == 1]
        controls = negatives + positives
        for q in negatives:  # negative controls via X conjugation
            be.x(q)
        if op == "X":
            be.x(targets[0], controls)
        elif op == "H":
            be.hadamard(targets[0], controls)
        elif op == "SWAP":
            be.swap(targets[0], targets[1], controls)
        elif op == "CNOT":
            be.cnot(targets[0], targets[1])
        elif op == "U1":
            be.u1(targets[0], theta, controls)
        else:  # pragma: no cover - translation invariant
            raise ValueError(op)
        for q in negatives:
            be.x(q)

    def _init_backend(self, backend_cls):
        be = backend_cls()
        be.init_n_qubit(self.encoding.num_qubits)
        for q in self.encoding.address_qubits:
            be.hadamard(q)
        if self.input_mode == "uniform":
            for q in self.encoding.bus_qubits:
                be.hadamard(q)
        return be

    def _apply_depolarizing(self, be, q: int, p: float, textbook: bool) -> None:
        if textbook:
            be.depolarizing(q, p)
            return
        # faithful mixture {I, X, Z, M} of the trajectory floor rule
        c0 = math.sqrt(1.0 - p)
        c1 = math.sqrt(p / 3.0)
        ks = [
            c0 * np.eye(2, dtype=complex),
            c1 * np.array([[0, 1], [1, 0]], dtype=complex),
            c1 * np.array([[1, 0], [0, -1]], dtype=complex),
            c1 * _QUBIT_M,
        ]
        be.kraus1q(q, [k.reshape(-1).tolist() for k in ks])

    def _apply_damping(self, be, entangle: int, gamma: float, textbook: bool, nojump: bool) -> None:
        enc = self.encoding
        s = math.sqrt(1.0 - gamma)
        diag = None
        total = 1.0
        n_active_nodes = 2**entangle - 1
        if not textbook:
            diag = np.asarray(be.density_matrix.diag()).ravel().real
            total = float(diag.sum())
        n = enc.num_qubits
        idx = np.arange(2**n)
        for v in range(enc.num_nodes):
            for q in (enc.node_a(v), enc.node_d(v)):
                if textbook:
                    be.amplitude_damping(q, gamma)
                    continue
                k0 = np.diag([1.0, s]).astype(complex)
                jump = None
                if not nojump and total > 0.0 and v < n_active_nodes:
                    # faithful mirror: jump weight = gamma * w / S on active
                    # positions, K0 branch scaled accordingly (weights taken
                    # on the current averaged rho)
                    w = float(diag[((idx >> q) & 1).astype(bool)].sum())
                    a = math.sqrt(max(1.0 - gamma * w / total, 0.0))
                    k0 = a * np.diag([1.0, s]).astype(complex)
                    jump = np.zeros((2, 2), dtype=complex)
                    jump[0, 1] = math.sqrt(gamma * w / total)
                ks = [k0] if jump is None else [k0, jump]
                be.kraus1q(q, [k.reshape(-1).tolist() for k in ks])

    # ------------------------------------------------------------------
    # public API
    # ------------------------------------------------------------------

    def to_gate_list(self) -> List[Gate]:
        """Flat noise-free gate list (input preparation gates excluded)."""
        return [gate for gates, _ in self._steps for gate in gates]

    def ideal_density_matrix(self) -> np.ndarray:
        """Density matrix of the noise-free circuit (pure state, trace 1)."""
        backend_cls = _require_uniqc()
        be = self._init_backend(backend_cls)
        for gates, _ in self._steps:
            for gate in gates:
                self._apply_gate(be, gate)
        return np.asarray(be.density_matrix.full())

    def simulate_density(self, mode: str = "faithful") -> np.ndarray:
        """Run the noisy circuit on the uniqc QuTiP density-matrix backend.

        Parameters
        ----------
        mode : {"faithful", "textbook", "legacy", "nojump"}
            "faithful" (default): depolarizing as the {I,X,Z,Y} mixture and
            fixed whole-tree amplitude damping, matching the current core;
            "textbook": equivalent standard TP channels; "legacy": retired
            rho-dependent damping mirror, not a standard channel certificate;
            "nojump": damping K0 branch only (trace = no-jump weight).
        """
        if mode not in ("faithful", "textbook", "legacy", "nojump"):
            raise ValueError(f"mode must be faithful/textbook/legacy/nojump (got {mode})")
        backend_cls = _require_uniqc()
        be = self._init_backend(backend_cls)
        textbook = mode == "textbook"
        for gates, entangle in self._steps:
            for gate in gates:
                self._apply_gate(be, gate)
            n_active = 2 * (2**entangle - 1)
            for probability, pauli in (
                (self.bit_flip, np.array([[0, 1], [1, 0]], dtype=complex)),
                (self.phase_flip, np.diag([1, -1]).astype(complex)),
            ):
                if probability > 0:
                    operators = [
                        math.sqrt(1 - probability) * np.eye(2, dtype=complex),
                        math.sqrt(probability) * pauli,
                    ]
                    for pos in range(n_active):
                        be.kraus1q(
                            self.encoding.noise_qubit(pos),
                            [matrix.reshape(-1).tolist() for matrix in operators],
                        )
            if self.depolarizing > 0.0:
                for pos in range(n_active):
                    self._apply_depolarizing(
                        be, self.encoding.noise_qubit(pos), self.depolarizing, textbook
                    )
            if self.damping > 0.0:
                self._apply_damping(
                    be,
                    entangle,
                    self.damping,
                    mode in ("faithful", "textbook"),
                    nojump=(mode == "nojump"),
                )
        return np.asarray(be.density_matrix.full())

    def marginal_distribution(self, rho: np.ndarray) -> Dict[str, float]:
        """Marginal (address:bus) distribution of a density matrix.

        Same key convention as ``QRAMCircuitQubit.get_output_distribution``
        ("addr:bus" with the plain integers), so the two sides compare
        key-for-key.
        """
        enc = self.encoding
        n = enc.num_qubits
        order = _backend_bit_order(_require_uniqc())

        def bit(index: int, q: int) -> int:
            return (index >> q) & 1 if order == "lsb" else (index >> (n - 1 - q)) & 1

        diagonal = np.real(np.diag(rho))
        dist: Dict[str, float] = {}
        for index, p in enumerate(diagonal):
            a = sum(bit(index, q) << k for k, q in enumerate(enc.address_qubits))
            b = sum(bit(index, q) << k for k, q in enumerate(enc.bus_qubits))
            key = f"{a}:{b}"
            dist[key] = dist.get(key, 0.0) + float(p)
        return dist

    def fidelity_with_ideal(self, rho: np.ndarray, rho_ideal: np.ndarray = None) -> float:
        """<psi_ideal|rho|psi_ideal>, the same convention as the C++ overlap_fid.

        For the pure ideal state this equals Re Tr(rho @ rho_ideal); it is the
        statistic that matches the QRAM-Simulator trajectory average
        ``E|<psi_ideal|psi_traj>|^2`` (avg_overlap_fid).
        """
        if rho_ideal is None:
            rho_ideal = self.ideal_density_matrix()
        return float(np.real(np.trace(rho @ rho_ideal)))

    def logical_density_matrix(self, rho: np.ndarray) -> np.ndarray:
        """Trace out tree qubits; visible index is address + (bus << addr_size).

        This retains address/bus coherences and sums only equal environment
        indices. It is independent of the sparse engine's observable code.
        """
        enc = self.encoding
        n = enc.num_qubits
        rho = np.asarray(rho, dtype=complex)
        if rho.shape != (2**n, 2**n):
            raise ValueError("density matrix shape does not match the encoded circuit")
        order = _backend_bit_order(_require_uniqc())
        visible = [*enc.address_qubits, *enc.bus_qubits]
        environment = [q for q in range(n) if q not in visible]
        indices = np.empty((2 ** len(visible), 2 ** len(environment)), dtype=np.int64)
        for index in range(2**n):

            def bits(qubits):
                return sum(
                    ((index >> (q if order == "lsb" else n - 1 - q)) & 1) << k
                    for k, q in enumerate(qubits)
                )

            indices[bits(visible), bits(environment)] = index
        reduced = np.zeros((len(indices), len(indices)), dtype=complex)
        for column in indices.T:
            reduced += rho[np.ix_(column, column)]
        return reduced

    def logical_fidelity_with_ideal(self, rho: np.ndarray) -> float:
        """Overlap with the pure ideal address/bus query after a physical partial trace.

        No router reset or postselection is imposed. This differs from the
        full-circuit ``fidelity_with_ideal`` and tree-insensitive conventions.
        """
        ideal = self.logical_density_matrix(self.ideal_density_matrix())
        return float(np.real(np.trace(self.logical_density_matrix(rho) @ ideal)))

    def run(self, mode: str = "faithful") -> Dict[str, object]:
        """Convenience wrapper: ideal + noisy density matrices and all metrics.

        Returns a dict with ``ideal_rho``/``ideal_dist``, ``rho``/``dist``/``trace``
        and ``fidelity`` (= fidelity_with_ideal). The modern complete channels
        have trace one. Only explicit legacy/nojump diagnostics may return
        sub-normalized distributions; their trace is not the core's trajectory norm.
        """
        rho_ideal = self.ideal_density_matrix()
        rho = self.simulate_density(mode)
        return {
            "ideal_rho": rho_ideal,
            "ideal_dist": self.marginal_distribution(rho_ideal),
            "rho": rho,
            "dist": self.marginal_distribution(rho),
            "trace": float(np.real(np.trace(rho))),
            "fidelity": self.fidelity_with_ideal(rho, rho_ideal),
        }
