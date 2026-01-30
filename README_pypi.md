# SparQ (arXiv:2503.15118v1)

## Introduction

SparQ is a quantum circuit programming tool and simulator based on **sparse quantum states**.

- **Sparse quantum states**: SparQ only processes the non-zero amplitudes of a quantum state.
- **Register-level abstraction**: SparQ operates on quantum states at the **register** level, which allows scalable handling at the qubit level and provides exceptional convenience for simulating arithmetic quantum circuits.
- **Extensibility**: SparQ's architecture is highly flexible and enables fundamental optimizations for special classes of quantum circuits. For example, QFT circuits can be simulated directly using FFT algorithms, achieving much higher efficiency than gate-by-gate simulation; similarly, quantum arithmetic circuits can be simulated via direct arithmetic operations, avoiding the complexity of decomposing them into elementary gates.

## Installation

### Requirements

- Python 3.9–3.13  
- NumPy  

### Optional

- CUDA 12.0+ (recommended, for GPU acceleration)

### Command

```
pip install pysparq
```


# About

## Contributors

This project is developed by the USTC-IAI Quantum Computing Team.

Developers:
- Agony5757 (chenzhaoyun@iai.ustc.edu.cn)
- RichardSun
- Itachixc
- YunJ1e
- cilysad
- TMYTiMidlY

## Related Projects

- [QPanda-lite](https://github.com/Agony5757/QPanda-lite)  
  A third-party NISQ quantum computing toolkit that covers quantum circuit programming, quantum circuit simulation, QASM parsing, OriginIR parsing, quantum circuit compilation, and execution on quantum cloud platforms.