# ChannelCorrespondence — qutrit QRAM noise model ↔ circuit-level channel simulation correspondence experiment

Research question: **Can QRAM-Simulator's (trajectory-level, TimeStep-scheduled) qutrit QRAM noise simulation be put in correspondence with circuit-level (density matrix / channel-level) noisy simulation?**

This directory is the complete deliverable of Step 1. The noise type in scope is **Depolarizing** (the Damping interface surface is retained; channelized correspondence is deferred to M3).
The **trajectory mode** is executed in its true sense: once sampling is complete (positions and the X/Z/Y/qutrit Weyl operators all determined), each case is realized as
**deterministic unitary gates** and **cross-checked exactly, case by case** against the same random case in QRAM-Simulator; the channel level
(density matrix) serves as the statistical control. The uniqc `NoisySimulator`/ErrorLoader mechanism is not enabled.

## Summary of Conclusions (TL;DR)

| Verification layer | Result |
| --- | --- |
| **Stage 0** noise-free bridge (encoded gate circuit vs QRAM noise-free run) | **Bitwise exact match (max\|ΔP\| ≈ 1e-16)** |
| **Stage 1** per random case (8 realizations, after sampling → concrete operators → unitary gates) | **All exact matches (1e-16, native and model double references agree)** |
| **Stage 2** channel level · mode c (channels placed on per-step active positions × marginal probability) | **F_classical ≥ 0.9966 (p = 0.005→0.3), TVD ≤ 0.054** |
| **Stage 2** mode b (channels only on the extracted positions of a single realization) | F 0.92–0.97: a single realization under-noises, as expected |
| **fidelity statistical consistency** | **F_quantum(channel) ≈ E\|⟨ψ_ideal\|ψ_traj⟩²\| (holds at all noise strengths)**, see the [convention discussion](#fidelity-convention-evidence) |
| **Stage 1-d** Damping per random case (with second-sampling interception + single-Kraus replay) | **All exact matches (1e-16, sub-normalized convention, γ=0.001→0.2 and mixed configurations)** |
| **Stage 2-d** Damping channel level · faithful mirror | **F_cls(normalized) ≥ 0.977, trace ≈ survival; the textbook AD channel shows systematic deviation**, see [Damping path verification](#damping-path-verification-m3-1) |
| **qubit architecture** Stage 0/1 (depol+damping, all 8 cases × 10 configurations) | **All exact matches (1e-16)**, see [qubit-based QRAM verification](#qubit-based-qram-verification-m3-1b-depolarizing--damping-complete) |
| **qubit architecture** channel-level faithful | **F_cls(normalized) ≥ 0.9974 for all configurations; F_quantum ≈ avg_overlap_fid; trace≈survival difference <1.5%** |
| OriginIR-ext textual channel path (data-position `Depolarizing q,(p)` inline) | Consistent with direct kraus driving, 0 error |
| renormalize, the two conventions | Depolarizing is CPTP (survival 1), both conventions agree; **under Damping they truly diverge** (see the damping section) |

**Core answer: the correspondence holds.** Compiling the TimeStep noise model into circuit-level channels as "per-step active positions × marginal probability p" yields a density-matrix output distribution that agrees with the QRAM-Simulator multi-trajectory average to within Monte Carlo error magnitude at all noise strengths;
moreover, the **state-level** (coherence included) `⟨ψ_ideal|ρ|ψ_ideal⟩` is statistically consistent with the trajectory ensemble's `E|⟨ψ_ideal|ψ_traj⟩|²`
— i.e., the user's intuition that "they should agree in a statistical sense" is confirmed under the no-post-selection, tree-sensitive convention.

Along the way, four upstream issues were found and fixed (three in the QRAM-Simulator library, one in uniqc; see
[Implementation-Level Fixes](#implementation-level-fixes)), and one input convention was confirmed (memory values must fit within data_size bits).

## Directory and Pipeline

```
QutritCorrespondenceExporter.cpp   C++ exporter (target Experiment_QRAM_ChannelCorrespondence)
run_correspondence.py              Python driver (run on an interpreter with uniqc, e.g., UnifiedQuantum/.venv)
results/                           run artifacts (gitignored, regenerable)
```

```
QRAM-Simulator (C++)                          uniqc (Python)
─────────────────────                          ─────────────────────
QRAMCircuit + set_noise_models         JSON     gate sequence → qutrit→qubit encoded circuit
TimeStep::generate schedule ─────────────────────→ Stage 0: statevector exact cross-check
  ├─ logical operators → encoded gates (control polarities included)  Stage 1: per-random-case unitary cross-check (schedule_r*.json)
  └─ noise operators {step,type,pos,coef}        Stage 2: channel placement (mode b/c) → density matrix
run_full × R trajectories → per-case distribution + averaged distribution    comparison: renormalize × {F_cls, TVD, F_quantum}
                  + four fidelity-convention reference quantities
```

## Usage

```bash
# C++ side (toolchain: workspace .tools/envs/devenv)
cmake --build build --target Experiment_QRAM_ChannelCorrespondence
cmake --build build --target Experiment_QRAM_ChannelCorrespondence_Qubit
cd Experiments/QRAM/ChannelCorrespondence
../../../build/bin/Experiment_QRAM_ChannelCorrespondence \
    --addrsize 2 --datasize 1 --memory "0,1,1,0" \
    --seed 20260921 --runs 500 --depolarizing 0.02 --outdir results/depol002
../../../build/bin/Experiment_QRAM_ChannelCorrespondence_Qubit \
    --addrsize 2 --datasize 1 --memory "0,1,1,0" \
    --seed 20260921 --runs 500 --depolarizing 0.02 --damping 0.02 --outdir results/qubit_mixed

# Python side (the "arch" field of the schedule JSON auto-routes the qutrit/qubit path)
<path-to-uniqc-venv>/bin/python run_correspondence.py --dir results/depol002 --stage all
```

Exporter parameters: `--addrsize/--datasize/--memory/--seed/--runs/--depolarizing/--damping (interface
surface, correspondence deferred to M3)/--exportruns (export the schedules of the first K trajectories
for per-case cross-checks, default 8)/--input{zerobus,uniform}/--outdir/--tracesteps`. Driver parameters: `--dir/--stage{0,1,2,all}/--tol`.

**memory value convention**: every cell must fit within `data_size` bits (e.g., data=1 allows only 0/1) —
`Branch::get_fidelity`'s `expect_bus = bus_input ^ memory[address]` XORs the raw value directly,
and out-of-range values are silently scored as zero contribution. The exporter validates this and rejects out-of-range inputs.

Main configuration: **addr=2, data=1** (12 encoded qubits; with addr=1, `layer_entangle_max≡0` and noise sampling is
always empty, so addr≥2 is the smallest instance that carries noise). The input is **zerobus** (address in uniform superposition, bus=0): a bus in uniform
superposition would make the joint output distribution degenerate to uniform under any conditional permutation, losing discriminative power.

## qutrit→qubit Encoding and Operator Audit Table

Registers (`encoding` field): address `addr_size` bits (bit b ↔ qubit b); bus `data_size` bits;
each routing node `v ∈ [0, 2^n−1)` (heap numbering) takes 3 bits: `a1, a0` (level encoding **W=|00> (ground), L=|01>,
R=|10>, |11> dead state**, corresponding to `typedefs.h`'s `W=-1, L=0, R=1`) and `d` (the data bit).
Noise position `pos`: `node = pos/2`, `pos%2`: 0 → the addr-qutrit's (a1,a0), 1 → the data bit.

Logical operator translation (mirroring the dispatch of `QRAMCircuit::run_valid_branches`, `qram_circuit_qutrit.cpp:811`):

| Operation | QRAM-Simulator semantics | Encoded gates |
| --- | --- | --- |
| `FirstCopy{ℓ}` | `node0.data ^= addr_bit(n−1−ℓ)` (MSB-first, `get_digit_reverse`) | CNOT(addr[n−1−ℓ], node0.d) |
| `CopyIn{d}` / `CopyOut{d}` | SWAP(bus[d], node0.data) (a no-op together with try_merge) | SWAP(bus[d], node0.d) |
| `SwapInternal{0}` | root `internal_swap`: (W,0)↔(L,0), (W,1)↔(R,0) | 2 transpositions (multi-controlled X conjugation) |
| `SwapInternal{ℓ≥1}` | each node of layer ℓ−1 applies internal_swap to its child according to the addr direction | transposition on the child × double control on parent (a1,a0)==L/R |
| `ControlSwap{ℓ}` | each node of layer ℓ SWAPs (this node's .d, child.d) according to the addr direction | doubly-controlled SWAP |
| `FetchData{d}` | leaf nodes with addr==L/R select a cell (L→even, R→odd), `data ^= memory[cell][d]` | doubly-controlled X on the leaf's d (per memory bit) |

Depolarizing noise operators (stage 1 unitaries / stage 2 channels; the `coef` floor rules mirror
`SubBranch::run_depolarizing`):

| Subsystem | floor rule | Unitary (stage 1) | Channel (stage 2) | OriginIR-ext text |
| --- | --- | --- | --- | --- |
| data bit (odd pos) | floor(3·coef) ∈ {X, Z, XZ} | direct gates | `Depolarizing q,(p)` (uniqc semantics = apply a uniform non-trivial Pauli with probability p, aligned with the QRAM marginal semantics) | ✓ |
| addr-qutrit (even pos) | floor(8·coef) ∈ {A1, A2, A1², A2², four products} (Weyl family) | permutation (transposition conjugation) + controlled phase | 4×4 Kraus of the uniform mixture over the 8 Weyl elements (identity on the dead state) | ✗ (non-Pauli, goes through kraus2q) |

**mode c marginal probability**: each step the sampler draws `nerror ~ Binomial(N, p)` (N=2(2^L−1)) and places the errors uniformly on the N active positions in
`[0, N)` (after the closed-interval bug fix) → the marginal probability of each position is exactly p;
steps with `entangle_max=0` carry no noise.

## Three-Level Verification Ladder

1. **Stage 0**: gate sequence only, statevector backend, asserted equal to the QRAM noise-free `run_full` distribution.
2. **Stage 1 (the main trajectory mode)**: the schedules of the first `--exportruns` trajectories are exported one by one
   (`schedule_r{i}.json`); each is realized as a deterministic unitary per the floor rules (sampling already complete, no probabilities left),
   and asserted equal, case by case, to that trajectory's exact `run_full` distribution.
3. **Stage 2**: channels placed on the noise positions (mode b: the extracted positions of a single realization; mode c: all active positions × p),
   QuTiP density-matrix backend, compared against the distribution averaged over R trajectories. Channel simulation needs many trajectories to converge (as the user expected),
   and mode c is exactly that convention.

## Implementation-Level Fixes

Found and fixed during cross-checking (the three QRAM-Simulator ones are on the current experiment branch `experiment/qutrit-noise-correspondence`;
ctest 190/190 passing):

1. **`QRAMNode::rotate_A1` if fall-through** (`qram_branch_qutrit.h`): `if (addr==W) addr=R;`
   falls through into `if (addr==R) addr=L;` → the implementation performed W→L and R→L (non-injective, non-unitary), inconsistent with the commented three-cycle
   L→W→R→L; moreover `QRAMState::rotate_A1` inserts (R,0) for absent nodes while an explicit (W,d)
   goes through the fall-through — the same physical state (W,0) produced different outputs depending on bookkeeping. Changed to a clean three-cycle with early returns.
2. **`QRAMState::state_of` absent semantics** (`qram_branch_qutrit.cpp`): absent uniformly returned
   0==L, so phase-type noise operators wrongly added an ω phase to ground-state (absent≡W) nodes. Changed to return 0 for odd positions and
   W for even positions. The qubit architecture's `state_of` returns bool and has no such problem.
3. **Noise-position sampling closed-interval off-by-one** (`time_step.cpp`, both `noise_one_step` overloads):
   `uniform_int_distribution(0, nqubits)` includes the endpoint, so index nqubits (a node outside the subtree) could be
   drawn. Changed to `uid(0, nqubits-1)` with a skip when nqubits==0.
4. **uniqc's `DensityOperatorSimulatorQutip.kraus2q`**: `Qobj(reshape(4,4))` produces
   dims=[[4],[4]], and qutip's `expand_operator` refuses to map to 2 targets. Fixed
   (`dims=[[2,2],[2,2]]`); upstream PR: IAI-USTC-Quantum/UnifiedQuantum#130
   (local checkout on the `fix/qutip-kraus2q-dims` branch; full test suite 2520 passed).

Fix verification: the exporter's **model-semantics trajectories** (pure state functions + the commented three-cycle) and the native `run_valid_branches`
are now bitwise identical (the native and model double references in reference.json are equal), kept as regression cross-validation.

Also confirmed (library unchanged): **memory values must fit within data_size bits** — `get_fidelity` XORs the raw
memory value directly, and out-of-range inputs are silently scored as zero (`set_memory_random`'s sampling range already follows the convention); the exporter
now validates inputs, and the fidelity reference-quantity computation also gained mask hardening.

## Results (addr=2, data=1, memory=[0,1,1,0], seed=20260921)

Distribution correspondence and fidelity conventions (500 trajectories; 200 for p=0.3):

| p | mode c F_cls | TVD | F_quantum(channel) | overlap (tree-sensitive, no post-selection) | nopost (tree-insensitive) | incoh (branch projection) | post (original avg_fid) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0.005 | 0.9997 | 0.0038 | 0.9295 | 0.9395 | 0.9559 | 0.9785 | 0.9497 |
| 0.02 | 1.0000 | 0.0019 | 0.7458 | 0.7316 | 0.8177 | 0.9240 | 0.7847 |
| 0.05 | 0.9990 | 0.0226 | 0.4779 | 0.4283 | 0.5941 | 0.8075 | 0.5232 |
| 0.1 | 0.9991 | 0.0249 | 0.2250 | 0.2005 | 0.3870 | 0.7075 | 0.3246 |
| 0.3 | 0.9966 | 0.0542 | 0.0120 | 0.0097 | 0.1478 | 0.5850 | 0.1394 |

(F_cls = classical distribution fidelity (Σ√pq)², vs the native trajectory average; vs the model reference it agrees to the third decimal place.
`results/*/driver_summary.json` has all conventions.)

## Fidelity Convention Evidence

An empirical answer to "why can't `sample_and_get_fidelity` be compared directly with fidelity / should they agree statistically":

1. **Statistical consistency holds**: the trajectory average under the no-post-selection, tree-sensitive convention,
   `E|⟨ψ_ideal|ψ_traj⟩|²` (avg_overlap_fid), agrees with the channel-level `⟨ψ_ideal|ρ_mode-c|ψ_ideal⟩`
   (F_quantum) at all noise strengths (the two table columns; the difference is within the 500-trajectory MC error). Algebraically this is the
   identity `⟨ψ|E[|ψ_traj⟩⟨ψ_traj|]|ψ⟩` — holding proves **channel ensemble = trajectory ensemble** (at the state level).
   The large gap previously observed (0.21 vs 0.75) was a memory out-of-range artifact and has been eliminated.
2. **Branch coherence vs branch projection: the means differ** (incoh − overlap ≈ 0.19 @p=0.02). Reason:
   events hitting the **shared root node** give multiple address branches **correlated** phases (the same Weyl operator yields different ω powers on the
   node states of different branches) — they are not independent zero-mean, so the cross-branch term `E[G_a G_b*]` does not vanish.
   "Adding the branch coherences together" is correct as a single-shot statistic (when the address register is not measured and readout is coherent),
   but what it induces is a **convention difference (here, a lower mean), not merely variance reduction**.
3. **Tree-sensitive vs tree-insensitive** (nopost − overlap ≈ 0.09 @p=0.02): F_a also counts amplitudes that are "bus-correct but with tree residue";
   the ideal-state inner product requires the tree to return to the ground state.
4. **Per-shot tree post-selection** (post vs nopost, a further ~0.03 drop): `sample_output` samples the tree configuration and censors
   inconsistent subbranches, replacing cross-configuration coherence terms with a P(τ)-weighted incoherent average — the mean shifts slightly (not just the variance).

## renormalize Discussion

With Depolarizing the whole chain is CPTP/unitary trajectories: the QRAM-side survival rate (`get_normalization_factor`) and the uniqc-side
trace(ρ) are identically 1, and **the two conventions (no renormalization / renormalization) give identical numbers**. Under Damping the two conventions truly diverge — see the next section.

## Damping Path Verification (M3-1)

### Audit and Interception

`noise_t{Damping: γ}` is split each step into several `Damp_Full{pos,γ}` (position sampling as for other types) plus the end-of-step global
`Damp_Common{γ}`:

| Operator | Semantics | Encoded side |
| --- | --- | --- |
| `Damp_Common` (qram_branch_qutrit.cpp:581) | amplitude ×√(1−γ) on every excited degree of freedom of the whole tree, deterministic and non-unitary | single-Kraus K0 (qutrit `diag(1,√(1−γ),√(1−γ),1)`; 1q for the data bit) |
| `Damp_Full` (qram_circuit_qutrit.cpp:400/:642) | **second sampling**: `prob_damp[k]` accumulates according to branch weights, and a single `uniform01×norm²` draw selects {L jump, R jump, no jump}; a jump censors the subbranches of non-target levels and then collapses the basis state, with no √γ factor | fixed-outcome single Kraus `\|W⟩⟨L\|` / `\|W⟩⟨R\|` (data bit `\|0⟩⟨1\|`) |

Interception of the "second sampling": the exporter replays it with a replica execution (aligned RNG draw by draw, entirely through public APIs);
the Damp_Full entries in `schedule_r{i}.json` carry an `outcome` field; the replica-vs-native per-trajectory distributions are asserted identical
(built-in validation). Driver Stage 1-d replays on the QuTiP backend: gates + per-step K0 (whole tree) + fixed-outcome jump
single Kraus → sub-normalized density matrix, whose diagonal is **cross-checked exactly (1e-16)** against that trajectory's sub-normalized QRAM distribution.

### Results (γ scan, 500 trajectories; mixed = depol 0.02 + γ 0.02)

| γ | survival | nojump trace | faithful trace | faithful F_cls(normalized)/TVD | textbook full F_cls(normalized)/TVD | F_quantum(faithful) vs avg_overlap_fid |
| --- | --- | --- | --- | --- | --- | --- |
| 0.001 | 0.979 | 0.982 | 0.976 | 0.999 / 0.010 | 0.998 / 0.015 | 0.840 ≈ 0.865 |
| 0.01 | 0.796 | 0.831 | 0.788 | **0.9998 / 0.009** | 0.994 / 0.041 | **0.651 ≈ 0.657** |
| 0.05 | 0.317 | 0.393 | 0.306 | **0.999 / 0.017** | 0.956 / 0.157 | **0.213 ≈ 0.206** |
| 0.2 | 0.044 | 0.022 | 0.024 | 0.977 / 0.103 | 0.931 / 0.206 | 0.005 ≈ 0.002 |
| mixed | 0.628 | 0.684 | 0.614 | **0.999 / 0.013** | 0.985 / 0.079 | 0.425 ≈ 0.430 |

### Conclusions

1. **Trajectory-level verification fully passes**: every random case, second sampling included, is reproduced exactly at the circuit level (Stage 1-d,
   8 cases × all γ and the mixed configurations, 1e-16) — the damping path's execution semantics are pinned down completely.
2. **The textbook AD channel is not their model** (the pre-registered suspicion is confirmed, with the magnitude corrected to O(γw)): the TVD of the normalized distribution of the per-degree, per-step
   `K0+K_L+K_R` (TP) channel grows to 0.21 with γ; the no-jump survival rate (nojump trace)
   is above their survival for γ≤0.05 (over-damping direction) but falls below it at γ=0.2 — a structural difference, not a simple scaling.
3. **The faithful mirror** (ρ-dependent mapping: whole-tree K0 weight `1−γw/S` + jump mixing only on active positions (nodes < 2^L−1)
   `√(γw_k/S)·M_k`, with no √γ amplitude factor) reproduces the normalized distribution at all γ (F_cls ≥ 0.977)
   and the survival rate (within 3% for γ≤0.05; 47% off at γ=0.2 — the approximation of taking the weights of the nonlinear mapping on the averaged
   ρ fails as γ grows).
4. **Statistical consistency holds under the correct channel**: `F_quantum(faithful) ≈ avg_overlap_fid` throughout
   (last column of the table above) — the "channel ensemble = trajectory ensemble" identity also holds in the non-unitary case, provided one uses
   their (ρ-dependent) channel rather than the textbook AD.
5. **renormalize convention divergence** (the original requirement of this experiment): `F_cls(not normalized)` drops from
   0.98 to 0.04 over γ=0.001→0.2 — the QRAM-side sub-normalized distribution (carrying survival weights) cannot be compared directly with the TP channel (trace=1);
   only after normalizing (dividing each by its own trace/survival) are they comparable. The nojump mode gives the circuit-level counterpart of the "no-jump survival probability"
   (trace ↔ the textbook no-jump probability, not their model's survival).

### Limitations

- The faithful mirror is a ρ-dependent nonlinear mapping with weights taken on the averaged ρ (Jensen-type bias) — the trace deviation grows at
  high γ; an exact average over per-trajectory weights would require MC integration (i.e., going back to the trajectory ensemble itself).
- Damp_Full's jump positions are only in the active subtree (`layer_entangle_max`), while Damp_Common's K0
  acts on the whole tree — the faithful mirror aligns the two separately; Stage 1-d's exact match proves this reading correct.
- The `run_normal` pruning path and the analytic multiplier (`_get_multiplier_impl_qutrit`/`QRAMLoadFast`)
  were not covered in this round (next milestone).

## qubit-based QRAM Verification (M3-1b, Depolarizing + Damping Complete)

`QubitCorrespondenceExporter` (target `Experiment_QRAM_ChannelCorrespondence_Qubit`) plus
the driver's qubit path reuse the same three-level ladder. Semantic differences from qutrit (audit highlights):

- **Encoding**: node v = 2 ordinary qubits (a = addr+data+2v, d = +1; position = v·2+lr, consistent with the noise-position
  convention); addr=2 → **9 encoded qubits** (12 for qutrit).
- **FetchData is phase kickback**: the leaf's data×addr selects a cell and applies a −1 phase; combined with the real H on all bus bits from `run_hadamard` at `CopyIn{0}` /
  `CopyOut{last}` — the net semantics is still `bus_out = bus_in ⊕ m[a]`
  (verified bitwise at Stage 0). The gate translation includes the `H` primitive; `SwapInternal` is emitted unconditionally
  (SWAP is the identity on ground-state nodes); `cswap` is singly controlled by the addr bit.
- **`run_bitphaseflip` fixed to a Y flip** (|0>→−|1>, |1>→|0>, ZX = iY, unitary — consistent with the
  qutrit architecture's odd-position semantics). The old implementation was a rank-1 non-unitary decay operator |1>→−|0> (equivalent to −K1); it caused
  sub-normalized Depolarizing trajectories, over-counted distributions, and `sample_output` zero-norm crashes; after the fix
  **Depolarizing is norm-preserving again (survival=1)**, and the `depol_textbook` and `depol_faithful`
  modes agree numerically (Y and ZX differ only by a global phase → the same channel).

Results (500 trajectories; Depolarizing's textbook/faithful are equivalent after the fix, so only one column):

| Configuration | stage1 per case | F_cls(normalized)/TVD | trace(faithful) vs survival | F_quantum(faithful) vs avg_overlap_fid |
| --- | --- | --- | --- | --- |
| depol p=0.005→0.3 | 8/8 exact | **0.9991–0.9999** / ≤0.024 | 1.000 vs 1.000 | 0.923/0.940 … 0.018/0.012 ✓ |
| damp γ=0.001→0.2 | 8/8 exact | **0.9967–0.9999** / ≤0.049 | 0.984/0.984 … 0.127/0.137 (difference <1.5%) | 0.837/0.860 … 0.057/0.058 ✓ |
| mixed (p=γ=0.02) | 8/8 exact | **0.9998** / 0.007 | 0.730 vs 0.737 | 0.508 ≈ 0.512 |

The textbook AD channel's (full mode) over-damping deviation direction matches qutrit (a structural difference, not a scaling).

## Limitations and Follow-ups

- Cross-validation of the `run_normal` pruning path and the analytic multiplier (`_get_multiplier_impl_qutrit`/`QRAMLoadFast`) (M3-2).
- Input validation: `set_noise_models` now requires probabilities ∈[0,1] and Damping γ<1 (γ=1 annihilates all excitations in one step,
  the only remaining zero-norm path after the bitphaseflip fix, now excluded at the input side).
- addr=3 (25 encoded qubits, beyond the practical size for a QuTiP density matrix) and comparison against the qubit architecture.
- Feed the bridge back into PySparQ bindings (`set_noise_models` + schedule export), eliminating the JSON hop.
- Upstream follow-up: the three QRAM-Simulator fixes can be packaged into standalone commits; the uniqc kraus2q fix **does not wait for an upstream
  release** — the local UnifiedQuantum checkout is pinned to the `fix/qutip-kraus2q-dims` branch
  (PR #130 is open, pending merge; switch back to main once it merges).
