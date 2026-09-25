# QRAM-Simulator 架构

本文档描述 QRAM-Simulator 仓库的整体架构、设计哲学与核心模块。

> 仓库分工：本仓库是**纯 C++ QRAM 模拟基座**（含 pybind11 薄绑定与
> `qram-simulator` PyPI 包）；稀疏态模拟器框架（SparQ）、高层算法与
> `pysparq` 富绑定位于 [SparQSim 仓库](https://github.com/IAI-USTC-Quantum/SparQSim)，
> 后者以 git submodule（`extern/qram-simulator`）形式消费本仓库。

---

## 1. 概述

### 1.1 项目目标

QRAM-Simulator 是用于模拟**量子随机存取存储器（QRAM）装载电路**的高性能 C++ 核心：

- **qubit / qutrit 双架构**：物理比特树与三能级节点树两种 QRAM 实现
- **噪声感知**：比特翻转 / 相位翻转 / 联合翻转 / 去极化 / 振幅衰减按时间片注入
- **分支剪枝**：good/bad 分支剪枝把仿真规模压缩到"坏分支全算 + 一条好分支作基准"
- **Python 绑定**：pybind11 薄绑定（`pip install qram-simulator`）导出全部核心工作类
- CUDA/GPU 后端保留在代码中，当前 CMake 暂时屏蔽 GPU 构建，默认 CPU-only

### 1.2 设计哲学

1. **稀疏态优先**：树态只记录非基态节点，复杂度与非零元素数同阶而非 2^n
2. **分支即轨迹**：以 (address, bus_input) 输入分支为仿真单元，噪声在轨迹振幅上累积
3. **性能与易用性并重**：C++ 核心（OpenMP/TBB 并行）提供性能，Python 绑定提供易用性
4. **自包含构建**：Eigen / fmt / argparse 全部 vendor 于 `ThirdParty/`，构建期不联网

---

## 2. 目录结构与模块

```
QRAM-Simulator/
├── Common/              # 共享基础设施（静态库 SparQ_Common）
│   ├── include/         # 全振幅桥接、矩阵、随机引擎、日志、迭代工具…
│   └── src/
├── QRAM/                # QRAM 电路核心（静态库 SparQ_QRAMSimulator）
│   ├── include/         # qubit/qutrit 电路、分支结构、时序调度
│   └── src/
├── bindings/python/     # pybind11 薄绑定（qram-simulator PyPI 包）
├── Experiments/         # QRAM 论文实验可执行程序
├── test/                # C++ 测试（自制 TEST 宏框架）
├── docs/                # 本文档（Sphinx）+ Doxygen 配置
└── ThirdParty/          # eigen / fmt / argparse / googletest（vendored）
```

### 2.1 Common/ —— 共享基础设施

| 文件 | 功能 |
|------|------|
| `typedefs.h` | 通用类型别名（`complex_t` / `memory_t` / `bus_t` / `u22_t`）、qutrit 能级常量（W/L/R） |
| `basic.h` | 位运算工具（`pow2` / `bitcount` / 补码）、定点编码、保真度、数据树随机填充 |
| `matrix.h` | 定点化稀疏矩阵 `SparseMatrix`、稠密矩阵/向量 `DenseMatrix` / `DenseVector` 与 Eigen 互转 |
| `state_manipulator.h` | `QRAMFullAmp`：QRAM 电路 ↔ 全振幅态向量桥接器 |
| `simple_quantum_simulator.h` | 简单全振幅电路原语（单比特门 / 测量 / 比特序工具） |
| `random_engine.h` | 全局 MT19937-64 单例（固定种子可复现整条仿真链路） |
| `logger.h` | 文件日志、计时器、RAII 函数剖面器、在线统计量、实验结果输出器 |
| `error_handler.h` | 统一异常族与 `TEST` / `TEST_FAIL` 测试宏 |
| `iterable.h` | Python 风格 `range` 与笛卡尔积 `product` 迭代器 |

### 2.2 QRAM/ —— QRAM 电路核心

| 文件 | 内容 |
|------|------|
| `qram_circuit_qubit.h/.cpp` | `qram_qubit::QRAMCircuit`：qubit 架构电路（剪枝仿真主入口） |
| `qram_circuit_qutrit.h/.cpp` | `qram_qutrit::QRAMCircuit`：qutrit 架构电路 |
| `qram_branch_qubit.h/.cpp` | `State` / `SystemState` / `Branch` / `BranchGroup`（qubit 分支结构） |
| `qram_branch_qutrit.h/.cpp` | `QRAMNode` / `QRAMState` / `SubBranch` / `Branch`（qutrit 分支结构） |
| `time_step.h/.cpp` | `TimeStep` 时序与噪声调度器、`OperationType` 枚举、`TimeSlices` 调度结构 |

两套架构共享同一个 `TimeStep` 调度器（以 `arch_qubit` / `arch_qutrit` 常量区分），
接口形状一致，可互换使用：

```cpp
qram_simulator::qram_qubit::QRAMCircuit  qram_q(4, 2);   // qubit 架构
qram_simulator::qram_qutrit::QRAMCircuit qram_t(4, 2);   // qutrit 架构
```

### 2.3 bindings/python/ —— Python 绑定层

`core_binding.cpp` 以 pybind11 导出核心工作类，scikit-build-core 负责打包：

| C++ 类 | Python 名 | 说明 |
|--------|-----------|------|
| `qram_qubit::QRAMCircuit` | `QRAMCircuitQubit` | qubit 架构电路 |
| `qram_qutrit::QRAMCircuit` | `QRAMCircuitQutrit` | qutrit 架构电路 |
| `QRAMFullAmp` | `QRAMFullAmp` | 全振幅态向量桥接（外部库集成入口） |
| `TimeStep` / `TimeSlices` / `OperationPack` / `Operation` | 同名 | 时序与噪声调度 |
| `OperationType` + `arch_qubit` / `arch_qutrit` | 同名 | 噪声模型键与架构常量 |
| `random_engine::set_seed` / `get_seed` | `set_seed` / `get_seed` | 全局随机种子控制 |

构建开关 `QRAM_BUILD_PYTHON_BINDINGS`（默认 OFF）保证嵌入消费方
（SparQSim 的 `add_subdirectory`）与纯 C++ CI 不受影响。

### 2.4 Experiments/ —— 论文实验

| 实验 | 说明 |
|------|------|
| `verify_noisy_simulation` | CI 回归对拍：QRAM 电路级噪声 ↔ 算子级 channel（ctest 注册） |
| `QRAM/QubitPaper` | arXiv:2503.13832 论文图件复现（PRA 化） |
| `QRAM/ChannelCorrespondence` | 噪声模型 ↔ 电路级 channel 对应关系审计 |
| `QRAM/QRAMFidelityV2` | 保真度模拟器对比 |
| `QRAM/TimeStep_BadRange` | 坏分支区间理论验证 |

---

## 3. 数据流

### 3.1 稀疏树态表示

```
┌─────────────────────────────────────────────────────────┐
│ State (qubit)                QRAMState (qutrit)          │
├─────────────────────────────────────────────────────────┤
│ std::set<size_t> nz_elements   std::map<size_t,QRAMNode> │
│  ├ 只记录 |1> 节点位置           ├ 只记录非 (W,0) 节点    │
│  └ 其余隐含为 |0>                └ 节点含 (addr, data)   │
│                                                         │
│ 完全二叉树层序编号：left = 2i+1, right = 2i+2            │
└─────────────────────────────────────────────────────────┘
```

内存与计算复杂度均与非零（非基态）节点数同阶，而非 2^n。

### 3.2 QRAM 装载工作流

```
构造 QRAMCircuit(addr_size, data_size[, memory])
        │
        ▼
set_noise_models({OperationType: 概率})        ← 可选；概率 ∈ [0,1]，
        │                                        Damping 要求 γ < 1
        ▼
set_input_random/uniform(n)                    ← 采样 n 条 (addr, bus) 输入分支
        │
        ▼
initialize_system()                            ← TimeStep::generate 生成含噪 TimeSlices
        │
        ▼
run_normal() / run_full() / run(version)
   ├ run_normal：坏分支全算 + 一条好分支作基准（其余好分支 XOR 镜像预测）
   └ run_full  ：全部分支演化，无剪枝 ground truth
        │
        ▼
sample_and_get_fidelity()                      ← 采样输出分支，|<理想|实际>|²
```

### 3.3 噪声注入

`TimeStep::noise_one_step` 在每个时间片后按噪声模型插入噪声操作：

| 噪声 | OperationType | 作用 |
|------|---------------|------|
| 比特翻转 | `BitFlip` | 轨迹分裂：翻转 / 不翻转，振幅按 √p 加权 |
| 相位翻转 | `PhaseFlip` | 轨迹振幅乘 −1（概率 p） |
| 联合翻转 | `BitPhaseFlip` | 比特与相位同时翻转 |
| 去极化 | `Depolarizing` | 随机 Pauli 混合 |
| 振幅衰减 | `Damping` | 好分支相对乘子 (1−γ)^Δn；`Damp_Common` / `Damp_Full` 两个注入粒度 |
| 置零 | `SetZero` | 节点置回基态 |

### 3.4 全振幅桥接（QRAMFullAmp）

```
全振幅态向量 ──_set_branches──▶ 输入分支集合（按 addr/data 比特分解）
      │                                │
      │                        qutrit QRAMCircuit::run
      │                        （要求非空噪声模型）
      │                                │
      │                        sample_output（轨迹采样）
      ▼                                ▼
新态向量 ◀──_reconstruct──────── 分支轨迹映射回态向量
（好分支 XOR 镜像预测 + Damping 乘子；重建后归一化校验）
```

---

## 4. 关键设计决策

### 4.1 Qutrit-based vs Qubit-based QRAM

**Qutrit 架构**：每个树节点为 (addr ∈ {W,L,R}, data ∈ {0,1}) 三能级系统，
路由由 A1/A2 旋转门（含 w 相位）完成；树结构与调度天然匹配，
门数更少、噪声累积更低。

**Qubit 架构**：节点为标准二能级比特，硬件兼容性更好，理论工具链更成熟；
配合 **good/bad 分支剪枝**——由 `TimeStep::get_bad_range_*` 推导坏分支地址区间，
好地址的分支组只需物化一条基准轨迹，其余按 XOR 镜像预测，
是 qubit 架构在大规模仿真下的核心加速手段（见
[qubit QRAM 剪枝理论](../paper/qubit_qram_pruning.md)）。

两套实现共享同一 `TimeStep` 调度器与噪声模型格式，接口形状一致。

### 4.2 版本化运行入口

`run(version)` 接受 `"full"` / `"normal"` / `"fast"` 等字符串；
qutrit 架构要求非空噪声模型才允许走 `run(version)`（无噪场景应在电路
之外直接处理），而 `run_normal()` / `run_full()` 无此约束。

### 4.3 CUDA 集成（保留代码）

`QRAM/include/cuda/qram_circuit_qutrit.cuh`（`CuQRAMCircuit`，thrust
`device_vector` 内存镜像）保留在代码库中；CondRot 原语重构期间 CMake
强制 `CUDA_FOUND FALSE`，默认 CPU-only（OpenMP 必需，TBB 可选加速）。

---

## 5. 参考

- 快速上手：[quickstart.md](quickstart.md)
- 安装指南：[installation.md](installation.md)
- 论文 1：[arXiv:2503.13832](https://arxiv.org/abs/2503.13832)
- 论文 2：[arXiv:2503.15118](https://arxiv.org/abs/2503.15118)
