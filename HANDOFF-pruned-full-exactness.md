# HANDOFF — pruned-vs-full exactness repair (qubit architecture)

Status: **fix implemented, compile-verified, regression battery NOT yet re-run.**
Branch: `fix/pruned-full-exactness` (commit `30add10`), pushed to `github`
(IAI-USTC-Quantum/QRAM-Simulator). The first action item below is to run the
battery; everything else follows from its outcome.

---

## 1. The contract

`QRAMCircuit::run(FULL_VER)` evolves every branch group explicitly.
`run(NORMAL_VER)` is the paper's pruning algorithm: it evolves only the groups
inside the marked bad range plus one reference good group, and reconstructs
the rest at output time. The two modes are driven from identical RNG seeds and
identical sampled noise histories, so **NORMAL_VER must reproduce FULL_VER
exactly, seed by seed** — per-address weights to ~1e-12, sampled fidelity to
~1e-9. (Bitwise equality is unreachable: the two modes accumulate the same
mathematics in different summation orders, ~1e-15 rounding.)

This was violated on 14 of 1000 perf_scan trajectories. This handoff documents
the root cause, the fix now on the branch, one rejected fix attempt that must
not be reintroduced, and the exact verification sequence.

## 2. Architecture facts (minimum needed to reason about the fix)

- `BranchGroup` = all input branches at one address; the pruning unit.
- `prepare_bad()` (qram_circuit_qubit.cpp ~135): `valid_branch_group_view` =
  bad groups + the FIRST good group (the reference). Other good groups are
  never evolved; their indices live in `good_branch_group_ids`.
- `materialize_good_branches()` (~625): XOR-mirror reconstruction — every good
  branch's final component list equals the reference branch's, with
  `data_bus ^= delta` and `amplitude *= sqrt(relative_multiplier)`
  (`relative_multiplier = (1-gamma)^(exponent difference)`, address-level
  K0-attenuation counter). Justification: mirror theorem, Theorem 3 in
  `docs/sphinx/source/en/paper/qubit_qram_pruning.md`.
- Mid-run nominal estimates (used BEFORE materialization):
  - `BranchGroup::get_prob()` — good && !predicted ⇒ input weight sum;
  - `BranchGroup::get_fidelity()` — good && !predicted ⇒ reference's;
  - `get_normalization_factor_with_damping()` (~566) — adds
    `ref_unit_norm * relative_multiplier * g_input` per good group;
  - `run_damp_full()` (~400) — jump-probability estimate with the same shape.
    All four read through the reference group, so they track the reference's
    surviving norm automatically.
- `run_damp_full` roulette: exactly ONE `uniform01` draw per `Damp_Full`
  operation, in both modes. On fire, `Branch::run_damp_full(qubit_id, k)`
  (qram_branch_qubit.cpp ~464) projects each component onto the jumped
  subspace: components with that node in |0⟩ are DELETED (weight → exactly 0),
  excited ones are kept and reset to |0⟩. Applied to view groups only.

## 3. Root cause (numerically confirmed)

The mirror theorem says every good branch shares the reference branch's
component-wise tree configuration. Hence a fired K1 jump acts on the good
branches exactly as on the reference:

- **Reference survives the fire** (some component excited at the fired node):
  good branches excited there decay in lockstep — the mirror amplitudes
  already carry this; idle ones are annihilated together with the reference's
  matching components. Reconstruction is exact. Empirically: all pre-fix
  fired-and-survived trajectories agreed to machine precision.
- **Reference fully annihilated**: every good branch is annihilated too (they
  share the reference's configurations, so the same projection empties them).
  Full mode leaves them at exactly zero weight. The reconstruction, however,
  silently skipped materialization (`ref_branch == nullptr → continue` inside
  the per-group loop) and left all other good groups at nominal weight.

That second case is the entire bug. A replay of the 14 failing seeds
(`QubitPaperExceptionHunt.cpp`) had already ruled out marking under-coverage:
zero marked-range escape, marked-branch weights identical between modes, and
a trace collapse on the full-mode side (e.g. 262.7 → 7.7) — the signature of
reference/group annihilation, not of prediction error.

## 4. Fix history — including the rejected attempt

### Attempt 1 (REJECTED — do not reintroduce)
A blanket mid-run marker: on any fired jump, mark ALL un-evolved good groups
annihilated. Rationale seemed plausible ("good addresses lie outside the fired
node's marked subtree, so the jumped node is idle |0⟩ in every good branch").
It fixed the 14 historical seeds bitwise (dW = 0, dF = 0) and passed the whole
121-case battery — but broke 6 perf_scan trajectories in which **the reference
survived the fire while some good branches are excited at the fired node**
(routing bit 1 inside their exposure window): those decay in lockstep with the
reference and must stay predictable. Deviations up to 6.6e-1. New mismatching
seeds (eps=1e-3, uniform-branch convention):
`7744644691974779904` (n=10 and n=12), `1183008555922881536`,
`3227135189013113856`, `3287329390135220224`, `4366419402265368576` (n=12).
This is recorded as the KNOWN TRAP comment in `run_damp_full`.

### Attempt 2 (CURRENT — commit 30add10)
Detection at materialization time only:

```cpp
// materialize_good_branches(), before the per-group loop:
Branch* ref_branch = nullptr;
for (auto& rb : ref.branches)
    if (rb.system_states_sz > 0) { ref_branch = &rb; break; }
if (ref_branch == nullptr) {
    for (size_t id : good_branch_group_ids)
        branch_groups[id].mark_annihilated();   // zero weight everywhere
    return;
}
```

`mark_annihilated()` sets `BranchGroup::annihilated`, clears all branch states
(`branch_probs` untouched), and `get_prob()`/`get_fidelity()` return 0 for
annihilated groups, so sampling, the normalization sum, and the fidelity sum
all exclude them — exactly like full mode's empty groups. After reference
death the mid-run estimates also vanish on their own (`ref.get_prob() == 0`
makes `ref_unit_norm == 0`), so no mid-run gating is needed; the `annihilated`
skips at the mid-run sites are defensive only.

Supporting changes in the same commit: `fired_jump_count` public counter on
`QRAMCircuit` (reset in `initialize_system`, incremented on each fired
roulette; diagnostic — lets tests assert the fired-jump path was exercised and
that both modes fired identically), plus the regression test
`Experiments/QRAM/QubitPaper/QubitPaperExactTest.cpp` (ctest name
`QubitPaperExactness`), and `build_paper.bat` now sets `QRAM_BUILD_TESTS=ON`
and builds three targets.

Files touched: `QRAM/src/qram_circuit_qubit.cpp`, `QRAM/src/qram_branch_qubit.cpp`,
`QRAM/include/qram_circuit_qubit.h`, `QRAM/include/qram_branch_qubit.h`,
`Experiments/QRAM/QubitPaper/QubitPaperExactTest.cpp` (new),
`Experiments/QRAM/QubitPaper/CMakeLists.txt`.

### What is and is not verified
- Compile: yes (all three targets, MSVC Release).
- Attempt 1 vs the 121-case battery: fully passed (that run is void for
  attempt 2, but it validated the battery itself, the RNG-parity assertion,
  and the measurement methodology).
- Attempt 2 vs the battery: **NOT RUN — this is the immediate next step.**
- Attempt 2 is expected to pass the 14 historical seeds (reference-death case,
  same semantics as attempt 1 for those) AND the 6 attempt-1 casualties
  (reference survived → materialize proceeds normally). Both classes are
  pinned in the battery as `hist` and `scanhist`.

## 5. First actions, in order

1. **Rebuild** (Windows, Git Bash): run `build_paper.bat` in the repo root.
   It calls vcvars64 (VS2022 Community on E:), configures Ninja Release into
   `build/paper-release` with `-DQRAM_BUILD_TESTS=ON`, and builds
   `Experiment_QRAM_QubitPaper`, `Experiment_QRAM_QubitPaperExactTest`,
   `verify_noisy_simulation`. Note: `build_paper.bat` is gitignored (it
   hardcodes a local VS path). Caveat: temporary .bat wrappers must live
   inside the repo directory — vcvars64 silently fails when invoked from /tmp.
2. **Run the battery**: `ctest` in `build/paper-release` (or run
   `bin/Experiment_QRAM_QubitPaperExactTest.exe` directly; ~90 s, 155 cases).
   Expect: all cases exact (dW ≤ ~1e-14, dF ≤ ~1e-15 observed under attempt 1
   for the classes it did not break), fired-jump counts equal in both modes
   per case, total fires ≥ 50.
3. **Re-run the perf scan**: `bin/Experiment_QRAM_QubitPaper.exe perf results`
   from `Experiments/QRAM/QubitPaper` (~19 min; writes `results/perf_scan.csv`;
   the pre-fix copy is preserved as `results/perf_scan.csv.prefixfix-bak`).
   Then verify with a short Python script:
   - all 1000 (eps, n, runseed) pairs agree at |ΔF| ≤ 1e-9;
   - **full-mode rows must be byte-identical to the pre-fix backup** — the fix
     only touches pruned-mode reconstruction and a counter, so any full-row
     change means the fix leaked into full mode (bug);
   - only fired-jump trajectories' normal rows may shift, converging to the
     full values.
4. **If the battery fails**: the primary suspect is the assumption recorded in
   the `materialize_good_branches` comment — *reference death is total and
   shared (no good branch survives a fire that killed the reference)*.
   Symptom: nonzero full-mode weight on unmarked addresses. Diagnostic path:
   replay the failing seed (pattern-match `QubitPaperExceptionHunt.cpp`),
   dump per-address weights, the fired `(pos, step)` list (scheduled `Damp_Full`
   ops are recoverable from `qram.operations`; which ones FIRED is only
   observable via the reference's survival), and the reference group's
   per-branch state counts. If a good branch genuinely survives a
   reference-killing fire, the mirror premise itself fails and per-group
   projection modeling at materialization becomes necessary: node 2i+1
   carries routing bit i, is excited iff bit i of the group's address is 1
   and the step lies in the exposure window [2i+1, out(2i+1)) — the same
   window the damping-multiplier counter uses (`_get_multiplier_impl_qubit`,
   time_step.cpp ~475).

## 6. Invariants and traps (condensed)

- Never reintroduce a mid-run blanket kill (KNOWN TRAP in `run_damp_full`).
- Never evolve the good groups "to be safe" — that erases the algorithm's
  speedup; correctness must come from bookkeeping.
- One `uniform01` per `Damp_Full` op in both modes; keep it that way. Any new
  randomness inside the run loop must be mode-independent or the RNG streams
  diverge and seed-level comparison becomes meaningless.
- `mark_annihilated` must not touch `branch_probs` (input weights are reused
  by `get_prob`'s nominal path for non-annihilated groups).
- In tests, compare per-address weights AFTER `sample_and_get_fidelity()`
  (post-collapse, post-normalization): before sampling, the pruned mode's good
  groups legitimately still hold un-materialized inputs (a false ~1e-3 "gap"
  was produced this way once — the comment in the test's `run_case` explains).
- Battery-level fire coverage (total fires ≥ 50) guards against testing only
  no-fire paths; per-case `require_fire` is deliberately used only for
  seeds known to fire (eps=1e-2 grids fire often enough that a battery-level
  check suffices there).
- Qutrit architecture: audited, NO gap — its good branches are live pointer
  aliases of the reference, which IS in the view, so fired jumps propagate
  automatically; there is no materialization step (the XOR-mirror TODO in
  `qram_circuit_qutrit.cpp` ~236 is commented out and MUST STAY disabled —
  porting it naively would reintroduce this exact bug). All paper qutrit
  numbers use full mode or jump-free trajectories.

## 7. Work beyond the code fix

Paper repo: `E:\git\QRAM-Simulator-dev\Qubit-based_QRAM_Simulator\main.tex`
(NOT a git repo; one-line-per-paragraph LaTeX style; pushed to USTC Overleaf
via `olcli upload main.tex`; compile-check with pdflatex/TeX Live 2024).

- `main.tex:482` (end of the exactness paragraph): the current sentence —
  deviations "trace back to … the family divergence extends slightly beyond
  the marked range … hardening the marking rule … was not needed" — is
  **falsified**. The true mechanism is the reconstruction-side bookkeeping gap
  fixed here. Rewrite: deviations originated in trajectories where a fired
  jump annihilated the reference good branch and the reconstruction failed to
  propagate the annihilation to the predicted branches; the simulator now
  detects reference death at materialization, and seed-by-seed exactness is
  enforced unconditionally by a regression test.
- Seed-by-seed paragraph (~line 468 region) and the exception counts: after
  the scan re-run, exceptions should be zero; update all numbers (1598/1600,
  58/1600, 6×10⁻², 1.2×10⁻³ are pre-fix values). Appendix `app:equiv`
  (`tab:equiv`) is produced by `QubitPaperScan equiv` (seeds 880000/880007/
  880014); re-run to refresh and confirm machine-precision agreement including
  fired trajectories.
- Verify Sec IV.D (jump sampling) wording still matches: the paper already
  asserts good branches carry zero weight on jump trajectories — the theory
  was right; the code now matches it.
- Open, independent item — E[B] recount: under the climb marking rule each
  branch has Θ(n²) marking sources, giving E[B] = Θ(n³p)·D, while
  `main.tex:451` derives O(n²p)·D from "only O(n) nodes on or adjacent to its
  routing path" (falsified by measured bad-fraction log-log slope 2.70 vs
  subtree model 1.92). Lines ~451 and ~478 need rewriting; check whether the
  abstract's "retains the qutrit-scheme scaling" needs a one-extra-factor-n
  caveat for the qubit encoding.
- Dangling cross-reference `main.tex:474` ("the double-counted input weight of
  Sec.~\ref{sec:algorithm}" — that content does not exist in Sec V).
- Optional: `QubitPaperVerify.cpp` prediction side is still type-gated
  (intentional — it is a damage measurement, not the marking rule).

## 8. Quick file map (post-fix, line numbers approximate)

| Where | What |
|---|---|
| `QRAM/src/qram_circuit_qubit.cpp` ~400 | `run_damp_full`: roulette, KNOWN TRAP comment, `++fired_jump_count` |
| `QRAM/src/qram_circuit_qubit.cpp` ~135/163 | `prepare_bad` / `prepare_all` (view construction) |
| `QRAM/src/qram_circuit_qubit.cpp` ~566 | `get_normalization_factor_with_damping` (defensive skip) |
| `QRAM/src/qram_circuit_qubit.cpp` ~625 | `materialize_good_branches` — **THE FIX** (reference-death detection + FIX STATUS/ASSUMPTION comment) |
| `QRAM/include/qram_circuit_qubit.h` ~55 | `fired_jump_count` |
| `QRAM/include/qram_branch_qubit.h` ~335 | `BranchGroup::annihilated` (+ OPEN RISK note), `mark_annihilated` decl |
| `QRAM/src/qram_branch_qubit.cpp` ~498/512/525/556 | `reset`, `mark_annihilated`, `get_fidelity` gate, `get_prob` gate |
| `Experiments/QRAM/QubitPaper/QubitPaperExactTest.cpp` | regression battery (155 cases, 5 batteries, 2 input conventions) |
| `Experiments/QRAM/QubitPaper/QubitPaperExceptionHunt.cpp` | seed replay / family-audit diagnostic tool |
| `results/perf_scan.csv` (+ `.prefixfix-bak`) | scan output (gitignored); backup = pre-fix state |
