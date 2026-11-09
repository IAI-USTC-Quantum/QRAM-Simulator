"""Depth-20 four-channel 2.1 evidence driver: same input, seeded, with anchors.

Thin wrapper over the packaged :mod:`qram_simulator.evidence` entry — the
single authoritative implementation — kept as the recorded repository-side
invocation. Product surfaces (``qram-simulator evidence`` CLI and the GUI
evidence panel) call the same code path.

Usage:
    python tools/depth20_four_channel.py --outdir build-evidence

Uses the installed qram_simulator package (native CPU engine); writes one
raw JSON per run plus depth20-four-channel-summary.json embedding seeds,
per-trajectory samples, memory digest, formulas and version provenance.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from qram_simulator.evidence import EvidenceConfig, run_evidence


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--outdir", type=Path, default=Path("build-evidence"))
    defaults = EvidenceConfig()
    parser.add_argument("--trajectories", type=int, default=defaults.trajectories)
    parser.add_argument("--stress-trajectories", type=int, default=defaults.stress_trajectories)
    parser.add_argument("--stress-probability", type=float, default=defaults.stress_probability)
    parser.add_argument(
        "--linearity-probability", type=float, default=defaults.linearity_probability
    )
    parser.add_argument("--seed", type=int, default=defaults.seed)
    args = parser.parse_args()
    args.outdir.mkdir(parents=True, exist_ok=True)
    summary = run_evidence(
        EvidenceConfig(
            trajectories=args.trajectories,
            stress_trajectories=args.stress_trajectories,
            stress_probability=args.stress_probability,
            linearity_probability=args.linearity_probability,
            seed=args.seed,
        ),
        outdir=args.outdir,
    )
    for label, result in sorted(summary["results"].items()):
        print(
            f"{label:24s} p={result['probability']:<8g} "
            f"logical={result['logical_mean']:.6f} "
            f"full={result['full_mean']:.6f} wall={result['wall_s']:.1f}s",
            flush=True,
        )
    failed = [name for name, ok in summary["checks"].items() if ok is not True]
    print("ANCHORS:", "ALL PASS" if not failed else f"FAILED {failed}")
    return 0 if not failed else 1


if __name__ == "__main__":
    raise SystemExit(main())
