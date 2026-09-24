<p align="center">
  <img src="banner.png" alt="QRAM-Simulator Banner" width="100%">
</p>

# QRAM-Simulator

[![arXiv:QRAM](https://img.shields.io/badge/QRAM_Simulator-arXiv%3A2503%2E13832-b31b1b.svg)](https://arxiv.org/abs/2503.13832)
[![arXiv:SparQ](https://img.shields.io/badge/SparQ-arXiv%3A2503%2E15118-6f42c1.svg)](https://arxiv.org/abs/2503.15118)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![GitHub](https://img.shields.io/badge/GitHub-IAI--USTC--Quantum%2FQRAM--Simulator-181717?logo=github)](https://github.com/IAI-USTC-Quantum/QRAM-Simulator)
[![CI](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/IAI-USTC-Quantum/QRAM-Simulator/actions/workflows/cmake-multi-platform.yml)
[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-4D6AE4)](https://iai-ustc-quantum.github.io/QRAM-Simulator/)

> **QRAM 电路模拟核心（纯 C++ 基座仓）**：Qutrit/Qubit 两种 QRAM 架构、噪声模型与 QRAM 论文实验

## 仓库分工

本仓库是纯 C++ 基座，不含任何 Python 组件；SparQ 框架（稀疏态模拟器、算法库、
Python 绑定、算法类实验）全部位于 SparQSim 仓库：

| 仓库 | 内容 | PyPI 包 |
|------|------|---------|
| **QRAM-Simulator**（本仓库） | C++ QRAM 电路核心（Common + QRAM）+ QRAM 论文实验 | 无（纯 C++） |
| [SparQSim](https://github.com/IAI-USTC-Quantum/SparQSim) | SparQ 框架（稀疏态模拟器、算法库、pysparq 与 qram_simulator 绑定、算法类实验） | `pysparq`、`qram-simulator` |

依赖方向：**SparQSim → QRAM-Simulator**。SparQSim 以 git submodule
（相对 URL `../QRAM-Simulator.git`）方式引用本仓库并编译 C++ 核心；两个仓库各自
独立 tag/发版，SparQSim 发版前将 submodule pin 到本仓库的对应 tag。

## 核心能力

- **QRAM 电路模拟**：Qutrit-based（`qram_circuit_qutrit.h`）与 Qubit-based
  （`qram_circuit_qubit.h`）两种实现
- **噪声模型**：退极化（Depolarizing）与振幅阻尼（Damping），含概率参数范围校验
- **架构剪枝**：qubit 架构 normal（剪枝）模式与 full（不剪枝）模式逐版本对拍
- **公共组件**：稀疏/稠密矩阵、随机引擎、错误处理、state manipulator 等

## C++ 快速开始

### 构建

```bash
git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
cd QRAM-Simulator
mkdir build && cd build

# 配置（CPU 版本；GPU/CUDA 后端当前暂缓，CMake 会构建 CPU-only 版本）
cmake .. -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

构建开关（均可通过 `-D...=ON/OFF` 控制）：

| 开关 | 默认 | 说明 |
|------|------|------|
| `QRAM_BUILD_TESTS` | ON | 构建 C++ 测试（test/） |
| `QRAM_BUILD_EXPERIMENTS` | ON | 构建 QRAM 实验程序（Experiments/） |

### 核心用法

```cpp
#include "qram_circuit_qubit.h"

using namespace qram_simulator;
using namespace qram_qubit;

// 1. 构造 QRAM 电路（addr_size=4, data_size=2）
QRAMCircuit qram(4, 2);
qram.set_memory_random();                     // 随机数据树

// 2. 注入噪声（可选；概率范围校验，Damping 要求 gamma < 1）
qram.set_noise_models({
    { OperationType::Depolarizing, 1e-3 },
    { OperationType::Damping, 1e-4 }
});

// 3. 运行（normal=架构剪枝 / full=不剪枝基准）
qram.set_input_uniform(100);
qram.run_normal();

// 4. 采样保真度
double fidelity = qram.sample_and_get_fidelity();
```

### 作为 CMake 子项目消费（SparQSim 的方式）

```cmake
# submodule: git submodule add ../QRAM-Simulator.git extern/qram-simulator
set(QRAM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(QRAM_BUILD_EXPERIMENTS OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/qram-simulator)   # 提供 SparQ_QRAMSimulator / SparQ_Common 目标与平铺头文件布局
```

### 运行实验

```bash
# QRAM 保真度实验（V2）
./build/bin/Experiment_QRAM_FidelityV2 ...
```

Python 侧（pysparq 全功能绑定与 qram_simulator 薄绑定）见
[SparQSim](https://github.com/IAI-USTC-Quantum/SparQSim) 仓库。

## 论文与引用

本仓库由两篇论文分别驱动：

### Paper 1: QRAM-Simulator — [arXiv:2503.13832](https://arxiv.org/abs/2503.13832)

> *Efficient Simulation of Quantum Random Access Memory*

- **QRAM 电路模拟**：Qutrit-based 与 Qubit-based 两种 QRAM 实现（C++ API），支持退极化和振幅阻尼噪声模型
- **稀疏态优化**：仅存储非零振幅，可实现 64+ 量子比特的结构化算法模拟
- **错误过滤**：针对含噪 QRAM 的错误过滤方案

**对应代码**：`QRAM/`、`Experiments/QRAM/`（错误过滤实验在 SparQSim 仓库）

### Paper 2: SparQ — [arXiv:2503.15118](https://arxiv.org/abs/2503.15118)

> *SparQ: A Sparse Quantum Circuit Simulator with Register-Level Abstraction*

将 Register Level Programming 拓展为通用稀疏态量子模拟器（QFT、Grover、哈密顿量
模拟、QDA、QCNN 等算法与扩展算法库）。

**对应代码**：`SparQ/`、`SparQ_Algorithm/` 与算法类实验已迁至
[SparQSim](https://github.com/IAI-USTC-Quantum/SparQSim) 仓库。

### BibTeX

```bibtex
@article{sun2025sparqsim,
  title={SparQSim: Simulating Scalable Quantum Algorithms via Sparse Quantum State Representations},
  author={Sun, Tai-Ping and Chen, Zhao-Yun and Wang, Yun-Jie and Xue, Cheng and Liu, Huan-Yu and Zhuang, Xi-Ning and Xu, Xiao-Fan and Wu, Yu-Chun and Guo, Guo-Ping},
  journal={arXiv preprint arXiv:2503.15118},
  year={2025}
}

@article{wang2025refined,
  title={Refined Criteria for QRAM Error Suppression via Efficient Large-Scale QRAM Simulator},
  author={Wang, Yun-Jie and Sun, Tai-Ping and Zhuang, Xi-Ning and Xu, Xiao-Fan and Liu, Huan-Yu and Xue, Cheng and Wu, Yu-Chun and Chen, Zhao-Yun and Guo, Guo-Ping},
  journal={arXiv preprint arXiv:2503.13832},
  year={2025}
}
```

### 复现论文结果

详细的实验复现指南见 [docs/paper/](docs/paper/) 目录：

- [docs/paper/README.md](docs/paper/README.md) - 论文关联文档
- [docs/paper/reproduction.md](docs/paper/reproduction.md) - [QRAM-Simulator](https://arxiv.org/abs/2503.13832) 实验复现指南

## 项目结构

```
QRAM-Simulator/
├── QRAM/               # QRAM 电路实现（Qutrit/Qubit-based）
├── Common/             # 公共组件（矩阵、随机引擎、state manipulator 等）
├── Experiments/        # QRAM 论文实验（QRAMFidelityV2、QubitPaper、ChannelCorrespondence 等）
├── test/               # C++ 测试（Common/QRAM 归属部分）
├── ThirdParty/         # vendored 依赖（Eigen、fmt、googletest、argparse）
└── docs/               # 文档（Doxygen、论文复现）
```

## 发版流程

1. 在 Gitea（开发主仓）合并变更到 main；
2. 同步到 GitHub 上游 `IAI-USTC-Quantum/QRAM-Simulator`；
3. 更新 `CHANGELOG.md`，打 tag（`vX.Y.Z`，注意历史上已有 v0.1.x，新系列从 v0.2.0 起）；
4. push tag 或创建 GitHub Release（本仓库为纯 C++ 源码仓，不发布 PyPI 包；
   `pysparq` / `qram-simulator` 的 wheel 由 SparQSim 仓库发布）。

## About Us

本项目由 **[IAI-USTC Quantum](https://github.com/IAI-USTC-Quantum)** 开发。

- **GitHub Organization**: [IAI-USTC-Quantum](https://github.com/IAI-USTC-Quantum) — 查看团队的所有开源项目
- **Documentation**: [iai-ustc-quantum.github.io](https://iai-ustc-quantum.github.io/) — 团队文档与项目主页

IAI-USTC Quantum 是合肥综合性国家科学中心人工智能研究院（Institute of Artificial Intelligence, Hefei Comprehensive National Science Center）量子人工智能团队。

主要开发者：Agony5757 (chenzhaoyun@iai.ustc.edu.cn)、RichardSun、Itachixc、YunJ1e、cilysad、TMYTiMidlY

## 相关项目

- [SparQSim](https://github.com/IAI-USTC-Quantum/SparQSim) - SparQ 框架与 Python 生态（pysparq、qram_simulator）
- [UnifiedQuantum](https://github.com/IAI-USTC-Quantum/UnifiedQuantum) - 统一量子计算框架

## 许可证

Apache-2.0 License
