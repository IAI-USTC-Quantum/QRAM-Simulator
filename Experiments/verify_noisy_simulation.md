# verify_noisy_simulation — QRAM-Simulator ↔ circuit-level noise simulation cross-check (CI case)

`Experiments/verify_noisy_simulation.cpp` (ctest case `verify_noisy_simulation`, run automatically with the
full CI regression suite, about 3 seconds): bakes the "symbolic sparse-tree trajectory engine ↔ circuit-level
noise simulation" cross-check established in this workspace into the repository, using a set of deterministic experiments to continuously guarantee the correctness of noise simulation in both architectures.

## Structure

- **Built-in standalone simulator** (zero code shared with the symbolic tree engine): a state-vector engine + a sparse density-matrix engine;
  gates/channels are unified as "local matrix + control-polarity set" primitives (arbitrary 1q/2q/3q local matrices and Kraus families supported,
  no gate decomposition needed).
- **Encoding**: a qutrit node is 3 qubits (W=00/L=01/R=10/dead state 11 + data); a qubit node is 2 qubits
  (addr + data; phase-kickback FetchData + real Hadamards in CopyIn/CopyOut).
- **Reference side**: `qram_qutrit::QRAMCircuit` / `qram_qubit::QRAMCircuit` run directly in-process
  (`run_full` full-branch trajectories); Damp_Full's second sampling is intercepted by a replica execution (aligned RNG draw by draw).

## Experiment Ladder (14 groups, 55 assertions)

| # | Experiment | Assertions |
| --- | --- | --- |
| S0 | qutrit/qubit noise-free bridge (addr=2, all memories) | output distributions bitwise equal (<1e-9) |
| S1 | depol p∈{0.02,0.3}, damp γ=0.05 per random case (6 cases each × both architectures) | sub-normalized distributions including jump outcomes bitwise equal (<1e-16 level) |
| S2 | depol/damp/mixed channel-level faithful mirror (300-trajectory reference) | F_cls(normalized)>0.99~0.995, TVD<0.05, \|trace−survival\|<0.01~0.02, \|F_quantum−avg_overlap_fid\|<0.05 |

All use fixed seeds (deterministic); failure returns a non-zero exit code. The `VNS_TRACE`/`VNS_TRACE2`/`VNS_DEBUG`
environment variables enable step-wise excitation/distribution debug probes.

## Implementation differences this cross-check caught and fixed during development (direct proof of the cross-check's value)

1. The density engine's first version had wrong control semantics (the control-not-satisfied side wrongly used the matrix's diagonal elements instead of the identity) — immediately exposed by the S2 zero-noise
   self-check (trace=0).
2. The qutrit `SwapInternal{ℓ≥1}` translation emitted a controlled internal_swap for both children — the ground `(W,0)` not selected by routing
   was wrongly excited into `(L,0)`; invisible in the (addr,bus) marginal distribution (slipped past S0),
   but over-damped by Damp_Common (S1 damping norm difference ~10%) and entering the marginal distribution under high-noise depol.
3. A bit-order misalignment in the qutrit (a1,a0) local matrices (L↔R swapped) — invisible in distributions for phase-type operators,
   exposed for permutation-type/jump operators at p=0.3 / under damping.
4. The qubit `CopyOut` Hadamard was emitted twice (once before and once after the swap) — `|±>` copied extra excitations between node0.d and the
   bus; the marginal distribution stayed correct, but the damping S1/S2 trace↔survival difference was 0.06;
   0.001 after the fix.

Additionally: the library-side `qram_qubit::SystemState::run_bitphaseflip` has been fixed to a Y flip per the design intent
(|0>→−|1>, |1>→|0>, consistent with the qutrit architecture's odd-position semantics); the old implementation was a rank-1 non-unitary operator
and was the root cause of the qubit architecture's `sample_output` zero-norm crashes under high noise (see the repository's experiment README).
Both architectures' `set_noise_models` gained probability-range validation (∈[0,1], Damping requires γ<1),
excluding at the input side the only remaining zero-norm path.

These findings corroborate those of the interactive pipeline (`Experiments/QRAM/ChannelCorrespondence/`, uniqc/QuTiP backend):
**marginal-distribution matching is not enough to guarantee a correct tree state; a damping/coherence-convention verification dimension is required.**

## Running

```bash
cmake --build build --target verify_noisy_simulation
./build/bin/verify_noisy_simulation          # or
ctest --test-dir build -R verify_noisy
```
