#!/usr/bin/env python3
"""从 QubitPaperVerify 输出的 CSV 生成验证图并打印验收指标。

用法：
    python3 plot_verify.py <results_dir> <out_dir>

输入：
    verify_hamming.csv  —— hamming 模式（H→K0^d→H 闭式 + 条件错误率）
    verify_qutrit.csv   —— qutrit 模式（标量权重对拍 + 分支内保真度）
    verify_avgfid.csv   —— avgfid 模式（轨迹平均 vs no-fault 桶 infidelity）
输出：
    fig_verify_hamming.pdf / fig_verify_scaling.pdf（同时存 .png 便于目检）

风格：PRA（APS）双栏 7.0 in，字号 8 pt；面板内不放结论文本，数值进 caption。
"""

import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

matplotlib.rcParams.update({
    "font.size": 8,
    "axes.labelsize": 8,
    "xtick.labelsize": 7,
    "ytick.labelsize": 7,
    "legend.fontsize": 6.5,
    "legend.title_fontsize": 6.5,
    "lines.linewidth": 1.1,
    "lines.markersize": 3.0,
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

N_LIST = (2, 3, 4, 5, 6, 8)
COLORS_N = {2: "#7f7f7f", 3: "#1f77b4", 4: "#2ca02c",
            5: "#ff7f0e", 6: "#9467bd", 8: "#d62728"}


def popcount(x):
    return bin(x).count("1")


def load_hamming(path):
    """返回 {(n,gamma,traj): (k,b_out,{word: prob})}（prob = |amp|^2/归一化）"""
    acc = defaultdict(dict)
    meta = {}
    with open(path) as fp:
        for r in csv.DictReader(fp):
            n = int(r["n"]); g = float(r["gamma"]); t = int(r["traj"])
            k = int(r["k"]); bout = int(r["b_out"]); w = int(r["word"])
            amp2 = float(r["amp_re"]) ** 2 + float(r["amp_im"]) ** 2
            key = (n, g, t)
            acc[key][w] = amp2
            meta[key] = (k, bout)
    out = {}
    for key, d in acc.items():
        norm = sum(d.values())
        out[key] = (*meta[key], {w: a / norm for w, a in d.items()})
    return out


def theory_word_prob(n, gamma, k, w):
    """p(c) = alpha^{2(k-w)} beta^{2w} / (alpha^2+beta^2)^k，a=sqrt(1-gamma)，d=2n"""
    ad = (1.0 - gamma) ** n          # a^d = (1-gamma)^n
    alpha = (1.0 + ad) / 2.0
    beta = (1.0 - ad) / 2.0
    return alpha ** (2 * (k - w)) * beta ** (2 * w) / (alpha ** 2 + beta ** 2) ** k


def fit_slope(xs, ys):
    """log10-log10 线性拟合斜率"""
    xs = [math.log10(x) for x in xs]
    ys = [math.log10(y) for y in ys]
    n = len(xs)
    sx, sy = sum(xs), sum(ys)
    sxx = sum(x * x for x in xs)
    sxy = sum(x * y for x, y in zip(xs, ys))
    return (n * sxy - sx * sy) / (n * sxx - sx * sx)


def style_axis(ax):
    ax.grid(True, color="#cccccc", linewidth=0.4, alpha=0.6)
    ax.set_axisbelow(True)
    ax.tick_params(direction="in", top=True, right=True)


def panel_tag(ax, tag, x=0.03, y=0.95, ha="left"):
    ax.text(x, y, tag, transform=ax.transAxes, fontsize=8,
            ha=ha, va="top", fontweight="bold")


def plot_hamming(ham, qutrit_rows, out_dir):
    fig, (ax_a, ax_b) = plt.subplots(1, 2, figsize=(7.0, 2.6))

    # 全 CSV 范围的理论/模拟最大相对偏差（进 caption，不画在面板内）
    max_rel = 0.0
    for (n, g, _), (k, bout, words) in ham.items():
        for w, p in words.items():
            pth = theory_word_prob(n, g, k, popcount(w ^ bout))
            if pth > 1e-12:
                max_rel = max(max_rel, abs(p - pth) / pth)

    # ---- (a) Hamming collapse：按 w 分组的字概率 vs 理论线 ----
    series = [(3, 1e-3), (5, 1e-3), (8, 1e-3), (8, 1e-4), (8, 1e-2)]
    markers = ["o", "s", "^", "D", "v"]
    for (n, g), mk in zip(series, markers):
        pts = defaultdict(list)  # w -> [p_meas 各字各轨迹]
        for (nn, gg, _), (k, bout, words) in ham.items():
            if nn != n or abs(gg - g) > 1e-15:
                continue
            for w, p in words.items():
                pts[popcount(w ^ bout)].append(p)
        k = 3
        xs, ys = [], []
        for wdist, plist in pts.items():
            for j, p in enumerate(plist):
                # 微小横向抖动让同 w 的 2^k 个字各自可见
                xs.append(wdist + (j % 8 - 3.5) * 0.012)
                ys.append(p)
        color = COLORS_N[n]
        ls = "-" if g == 1e-3 else ("--" if g == 1e-4 else ":")
        ax_a.scatter(xs, ys, s=6, color=color, alpha=0.55, marker=mk,
                     linewidths=0, label=rf"$n={n},\ \gamma=10^{{{round(math.log10(g))}}}$")
        ws = list(range(0, k + 1))
        ax_a.plot(ws, [theory_word_prob(n, g, k, w) for w in ws],
                  ls, color=color, linewidth=1.0)
    ax_a.set_yscale("log")
    ax_a.set_ylim(1e-13, 3)
    ax_a.set_xticks(range(0, 4))
    ax_a.set_xlabel(r"error weight $w=\mathrm{Ham}(c\oplus b_{\mathrm{out}})$")
    ax_a.set_ylabel(r"word probability $p(c)$")
    ax_a.legend(loc="lower left", markerscale=0.8)
    panel_tag(ax_a, "(a)", x=0.97, ha="right")
    style_axis(ax_a)

    # ---- (b) qutrit 标量：显式演化 vs 解析计数器 ----
    xs = [float(r["weight_ratio_theory"]) for r in qutrit_rows]
    ys = [float(r["weight_ratio_meas"]) for r in qutrit_rows]
    ax_b.scatter(xs, ys, s=6, color="#1f77b4", alpha=0.5, linewidths=0)
    lo, hi = min(xs + ys), max(xs + ys)
    ax_b.plot([lo * 0.98, hi * 1.02], [lo * 0.98, hi * 1.02],
              "-", color="#d62728", linewidth=1.0, label=r"$y=x$")
    ax_b.set_xlabel(r"analytic counter $(1-\gamma)^{\Delta c}$")
    ax_b.set_ylabel(r"measured $|A_i|^2/|A_{\mathrm{ref}}|^2$")
    ax_b.legend(loc="upper left")
    panel_tag(ax_b, "(b)", x=0.97, ha="right")
    style_axis(ax_b)

    fig.tight_layout(pad=0.3)
    fig.savefig(out_dir / "fig_verify_hamming.pdf")
    fig.savefig(out_dir / "fig_verify_hamming.png", dpi=300)
    plt.close(fig)
    return max_rel


def plot_scaling(ham, avgfid_rows, out_dir):
    gammas = [1e-5, 3e-5, 1e-4, 3e-4, 1e-3, 3e-3, 1e-2, 3e-2]
    k = 3

    # hamming → 条件（no-jump）数据寄存器错误率
    cond = defaultdict(list)  # (n,gamma) -> [1-p(b_out)]
    for (n, g, _), (_, bout, words) in ham.items():
        cond[(n, g)].append(1.0 - words.get(bout, 0.0))
    cond_err = {key: sum(v) / len(v) for key, v in cond.items()}

    # avgfid → 全体平均 / no-fault 桶平均；fired = inf>0.5（jump 实际点火）
    bucket_all = defaultdict(list)
    bucket_nf = defaultdict(list)
    fired_cnt = defaultdict(int)
    traj_cnt = defaultdict(int)
    for r in avgfid_rows:
        n = int(r["n"]); g = float(r["gamma"]); inf = float(r["infidelity"])
        key = (n, g)
        bucket_all[key].append(inf)
        traj_cnt[key] += 1
        if int(r["has_fault"]) == 0:
            bucket_nf[key].append(inf)
        if inf > 0.5:
            fired_cnt[key] += 1
    mean_all = {key: max(sum(v) / len(v), 1e-17) for key, v in bucket_all.items()}
    mean_nf = {key: max(sum(v) / len(v), 1e-17)
               for key, v in bucket_nf.items() if v}

    fig, (ax_a, ax_b) = plt.subplots(1, 2, figsize=(7.0, 2.6))

    # ---- (a) 条件错误率：per-qubit，斜率 ~2，系数 n^2 gamma^2 / 4 ----
    slopes = []
    print("\n[scaling] conditional (no-jump) per-qubit data-register error:")
    for n in N_LIST:
        gs = [g for g in gammas if (n, g) in cond_err]
        es = [cond_err[(n, g)] / k for g in gs]
        slope = fit_slope(gs, es)
        slopes.append(slope)
        # 系数检查：取 gamma=1e-4（小 γ、远离 fp 地板）
        g0 = 1e-4
        ratio = (cond_err[(n, g0)] / k) / (n * n * g0 * g0 / 4)
        print(f"  n={n}: slope={slope:.4f}  coeff ratio vs n^2 g^2/4 = {ratio:.4f}")
        ax_a.plot(gs, es, "o", color=COLORS_N[n], markersize=3.0,
                  label=rf"$n={n}$")
        ax_a.plot(gs, [n * n * g * g / 4 for g in gs], "-",
                  color=COLORS_N[n], linewidth=0.9, alpha=0.8)
    print(f"  fitted slope range: [{min(slopes):.4f}, {max(slopes):.4f}]")
    ax_a.set_xscale("log")
    ax_a.set_yscale("log")
    ax_a.set_xlabel(r"damping rate $\gamma$")
    ax_a.set_ylabel(r"conditional error / qubit")
    ax_a.legend(loc="lower right", ncol=2)
    panel_tag(ax_a, "(a)")
    style_axis(ax_a)

    # ---- (b) 轨迹平均（斜率 ~1）与 no-fault 桶（斜率 ~2） ----
    # 一阶斜率只在线性窗口内拟合：要求 fired 统计充足（>=20 条）且未饱和
    # （mean < 0.3）。 fired < 20 的欠采样点（极小 γ 的泊松地板）不画出，
    # 以免稀有事件量化噪声造成 V 形伪影。
    print("\n[scaling] trajectory-averaged infidelity (avgfid):")
    omitted = []
    for n in (3, 5, 8):
        gs = [g for g in gammas if (n, g) in mean_all]
        win = [g for g in gs
               if fired_cnt[(n, g)] >= 20 and mean_all[(n, g)] < 0.3]
        slope_all = fit_slope(win, [mean_all[(n, g)] for g in win]) \
            if len(win) >= 2 else float("nan")
        slope_pf = fit_slope(win, [fired_cnt[(n, g)] / traj_cnt[(n, g)]
                                   for g in win]) if len(win) >= 2 else float("nan")
        gs_nf = [g for g in gammas if (n, g) in mean_nf]
        es_nf = [mean_nf[(n, g)] for g in gs_nf]
        slope_nf = fit_slope(gs_nf, es_nf) if len(gs_nf) >= 2 else float("nan")
        print(f"  n={n}: slope(all, window)={slope_all:.4f}  "
              f"slope(P_fire, window)={slope_pf:.4f}  "
              f"slope(no-fault bucket)={slope_nf:.4f}")
        print(f"      window={['%g' % g for g in win]}  "
              f"fires={[(f'{g:g}', fired_cnt[(n, g)], traj_cnt[(n, g)]) for g in win]}")
        # 连线只穿过统计充足的点（fired >= 20）
        strong = [g for g in gs if fired_cnt[(n, g)] >= 20]
        omitted += [(n, g) for g in gs if g not in strong]
        ax_b.plot(strong, [mean_all[(n, g)] for g in strong], "-o",
                  color=COLORS_N[n], markersize=3.0, linewidth=1.0,
                  label=rf"$n={n}$ all: ${slope_all:.2f}$")
        ax_b.plot(gs_nf, es_nf, "--s", color=COLORS_N[n], markersize=3.0,
                  markerfacecolor="none", linewidth=1.0,
                  label=rf"$n={n}$ no-fault: ${slope_nf:.2f}$")
    for n, g in omitted:
        print(f"  omitted (fired<20): n={n}, gamma={g:g}")
    # 参考斜率线 ∝γ（穿过线性区上方空白处）
    gref = [3e-4, 3e-3]
    ax_b.plot(gref, [3.0 * g for g in gref], ":", color="#555555",
              linewidth=0.9)
    ax_b.text(7e-4, 3.0 * 7e-4 * 1.25, r"$\propto\gamma$", fontsize=6.5,
              color="#555555", rotation=38)
    ax_b.set_xscale("log")
    ax_b.set_yscale("log")
    ax_b.set_xlabel(r"damping rate $\gamma$")
    ax_b.set_ylabel(r"trajectory infidelity $1-F$")
    ax_b.legend(loc="lower right")
    panel_tag(ax_b, "(b)")
    style_axis(ax_b)

    fig.tight_layout(pad=0.3)
    fig.savefig(out_dir / "fig_verify_scaling.pdf")
    fig.savefig(out_dir / "fig_verify_scaling.png", dpi=300)
    plt.close(fig)


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    results = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])
    out_dir.mkdir(parents=True, exist_ok=True)

    ham = load_hamming(results / "verify_hamming.csv")
    with open(results / "verify_qutrit.csv") as fp:
        qutrit_rows = list(csv.DictReader(fp))
    with open(results / "verify_avgfid.csv") as fp:
        avgfid_rows = list(csv.DictReader(fp))

    max_rel = plot_hamming(ham, qutrit_rows, out_dir)
    rel_dev = max(abs(float(r["weight_ratio_meas"]) - float(r["weight_ratio_theory"]))
                  / float(r["weight_ratio_theory"])
                  for r in qutrit_rows if float(r["weight_ratio_theory"]) > 0)
    fid_dev = max(abs(float(r["fidelity_to_ideal"]) - 1.0) for r in qutrit_rows)
    print("fig_verify_hamming ->", out_dir / "fig_verify_hamming.pdf")
    print(f"  hamming max rel dev (p_th>1e-12, full CSV): {max_rel:.3e}")
    print(f"  qutrit max ratio rel dev: {rel_dev:.3e}   max |F-1|: {fid_dev:.3e}")
    plot_scaling(ham, avgfid_rows, out_dir)
    print("fig_verify_scaling ->", out_dir / "fig_verify_scaling.pdf")


if __name__ == "__main__":
    main()
