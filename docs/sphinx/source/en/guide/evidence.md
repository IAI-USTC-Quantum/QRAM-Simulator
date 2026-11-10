# Depth-20 Four-Channel Evidence Replay (Depth-20 four-channel)

The fixed depth-20 four-channel material — **20 tree layers, 1,048,576 seeded random data
records, the four noise channels** (BitFlip / PhaseFlip / Depolarizing /
Damping) on one identical input — ships as a standard product entry, not a
repository-internal script. Every channel and probability point consumes the
same memory digest and the same 16-address coherent superposition.

```bash
qram-simulator evidence                       # print the summary JSON
qram-simulator evidence --outdir records/     # + per-run raw JSON files
qram-simulator evidence --out summary.json
python tools/depth20_four_channel.py --outdir build-evidence   # repo wrapper
```

The GUI exposes the same replay as the **深度20四信道证据复算** panel
(default configuration, about one to three minutes on a laptop CPU, a
300-second deadline, and a JSON download). The panel and the CLI call the
same packaged module `qram_simulator.evidence`; ordinary users never touch
native internals.

## Configuration

| Parameter | Default | Meaning |
|---|---|---|
| `--trajectories` | 2048 | Monte-Carlo trajectories per channel point (recorded value 2048) |
| `--stress-trajectories` | 256 | trajectories at the phase-flip stress point |
| `--stress-probability` | 1e-4 | phase-flip stress probability (vacancy-model calibration point) |
| `--linearity-probability` | 4e-6 | X-type linearity point (damping uses 2e-6, fixed) |
| `--seed` | 20261109 | seed of the memory, the input superposition and every trajectory |

The core is fixed by the recorded material and is not a knob: depth 20, one
data bit, 16 superposed input addresses, the four channels at p=1e-6. For the
default seed the replay verifies that its memory digest equals the recorded
`77a36d821e367599b802a0969566f6507f7299dc4888e72b6fbe7b2cafe5a767` (the
v0.3.0 record) and fails loudly otherwise.

## The sixteen theory anchors

The summary (`qramsim.depth20-four-channel.v1`) embeds seeds, per-trajectory
samples, the memory digest, the theory formulas and version provenance, and
evaluates sixteen explicit same-depth anchors:

1. **Noiseless identity** — unitary, trace-preserving loading gives full and
   logical fidelity exactly 1;
2. **Phase-flip vacancy model** — Z strikes only matter on excited positions;
   the effective excited-route size is calibrated at the stress probability
   and *predicts* (never fits) the dip fraction and mean deficit at p=1e-6,
   verified within Poisson tolerance;
3. **X-type residue collapse** — X strikes on idle positions are never
   uncomputed, so the full-circuit overlap collapses while the tree-traced
   logical fidelity stays clearly higher;
4. **First-order Pauli composition and linear-in-p deficits** — the
   depolarizing deficit combines the 2/3 X-type flip damage with the 2/3
   Z/Y phase-dip contribution, and deficits scale linearly in p;
5. **Damping K0 occupation scale** — the joint whole-tree damping deficit
   stays on the documented O(depth × routing window) schedule scale;
6. **Normalization** — every trajectory keeps trace error < 1e-9.

Sampling-type anchors (stress decay, monotonicity, the vacancy prediction
window, the damping occupation scale) are calibrated for the recorded
2048-trajectory configuration; a reduced-trajectory run may fail individual
sampling anchors honestly — lower counts see fewer rare jump events. The
noiseless identity, digest, X-type collapse and normalization anchors are
deterministic at any size.

## Scope

One representative seeded input, not an exhaustive-input proof; 95% marginal
Monte-Carlo intervals per observable with no unified confidence certificate;
not a hardware run and not an acceptance certificate. Historical summaries
are never edited — a model change recomputes into new artifacts and keeps
the old ones.
