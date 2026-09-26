#!/usr/bin/env python3
"""Driver for the correspondence experiment between the qutrit QRAM
(QRAM-Simulator) and circuit-level channel simulation (uniqc).

Consumes schedule.json / reference.json exported by QutritCorrespondenceExporter:

  Stage 0  noise-free bridge correctness: output distribution of the encoded
           gate circuit on the uniqc statevector backend vs the exact
           distribution of the QRAM-Simulator noise-free run -- must match
           bit by bit.
  Stage 1  fixed-noise realization: the extracted noise operators
           (type+coef -> deterministic unitary, mirroring the floor selection
           rules of SubBranch::run_depolarizing etc.) inserted as gates
           -- vs the exact distribution of run_full on the same-seed schedule,
           must be equal.
  Stage 2  channel level (the main event): channels placed at noise positions
             mode b = the extracted positions (where noise occurred in that
             realization);
             mode c = all active positions per step (the convention matching
             trajectory-averaged statistics).
           Data qubits use a Pauli channel (same semantics as the OriginIR-ext
           text channel); qutrit qubits use an 8-element Weyl-mixture Kraus
           (beyond the OriginIR-ext text channel set, injected via the kraus2q
           primitive of the same QuTiP density-matrix backend).
           Comparison metrics: renormalized or not x {classical distribution
           fidelity, TVD, quantum fidelity}.

Usage (with an interpreter that has uniqc installed, e.g.
UnifiedQuantum/.venv/bin/python):
  python run_correspondence.py --dir results/depol002 --stage all
"""

import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np
from uniqc import Circuit
from uniqc.simulator.simulator import Simulator
from uniqc.simulator.qutip_sim_impl import DensityOperatorSimulatorQutip

TWO_PI_3 = 2.0 * math.pi / 3.0


def omega(k: int) -> complex:
    return np.exp(1j * TWO_PI_3 * k)


# --------------------------------------------------------------------------
# JSON gate IR -> uniqc.Circuit (used by Stage 0/1)
# --------------------------------------------------------------------------

class _NullCtx:
    def __enter__(self):
        return None

    def __exit__(self, *args):
        return False


def gate(c: Circuit, op: str, qubits, ctrls=(), theta=None):
    """Apply a gate with a (positive/negative polarity) control set. Negative controls are wrapped with X into normal controls."""
    ctrls = list(ctrls)
    neg = [q for q, v in ctrls if v == 0]
    pos = [q for q, v in ctrls if v == 1]
    allc = neg + pos
    for q in neg:
        c.x(q)
    ctx = c.control(*allc) if allc else _NullCtx()
    with ctx:
        if op == "X":
            c.x(qubits[0])
        elif op == "H":
            c.h(qubits[0])
        elif op == "SWAP":
            c.swap(qubits[0], qubits[1])
        elif op == "CNOT":
            c.cnot(qubits[0], qubits[1])
        elif op == "U1":
            c.u1(qubits[0], theta)
        else:
            raise ValueError(f"unknown op {op}")
    for q in neg:
        c.x(q)


def emit_transposition(c: Circuit, qs, pi: int, pj: int, extra_ctrls=()):
    """qs[k] <-> index bit k. Basis-state transposition (pi <-> pj), overall
    controlled by extra_ctrls. Same conjugation algorithm as the C++ side's
    emit_transposition."""
    k = len(qs)
    D = [b for b in range(k) if ((pi >> b) & 1) != ((pj >> b) & 1)]
    if not D:
        return

    def others_ctrl(exclude):
        ret = list(extra_ctrls)
        for b in range(k):
            if b != exclude:
                ret.append((qs[b], (pi >> b) & 1))
        return ret

    if len(D) == 1:
        gate(c, "X", [qs[D[0]]], others_ctrl(D[0]))
        return
    p, s = D[0], (pi >> D[0]) & 1
    rest = D[1:]
    if s:
        gate(c, "X", [qs[p]], extra_ctrls)
    for b in rest:
        gate(c, "CNOT", [qs[p], qs[b]], extra_ctrls)
    gate(c, "X", [qs[p]], others_ctrl(p))
    for b in reversed(rest):
        gate(c, "CNOT", [qs[p], qs[b]], extra_ctrls)
    if s:
        gate(c, "X", [qs[p]], extra_ctrls)


# addr-qutrit levels (4x4 index = 2*bit(a1)+bit(a0)): W=0, L=1, R=2, dead=3
QUTRIT_A1 = {0: 2, 2: 1, 1: 0}     # rotate_A1: L->W, W->R, R->L  (old->new)
QUTRIT_A1_2 = {0: 1, 1: 2, 2: 0}   # rotate_A2 (permutation): W->L, L->R, R->W


def emit_qutrit_perm(c: Circuit, a1: int, a0: int, mapping):
    """A 3-cycle decomposed into 2 transpositions; transpositions are emitted on (a1,a0) by conjugation. bit0=a0, bit1=a1."""
    qs = [a0, a1]
    m = dict(mapping)
    m.setdefault(3, 3)
    # Decompose the mapping into transpositions (application order = emission order: ascending (cycle[0], cycle[i]) compose back into the original mapping)
    seen = set()
    trans = []
    for x in sorted(m):
        if x in seen:
            continue
        cycle = []
        y = x
        while y not in seen:
            seen.add(y)
            cycle.append(y)
            y = m[y]
        for i in range(1, len(cycle)):
            trans.append((cycle[0], cycle[i]))
    for i, j in trans:
        emit_transposition(c, qs, i, j)


def emit_qutrit_phase(c: Circuit, a1: int, a0: int, s: int):
    """diag: φ(L)=ω^s, φ(R)=ω^{2s}, φ(W)=1 (phase semantics of SubBranch::run_A2)."""
    gate(c, "U1", [a0], [(a1, 0)], theta=TWO_PI_3 * s)
    gate(c, "U1", [a1], [(a0, 0)], theta=2.0 * TWO_PI_3 * s)


def emit_swap_then_phase_r(c: Circuit, a1: int, a0: int):
    """BitPhaseFlip (addr): applies a -1 phase to the R level after addr_flip.
    (Already absorbed into Depolarizing-only; kept as a constituent primitive
    for QUTRIT Weyl-family decomposition.)"""
    c.swap(a1, a0)
    gate(c, "U1", [a1], [(a0, 0)], theta=math.pi)


def apply_noise_unitary(c: Circuit, o: dict, enc: dict):
    """Stage 1: realize the extracted Depolarizing operator as a deterministic
    unitary following QRAM-Simulator's floor rules (after sampling it has
    already become a concrete X/Z/Y or qutrit Weyl operator)."""
    node = enc["nodes"][o["node"]]
    coef = o["coef"]
    if o["type"] != "Depolarizing":
        raise ValueError(f"Only Depolarizing is supported (got {o['type']}; Damping is M3)")
    if o["sub"] == "data":
        q = node["d"]
        k = math.floor(3 * coef)
        if k == 0:
            c.x(q)
        elif k == 1:
            c.z(q)
        else:
            c.x(q)
            c.z(q)
    else:
        a1, a0 = node["a1"], node["a0"]
        k = math.floor(8 * coef)
        phase = k in (1, 4, 5)
        s = 2 if k in (3, 6, 7) else 1
        perm = None
        if k in (0, 4, 6):
            perm = QUTRIT_A1
        elif k in (2, 5, 7):
            perm = QUTRIT_A1_2
        if phase:
            emit_qutrit_phase(c, a1, a0, s)
        if perm is not None:
            emit_qutrit_perm(c, a1, a0, perm)


def build_circuit(sched: dict, noise: str) -> Circuit:
    """noise: 'skip' (Stage 0) | 'unitary' (Stage 1)"""
    enc = sched["encoding"]
    c = Circuit()
    # Touch every qubit (X*X is identity): uniqc compacts and reorders untouched
    # qubits; after explicit registration the index mapping is qubit k <-> basis bit k
    for q in range(enc["num_qubits"]):
        c.x(q)
        c.x(q)
    for q in enc["address_qubits"]:
        c.h(q)
    if sched["input"]["mode"] == "uniform":
        for q in enc["bus_qubits"]:
            c.h(q)
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                gate(c, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                     theta=o.get("theta"))
            elif o["kind"] == "noise":
                if noise == "unitary":
                    apply_noise_unitary(c, o, enc)
                elif noise != "skip":
                    raise ValueError(noise)
    return c


# --------------------------------------------------------------------------
# Distribution extraction and comparison
# --------------------------------------------------------------------------

def detect_index_bit_order() -> str:
    """Probe the basis-index bit convention of the uniqc statevector: whether qubit 0 is the LSB or the MSB."""
    c = Circuit()
    c.x(0)
    c.h(1)  # make sure the circuit has 2 qubits
    c.h(1)
    sim = Simulator(backend_type="statevector")
    probs = np.abs(np.asarray(sim.simulate_statevector(c.originir))) ** 2
    return "lsb" if int(np.argmax(probs)) == 1 else "msb"


def marginal_address_bus(probs, sched: dict, order: str) -> dict:
    enc = sched["encoding"]
    n = enc["num_qubits"]
    addr_qs, bus_qs = enc["address_qubits"], enc["bus_qubits"]

    def bit(idx, q):
        return (idx >> q) & 1 if order == "lsb" else (idx >> (n - 1 - q)) & 1

    dist = {}
    for idx, p in enumerate(probs):
        a = sum(bit(idx, q) << k for k, q in enumerate(addr_qs))
        b = sum(bit(idx, q) << k for k, q in enumerate(bus_qs))
        key = f"{a}:{b}"
        dist[key] = dist.get(key, 0.0) + float(p)
    return dist


def classical_fidelity(p: dict, q: dict) -> float:
    keys = set(p) | set(q)
    return float(sum(math.sqrt(p.get(k, 0.0) * q.get(k, 0.0)) for k in keys) ** 2)


def tvd(p: dict, q: dict) -> float:
    keys = set(p) | set(q)
    return 0.5 * float(sum(abs(p.get(k, 0.0) - q.get(k, 0.0)) for k in keys))


def compare_dist(name: str, got: dict, want: dict, tol: float) -> bool:
    keys = sorted(set(got) | set(want))
    diff = max(abs(got.get(k, 0.0) - want.get(k, 0.0)) for k in keys)
    ok = diff < tol
    print(f"  [{name}] max|ΔP| = {diff:.3e}  ->  {'MATCH' if ok else 'MISMATCH'}")
    if not ok:
        for k in keys:
            g, w = got.get(k, 0.0), want.get(k, 0.0)
            if abs(g - w) > tol:
                print(f"    {k}: uniqc={g:.12f}  qram={w:.12f}")
    return ok


# --------------------------------------------------------------------------
# Stage 1-d / Stage 2-d: Damping paths (single-Kraus trajectory replay + channel-level full/nojump)
# --------------------------------------------------------------------------

def be_transposition(be, qs, pi: int, pj: int, extra_ctrls=()):
    """Backend version of emit_transposition (applied directly on the density matrix)."""
    k = len(qs)
    D = [b for b in range(k) if ((pi >> b) & 1) != ((pj >> b) & 1)]
    if not D:
        return

    def others_ctrl(exclude):
        ret = list(extra_ctrls)
        for b in range(k):
            if b != exclude:
                ret.append((qs[b], (pi >> b) & 1))
        return ret

    if len(D) == 1:
        be_gate(be, "X", [qs[D[0]]], others_ctrl(D[0]))
        return
    p, s = D[0], (pi >> D[0]) & 1
    rest = D[1:]
    if s:
        be_gate(be, "X", [qs[p]], extra_ctrls)
    for b in rest:
        be_gate(be, "CNOT", [qs[p], qs[b]], extra_ctrls)
    be_gate(be, "X", [qs[p]], others_ctrl(p))
    for b in reversed(rest):
        be_gate(be, "CNOT", [qs[p], qs[b]], extra_ctrls)
    if s:
        be_gate(be, "X", [qs[p]], extra_ctrls)


def apply_depol_unitary_be(be, o: dict, enc: dict):
    """Deterministic unitary of Depolarizing (backend version, used for the Stage 1-d replay of mixed configurations)."""
    node = enc["nodes"][o["node"]]
    if o["sub"] == "data":
        q = node["d"]
        k = math.floor(3 * o["coef"])
        if k == 0:
            be.x(q)
        elif k == 1:
            be.z(q)
        else:
            be.x(q)
            be.z(q)
        return
    a1, a0 = node["a1"], node["a0"]
    k = math.floor(8 * o["coef"])
    if k in (1, 3, 4, 5, 6, 7):
        s = 2 if k in (3, 6, 7) else 1
        be_gate(be, "U1", [a0], [(a1, 0)], theta=TWO_PI_3 * s)
        be_gate(be, "U1", [a1], [(a0, 0)], theta=2.0 * TWO_PI_3 * s)
    perm = None
    if k in (0, 4, 6):
        perm = {0: 2, 2: 1, 1: 0}
    elif k in (2, 5, 7):
        perm = {0: 1, 1: 2, 2: 0}
    if perm is not None:
        m = dict(perm)
        m.setdefault(3, 3)
        seen = set()
        trans = []
        for x in sorted(m):
            if x in seen:
                continue
            cycle = []
            y = x
            while y not in seen:
                seen.add(y)
                cycle.append(y)
                y = m[y]
            for i in range(1, len(cycle)):
                trans.append((cycle[0], cycle[i]))
        for i, j in trans:
            be_transposition(be, [a0, a1], i, j)


def apply_jump_kraus(be, o: dict, enc: dict):
    """Damp_Full jump with a fixed outcome (single Kraus, no √γ factor -- the weight is carried by the position sampling probability)."""
    node = enc["nodes"][o["node"]]
    if o["sub"] == "data":
        K = np.array([[0.0, 1.0], [0.0, 0.0]], dtype=complex)  # |0><1|
        be.kraus1q(node["d"], [K.reshape(-1).tolist()])
    else:
        K = np.zeros((4, 4), dtype=complex)
        K[0, 1 if o["outcome"] == 0 else 2] = 1.0  # |W><L| or |W><R|
        be.kraus2q(node["a1"], node["a0"], [K.reshape(-1).tolist()])


def apply_k0_kraus(be, enc: dict, gamma: float, jumps: bool):
    """Encoding-side counterpart of Damp_Common: acts on all node degrees of freedom.
    jumps=False -> K0 only (nojump mode); jumps=True -> the full textbook 3-level AD channel."""
    s = math.sqrt(1.0 - gamma)
    for node in enc["nodes"]:
        if jumps:
            be.amplitude_damping(node["d"], gamma)
            K0 = np.diag([1.0, s, s, 1.0]).astype(complex)
            KL = np.zeros((4, 4), dtype=complex)
            KL[0, 1] = math.sqrt(gamma)
            KR = np.zeros((4, 4), dtype=complex)
            KR[0, 2] = math.sqrt(gamma)
            be.kraus2q(node["a1"], node["a0"],
                       [K.reshape(-1).tolist() for K in (K0, KL, KR)])
        else:
            be.kraus1q(node["d"], [np.diag([1.0, s]).astype(complex).reshape(-1).tolist()])
            be.kraus2q(node["a1"], node["a0"],
                       [np.diag([1.0, s, s, 1.0]).astype(complex).reshape(-1).tolist()])


def excitation_weights(be, enc: dict):
    """Read the excitation weight of each degree of freedom from the current density-matrix diagonal (used by the faithful mirror of the sub-normalized trajectory ensemble)."""
    diag = np.asarray(be.density_matrix.diag())
    s = float(np.real(np.sum(diag)))
    n = enc["num_qubits"]
    idx = np.arange(2 ** n)
    weights = {}
    for node in enc["nodes"]:
        a1_bits = ((idx >> node["a1"]) & 1).astype(bool)
        a0_bits = ((idx >> node["a0"]) & 1).astype(bool)
        d_bits = ((idx >> node["d"]) & 1).astype(bool)
        weights[(node["id"], "addr")] = {
            "L": float(np.real(diag[(~a1_bits) & a0_bits].sum())),
            "R": float(np.real(diag[a1_bits & (~a0_bits)].sum())),
        }
        weights[(node["id"], "data")] = float(np.real(diag[d_bits].sum()))
    return weights, s


def apply_faithful_kraus(be, enc: dict, gamma: float, entangle_max: int):
    """Faithful mirror: step-by-step reproduction of the average effect of their
    sampling scheme (a ρ-dependent nonlinear map).
    Jumps can only occur at the active positions that were drawn (nodes < 2^L-1):
    K0 branch weight 1-γw/S, jump branch √(γ·w_k/S)·M_k (trajectories are not
    renormalized, contribution = (γw_k/S)·M_kρM_k†);
    inactive nodes only undergo the pure K0 of Damp_Common.
    The nonlinear map takes its weights on the averaged ρ, which differs from the
    per-trajectory average at first order and beyond (see README)."""
    weights, s = excitation_weights(be, enc)
    if s <= 0:
        return
    sq = math.sqrt(1.0 - gamma)
    n_active_nodes = (2 ** entangle_max) - 1
    for node in enc["nodes"]:
        w = weights[(node["id"], "data")]
        wL = weights[(node["id"], "addr")]["L"]
        wR = weights[(node["id"], "addr")]["R"]
        active = node["id"] < n_active_nodes
        a_d = math.sqrt(max(1.0 - gamma * w / s, 0.0)) if active else 1.0
        a_q = math.sqrt(max(1.0 - gamma * (wL + wR) / s, 0.0)) if active else 1.0
        K0d = np.diag([a_d, a_d * sq]).astype(complex)
        K0q = np.diag([a_q, a_q * sq, a_q * sq, a_q]).astype(complex)
        if active:
            MJ = np.zeros((2, 2), dtype=complex)
            MJ[0, 1] = math.sqrt(gamma * w / s)
            ML = np.zeros((4, 4), dtype=complex)
            ML[0, 1] = math.sqrt(gamma * wL / s)
            MR = np.zeros((4, 4), dtype=complex)
            MR[0, 2] = math.sqrt(gamma * wR / s)
            be.kraus1q(node["d"], [K0d.reshape(-1).tolist(), MJ.reshape(-1).tolist()])
            be.kraus2q(node["a1"], node["a0"],
                       [K.reshape(-1).tolist() for K in (K0q, ML, MR)])
        else:
            be.kraus1q(node["d"], [K0d.reshape(-1).tolist()])
            be.kraus2q(node["a1"], node["a0"], [K0q.reshape(-1).tolist()])


def init_backend(sched: dict):
    enc = sched["encoding"]
    be = DensityOperatorSimulatorQutip()
    be.init_n_qubit(enc["num_qubits"])
    for q in enc["address_qubits"]:
        be.hadamard(q)
    if sched["input"]["mode"] == "uniform":
        for q in enc["bus_qubits"]:
            be.hadamard(q)
    return be


def marginal_from_rho(rho, sched: dict, order: str) -> dict:
    enc = sched["encoding"]
    n = enc["num_qubits"]

    def bit(idx, q):
        return (idx >> q) & 1 if order == "lsb" else (idx >> (n - 1 - q)) & 1

    dist = {}
    for idx in range(2 ** n):
        a = sum(bit(idx, q) << k for k, q in enumerate(enc["address_qubits"]))
        b = sum(bit(idx, q) << k for k, q in enumerate(enc["bus_qubits"]))
        key = f"{a}:{b}"
        dist[key] = dist.get(key, 0.0) + float(rho[idx, idx].real)
    return dist


def replay_trajectory(sched: dict, order: str):
    """Stage 1-d: operator-by-operator replay of the exported realization
    (including Damp_Full outcomes).
    Gates = unitary; Depolarizing = deterministic unitary; Damp_Common = K0
    single Kraus (whole tree); Damp_Full = fixed-outcome jump single Kraus
    (nojump skipped). Returns the sub-normalized ρ and the marginal distribution."""
    enc = sched["encoding"]
    be = init_backend(sched)
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                be_gate(be, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                        theta=o.get("theta"))
            elif o["type"] == "Depolarizing":
                apply_depol_unitary_be(be, o, enc)
            elif o["type"] == "Damp_Full":
                if o.get("outcome", -2) == -2:
                    raise ValueError("Damp_Full is missing outcome (the exporter must replicate the execution record)")
                if o.get("outcome", -1) >= 0:
                    apply_jump_kraus(be, o, enc)
            elif o["type"] == "Damp_Common":
                apply_k0_kraus(be, enc, o["coef"], jumps=False)
            else:
                raise ValueError(o["type"])
    rho = np.array(be.density_matrix.full())
    return rho, marginal_from_rho(rho, sched, order)


def run_stage2_damping(sched: dict, order: str, damp_mode: str, depol_mode: str = "c"):
    """Stage 2-d: damp_mode ∈ {full, nojump, faithful}.
    full     = at each Damp_Common occurrence, apply the textbook 3-level AD channel (TP) to all node degrees of freedom;
    nojump   = K0 single Kraus only (non-TP, trace = no-jump survival probability);
    faithful = ρ-dependent map reproducing the average effect of their sampling scheme amplitude by amplitude (see apply_faithful_kraus).
    Depolarizing (mixed configuration) applies channels per the depol_mode b/c position strategy."""
    enc = sched["encoding"]
    noise_cfg = sched["noise"]
    be = init_backend(sched)
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                be_gate(be, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                        theta=o.get("theta"))
            elif o["type"] == "Depolarizing" and depol_mode == "b":
                apply_channel_at(be, o, enc, noise_cfg)
        if depol_mode == "c":
            n_active = 2 * ((2 ** st["entangle_max"]) - 1)
            for pos in range(n_active):
                if "Depolarizing" in noise_cfg:
                    apply_channel_at(be, {"type": "Depolarizing", "node": pos // 2,
                                          "sub": "data" if pos % 2 else "addr"},
                                     enc, noise_cfg)
        for o in st["ops"]:
            if o["kind"] == "noise" and o["type"] == "Damp_Common":
                if damp_mode == "faithful":
                    apply_faithful_kraus(be, enc, o["coef"], st["entangle_max"])
                else:
                    apply_k0_kraus(be, enc, o["coef"], jumps=(damp_mode == "full"))
    rho = np.array(be.density_matrix.full())
    return rho, marginal_from_rho(rho, sched, order)


# --------------------------------------------------------------------------
# qubit architecture: noise primitives / per-case replay / channel level
# --------------------------------------------------------------------------

# run_bitphaseflip (library already fixed to a Y flip): |0>->-|1>, |1>->|0> (ZX = iY, unitary).
# The old implementation was |0>->|0>, |1>->-|0> (rank-1, non-unitary) -- coherent
# cancellation on repeated states used to cause zero-norm trajectories and
# sample_output crashes; after the fix, qubit-architecture Depolarizing is
# restored to norm-preserving trajectories.
QUBIT_M = np.array([[0.0, 1.0], [-1.0, 0.0]], dtype=complex)


def qubit_noise_qubit_of(o: dict, enc: dict) -> int:
    node = enc["nodes"][o["node"]]
    return node["d"] if o["sub"] == "data" else node["a"]


def apply_qubit_noise_op_be(be, o: dict, enc: dict):
    t = o["type"]
    if t == "Damp_Common":
        K0 = np.diag([1.0, math.sqrt(1 - o["coef"])]).astype(complex)
        for node in enc["nodes"]:
            be.kraus1q(node["a"], [K0.reshape(-1).tolist()])
            be.kraus1q(node["d"], [K0.reshape(-1).tolist()])
        return
    q = qubit_noise_qubit_of(o, enc)
    if t == "BitFlip":
        be.x(q)
    elif t == "PhaseFlip":
        be.z(q)
    elif t == "BitPhaseFlip":
        be.kraus1q(q, [QUBIT_M.reshape(-1).tolist()])
    elif t == "Depolarizing":
        k = math.floor(3 * o.get("coef", 0.0))
        if k == 0:
            be.x(q)
        elif k == 1:
            be.z(q)
        else:
            be.kraus1q(q, [QUBIT_M.reshape(-1).tolist()])
    elif t == "Damp_Full":
        if o.get("outcome", -2) == -2:
            raise ValueError("Damp_Full is missing outcome (the exporter must replicate the execution record)")
        if o.get("outcome", -1) >= 0:
            K = np.array([[0.0, 1.0], [0.0, 0.0]], dtype=complex)  # |0><1|
            be.kraus1q(q, [K.reshape(-1).tolist()])
    else:
        raise ValueError(t)


def replay_trajectory_qubit(sched: dict, order: str):
    """Replay fixed joint Kraus labels; preserve the original coherent state.

    New exports normalize at each damping-layer boundary. Old exports retain
    their historical raw-norm convention for reproducibility.
    """
    enc = sched["encoding"]
    be = init_backend(sched)
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                be_gate(be, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                        theta=o.get("theta"))
            else:
                apply_qubit_noise_op_be(be, o, enc)
                if o["type"] == "Damp_Common" and sched.get("normalize_damping_layers"):
                    trace = float(np.real(be.density_matrix.tr()))
                    if trace <= 0:
                        raise ValueError("zero-norm joint damping outcome")
                    be.density_matrix = be.density_matrix / trace
    rho = np.array(be.density_matrix.full())
    return rho, marginal_from_rho(rho, sched, order)


def run_stage2_qubit(sched: dict, order: str, mode: str):
    """Independent qubit circuit channel; `full` is standard whole-tree AD.

    `faithful` retains the retired state-dependent mirror only for old exports;
    it is not a fixed physical amplitude-damping channel. Pauli noise is the
    active-front {I, X, Z, ZX} mixture with total error probability p.
    """
    enc = sched["encoding"]
    noise_cfg = sched["noise"]
    be = init_backend(sched)
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                be_gate(be, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                        theta=o.get("theta"))
        if "Depolarizing" in noise_cfg:
            faithful = mode != "depol_textbook"
            p = noise_cfg["Depolarizing"]
            n_active = 2 * ((2 ** st["entangle_max"]) - 1)
            for pos in range(n_active):
                node = enc["nodes"][pos // 2]
                q = node["d"] if pos % 2 else node["a"]
                if not faithful:
                    be.depolarizing(q, p)
                else:
                    c0 = math.sqrt(1 - p)
                    c1 = math.sqrt(p / 3.0)
                    kx = np.array([[0, 1], [1, 0]], dtype=complex)
                    kz = np.array([[1, 0], [0, -1]], dtype=complex)
                    ks = [c0 * np.eye(2, dtype=complex), c1 * kx, c1 * kz, c1 * QUBIT_M]
                    be.kraus1q(q, [K.reshape(-1).tolist() for K in ks])
        for o in st["ops"]:
            if o["kind"] == "noise" and o["type"] == "Damp_Common":
                gamma = o["coef"]
                s = math.sqrt(1 - gamma)
                diag = np.asarray(be.density_matrix.diag())
                S = float(np.real(diag.sum()))
                if S <= 0:
                    continue
                n = enc["num_qubits"]
                idx = np.arange(2 ** n)
                n_active_nodes = (2 ** st["entangle_max"]) - 1
                for node in enc["nodes"]:
                    for q in (node["a"], node["d"]):
                        if mode == "full":
                            be.amplitude_damping(q, gamma)
                            continue
                        K0 = np.diag([1.0, s]).astype(complex)
                        KJ = None
                        if mode == "faithful":
                            w = float(np.real(diag[((idx >> q) & 1).astype(bool)].sum()))
                            if node["id"] < n_active_nodes:
                                a = math.sqrt(max(1.0 - gamma * w / S, 0.0))
                                K0 = np.diag([a, a * s]).astype(complex)
                                KJ = np.zeros((2, 2), dtype=complex)
                                KJ[0, 1] = math.sqrt(gamma * w / S)
                        be.kraus1q(q, [K.reshape(-1).tolist() for K in ((K0, KJ) if KJ is not None else (K0,))])
    rho = np.array(be.density_matrix.full())
    return rho, marginal_from_rho(rho, sched, order)


# --------------------------------------------------------------------------
# Stage 2: direct drive on the QuTiP density-matrix backend (channel level)
# --------------------------------------------------------------------------

def be_gate(be, op: str, qubits, ctrls=(), theta=None):
    ctrls = list(ctrls)
    neg = [q for q, v in ctrls if v == 0]
    pos = [q for q, v in ctrls if v == 1]
    allc = neg + pos
    for q in neg:
        be.x(q)
    if op == "X":
        be.x(qubits[0], allc)
    elif op == "H":
        be.hadamard(qubits[0], allc)
    elif op == "SWAP":
        be.swap(qubits[0], qubits[1], allc)
    elif op == "CNOT":
        be.cnot(qubits[0], qubits[1], allc)
    elif op == "U1":
        be.u1(qubits[0], theta, allc)
    else:
        raise ValueError(op)
    for q in neg:
        be.x(q)


def qutrit_unitary_4x4(coef: float) -> np.ndarray:
    """4x4 embedding of the 8-element Weyl unitary of qutrit Depolarizing
    (identity on the dead state).
    Index = 2·bit(a1)+bit(a0): W=0, L=1, R=2, dead=3."""
    k = math.floor(8 * coef)
    perm = {0: 2, 2: 1, 1: 0} if k in (0, 4, 6) else (
        {0: 1, 1: 2, 2: 0} if k in (2, 5, 7) else None)
    s = 2 if k in (3, 6, 7) else 1
    U = np.zeros((4, 4), dtype=complex)
    U[3, 3] = 1.0  # identity on the dead state
    for old in range(3):
        new = perm[old] if perm else old
        ph = 1.0
        if k in (1, 4, 5, 6, 7):
            # Per run_A2 semantics the phase attaches to the pre-permutation level: L->ω^s, R->ω^{2s}
            ph = omega(s) if old == 1 else (omega(2 * s) if old == 2 else 1.0)
        U[new, old] = ph
    return U


def apply_channel_at(be, o: dict, enc: dict, noise_cfg: dict, p_scale: float = 1.0):
    """Place a channel at the extracted position. p is the per-qubit probability of Depolarizing in the QRAM noise model."""
    node = enc["nodes"][o["node"]]
    p = noise_cfg[o["type"]] * p_scale
    if o["type"] != "Depolarizing":
        raise ValueError(f"Only Depolarizing is supported (got {o['type']}; Damping is M3)")
    if o["sub"] == "data":
        be.depolarizing(node["d"], p)
    else:
        a1, a0 = node["a1"], node["a0"]
        kraus = [np.sqrt(1 - p) * np.eye(4, dtype=complex)]
        for kk in range(8):
            kraus.append(np.sqrt(p / 8.0) * qutrit_unitary_4x4((kk + 0.5) / 8.0))
        be.kraus2q(a1, a0, [K.reshape(-1).tolist() for K in kraus])


def run_stage2(sched: dict, mode: str, order: str):
    enc = sched["encoding"]
    noise_cfg = sched["noise"]
    be = DensityOperatorSimulatorQutip()
    be.init_n_qubit(enc["num_qubits"])
    for q in enc["address_qubits"]:
        be.hadamard(q)
    if sched["input"]["mode"] == "uniform":
        for q in enc["bus_qubits"]:
            be.hadamard(q)

    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                be_gate(be, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                        theta=o.get("theta"))
            elif o["kind"] == "noise" and mode == "b":
                apply_channel_at(be, o, enc, noise_cfg)
        if mode == "c" and noise_cfg:
            n_active = 2 * ((2 ** st["entangle_max"]) - 1)
            # After the closed-interval sampling fix: each step's nerror ~ Binomial(n_active, p)
            # falls uniformly into the n_active active positions of [0, n_active)
            # -> each position's marginal probability is exactly p;
            # steps with n_active=0 (entangle_max=0) carry no noise.
            for pos in range(n_active):
                for t in noise_cfg:
                    apply_channel_at(be, {
                        "type": t, "node": pos // 2,
                        "sub": "data" if pos % 2 else "addr"}, enc, noise_cfg)

    rho = np.array(be.density_matrix.full())
    n = enc["num_qubits"]

    def bit(idx, q):
        return (idx >> q) & 1 if order == "lsb" else (idx >> (n - 1 - q)) & 1

    addr_qs, bus_qs = enc["address_qubits"], enc["bus_qubits"]
    dist = {}
    for idx in range(2 ** n):
        a = sum(bit(idx, q) << k for k, q in enumerate(addr_qs))
        b = sum(bit(idx, q) << k for k, q in enumerate(bus_qs))
        key = f"{a}:{b}"
        dist[key] = dist.get(key, 0.0) + float(rho[idx, idx].real)
    return rho, dist


def run_stage2_originir(sched: dict, order: str, outdir):
    """OriginIR-ext text path for mode b: data-qubit channels are injected
    inline as text opcodes (Depolarizing/BitFlip/PhaseFlip/PauliError1Q, same
    semantics as OriginIR-ext channel statements); the 8-element Weyl mixture on
    qutrit qubits (addr subsystem) exceeds the text channel set -> the text is a
    complete circuit only when the schedule contains no addr-qubit noise, in
    which case it can run directly on the standard Simulator path and be
    cross-checked against the direct-drive result."""
    enc = sched["encoding"]
    noise_cfg = sched["noise"]
    c = Circuit()
    for q in range(enc["num_qubits"]):
        c.x(q)
        c.x(q)
    for q in enc["address_qubits"]:
        c.h(q)
    if sched["input"]["mode"] == "uniform":
        for q in enc["bus_qubits"]:
            c.h(q)
    n_addr_ops = 0
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                gate(c, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                     theta=o.get("theta"))
            elif o["kind"] == "noise":
                if o["type"] != "Depolarizing":
                    raise ValueError(f"Only Depolarizing is supported (got {o['type']})")
                if o["sub"] == "data":
                    c.add_gate("Depolarizing", enc["nodes"][o["node"]]["d"],
                               params=noise_cfg["Depolarizing"])
                else:
                    n_addr_ops += 1
    text = c.originir
    (outdir / "circuit_channels.originir").write_text(text)
    if n_addr_ops:
        print(f"  [originir] text exported, but it contains {n_addr_ops} qutrit(addr)-qubit noise ops"
              f" -- beyond the OriginIR-ext text channel set; full simulation goes through the direct kraus path")
        return None
    rho = np.asarray(Simulator(backend_type="density_matrix_qutip").simulate_density_matrix(text))
    n = enc["num_qubits"]

    def bit(idx, q):
        return (idx >> q) & 1 if order == "lsb" else (idx >> (n - 1 - q)) & 1

    dist = {}
    for idx in range(2 ** n):
        a = sum(bit(idx, q) << k for k, q in enumerate(enc["address_qubits"]))
        b = sum(bit(idx, q) << k for k, q in enumerate(enc["bus_qubits"]))
        dist[f"{a}:{b}"] = dist.get(f"{a}:{b}", 0.0) + float(rho[idx, idx].real)
    return dist


# --------------------------------------------------------------------------
# Main flow
# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", required=True, help="exporter output directory (containing schedule.json / reference.json)")
    ap.add_argument("--stage", default="all", choices=["0", "1", "2", "all"])
    ap.add_argument("--tol", type=float, default=1e-9)
    args = ap.parse_args()

    d = Path(args.dir)
    sched = json.loads((d / "schedule.json").read_text())
    ref = json.loads((d / "reference.json").read_text())

    sched_noise_types = {o["type"] for st in sched["steps"] for o in st["ops"]
                         if o["kind"] == "noise"}
    unsupported = sched_noise_types - {"Depolarizing", "Damp_Full", "Damp_Common"}
    if unsupported:
        print(f"FAILED: schedule contains unsupported noise types {unsupported}")
        sys.exit(1)
    has_damp = bool(sched_noise_types & {"Damp_Full", "Damp_Common"})
    arch = sched.get("arch", "qutrit")
    if (arch == "qubit" and sched.get("damping_semantics") == "joint_auxiliary_whole_tree_v1"
            and sched.get("input", {}).get("mode") != "zerobus"):
        raise ValueError("Coherent circuit comparison requires the one-bus-column-per-address zerobus input; "
                         "the engine's multi-column probability bookkeeping is a different input convention")
    order = detect_index_bit_order()
    print(f"uniqc statevector index convention: qubit0={'LSB' if order == 'lsb' else 'MSB'}")

    summary = {"dir": str(d), "index_order": order}
    ok = True

    if args.stage in ("0", "all"):
        print("\n== Stage 0: noise-free bridge correctness (encoded gate circuit vs QRAM-Simulator noise-free run) ==")
        c = build_circuit(sched, "skip")
        sim = Simulator(backend_type="statevector")
        psi = np.asarray(sim.simulate_statevector(c.originir))
        probs = np.abs(psi) ** 2
        dist = marginal_address_bus(probs, sched, order)
        ok &= compare_dist("stage0", dist, ref["noisefree_dist"], args.tol)
        summary["stage0"] = {"dist": dist, "ok": bool(ok)}
        (d / "circuit_noisefree.originir").write_text(c.originir)

    if args.stage in ("1", "all"):
        if arch == "qubit":
            print("\n== Stage 1 (qubit): per-random-case replay (gates + Depolarizing unitary/Kraus + K0/jump single Kraus) ==")
            n_cases = 0
            case_ok = True
            for sched_file in sorted(d.glob("schedule_r*.json")):
                ridx = sched_file.stem.replace("schedule_r", "")
                want = ref.get("realizations", {}).get(ridx)
                if want is None:
                    continue
                cs = json.loads(sched_file.read_text())
                rho, dist = replay_trajectory_qubit(cs, order)
                case_ok &= compare_dist(f"case r{ridx}", dist, want, args.tol)
                n_cases += 1
            print(f"  per-case replay: {n_cases} random realizations, {'all exact matches' if case_ok else 'mismatches present'}")
            ok &= case_ok
            summary["stage1"] = {"arch": "qubit", "cases": n_cases, "cases_ok": bool(case_ok)}
        elif has_damp:
            print("\n== Stage 1-d: per-random-case replay (gates + Depolarizing unitary + K0 single Kraus + "
                  "fixed-outcome jump single Kraus) ==")
            n_cases = 0
            case_ok = True
            for sched_file in sorted(d.glob("schedule_r*.json")):
                ridx = sched_file.stem.replace("schedule_r", "")
                want = ref.get("realizations", {}).get(ridx)
                if want is None:
                    continue
                cs = json.loads(sched_file.read_text())
                rho, dist = replay_trajectory(cs, order)
                case_ok &= compare_dist(f"case r{ridx}", dist, want, args.tol)
                n_cases += 1
            print(f"  per-case replay: {n_cases} random realizations (sub-normalized convention), "
                  f"{'all exact matches' if case_ok else 'mismatches present'}")
            ok &= case_ok
            summary["stage1d"] = {"cases": n_cases, "cases_ok": bool(case_ok)}
        else:
            print("\n== Stage 1: exact per-random-case cross-check (sampling done -> concrete operators -> deterministic unitary gates) ==")
            n_cases = 0
            case_ok = True
            for sched_file in sorted(d.glob("schedule_r*.json")):
                ridx = sched_file.stem.replace("schedule_r", "")
                want = ref.get("realizations", {}).get(ridx)
                if want is None:
                    continue
                cs = json.loads(sched_file.read_text())
                c = build_circuit(cs, "unitary")
                psi = np.asarray(Simulator(backend_type="statevector")
                                 .simulate_statevector(c.originir))
                dist = marginal_address_bus(np.abs(psi) ** 2, cs, order)
                case_ok &= compare_dist(f"case r{ridx}", dist, want, args.tol)
                n_cases += 1
            print(f"  per-case cross-check: {n_cases} random realizations, {'all exact matches' if case_ok else 'mismatches present'}")
            ok &= case_ok
            # r=0 summary row + model cross-validation (after the library fix, native and model should agree bit by bit)
            c = build_circuit(sched, "unitary")
            psi = np.asarray(Simulator(backend_type="statevector")
                             .simulate_statevector(c.originir))
            dist = marginal_address_bus(np.abs(psi) ** 2, sched, order)
            ok &= compare_dist("stage1 r0 vs native", dist, ref["single_run_dist"], args.tol)
            if ref.get("model_single_run_dist"):
                ok &= compare_dist("stage1 r0 vs model", dist, ref["model_single_run_dist"], args.tol)
            (d / "circuit_realization.originir").write_text(c.originir)
            summary["stage1"] = {"dist": dist, "cases": n_cases, "cases_ok": bool(case_ok)}

    if args.stage in ("2", "all"):
        if arch == "qubit":
            print("\n== Stage 2 (qubit): channel-level simulation ==")
            psi_ideal = np.asarray(Simulator(backend_type="statevector").simulate_statevector(
                build_circuit(sched, "skip").originir))
            s_qram = ref.get("survival", sum(ref["avg_dist"].values()))
            joint = sched.get("damping_semantics") == "joint_auxiliary_whole_tree_v1"
            modes = (["full"] if joint else
                     (["full", "nojump", "faithful"] if has_damp
                      else ["depol_textbook", "depol_faithful"]))
            rows = {}
            for mode in modes:
                rho, dist = run_stage2_qubit(sched, order, mode)
                trace = float(np.real(np.trace(rho)))
                qram_dist = ref["avg_dist"]
                row = {
                    "F_classical_norm": classical_fidelity(
                        {k: v / s_qram for k, v in qram_dist.items()},
                        {k: v / trace for k, v in dist.items()} if trace > 0 else dist),
                    "F_classical_unnorm": classical_fidelity(qram_dist, dist),
                    "TVD": tvd({k: v / s_qram for k, v in qram_dist.items()},
                               {k: v / trace for k, v in dist.items()} if trace > 0 else dist),
                    "trace_rho": trace,
                    "qram_survival": s_qram,
                    "uniqc_dist": dist,
                    "F_quantum_vs_ideal": float(np.real(np.conj(psi_ideal) @ rho @ psi_ideal)),
                }
                rows[mode] = row
                print(f"  [{mode:15s}] F_cls(norm)={row['F_classical_norm']:.6f}  "
                      f"F_cls(unnorm)={row['F_classical_unnorm']:.6f}  TVD={row['TVD']:.6f}  "
                      f"trace(ρ)={trace:.6f}  QRAM survival={s_qram:.6f}  "
                      f"F_quantum={row['F_quantum_vs_ideal']:.6f}")
            fid_row = {
                "F_quantum_reference": (rows.get("full", {}) if joint else rows.get("faithful", rows.get("depol_faithful", {})))
                    .get("F_quantum_vs_ideal"),
                "qram_avg_overlap_fid": ref.get("avg_overlap_fid"),
                "qram_avg_fid_nopost": ref.get("avg_fid_nopost"),
                "qram_avg_fid_incoh": ref.get("avg_fid_incoh"),
                "qram_avg_fidelity": ref.get("avg_fidelity"),
                "trace_reference": (rows.get("full", {}) if joint else rows.get("faithful", rows.get("depol_faithful", {})))
                    .get("trace_rho"),
                "trace_nojump": rows.get("nojump", {}).get("trace_rho"),
                "qram_survival": s_qram,
            }
            summary["stage2"] = rows
            summary["fidelity_metrics"] = fid_row
            print(f"  -- F_quantum(reference) = {fid_row['F_quantum_reference']}   "
                  f"QRAM avg_overlap_fid = {fid_row['qram_avg_overlap_fid']}   "
                  f"trace(reference) = {fid_row['trace_reference']}   "
                  f"QRAM survival = {fid_row['qram_survival']}")
        elif has_damp:
            print("\n== Stage 2-d: channel-level damping simulation (full = textbook 3-level AD channel; nojump = K0 only) ==")
            psi_ideal = np.asarray(Simulator(backend_type="statevector").simulate_statevector(
                build_circuit(sched, "skip").originir))
            s_qram = ref.get("survival", sum(ref["avg_dist"].values()))
            rows = {}
            for mode in ("full", "nojump", "faithful"):
                rho, dist = run_stage2_damping(sched, order, mode)
                trace = float(np.real(np.trace(rho)))
                qram_dist = ref["avg_dist"]
                row = {
                    "F_classical_norm": classical_fidelity(
                        {k: v / s_qram for k, v in qram_dist.items()},
                        {k: v / trace for k, v in dist.items()} if trace > 0 else dist),
                    "F_classical_unnorm": classical_fidelity(qram_dist, dist),
                    "TVD": tvd({k: v / s_qram for k, v in qram_dist.items()},
                               {k: v / trace for k, v in dist.items()} if trace > 0 else dist),
                    "trace_rho": trace,
                    "qram_survival": s_qram,
                    "uniqc_dist": dist,
                    "F_quantum_vs_ideal": float(np.real(np.conj(psi_ideal) @ rho @ psi_ideal)),
                }
                rows[mode] = row
                print(f"  [{mode:6s}] F_cls(norm)={row['F_classical_norm']:.6f}  "
                      f"F_cls(unnorm)={row['F_classical_unnorm']:.6f}  TVD={row['TVD']:.6f}  "
                      f"trace(ρ)={trace:.6f}  QRAM survival={s_qram:.6f}  "
                      f"F_quantum={row['F_quantum_vs_ideal']:.6f}")
            fid_row = {
                "F_quantum_full": rows["full"]["F_quantum_vs_ideal"],
                "qram_avg_overlap_fid": ref.get("avg_overlap_fid"),
                "qram_avg_fid_nopost": ref.get("avg_fid_nopost"),
                "qram_avg_fid_incoh": ref.get("avg_fid_incoh"),
                "qram_avg_fidelity": ref.get("avg_fidelity"),
                "trace_nojump": rows["nojump"]["trace_rho"],
                "qram_survival": s_qram,
            }
            summary["stage2d"] = rows
            summary["fidelity_metrics_damp"] = fid_row
            print("  -- survival and fidelity conventions (nojump.trace <-> QRAM survival; "
                  "F_quantum(full) <-> avg_overlap_fid) --")
            print(f"  uniqc trace(nojump) = {fid_row['trace_nojump']:.6f}   "
                  f"QRAM survival = {fid_row['qram_survival']:.6f}")
            print(f"  uniqc F_quantum(full) = {fid_row['F_quantum_full']:.6f}   "
                  f"QRAM avg_overlap_fid = {fid_row['qram_avg_overlap_fid']:.6f}   "
                  f"nopost = {fid_row['qram_avg_fid_nopost']:.6f}   "
                  f"incoh = {fid_row['qram_avg_fid_incoh']:.6f}   "
                  f"post = {fid_row['qram_avg_fidelity']:.6f}")
        else:
            print("\n== Stage 2: channel-level simulation (QuTiP density-matrix backend) ==")
            noise_ops_b = [(st["step"], o) for st in sched["steps"] for o in st["ops"]
                           if o["kind"] == "noise"]
            print(f"  number of extracted noise positions (mode b): {len(noise_ops_b)}")
            psi_ideal = np.asarray(Simulator(backend_type="statevector").simulate_statevector(
                build_circuit(sched, "skip").originir))

            oi_dist = run_stage2_originir(sched, order, d)
            if oi_dist is not None:
                _, direct_dist = run_stage2(sched, "b", order)
                diff = max(abs(oi_dist.get(k, 0.0) - direct_dist.get(k, 0.0))
                           for k in set(oi_dist) | set(direct_dist))
                print(f"  [originir text path] vs kraus direct-drive mode b: max|ΔP| = {diff:.3e}"
                      f" -> {'consistent' if diff < 1e-9 else 'inconsistent'}")

            rows = {}
            for mode in ("b", "c"):
                rho, dist = run_stage2(sched, mode, order)
                trace = float(np.real(np.trace(rho)))
                f_quantum = float(np.real(np.conj(psi_ideal) @ rho @ psi_ideal))
                for ref_name, ref_key in (("model", "model_avg_dist"), ("native", "avg_dist")):
                    qram_dist = ref[ref_key]
                    s_qram = sum(qram_dist.values())
                    f_unnorm = classical_fidelity(qram_dist, dist)
                    f_norm = classical_fidelity(
                        {k: v / s_qram for k, v in qram_dist.items()},
                        {k: v / trace for k, v in dist.items()})
                    row = {
                        "F_classical_unnorm": f_unnorm,
                        "F_classical_norm": f_norm,
                        "TVD": tvd({k: v / s_qram for k, v in qram_dist.items()},
                                   {k: v / trace for k, v in dist.items()}),
                        "trace_rho": trace,
                        "qram_survival": s_qram,
                        "uniqc_dist": dist,
                        "F_quantum_vs_ideal": f_quantum,
                        "qram_avg_fidelity": ref.get(
                            "model_avg_fidelity" if ref_name == "model" else "avg_fidelity"),
                    }
                    rows[f"{mode}:{ref_name}"] = row
                    print(f"  [mode {mode} vs {ref_name}] F_unnorm={f_unnorm:.6f}  "
                          f"F_norm={f_norm:.6f}  TVD={row['TVD']:.6f}  trace(ρ)={trace:.6f}  "
                          f"QRAM survival={s_qram:.6f}  F_quantum={f_quantum:.6f}")

            # Fidelity-convention comparison: F_quantum (= <ψ_ideal|ρ|ψ_ideal> = E|<ψ_ideal|ψ_traj>|²)
            # against the four QRAM-side statistics -- separating the three sources:
            # post-selection / tree-sensitive counting / branch-coherent combination
            fid_row = {
                "F_quantum_uniqc_mode_b": rows["b:native"]["F_quantum_vs_ideal"],
                "F_quantum_uniqc_mode_c": rows["c:native"]["F_quantum_vs_ideal"],
                "qram_avg_overlap_fid": ref.get("avg_overlap_fid"),
                "qram_avg_fid_nopost": ref.get("avg_fid_nopost"),
                "qram_avg_fid_incoh": ref.get("avg_fid_incoh"),
                "qram_avg_fidelity": ref.get("avg_fidelity"),
            }
            summary["stage2"] = rows
            summary["fidelity_metrics"] = fid_row
            print("  -- fidelity convention comparison (the pair that should agree statistically is F_quantum(mode c) vs avg_overlap_fid) --")
            print(f"  uniqc F_quantum(mode c)      = {fid_row['F_quantum_uniqc_mode_c']:.6f}")
            print(f"  QRAM avg_overlap_fid(no post-selection, tree-sensitive) = {fid_row['qram_avg_overlap_fid']:.6f}")
            print(f"  QRAM avg_fid_nopost(no post-selection, tree-insensitive) = {fid_row['qram_avg_fid_nopost']:.6f}")
            print(f"  QRAM avg_fid_incoh(branch projection combination)       = {fid_row['qram_avg_fid_incoh']:.6f}")
            print(f"  QRAM avg_fidelity(per-shot tree post-selection)         = {fid_row['qram_avg_fidelity']:.6f}")

    (d / "driver_summary.json").write_text(json.dumps(summary, indent=1, ensure_ascii=False))
    print(f"\nresults written to {d}/driver_summary.json")
    if not ok:
        print("FAILED: Stage 0/1 exact match did not pass")
        sys.exit(1)
    print("PASSED")


if __name__ == "__main__":
    main()
