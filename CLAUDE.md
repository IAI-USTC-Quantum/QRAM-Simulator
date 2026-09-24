# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

QRAM-Simulator (repo name kept) is the **pure C++ base repository** of the QRAM
circuit simulator: the Qutrit/Qubit QRAM circuit cores (`QRAM/`), shared
infrastructure (`Common/`) and the QRAM paper experiments (`Experiments/`).
The SparQ framework — sparse-state simulator (`SparQ/`), algorithm library
(`SparQ_Algorithm/`), all Python bindings (pysparq and qram_simulator) and the
quantum-algorithm experiments — lives in the separate **SparQSim** repository,
which consumes this repository as a git submodule. Dependency direction:
**SparQSim → QRAM-Simulator** (this repo must never reference SparQSim code).

## Build Commands

```bash
# CPU build (tests + experiments on by default)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run tests
cd build && ctest --output-on-failure

# Run specific test binary
./build/bin/CorrectnessTest

# Consumer mode (what SparQSim's build does)
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DQRAM_BUILD_TESTS=OFF -DQRAM_BUILD_EXPERIMENTS=OFF

# Format/lint (pre-commit)
pre-commit run --all-files
```

## Architecture

### Key C++ Components

- **`QRAM/include/`** — Two QRAM circuit implementations:
  - `qram_circuit_qutrit.h` — More efficient, tree-based
  - `qram_circuit_qubit.h` — Hardware-compatible (implementations live in `QRAM/src/*.cpp`)
  - `time_step.h` — QRAM tree time-step bookkeeping
- **`Common/`** — Shared math, matrix, logging, error handling, random engine,
  `state_manipulator.h` (full-amplitude reference QRAM application)
- **`ThirdParty/`** — vendored Eigen, fmt, googletest, argparse

### Include Graph (do not break)

Header search paths are wired by the single global `include_directories()` at the
root CMakeLists; all includes are flat filenames. Known hard edges:

- `Common` ↔ `QRAM` is a circular include pair (`state_manipulator.h` ↔ `time_step.h`) —
  both stay in this repository
- `SparQ` headers (now in SparQSim) include `QRAM`/`Common` headers one-way;
  the reverse must never happen

### CMake Targets

- `SparQ_QRAMSimulator` (QRAM/src), `SparQ_Common` (Common/src) — the two exported
  libraries; both carry the flat include layout via `BUILD_INTERFACE`/`INSTALL_INTERFACE`
- The umbrella `SparQ` target is defined in the **SparQSim** repository
  (links the two targets above plus its local `SparQ_Simulator`/`SparQ_Algorithm`)
- Options: `QRAM_BUILD_TESTS` (test/), `QRAM_BUILD_EXPERIMENTS` (Experiments/)

## Code Style

- **C++**: LLVM-based clang-format with 4-space indent, 120 column limit, attach braces. Run `clang-format -i` via pre-commit.
- **CMake**: cmake-format + cmake-lint
- ThirdParty code is excluded from all formatting/linting rules

## Papers

This repository is supported by two papers with distinct contributions:

- **QRAM-Simulator** ([arXiv:2503.13832](https://arxiv.org/abs/2503.13832)): QRAM simulation, noise models, error filtration. Code: `QRAM/`, `Experiments/QRAM/`
- **SparQ** ([arXiv:2503.15118](https://arxiv.org/abs/2503.15118)): General-purpose sparse-state simulator, extended algorithm library. Code: `SparQ/`, `SparQ_Algorithm/` and the algorithm experiments — all in the SparQSim repository

## Documentation

- **C++ API docs**: Doxygen → `docs/api/html/`, deployed at `https://iai-ustc-quantum.github.io/QRAM-Simulator/api/` (workflow `.github/workflows/docs.yml`), input = `Common/include` + `QRAM/include`
- Python/Sphinx documentation lives in the SparQSim repository

The Gitea repository runs CPU C++ tests and a consumer-mode configure check through
`.gitea/workflows/ci.yml` (consumer-mode builds the `SparQ_QRAMSimulator` target).

## Git Workflow

**IMPORTANT: Push active development only to the Gitea origin. Never push to
the GitHub upstream remote.**

Repository role:
- `origin` → `git@git.chenzhaoyun.com:agony/QRAM-Simulator.git` (Gitea, primary)
- `upstream` → `git@github.com:IAI-USTC-Quantum/QRAM-Simulator.git` (GitHub, releases)

SparQSim consumes this repo via submodule with a **relative URL** (`../QRAM-Simulator.git`),
which resolves correctly on both Gitea and GitHub — do not rewrite it to an absolute URL.

### CI Verification Before Submitting to Upstream

**CRITICAL: Always verify CI passes on fork before submitting PR to upstream.**

1. Push changes to fork: `git push origin <branch-name>`
2. Check CI status on fork: `gh run list --repo Agony5757/QRAM-Simulator`
3. View CI details: `gh run view <run-id> --repo Agony5757/QRAM-Simulator`
4. Only after all CI checks pass, create PR to upstream

### CI Jobs

The CI workflows include:
- **cmake-multi-platform.yml** (GitHub): Build + ctest on Ubuntu (GCC C++17/C++20), Windows (MSVC)
- **docs.yml** (GitHub): Doxygen build
- **ci.yml** (Gitea): CPU C++ tests + consumer-mode configure check

## Releasing

Tag `vX.Y.Z` (history already contains v0.1.x from the monorepo era — the new
independent series starts at **v0.2.0**). This repo is source-only: no PyPI
package is published from here (`pysparq` / `qram-simulator` wheels are built
in the SparQSim repository). After a core release, bump the
submodule pin in SparQSim if pysparq needs the changes.

## Dependencies

Vendored in `ThirdParty/`: Eigen 3.4.0, fmt, googletest (v1.14.0, consumed by
SparQSim's test suite via the submodule), argparse.
External requirements: OpenMP (required), TBB (optional parallelization).
CUDA is currently force-disabled pending the CondRot primitive refactor.
