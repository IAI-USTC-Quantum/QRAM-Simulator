# QRAM Simulator

This document gives a plain‑language map of the repository and a consolidated reference for QRAM‑related components.

### Summary

1. **Create a QRAM circuit** with an address size and data size, and fill its memory.
2. **Create SparQ registers** for the address and data.
3. **Use `QRAMLoad`** to load the QRAM’s memory value into the data register, conditioned on the address.
4. **Run additional algorithms** (state prep, block encoding, Grover, etc.) that internally call `QRAMLoad`.


## Numerical simulation testing locations

Below is where QRAM timing tests and error‑filtration tests live in this repo, with file‑level citations.

### QRAM timing / performance testing locations

#### 1) QRAM fidelity tests that record profiler timing

These tests wrap QRAM operations in profiler scopes and print profile summaries, which is where timing/perf for QRAM is captured:

- **Experiment fidelity test**: `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp`  
  *This experiment explicitly wraps the QRAM load in a profiler scope (`profiler _("**RECORD**")`) and prints profiling summaries afterward, which is another place QRAM timing is recorded.*

#### 2) QRAM simulator comparison (profiling per‑run)

The "V2" QRAM simulator test sets up a per‑run `profiler` instance and also profiles the main test loop, which is useful for timing comparisons between "full" and "normal" QRAM executions:

- `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp`  
  *This defines a profiler per QRAM run and uses `profiler _("MainLoop")` inside the comparison loop, so the time for QRAM execution versions is tracked here.*

If you specifically meant "wall‑clock benchmarks," the experiments above use the project’s `profiler` mechanism (not raw `std::chrono`) to capture timings.

### Error‑filtration testing locations

The error filtration work is concentrated here:

- **Multi‑EF QRAM experiment**: `Experiments/ErrorFiltration/testMultiEFQRAM.cpp`  
  *This file defines noise models (depolarizing + damping), prepares QRAM inputs, runs QRAM loads under noise, and computes fidelities for error‑filtration experiments.*

Key indications in that file:

- Noise model construction for error filtration tests (depolarizing + damping) and QRAM setup with noise.
- No‑error‑filtration path (`main_no_ef`) that runs the QRAM load, followed by a noisefree load for fidelity comparison.

If you want to run them (high level):

- **Timing / profiling runs**: the QRAM fidelity tests and simulator comparison experiments under `Experiments/QRAM/` or `test/CPUTest/` are the intended entry points.
- **Error filtration**: the `Experiments/ErrorFiltration/testMultiEFQRAM.cpp` experiment is the dedicated location.

The classes and operators above are the "official" interface points that enable that flow.

#### 1) Repository structure

The top‑level CMake configuration wires together the major modules below. If you are new to the codebase, this is the easiest map to start from:

- **Common/** — shared utilities that are reused across modules. It is included in the top‑level build. 
- **QRAM/** — the QRAM simulator and data structures. This is the core module for QRAM logic.
- **SparQ/** — the sparse‑state simulator core (operators and execution framework).
- **SparQ_Algorithm/** — higher‑level algorithms built on top of SparQ and QRAM (e.g., state preparation, block‑encoding).
- **PySparQ/** — Python bindings and API definitions.
- **ThirdParty/** — bundled third‑party dependencies.
- **test/** and **Experiments/** — tests and experiment drivers.

#### 2) QRAM at a glance

At a high level, the QRAM support in this repository provides:

- **A QRAM circuit model** with an address size and data size.
- **Memory storage** indexed by address.
- **Operations to run the QRAM circuit** (including noise models).
- **Operators that integrate QRAM into the SparQ sparse‑state simulator** (e.g., QRAM load operators).

The details are captured in the sections below.

#### 3) Core QRAM circuit types

There are two QRAM circuit implementations:

##### 3.1 Qutrit‑based QRAM (`qram_qutrit::QRAMCircuit`)

Key structure (from `QRAM/include/qram_circuit_qutrit.h`):

- **Address/data sizes**: `address_size`, `data_size`.
- **Time‑step logic**: `time_step` used to generate QRAM operations.
- **Memory**: `memory`, sized to `2^address_size`.
- **Noise model support**: `noise_parameters`, plus helpers like `is_noise_free()` and `has_damping()`.
- **Branching state**: `branches`, `branch_probs`, `valid_branch_view`, `first_good_branch`, and `good_branch_ids`.
- **Execution entry points**: `initialize_system()`, `run_normal()`, `run_full()`, `run_good_only()`, and `run(version)`.
- **Sampling/normalization utilities**: `sample_output()` and `normalization()` plus damping‑aware variants.

##### 3.2 GPU‑accelerated QRAM (CUDA)

For qutrit QRAM, there is a CUDA implementation:

- **`qram_qutrit::CuQRAMCircuit`** extends `QRAMCircuit`.
- **GPU memory mirror**: `memory_dev` (a `thrust::device_vector`).
- **Memory setters** synchronize host and device memory.

This provides GPU memory storage used by CUDA‑enabled QRAM operations.

#### 4) Internal QRAM data structures (qutrit model)

The QRAM qutrit implementation uses several data structures to model internal state:

##### 4.1 QRAMNode

Each node holds **address** and **data** values with helper operations such as:

- flipping address/data bits,
- internal swap,
- rotating among qutrit states (A1/A2 rotations),
- checking a zero state.

##### 4.2 QRAMState

`QRAMState` stores a **sparse map** of non‑zero elements, with helpers for:

- tree navigation (`left_of`, `right_of`, `parent_of`),
- bus input/output (`busin`, `busout`),
- conditional swaps and rotations,
- checking and setting internal states.

### 4.3 SubBranch and Branch

The QRAM circuit evaluates a collection of branches:

- **SubBranch** contains a `QRAMState`, a data bus, and amplitude, and exposes operations like `run_acopy`, `run_busin`, `run_busout`, etc.
- **Branch** tracks a QRAM address, bus input, relative probabilities, and associated `SubBranch` instances.


#### 5) QRAM operators in SparQ

SparQ exposes QRAM‑aware operators for the sparse‑state simulator:

##### 5.1 `QRAMLoad`

`QRAMLoad` is a self‑adjoint operator that integrates a `qram_qutrit::QRAMCircuit` into a SparQ simulation. It stores the QRAM pointer and the address/data register IDs, and supports CUDA when enabled. 

The user‑facing behavior in the SparQ operator list describes the QRAM load as:

> \|a⟩\|z⟩ → \|a⟩\|z ⊕ d[a]⟩,  
> where `d[a]` is the classical data stored in the QRAM circuit and the address/data register sizes must match the QRAM circuit sizes.

##### 5.2 `QRAMLoadFast`

`QRAMLoadFast` is a related operator optimized for speed, also restricted to the qutrit QRAM circuit and with separate noise‑handling paths. It is defined in `SparQ/include/qram.h`.

##### 5.3 `QRAMInputGenerator`

`QRAMInputGenerator` is a helper for generating randomized or full input superpositions over address/data registers, enforcing size limits and normalization. This utility lives in `SparQ/include/qram.h`.

#### 6) Python API (PySparQ)

The Python bindings expose QRAM‑related classes:

- **`QRAMCircuit_qutrit`** — constructors accepting address size, data size, and optional memory arrays.
- **`QRAMLoad`** and **`QRAMLoadFast`** — QRAM load operators with control helpers (`conditioned_by_*`) and access to the underlying QRAM circuit.

These are defined in `PySparQ/pysparq/_core.pyi`.

#### 7) QRAM in higher‑level algorithms

QRAM is used directly in algorithm modules; for example:

- **State preparation via QRAM** uses repeated `QRAMLoad` calls as part of its algorithm, loading "parent" and "child" data branches to compute rotations in a state‑preparation routine.

This shows how QRAM primitives are composed into higher‑level algorithms.
