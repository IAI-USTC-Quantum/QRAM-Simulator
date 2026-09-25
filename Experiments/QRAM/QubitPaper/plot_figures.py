#!/usr/bin/env python3
"""Regenerate the paper figures fig_fidelity.pdf / fig_perf.pdf from the CSVs
output by QubitPaperScan.

Usage:
    python3 plot_figures.py <results_dir> <out_dir>

Inputs:
    fidelity_scan.csv  -- fidelity-mode output (fig 3)
    perf_scan.csv      -- perf-mode output (fig 4)

Style: PRA (APS) single-column 3.4 in / two-column 7.0 in, 8 pt body font;
figures are inserted into LaTeX (\\columnwidth / \\textwidth) at their original
size, so the text in the figure is the final printed font size.
"""

import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

# APS/PRA convention: no title, (a)/(b) corner tags inside the axes; Helvetica-like sans-serif small font
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


def aggregate(rows, key_fields, value_field):
    """Group by key_fields; return {key: (mean, sample std over the runs)}"""
    acc = defaultdict(list)
    for r in rows:
        acc[tuple(r[f] for f in key_fields)].append(r[value_field])
    out = {}
    for k, v in acc.items():
        mean = sum(v) / len(v)
        std = (sum((x - mean) ** 2 for x in v) / (len(v) - 1)) ** 0.5 \
            if len(v) > 1 else 0.0
        out[k] = (mean, std)
    return out


ERRBAR = dict(capsize=1.4, elinewidth=0.55, capthick=0.55, markeredgewidth=0.6)


def log_yerr(means, stds):
    """Error bars for a log axis: clamp the lower bar so it never goes <= 0."""
    return [[min(s, 0.9 * m) for m, s in zip(means, stds)], list(stds)]


def style_axis(ax):
    ax.grid(True, color="#cccccc", linewidth=0.4, alpha=0.6)
    ax.set_axisbelow(True)
    ax.tick_params(direction="in", top=True, right=True)


def panel_tag(ax, tag, x=0.03, y=0.95):
    ax.text(x, y, tag, transform=ax.transAxes, fontsize=8,
            ha="left", va="top", fontweight="bold")


def plot_fidelity(rows, out_dir):
    qubit = [r for r in rows if r["arch"] == "qubit" and r["version"] == "full"]
    fid = aggregate(qubit, ["eps", "n"], "fid")

    fig, ax = plt.subplots(figsize=(3.4, 2.5))

    for eps in (1e-5, 1e-4, 1e-3):
        ns = sorted(n for (e, n) in fid if e == eps)
        means = [fid[(eps, n)][0] for n in ns]
        stds = [fid[(eps, n)][1] for n in ns]
        ax.errorbar(ns, means, yerr=stds, fmt="-o", color=COLORS[eps],
                    label=r"$10^{-%d}$" % round(-math.log10(eps)), **ERRBAR)
    ax.set_xlabel("Address size $n$")
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
    per = aggregate(rows, ["eps", "n", "version"], "time_ms")
    branches = aggregate(rows, ["eps", "n", "version"], "branches")

    ns = sorted({n for (_, n, _) in per})
    fig, (ax_a, ax_b) = plt.subplots(1, 2, figsize=(7.0, 2.55))

    for eps in (0.0, 1e-5, 1e-4, 1e-3):
        color = COLORS[eps]
        marker = MARKERS[eps]
        label = r"$10^{-%d}$" % round(-math.log10(eps)) if eps > 0 else "0"
        for ax, data, ylab in ((ax_a, per, "Time per query (ms)"),
                               (ax_b, branches, "Explicitly simulated branches")):
            full_m = [data[(eps, n, "full")][0] for n in ns]
            full_s = [data[(eps, n, "full")][1] for n in ns]
            norm_m = [data[(eps, n, "normal")][0] for n in ns]
            norm_s = [data[(eps, n, "normal")][1] for n in ns]
            ax.errorbar(ns, full_m, yerr=log_yerr(full_m, full_s), fmt="-",
                        marker=marker, color=color, label=label, **ERRBAR)
            ax.errorbar(ns, norm_m, yerr=log_yerr(norm_m, norm_s), fmt=":",
                        marker=marker, color=color, markerfacecolor="none",
                        **ERRBAR)
    ax_a.set_yscale("log")
    ax_a.set_xlabel("Address size $n$")
    ax_a.set_ylabel("Time per query (ms)")
    panel_tag(ax_a, "(a)")
    ax_b.set_yscale("log")
    ax_b.set_xlabel("Address size $n$")
    ax_b.set_ylabel("Explicitly simulated branches")
    panel_tag(ax_b, "(b)")
    for ax in (ax_a, ax_b):
        ax.set_xticks(ns)
        style_axis(ax)

    handles, labels = ax_a.get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=5,
               title="Solid: full   dotted: pruned;   $\\varepsilon=\\gamma$:",
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
