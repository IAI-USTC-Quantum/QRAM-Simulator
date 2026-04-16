# QRAM-Simulator & PySparQ

[![arXiv](https://img.shields.io/badge/arXiv-2503.13832-b31b1b.svg)](https://arxiv.org/abs/2503.13832)
[![PyPI](https://img.shields.io/pypi/v/pysparq.svg)](https://pypi.org/project/pysparq/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

> **稀疏态量子模拟器，支持 Register Level Programming**

## 双软件架构

本仓库包含两个紧密协作的软件组件，面向不同的用户群体和使用场景：

| 软件 | 定位 | 用户群体 | 核心优势 |
|------|------|----------|----------|
| **QRAM-Simulator** | C++ 稀疏态量子模拟器核心 | 研究人员、高性能计算开发者 | 极致性能、精细控制 |
| **PySparQ** | Python 绑定与高层次 API | 量子算法开发者、教育工作者 | 快速原型、易于上手 |

- **QRAM-Simulator**: 提供底层的稀疏态量子模拟能力，包括量子随机存取存储器 (QRAM) 模拟、量子算术运算、噪声模型等。适用于需要深度优化和高性能的场景。
- **PySparQ**: 通过 pybind11 将 C++ 核心能力暴露给 Python，提供简洁的 API 用于快速开发和验证量子算法。

## Register Level Programming（核心特性）

### 根本性差异

传统量子框架与 SparQ 的核心区别：

| 维度 | 传统方式 | Register Level Programming |
|------|----------|---------------------------|
| **状态存储** | 量子比特数组/张量网络 | `uint64_t` 直接存储寄存器值 |
| **算术运算** | 编译成量子门序列 | 直接对寄存器值进行算术操作 |
| **开发模式** | 自底向上（从门电路构建） | 自顶向下（先写高层模块再细化） |
| **QAdder 实现** | 分解为数百个量子门 | 直接调用加法器，自动门分解 |

### 自顶向下开发流程

```
传统方式：                        SparQ 方式：
┌──────────────┐                 ┌──────────────┐
│ 量子门序列   │                 │ 高层算法模块 │ ← 先写这里
│   ↓ ↓ ↓     │                 │   ↓ 细化    │
│ 振幅计算    │                 │ 寄存器操作   │
└──────────────┘                 │   ↓ 验证    │
                                 │ 回归测试    │
                                 └──────────────┘
```

### 代码示例对比

**传统框架 (Qiskit)**：
```python
# 需要手动构建加法器门电路
adder = DraperQFTAdder(num_state_qubits=4)
circuit.append(adder, [0, 1, 2, 3, 4, 5, 6, 7])
```

**PySparQ**：
```python
import pysparq as ps

# 直接创建寄存器并执行加法
state = ps.SparseState()

# 添加寄存器（返回寄存器 ID）
a_id = ps.AddRegister("a", ps.StateStorageType.UnsignedInteger, 4)(state)
b_id = ps.AddRegister("b", ps.StateStorageType.UnsignedInteger, 4)(state)

# QAdder 直接操作寄存器值，无需编译成门序列
# 将 a + b 存储到输出寄存器
out_id = ps.AddRegister("out", ps.StateStorageType.UnsignedInteger, 5)(state)
ps.Add_UInt_UInt("a", "b", "out")(state)
```

## 与主流量子框架对比

| 特性 | QRAM-Simulator/PySparQ | Qiskit | Cirq | PennyLane |
|------|------------------------|--------|------|-----------|
| **状态表示** | 稀疏态（仅非零振幅） | 稠密态/张量网络 | 稠密态 | 混合态/自动微分 |
| **编程模型** | Register Level | 门序列 | 门序列 | 可微分编程 |
| **模拟规模** | 可达 64+ 量子比特（特定结构） | 通常 20-30 量子比特 | 类似 Qiskit | 类似 Qiskit |
| **算术电路** | 原生寄存器操作 | 需分解为门 | 需分解为门 | 需分解为门 |
| **QRAM 支持** | 原生支持 | 无原生支持 | 无原生支持 | 无原生支持 |
| **GPU 加速** | CUDA 支持 | 有限支持 | 有限支持 | 有限支持 |

### 何时使用 QRAM-Simulator？

✅ **适合使用**：
- 大规模量子算术电路（加法器、乘法器）
- 需要 QRAM 的量子算法（Grover、量子机器学习）
- 稀疏态结构的量子模拟
- 噪声模型研究

❌ **不适合使用**：
- 通用量子电路（门种类复杂、态稠密）
- 需要与真实量子硬件直接交互
- 变分量子算法（需要自动微分）

## PySparQ 快速开始

### 安装

```bash
pip install pysparq
```

**要求**：
- Python 3.9 – 3.13
- NumPy

**可选（推荐）**：
- CUDA 12.0+ （用于 GPU 加速）

### 5 分钟上手示例

```python
import pysparq as ps
import numpy as np

# 1. 创建稀疏态
state = ps.SparseState()

# 2. 定义寄存器（Register Level Programming 的核心）
# AddRegister 返回寄存器 ID，用于后续操作
addr_id = ps.AddRegister("addr", ps.StateStorageType.UnsignedInteger, 4)(state)
data_id = ps.AddRegister("data", ps.StateStorageType.UnsignedInteger, 8)(state)

# 3. 初始化叠加态（所有地址等概率）- Register Level 特性
ps.Hadamard_Int("addr", 4)(state)  # 对 4-bit 寄存器应用 Hadamard
print("After Hadamard:", ps.StatePrint(state))

# 4. 创建 QRAM 并加载数据
memory = [i * 2 for i in range(16)]  # 16个地址，每个8-bit数据
qram = ps.QRAMCircuit_qutrit(4, 8, memory)

# 5. 执行 QRAM 加载：|addr⟩|0⟩ → |addr⟩|memory[addr]⟩
ps.QRAMLoad(qram, "addr", "data")(state)
print("After QRAM Load:", ps.StatePrint(state))

# 6. 直接进行算术操作（无需编译成门！）
# data = data + 5，使用寄存器级别加法
ps.Add_ConstUInt("data", 5)(state)
print("After Add:", ps.StatePrint(state))

# 7. 测量 - 获取某个寄存器的概率分布
prob_data = ps.Prob(state, data_id)
print("Data register probabilities:", prob_data)
```

### Register Level 特性的关键 API

```python
# 量子算术 - 直接寄存器操作（Register Level Programming 核心）
ps.Add_UInt_UInt(in1, in2, out)      # out = in1 + in2
ps.Add_UInt_ConstUInt(reg, const)    # reg = reg + const
ps.Add_ConstUInt(reg, const)         # reg = reg + const（简化版）
ps.Mult_UInt_ConstUInt(in, c, out)   # out = in * c
ps.ShiftLeft(reg, n)                 # 左移 n 位
ps.ShiftRight(reg, n)                # 右移 n 位

# 基础量子门
ps.Hadamard_Int(reg, n_digits)       # 对整数寄存器应用 Hadamard
ps.Xgate_Bool(reg, pos)              # X 门（特定比特位）
ps.Zgate_Bool(reg, pos)              # Z 门

# QRAM 操作
ps.QRAMLoad(qram, addr_reg, data_reg)      # QRAM 加载
ps.QRAMLoadFast(qram, addr_reg, data_reg)  # 快速版本
```

## QRAM-Simulator C++ API

### 构建指南

```bash
# 克隆仓库
git clone https://github.com/Agony5757/QRAM-Simulator.git
cd QRAM-Simulator

# 创建构建目录
mkdir build && cd build

# 配置（CPU 版本）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 配置（CUDA 版本，可选）
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_CUDA=ON

# 编译
make -j$(nproc)
```

### 核心概念

```cpp
#include "SparQ/include/sparse_state_simulator.h"

using namespace qram_simulator;

// 1. 创建系统
System sys;

// 2. 声明寄存器
auto addr_id = sys.create_register("addr", 4);  // 4-bit 地址
auto data_id = sys.create_register("data", 8);  // 8-bit 数据

// 3. 创建稀疏态
std::vector<System> state;
state.emplace_back(sys);

// 4. 应用操作（Register Level!）
Hadamard(addr_id)(state);           // 叠加态
QAdder(data_id, addr_id)(state);    // 直接加法操作

// 5. 测量
auto result = measure(state);
```

### 核心组件

- **SparseState**: 稀疏态存储与操作
- **System**: 寄存器管理和系统配置
- **Operators**: 量子操作（门、算术、QRAM）
  - `basic_gates.h`: 基础量子门
  - `quantum_arithmetic.h`: 量子算术（加减乘除、移位）
  - `qram.h`: QRAM 加载操作
  - `qft.h`: 快速傅里叶变换（优化实现）

### 运行实验

```bash
# QRAM 保真度实验
./build/bin/Experiment_QRAM_Fidelity \
    --addrsize 15 --datasize 3 \
    --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 \
    --seed 123456 --version normal
```

## 论文与引用

### 相关论文

1. **QRAM-Simulator**: [arXiv:2503.13832](https://arxiv.org/abs/2503.13832)
2. **SparQ 稀疏态模拟器**: [arXiv:2503.15118](https://arxiv.org/abs/2503.15118)

### BibTeX

```bibtex
@article{qram_simulator_2025,
  title={Efficient Simulation of Quantum Random Access Memory},
  author={Chen, Zhaoyun and others},
  journal={arXiv preprint arXiv:2503.13832},
  year={2025}
}
```

### 复现论文结果

详细的实验复现指南见 [docs/paper/](docs/paper/) 目录：

- [docs/paper/README.md](docs/paper/README.md) - 论文关联文档
- [docs/paper/reproduction.md](docs/paper/reproduction.md) - 实验运行详细指南

## 项目结构

```
QRAM-Simulator/
├── SparQ/              # C++ 稀疏态模拟器核心
│   ├── include/        # 头文件（运算符、系统操作）
│   └── src/            # 源文件
├── QRAM/               # QRAM 电路实现
│   └── include/        # Qutrit-based QRAM
├── PySparQ/            # Python 绑定
│   └── pysparq/        # Python 包
├── SparQ_Algorithm/    # 高层算法（状态准备、块编码等）
├── Experiments/        # 论文实验代码
│   └── QRAM/           # QRAM 相关实验
├── test/               # 单元测试
└── docs/               # 文档
    └── paper/          # 论文复现文档
```

## 贡献者

本项目由 **USTC-IAI 量子计算团队** 开发。

主要开发者：
- Agony5757 (chenzhaoyun@iai.ustc.edu.cn)
- RichardSun
- Itachixc
- YunJ1e
- cilysad
- TMYTiMidlY

## 相关项目

- [QPanda-lite](https://github.com/Agony5757/QPanda-lite) - NISQ 量子计算工具包

## 许可证

Apache-2.0 License
