# Installation

QRAM-Simulator can be used in two ways: **Python package installation** (recommended for external-library and script users) and **C++ source build** (recommended for developers who need deep customization).

(inst-python)=

## Python Installation (pip)

```bash
pip install qram-simulator
```

Published wheels cover CPython 3.10–3.13 on Linux x86_64 (manylinux) and Windows AMD64. The package ships the pybind11-compiled extension `_core` and has no pure-Python dependencies.

```python
import qram_simulator as qs

print(qs.__version__)
print(qs.QRAMCircuitQubit, qs.QRAMCircuitQutrit, qs.QRAMFullAmp)
```

### Installing from Source

```bash
git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
cd QRAM-Simulator
pip install .
```

Build dependencies (fetched automatically into pip's isolated build environment): CMake ≥ 3.18, a C++17 compiler, and OpenMP. Linux requires `libgomp`; Windows uses the OpenMP that ships with MSVC.

### Running the Tests

```bash
pip install .[dev]
pytest bindings/python/test
```

## C++ Build (CMake)

(inst-options)=

### Standalone Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Build toggles:

| Toggle | Default | Description |
|--------|---------|-------------|
| [`QRAM_BUILD_TESTS`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/CMakeLists.txt#L24) | ON | Build the C++ test targets (`test/`) |
| [`QRAM_BUILD_EXPERIMENTS`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/CMakeLists.txt#L25) | ON | Build the paper experiment programs (`Experiments/`) |
| [`QRAM_BUILD_PYTHON_BINDINGS`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/CMakeLists.txt#L26) | OFF | Build the pybind11 bindings ([`bindings/python/`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/tree/main/bindings/python), requires pybind11) |
| [`QRAM_ENABLE_CUDA`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/CMakeLists.txt#L31) | OFF | Currently force-disabled (CPU-only during the CondRot refactor) |

Run the CI regression cross-checks:

```bash
cd build && ctest --output-on-failure
```

### Consuming as a Subproject (the SparQSim Way)

```cmake
set(QRAM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(QRAM_BUILD_EXPERIMENTS OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/qram-simulator)   # git submodule

target_link_libraries(your_target PRIVATE SparQ_QRAMSimulator SparQ_Common)
```

The two library targets ([`SparQ_QRAMSimulator`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/QRAM/src/CMakeLists.txt#L1) / [`SparQ_Common`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/Common/src/CMakeLists.txt#L1), names kept for historical reasons) expose the flat header layout via `BUILD_INTERFACE`; once installed they can also be consumed through [`find_package(QRAMSimulator)`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/cmake/QRAMSimulatorConfig.cmake.in) (the `QRAMSimulator::` namespace).

## Dependency Notes

All third-party dependencies are vendored under [`ThirdParty/`](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/tree/main/ThirdParty); the build never accesses the network:

| Dependency | Purpose |
|------------|---------|
| Eigen 3.4.0 | Linear algebra ([matrix.h](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/blob/main/Common/include/matrix.h) / linear solvers) |
| fmt | Formatting and log output |
| argparse | Command-line parsing for the experiment programs |
| googletest | Vendored but not yet wired up (tests use an in-house TEST macro framework) |

System-level dependencies: OpenMP (required) and TBB (optional; falls back to OpenMP automatically when not found).

## Related Pages

- [Quickstart](quickstart.md) — the first step after installation
- [QRAM-Simulator Architecture](architecture.md) — module layout and data flow
