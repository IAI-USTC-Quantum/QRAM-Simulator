#!/usr/bin/env python3
"""从 QubitPaperScan 输出的 CSV 重生成论文图 fig_fidelity.pdf / fig_perf.pdf。

用法：
    python3 plot_figures.py <results_dir> <out_dir>

输入：
    fidelity_scan.csv  —— fidelity 模式输出（fig 3）
    perf_scan.csv      —— perf 模式输出（fig 4）
"""

import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

COLORS = {0.0: "#7f7f7f", 1e-5: "#1f77b4", 1e-4: "#ff7f0e", 1e-3: "#d62728"}
MARKERS = {0.0: "D", 1e-5: "o", 1e-4: "s", 1e-3: "^"}


def load_csv(path):
    rows = []
    with open(path) as fp:
        for row in csv.DictReader(fp):
            rows.append(
                dict(
                    arch=row["arch"],
                    eps=float(row["eps"]),
                    n=int(row["n"]),
                    version=row["version"],
                    fid=float(row["fid"]),
                    time_ms=float(row["time_run_ms"]) + float(row["time_sample_ms"]),
                    states=int(row["states"]),
                )
            )
    return rows


def aggregate_mean(rows, key_fields, value_field):
    """按 key_fields 分组求 value_field 的均值"""
    acc = defaultdict(lambda: [0.0, 0])
    for r in rows:
        key = tuple(r[f] for f in key_fields)
        acc[key][0] += r[value_field]
        acc[key][1] += 1
    return {k: s / c for k, (s, c) in acc.items()}


def style_axis(ax):
    ax.grid(True, color="#cccccc", linewidth=0.7, alpha=0.6)
    ax.set_axisbelow(True)


def plot_fidelity(rows, out_dir):
    qubit = [r for r in rows if r["arch"] == "qubit" and r["version"] == "full"]
    fid = aggregate_mean(qubit, ["eps", "n"], "fid")

    fig, (ax_a, ax_b) = plt.subplots(2, 1, figsize=(3.4, 4.9))

    # ---- (a) qubit encoding, eps sweep ----
    for eps in (1e-5, 1e-4, 1e-3):
        ns = sorted(n for (e, n) in fid if e == eps)
        ax_a.plot(ns, [fid[(eps, n)] for n in ns], "-o", color=COLORS[eps],
                  markersize=5, label=r"$10^{-%d}$" % round(-math.log10(eps)))
    ax_a.set_title(r"(a) qubit encoding, $\varepsilon=\gamma$ sweep")
    ax_a.set_ylabel("Fidelity (qubit)")
    ax_a.set_ylim(0.0, 1.02)
    ax_a.legend(title=r"$\varepsilon=\gamma$", loc="center left")
    ax_a.tick_params(labelbottom=False)
    style_axis(ax_a)

    # ---- (b) qubit vs qutrit at 1e-5 ----
    qutrit = [r for r in rows if r["arch"] == "qutrit"]
    fid_q = aggregate_mean(qutrit, ["n"], "fid")
    ns_q = sorted(fid_q)
    ns_b = sorted(n for (e, n) in fid if e == 1e-5)
    ax_b.plot(ns_b, [fid[(1e-5, n)] for n in ns_b], "-o", color="#d62728",
              markersize=5, label="qubit")
    ax_b.plot(ns_q, [fid_q[n] for n in ns_q], "--s", color="#1f77b4",
              markersize=5, label="qutrit")
    ax_b.set_title(r"(b) qubit vs. qutrit at $\varepsilon=\gamma=10^{-5}$")
    ax_b.set_xlabel("address size $n$")
    ax_b.set_ylabel("Fidelity")
    ax_b.set_ylim(0.0, 1.02)
    ax_b.legend(loc="center left")
    style_axis(ax_b)

    # inset：1e-5 下两条线的放大图
    lo = min(min(fid[(1e-5, n)] for n in ns_b), min(fid_q[n] for n in ns_q))
    ylo = math.floor((lo - 0.002) * 1000) / 1000
    inset = ax_b.inset_axes([0.52, 0.14, 0.44, 0.38])
    inset.plot(ns_b, [fid[(1e-5, n)] for n in ns_b], "-o", color="#d62728",
               markersize=3, linewidth=1)
    inset.plot(ns_q, [fid_q[n] for n in ns_q], "--s", color="#1f77b4",
               markersize=3, linewidth=1)
    inset.set_ylim(ylo, 1.0005)
    inset.tick_params(labelsize=7)
    inset.grid(True, color="#cccccc", linewidth=0.5, alpha=0.6)
    inset.text(0.06, 0.12, "zoom", transform=inset.transAxes, fontsize=7, style="italic")

    fig.tight_layout()
    fig.savefig(out_dir / "fig_fidelity.pdf")
    fig.savefig(out_dir / "fig_fidelity.png", dpi=200)
    plt.close(fig)


def plot_perf(rows, out_dir):
    per = aggregate_mean(rows, ["eps", "n", "version"], "time_ms")
    states = aggregate_mean(rows, ["eps", "n", "version"], "states")

    ns = sorted({n for (_, n, _) in per})
    fig, (ax_a, ax_b) = plt.subplots(1, 2, figsize=(7.0, 2.9))

    for eps in (0.0, 1e-5, 1e-4, 1e-3):
        color = COLORS[eps]
        marker = MARKERS[eps]
        label = r"$10^{-%d}$" % round(-math.log10(eps)) if eps > 0 else "0"
        ls_full = "-." if eps == 0.0 else "-"
        for ax, data, ylab in ((ax_a, per, "time per query (ms)"),
                               (ax_b, states, "evolved branch states")):
            ax.plot(ns, [data[(eps, n, "full")] for n in ns], ls_full, marker=marker,
                    color=color, markersize=5, label=label)
            ax.plot(ns, [data[(eps, n, "normal")] for n in ns], ":", marker=marker,
                    color=color, markersize=5, markerfacecolor="none")
    ax_a.set_yscale("log")
    ax_a.set_title("(a) runtime")
    ax_a.set_xlabel("address size $n$")
    ax_a.set_ylabel("time per query (ms)")
    ax_b.set_yscale("log")
    ax_b.set_title("(b) explicitly evolved branch states")
    ax_b.set_xlabel("address size $n$")
    ax_b.set_ylabel("evolved branch states")
    for ax in (ax_a, ax_b):
        style_axis(ax)

    handles, labels = ax_a.get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=4,
               title="solid: full   dotted: pruned;   $\\varepsilon=\\gamma$:",
               bbox_to_anchor=(0.5, -0.02))
    fig.tight_layout(rect=(0, 0.15, 0.98, 1))
    fig.savefig(out_dir / "fig_perf.pdf")
    fig.savefig(out_dir / "fig_perf.png", dpi=200)
    plt.close(fig)


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    results = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])
    out_dir.mkdir(parents=True, exist_ok=True)

    plot_fidelity(load_csv(results / "fidelity_scan.csv"), out_dir)
    print("fig_fidelity ->", out_dir / "fig_fidelity.pdf")
    plot_perf(load_csv(results / "perf_scan.csv"), out_dir)
    print("fig_perf     ->", out_dir / "fig_perf.pdf")


if __name__ == "__main__":
    main()
