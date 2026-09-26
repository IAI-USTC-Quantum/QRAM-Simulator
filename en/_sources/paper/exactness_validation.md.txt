# Pruned/full exactness validation

`QRAMCircuitQubit` provides full evolution (`FULL_VER`) and pruned evolution
(`NORMAL_VER`). For the same input, memory and sampled noise history, both
modes must implement the same trajectory. Pruning reconstructs predictable
groups from a reference group; it does not introduce a truncation or a
probabilistic approximation to their evolution.

This guide describes the verification protocol and its reproducible test
program, `Experiment_QRAM_QubitPaperExactTest`.

## Equivalence contract

Construct each circuit with the same configuration seed, then reset the
trajectory seed immediately before running each mode. Check the following
quantities separately:

| Quantity | Comparison |
|---|---|
| Sampled operations | Identical operation history |
| Random generator | Identical state after evolution and after output sampling |
| Damping jumps | Equal fired-jump counts |
| State before measurement | Match all input-weighted complex components after materializing predicted groups |
| Sampled tree | Identical residual tree configuration |
| State after measurement | Match all normalized input-weighted complex components |
| Address probabilities | Include each input branch's probability weight; total probability is one |
| Fidelity | Compare the same definition against the same ideal output |

Components are keyed by `(address, input bus, output bus, residual tree)`.
Keeping the input-branch label tests each represented trajectory-map column,
so errors in different columns cannot cancel in a single scalar fidelity.
The component and address-probability tolerances are $10^{-12}$; the fidelity
tolerance is $10^{-9}$. These are numerical assertion thresholds for different
floating-point accumulation orders, not approximation parameters of pruning.

Materialize predicted groups **before** inspecting the pre-measurement state.
Their stored unevolved input components are not output states. Match the
post-measurement state only after both modes sample the tree and normalize.

## Shared-reference invariants

A damping jump projects the reference's components. If some components
survive, their amplitudes remain the basis for reconstruction. A jump alone
therefore does not justify clearing every predicted group.

If no reference component survives, every group represented by that shared
reference must contribute zero probability and fidelity.
`materialize_good_branches()` marks these groups `annihilated` and clears
their current components. It preserves `branch_probs`, which defines the
input reused by the next query. `reset()` restores the trajectory state and
clears the flag. During evolution, predicted population estimates already
inherit the reference's surviving norm.

Each nonempty damping candidate layer consumes one joint auxiliary draw in both modes; an empty candidate layer consumes none. The actual joint jump sets must agree, as well as the candidate operation history. See [joint damping](joint_damping.md) for the physical-channel correction and changed seed mapping.
The qubit materialization rule is separate from the qutrit reference-alias
representation; the two representations should be verified independently.

## Test coverage

| Entry | Coverage |
|---|---|
| Default invocation | 155 pinned, stress and zero-noise cases; both full-address zero-bus and sampled uniform-branch inputs |
| Fixed-history fixtures | Reference death, reference survival, and reuse of the same circuit after projection |
| `--channels` | 120 cases: Damping, Depolarizing, BitFlip, PhaseFlip and BitPhaseFlip; $n=3,5$, $k=1,2,4$; uniform/nonuniform input weights |
| `--perf-shard i count` | The 1000-pair performance grid: $k=3$, $n=4,6,8,10,12$, $\varepsilon=\gamma\in\{0,10^{-5},10^{-4},10^{-3}\}$, 50 seeds per point |
| `--case n eps seed [uniform]` | One mixed-noise trajectory; optional sampled uniform-branch input |

The fixed-history fixtures exercise both outcomes of reference projection
without depending on a platform-specific noise distribution. The default
155-case battery additionally requires at least 50 actual fired jumps.

C++ standard distributions are not required to produce identical samples
across standard libraries. Compare full and pruned modes within the same
toolchain. Pinned stochastic fixtures require their specific MSVC firing
outcomes only on MSVC; every platform runs their numerical comparisons and
the fixed-history projection fixtures.

## Build and run

From the repository root, with a C++17 compiler and CMake available:

```bash
cmake -S . -B build-exact -DCMAKE_BUILD_TYPE=Release \
  -DCACHED_REGISTER_SIZE=8 -DQRAM_BUILD_TESTS=ON \
  -DQRAM_BUILD_EXPERIMENTS=ON
cmake --build build-exact --parallel 8
OMP_NUM_THREADS=1 ctest --test-dir build-exact --output-on-failure
./build-exact/bin/CorrectnessTest
```

CTest runs `QubitPaperExactness`, `QubitPaperExactnessChannels` and the existing
`verify_noisy_simulation` circuit-level check. Every assertion failure returns
a nonzero exit status. The default exactness test includes the fixed-history
fixtures and aggregate fired-jump coverage check.

For a Windows multi-configuration generator, build with `--config Release`
and use `ctest --test-dir build-exact -C Release --output-on-failure`.
Executables are under `build-exact/bin/` with the `.exe` suffix.

Run individual checks or the entire performance grid:

```bash
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest --fixtures
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest --channels
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest \
  --case 8 0.001 9208654621621431296
./build-exact/bin/Experiment_QRAM_QubitPaperExactTest --perf-shard 0 1
```

To distribute the grid across eight independent processes, run
`--perf-shard i 8` for every `i` from 0 through 7 and require all eight exit
codes to be zero. `--battery-shard i 8` similarly partitions the 155-case
battery. Sharded runs do not perform the default battery's aggregate
fire-coverage check or fixed-history fixtures; run the default test as well.
Use separate processes because each process owns a global simulator RNG.

## Paper data and a stable full-mode baseline

Generate the original paper data products separately from component-level
tests:

```bash
./build-exact/bin/Experiment_QRAM_QubitPaper perf build-exact/validation
./build-exact/bin/Experiment_QRAM_QubitPaper equiv build-exact/validation
```

`perf_scan.csv` contains 2000 rows, one full and one normal row for each of
1000 `(eps, n, traj, runseed)` keys. Require complete pairs and compare their
fidelities. This CSV comparison supplements the component-level test; equal
fidelities alone do not establish state equality.

When changing reconstruction, build a separate reference revision with the
same compiler and configuration and compare its full-mode rows with the new
full-mode rows. Match `arch`, `eps`, `n`, `traj`, `runseed`, `version`, `fid`,
`states` and `branches`. Exclude `time_run_ms` and `time_sample_ms` from equality
checks because wall-clock timing is not deterministic.

Run performance measurements under controlled machine load. Timings collected
while validation processes compete for resources should not replace the
paper's runtime benchmarks. Preserve the code revision, compiler, build flags,
input convention and seeds with each data set.

## Validated numerical reference

The 2026-09-26 CPU Release validation used GCC 14.4.0/libstdc++, CMake 4.4.3,
Ninja, `CACHED_REGISTER_SIZE=8` and one OpenMP thread per process.

| Check | Result |
|---|---|
| Default seed battery | 155/155 pairs; 68 fired jumps |
| Individual-channel cases | 120/120 pairs |
| Performance grid | 1000/1000 pairs; 64 fired jumps |
| Maximum component difference before measurement, performance grid | $8.47\times10^{-16}$ |
| Maximum component difference after measurement, performance grid | $6.94\times10^{-16}$ |
| Maximum fidelity difference, performance grid | $8.99\times10^{-15}$ |
| Maximum physical address-probability difference, 275 seed/channel cases | $2.78\times10^{-16}$ |
| Full-mode physical CSV rows against reference revision `d91e4f0` | 1000/1000 identical |
| Appendix equivalence grid | 12/12 pairs; maximum $|\Delta F|=2.0\times10^{-15}$ |
| Registered CTest suite | 3/3 passed |

Archive generated CSV files, summary metrics and test logs under an ignored
build directory such as `build-exact/validation/`. CTest's detailed log is
`build-exact/Testing/Temporary/LastTest.log`.

These checks establish numerical equivalence to `FULL_VER` under the tested
channel and input conventions. They complement the mathematical shared-state
argument. Validating the physical channel against an independent formulation
is a separate obligation from validating the pruning transformation.
