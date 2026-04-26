# QRAM Simulator 架构

本文档描述 QRAM Simulator 项目的整体架构、设计哲学和核心模块。

## 概述

### 项目目标

QRAM Simulator 是一个用于模拟**量子随机存取存储器（QRAM）**和**稀疏态量子计算**的高性能模拟器。它旨在为量子算法研究者和开发者提供一个能够：

- 高效模拟大规模量子系统的工具（利用稀疏态表示）
- 精确建模 QRAM 电路的行为和噪声影响
- 支持 GPU 加速以获得更高的计算性能
- 提供简洁的 Python API 便于快速原型开发

### 设计哲学

1. **稀疏态优先**：利用量子态的稀疏性，将存储和计算复杂度从指数级降低到与纠缠程度成正比
2. **模块化设计**：清晰的模块边界，便于扩展和维护
3. **性能与易用性并重**：C++ 核心提供性能，Python 绑定提供易用性
4. **噪声感知**：原生支持去极化和退相干噪声模型

## 核心模块

```
QRAM-Simulator/
├── Common/           # 共享基础设施
├── SparQ/            # 稀疏态模拟器核心
├── QRAM/             # QRAM 电路实现
├── SparQ_Algorithm/  # 高层量子算法
├── PySparQ/          # Python 绑定
├── Experiments/      # 实验代码
└── ThirdParty/       # 第三方依赖（Eigen）
```

### Common/ - 共享基础设施

**职责**：提供跨模块的通用工具和数据结构

| 文件 | 功能 |
|------|------|
| `basic.h` | 基础数学工具、复数运算、哈希函数 |
| `matrix.h` | 稀疏/稠密矩阵的 Eigen 封装和扩展 |
| `logger.h` | 日志系统，支持分级日志和性能计时 |
| `error_handler.h` | 统一的错误处理和异常定义 |
| `typedefs.h` | 项目通用类型别名（QIndex、Complex 等） |

### SparQ/ - 稀疏态模拟器核心

**职责**：实现稀疏态量子模拟的核心功能

**核心类**：

- **`System`**：单个计算基态，包含复数振幅 ``amplitude`` 和寄存器值数组 ``registers``
- **`SparseState`**：稀疏量子态，托管 ``std::vector<System>``，默认构造创建 ``|0...0⟩`` 初态
- **`BaseOperator`**：算子的统一接口，支持复合算子和条件算子
- **`SelfAdjointOperator`**：自伴算子（``dag() == operator()``），如 Hadamard、Pauli-X

### QRAM/ - QRAM 电路实现

**核心类**：

- **`QRAMCircuit`（qutrit/qubit）**：QRAM 电路实现
  - 地址线和数据线位数配置
  - 时步逻辑：生成 QRAM 操作序列
  - 噪声模型：支持去极化和退相干噪声

- **`CuQRAMCircuit`**（CUDA 版本）：GPU 加速实现

### PySparQ/ - Python 绑定

通过 pybind11 提供完整 Python API，包括：

- `QuantumState` → Python 类
- `QuantumGate` 及其子类 → Python 类
- `QRAMCircuit_qutrit/qubit` → Python 类
- 所有算子（`QRAMLoad`、`QRAMLoadFast` 等）
- 类型提示文件（`_core.pyi`）提供 IDE 支持

## 量子态表示

```
┌──────────────────────────────────────────────────────────────┐
│                      SparseState                              │
├──────────────────────────────────────────────────────────────┤
│  稀疏存储: std::vector<System> basis_states                  │
│  - 每个 System = { amplitude: complex, registers: uint64_t[] }│
│  - 寄存器值组合唯一（干涉时振幅相加去重）                    │
│  - 仅存储 |amplitude| > ε 的非零基态                         │
│  - 默认构造创建 |0...0⟩ 初态                                 │
├──────────────────────────────────────────────────────────────┤
│  System（静态共享元数据）                                     │
│  - name_register_map: [(name, type, size, status), ...]      │
│  - 寄存器以 uint64_t 存储，支持 |a⟩|b⟩|c⟩ 多寄存器编码     │
└──────────────────────────────────────────────────────────────┘
```

**关键特性**：
- 内存使用：O(k × r)，k 为非零基态数，r 为寄存器数
- 对比稠密表示的 O(2^n)，可实现更大规模的模拟
- 寄存器级操作：AddRegister ≈ ⊗|0⟩，RemoveRegister ≈ PartialTrace

## 关键设计决策

### 为什么使用稀疏态模拟

1. **内存效率**：稀疏表示内存使用与纠缠程度成正比
2. **计算效率**：门操作只需处理非零振幅的基态
3. **适用场景**：QRAM 操作、量子漫步、稀疏哈密顿量模拟

### Qutrit-based vs Qubit-based QRAM

两者的核心区别在于 **address 编码方式**，进而导致不同的 error scaling：

- **Qutrit-based**（0/1/wait 三态编码）：address 存在关闭（wait）状态
  - 经典信息以 **classical controlled X** 方式编码
  - Z-basis 直接传下去，无需基变换
  - Error scaling: ε ∈ O(L²)，L 为 layer number

- **Qubit-based**（0/1 两态编码）：address 无关闭状态，默认常开（向左 routing）
  - 无法直接将 data qubit 传下去，需要将 data 换到 **Pauli-X basis**
  - 经典信息以 **classical controlled Z** 方式传入
  - Error scaling: ε ∈ O(L³)，L 为 layer number

## 扩展性

### 添加新量子门

1. 在 `SparQ/include/` 创建新的头文件
2. 继承 `QuantumGate` 或 `SparseGate` 基类
3. 在 `SparQ/src/` 实现 `apply()` 方法

### 添加新算法

1. 在 `SparQ_Algorithm/include/` 创建算法头文件
2. 继承 `QuantumAlgorithm` 基类
3. 实现 `prepare()`、`run()` 和 `measure()` 方法

### 添加 Python 绑定

1. 在 `PySparQ/src/pybind_wrapper.cpp` 添加绑定代码
2. 在 `_core.pyi` 添加类型提示
3. 重新构建 PySparQ 模块
