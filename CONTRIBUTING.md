# Contributing Guide

First off, thank you for considering contributing to the QRAM-Simulator project! 🎉

This guide helps you understand how to get involved in developing the project, including filing issues, setting up the development environment, following the coding conventions, and submitting pull requests.

## Table of Contents

- [Filing Issues](#filing-issues)
- [Development Environment Setup](#development-environment-setup)
- [Coding Conventions](#coding-conventions)
- [Pull Request Workflow](#pull-request-workflow)

## Filing Issues

### Bug Reports

If you find a bug, please create an issue using the [bug report template](.github/ISSUE_TEMPLATE/bug_report.md) and provide as much of the following information as possible:

- A clear description of the problem
- Steps to reproduce
- Expected behavior vs. actual behavior
- Environment information (operating system, compiler, CMake version, etc.)
- Error logs or screenshots

### Feature Requests

If you have an idea for a new feature, please create an issue using the [feature request template](.github/ISSUE_TEMPLATE/feature_request.md), including:

- A description of the feature
- Use cases
- Desired API/interface design
- Alternative solutions (if any)

## Development Environment Setup

### Prerequisites

- **CMake**: >= 3.18
- **C++ compiler**: with C++17 support
- **Git**: any recent version

### Optional Dependencies

- **CUDA Toolkit**: if you need GPU support
- **TBB (Intel Threading Building Blocks)**: for parallel computing optimizations
- **OpenMP**: usually included in modern compilers

### Build Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
   cd QRAM-Simulator
   ```

2. **Create a build directory**
   ```bash
   mkdir build && cd build
   ```

3. **Configure the project**
   ```bash
   cmake ..
   ```
   
   To specify particular options:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release -DCACHED_REGISTER_SIZE=32
   ```
   `CACHED_REGISTER_SIZE` only controls the number of register slots initially
   reserved per basis state in CPU builds; the underlying `std::vector` keeps
   growing as needed. It remains a fixed capacity only in CUDA builds.

4. **Build**
   ```bash
   cmake --build . -j$(nproc)
   ```

5. **Run the tests**
   ```bash
   ./bin/run_tests
   ```

### Python Bindings Development

This repository is a pure C++ base and contains no Python bindings. The
`pysparq` and `qram_simulator` packages are both developed in the
[SparQSim repository](https://github.com/IAI-USTC-Quantum/SparQSim)
(which references this repository as a submodule).

## Coding Conventions

### Basic Rules

- **C++ standard**: C++17
- **Indentation**: 4 spaces (no tabs)
- **File encoding**: UTF-8

### Naming Conventions

| Type | Naming style | Example |
|------|---------|------|
| Namespaces | lowercase + underscores | `qram_simulator` |
| Class names | PascalCase | `QRAMLoad`, `SparseState` |
| Function names | snake_case | `noise_free_impl`, `make_mask` |
| Member variables | camelCase + underscore suffix | `register_addr_`, `state_vector_` |
| Local variables | camelCase | `tempValue`, `index` |
| Macros/constants | uppercase + underscores | `MAX_QUBITS`, `CACHE_SIZE` |
| Template parameters | PascalCase | `typename InputIt` |

Quantum operator naming (type slots, variant suffixes, inverse-operation
notation, etc.) follows
[docs/naming_conventions.md](docs/naming_conventions.md); please read it
before adding new operators.

### Header File Rules

- Use `#pragma once` as the include guard
- Include order: system headers → third-party libraries → project headers

```cpp
#pragma once

// System headers
#include <vector>
#include <memory>

// Third-party libraries
#include <eigen/Dense>

// Project headers
#include "QRAM/QRAM.h"
```

### Code Formatting

The project uses `.clang-format` for code formatting. Before committing code, make sure:

```bash
# Format all source files
find . -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i
```

### Comment Conventions

- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Functions and classes should use Doxygen-style comments

```cpp
/**
 * @brief Perform a QRAM loading operation
 * @param address address register state
 * @param data data register state
 * @return the quantum state after loading
 */
QState qramLoad(const QState& address, const QState& data);
```

## Pull Request Workflow

### 1. Clone the repository

### 2. Create a feature branch

```bash
git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
cd QRAM-Simulator
git checkout -b feature/your-feature-name
```

Suggested branch naming:
- `feature/` - new features
- `bugfix/` - bug fixes
- `docs/` - documentation updates
- `refactor/` - code refactoring

### 3. Commit your changes

Write a clear commit message:

```bash
git add .
git commit -m "feat: add GPU acceleration support for the sparse state simulator

- Implement CUDA kernels for parallel amplitude computation
- Add memory pool management to reduce allocation overhead
- ~3x performance improvement measured on V100"
```

Commit message format:
- `feat:` new feature
- `fix:` bug fix
- `docs:` documentation update
- `style:` code formatting changes (no functional impact)
- `refactor:` code refactoring
- `perf:` performance optimization
- `test:` test related
- `chore:` build/tooling related

### 4. Push to your fork

```bash
git push origin feature/your-feature-name
```

### 5. Create a pull request

1. Visit your forked repository
2. Click "Compare & pull request"
3. Fill in the PR description using the provided [PR template](.github/PULL_REQUEST_TEMPLATE.md)
4. Link related issues (if any): `Fixes #123`
5. Submit the PR

### 6. Wait for CI to pass and for review

- Make sure all CI checks pass
- Wait for maintainer review
- Make changes based on feedback
- Keep your PR in sync with the main branch: `git pull upstream main`

### PR review checklist

Before submitting a PR, please confirm:

- [ ] Code follows the project coding conventions
- [ ] All tests pass
- [ ] New features have corresponding test coverage
- [ ] Documentation has been updated (if needed)
- [ ] Commit messages are clear and meaningful
- [ ] The PR description is complete and links related issues

## Getting Help

If you have any questions, you can get help in the following ways:

- Ask in an issue
- Browse the existing documentation and code
- Refer to the project [README.md](README.md)

Thanks again for your contribution! 🙏
