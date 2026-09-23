#!/usr/bin/env python3
"""分析 kscan.csv：qubit vs qutrit 保真度随 k 和 n 的标度。

输出：均值表（含 SEM）、deficit 比值、log-log 斜率拟合、汇总图 fig_kscan.pdf。
"""

import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load(path):
    rows = []
    with open(path) as f:
        for r in csv.DictReader(f):
            rows.append(dict(arch=r["arch"], eps=float(r["eps"]), n=int(r["n"]),
                             k=int(r["k"]), fid=float(r["fid"])))
    return rows


def stats(xs):
    m = sum(xs) / len(xs)
    var = sum((x - m) ** 2 for x in xs) / len(xs)
    return m, math.sqrt(var / len(xs))


def main():
    results = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])
    rows = load(results / "kscan.csv")

    groups = defaultdict(list)
    for r in rows:
        groups[(r["arch"], r["eps"], r["n"], r["k"])].append(r["fid"])

    def get(arch, eps, n, k):
        m, s = stats(groups[(arch, eps, n, k)])
        return m, s

    # ---- (a) k 依赖（n=8 固定） ----
    print("== deficit (1-F) vs k, n=8 ==")
    for eps in (1e-5, 1e-4):
        print(f"eps={eps:g}")
        for k in (1, 2, 3, 5):
            mq, sq = get("qubit", eps, 8, k)
            mt, st = get("qutrit", eps, 8, k)
            dq, dt = 1 - mq, 1 - mt
            ratio = dq / dt if dt > 0 else float("nan")
            print(f"  k={k}: qubit={mq:.5f}±{sq:.5f} (def {dq:.2e})  "
                  f"qutrit={mt:.5f}±{st:.5f} (def {dt:.2e})  ratio={ratio:.2f}")

    # ---- (b) n 标度（eps=1e-4, k∈{1,5}） ----
    print("== log-log slope of deficit vs n, eps=1e-4 ==")
    ns = [4, 6, 8, 10]
    for k in (1, 5):
        for arch in ("qubit", "qutrit"):
            xs = [math.log(n) for n in ns]
            ys = []
            for n in ns:
                m, _ = get(arch, 1e-4, n, k)
                ys.append(math.log(max(1 - m, 1e-9)))
            xbar = sum(xs) / len(xs)
            ybar = sum(ys) / len(ys)
            slope = sum((x - xbar) * (y - ybar) for x, y in zip(xs, ys)) / \
                sum((x - xbar) ** 2 for x in xs)
            pts = " ".join(f"n{n}:{1-get(arch,1e-4,n,k)[0]:.3e}" for n in ns)
            print(f"  k={k} {arch}: slope={slope:.2f}   ({pts})")

    # ---- 图 ----
    fig, (ax_a, ax_b) = plt.subplots(1, 2, figsize=(7.2, 3.0))
    ks = (1, 2, 3, 5)
    style = {"qubit": dict(color="#d62728", marker="o", ls="-"),
             "qutrit": dict(color="#1f77b4", marker="s", ls="--")}
    for eps, ls in ((1e-5, "-"), (1e-4, ":")):
        for arch in ("qubit", "qutrit"):
            sty = dict(style[arch]); sty["ls"] = ls
            ys = [1 - get(arch, eps, 8, k)[0] for k in ks]
            ax_a.plot(ks, ys, **sty, markersize=5,
                      label=f"{arch}, $\\varepsilon$=10$^{{-{round(-math.log10(eps))}}}$")
    ax_a.set_yscale("log")
    ax_a.set_xlabel("data width $k$")
    ax_a.set_ylabel("infidelity $1-F$")
    ax_a.set_title("(a) $k$-dependence, $n=8$")
    ax_a.legend(fontsize=7)
    ax_a.grid(True, alpha=0.4)

    for k, mk in ((1, "o"), (5, "^")):
        for arch in ("qubit", "qutrit"):
            ys = [1 - get(arch, 1e-4, n, k)[0] for n in ns]
            ax_b.plot(ns, ys, marker=mk, color=style[arch]["color"],
                      ls=style[arch]["ls"], markersize=5, label=f"{arch}, $k$={k}")
    ax_b.set_xscale("log")
    ax_b.set_yscale("log")
    ax_b.set_xticks(ns)
    ax_b.set_xticklabels([str(n) for n in ns])
    ax_b.set_xlabel("address size $n$")
    ax_b.set_ylabel("infidelity $1-F$")
    ax_b.set_title("(b) $n$-scaling, $\\varepsilon=10^{-4}$")
    ax_b.legend(fontsize=7)
    ax_b.grid(True, alpha=0.4)

    fig.tight_layout()
    fig.savefig(out_dir / "fig_kscan.pdf")
    fig.savefig(out_dir / "fig_kscan.png", dpi=200)
    print("fig_kscan ->", out_dir / "fig_kscan.pdf")


if __name__ == "__main__":
    main()
