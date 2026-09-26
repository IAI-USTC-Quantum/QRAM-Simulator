# Paper Reproduction Documentation

[Exact joint damping: algorithm, legacy defects and channel validation](joint_damping.md).

For the qubit-QRAM complete-state equivalence protocol, test coverage and commands, see [Pruned/full exactness validation](exactness_validation.md).

This directory contains paper-related documentation and reproduction guides.

## Paper 1: QRAM-Simulator — [arXiv:2503.13832](https://arxiv.org/abs/2503.13832)

> *Efficient Simulation of Quantum Random Access Memory*

Proposes the sparse-state QRAM simulator and the Register Level Programming paradigm.

**Core contributions**: QRAM circuit simulation (qutrit/qubit), noise models, sparse-state optimization, and the error filtration scheme

**Experiment code**:

| Experiment | Code path | CMake Target |
|------|----------|--------------|
| QRAM fidelity experiment | `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp` | `Experiment_QRAM_Fidelity` |
| QRAM simulator comparison | `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp` | `Experiment_QRAM_FidelityV2` |
| Error filtration experiment | `Experiments/ErrorFiltration/testMultiEFQRAM.cpp` | `Experiment_ErrorFiltration` |

**Reproduction guide**: [reproduction.md](reproduction.md)

## Theory Notes: Pruning and Fast Simulation for the Qubit Architecture

- [qubit_qram_pruning.md](qubit_qram_pruning.md): the theory of branch prediction for the qubit-encoded QRAM under amplitude damping noise. Derives the closed-form solution of the H→K₀→H structure (good-branch data output amplitudes $(1\pm a^{2n})/2$, the Hamming-weight formula), presents the complete pruning algorithm and complexity analysis, and lists the implementation gaps against the current code (`get_multiplier_qubit` wiring, the qubit branch of `fill_bad_range`, the $2^k$-component write-out of `_reconstruct`).
- [qubit_error_propagation.md](qubit_error_propagation.md): a theoretical walkthrough of the qubit encoding's **error propagation mechanism**, validated by single-error injection. Core conclusion: the qutrit subtree-containment criterion truly rests on "off-path components ending in the same final tree configuration" (the W guard freezes fault residues), whereas the qubit encoding cannot distinguish "idle" from "pointing left", so residues migrate upward along ancestors and configuration families diverge up to subtree(parent(v)) — the left-child climb rule of `get_bad_range_qubit` is exactly the envelope of this divergence (verified case by case at n=3); the pure damping channel is exempt (the pure subtree criterion applies).

## Paper 2: SparQ — [arXiv:2503.15118](https://arxiv.org/abs/2503.15118)

> *SparQ: A Sparse Quantum Circuit Simulator with Register-Level Abstraction*

Extends Register Level Programming into a general-purpose sparse-state simulator and provides a Python interface (PySparQ).

**Core contributions**: a general-purpose sparse-state simulator, an extended algorithm library (QFT, Grover, QDA, QCNN, etc.), the PySparQ Python API, and the retained CUDA backend code (GPU builds are currently disabled in CMake)

**Experiment code**:

| Experiment | Code path |
|------|----------|
| QFT | `Experiments/QFT/` |
| Grover | `Experiments/Grover/` |
| State preparation | `Experiments/StatePreparation/` |
| Quantum differentiation algorithm | `Experiments/QDA/` |
| QCNN | `Experiments/QCNN/` |
| Linear system solving | `Experiments/CKS/` |

## Quick Reproduction (Paper 1)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration

./bin/Experiment_QRAM_Fidelity --addrsize 15 --datasize 3 --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 --seed 123456 --version normal
```
