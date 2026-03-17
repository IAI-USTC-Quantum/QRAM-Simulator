# QRAM Simulator

This document gives a plain‑language map of the repository and a consolidated reference for QRAM‑related components.

### Summary

1. **Create a QRAM circuit** with an address size and data size, and fill its memory.
2. **Create SparQ registers** for the address and data.
3. **Use `QRAMLoad`** to load the QRAM’s memory value into the data register, conditioned on the address.
4. **Run additional algorithms** (state prep, block encoding, Grover, etc.) that internally call `QRAMLoad`.


# Numerical simulation testing locations (for the paper reference)

This section collects the reproducibility materials for the paper's main numerical results:

- Simulator source code used by the experiments.
- Noise-model definitions and default noise settings.
- Build and run commands to regenerate core result files.

## What is included (with file-level pointers)

- **QRAM fidelity experiment (main simulation + profiling)**: `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp`
- **QRAM simulator comparison (full vs normal + profiling)**: `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp`
- **Error-filtration experiment**: `Experiments/ErrorFiltration/testMultiEFQRAM.cpp`

All three programs are built by CMake targets:

- `Experiment_QRAM_Fidelity`
- `Experiment_QRAM_FidelityV2`
- `Experiment_ErrorFiltration`

## Noise model settings used by these experiments

Noise is configured in-code via:

- `OperationType::Depolarizing`
- `OperationType::Damping`

Locations:

- `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp` 
- `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp` 
- `Experiments/ErrorFiltration/testMultiEFQRAM.cpp` 

Default values in source:

- `QRAMFidelityTest.cpp`: `depolarizing = 0.0`, `damping = 0.0` (argument defaults); built-in demo run uses `1e-4`, `1e-5`.
- `QRAMSimulatorTest.cpp`: `depolarizing = 0.0`, `damping = 0.0` (argument defaults); current compiled-in test run uses `1e-4`, `1e-4`.
- `testMultiEFQRAM.cpp`: `depolarizing = 1e-5`, `damping = 1e-5` (defaults in `main`).

## How to reproduce the main result files

### 1) Build
We use Visual Studio Code to build the executable files.

### 2) Run (example commands)
For a single parameter setting (addrsize, datasize, shots, inputsize, depolarizing, damping, seed, version), we will have python script to automately generate the following command line.

Windows (PowerShell):

```powershell
.\build\bin\Experiment_QRAM_Fidelity.exe --addrsize 15 --datasize 3 --shots 100 --inputsize 10 --depolarizing 1e-4 --damping 1e-5 --seed 123456 --version normal
.\build\bin\Experiment_QRAM_FidelityV2.exe --addrsize 10 --datasize 3 --shots 100 --inputsize 500 --depolarizing 1e-4 --damping 1e-4 --seed 123456789 --architecture qutrit --experimentname qutrit_scheme
.\build\bin\Experiment_ErrorFiltration.exe 5 1 10 1000 1e-5 1e-5 12345 normal
```

Note: this repository currently provides simulation executables and raw output artifacts. If the manuscript requires exact plotting/table scripts, add them as a small post-processing script layer that reads the files above.
## 1) Repository structure (plain‑language map)

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
