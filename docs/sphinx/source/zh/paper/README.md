# 论文复现文档

[精确联合阻尼抽样：算法、旧方法缺陷与通道验证](joint_damping.md)。

qubit QRAM 的完整状态等价性要求、测试覆盖及运行命令见[剪枝与全量演化的精确性验证](exactness_validation.md)。

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

## 理论笔记：Qubit 架构剪枝与快速模拟

- [qubit_qram_pruning.md](qubit_qram_pruning.md)：qubit 编码 QRAM 在振幅阻尼噪声下的分支预测理论。给出 H→K₀→H 结构的闭式解（good 分支数据输出振幅 $(1\pm a^{2n})/2$、Hamming 权重公式）、完整剪枝算法与复杂度分析，并对照现有代码列出实现缺口（`get_multiplier_qubit` 接线、`fill_bad_range` qubit 分支、`_reconstruct` 的 $2^k$ 分量写出）。
- [qubit_error_propagation.md](qubit_error_propagation.md)：qubit 编码的**错误传播机制**理论走读与单错误注入验证。核心结论：qutrit 子树包含判据的真正依据是"路径外分量末态构型同一"（W 守卫冻结故障残留），qubit 因"空闲=指向左"不可区分导致残留沿祖先上拉迁移、构型家族分歧至 subtree(parent(v))——`get_bad_range_qubit` 的左孩子上溯规则正是该分歧包络（n=3 逐例实测吻合）；纯阻尼通道豁免（可用纯子树判据）。

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
