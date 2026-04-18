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

- **`QuantumState`**：稀疏态表示，仅存储非零振幅的基态
- **`QuantumGate`** 和 **`SparseGate`**：门操作的抽象基类和稀疏优化实现
- **`QuantumOperator`**：算子的统一接口，支持复合算子和条件算子

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
┌─────────────────────────────────────────────────────────┐
│                    QuantumState                          │
├─────────────────────────────────────────────────────────┤
│  稀疏存储: map<QIndex, Complex> amplitudes              │
│  - QIndex: 基态的整数索引 (0 到 2^n-1)                  │
│  - Complex: 复数振幅                                    │
│  - 仅存储 |amplitude| > ε 的非零项                      │
└─────────────────────────────────────────────────────────┘
```

**关键特性**：
- 内存使用：O(k × n)，k 为非零项数，n 为量子比特数
- 对比稠密表示的 O(2^n)，可实现更大规模的模拟

## 关键设计决策

### 为什么使用稀疏态模拟

1. **内存效率**：稀疏表示内存使用与纠缠程度成正比
2. **计算效率**：门操作只需处理非零振幅的基态
3. **适用场景**：QRAM 操作、量子漫步、稀疏哈密顿量模拟

### Qutrit-based vs Qubit-based QRAM

- **Qutrit-based**：更自然的树结构表示，更高效的电路深度
- **Qubit-based**：硬件兼容性更好，更成熟的理论

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
