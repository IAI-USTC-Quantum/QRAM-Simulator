# 论文复现文档

本目录包含与论文相关的文档和复现指南。

## 关联论文

- **QRAM-Simulator**: [arXiv:2503.13832](https://arxiv.org/abs/2503.13832) - Efficient Simulation of Quantum Random Access Memory
- **SparQ**: [arXiv:2503.15118](https://arxiv.org/abs/2503.15118) - SparQ: A Sparse Quantum Circuit Simulator with Register-Level Abstraction

## 文件说明

| 文件 | 说明 |
|------|------|
| [reproduction.md](reproduction.md) | 详细的实验复现步骤和命令 |
| README.md | 本文档 |

## 实验代码位置

论文中的数值结果由以下代码生成：

| 实验 | 代码路径 | CMake Target |
|------|----------|--------------|
| QRAM 保真度实验（主模拟 + 性能分析） | `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp` | `Experiment_QRAM_Fidelity` |
| QRAM 模拟器对比（完整版 vs 简化版） | `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp` | `Experiment_QRAM_FidelityV2` |
| 错误过滤实验 | `Experiments/ErrorFiltration/testMultiEFQRAM.cpp` | `Experiment_ErrorFiltration` |

## 快速复现

```bash
# 1. 构建实验
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration

# 2. 运行实验（示例）
./bin/Experiment_QRAM_Fidelity --addrsize 15 --datasize 3 --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 --seed 123456 --version normal
```

详细的参数说明和完整复现流程请参考 [reproduction.md](reproduction.md)。

## 结果说明

实验输出包含：
- 保真度随地址位数、噪声参数的 scaling 数据
- 不同 QRAM 架构的性能对比
- 错误过滤方案的效果评估

原始实验数据已随论文提供。基于这些输出，可以使用标准的数据分析脚本进行绘图和制表。
