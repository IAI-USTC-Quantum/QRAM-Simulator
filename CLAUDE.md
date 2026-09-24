# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

qram-simulator is the C++ core of a sparse-state quantum circuit simulator with
native QRAM support and a "Register Level Programming" paradigm. The full-featured
Python framework (pysparq) lives in the separate SparQSim repository, which consumes
this repository as a git submodule; this repo ships only a thin `qram_simulator`
binding under `bindings/python/`.

## Build Commands

```bash
# CPU build (tests + experiments on by default)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run tests
cd build && ctest --output-on-failure

# Run specific CPU test binary
./build/bin/SparQ_Example

# Consumer mode (what SparQSim's build does)
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DQRAM_BUILD_TESTS=OFF -DQRAM_BUILD_EXPERIMENTS=OFF -DBUILD_EXAMPLES=OFF

# Thin Python bindings (requires pip-installed pybind11 in the environment)
pip install pybind11
cmake .. -DQRAM_BUILD_PYTHON_BINDINGS=ON -DQRAM_BUILD_TESTS=OFF -DQRAM_BUILD_EXPERIMENTS=OFF

# qram_simulator wheel from this repo
pip install .

# Format/lint (pre-commit)
pre-commit run --all-files
```

## Architecture

### Core Paradigm: Register Level Programming

Instead of composing circuits from individual gates, SparQ operates directly on named registers using `uint64_t` storage. High-level arithmetic operations (Add, Mult, Shift) are applied to registers directly and automatically decomposed into gates internally. Development flows top-down: write high-level algorithm modules first, then refine with register operations.

### Key C++ Components

- **`SparQ/include/sparse_state_simulator.h`** — Core state representation using `map<QIndex, Complex>` (only non-zero amplitudes stored); umbrella header that pulls in all operator headers
- **`SparQ/include/system_operations.h`** — Register management (creation, lifecycle, storage types: UnsignedInteger, SignedInteger, Boolean)
- **`SparQ/include/quantum_arithmetic.h`** — Register-level arithmetic (Add_UInt_UInt, Mult_UInt_ConstUInt, Shift, etc.)
- **`SparQ/include/basic_gates.h`** — Single-qubit gates (`X_Bool` … `U3_Bool`, plus the parameterized carriers `Phase_Bool` / `Rot_Bool`); operator naming follows `docs/naming_conventions.md`
- **`SparQ/include/qft.h`** — Optimized QFT implementation
- **`SparQ/include/qram.h`** — QRAM load operations
- **`SparQ/include/condrot.h`** — Conditional rotations
- **`SparQ/include/hadamard.h`** — Hadamard on integer registers
- **`QRAM/include/`** — Two QRAM circuit implementations:
  - `QRAMCircuit_qutrit` — More efficient, tree-based
  - `QRAMCircuit_qubit` — Hardware-compatible
- **`SparQ_Algorithm/`** — High-level algorithms (state preparation, block encoding, Hamiltonian simulation, QDA)
- **`Common/`** — Shared math, matrix, logging, error handling infrastructure

### Include Graph (do not break)

Header search paths are wired by the single global `include_directories()` at the
root CMakeLists; all includes are flat filenames. Known hard edges:

- `SparQ` **includes** `QRAM` headers (`basic_components.h` → `qram_circuit_qutrit.h`,
  `qram.h` uses `qram_qutrit::QRAMCircuit` extensively) — SparQ is not separable from QRAM
- `Common` ↔ `QRAM` is a circular include pair (`state_manipulator.h` ↔ `time_step.h`)
- The umbrella `SparQ` target is defined in `SparQ_Algorithm/src/CMakeLists.txt`, not in `SparQ/`

### Thin Python Bindings

- **`bindings/python/`** — deliberately minimal pybind11 surface (`System`, `SparseState`,
  basic gates/arithmetic, QFT, measurement, QRAM load, StatePrint). No `conditioned_by_*`
  control surface. The full binding lives in SparQSim's `PySparQ/core.cpp`.
- When the C++ API changes, both binding surfaces may need updates — cross-reference
  the change in both CHANGELOGs.

## Code Style

- **C++**: LLVM-based clang-format with 4-space indent, 120 column limit, attach braces. Run `clang-format -i` via pre-commit.
- **Python**: black (line-length=100), isort (profile=black), flake8
- **CMake**: cmake-format + cmake-lint
- ThirdParty code is excluded from all formatting/linting rules

## Papers

This repository is supported by two papers with distinct contributions:

- **QRAM-Simulator** ([arXiv:2503.13832](https://arxiv.org/abs/2503.13832)): QRAM simulation, Register Level Programming paradigm, sparse state optimization, noise models, error filtration. Code: `QRAM/`, `Experiments/QRAM/`, `Experiments/ErrorFiltration/`
- **SparQ** ([arXiv:2503.15118](https://arxiv.org/abs/2503.15118)): General-purpose sparse-state simulator, extended algorithm library (QFT, Grover, QDA, QCNN, Hamiltonian sim). Code: `SparQ/`, `SparQ_Algorithm/`, `Experiments/QFT/`, `Experiments/Grover/`, `Experiments/QDA/`, `Experiments/QCNN/`

## Documentation

- **C++ API docs**: Doxygen → `docs/api/html/`, deployed at `https://iai-ustc-quantum.github.io/qram-simulator/api/` (workflow `.github/workflows/docs.yml`)
- Python/Sphinx documentation moved to the SparQSim repository

The Gitea repository runs CPU C++ tests and a consumer-mode configure check through
`.gitea/workflows/ci.yml`.

## Git Workflow

**IMPORTANT: Push active development only to the Gitea origin. Never push to
the GitHub upstream remote.**

Repository role:
- `origin` → `git@git.chenzhaoyun.com:agony/qram-simulator.git` (Gitea, primary)
- `upstream` → `git@github.com:IAI-USTC-Quantum/qram-simulator.git` (GitHub, releases)

SparQSim consumes this repo via submodule with a **relative URL** (`../qram-simulator.git`),
which resolves correctly on both Gitea and GitHub — do not rewrite it to an absolute URL.

### CI Verification Before Submitting to Upstream

**CRITICAL: Always verify CI passes on fork before submitting PR to upstream.**

1. Push changes to fork: `git push origin <branch-name>`
2. Check CI status on fork: `gh run list --repo Agony5757/qram-simulator`
3. View CI details: `gh run view <run-id> --repo Agony5757/qram-simulator`
4. Only after all CI checks pass, create PR to upstream

### CI Jobs

The CI workflows include:
- **cmake-multi-platform.yml** (GitHub): Build + ctest on Ubuntu (GCC C++17/C++20), Windows (MSVC)
- **docs.yml** (GitHub): Doxygen build
- **pypi-publish.yml** (GitHub): cibuildwheel cp310–313 × (manylinux/win) + sdist → PyPI `qram-simulator` on `v*` tags
- **ci.yml** (Gitea): CPU C++ tests + consumer-mode configure check

## Releasing

Tag `vX.Y.Z` (history already contains v0.1.x from the monorepo era — the new
independent series starts at **v0.2.0**). Version for the Python package comes
from setuptools-scm over this repo's tags. After a core release, bump the
submodule pin in SparQSim if pysparq needs the changes.

## Dependencies

Vendored in `ThirdParty/`: Eigen 3.4.0, fmt, googletest (v1.14.0), argparse.
pybind11 is **not** vendored — it comes from the Python build environment
(`pip install pybind11`; pyproject build-system.requires covers wheel builds).
External requirements: OpenMP (required), TBB (optional parallelization).
CUDA is currently force-disabled pending the CondRot primitive refactor.
