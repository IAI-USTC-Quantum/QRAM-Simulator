# 论文复现文档

本目录包含与论文相关的文档和复现指南。

## Paper 1: QRAM-Simulator — [arXiv:2503.13832](https://arxiv.org/abs/2503.13832)

> *Efficient Simulation of Quantum Random Access Memory*

提出稀疏态 QRAM 模拟器和 Register Level Programming 范式。

**核心贡献**：QRAM 电路模拟（Qutrit/Qubit）、噪声模型、稀疏态优化、错误过滤方案

**实验代码**：

| 实验 | 代码路径 | CMake Target |
|------|----------|--------------|
| QRAM 保真度实验 | `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp` | `Experiment_QRAM_Fidelity` |
| QRAM 模拟器对比 | `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp` | `Experiment_QRAM_FidelityV2` |
| 错误过滤实验 | `Experiments/ErrorFiltration/testMultiEFQRAM.cpp` | `Experiment_ErrorFiltration` |

**复现指南**：[reproduction.md](reproduction.md)

## Paper 2: SparQ — [arXiv:2503.15118](https://arxiv.org/abs/2503.15118)

> *SparQ: A Sparse Quantum Circuit Simulator with Register-Level Abstraction*

将 Register Level Programming 拓展为通用稀疏态模拟器，并提供 Python 接口 (PySparQ)。

**核心贡献**：通用稀疏态模拟器、扩展算法库（QFT、Grover、QDA、QCNN 等）、PySparQ Python API、保留 CUDA 后端代码（当前 CMake 暂时屏蔽 GPU 构建）

**实验代码**：

| 实验 | 代码路径 |
|------|----------|
| QFT | `Experiments/QFT/` |
| Grover | `Experiments/Grover/` |
| 状态准备 | `Experiments/StatePreparation/` |
| 量子微分算法 | `Experiments/QDA/` |
| QCNN | `Experiments/QCNN/` |
| 线性系统求解 | `Experiments/CKS/` |

## 快速复现（Paper 1）

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration

./bin/Experiment_QRAM_Fidelity --addrsize 15 --datasize 3 --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 --seed 123456 --version normal
```
