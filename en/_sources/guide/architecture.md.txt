# QRAM-Simulator Architecture

This document describes the overall architecture, design philosophy, and core modules of the QRAM-Simulator repository.

> Repository split: this repository is the **pure C++ QRAM simulation foundation** (with a thin pybind11 binding and the `qram-simulator` PyPI package); the sparse-state simulator framework (SparQ), the higher-level algorithms, and the rich `pysparq` bindings live in the [SparQSim repository](https://github.com/IAI-USTC-Quantum/SparQSim), which consumes this repository as a git submodule (`extern/qram-simulator`).

---

## 1. Overview

### 1.1 Project Goals

QRAM-Simulator is a high-performance C++ core for simulating **quantum random access memory (QRAM) loading circuits**:

- **Dual qubit / qutrit architectures**: two QRAM implementations — a physical qubit tree and a three-level node tree
- **Noise aware**: bit-flip / phase-flip / combined-flip / depolarizing / amplitude-damping noise injected per time slice
- **Branch pruning**: good/bad branch pruning shrinks the simulation to "all bad branches computed + one good branch as the baseline"
- **Python bindings**: a thin pybind11 layer (`pip install qram-simulator`) exports every core workhorse class
- The CUDA/GPU backend is retained in the code base; CMake currently masks GPU builds, so the default is CPU-only

### 1.2 Design Philosophy

1. **Sparse state first**: the tree state records only non-basis-state nodes, so complexity is on the order of the number of nonzero elements rather than 2^n
2. **Branch as trajectory**: the (address, bus_input) input branch is the unit of simulation, and noise accumulates on the trajectory amplitudes
3. **Performance and usability together**: the C++ core (OpenMP/TBB parallelism) provides performance, the Python bindings provide usability
4. **Self-contained build**: Eigen / fmt / argparse are all vendored under `ThirdParty/`; the build needs no network access

---

## 2. Directory Layout and Modules

```
QRAM-Simulator/
├── Common/              # shared infrastructure (static library SparQ_Common)
│   ├── include/         # full-amplitude bridge, matrices, random engine, logging, iteration utilities…
│   └── src/
├── QRAM/                # QRAM circuit core (static library SparQ_QRAMSimulator)
│   ├── include/         # qubit/qutrit circuits, branch structures, time-step scheduling
│   └── src/
├── bindings/python/     # thin pybind11 bindings (qram-simulator PyPI package)
├── Experiments/         # QRAM paper experiment executables
├── test/                # C++ tests (in-house TEST macro framework)
├── docs/                # this documentation (Sphinx) + Doxygen configuration
└── ThirdParty/          # eigen / fmt / argparse / googletest (vendored)
```

### 2.1 Common/ — Shared Infrastructure

| File | Purpose |
|------|---------|
| `typedefs.h` | Common type aliases (`complex_t` / `memory_t` / `bus_t` / `u22_t`), qutrit level constants (W/L/R) |
| `basic.h` | Bit-manipulation utilities (`pow2` / `bitcount` / complement), fixed-point encoding, fidelity, random data-tree filling |
| `matrix.h` | Fixed-point sparse matrix `SparseMatrix`, dense matrix/vector `DenseMatrix` / `DenseVector`, and conversions to and from Eigen |
| `state_manipulator.h` | `QRAMFullAmp`: bridge between QRAM circuits and full-amplitude state vectors |
| `simple_quantum_simulator.h` | Simple full-amplitude circuit primitives (single-qubit gates / measurement / qubit-order utilities) |
| `random_engine.h` | Global MT19937-64 singleton (a fixed seed makes the whole simulation chain reproducible) |
| `logger.h` | File logging, timers, an RAII function profiler, online statistics, experiment result writers |
| `error_handler.h` | Unified exception family and the `TEST` / `TEST_FAIL` test macros |
| `iterable.h` | Python-style `range` and Cartesian-product `product` iterators |

### 2.2 QRAM/ — QRAM Circuit Core

| File | Contents |
|------|----------|
| `qram_circuit_qubit.h/.cpp` | `qram_qubit::QRAMCircuit`: the qubit-architecture circuit (main entry point of pruned simulation) |
| `qram_circuit_qutrit.h/.cpp` | `qram_qutrit::QRAMCircuit`: the qutrit-architecture circuit |
| `qram_branch_qubit.h/.cpp` | `State` / `SystemState` / `Branch` / `BranchGroup` (qubit branch structures) |
| `qram_branch_qutrit.h/.cpp` | `QRAMNode` / `QRAMState` / `SubBranch` / `Branch` (qutrit branch structures) |
| `time_step.h/.cpp` | `TimeStep` time-step and noise scheduler, the `OperationType` enum, and the `TimeSlices` schedule structure |

Both architectures share the same `TimeStep` scheduler (distinguished by the `arch_qubit` / `arch_qutrit` constants); their interface shapes are identical, so they can be used interchangeably:

```cpp
qram_simulator::qram_qubit::QRAMCircuit  qram_q(4, 2);   // qubit architecture
qram_simulator::qram_qutrit::QRAMCircuit qram_t(4, 2);   // qutrit architecture
```

### 2.3 bindings/python/ — Python Binding Layer

`core_binding.cpp` exports the core workhorse classes through pybind11, and scikit-build-core handles the packaging:

| C++ Class | Python Name | Description |
|-----------|-------------|-------------|
| `qram_qubit::QRAMCircuit` | `QRAMCircuitQubit` | qubit-architecture circuit |
| `qram_qutrit::QRAMCircuit` | `QRAMCircuitQutrit` | qutrit-architecture circuit |
| `QRAMFullAmp` | `QRAMFullAmp` | full-amplitude state-vector bridge (entry point for external library integration) |
| `TimeStep` / `TimeSlices` / `OperationPack` / `Operation` | same names | time-step and noise scheduling |
| `OperationType` + `arch_qubit` / `arch_qutrit` | same names | noise-model keys and architecture constants |
| `random_engine::set_seed` / `get_seed` | `set_seed` / `get_seed` | global random seed control |

The `QRAM_BUILD_PYTHON_BINDINGS` build toggle (OFF by default) keeps embedding consumers (SparQSim's `add_subdirectory`) and the pure C++ CI unaffected.

### 2.4 Experiments/ — Paper Experiments

| Experiment | Description |
|------------|-------------|
| `verify_noisy_simulation` | CI regression cross-check: QRAM circuit-level noise ↔ operator-level channels (registered with ctest) |
| `QRAM/QubitPaper` | figure reproduction for the arXiv:2503.13832 paper (PRA version) |
| `QRAM/ChannelCorrespondence` | audit of the correspondence between noise models and circuit-level channels |
| `QRAM/QRAMFidelityV2` | fidelity simulator comparison |
| `QRAM/TimeStep_BadRange` | verification of the bad-branch range theory |

---

## 3. Data Flow

### 3.1 Sparse Tree-State Representation

```
┌──────────────────────────────────────────────────────────────┐
│ State (qubit)                  QRAMState (qutrit)            │
├──────────────────────────────────────────────────────────────┤
│ std::set<size_t> nz_elements     std::map<size_t,QRAMNode>   │
│  ├ records only |1> node         ├ records only nodes not    │
│  │   positions                   │   in state (W,0)          │
│  └ everything else implied |0>   └ node carries (addr,data)  │
│                                                              │
│ complete binary tree, level-order numbering:                 │
│ left = 2i+1, right = 2i+2                                    │
└──────────────────────────────────────────────────────────────┘
```

Both memory and compute complexity are on the order of the number of nonzero (non-basis-state) nodes, not 2^n.

### 3.2 QRAM Loading Workflow

```
construct QRAMCircuit(addr_size, data_size[, memory])
        │
        ▼
set_noise_models({OperationType: probability})   ← optional; probability ∈ [0,1],
        │                                          Damping requires γ < 1
        ▼
set_input_random/uniform(n)                      ← sample n (addr, bus) input branches
        │
        ▼
initialize_system()                              ← TimeStep::generate builds noisy TimeSlices
        │
        ▼
run_normal() / run_full() / run(version)
   ├ run_normal: all bad branches computed + one good branch as the baseline
   │             (remaining good branches predicted by XOR mirroring)
   └ run_full  : every branch evolved, unpruned ground truth
        │
        ▼
sample_and_get_fidelity()                        ← sample output branches, |<ideal|actual>|²
```

### 3.3 Noise Injection

`TimeStep::noise_one_step` inserts noise operations after each time slice according to the noise model:

| Noise | OperationType | Effect |
|-------|---------------|--------|
| Bit flip | `BitFlip` | trajectory splitting: flip / no-flip, amplitudes weighted by √p |
| Phase flip | `PhaseFlip` | trajectory amplitude multiplied by −1 (with probability p) |
| Combined flip | `BitPhaseFlip` | flips bit and phase simultaneously |
| Depolarizing | `Depolarizing` | random Pauli mixture |
| Amplitude damping | `Damping` | relative good-branch multiplier (1−γ)^Δn; two injection granularities, `Damp_Common` / `Damp_Full` |
| Set to zero | `SetZero` | node reset to the basis state |

### 3.4 Full-Amplitude Bridge (QRAMFullAmp)

```
full-amplitude state vector ──_set_branches──▶ input branch set (split by addr/data bits)
      │                                │
      │                        qutrit QRAMCircuit::run
      │                        (requires a non-empty noise model)
      │                                │
      │                        sample_output (trajectory sampling)
      ▼                                ▼
new state vector ◀──_reconstruct──────── branch trajectories mapped back onto the state vector
(good branches predicted by XOR mirroring + Damping multipliers; renormalization check after reconstruction)
```

---

## 4. Key Design Decisions

### 4.1 Qutrit-based vs Qubit-based QRAM

**Qutrit architecture**: every tree node is a three-level system (addr ∈ {W,L,R}, data ∈ {0,1}); routing is performed by A1/A2 rotation gates (carrying a w phase). The tree structure matches the schedule naturally, with fewer gates and lower noise accumulation.

**Qubit architecture**: nodes are standard two-level bits, with better hardware compatibility and a more mature theoretical toolkit. Combined with **good/bad branch pruning** — bad-branch address ranges are derived from `TimeStep::get_bad_range_*`, so the branch group of a good address materializes only one baseline trajectory and predicts the rest by XOR mirroring — this is the core speedup of the qubit architecture for large-scale simulation (see [qubit QRAM pruning theory](../paper/qubit_qram_pruning.md)).

The two implementations share the same `TimeStep` scheduler and noise-model format, and their interface shapes are identical.

### 4.2 Versioned Run Entry Points

`run(version)` accepts strings such as `"full"` / `"normal"` / `"fast"`. The qutrit architecture requires a non-empty noise model before `run(version)` may be used (noise-free scenarios should be handled directly outside the circuit), while `run_normal()` / `run_full()` carry no such constraint.

### 4.3 CUDA Integration (Retained Code)

`QRAM/include/cuda/qram_circuit_qutrit.cuh` (`CuQRAMCircuit`, a thrust `device_vector` memory mirror) is retained in the code base; for the duration of the CondRot primitive refactor, CMake forces `CUDA_FOUND FALSE`, so the default is CPU-only (OpenMP required, TBB an optional accelerator).

---

## 5. References

- Quickstart: [quickstart.md](quickstart.md)
- Installation guide: [installation.md](installation.md)
- Paper 1: [arXiv:2503.13832](https://arxiv.org/abs/2503.13832)
- Paper 2: [arXiv:2503.15118](https://arxiv.org/abs/2503.15118)
