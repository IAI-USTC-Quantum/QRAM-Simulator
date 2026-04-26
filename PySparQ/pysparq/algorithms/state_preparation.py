"""State preparation via QRAM-based binary tree decomposition.

This module implements quantum state preparation using a QRAM circuit to
store a binary-tree encoding of the target amplitude distribution.  The
key operator :class:`StatePrepViaQRAM` traverses the tree level by level,
loading parent/child norms from QRAM and applying conditional rotations
to gradually build up the desired amplitudes.

A high-level wrapper :class:`StatePreparation` is also provided that
manages the full pipeline: random distribution generation, tree
construction, QRAM creation, and execution.

Ported from the C++ SparQ codebase:
  - ``SparQ_Algorithm/include/state_preparation.h``
  - ``SparQ_Algorithm/src/state_preparation.cpp``

Reference:
    - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

from __future__ import annotations

import math

import numpy as np

import pysparq as ps
from pysparq.operators import ControllableOperatorMixin

from pysparq.algorithms.qram_utils import (
    get_complement,
    make_func,
    make_func_inv,
    make_vector_tree,
    pow2,
)


class StatePrepViaQRAM(ControllableOperatorMixin):
    """QRAM-based quantum operator for state preparation via binary tree decomposition.

    Given a QRAM circuit that stores the binary-tree representation of a
    target amplitude distribution, this operator prepares the corresponding
    quantum state on the *work_qubit* register.  The tree is traversed from
    the root to the leaves; at each level a conditional rotation is applied
    whose angle is derived from the ratio of child and parent norms stored
    in QRAM.

    Attributes:
        qram: The QRAMCircuit_qutrit holding the tree-encoded distribution.
        work_qubit: Name of the register on which the state is prepared.
        addr_size: Bit-width of *work_qubit* (number of tree levels).
        data_size: Bit-width of signed integer data stored in QRAM.
        rational_size: Bit-width of the rational rotation-angle register.
    """

    def __init__(
        self,
        qram: object,
        work_qubit: str,
        data_size: int,
        rational_size: int,
    ):
        super().__init__()
        self.qram = qram
        self.work_qubit = work_qubit
        self.addr_size = ps.System.size_of(work_qubit)
        self.data_size = data_size
        self.rational_size = rational_size

    def __call__(self, state: ps.SparseState) -> None:
        """Apply the forward state-preparation operator.

        Ported from ``State_Prep_via_QRAM::impl()``.
        """
        ps.AddRegister("addr_parent", ps.UnsignedInteger, self.addr_size + 1)(state)
        ps.AddRegister("addr_child", ps.UnsignedInteger, self.addr_size + 1)(state)
        ps.AddRegister("data_parent", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("data_child", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("div_result", ps.Rational, self.rational_size)(state)

        n_digit = ps.System.size_of("div_result")
        func = lambda value: make_func(value, n_digit)

        for k in range(self.addr_size):
            ps.SplitRegister(self.work_qubit, "rotation", 1)(state)
            ps.System.set_register_type("rotation", ps.Boolean)

            ps.Add_ConstUInt("addr_parent", pow2(k) - 1)(state)
            ps.Add_UInt_UInt_InPlace(self.work_qubit, "addr_parent")(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)

            if k != self.addr_size - 1:
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.Div_Sqrt_Arccos_Int_Int(
                    "data_child", "data_parent", "div_result"
                )(state)
                ps.CondRot_General_Bool("div_result", "rotation", func)(state)
                ps.ClearZero()(state)
                # Uncompute
                ps.Div_Sqrt_Arccos_Int_Int(
                    "data_child", "data_parent", "div_result"
                )(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            else:
                # Last iteration: different addressing for leaf level
                ps.ShiftLeft("addr_parent", 1)(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.Add_ConstUInt("addr_child", 1)(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)
                ps.CondRot_General_Bool("div_result", "rotation", func)(state)
                ps.ClearZero()(state)
                # Uncompute
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.Add_ConstUInt("addr_child", 1).dag(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.ShiftRight("addr_parent", 1)(state)

            ps.Xgate_Bool("addr_child", 0)(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Add_UInt_UInt_InPlace(self.work_qubit, "addr_parent").dag(state)
            ps.Add_ConstUInt("addr_parent", pow2(k) - 1).dag(state)
            ps.CombineRegister(self.work_qubit, "rotation")(state)
            ps.ShiftLeft(self.work_qubit, 1)(state)

        ps.ShiftRight(self.work_qubit, 1)(state)

        ps.RemoveRegister("addr_parent")(state)
        ps.RemoveRegister("data_parent")(state)
        ps.RemoveRegister("addr_child")(state)
        ps.RemoveRegister("data_child")(state)
        ps.RemoveRegister("div_result")(state)

        ps.ClearZero()(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply the inverse (adjoint) state-preparation operator.

        Ported from ``State_Prep_via_QRAM::impl_dag()``.  The loop runs
        in reverse order and uses ``make_func_inv`` for the rotation
        matrices.
        """
        ps.AddRegister("addr_parent", ps.UnsignedInteger, self.addr_size + 1)(state)
        ps.AddRegister("addr_child", ps.UnsignedInteger, self.addr_size + 1)(state)
        ps.AddRegister("data_parent", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("data_child", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("div_result", ps.Rational, self.rational_size)(state)

        n_digit = ps.System.size_of("div_result")
        func = lambda value: make_func_inv(value, n_digit)

        ps.ShiftLeft(self.work_qubit, 1)(state)

        for k in range(self.addr_size):
            ps.ShiftRight(self.work_qubit, 1)(state)
            ps.SplitRegister(self.work_qubit, "rotation", 1)(state)
            ps.System.set_register_type("rotation", ps.Boolean)

            idx = self.addr_size - 1 - k
            ps.Add_ConstUInt("addr_parent", pow2(idx) - 1)(state)
            ps.Add_UInt_UInt_InPlace(self.work_qubit, "addr_parent")(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)

            if k != 0:
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.Div_Sqrt_Arccos_Int_Int(
                    "data_child", "data_parent", "div_result"
                )(state)
                ps.CondRot_General_Bool("div_result", "rotation", func)(state)
                ps.ClearZero()(state)
                # Uncompute
                ps.Div_Sqrt_Arccos_Int_Int(
                    "data_child", "data_parent", "div_result"
                )(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            else:
                # k == 0 corresponds to last level (leaf level)
                ps.ShiftLeft("addr_parent", 1)(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.Add_ConstUInt("addr_child", 1)(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)
                ps.CondRot_General_Bool("div_result", "rotation", func)(state)
                ps.ClearZero()(state)
                # Uncompute
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.Add_ConstUInt("addr_child", 1).dag(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.ShiftRight("addr_parent", 1)(state)

            ps.Xgate_Bool("addr_child", 0)(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Add_UInt_UInt_InPlace(self.work_qubit, "addr_parent").dag(state)
            ps.Add_ConstUInt("addr_parent", pow2(idx) - 1).dag(state)
            ps.CombineRegister(self.work_qubit, "rotation")(state)

        ps.RemoveRegister("addr_parent")(state)
        ps.RemoveRegister("data_parent")(state)
        ps.RemoveRegister("addr_child")(state)
        ps.RemoveRegister("data_child")(state)
        ps.RemoveRegister("div_result")(state)

        ps.ClearZero()(state)


class StatePreparation:
    """High-level wrapper that manages the full state-preparation pipeline.

    Orchestrates random distribution generation, binary tree construction,
    QRAM circuit creation, and execution of the
    :class:`StatePrepViaQRAM` operator.

    Attributes:
        qubit_number: Number of qubits (``log2`` of distribution length).
        data_size: Bit-width for signed integer encoding of amplitudes.
        data_range: Bit-range controlling the random amplitude magnitude.
        rational_size: Bit-width for the rational rotation-angle register.
        dist: The raw distribution values (unsigned two's-complement).
        tree: The binary tree built from *dist* for QRAM storage.
        qram: The QRAM circuit instance.
    """

    def __init__(self, qubit_number: int, data_size: int, data_range: int):
        self.qubit_number = qubit_number
        self.data_size = data_size
        self.data_range = data_range
        self.rational_size = min(50, data_size * 2)

        self.dist: list[int] = []
        self.tree: list[int] = []
        self.qram: object | None = None

        self._state: ps.SparseState | None = None

    def random_distribution(self) -> None:
        """Generate a random amplitude distribution.

        Each entry is sampled uniformly from ``[0, 2**data_range)``.
        Values in the upper half of the range are mapped to negative
        amplitudes via two's complement at *data_size* bits.
        """
        sz = pow2(self.qubit_number)
        data_max = pow2(self.data_range)
        self.dist = [0] * sz

        for i in range(sz):
            data = int(np.random.randint(0, data_max))
            if data >= pow2(self.data_range - 1):
                data = pow2(self.data_size) - (pow2(self.data_range) - data)
            self.dist[i] = data

    def show_distribution(self) -> None:
        """Print the distribution with original values and normalized amplitudes."""
        total = sum(
            get_complement(a, self.data_size) ** 2 for a in self.dist
        )
        norm = math.sqrt(total)
        header = f"{'index':^5} | {'original':^8} | {'amplitude':^12}"
        rows = []
        for i, val in enumerate(self.dist):
            orig = get_complement(val, self.data_size)
            amp = orig / norm if norm != 0 else 0.0
            rows.append(f"{i:^5} | {orig:^8} | {amp:f}")
        print(header)
        print("\n".join(rows))

    def get_real_dist(self) -> list[float]:
        """Return the normalized amplitude distribution as floats.

        Returns:
            List of amplitudes ``get_complement(dist[i]) / A`` where
            ``A = sqrt(sum of squares)``.
        """
        total = sum(
            get_complement(a, self.data_size) ** 2 for a in self.dist
        )
        norm = math.sqrt(total)
        return [
            get_complement(a, self.data_size) / norm if norm != 0 else 0.0
            for a in self.dist
        ]

    def make_tree(self) -> None:
        """Build the binary tree from the current distribution."""
        self.tree = make_vector_tree(self.dist, self.data_size)

    def show_tree(self) -> None:
        """Print the binary tree level by level."""
        begin = 1
        for n in range(self.qubit_number + 1):
            level_items = []
            for i in range(pow2(n)):
                showdata = self.tree[begin + i]
                if n == self.qubit_number:
                    showdata = get_complement(self.tree[begin + i], self.data_size)
                level_items.append(f"({begin + i}) {showdata:^4}")
            print(" ".join(level_items))
            begin += pow2(n)

    def make_qram(self) -> None:
        """Create a QRAM circuit sized for the tree data.

        The QRAM has ``qubit_number + 1`` address bits and *data_size*
        data bits.
        """
        self.qram = ps.QRAMCircuit_qutrit(self.qubit_number + 1, self.data_size)

    def set_qram(self) -> None:
        """Load the binary tree data into the QRAM circuit.

        Since the Python bindings do not expose ``set_memory``, the QRAM
        is recreated with the tree as constructor argument.
        """
        self.qram = ps.QRAMCircuit_qutrit(
            self.qubit_number + 1, self.data_size, self.tree
        )

    def get_fidelity(self) -> float:
        """Compute the fidelity of the prepared state against the target.

        Returns:
            The fidelity ``|<psi_target | psi_prepared>|^2``.
        """
        if self._state is None:
            return 0.0

        total = sum(
            get_complement(a, self.data_size) ** 2 for a in self.dist
        )
        norm = math.sqrt(total)

        addr_reg = ps.System.get_id("addr_parent")
        fid = complex(0, 0)

        for basis in self._state.basis_states:
            idx = basis.get(addr_reg).value
            target_amp = get_complement(self.dist[idx], self.data_size) / norm
            prepared_amp = basis.amplitude
            fid += target_amp * prepared_amp

        if not math.isnan(fid.real):
            return fid.real * fid.real + fid.imag * fid.imag
        return 0.0

    def run(self) -> None:
        """Execute the full state-preparation pipeline.

        Creates the quantum state with the work register and applies
        :class:`StatePrepViaQRAM` to prepare the target distribution.
        StatePrepViaQRAM manages its own temporary registers internally.
        """
        import warnings
        warnings.warn(
            "StatePreparation.run() is deprecated; "
            "use make_tree_and_qram() instead.",
            DeprecationWarning,
            stacklevel=2,
        )
        ps.System.clear()

        addr_sz = self.qubit_number + 1
        ps.System.add_register("addr_parent", ps.UnsignedInteger, addr_sz)

        self._state = ps.SparseState()

        state_prep_op = StatePrepViaQRAM(
            self.qram, "addr_parent", self.data_size, self.rational_size
        )
        state_prep_op(self._state)


# ==============================================================================
# Functional v2 API
# ==============================================================================


def make_tree_and_qram(
    distribution: list[float] | np.ndarray,
    *,
    data_size: int = 8,
    rational_size: int | None = None,
    work_reg: str | None = None,
    qram: object | None = None,
) -> tuple[StatePrepViaQRAM, object, str]:
    """Functional state-preparation pipeline (v2).

    Returns ``(operator, qram, work_reg)``.
    Does NOT call ``ps.System.clear()`` — caller is responsible.
    Does NOT return a result object — caller manages the state lifecycle.

    Example:
        >>> ps.System.clear()
        >>> op, qram, work_reg = make_tree_and_qram(
        ...     [0.5, 0.3, 0.1, 0.1], data_size=8)
        >>> state = ps.SparseState()
        >>> op(state)    # apply forward
        >>> op.dag(state)  # apply inverse
    """
    rational_size = rational_size or min(50, data_size * 2)
    dist_list = list(distribution)

    # Convert floats to integers for QRAM storage.
    # Scale so the largest value fits within the positive range of data_size bits.
    float_max = max(abs(v) for v in dist_list) if dist_list else 1.0
    scale = (1 << (data_size - 1)) / float_max if float_max > 0 else 1
    int_dist = [int(round(v * scale)) for v in dist_list]

    tree = make_vector_tree(int_dist, data_size)
    n_qubits = int(math.log2(len(dist_list)))
    if qram is None:
        qram = ps.QRAMCircuit_qutrit(n_qubits + 1, data_size, tree)
    if work_reg is None:
        work_reg = "addr_parent"
    ps.System.add_register(work_reg, ps.UnsignedInteger, n_qubits + 1)
    op = StatePrepViaQRAM(qram, work_reg, data_size, rational_size)
    return op, qram, work_reg


# Alias for discoverability
prepare_state_v2 = make_tree_and_qram


def create_state_preparation_demo() -> str:
    """Return a demo script string for state preparation.

    Returns:
        A multi-line Python source string that exercises the module.
    """
    return '''\
"""Demo: state preparation via QRAM."""

import pysparq as ps
from pysparq.algorithms.state_preparation import StatePreparation

# Parameters
qubit_number = 3
data_size = 8
data_range = 4

# Create the state preparation pipeline
sp = StatePreparation(qubit_number, data_size, data_range)

# Generate a random distribution
sp.random_distribution()
sp.show_distribution()

# Build binary tree and QRAM
sp.make_tree()
sp.show_tree()
sp.make_qram()
sp.set_qram()

# Run state preparation
sp.run()

# Check fidelity
fidelity = sp.get_fidelity()
print(f"Fidelity: {fidelity:.6f}")
'''
