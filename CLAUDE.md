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

# Python package — use uv venv ONLY (do not use pip or conda)
uv venv .venv && source .venv/bin/activate
uv pip install -e PySparQ/

# Run Python tests (from project root)
source .venv/bin/activate
pytest PySparQ/test/ -v

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

## Sphinx Documentation

The Sphinx docs use the **Furo** theme, which provides a sidebar table of contents automatically. When writing `.rst` documentation, if a `.. contents::` directive is used for an inline TOC, it **must** include the `:class:` attribute to suppress the Furo duplication warning:

```rst
.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here
```

Without `:class: this-will-duplicate-information-and-it-is-still-useful-here`, Furo will emit a warning about duplicated TOC information. This applies to all `.rst` files under `docs/sphinx/source/`.

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

**IMPORTANT: Never push directly to the upstream repo. Always work on fork and submit PRs.**

Repository role:
- `origin` → personal fork (`Agony5757/QRAM-Simulator`), active development
- Upstream (`IAI-USTC-Quantum/QRAM-Simulator`) — PR target, accessed via `gh` CLI (no `upstream` git remote configured)

Cross-repo operations (sync, PR) rely on `gh` CLI rather than a git upstream remote.

### CI Verification Before Submitting to Upstream

**CRITICAL: Always verify CI passes on fork before submitting PR to upstream.**

1. Push changes to fork: `git push origin <branch-name>`
2. Check CI status on fork: `gh run list --repo Agony5757/QRAM-Simulator`
3. View CI details: `gh run view <run-id> --repo Agony5757/QRAM-Simulator`
4. Only after all CI checks pass, create PR to upstream

### Typical Workflow

```bash
# Sync fork with upstream first
gh repo sync Agony5757/QRAM-Simulator --source IAI-USTC-Quantum/QRAM-Simulator --branch main
git fetch origin
git rebase origin/main

# Create feature branch from origin (fork)
git checkout -b feature/my-feature origin/main

# Push to fork
git push origin feature/my-feature

# Check CI status on fork
gh run list --repo Agony5757/QRAM-Simulator --limit 3

# After CI passes, create PR to upstream
gh pr create --repo IAI-USTC-Quantum/QRAM-Simulator \
    --head Agony5757:feature/my-feature \
    --title "feat: description" \
    --body "Summary and test plan"
```

### Sync Fork with Upstream

```bash
gh repo sync Agony5757/QRAM-Simulator --source IAI-USTC-Quantum/QRAM-Simulator --branch main
git fetch origin
git rebase origin/main
```

### CI Jobs

The CI workflow includes:
- **Build**: Ubuntu (GCC C++17/C++20), Windows (MSVC)
- **Test**: C++ unit tests via ctest
- **Python Tests**: Multiple Python versions (3.9, 3.11, 3.12) on Ubuntu and Windows
- **Docs**: Doxygen + Sphinx documentation build

## Critical: CKS/QDA Python Porting Status

**CKS 和 QDA Python 实现的端到端 fidelity 测试状态：**

### CKS (Chebyshev-Kothari-Somma Linear Solver)

Python 实现在 `PySparQ/pysparq/algorithms/cks_solver.py` 中，包括：
- `ChebyshevPolynomialCoefficient` ✓ (数学正确性测试通过)
- `get_coef_positive_only` / `get_coef_common` ✓ (酉性验证通过)
- `SparseMatrix` ✓
- `TOperator` ✓ (结构正确)
- `QuantumWalk` / `QuantumWalkNSteps` ✓ (结构正确)

**未完成：端到端 fidelity 测试**

- `test_cks_integration.py::TestQuantumWalkFidelity` 中的 `test_quantum_walk_chebyshev_fidelity` 已用真实量子行走执行替代 TODO 注释，对应 C++ `CorrectnessTest_Common.inl` 中的 `Chebyshev_test()`（fidelity >= 0.999）
- `test_lcu_linear_solver_fidelity` 同样，对应 `linear_solver_theory_compare_test()`（fidelity >= 0.9999）

**已知限制**：`QuantumBinarySearch._find_column_position` 的逆操作在某些情况下未完全 uncompute，可能导致 `RemoveRegister` 调用时出现 `RuntimeError`。

### QDA (Quantum Discrete Adiabatic Linear Solver)

Python 实现在 `PySparQ/pysparq/algorithms/qda_solver.py` 中，包括：
- `compute_fs` ✓ (数学正确性测试通过)
- `compute_rotation_matrix` ✓ (酉性验证通过)
- `chebyshev_T` ✓
- `dolph_chebyshev` / `compute_fourier_coeffs` ✓
- `WalkS` / `BlockEncodingHs` / `LCU` / `Filtering` ✓
- `BlockEncodingHs.dag()` ✓ (已实现，21 步逆操作)
- `qda_solve()` ✓ (已重写，正确的 `n_bits` 和步数公式)

**端到端 fidelity 测试已激活**：`test_walks_fidelity_tridiagonal` 和 `test_walks_fidelity_via_qram` 已启用，与 C++ `CorrectnessTest_QDA_CompareList.inl` 参考值对比（要求 diff < 1e-5）

### 下一步行动

1. **完善 QDA 结果提取**：`qda_solve` 当前返回 `np.linalg.solve` 的经典结果，真正的量子振幅提取逻辑是 TODO
2. **修复 CKS uncompute 边界情况**：解决 `QuantumBinarySearch._find_column_position` 的逆操作清理问题
3. **完善 QDA `is_positive_definite=True` 分支测试**：当前测试聚焦于 `is_positive_definite=False` (Neg) 情况

## Python Test Structure

```
PySparQ/test/
  conftest.py             fresh_system fixture (autouse), helpers
  test_doc_examples.py     示例代码可执行性测试
  test_dynamic_operator.py
  algorithms/
    conftest.py           tridiagonal_matrix, random_unitary, simple_linear_system fixtures
    test_cks_integration.py    CKS 数学正确性 + 端到端 fidelity 测试
    test_cks_solver.py         CKS 各组件单元测试
    test_qda_integration.py    QDA 数学正确性 + 端到端 fidelity 测试
    test_qda_solver.py         QDA 各组件单元测试
    test_condition_mixin.py     条件操作符测试
    test_grover.py             Grover 测试
    test_shor.py               Shor 测试
    test_state_preparation.py  态制备测试
    test_block_encoding.py      块编码测试
    test_qram_utils.py         QRAM 工具测试
```

### Python API Design

- `pysparq/algorithms/*.py` 中的算法类（`TOperator`, `QuantumWalk` 等）继承自 `ControllableOperatorMixin`，提供 `.conditioned_by_*()` 方法链和 `.dag()` 逆操作
- `pysparq/` 包有两套 API：旧版（可变状态 + `ps.System.clear()` 中间调用）标记为 deprecated
- 量子态 `ps.SparseState` 是 pybind11 C++ 对象，**不支持 deepcopy/pickle**，原地变异是预期行为（与 C++ 实现一致）

## Dependencies

All vendored in `ThirdParty/`: Eigen 3.4.0, fmt, pybind11, argparse. External requirements: OpenMP (required), CUDA 12+ (optional GPU), TBB (optional parallelization), Google Test (fetched via CMake FetchContent).
