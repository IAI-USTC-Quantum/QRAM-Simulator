# QRAM Addressing Test Plan: Records, Data Exchange and Mitigation Threshold

This page describes reproducible QRAM addressing and data-exchange tests. QRAM-Simulator provides local simulation and offline record validation; UniQC maintains cloud-platform access. Source flags distinguish simulated records from hardware records. These steps do not submit device jobs.

## Tested object and its three non-interchangeable functions

| Function | Entry | What it is / is not |
|---|---|---|
| Three-layer QRAM loading | `qram-simulator run` | noisy bucket-brigade routing through a physical tree (2-3 routing layers in the GUI; up to depth 20 via CLI); not a readout experiment |
| QPU addressing data access | `qram-simulator addressing` | offline standard I/O: seeded 2-3 layer, 4-8 record random memories, uniform coherent addressing, mean `(address:bus)` distributions; **no hardware access from this repo** |
| Mitigation simulation | same entry, mitigation block | calibrated symmetric bus-flip confusion inverted per address — a *simulation reference* for the mitigation criterion, not a hardware-readout mitigation run |

## Real 2-3 layer, 4-8 record random data

The addressing entry derives its memory from the seed deterministically:
layer 2 → 4 records, layer 3 → 8 records, one bit per record, recorded in
the JSON (`memory.records`). Fixed fixtures (seed 20261109, 256
trajectories) exist for every channel and both layer counts — regenerate
with the fixed commands:

```bash
for L in 2 3; do
  for C in none BitFlip PhaseFlip Depolarizing Damping; do
    qram-simulator addressing --layers $L --channel $C \
      --probability 0.001 --trajectories 256 --seed 20261109 \
      --out addressing-L$L-$C.json
  done
done
```

(The noiseless records use `--channel none --probability 0`.)

## QPU data access — the offline standard I/O

`qramsim.addressing.v1` JSON is the declared interface consumed by the
uniqc side: configuration, seeded memory, resources, the mean output
distribution, raw/mitigated correct-addressing values, the calibrated flip
rate, the mitigation formula and explicit scope flags. Export comes from
the CLI (`--out`) or the GUI panel (download). Re-import for verification:

```bash
qram-simulator addressing --evaluate addressing-L3-Depolarizing.json
```

`--evaluate` performs the **offline import**: it validates the schema and
required numeric fields of a record produced by this software *or by the
declared uniqc consumer*, recomputes the mitigated-addressing criterion
from the recorded values, and reports consistency flags
(`qramsim.addressing-eval.v1`). It never re-runs trajectories and never
touches hardware; evaluation of a simulated record is not a hardware test.

## Error-mitigated fidelity ≥ 75% — existing records

The criterion uses the declared example threshold —
`mitigated_correct_addressing >= 0.75`, with value and operator recorded in
the JSON (`c.threshold`, `criterion.operator`, `criterion.value`,
`criterion.pass`). The existing layer-2/layer-3 records (seed 20261109,
p=0.001, 256 trajectories) all pass, e.g.:

- L3 Depolarizing: raw 0.9941 → mitigated **0.9985** (pass)
- L2 BitFlip / PhaseFlip / Depolarizing / Damping and L3 variants:
  recorded in the same artifact set; regenerate with the fixed commands
  above and compare semantically (same seed ⇒ same memory and samples on
  the same backend).

The mitigation model is the symmetric single-bit bus-flip confusion matrix
calibrated against the known memory, inverted per address and clipped to
[0, 1]: `true = ((1-e)*obs - e*obs_flipped) / (1-2e)` with `e` the
calibrated flip rate. It mitigates routing-induced flips to the extent they
follow that model — it is not a certificate over arbitrary error models,
and mitigated values can only be interpreted together with the recorded
model and flip rate.

## Fixed operation instructions (GUI)

1. `qram-simulator gui` → **寻址缓解仿真** panel;
2. set layers (2/3), channel, probability (≤ 0.1), trajectories (≤ 256),
   seed; run; the criterion verdict, flip rate, distribution and records
   are shown;
3. download `qram-addressing.json` (the `qramsim.addressing.v1` record);
4. re-import offline with `qram-simulator addressing --evaluate <file>`.

The GUI distinguishes this mitigation simulation from the noisy QRAM
experiment panel and from the ideal wide-word query panel; the three are
not interchangeable and the interface labels keep them apart.
