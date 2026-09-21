#!/usr/bin/env python3
"""qutrit QRAM（QRAM-Simulator）↔ 电路级 channel 模拟（uniqc）对应实验驱动。

消费 QutritCorrespondenceExporter 导出的 schedule.json / reference.json：

  Stage 0  无噪声桥正确性：编码门电路在 uniqc statevector 后端的输出分布
           vs QRAM-Simulator 无噪声 run 的精确分布 —— 必须逐位相等。
  Stage 1  固定噪声实现化：提取的噪声算子（type+coef → 确定性 unitary，
           镜像 SubBranch::run_depolarizing 等的 floor 选择规则）作为门插入
           —— vs 同一 seed 调度的 run_full 精确分布，必须相等。
  Stage 2  channel 级（正题）：噪声位置上放置信道
             mode b = 提取出的位置（该次实现化中噪声出现的地方）；
             mode c = 每步全活跃位置（与轨迹平均统计对应的口径）。
           数据位用 Pauli 信道（与 OriginIR-ext 文本信道语义一致），
           qutrit 位用 8 元 Weyl 混合的 Kraus（超出 OriginIR-ext 文本信道集，
           经同一 QuTiP 密度矩阵后端的 kraus2q 原语注入）。
           对比口径：renormalize 与否 × {经典分布 fidelity, TVD, 量子 fidelity}。

用法（在含 uniqc 的解释器下，例如 UnifiedQuantum/.venv/bin/python）：
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
# JSON 门 IR → uniqc.Circuit（Stage 0/1 用）
# --------------------------------------------------------------------------

class _NullCtx:
    def __enter__(self):
        return None

    def __exit__(self, *args):
        return False


def gate(c: Circuit, op: str, qubits, ctrls=(), theta=None):
    """应用一个带（正/负极性）控制集的门。负控制用 X 包裹成正常控制。"""
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
    """qs[k] ↔ 索引 bit k。基矢置换 (pi ↔ pj)，整体受 extra_ctrls 控制。
    与 C++ 侧 emit_transposition 相同的共轭算法。"""
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


# addr-qutrit 能级（4×4 索引 = 2*bit(a1)+bit(a0)）：W=0, L=1, R=2, dead=3
QUTRIT_A1 = {0: 2, 2: 1, 1: 0}     # rotate_A1: L→W, W→R, R→L  (旧→新)
QUTRIT_A1_2 = {0: 1, 1: 2, 2: 0}   # rotate_A2(置换): W→L, L→R, R→W


def emit_qutrit_perm(c: Circuit, a1: int, a0: int, mapping):
    """3-循环分解为 2 个对换，对换用共轭法在 (a1,a0) 上发射。bit0=a0, bit1=a1。"""
    qs = [a0, a1]
    m = dict(mapping)
    m.setdefault(3, 3)
    # 把映射分解成对换（应用顺序 = 发射顺序：升序 (cycle[0], cycle[i]) 合成出原映射）
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
    """diag：φ(L)=ω^s、φ(R)=ω^{2s}、φ(W)=1（SubBranch::run_A2 的相位语义）。"""
    gate(c, "U1", [a0], [(a1, 0)], theta=TWO_PI_3 * s)
    gate(c, "U1", [a1], [(a0, 0)], theta=2.0 * TWO_PI_3 * s)


def emit_swap_then_phase_r(c: Circuit, a1: int, a0: int):
    """BitPhaseFlip（addr）：addr_flip 后对 R 能级加 -1 相位。（已随 Depolarizing-only 收编，
    保留作为 QUTRIT Weyl 族分解的组成原语备用。）"""
    c.swap(a1, a0)
    gate(c, "U1", [a1], [(a0, 0)], theta=math.pi)


def apply_noise_unitary(c: Circuit, o: dict, enc: dict):
    """Stage 1：把提取的 Depolarizing 算子按 QRAM-Simulator 的 floor 规则落成
    确定性 unitary（抽样完成后已形成具体的 X/Z/Y 或 qutrit Weyl 算子）。"""
    node = enc["nodes"][o["node"]]
    coef = o["coef"]
    if o["type"] != "Depolarizing":
        raise ValueError(f"仅支持 Depolarizing（收到 {o['type']}；Damping 为 M3）")
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
    """noise: 'skip'（Stage 0）| 'unitary'（Stage 1）"""
    enc = sched["encoding"]
    c = Circuit()
    # 触碰全部 qubit（X·X 恒等）：uniqc 对未触碰 qubit 会做压缩重排，
    # 显式注册后索引映射才是 qubit k ↔ 基矢 bit k
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
# 分布提取与比较
# --------------------------------------------------------------------------

def detect_index_bit_order() -> str:
    """探测 uniqc statevector 的基矢索引 bit 约定：qubit 0 是 LSB 还是 MSB。"""
    c = Circuit()
    c.x(0)
    c.h(1)  # 保证电路有 2 个 qubit
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
# Stage 1-d / Stage 2-d：Damping 路径（单 Kraus 轨迹回放 + 信道级 full/nojump）
# --------------------------------------------------------------------------

def be_transposition(be, qs, pi: int, pj: int, extra_ctrls=()):
    """emit_transposition 的后端版（密度矩阵上直接施加）。"""
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
    """Depolarizing 的确定性酉（后端版，混合配置的 Stage 1-d 回放用）。"""
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
    """Damp_Full 固定结果的跳变（单 Kraus，无 √γ 因子——权重由位置抽样概率承载）。"""
    node = enc["nodes"][o["node"]]
    if o["sub"] == "data":
        K = np.array([[0.0, 1.0], [0.0, 0.0]], dtype=complex)  # |0><1|
        be.kraus1q(node["d"], [K.reshape(-1).tolist()])
    else:
        K = np.zeros((4, 4), dtype=complex)
        K[0, 1 if o["outcome"] == 0 else 2] = 1.0  # |W><L| 或 |W><R|
        be.kraus2q(node["a1"], node["a0"], [K.reshape(-1).tolist()])


def apply_k0_kraus(be, enc: dict, gamma: float, jumps: bool):
    """Damp_Common 的编码侧对应：作用于全部节点自由度。
    jumps=False → 仅 K0（nojump 模式）；jumps=True → 教科书 3 能级 AD 完整信道。"""
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
    """从当前密度矩阵对角线读各自由度的激发权重（次归一化轨迹系的 faithful 镜像用）。"""
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
    """忠实镜像：逐步复现他们采样方案的平均效果（ρ 依赖的非线性映射）。
    跳变只可能出现在被抽中的活跃位置（节点 < 2^L−1）：K0 分支权重 1−γw/S、
    跳变分支 √(γ·w_k/S)·M_k（轨迹不归一化，贡献 = (γw_k/S)·M_kρM_k†）；
    非活跃节点只承受 Damp_Common 的纯 K0。
    非线性映射在平均 ρ 上取权重，与逐轨迹平均有一阶以上的差异（README 说明）。"""
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
    """Stage 1-d：按导出的实现化（含 Damp_Full 的 outcome）逐算子回放。
    门 = 酉；Depolarizing = 确定性酉；Damp_Common = K0 单 Kraus（全树）；
    Damp_Full = 固定结果跳变单 Kraus（nojump 跳过）。返回次归一化 ρ 与边缘分布。"""
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
                    raise ValueError("Damp_Full 缺 outcome（需由导出器复刻执行记录）")
                if o.get("outcome", -1) >= 0:
                    apply_jump_kraus(be, o, enc)
            elif o["type"] == "Damp_Common":
                apply_k0_kraus(be, enc, o["coef"], jumps=False)
            else:
                raise ValueError(o["type"])
    rho = np.array(be.density_matrix.full())
    return rho, marginal_from_rho(rho, sched, order)


def run_stage2_damping(sched: dict, order: str, damp_mode: str, depol_mode: str = "c"):
    """Stage 2-d：damp_mode ∈ {full, nojump, faithful}。
    full    = 每步（Damp_Common 出现处）对全部节点自由度施加教科书 3 能级 AD 信道（TP）；
    nojump  = 仅 K0 单 Kraus（非 TP，trace = 无跳变生存概率）；
    faithful= 逐幅度复现其采样方案平均效果的 ρ 依赖映射（见 apply_faithful_kraus）。
    Depolarizing（混合配置）按 depol_mode b/c 位置策略施加信道。"""
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
# qubit 架构：噪声原语 / 逐 case 回放 / 信道级
# --------------------------------------------------------------------------

# run_bitphaseflip（库已修复为 Y flip）：|0>→−|1>、|1>→|0>（ZX = iY，酉）。
# 旧实现为 |0>→|0>、|1>→−|0>（秩 1 非酉）——重复态相干抵消曾致零范数轨迹与
# sample_output 崩溃；修复后 qubit 架构 Depolarizing 恢复为保范数轨迹。
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
            raise ValueError("Damp_Full 缺 outcome（需由导出器复刻执行记录）")
        if o.get("outcome", -1) >= 0:
            K = np.array([[0.0, 1.0], [0.0, 0.0]], dtype=complex)  # |0><1|
            be.kraus1q(q, [K.reshape(-1).tolist()])
    else:
        raise ValueError(t)


def replay_trajectory_qubit(sched: dict, order: str):
    """Stage 1（qubit）：逐算子回放。Depolarizing k=2（bitphaseflip）以单 Kraus M
    实现（物理相干语义）；Damp 算子同 qutrit 结构（全为 1q）。"""
    enc = sched["encoding"]
    be = init_backend(sched)
    for st in sched["steps"]:
        for o in st["ops"]:
            if o["kind"] == "gate":
                be_gate(be, o["op"], o["q"], [tuple(x) for x in o.get("ctrl", [])],
                        theta=o.get("theta"))
            else:
                apply_qubit_noise_op_be(be, o, enc)
    rho = np.array(be.density_matrix.full())
    return rho, marginal_from_rho(rho, sched, order)


def run_stage2_qubit(sched: dict, order: str, mode: str):
    """Stage 2（qubit）。mode ∈ {depol_textbook, depol_faithful, full, nojump, faithful}：
    depol_*  = 活跃位置 [0, 2(2^L−1)) × 边缘概率 p；textbook = TP Pauli 去极化；
               faithful = {I, X, Z, M} 混合（M 非 TP → 轨迹系综物理上次归一化）；
    damping* = Damp_Common 出现处对全树每 qubit 施加；full = 教科书 AD（TP）、
               nojump = 仅 K0、faithful = ρ 依赖跳变混合（仅活跃节点）。"""
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
# Stage 2：QuTiP 密度矩阵后端直接驱动（信道级）
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
    """qutrit Depolarizing 的 8 元 Weyl unitary 的 4×4 嵌入（死态恒等）。
    索引 = 2·bit(a1)+bit(a0)：W=0, L=1, R=2, dead=3。"""
    k = math.floor(8 * coef)
    perm = {0: 2, 2: 1, 1: 0} if k in (0, 4, 6) else (
        {0: 1, 1: 2, 2: 0} if k in (2, 5, 7) else None)
    s = 2 if k in (3, 6, 7) else 1
    U = np.zeros((4, 4), dtype=complex)
    U[3, 3] = 1.0  # 死态恒等
    for old in range(3):
        new = perm[old] if perm else old
        ph = 1.0
        if k in (1, 4, 5, 6, 7):
            # 相位按 run_A2 语义挂在置换前的能级上：L→ω^s，R→ω^{2s}
            ph = omega(s) if old == 1 else (omega(2 * s) if old == 2 else 1.0)
        U[new, old] = ph
    return U


def apply_channel_at(be, o: dict, enc: dict, noise_cfg: dict, p_scale: float = 1.0):
    """在提取位置上放置信道。p 为 QRAM 噪声模型里 Depolarizing 的逐位概率。"""
    node = enc["nodes"][o["node"]]
    p = noise_cfg[o["type"]] * p_scale
    if o["type"] != "Depolarizing":
        raise ValueError(f"仅支持 Depolarizing（收到 {o['type']}；Damping 为 M3）")
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
            # 修复闭区间采样后：每步 nerror ~ Binomial(n_active, p) 均匀落入
            # [0, n_active) 的 n_active 个活跃位置 → 每位置边缘概率恰为 p；
            # n_active=0（entangle_max=0）的步无噪声。
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
    """mode b 的 OriginIR-ext 文本路径：数据位信道以文本 opcode 内联注入
    （Depolarizing/BitFlip/PhaseFlip/PauliError1Q，与 OriginIR-ext 信道语句同语义）；
    qutrit 位（addr 子系统）的 8 元 Weyl 混合超出文本信道集 → 仅当调度中不存在
    addr 位噪声时该文本才是完整电路，可直接跑标准 Simulator 路径并与直驱结果对拍。"""
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
                    raise ValueError(f"仅支持 Depolarizing（收到 {o['type']}）")
                if o["sub"] == "data":
                    c.add_gate("Depolarizing", enc["nodes"][o["node"]]["d"],
                               params=noise_cfg["Depolarizing"])
                else:
                    n_addr_ops += 1
    text = c.originir
    (outdir / "circuit_channels.originir").write_text(text)
    if n_addr_ops:
        print(f"  [originir] 文本已导出，但含 {n_addr_ops} 个 qutrit(addr) 位噪声"
              f"——超出 OriginIR-ext 文本信道集，完整模拟走 kraus 直驱路径")
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
# 主流程
# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", required=True, help="exporter 输出目录（含 schedule.json / reference.json）")
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
        print(f"FAILED: 调度含未支持噪声类型 {unsupported}")
        sys.exit(1)
    has_damp = bool(sched_noise_types & {"Damp_Full", "Damp_Common"})
    arch = sched.get("arch", "qutrit")
    order = detect_index_bit_order()
    print(f"uniqc statevector 索引约定: qubit0={'LSB' if order == 'lsb' else 'MSB'}")

    summary = {"dir": str(d), "index_order": order}
    ok = True

    if args.stage in ("0", "all"):
        print("\n== Stage 0: 无噪声桥正确性（编码门电路 vs QRAM-Simulator 无噪声 run）==")
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
            print("\n== Stage 1（qubit）: 逐随机 case 回放（门 + Depolarizing 酉/Kraus + K0/跳变单 Kraus）==")
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
            print(f"  逐 case 回放: {n_cases} 个随机实现化，{'全部精确匹配' if case_ok else '存在不匹配'}")
            ok &= case_ok
            summary["stage1"] = {"arch": "qubit", "cases": n_cases, "cases_ok": bool(case_ok)}
        elif has_damp:
            print("\n== Stage 1-d: 逐随机 case 回放（门 + Depolarizing 酉 + K0 单 Kraus + "
                  "固定结果跳变单 Kraus）==")
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
            print(f"  逐 case 回放: {n_cases} 个随机实现化（次归一化口径），"
                  f"{'全部精确匹配' if case_ok else '存在不匹配'}")
            ok &= case_ok
            summary["stage1d"] = {"cases": n_cases, "cases_ok": bool(case_ok)}
        else:
            print("\n== Stage 1: 逐随机 case 精确对拍（抽样完成 → 具体算子 → 确定性 unitary 门）==")
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
            print(f"  逐 case 对拍: {n_cases} 个随机实现化，{'全部精确匹配' if case_ok else '存在不匹配'}")
            ok &= case_ok
            # r=0 汇总行 + 模型交叉验证（库修复后 native 与 model 应逐位一致）
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
            print("\n== Stage 2（qubit）: channel 级模拟 ==")
            psi_ideal = np.asarray(Simulator(backend_type="statevector").simulate_statevector(
                build_circuit(sched, "skip").originir))
            s_qram = ref.get("survival", sum(ref["avg_dist"].values()))
            modes = (["full", "nojump", "faithful"] if has_damp
                     else ["depol_textbook", "depol_faithful"])
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
                print(f"  [{mode:15s}] F_cls(归一)={row['F_classical_norm']:.6f}  "
                      f"F_cls(不归一)={row['F_classical_unnorm']:.6f}  TVD={row['TVD']:.6f}  "
                      f"trace(ρ)={trace:.6f}  QRAM survival={s_qram:.6f}  "
                      f"F_quantum={row['F_quantum_vs_ideal']:.6f}")
            fid_row = {
                "F_quantum_faithful": rows.get("faithful", rows.get("depol_faithful", {}))
                    .get("F_quantum_vs_ideal"),
                "qram_avg_overlap_fid": ref.get("avg_overlap_fid"),
                "qram_avg_fid_nopost": ref.get("avg_fid_nopost"),
                "qram_avg_fid_incoh": ref.get("avg_fid_incoh"),
                "qram_avg_fidelity": ref.get("avg_fidelity"),
                "trace_faithful": rows.get("faithful", rows.get("depol_faithful", {}))
                    .get("trace_rho"),
                "trace_nojump": rows.get("nojump", {}).get("trace_rho"),
                "qram_survival": s_qram,
            }
            summary["stage2"] = rows
            summary["fidelity_metrics"] = fid_row
            print(f"  -- F_quantum(faithful) = {fid_row['F_quantum_faithful']}   "
                  f"QRAM avg_overlap_fid = {fid_row['qram_avg_overlap_fid']}   "
                  f"trace(faithful) = {fid_row['trace_faithful']}   "
                  f"QRAM survival = {fid_row['qram_survival']}")
        elif has_damp:
            print("\n== Stage 2-d: channel 级阻尼模拟（full = 教科书 3 能级 AD 信道；nojump = 仅 K0）==")
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
                print(f"  [{mode:6s}] F_cls(归一)={row['F_classical_norm']:.6f}  "
                      f"F_cls(不归一)={row['F_classical_unnorm']:.6f}  TVD={row['TVD']:.6f}  "
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
            print("  -- 生存率与 fidelity 口径（nojump.trace ↔ QRAM survival；"
                  "F_quantum(full) ↔ avg_overlap_fid）--")
            print(f"  uniqc trace(nojump) = {fid_row['trace_nojump']:.6f}   "
                  f"QRAM survival = {fid_row['qram_survival']:.6f}")
            print(f"  uniqc F_quantum(full) = {fid_row['F_quantum_full']:.6f}   "
                  f"QRAM avg_overlap_fid = {fid_row['qram_avg_overlap_fid']:.6f}   "
                  f"nopost = {fid_row['qram_avg_fid_nopost']:.6f}   "
                  f"incoh = {fid_row['qram_avg_fid_incoh']:.6f}   "
                  f"post = {fid_row['qram_avg_fidelity']:.6f}")
        else:
            print("\n== Stage 2: channel 级模拟（QuTiP 密度矩阵后端）==")
            noise_ops_b = [(st["step"], o) for st in sched["steps"] for o in st["ops"]
                           if o["kind"] == "noise"]
            print(f"  提取噪声位置数（mode b）: {len(noise_ops_b)}")
            psi_ideal = np.asarray(Simulator(backend_type="statevector").simulate_statevector(
                build_circuit(sched, "skip").originir))

            oi_dist = run_stage2_originir(sched, order, d)
            if oi_dist is not None:
                _, direct_dist = run_stage2(sched, "b", order)
                diff = max(abs(oi_dist.get(k, 0.0) - direct_dist.get(k, 0.0))
                           for k in set(oi_dist) | set(direct_dist))
                print(f"  [originir 文本路径] vs kraus 直驱 mode b: max|ΔP| = {diff:.3e}"
                      f" -> {'一致' if diff < 1e-9 else '不一致'}")

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
                          f"QRAM存活={s_qram:.6f}  F_quantum={f_quantum:.6f}")

            # fidelity 口径对比：F_quantum（=⟨ψ_ideal|ρ|ψ_ideal⟩ = E|⟨ψ_ideal|ψ_traj⟩|²）
            # 对 QRAM 侧四个统计量 —— 分离「后选择 / 树敏感计数 / 分支相干组合」三个来源
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
            print("  -- fidelity 口径对比（统计意义上应一致的是 F_quantum(mode c) vs avg_overlap_fid）--")
            print(f"  uniqc F_quantum(mode c)      = {fid_row['F_quantum_uniqc_mode_c']:.6f}")
            print(f"  QRAM avg_overlap_fid(无后选择,树敏感) = {fid_row['qram_avg_overlap_fid']:.6f}")
            print(f"  QRAM avg_fid_nopost(无后选择,树不敏感) = {fid_row['qram_avg_fid_nopost']:.6f}")
            print(f"  QRAM avg_fid_incoh(分支投影组合)       = {fid_row['qram_avg_fid_incoh']:.6f}")
            print(f"  QRAM avg_fidelity(逐shot树后选择版)    = {fid_row['qram_avg_fidelity']:.6f}")

    (d / "driver_summary.json").write_text(json.dumps(summary, indent=1, ensure_ascii=False))
    print(f"\n结果写入 {d}/driver_summary.json")
    if not ok:
        print("FAILED: Stage 0/1 精确匹配未通过")
        sys.exit(1)
    print("PASSED")


if __name__ == "__main__":
    main()
