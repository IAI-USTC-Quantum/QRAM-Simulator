"""Semi-quantitative comparison: circuit-level baseline vs QRAM-Simulator.

两套模拟器跑同一个 qubit 架构 QRAM 装载任务，对比结果：

- **QRAM-Simulator 本体**（C++ 稀疏轨迹引擎，Python 绑定驱动）：每次
  ``run_full`` 采样一条噪声轨迹，重复 R 次取平均（结果带蒙特卡洛误差）；
- **uniqc 线路级模拟**（``CircuitQRAMQubit``，UnifiedQuantum 的 QuTiP 密度
  矩阵后端）：同一装载线路逐门展开，噪声作为信道精确演化（精确值）。

对比口径（两侧定义完全相同）：

- **保真度** = |⟨ψ_理想|ψ_实际⟩|²（QRAM 侧为 500 条轨迹平均，uniqc 侧为
  密度矩阵的精确系综值 ⟨ψ_理想|ρ|ψ_理想⟩）——理想态指数据正确写回 bus 且
  树回到基态的态；
- **分布 F_cls / TVD** = 两侧 (地址,bus) 输出分布的经典保真度 (Σ√pq)² 与
  总变差距离（各自按自身总概率归一化后比较）；
- **survival / trace** = 阻尼噪声下的存活概率（QRAM 侧轨迹范数平方之和，
  uniqc 侧 trace(ρ)）。

无噪 bridge：两侧无噪输出分布逐位一致（baseline 的门序列直接取自 C++
TimeStep 调度器）。

Usage::

    python -m qram_simulator.baseline.compare [--addr 2] [--runs 500] \
        [--seed 20260925] [--outdir results]

Results are printed as a table and written to ``<outdir>/results.json`` and
``<outdir>/results.csv``.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import time
from pathlib import Path
from typing import Dict, List, Optional

import qram_simulator
from qram_simulator import OperationType, QRAMCircuitQubit, set_seed

from .circuit_qram import CircuitQRAMQubit
from .metrics import classical_fidelity, dist_total, mean_of_dists, normalize, tvd

# default sweep: depolarizing / damping / mixed (mirrors the sweep of the
# ChannelCorrespondence experiment, where these values were validated)
DEFAULT_SWEEP: List[Dict[str, float]] = (
    [{"depol": p} for p in (0.005, 0.02, 0.05, 0.1, 0.3)]
    + [{"damp": g} for g in (0.001, 0.01, 0.05, 0.2)]
    + [{"depol": p, "damp": g}
       for p, g in ((0.005, 0.005), (0.02, 0.02), (0.05, 0.02),
                    (0.02, 0.05), (0.05, 0.05), (0.1, 0.05))]
)


def _noise_models(config: Dict[str, float]) -> Dict[OperationType, float]:
    noise: Dict[OperationType, float] = {}
    if config.get("depol", 0.0) > 0.0:
        noise[OperationType.Depolarizing] = config["depol"]
    if config.get("damp", 0.0) > 0.0:
        noise[OperationType.Damping] = config["damp"]
    return noise


def run_qram_side(addr_size: int, data_size: int, memory, config: Dict[str, float],
                  runs: int, seed: int) -> Dict[str, object]:
    """R independent run_full trajectories of QRAMCircuitQubit, zerobus input.

    Mirrors the reference loop of QubitCorrespondenceExporter: per trajectory
    the distribution and the three no-post-selection fidelity conventions are
    read off the evolved branch groups, and only afterwards the per-shot
    post-selected fidelity is sampled (it mutates the groups).
    """
    set_seed(seed)
    qram = QRAMCircuitQubit(addr_size, data_size, memory)
    qram.set_noise_models(_noise_models(config))
    qram.set_input_zerobus()

    dists: List[Dict[str, float]] = []
    overlap, nopost, incoh = [], [], []
    post: List[float] = []
    sample_failures = 0
    for _ in range(runs):
        qram.run_full()
        dists.append(qram.get_output_distribution())
        conv = qram.get_fidelity_conventions()
        overlap.append(conv["overlap_fid"])
        nopost.append(conv["fid_nopost"])
        incoh.append(conv["fid_incoh"])
        try:
            post.append(qram.sample_and_get_fidelity())
        except RuntimeError:
            sample_failures += 1  # all-zero-norm trajectory (damping extreme)

    avg_dist = mean_of_dists(dists)
    survival = dist_total(avg_dist)

    # noise-free reference of the same input (deterministic)
    qram0 = QRAMCircuitQubit(addr_size, data_size, memory)
    qram0.set_input_zerobus()
    qram0.run_full()
    noisefree = qram0.get_output_distribution()

    def _mean_err(values: List[float]) -> Dict[str, float]:
        n = len(values)
        mean = sum(values) / n
        err = (sum((v - mean) ** 2 for v in values) / max(n - 1, 1)) ** 0.5 / math.sqrt(n) if n > 1 else 0.0
        return {"mean": mean, "mc_error": err}

    return {
        "avg_dist": avg_dist,
        "survival": survival,
        "noisefree_dist": noisefree,
        "avg_overlap_fid": _mean_err(overlap),
        "avg_fid_nopost": _mean_err(nopost),
        "avg_fid_incoh": _mean_err(incoh),
        "avg_fidelity_post": _mean_err(post) if post else {"mean": None, "mc_error": None},
        "sample_failures": sample_failures,
        "runs": runs,
    }


def run_circuit_side(addr_size: int, data_size: int, memory, config: Dict[str, float],
                     mode: str) -> Dict[str, object]:
    baseline = CircuitQRAMQubit(
        addr_size, data_size, memory,
        depolarizing=config.get("depol", 0.0),
        damping=config.get("damp", 0.0),
    )
    rho_ideal = baseline.ideal_density_matrix()
    rho = baseline.simulate_density(mode)
    return {
        "ideal_dist": baseline.marginal_distribution(rho_ideal),
        "dist": baseline.marginal_distribution(rho),
        "trace": float(rho.trace().real),
        "fidelity": baseline.fidelity_with_ideal(rho, rho_ideal),
        "mode": mode,
    }


def compare_one(config: Dict[str, float], addr_size: int, data_size: int, memory,
                runs: int, seed: int, modes: List[str],
                bridge: Optional[Dict[str, object]] = None) -> Dict[str, object]:
    """One noise configuration: both sides + all convention-identical metrics."""
    qram = run_qram_side(addr_size, data_size, memory, config, runs, seed)

    # noise-free bridge of both sides (identical for every configuration --
    # computed once by the caller and reused)
    if bridge is None:
        bridge = run_circuit_side(addr_size, data_size, memory,
                                  {"depol": 0.0, "damp": 0.0}, "faithful")
    bridge_max_dp = max(
        abs(bridge["ideal_dist"].get(k, 0.0) - qram["noisefree_dist"].get(k, 0.0))
        for k in set(bridge["ideal_dist"]) | set(qram["noisefree_dist"])
    )

    channels = {}
    for mode in modes:
        circ = run_circuit_side(addr_size, data_size, memory, config, mode)
        qram_dist_norm = normalize(qram["avg_dist"])
        circ_dist_norm = normalize(circ["dist"]) if circ["trace"] > 0 else circ["dist"]
        channels[mode] = {
            "F_cls_norm": classical_fidelity(qram_dist_norm, circ_dist_norm),
            "TVD_norm": tvd(qram_dist_norm, circ_dist_norm),
            "F_cls_unnorm": classical_fidelity(qram["avg_dist"], circ["dist"]),
            "trace": circ["trace"],
            "qram_survival": qram["survival"],
            "F_quantum": circ["fidelity"],
            "uniqc_dist": circ["dist"],
        }

    primary = channels[modes[0]]
    ov = qram["avg_overlap_fid"]
    return {
        "config": config,
        "addr_size": addr_size,
        "data_size": data_size,
        "memory": list(memory),
        "seed": seed,
        "bridge_max_dp_noisefree": bridge_max_dp,
        "qram": qram,
        "channels": channels,
        "primary": primary,
        "fidelity_gap": (primary["F_quantum"] - ov["mean"]) if ov["mean"] is not None else None,
        "fidelity_gap_in_mc_error": (
            abs(primary["F_quantum"] - ov["mean"]) <= 3.0 * ov["mc_error"]
            if ov["mean"] is not None else None
        ),
    }


def _fmt(x: Optional[float], digits: int = 4) -> str:
    if x is None:
        return "-"
    return f"{x:.{digits}f}"


def print_table(results: List[Dict[str, object]]) -> None:
    # the two fidelity columns adjacent, both under the same convention:
    # QRAM F = trajectory average (with Monte-Carlo error), uniqc F = exact
    header = (
        f"{'config':<20}{'QRAM F':>9}{'+-mc':>8}{'uniqc F':>9}{'gap':>8}{'ok':>4}"
        f"{'F_cls':>8}{'TVD':>8}{'trace':>8}{'surv':>8}"
    )
    print(header)
    print("-" * len(header))
    for row in results:
        cfg = row["config"]
        name = "+".join(f"{k}={v:g}" for k, v in cfg.items()) or "noise-free"
        p = row["primary"]
        ov = row["qram"]["avg_overlap_fid"]
        gap = row["fidelity_gap"]
        ok = row["fidelity_gap_in_mc_error"]
        print(
            f"{name:<20}{_fmt(ov['mean']):>9}{_fmt(ov['mc_error']):>8}"
            f"{_fmt(p['F_quantum']):>9}{_fmt(abs(gap) if gap is not None else None):>8}"
            f"{('Y' if ok else 'N') if ok is not None else '-':>4}"
            f"{_fmt(p['F_cls_norm']):>8}{_fmt(p['TVD_norm']):>8}"
            f"{_fmt(p['trace']):>8}{_fmt(p['qram_survival']):>8}"
        )
    bridge = max(r["bridge_max_dp_noisefree"] for r in results)
    print(f"\nnoise-free bridge: max|dP| = {bridge:.3e} (QRAM noise-free vs uniqc ideal, bitwise)")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--addr", type=int, default=2, choices=(1, 2),
                        help="tree depth / address width (default 2; 1 is the noise-free bridge only)")
    parser.add_argument("--data", type=int, default=1, choices=(1, 2),
                        help="data width per memory cell (default 1)")
    parser.add_argument("--memory", default=None,
                        help="comma-separated memory cells (default 0,1,1,0 for addr=2)")
    parser.add_argument("--runs", type=int, default=500,
                        help="number of QRAM-Simulator noise trajectories per configuration")
    parser.add_argument("--seed", type=int, default=20260925)
    parser.add_argument("--modes", nargs="+", default=["faithful"],
                        choices=("faithful", "textbook", "nojump"),
                        help="circuit-side channel variants to evaluate in addition to "
                             "the primary one (advanced: 'textbook' swaps in the textbook "
                             "TP channels, which deviate systematically under damping; "
                             "'nojump' keeps only the damping no-jump branch)")
    parser.add_argument("--outdir", default="qram-baseline-results",
                        help="output directory for results.json / results.csv")
    parser.add_argument("--quick", action="store_true",
                        help="reduced sweep (one depol + one damp + mixed), 100 runs")
    args = parser.parse_args(argv)

    if args.memory is not None:
        memory = [int(x) for x in args.memory.split(",")]
    else:
        memory = [0, 1, 1, 0][: 2 ** args.addr]
        memory = (memory * (2 ** args.addr))[: 2 ** args.addr]
    if len(memory) != 2 ** args.addr:
        parser.error(f"memory needs 2**addr = {2**args.addr} cells")

    sweep = DEFAULT_SWEEP
    runs = args.runs
    if args.quick:
        sweep = [{"depol": 0.05}, {"damp": 0.05}, {"depol": 0.02, "damp": 0.02}]
        runs = min(runs, 100)

    print(f"qram-simulator {qram_simulator.__version__} | addr={args.addr} data={args.data} "
          f"memory={memory} runs={runs} seed={args.seed}")
    if args.addr == 1:
        print("note: addr=1 has no position-sampled noise in the QRAM-Simulator model "
              "(layer_entangle_max == 0; under Damping only the whole-tree Damp_Common acts)")

    results = []
    bridge = run_circuit_side(args.addr, args.data, memory,
                              {"depol": 0.0, "damp": 0.0}, "faithful")
    for config in sweep:
        t0 = time.time()
        row = compare_one(config, args.addr, args.data, memory, runs, args.seed,
                          args.modes, bridge=bridge)
        results.append(row)
        name = "+".join(f"{k}={v:g}" for k, v in config.items())
        print(f"[{time.time() - t0:6.1f}s] {name}: QRAM F={row['qram']['avg_overlap_fid']['mean']:.4f} "
              f"vs uniqc F={row['primary']['F_quantum']:.4f}  "
              f"F_cls={row['primary']['F_cls_norm']:.4f} TVD={row['primary']['TVD_norm']:.4f}")

    print()
    print_table(results)

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    payload = {
        "package_version": qram_simulator.__version__,
        "addr_size": args.addr,
        "data_size": args.data,
        "memory": memory,
        "runs": runs,
        "seed": args.seed,
        "modes": args.modes,
        "results": [
            {
                "config": r["config"],
                "bridge_max_dp_noisefree": r["bridge_max_dp_noisefree"],
                "qram": {
                    "avg_dist": r["qram"]["avg_dist"],
                    "survival": r["qram"]["survival"],
                    "avg_overlap_fid": r["qram"]["avg_overlap_fid"],
                    "avg_fid_nopost": r["qram"]["avg_fid_nopost"],
                    "avg_fid_incoh": r["qram"]["avg_fid_incoh"],
                    "avg_fidelity_post": r["qram"]["avg_fidelity_post"],
                    "sample_failures": r["qram"]["sample_failures"],
                },
                "channels": {
                    mode: {k: v for k, v in ch.items() if k != "uniqc_dist"}
                    for mode, ch in r["channels"].items()
                },
                "uniqc_dist": {mode: r["channels"][mode]["uniqc_dist"] for mode in r["channels"]},
                "fidelity_gap": r["fidelity_gap"],
                "fidelity_gap_in_mc_error": r["fidelity_gap_in_mc_error"],
            }
            for r in results
        ],
    }
    (outdir / "results.json").write_text(json.dumps(payload, indent=1), encoding="utf-8")

    with (outdir / "results.csv").open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow([
            "config", "mode", "F_cls_norm", "TVD_norm", "F_cls_unnorm",
            "trace", "qram_survival", "F_quantum", "avg_overlap_fid",
            "overlap_mc_error", "avg_fid_nopost", "avg_fid_incoh",
            "avg_fidelity_post", "bridge_max_dp",
        ])
        for r in results:
            for mode, ch in r["channels"].items():
                q = r["qram"]
                writer.writerow([
                    "+".join(f"{k}={v:g}" for k, v in r["config"].items()),
                    mode, ch["F_cls_norm"], ch["TVD_norm"], ch["F_cls_unnorm"],
                    ch["trace"], q["survival"], ch["F_quantum"],
                    q["avg_overlap_fid"]["mean"], q["avg_overlap_fid"]["mc_error"],
                    q["avg_fid_nopost"]["mean"], q["avg_fid_incoh"]["mean"],
                    q["avg_fidelity_post"]["mean"], r["bridge_max_dp_noisefree"],
                ])

    print(f"\nresults written to {outdir / 'results.json'} and {outdir / 'results.csv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
