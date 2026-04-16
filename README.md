# QRAM Simulator

[![arXiv](https://img.shields.io/badge/arXiv-2503.13832-b31b1b.svg)](https://arxiv.org/abs/2503.13832)

A high-performance sparse-state quantum simulator with QRAM (Quantum Random Access Memory) support.

This repository provides:
- **SparQ**: Sparse-state quantum simulator core
- **QRAM**: Quantum Random Access Memory circuit implementation
- **PySparQ**: Python bindings for easy prototyping
- **Algorithms**: Grover search, state preparation, block encoding, and more

## Table of Contents

- [Quick Start](#quick-start)
- [Dependencies](#dependencies)
- [Building](#building)
- [Usage Examples](#usage-examples)
- [Repository Structure](#repository-structure)
- [Paper Reproducibility](#reproducibility-of-numerical-results-paper-reference)

## Quick Start

### C++ Example

```cpp
#include "sparse_state_simulator.h"
#include "qram.h"

using namespace qram_simulator;

int main() {
    // Create QRAM with 2-bit address, 3-bit data
    qram_qutrit::QRAMCircuit qram(2, 3);
    qram.set_memory({5, 3, 7, 1});  // 4 memory locations
    
    // Create quantum state
    SparseState state;
    auto addr = AddRegister("addr", UnsignedInteger, 2)(state);
    auto data = AddRegister("data", UnsignedInteger, 3)(state);
    
    // Create superposition and load from QRAM
    (Hadamard_Int_Full(addr))(state);
    (QRAMLoad(&qram, addr, data))(state);
    
    // Print result
    (StatePrint(Detail))(state);
    return 0;
}
```

### Python Example

```python
from pysparq import *

# Create quantum state
state = SparseState()

# Add registers
addr_reg = System.add_register("addr", StateStorageType.UnsignedInteger, 2)
data_reg = System.add_register("data", StateStorageType.UnsignedInteger, 3)

# Create QRAM and apply operations
qram = QRAMCircuit_qutrit(2, 3, [5, 3, 7, 4])
Hadamard_Int_Full("addr").apply(state)
QRAMLoad(qram, "addr", "data").apply(state)

# Print state
StatePrint(StatePrintDisplay.Detail).apply(state)
```

## Dependencies

### Required

| Dependency | Version | Purpose |
|------------|---------|---------|
| CMake | >= 3.18 | Build system |
| GCC or Clang | C++17 support | Compiler |
| OpenMP | - | Parallelization |

### Optional

| Dependency | Version | Purpose |
|------------|---------|---------|
| CUDA | >= 11.0 | GPU acceleration |
| Python | >= 3.9 | Python bindings (PySparQ) |
| NumPy | >= 1.20 | Python array support |
| TBB | - | Intel Threading Building Blocks |

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libomp-dev

# For Python bindings
sudo apt-get install -y python3-dev python3-pip
pip3 install numpy pybind11

# Optional: CUDA (see NVIDIA documentation)
# Optional: TBB
sudo apt-get install -y libtbb-dev
```

**macOS:**
```bash
brew install cmake llvm libomp

# For Python bindings
pip3 install numpy pybind11
```

**CentOS/RHEL:**
```bash
sudo yum install -y cmake gcc-c++

# OpenMP is included with GCC
# For Python bindings
sudo yum install -y python3-devel
pip3 install numpy pybind11
```

## Building

### Clone the Repository

```bash
git clone --recursive https://github.com/your-repo/QRAM-Simulator.git
cd QRAM-Simulator
```

### Standard Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Build Options

```bash
# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# With GPU support (requires CUDA)
cmake -DUSE_CUDA=ON ..

# Build examples
cmake -DBUILD_EXAMPLES=ON ..

# Build Python bindings
cmake -DBUILD_PYTHON=ON ..

# Combine options
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBUILD_PYTHON=ON ..
```

### Python Bindings

```bash
# Install from source
pip install .

# Or install in development mode
pip install -e .

# Verify installation
python -c "from pysparq import *; print('PySparQ installed successfully')"
```

## Usage Examples

See the `examples/` directory for complete examples:

| Example | Description |
|---------|-------------|
| `hello_qram.cpp` | Basic QRAM circuit creation and memory loading |
| `basic_gates.cpp` | Quantum gates (X, Y, Z, H, CNOT, rotations) |
| `grover_search.cpp` | Grover's search algorithm with QRAM |

### Running Examples

```bash
# After building with -DBUILD_EXAMPLES=ON
cd build/bin
./hello_qram
./basic_gates
./grover_search
```

---

[![CI](https://github.com/Agony5757/QRAM-Simulator/workflows/CMake%20on%20multiple%20platforms/badge.svg)](https://github.com/Agony5757/QRAM-Simulator/actions)

This document provides a structured overview of the repository and a reference for QRAM-related components.

### Summary

1. **Create a QRAM circuit** with an address size and data size, and fill its memory.
2. **Create SparQ registers** for the address and data.
3. **Use `QRAMLoad`** to load the QRAM's memory value into the data register, conditioned on the address.
4. **Run additional algorithms** (state prep, block encoding, Grover, etc.) that internally call `QRAMLoad`.


# Reproducibility of Numerical Results (Paper Reference)

For a detailed description of the experimental setup, parameter regimes, and analysis methodology underlying these simulations, we refer to the associated manuscript:
https://arxiv.org/abs/2503.13832

The code and commands provided below are sufficient to reproduce the numerical results reported in the paper.

This section collects the materials required to reproduce the main numerical results of the paper:

- Simulator source code used by the experiments.
- Noise-model definitions and default noise settings.
- Build and run commands to regenerate core result files.

## What is included (with file-level pointers)

- **QRAM fidelity experiment (main simulation + profiling)**: `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp`
- **QRAM simulator comparison (full vs normal + profiling)**: `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp`
- **Error-filtration experiment**: `Experiments/ErrorFiltration/testMultiEFQRAM.cpp`

These programs correspond to the main numerical results reported in the manuscript, including QRAM fidelity scaling, simulator comparison, and error-filtration analysis.

The repository provides CMake targets for building these programs:

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
The experiment programs are compiled into executable files from the provided source code. 
Users can build the executables using a standard C++ compilation workflow in their preferred development environment.

### 2) Run (example commands)
For a single parameter setting (addrsize, datasize, shots, inputsize, depolarizing, damping, seed, version), example command-line invocations are provided below to illustrate how to run the experiments for representative parameter settings. Additional parameter sweeps can be constructed following the same format.

Windows (PowerShell):

```powershell
.\build\bin\Experiment_QRAM_Fidelity.exe --addrsize 15 --datasize 3 --shots 100 --inputsize 10 --depolarizing 1e-4 --damping 1e-5 --seed 123456 --version normal
.\build\bin\Experiment_QRAM_FidelityV2.exe --addrsize 10 --datasize 3 --shots 100 --inputsize 500 --depolarizing 1e-4 --damping 1e-4 --seed 123456789 --architecture qutrit --experimentname qutrit_scheme
.\build\bin\Experiment_ErrorFiltration.exe 5 1 10 1000 1e-5 1e-5 12345 normal
```
Note: The repository provides the simulation executables and raw output data used in the manuscript. 
Post-processing (plotting and table generation) can be performed using standard analysis scripts based on these outputs.

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
