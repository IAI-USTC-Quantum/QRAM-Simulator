# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

QRAM-Simulator is a sparse-state quantum circuit simulator with native QRAM support and a "Register Level Programming" paradigm. It consists of two components:
- **QRAM-Simulator** (C++): High-performance sparse-state simulator core
- **PySparQ** (Python bindings via pybind11): High-level API for algorithm development

## Build Commands

```bash
# CPU build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# GPU build (requires CUDA toolkit)
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_CUDA=ON
make -j$(nproc)

# Run tests
cd build && ctest --output-on-failure

# Run specific CPU test binary
./build/bin/SparQ_Example

# Python package (pip install from source)
pip install .

# Format/lint (pre-commit)
pre-commit run --all-files
```

## Architecture

### Core Paradigm: Register Level Programming

Instead of composing circuits from individual gates, SparQ operates directly on named registers using `uint64_t` storage. High-level arithmetic operations (Add, Mult, Shift) are applied to registers directly and automatically decomposed into gates internally. Development flows top-down: write high-level algorithm modules first, then refine with register operations.

### Key C++ Components

- **`SparQ/include/sparse_state_simulator.h`** — Core state representation using `map<QIndex, Complex>` (only non-zero amplitudes stored)
- **`SparQ/include/system_operations.h`** — Register management (creation, lifecycle, storage types: UnsignedInteger, SignedInteger, Boolean)
- **`SparQ/include/quantum_arithmetic.h`** — Register-level arithmetic (Add_UInt_UInt, Mult_UInt_ConstUInt, Shift, etc.)
- **`SparQ/include/basic_gates.h`** — Fundamental quantum gates (H, X, Y, Z, S, T, RX, RY, RZ, CNOT, Toffoli)
- **`SparQ/include/qft.h`** — Optimized QFT implementation
- **`SparQ/include/qram.h`** — QRAM load operations
- **`SparQ/include/condrot.h`** — Conditional rotations
- **`SparQ/include/hadamard.h`** — Hadamard on integer registers
- **`QRAM/include/`** — Two QRAM circuit implementations:
  - `QRAMCircuit_qutrit` — More efficient, tree-based
  - `QRAMCircuit_qubit` — Hardware-compatible
- **`SparQ_Algorithm/`** — High-level algorithms (state preparation, block encoding, Hamiltonian simulation, QDA)
- **`Common/`** — Shared math, matrix, logging, error handling infrastructure

### Python Bindings

- **`PySparQ/pysparq/`** — pybind11 bindings exposing the full C++ API
- All register-level operations are exposed: `Add_UInt_UInt`, `Add_ConstUInt`, `Mult_UInt_ConstUInt`, `Hadamard_Int`, `QRAMLoad`, etc.

## Code Style

- **C++**: LLVM-based clang-format with 4-space indent, 120 column limit, attach braces. Run `clang-format -i` via pre-commit.
- **Python**: black (line-length=100), isort (profile=black), flake8
- **CMake**: cmake-format + cmake-lint
- ThirdParty code is excluded from all formatting/linting rules

## Papers

This repository is supported by two papers with distinct contributions:

- **QRAM-Simulator** ([arXiv:2503.13832](https://arxiv.org/abs/2503.13832)): QRAM simulation, Register Level Programming paradigm, sparse state optimization, noise models, error filtration. Code: `QRAM/`, `Experiments/QRAM/`, `Experiments/ErrorFiltration/`
- **SparQ** ([arXiv:2503.15118](https://arxiv.org/abs/2503.15118)): General-purpose sparse-state simulator, extended algorithm library (QFT, Grover, QDA, QCNN, Hamiltonian sim), PySparQ Python API, GPU acceleration. Code: `SparQ/`, `SparQ_Algorithm/`, `PySparQ/`

## GitHub Pages

Doxygen documentation is auto-deployed on push to `main`:
- **Landing page**: `https://iai-ustc-quantum.github.io/QRAM-Simulator/` (source: `docs/index.html`)
- **C++ API docs**: `https://iai-ustc-quantum.github.io/QRAM-Simulator/api/` (source: Doxygen → `docs/api/html/`)
- Deployed via `peaceiris/actions-gh-pages` to `gh-pages` branch (see `.github/workflows/cmake-multi-platform.yml`, `docs` job)

## Git Workflow

**IMPORTANT: Never push directly to `origin/main`. Always work on fork and submit PRs.**

Remote configuration:
- `origin` → upstream organization repo (`IAI-USTC-Quantum/QRAM-Simulator`)
- `fork` → personal fork (`Agony5757/QRAM-Simulator`)

Workflow:
1. All development happens on `fork` (personal fork)
2. Changes are submitted via PR from `fork` to `origin/main`
3. Never push directly to `origin` - it should be protected

Typical workflow:
```bash
# Create feature branch on fork
git checkout -b feature/my-feature fork/main

# Push to fork
git push fork feature/my-feature

# Create PR to origin/main
gh pr create --repo IAI-USTC-Quantum/QRAM-Simulator --head Agony5757:feature/my-feature
```

Sync fork with upstream:
```bash
gh repo sync Agony5757/QRAM-Simulator --source IAI-USTC-Quantum/QRAM-Simulator --branch main
git fetch fork
```

## Dependencies

All vendored in `ThirdParty/`: Eigen 3.4.0, fmt, pybind11, argparse. External requirements: OpenMP (required), CUDA 12+ (optional GPU), TBB (optional parallelization), Google Test (fetched via CMake FetchContent).
