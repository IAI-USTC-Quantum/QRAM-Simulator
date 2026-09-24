#!/usr/bin/env python3
"""从 QubitPaperScan 输出的 CSV 重生成论文图 fig_fidelity.pdf / fig_perf.pdf。

用法：
    python3 plot_figures.py <results_dir> <out_dir>

输入：
    fidelity_scan.csv  —— fidelity 模式输出（fig 3）
    perf_scan.csv      —— perf 模式输出（fig 4）

风格：PRA（APS）单栏 3.4 in / 双栏 7.0 in，正文字号 8 pt，出图后按原尺寸
插入 LaTeX（\\columnwidth / \\textwidth），图中文字即为最终印刷字号。
"""

import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

# APS/PRA 常规：无标题，(a)/(b) 角标在轴内；Helvetica 类无衬线小字号
matplotlib.rcParams.update({
    "font.size": 8,
    "axes.labelsize": 8,
    "xtick.labelsize": 7,
    "ytick.labelsize": 7,
    "legend.fontsize": 6.5,
    "legend.title_fontsize": 6.5,
    "lines.linewidth": 1.1,
    "lines.markersize": 3.2,
    "axes.linewidth": 0.7,
    "xtick.major.width": 0.7,
    "ytick.major.width": 0.7,
    "xtick.minor.width": 0.5,
    "ytick.minor.width": 0.5,
    "grid.linewidth": 0.4,
    "legend.borderpad": 0.3,
    "legend.handlelength": 1.5,
    "legend.handletextpad": 0.5,
    "legend.columnspacing": 0.9,
    "legend.labelspacing": 0.3,
})

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
                    branches=int(row.get("branches") or 0),
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
    ax.grid(True, color="#cccccc", linewidth=0.4, alpha=0.6)
    ax.set_axisbelow(True)
    ax.tick_params(direction="in", top=True, right=True)


def panel_tag(ax, tag, x=0.03, y=0.95):
    ax.text(x, y, tag, transform=ax.transAxes, fontsize=8,
            ha="left", va="top", fontweight="bold")


def plot_fidelity(rows, out_dir):
    qubit = [r for r in rows if r["arch"] == "qubit" and r["version"] == "full"]
    fid = aggregate_mean(qubit, ["eps", "n"], "fid")

    fig, ax = plt.subplots(figsize=(3.4, 2.5))

    for eps in (1e-5, 1e-4, 1e-3):
        ns = sorted(n for (e, n) in fid if e == eps)
        ax.plot(ns, [fid[(eps, n)] for n in ns], "-o", color=COLORS[eps],
                label=r"$10^{-%d}$" % round(-math.log10(eps)))
    ax.set_xlabel("address size $n$")
    ax.set_ylabel("Fidelity")
    ax.set_ylim(0.0, 1.02)
    ax.set_xlim(2.6, 10.4)
    ax.legend(title=r"$\varepsilon=\gamma$", loc="lower left")
    style_axis(ax)

    fig.tight_layout(pad=0.3)
    fig.savefig(out_dir / "fig_fidelity.pdf")
    fig.savefig(out_dir / "fig_fidelity.png", dpi=300)
    plt.close(fig)


def plot_perf(rows, out_dir):
    per = aggregate_mean(rows, ["eps", "n", "version"], "time_ms")
    branches = aggregate_mean(rows, ["eps", "n", "version"], "branches")

    ns = sorted({n for (_, n, _) in per})
    fig, (ax_a, ax_b) = plt.subplots(1, 2, figsize=(7.0, 2.55))

    for eps in (0.0, 1e-5, 1e-4, 1e-3):
        color = COLORS[eps]
        marker = MARKERS[eps]
        label = r"$10^{-%d}$" % round(-math.log10(eps)) if eps > 0 else "0"
        ls_full = "-." if eps == 0.0 else "-"
        for ax, data, ylab in ((ax_a, per, "time per query (ms)"),
                               (ax_b, branches, "explicitly simulated branches")):
            ax.plot(ns, [data[(eps, n, "full")] for n in ns], ls_full, marker=marker,
                    color=color, label=label)
            ax.plot(ns, [data[(eps, n, "normal")] for n in ns], ":", marker=marker,
                    color=color, markerfacecolor="none")
    ax_a.set_yscale("log")
    ax_a.set_xlabel("address size $n$")
    ax_a.set_ylabel("time per query (ms)")
    panel_tag(ax_a, "(a)")
    ax_b.set_yscale("log")
    ax_b.set_xlabel("address size $n$")
    ax_b.set_ylabel("explicitly simulated branches")
    panel_tag(ax_b, "(b)")
    for ax in (ax_a, ax_b):
        ax.set_xticks(ns)
        style_axis(ax)

    handles, labels = ax_a.get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=5,
               title="solid: full   dotted: pruned;   $\\varepsilon=\\gamma$:",
               bbox_to_anchor=(0.5, -0.01))
    fig.tight_layout(rect=(0, 0.16, 1, 1), pad=0.3)
    fig.savefig(out_dir / "fig_perf.pdf")
    fig.savefig(out_dir / "fig_perf.png", dpi=300)
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
