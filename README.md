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

> **稀疏态量子模拟器核心（C++），支持 Register Level Programming 与原生 QRAM**

## 仓库分工

本仓库是 C++ 核心引擎，独立发版。Python 生态拆分为两个包：

| 仓库 | 内容 | PyPI 包 |
|------|------|---------|
| **QRAM-Simulator**（本仓库） | C++ 稀疏态模拟器核心 + 薄 Python 绑定 | `qram-simulator`（import `qram_simulator`） |
| [SparQSim](https://github.com/IAI-USTC-Quantum/SparQSim) | pysparq 全功能 Python 框架（算法、RIR、动态算子） | `pysparq` |

SparQSim 以 git submodule（相对 URL `../QRAM-Simulator.git`）方式引用本仓库并编译
C++ 核心；两个仓库各自独立 tag/发版，pysparq 发版前将 submodule pin 到本仓库的
对应 tag。

## Register Level Programming（核心特性）

与传统量子框架"逐门构建电路"不同，QRAM-Simulator 直接在 **寄存器** 层面编程：

```cpp
// 传统方式：手动构建加法器门电路（数十个门）
// Register Level：一行完成
Add_UInt_UInt("a", "b", "result")(state);   // result = a + b
```

- 以 `uint64_t` 寄存器直接存储替代逐门构建，自顶向下开发量子算法
- 仅存储非零振幅（稀疏态），可实现 64+ 量子比特的结构化算法模拟
- 原生 QRAM（qutrit/qubit 两种实现），支持退极化与振幅阻尼噪声模型

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
| `QRAM_BUILD_TESTS` | ON | 构建 C++ 测试（SparQ/test、test/），需要 vendored googletest |
| `QRAM_BUILD_EXPERIMENTS` | ON | 构建实验程序（Experiments/） |
| `QRAM_BUILD_PYTHON_BINDINGS` | OFF | 构建 `qram_simulator` 薄绑定（需要 pybind11） |
| `BUILD_EXAMPLES` | ON | 构建 examples/ 示例程序 |

### 核心概念

```cpp
#include "SparQ/include/sparse_state_simulator.h"

using namespace qram_simulator;

// 1. 创建稀疏态
SparseState state;

// 2. 声明寄存器
auto addr_id = AddRegister("addr", UnsignedInteger, 4)(state);  // 4-bit 地址
auto data_id = AddRegister("data", UnsignedInteger, 8)(state);  // 8-bit 数据

// 3. 初始化寄存器（指定值）
Init_Unsafe("addr", 3)(state);
Init_Unsafe("data", 5)(state);

// 4. 应用操作（Register Level!）
Hadamard_Int_Full(addr_id)(state);        // 叠加态：|3⟩ → (|0⟩+|1⟩+...+|15⟩)/√16
(StatePrint(Detail))(state);
// 输出：16 个叠加态，每个基态振幅 0.25

Add_UInt_UInt(addr_id, data_id, addr_id)(state);  // addr = addr + data = 3+5 = 8
(StatePrint(Detail))(state);
// 输出：addr=|8>, data=|5>, 所有振幅 1.0
```

### 核心组件

- **SparseState**: 稀疏态存储与操作
- **System**: 寄存器管理和系统配置
- **Operators**: 量子操作（门、算术、QRAM）
  - `basic_gates.h`: 基础量子门
  - `quantum_arithmetic.h`: 量子算术（加减乘除、移位）
  - `qram.h`: QRAM 加载操作
  - `qft.h`: 快速傅里叶变换（优化实现）

### 作为 CMake 子项目消费（SparQSim 的方式）

```cmake
# submodule: git submodule add ../QRAM-Simulator.git extern/qram-simulator
set(QRAM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(QRAM_BUILD_EXPERIMENTS OFF CACHE BOOL "" FORCE)
set(QRAM_BUILD_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/qram-simulator)   # 提供 SparQ 目标与全局 include 路径
```

### 运行实验

```bash
# QRAM 保真度实验
./build/bin/Experiment_QRAM_Fidelity \
    --addrsize 15 --datasize 3 \
    --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 \
    --seed 123456 --version normal
```

## qram_simulator 薄 Python 绑定

本仓库自带一个刻意最小化的 Python API（`bindings/python/`），只绑核心原语：
`System`/`SparseState` 寄存器管理、Init/Hadamard/X、基础算术（Add 族）、QFT、
`MeasureZ`/`Reset`/`Probability`、`PartialTrace`、`QRAMCircuit_qutrit`/`QRAMLoad`、
`StatePrint`。

```bash
pip install qram-simulator
```

```python
from qram_simulator import System, SparseState, StateStorageType
from qram_simulator import Init_Unsafe, Hadamard_Int, MeasureZ, set_seed

System.add_register("q", StateStorageType.UnsignedInteger, 4)
state = SparseState()
Init_Unsafe("q", 5)(state)
Hadamard_Int("q", 4)(state)
set_seed(0)
outcome, prob = MeasureZ("q")(state)
```

全功能 Python API（算子条件控制、算法库、RIR 解释器、动态算子编译等）见
[pysparq](https://pypi.org/project/pysparq/)（SparQSim 仓库）。

## 论文与引用

本仓库由两篇论文分别驱动，各自贡献了不同的核心能力：

### Paper 1: QRAM-Simulator — [arXiv:2503.13832](https://arxiv.org/abs/2503.13832)

> *Efficient Simulation of Quantum Random Access Memory*

面向 QRAM 模拟的稀疏态量子模拟器，提出了 **Register Level Programming** 范式。主要贡献：

- **QRAM 电路模拟**：Qutrit-based 与 Qubit-based 两种 QRAM 实现（C++ API），支持退极化和振幅阻尼噪声模型
- **Register Level Programming**：以 `uint64_t` 寄存器直接存储替代逐门构建，支持自顶向下的量子算法开发
- **稀疏态优化**：仅存储非零振幅，可实现 64+ 量子比特的结构化算法模拟
- **错误过滤**：针对含噪 QRAM 的错误过滤方案

**对应代码**：`QRAM/`、`Experiments/QRAM/`、`Experiments/ErrorFiltration/`

### Paper 2: SparQ — [arXiv:2503.15118](https://arxiv.org/abs/2503.15118)

> *SparQ: A Sparse Quantum Circuit Simulator with Register-Level Abstraction*

将 Register Level Programming 拓展为通用稀疏态量子模拟器。主要贡献：

- **通用稀疏态模拟器**：支持 QFT、Grover、哈密顿量模拟、量子微分算法（QDA）、QCNN 等多种算法
- **扩展算法库**：状态准备、块编码、量子游走、线性系统求解等高层算法
- **CPU-only 构建**：CUDA/GPU 后端当前暂时屏蔽，待 CondRot primitive 重构稳定后恢复

**对应代码**：`SparQ/`、`SparQ_Algorithm/`、`Experiments/QFT/`、`Experiments/Grover/`、`Experiments/QDA/`、`Experiments/QCNN/`

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
├── SparQ/              # C++ 稀疏态模拟器核心
│   ├── include/        # 头文件（运算符、系统操作）
│   └── src/            # 源文件
├── QRAM/               # QRAM 电路实现（Qutrit/Qubit-based）
├── Common/             # 公共组件（矩阵、随机引擎等）
├── SparQ_Algorithm/    # 高层算法（状态准备、块编码等）
├── bindings/python/    # qram_simulator 薄 Python 绑定
├── Experiments/        # 论文实验代码
├── test/               # C++ 单元测试（googletest 与自研断言）
├── ThirdParty/         # vendored 依赖（Eigen、fmt、googletest、argparse）
└── docs/               # 文档（Doxygen、论文复现）
```

## 发版流程

1. 在 Gitea（开发主仓）合并变更到 main；
2. 同步到 GitHub 上游 `IAI-USTC-Quantum/QRAM-Simulator`；
3. 更新 `CHANGELOG.md`，打 tag（`vX.Y.Z`，注意历史上已有 v0.1.x，新系列从 v0.2.0 起）；
4. push tag 或创建 GitHub Release → `pypi-publish` 工作流自动构建
   cp310–313 × (manylinux / win_amd64) wheel + sdist 并发布到 PyPI
   （Trusted Publishing / OIDC）。

## About Us

本项目由 **[IAI-USTC Quantum](https://github.com/IAI-USTC-Quantum)** 开发。

- **GitHub Organization**: [IAI-USTC-Quantum](https://github.com/IAI-USTC-Quantum) — 查看团队的所有开源项目
- **Documentation**: [iai-ustc-quantum.github.io](https://iai-ustc-quantum.github.io/) — 团队文档与项目主页

IAI-USTC Quantum 是合肥综合性国家科学中心人工智能研究院（Institute of Artificial Intelligence, Hefei Comprehensive National Science Center）量子人工智能团队。

主要开发者：Agony5757 (chenzhaoyun@iai.ustc.edu.cn)、RichardSun、Itachixc、YunJ1e、cilysad、TMYTiMidlY

## 相关项目

- [SparQSim](https://github.com/IAI-USTC-Quantum/SparQSim) - pysparq 全功能 Python 框架
- [UnifiedQuantum](https://github.com/IAI-USTC-Quantum/UnifiedQuantum) - 统一量子计算框架

## 许可证

Apache-2.0 License
