# Paper Experiment Reproduction Guide

This document provides detailed experiment setups, parameter ranges, and run commands for reproducing the numerical results in the paper.

## Paper Information

- **Title**: Efficient Simulation of Quantum Random Access Memory
- **arXiv**: [2503.13832](https://arxiv.org/abs/2503.13832)
- **Code repository**: https://github.com/IAI-USTC-Quantum/QRAM-Simulator

## Environment Preparation

### System Requirements

- **Operating system**: Linux (Ubuntu 20.04+ recommended), Windows (Visual Studio 2019+), macOS
- **Compiler**: GCC 9+, Clang 10+, MSVC 2019+
- **CMake**: 3.15+
- **GPU**: the CUDA/GPU backend is currently disabled in CMake; experiment reproduction uses a CPU-only build by default

### Build Steps

```bash
# Clone the repository
git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
cd QRAM-Simulator

# Create a build directory
mkdir build && cd build

# Configure the project
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build all experiment targets
make -j$(nproc) Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration
```

### Windows (PowerShell) Build

```powershell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration --config Release
```

## Experiment Overview

| Experiment | Description | Paper section |
|------|------|-------------|
| **QRAM Fidelity** | Scaling of QRAM fidelity with parameters | Main results |
| **QRAM Simulator Comparison** | Comparison of different simulator configurations | Method validation |
| **Error Filtration** | Effect of the error filtration scheme | Application case |

---

## Experiment 1: QRAM Fidelity

**Code location**: `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp`

**Build target**: `Experiment_QRAM_Fidelity`

### Command-Line Arguments

| Parameter | Description | Default |
|------|------|--------|
| `--addrsize` | Address register width (bits) | 10 |
| `--datasize` | Data register width (bits) | 3 |
| `--shots` | Number of measurements | 100 |
| `--inputsize` | Size of the input superposition | 10 |
| `--depolarizing` | Depolarizing noise strength | 0.0 |
| `--damping` | Amplitude damping noise strength | 0.0 |
| `--seed` | Random seed | 123456 |
| `--version` | Run version: `normal` or `full` | normal |

### Run Examples

**Linux/macOS**:
```bash
./build/bin/Experiment_QRAM_Fidelity \
    --addrsize 15 --datasize 3 \
    --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 \
    --seed 123456 --version normal
```

**Windows (PowerShell)**:
```powershell
.\build\bin\Experiment_QRAM_Fidelity.exe `
    --addrsize 15 --datasize 3 `
    --shots 100 --inputsize 10 `
    --depolarizing 1e-4 --damping 1e-5 `
    --seed 123456 --version normal
```

### Parameter Sweep Recommendations

To reproduce the scaling plots in the paper, sweep the following parameters:

```bash
# Address width sweep (fixed noise)
for addr in 5 10 15 20 25 30; do
    ./build/bin/Experiment_QRAM_Fidelity \
        --addrsize $addr --datasize 3 \
        --shots 100 --inputsize 10 \
        --depolarizing 1e-4 --damping 1e-5 \
        --seed 123456 --version normal
done

# Noise strength sweep (fixed address width)
for depol in 0.0 1e-5 5e-5 1e-4 5e-4 1e-3; do
    ./build/bin/Experiment_QRAM_Fidelity \
        --addrsize 15 --datasize 3 \
        --shots 100 --inputsize 10 \
        --depolarizing $depol --damping 1e-5 \
        --seed 123456 --version normal
done
```

### Output Description

The program output contains:
- Simulation configuration parameters
- QRAM circuit depth and gate count statistics
- Fidelity estimates with confidence intervals
- Runtime statistics

---

## Experiment 2: QRAM Simulator Comparison

**Code location**: `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp`

**Build target**: `Experiment_QRAM_FidelityV2`

### Command-Line Arguments

| Parameter | Description | Default |
|------|------|--------|
| `--addrsize` | Address register width (bits) | 10 |
| `--datasize` | Data register width (bits) | 3 |
| `--shots` | Number of measurements | 100 |
| `--inputsize` | Size of the input superposition | 500 |
| `--depolarizing` | Depolarizing noise strength | 0.0 |
| `--damping` | Amplitude damping noise strength | 0.0 |
| `--seed` | Random seed | 123456789 |
| `--architecture` | QRAM architecture: `qutrit` or `qubit` (qubit currently supports only full mode, serving as the ground truth) | qutrit |
| `--experimentname` | Experiment name label | test |

### Run Examples

```bash
./build/bin/Experiment_QRAM_FidelityV2 \
    --addrsize 10 --datasize 3 \
    --shots 100 --inputsize 500 \
    --depolarizing 1e-4 --damping 1e-4 \
    --seed 123456789 --architecture qutrit \
    --experimentname qutrit_scheme
```

### Comparison Experiments

```bash
# Qutrit architecture vs Standard architecture
./build/bin/Experiment_QRAM_FidelityV2 \
    --addrsize 10 --datasize 3 \
    --depolarizing 1e-4 --damping 1e-4 \
    --architecture qutrit --experimentname qutrit_test

./build/bin/Experiment_QRAM_FidelityV2 \
    --addrsize 10 --datasize 3 \
    --depolarizing 1e-4 --damping 1e-4 \
    --architecture standard --experimentname standard_test
```

---

## Experiment 3: Error Filtration

**Code location**: `Experiments/ErrorFiltration/testMultiEFQRAM.cpp`

**Build target**: `Experiment_ErrorFiltration`

### Command-Line Arguments (positional)

| Position | Parameter | Description | Example value |
|------|------|------|--------|
| 1 | `addrsize` | Address register width (bits) | 5 |
| 2 | `datasize` | Data register width (bits) | 1 |
| 3 | `num_qrams` | Number of QRAMs | 10 |
| 4 | `shots` | Number of measurements | 1000 |
| 5 | `depolarizing` | Depolarizing noise strength | 1e-5 |
| 6 | `damping` | Amplitude damping noise strength | 1e-5 |
| 7 | `seed` | Random seed | 12345 |
| 8 | `version` | Run version | normal |

### Run Examples

**Linux/macOS**:
```bash
./build/bin/Experiment_ErrorFiltration 5 1 10 1000 1e-5 1e-5 12345 normal
```

**Windows (PowerShell)**:
```powershell
.\build\bin\Experiment_ErrorFiltration.exe 5 1 10 1000 1e-5 1e-5 12345 normal
```

### Parameter Notes

- **num_qrams**: number of cascaded QRAM modules, used to test error accumulation
- **version**: either `normal` (standard run) or `good_only` (keep only the "good branches")

---

## Noise Model Description

All experiments use the following noise models:

### Depolarizing Noise

```cpp
OperationType::Depolarizing
```

Replaces the quantum state with the fully mixed state with probability `p`. Equivalent to applying a random Pauli X/Y/Z gate.

### Amplitude Damping

```cpp
OperationType::Damping
```

Models the $|1\rangle \rightarrow |0\rangle$ decay caused by energy dissipation.

### Default Noise Values

| Experiment | Depolarizing | Damping |
|------|--------|------|
| QRAMFidelityTest | 0.0 (default), 1e-4 (demo) | 0.0 (default), 1e-5 (demo) |
| QRAMSimulatorTest | 0.0 (default), 1e-4 (demo) | 0.0 (default), 1e-4 (demo) |
| ErrorFiltration | 1e-5 | 1e-5 |

---

## Reproduction Checklist

- [ ] Successfully build all three experiment targets
- [ ] Run the QRAM Fidelity experiment and collect fidelity data
- [ ] Complete the address width sweep (5-30 bits)
- [ ] Complete the noise strength sweep
- [ ] Run the Simulator Comparison experiment to compare different architectures
- [ ] Run the Error Filtration experiment to verify the filtering effect
- [ ] Collect all output data for post-processing

---

## Frequently Asked Questions

### Q: How long do the experiments take?

A: It depends on the parameter settings:
- Small parameters (`addrsize=5-10`): a few seconds to a few minutes
- Medium parameters (`addrsize=15-20`): a few minutes to tens of minutes
- Large parameters (`addrsize=25+`): possibly several hours

### Q: How do I enable GPU acceleration?

A: It cannot be enabled at the moment. The CUDA/GPU backend is temporarily disabled in CMake; the option below will be logged as requested, but the build is still CPU-only:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DQRAM_ENABLE_CUDA=ON
```

### Q: Where are the output files?

A: Experiment results are written to standard output (stdout) by default. You can redirect them to a file:
```bash
./Experiment_QRAM_Fidelity [args] > results.txt
```

---

## Contact

If you run into problems reproducing the results, please reach out via:
- Open a GitHub issue
- Email: chenzhaoyun@iai.ustc.edu.cn
