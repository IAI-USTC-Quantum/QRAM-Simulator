# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.5.0] - 2026-11-09

### Added
- **Task-2.1 evidence replay as a standard product entry**
  (`qram_simulator.evidence`, `qram-simulator evidence`, GUI **任务2.1 四信道
  证据复算** panel): the fixed depth-20 / 1,048,576-record four-channel
  configuration with its sixteen same-depth theory anchors runs from the
  installed package — no repository checkout, no native internals exposed.
  The replay verifies its memory digest against the recorded v0.3.0 value,
  writes per-run raw JSONs plus the `qramsim.depth20-four-channel.v1`
  summary, and the GUI offers the recorded default configuration with a
  clearly labeled 300-second deadline and JSON download;
  `tools/depth20_four_channel.py` is now a thin wrapper over the packaged
  module (single authoritative path).
- **Offline import for `qramsim.addressing.v1` records**
  (`qram-simulator addressing --evaluate RECORD`): validates a record
  produced by this software or the declared uniqc consumer and recomputes
  the mitigated-addressing criterion offline (`qramsim.addressing-eval.v1`)
  — historical data stays historical, no trajectory is re-run, no hardware
  is touched. Task-4 test plan published as a bilingual guide (seeded 2-3
  layer / 4-8 record data, QPU-side data access via the offline standard
  I/O, existing `>=0.75` mitigated-fidelity records and fixed operation
  instructions).
- **GUI addressing panel** (**寻址缓解仿真（任务2.2参考）**): the task-2.2
  mitigation-simulation reference is now selectable, runnable and exportable
  from the GUI exactly like the CLI (same child-process service layer, same
  `qramsim.addressing.v1` output, GUI limits trajectories to 256 and noise
  to 0.1).

### Documentation
- New bilingual guides "Depth-20 four-channel evidence replay (task 2.1)"
  and "Task-4 QRAM test plan"; README/README_zh sections for the `evidence`
  CLI, the `--evaluate` offline import and the new GUI panels.

### Fixed
- `qram-simulator info` crashed with AttributeError when the common output
  block read `args.out`, an option the info parser never declared — broken
  since the standalone CLI shipped in v0.3.0. It now accepts `--out` like
  every other subcommand (pinned by a test).

### Engineering
- Normalize `bindings/python` and `tools` with the repository's pre-commit
  black/isort/flake8 configuration (whole files, no semantic change) so
  `black --check` / `flake8` pass across the Python tree; wheel and
  installed-product checkers pin the new `evidence.py` module.

## [0.4.0] - 2026-11-09

### Added
- **Ideal sparse wide-word queries** (`qram_simulator.SparseWordMemory` and
  `qram_simulator.word_backend`): logical address/data words of arbitrary
  width (64/65/127/128/129/256 bits and beyond) carried exactly across
  several 64-bit machine words — never truncated, never packed into a single
  host integer. `.xor` is the exact self-inverse classical read;
  `.apply_xor` executes the coherent query through real native X /
  controlled-X operations on frozen `(word_register_name, bit_index)`
  coordinates (SparQSim installs this single source as `pysparq/_qram_words.py`
  from its submodule). Conditions are `(little_endian_bits, expected_value)`
  pairs; zero controls coinciding with address bits restrict the query
  instead of re-addressing. Nominal address space (`2**width` labels), sparse
  records and allocatable capacity are kept explicitly distinct — no entity
  is allocated for the nominal space and the ideal path makes no
  noisy-routing claim. Ships with a real exact-statevector reference backend,
  a shared cross-repository verification fixture
  (`qram.word-query-fixture.v1`), and tests pinning the noisy native path's
  explicit 64-bit boundary.
- **Standalone `qram-simulator words` CLI and GUI wide-word panel**: seeded
  sparse wide memories, classical + executed-chain self-checks (per-basis
  classical match and query involution), `qramsim.words.v1` JSON with wide
  integers as decimal strings.
- **Noisy addressing & mitigation simulation reference**
  (`qram-simulator addressing`, `qramsim.addressing.v1`): 2-3 layer QRAM
  (4/8 seeded records) under all four channels with a uniform coherent
  addressing input, mean (address:bus) distributions, raw correct-addressing
  probability, and a calibrated symmetric bus-flip readout-mitigation
  simulation. The `>=0.75` criterion records value and operator under the
  direct task-sheet sub-item semantics; the JSON is the offline standard I/O
  for the uniqc side and the real-machine business stays frozen elsewhere.
- **Depth-20 four-channel evidence driver**
  (`tools/depth20_four_channel.py`): reproduces the task-2.1 configuration
  (20 layers, 1,048,576 seeded records — memory digest identical to the
  v0.3.0 record — four channels) on one identical input and checks explicit
  same-depth theory anchors: noiseless identity, the phase-flip vacancy
  model calibrated at a stress probability and *predicted* at p=1e-6, the
  X-type residue collapse separating full from logical fidelity, first-order
  depolarizing composition and linear-in-p deficits, and the damping K0
  occupation scale. Per-trajectory samples, seeds, formulas and provenance
  are embedded in the artifacts; no exhaustive-input or unified-confidence
  claim is made.

### Documentation
- New bilingual guide "Wide sparse word queries"; parallel/GPU support
  matrix with explicit re-test entries (CUDA compile-only vs untested
  runtime; CPU ensemble MPI is not distributed sparse-state slicing); the
  stale zh README GPU paragraph now matches the optional `QRAM_ENABLE_CUDA`
  build reality; README sections for `words` and `addressing`.

## [0.3.0] - 2026-11-09

### Delivery and builds
- Add the standalone `qram-simulator info`, `run` and `gui` interfaces with
  deterministic memory provenance, explicit selected input addresses, native
  noisy trajectories, trace checks and sampling intervals. Reports distinguish
  full-circuit overlap from a logical-query or acceptance certificate.
- Add explicit observable materialization without consuming measurement RNG,
  with pruned/full agreement covered across all four noisy channels.
- Restore CUDA core compilation on current main and validate CUDA12.8/13.3;
  honor actual C++20 builds. These compile checks do not certify GPU execution.
- Include exact vendored/binding/runtime notices in binary and source packages.
  Test each installed, repaired release wheel's notices, native computation,
  CLI and packaged local GUI before publication. Release tags trigger one
  upload; creating a GitHub Release does not duplicate publication.

### Fixed
- Propagate an empty qubit reference group to all predicted groups during
  materialization, preserving input probabilities for subsequent queries.
  Surviving reference components remain available for exact reconstruction.

### Validation
- Compare pruned and full complex components before and after tree measurement,
  together with sampled trees, RNG states, input-weighted probabilities and
  fidelity. Cover reference projection, circuit reuse, five noise channels,
  multiple bus widths and the 1000-pair performance grid.
- Publish the verification protocol and reproduction commands in the English
  and Chinese documentation.

### Added
- **Circuit-level QRAM baseline in the Python package** (`qram_simulator.baseline`,
  extra `pip install "qram-simulator[baseline]"`): a gate-level re-implementation
  of the qubit-architecture QRAM loading circuit on the UnifiedQuantum (uniqc)
  QuTiP density-matrix backend — 1-2 tree layers (density-matrix budget), noise
  channels only on the routing tree, schedule taken from the C++ `TimeStep`
  through the binding (single source of truth). Ships a packaged semi-quantitative
  comparison experiment (`python -m qram_simulator.baseline.compare`): noise-free
  bridge at machine precision, and over the full noise sweep F_cls ≥ 0.9988 /
  TVD ≤ 0.026 with `⟨ψ_ideal|ρ|ψ_ideal⟩` = `E|⟨ψ_ideal|ψ_traj⟩|²` within Monte
  Carlo error (see `bindings/python/qram_simulator/baseline/results.md`).
- **MPI ensemble parallel execution** (`qram_simulator.mpi`): trajectory-level
  parallelism for noisy QRAM Monte-Carlo ensembles under `mpirun` — ranks shard
  trajectories cyclically (`[rank::size]`), every trajectory is reseeded
  deterministically (`set_seed(seed + index)`) so aggregated results are
  independent of the process count (per-trajectory samples bit-identical; the
  histogram matches a serial run up to float summation order, ~1e-16), and
  rank 0 gathers and aggregates. Without mpi4py the same code path degenerates
  to a serial `SerialComm` run (importable and testable in MPI-less
  environments). Honest scope: single-node multi-process **CPU** ensemble
  parallelism — the bindings are CPU-only (`gpu_per_rank = 0` is recorded), it
  is not multi-GPU and not a distributed sparse-state partitioning. Tests in
  `bindings/python/test/test_mpi_ensemble.py` (logic layer runs everywhere;
  the `mpirun -np 4` integration test runs where mpirun + mpi4py exist).

### Changed
- **`QRAMCircuitQubit` binding surface for scripted experiments**: new
  `set_input_zerobus()` (uniform address superposition with bus = 0, the
  discriminating input), `get_output_distribution()` (marginal `addr:bus`
  distribution of the current run) and `get_fidelity_conventions()` (the three
  no-post-selection fidelity statistics — `overlap_fid` / `fid_nopost` /
  `fid_incoh`); `TimeStep.layer_entangle_max(step)` and `OperationPack.operations`
  are now exposed so Python can read the exact schedule.

## [0.2.1] - 2026-09-25

### Fixed
- **Co-installation compatibility with the `pysparq` package**: the
  `QRAMCircuitQutrit` binding now uses `py::module_local()` — the C++ type
  `qram_qutrit::QRAMCircuit` is also registered by pysparq's rich bindings
  (under the Python name `QRAMCircuit_qutrit`), and pybind11's global
  registration keyed on the C++ typeid made importing both packages in one
  process raise
  `generic_type: type "QRAMCircuit_qutrit" is already registered`. With the
  localized type the two packages coexist (verified for both import orders);
  type instances are used only within their own module and never flow across
  modules.

## [0.2.0] - 2026-09-25

### Added
- **pybind11 bindings layer returns to this repository**: `bindings/python/`
  (`core_binding.cpp` + `qram_simulator/__init__.py` + pytest suite) exports
  the core working classes — `QRAMCircuitQubit` / `QRAMCircuitQutrit` /
  `QRAMFullAmp` / `TimeStep` (with `TimeSlices` / `OperationPack` /
  `Operation` lightweight views), the `OperationType` enum, the `ARCH_QUBIT` /
  `ARCH_QUTRIT` constants, and global random-seed control (`set_seed` /
  `get_seed`), so external Python libraries and scripted experiments can drive
  the full "construct → set noise → run → fidelity" workflow directly
- **Root `pyproject.toml`**: scikit-build-core + pybind11 + setuptools-scm
  packaging for `qram-simulator` (cp310–313, manylinux x86_64 + win_amd64);
  `__version__` is read from dist-info at runtime (working around
  scikit-build-core filtering wheel files per .gitignore)
- **`.github/workflows/pypi-publish.yml`**: triggered by `v*` tags / GitHub
  Releases → cibuildwheel multi-platform wheels + sdist (with self-containment
  checks) → PyPI trusted publishing (OIDC); `python-bindings.yml` guard CI
  (wheel build + install + pytest, ubuntu/windows × py3.10/3.12)
- **Sphinx documentation site** (`docs/sphinx/`): a single site = MyST guides
  (installation / quick start / architecture rewrite) + breathe ingesting the
  Doxygen C++ API + pybind11-stubgen → autoapi for the Python API + paper
  documentation migration; `docs.yml` rewritten for this pipeline, and
  gh-pages fully replaced by the Sphinx site (Doxyfile enables
  `GENERATE_XML`)
- **Chinese Doxygen comments across the repository**: the five headers under
  `QRAM/include/` and all public headers under `Common/include/` (QRAMCircuit
  ×2, TimeStep, the State/Branch families, QRAMFullAmp, matrices, random
  engine, logging, etc.) now carry complete `/** @brief @param @return */`
  comments, wired through the Doxygen/breathe/Sphinx pipeline
- **`LICENSE`**: the full Apache-2.0 text added (pyproject and README had
  already declared this license, so GitHub license detection now takes
  effect); CONTRIBUTING clone examples unified to the GitHub address

### Changed
- **Second repository split: the SparQ framework moved out entirely and this
  repository converged to a pure C++ QRAM base**.
  `SparQ/`, `SparQ_Algorithm/`, `bindings/python/` (thin bindings),
  `examples/`, the algorithm experiments
  (QDA/Grover/StatePreparation/QCNN/QFT/CKS/Shor/GHZ/ErrorFiltration/GPUTime),
  and the SparQ-operator-dependent QRAMFidelity(v1)/QRAM_Qubit all moved to
  the SparQSim repository; CommonTest was split by ownership (the algorithm
  blocks stay with the full version in SparQSim; this repository keeps the
  Common and pure-QRAM parts). The dependency direction is fixed as
  **SparQSim → QRAM-Simulator**; this repository no longer contains any SparQ
  code or Python components, and the `qram-simulator` package was instead
  built and published from the SparQSim repository
  (note: that decision was later superseded by the bindings-layer return
  entry under Unreleased in this release — `qram-simulator` is now built and
  published independently from this repository)
- The umbrella `SparQ` CMake target moved out together with SparQ_Algorithm
  (now defined in the SparQSim root CMakeLists); this repository's exported
  targets converged to `SparQ_Common` + `SparQ_QRAMSimulator` (flattened
  headers exposed via the targets' BUILD_INTERFACE for SparQSim to compose),
  and `SparQ_Common` gained a PUBLIC link to fmt
- Build switches converged to `QRAM_BUILD_TESTS` / `QRAM_BUILD_EXPERIMENTS`
  (`QRAM_BUILD_PYTHON_BINDINGS` and `BUILD_EXAMPLES` removed); consumer-mode
  CI switched to `--target SparQ_QRAMSimulator`
- **First split**: this repository (the QRAM-Simulator monorepo) was split
  into two independent repositories; this repository keeps the QRAM-Simulator
  name and became a pure C++ core repository released independently; the
  PySparQ/pysparq full-featured Python framework moved to the **SparQSim**
  repository, which references and compiles this repository's core via a git
  submodule (relative URL `../QRAM-Simulator.git`). For PySparQ-related
  history before this entry, see the SparQSim repository's CHANGELOG and this
  file's git history (paths were removed by the split; `git log --follow` can
  trace them)
- Root CMakeLists gained the `QRAM_BUILD_TESTS` / `QRAM_BUILD_EXPERIMENTS` /
  `QRAM_BUILD_PYTHON_BINDINGS` switches for SparQSim to consume via
  add_subdirectory (tests/experiments default ON, bindings default OFF)

### Removed
- `PySparQ/`, the old `pyproject.toml`, `.cibuildwheel-hooks/`, `docs/sphinx/`,
  Python examples, and the Python jobs in the workflows (all moved to
  SparQSim)

### Added
- **qram_simulator thin Python bindings** (`bindings/python/`): a
  deliberately minimal core-primitive API — System/SparseState register
  management, Init/Hadamard/X, the Add arithmetic family, QFT,
  MeasureZ/Reset/Probability, PartialTrace, QRAMCircuit_qutrit/QRAMLoad,
  StatePrint; with pytest smoke tests
- Root `pyproject.toml` (qram-simulator package): scikit-build-core +
  setuptools-scm; wheel builds disable tests/experiments via cmake.define,
  and the sdist stays self-contained
- **Width and truncation conventions** (new chapter in `docs/operators.md`,
  the authoritative contract): LSB alignment; read extension carried by the
  name slots (`_UInt_` zero-extends / `_SInt_` sign-extends / `AnyInt`
  follows the register's declared type), which also holds in release builds;
  results written XOR with `mod 2^out_width`, and the output width may differ
  arbitrarily from the input; out-of-domain inputs get total semantics
  (division by zero → quotient 0); flag predicates evaluated over the
  full-precision domain; widths 1..64 all supported; the only behavior change
  is the AnyInt slot semantics of `Add_AnyInt_AnyInt_InPlace`
- **16 new arithmetic operators** (CPU+CUDA+PySparQ bindings, integer cores +
  the flag-predicate family, serving the de-`compile_operator`-ization of
  pyqecclang QFVM arithmetic auto-compilation): `Sub_UInt_UInt`, `Neg_UInt`,
  `Abs_SInt`, `Mul_UInt_UInt`, `Div_UInt_UInt`, `Sqrt_UInt`,
  `Select_Bool_UInt_UInt`, `And/Or/Xor_UInt_UInt`, `Less_SInt_SInt`,
  `Carry_UInt_UInt`, `Overflow_SInt_SInt`, `MulOverflow_UInt_UInt`,
  `IsZero_UInt`, `Negative_SInt`
- **Common/include/basic.h**: `width_mask` / `isqrt_u64` (HOST_DEVICE,
  bit-exact integer algorithms, bit-identical across CPU and CPU-CUDA)
- **PySparQ/pysparq/conformance.py**: `two_complement_decode` / `sign_extend`
  / `WIDTH_BOUNDARIES` / the declarative `width_matrix_case` (width
  combinations × independent models × nonzero-output-start collisions ×
  both-order dagger × superposition linearity × control matrix, all in one
  runner); **PySparQ/test/test_width_conventions.py**: width-boundary matrix
  for the 16 new operators plus the existing Add/Assign/Compare (including
  the 63/64-bit boundary)
- **SparQ/test/quantum_arithmetic.cpp**: mixed-width truth tables for the 16
  new operators + parameterized width scans + a dedicated section for the new
  Add_AnyInt semantics; **test/GPUTest/ArithmeticTest**: consistency cases
  for the CUDA fixes (new)
- **docs/naming_conventions.md**: additions covering the slot-carried
  semantics rules and the predicate-operator-family naming rules

### Changed
- **CUDA reversibility/UB fixes**: the unconditional `Add_UInt_UInt` path
  changed from overwriting to XOR + width mask (previously non-reversible on
  non-zero output registers); the `Add_UInt_UInt_InPlace`/`Add_Mult`/`Assign`
  CUDA paths gained width masks (eliminating the CPU/GPU behavioral
  divergence for unequal widths); all `%=(1<<w)`/`1<<w` UB at w=64 replaced
  with `width_mask`; `1ULL<<64` UB in `cu_as_uint64/double/bool/int64` and
  `CuConditionSatisfied` fixed (64-bit-register CUDA reads were previously
  masked to 0)
- **`Add_AnyInt_AnyInt_InPlace` semantics migration (the only behavior
  change)**: AnyInt slots now extend according to the register's declared
  type (SInt operands were previously read as unsigned bit patterns);
  width/type now read at execution time; added an always-on `lhs==rhs`
  aliasing rejection. Equal-width non-negative usage is unaffected
- **`CustomArithmetic`**: the output XOR gained a width mask (the previous
  bare write could exceed the width)
- **`consumer_runtime_inventory.json`**: the accepted list gained the 16 new
  operators (xor_into); contract tests gained independent-model cases for
  Sub/Div/Select
- **docs/naming_conventions.md** (naming-refactor task): authoritative operator
  naming ruleset — symbol layers, the `Family_Slots_Variants` grammar with closed type-tag /
  modifier / variant vocabularies, slot and `_Bool` semantics, the `_InPlace`
  naming contract, dagger-first inversion principle, gate-family rules
  (`<Axis>_Bool` without the `gate` infix), RPN composition reading, the
  deprecation/alias mechanism, and the full rename audit table
- **SparQ/src/qft.cpp**, **SparQ/include/qft.h**: `QFT::dag()` now applies
  the inverse transform (delegating to `InverseQFT` with control conditions
  preserved) and is exposed in PySparQ via `BIND_DAG_METHODS(QFT)`
- **PySparQ/pysparq/__init__.py**: PEP 562 module `__getattr__` providing
  old spellings of the 18 renamed Python operator names as deprecated aliases
  (`DeprecationWarning`), keeping external consumers (pyqecclang, qfvm)
  working during migration; `PySparQ/consumer_runtime_inventory.json` gains a
  `deprecated_aliases` section documenting the mapping
- **PySparQ/pysparq/rir.py**: native execution of QECC.Lang RIR JSON documents
  (schema versions 0.1–0.3) on PySparQ sparse states — module-graph expansion at
  interpretation time (`Call` inlining via register renaming, `Repeat` replay,
  `Adjoint` inversion, multi-bit `Control` accumulation) with native operator
  dispatch (`Add_ConstUInt_InPlace`, `GlobalPhase`, `QRAMLoad`) whenever an
  RIR operand aligns with a whole register; exposed as `run_rir`, `run_rir_file`,
  `load_rir`, `RIRResult`, `RIRError`
- **PySparQ/test/test_rir.py**: regression tests for RIR loading, gate-level
  execution, module expansion, register-view arithmetic and QRAM loading
- **docs/sphinx/source/guide/rir.rst**, **docs/sphinx/source/api/rir.rst**,
  **docs/pysparq.md**: documentation of PySparQ as the natural RIR interpreter,
  with worked examples (Bell state, `Call`/`Repeat`/`Adjoint` module graphs,
  register views, QRAM loading)
- **SparQ/include/measurement.h**, **SparQ/src/measurement.cpp**: First-class
  seedable sparse-state operations for a dynamic executor: `MeasureZ`
  (projective Z-basis measurement: Born-rule sampling + collapse +
  renormalize), `Reset` (measurement + classical conditioned flip to a
  target value), `Probability` (read-only Born-rule query + full
  single-register outcome distribution)
- **PySparQ/core.cpp**: pybind11 bindings for `MeasureZ`, `Reset`,
  `Probability`, and the global seedable RNG (`set_seed`, `get_seed`,
  `reseed`, `time_seed`)
- **PySparQ/pysparq/conformance.py**: Reusable semantic conformance harness
  (arbitrary nonzero outputs, basis-exhaustive/spot-sampled coverage,
  output-collision detection, superposition linearity, positive/negative/
  multi-register controls, forward+dagger and dagger+forward identity)
- **PySparQ/test/test_semantic_conformance.py**: Conformance matrix applied
  to representative built-ins (`Add_UInt_UInt`, `Add_UInt_UInt_InPlace`,
  `Assign`, `Compare_UInt_UInt`, `CustomArithmetic`, `Swap_General_General`,
  `Xgate_Bool`, `FlipBools`, `QRAMLoad`), plus a negative-control test
  proving the harness rejects a destructive overwrite/clearing-dagger
  implementation
- **PySparQ/test/test_measurement.py**: `MeasureZ`/`Reset`/`Probability`
  tests, including determinism under `set_seed` and Born-rule cross-checks
  against a dense-state reference
- **PySparQ/pysparq/conformance.py**: `assert_collision_free_for_output_starts`
  strengthens collision detection for `xor_into` operators by re-checking
  injectivity with the output register(s) seeded at several arbitrary
  (including nonzero) starting values, not only the conventional
  clean/zero-ancilla start; applied to `Add_UInt_UInt`, `Assign`,
  `Compare_UInt_UInt`, `CustomArithmetic`, and `QRAMLoad` in
  `test_semantic_conformance.py`, plus a negative-control test proving it
  rejects the destructive overwrite implementation at nonzero starts too
- **PySparQ/test/test_measurement.py**: `TestMeasureZNormalizationValidation`
  and `TestRegisterValidation` cover the C++ hardening below (non-normalized
  input rejection, unknown/out-of-range/inactive/duplicate register
  rejection, out-of-width-range Reset target/Probability value rejection)

### Fixed
- **SparQ/include/measurement.h**, **SparQ/src/measurement.cpp**:
  - `MeasureZ` now validates that the input state's total Born-rule
    probability is finite and within `kNormalizationThreshold` (`1e-5`,
    matching `CheckNormalization`'s default) of 1.0 before sampling, and
    raises instead of silently biasing towards/falling back to the last
    branch when a caller passes a non-normalized state. `Reset` inherits
    this guard since it delegates to `MeasureZ`.
  - `MeasureZ`/`Reset`/`Probability` constructors now validate every
    register name/id: unknown names and out-of-range or inactive
    (removed) ids raise `invalid_argument` instead of silently resolving
    to `SIZE_MAX`/an unchecked id (previously undefined behavior on use);
    duplicate registers within one call (e.g. `Reset(["a", "a"], [1, 2])`)
    are rejected instead of silently applying a contradictory target.
  - `Reset` targets and `Probability` values are validated against
    `System::size_of(id)` and now raise `invalid_argument` if they do not
    fit the register's bit width, instead of being silently truncated.

### Changed
- **Naming refactor (19 operator renames, per docs/naming_conventions.md)**:
  the gate family loses the `gate` infix (`Xgate_Bool` → `X_Bool`, 11 gates);
  `inverseQFT` → `InverseQFT` (last lowercase-initial class name); truthless
  type tags fixed (`GlobalPhase_Int` → `GlobalPhase`,
  `Div_Sqrt_Arccos_Int_Int` → `Div_Sqrt_Arccos_UInt_UInt`,
  `Sqrt_Div_Arccos_Int_Int` → `Sqrt_Div_Arccos_Int_UInt`);
  `AddAssign_AnyInt_AnyInt_InPlace` → `Add_AnyInt_AnyInt_InPlace`;
  `Hadamard_PartialQubit` → `Hadamard_Partial`;
  `CondRot_General_Bool_fast` → `CondRot_General_Bool_Fast` (C++ only);
  `QuantumBinarySearchFast` → `QuantumBinarySearch_Fast`. Old C++ spellings
  remain available as `[[deprecated]] using` aliases and old Python spellings
  via the deprecation aliases above; all in-repo consumers (SparQ,
  SparQ_Algorithm, Experiments, tests, examples, PySparQ, docs) use the new
  names. `GetRotateAngle_Int_Int` was verified truthful (signed
  interpretation) and unchanged
- **docs**: sphinx operator pages drift fixed (short names missing `_InPlace`
  for `Add_ConstUInt_InPlace`/`Add_Mult_UInt_ConstUInt_InPlace`/
  `Mod_Mult_UInt_ConstUInt_InPlace`/`Shift*_InPlace`; removed the phantom
  `CheckNormalization_Renormalize`; corrected the false
  "ShiftLeft.dag() not implemented" claim); CLAUDE.md no longer claims
  nonexistent CNOT/Toffoli classes; CONTRIBUTING.md's function-naming example
  corrected to the real `noise_free_impl` and now links the naming ruleset
- **SparQ/include/basic_components.h**, **SparQ/src/basic_components.cpp**:
  CPU basis-state register storage now uses `std::vector<StateStorage>`.
  `CACHED_REGISTER_SIZE` is the initial reserved block rather than a hard CPU
  limit; storage grows on demand and removed slots are reused. CUDA retains
  its fixed layout and 64-register ceiling required by device kernels and
  their `uint64_t` active-register bitmap.
- **PySparQ/pysparq/dynamic_operator/__init__.py**,
  **PySparQ/pysparq/dynamic_operator/README.md**: Documented that
  `compile_operator()` cannot prove unitarity and is forbidden on the
  supported QCFD path (QECC.Lang-driven qfvm/qnls/qham); QCFD semantics
  must use named PySparQ built-ins validated by `pysparq.conformance`
- **README.md**: Documented the new seedable measurement/reset/probability
  API in the Register Level key-API section

---

## [0.1.1] - 2026-05-01

### Added
- **PySparQ/core.cpp**: Native C++ bindings for CKS primitives:
  `CondRot_General_Bool_QW`, `QuantumBinarySearchFast`, `GetRowAddr`,
  `GetDataAddr`; `SparseMatrix` named constructor arguments and
  read-only properties (`nnz_col`, `n_row`, `data_size`, `sparsity`)
- **PySparQ/pysparq/__init__.py**: Export 4 new CKS primitives
- **PySparQ/pysparq/algorithms/cks_solver.py**: Full refactor:
  `_CKSWalkEnvironment` class with bytecode-based unpacking, global caches
  (`_CKS_ENV_CACHE`, `_CKS_STATE_STEP_CACHE`), `SparseMatrix.from_dense()`
  with L2-normalization, `TOperator` oracle call order aligned to C++,
  `QuantumWalkNSteps` power-of-two padding
- **PySparQ/pysparq/algorithms/qda_solver.py**: `BlockEncodingHs.dag()`
  implemented (21-operation reverse sequence); `qda_solve()` rewritten
  with correct `n_bits` and step formula
- **PySparQ/test/algorithms/test_cks_integration.py**: Full end-to-end
  quantum walk execution against C++ reference matrices
- **PySparQ/test/algorithms/test_qda_integration.py**: `WalkS` fidelity
  tests against C++ `CorrectnessTest_QDA_CompareList.inl` reference values

### Changed
- **PySparQ/src/qda_algo.cpp**: Conditional rotation primitives refactored
  to `CondRot_Fixed_Bool` and `CondRot_General_Bool_fast`; old
  function-based API removed
- **PySparQ/**: `block_encoding.py`, `state_preparation.py`,
  `dynamic_operator/compiler.py`, `dynamic_operator/operator_wrapper.py`
  updated to new condrot API
- **SparQ/src/condrot.cpp**: Internal condrot implementation aligned
- **SparQ_Algorithm/**: `hamiltonian_simulation.h/cpp` aligned with new
  condrot API
- **docs/paper/reproduction.md**: Updated CUDA build instructions;
  upstream repo URL fixed
- **CONTRIBUTING.md**: Updated clone instructions to upstream;
  added `uv venv` guidance
- **CLAUDE.md**: Added CKS/QDA Python porting status and open issues

### Fixed
- **PySparQ/src/qda_algo.cpp**: Trailing newline added

### Removed
- **PySparQ/**: Old function-based conditional rotation Python API
  hidden from `__init__.py`

### Documentation
- **docs/paper/reproduction.md**: Fix repository URL from fork to upstream
- **CONTRIBUTING.md**: Fix clone URL from fork to upstream
- **CLAUDE.md**: Add CKS/QDA Python porting status and known open issues

---

## [0.1.0] - 2025-04-15

### Added

#### Core simulator
- **Sparse-state quantum simulator core (SparQ)**
  - Quantum state storage based on sparse vector representation
  - Efficient gate-operation execution engine
  - Support for any number of qubits
  - Built-in normalization validation and state checks

#### QRAM implementation
- **Qutrit-based QRAM circuits**
  - Hierarchical tree-structured representation
  - Configurable address-line and data-line widths
  - Time-step logic generating quantum operation sequences
  - Branch probability tracking and sampling

- **Qubit-based QRAM circuits**
  - Hardware-compatible implementation
  - Shares the same API as the qutrit version

- **CUDA GPU acceleration support**
  - GPU memory management (`thrust::device_vector`)
  - Parallel gate-operation kernels
  - Asynchronous execution support
  - CPU-GPU hybrid execution strategy

#### Basic quantum gates
- **Single-qubit gates**: H (Hadamard), X, Y, Z, S, T
- **Rotation gates**: RX, RY, RZ, general rotation gates
- **Multi-qubit gates**: CNOT, Toffoli, multi-controlled gates
- **Conditional gates**: conditional rotation, conditional swap
- **Phase gates**: S, T, phase rotation, parallel phase operations

#### Advanced quantum algorithms
- **Quantum Fourier transform (QFT)**
- **Grover's search algorithm**
- **Quantum arithmetic**
  - Quantum adders
  - Quantum multipliers
  - Quantum comparators
- **Block encoding**
- **State preparation**
- **Quantum random walk**
- **Hamiltonian simulation**

#### Noise models
- **Depolarizing noise**
- **Amplitude damping noise**
- **Configurable noise parameters**
- **Noise impact analysis tools**

#### Python bindings (PySparQ)
- **Complete Python API**
  - All core classes exposed to Python
  - Type hint support (`_core.pyi`)
- **Pybind11 wrappers**
  - Efficient C++-Python interoperability
  - Automatic memory management
- **pip installation support**
  - `pyproject.toml` configuration
  - Pre-compiled wheel distribution

#### Experimental code
- **QRAM fidelity tests**
  - `QRAMFidelityTest.cpp`: main simulation + performance profiling
  - `QRAMSimulatorTest.cpp`: full vs. plain simulator comparison
- **Error filtration experiments**
  - `testMultiEFQRAM.cpp`: multi-error-filtration QRAM
- **Algorithm validation**
  - QFT, Grover, Shor algorithms
  - Quantum convolutional neural networks (QCNN)
  - Quantum differentiation algorithm (QDA)
  - GHZ state preparation

#### Infrastructure
- **CMake build system**
  - Cross-platform support (Linux, Windows, macOS)
  - Automatic dependency detection
  - Optional CUDA compilation
- **Eigen integration**
  - Embedded in header-only form (ThirdParty/eigen-3.4.0)
  - Sparse/dense matrix operation support
- **Logging system**
  - Leveled logging (DEBUG, INFO, WARN, ERROR)
  - Performance timing support
- **Unit test framework**
  - Core module test coverage
  - CI/CD integration (GitHub Actions)

### Changed
- None (initial release)

### Deprecated
- None

### Removed
- None

### Fixed
- None (initial release)

### Security
- None

---

## Version history overview

| Version | Date | Key updates |
|------|------|----------|
| 0.1.1 | 2026-05-01 | CKS/QDA Python primitive alignment, native C++ bindings, integration tests activated |
| 0.1.0 | 2025-04-15 | First release with the complete QRAM simulator, sparse-state simulation core, Python bindings, and experimental code |

---

## Future plans

### [1.1.0] - Planned

#### Expected new features
- [ ] More noise models (phase damping, bit flip, etc.)
- [ ] Visualization tools (quantum state evolution diagrams)
- [ ] Performance optimizations (vectorization, more efficient sparse operations)
- [ ] More algorithm implementations (VQE, QAOA)
- [ ] Interactive Jupyter Notebook tutorials

#### Expected improvements
- [ ] Complete Python API documentation
- [ ] More usage examples
- [ ] Performance benchmark suite

---

## References

- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)
- Project documentation: [README.md](README.md) | [Architecture guide](docs/sphinx/source/en/guide/architecture.md)
