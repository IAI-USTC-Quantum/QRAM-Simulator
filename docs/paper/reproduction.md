# 论文实验复现指南

本文档提供详细的实验设置、参数范围和运行命令，用于复现论文中的数值结果。

## 论文信息

- **标题**: Efficient Simulation of Quantum Random Access Memory
- **arXiv**: [2503.13832](https://arxiv.org/abs/2503.13832)
- **代码仓库**: https://github.com/Agony5757/QRAM-Simulator

## 环境准备

### 系统要求

- **操作系统**: Linux (推荐 Ubuntu 20.04+), Windows (Visual Studio 2019+), macOS
- **编译器**: GCC 9+, Clang 10+, MSVC 2019+
- **CMake**: 3.15+
- **可选**: CUDA 12.0+ (用于 GPU 加速实验)

### 构建步骤

```bash
# 克隆仓库
git clone https://github.com/Agony5757/QRAM-Simulator.git
cd QRAM-Simulator

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译所有实验目标
make -j$(nproc) Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration
```

### Windows (PowerShell) 构建

```powershell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target Experiment_QRAM_Fidelity Experiment_QRAM_FidelityV2 Experiment_ErrorFiltration --config Release
```

## 实验概述

| 实验 | 说明 | 对应论文章节 |
|------|------|-------------|
| **QRAM Fidelity** | QRAM 保真度随参数 scaling | 主结果 |
| **QRAM Simulator Comparison** | 不同模拟器配置对比 | 方法验证 |
| **Error Filtration** | 错误过滤方案效果 | 应用案例 |

---

## 实验 1: QRAM Fidelity

**代码位置**: `Experiments/QRAM/QRAMFidelity/QRAMFidelityTest.cpp`

**编译目标**: `Experiment_QRAM_Fidelity`

### 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `--addrsize` | 地址寄存器位数 | 10 |
| `--datasize` | 数据寄存器位数 | 3 |
| `--shots` | 测量次数 | 100 |
| `--inputsize` | 输入叠加态大小 | 10 |
| `--depolarizing` | 退极化噪声强度 | 0.0 |
| `--damping` | 振幅阻尼噪声强度 | 0.0 |
| `--seed` | 随机数种子 | 123456 |
| `--version` | 运行版本: `normal` 或 `full` | normal |

### 运行示例

**Linux/macOS**:
```bash
./build/bin/Experiment_QRAM_Fidelity \
    --addrsize 15 --datasize 3 \
    --shots 100 --inputsize 10 \
    --depolarizing 1e-4 --damping 1e-5 \
    --seed 123456 --version normal
```

**Windows (PowerShell)**:
```powershell
.\build\bin\Experiment_QRAM_Fidelity.exe `
    --addrsize 15 --datasize 3 `
    --shots 100 --inputsize 10 `
    --depolarizing 1e-4 --damping 1e-5 `
    --seed 123456 --version normal
```

### 参数扫描建议

为复现论文中的 scaling 图，建议扫描以下参数：

```bash
# 地址位数扫描 (固定噪声)
for addr in 5 10 15 20 25 30; do
    ./build/bin/Experiment_QRAM_Fidelity \
        --addrsize $addr --datasize 3 \
        --shots 100 --inputsize 10 \
        --depolarizing 1e-4 --damping 1e-5 \
        --seed 123456 --version normal
done

# 噪声强度扫描 (固定地址位数)
for depol in 0.0 1e-5 5e-5 1e-4 5e-4 1e-3; do
    ./build/bin/Experiment_QRAM_Fidelity \
        --addrsize 15 --datasize 3 \
        --shots 100 --inputsize 10 \
        --depolarizing $depol --damping 1e-5 \
        --seed 123456 --version normal
done
```

### 输出说明

程序输出包含：
- 模拟配置参数
- QRAM 电路深度和门数量统计
- 保真度估计值及其置信区间
- 运行时间统计

---

## 实验 2: QRAM Simulator Comparison

**代码位置**: `Experiments/QRAM/QRAMFidelityV2/QRAMSimulatorTest.cpp`

**编译目标**: `Experiment_QRAM_FidelityV2`

### 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `--addrsize` | 地址寄存器位数 | 10 |
| `--datasize` | 数据寄存器位数 | 3 |
| `--shots` | 测量次数 | 100 |
| `--inputsize` | 输入叠加态大小 | 500 |
| `--depolarizing` | 退极化噪声强度 | 0.0 |
| `--damping` | 振幅阻尼噪声强度 | 0.0 |
| `--seed` | 随机数种子 | 123456789 |
| `--architecture` | QRAM 架构: `qutrit` 或 `standard` | qutrit |
| `--experimentname` | 实验名称标识 | test |

### 运行示例

```bash
./build/bin/Experiment_QRAM_FidelityV2 \
    --addrsize 10 --datasize 3 \
    --shots 100 --inputsize 500 \
    --depolarizing 1e-4 --damping 1e-4 \
    --seed 123456789 --architecture qutrit \
    --experimentname qutrit_scheme
```

### 对比实验

```bash
# Qutrit 架构 vs Standard 架构
./build/bin/Experiment_QRAM_FidelityV2 \
    --addrsize 10 --datasize 3 \
    --depolarizing 1e-4 --damping 1e-4 \
    --architecture qutrit --experimentname qutrit_test

./build/bin/Experiment_QRAM_FidelityV2 \
    --addrsize 10 --datasize 3 \
    --depolarizing 1e-4 --damping 1e-4 \
    --architecture standard --experimentname standard_test
```

---

## 实验 3: Error Filtration

**代码位置**: `Experiments/ErrorFiltration/testMultiEFQRAM.cpp`

**编译目标**: `Experiment_ErrorFiltration`

### 命令行参数 (位置参数)

| 位置 | 参数 | 说明 | 示例值 |
|------|------|------|--------|
| 1 | `addrsize` | 地址寄存器位数 | 5 |
| 2 | `datasize` | 数据寄存器位数 | 1 |
| 3 | `num_qrams` | QRAM 数量 | 10 |
| 4 | `shots` | 测量次数 | 1000 |
| 5 | `depolarizing` | 退极化噪声强度 | 1e-5 |
| 6 | `damping` | 振幅阻尼噪声强度 | 1e-5 |
| 7 | `seed` | 随机数种子 | 12345 |
| 8 | `version` | 运行版本 | normal |

### 运行示例

**Linux/macOS**:
```bash
./build/bin/Experiment_ErrorFiltration 5 1 10 1000 1e-5 1e-5 12345 normal
```

**Windows (PowerShell)**:
```powershell
.\build\bin\Experiment_ErrorFiltration.exe 5 1 10 1000 1e-5 1e-5 12345 normal
```

### 参数说明

- **num_qrams**: 串联的 QRAM 模块数量，用于测试错误累积
- **version**: 可以是 `normal`（标准运行）或 `good_only`（仅保留"好分支"）

---

## 噪声模型说明

所有实验使用以下噪声模型：

### 退极化噪声 (Depolarizing)

```cpp
OperationType::Depolarizing
```

以概率 `p` 将量子态替换为完全混合态。等效于随机应用 Pauli X/Y/Z 门。

### 振幅阻尼 (Amplitude Damping)

```cpp
OperationType::Damping
```

模拟能量耗散导致的 $|1\rangle \rightarrow |0\rangle$ 衰减。

### 默认噪声值

| 实验 | 退极化 | 阻尼 |
|------|--------|------|
| QRAMFidelityTest | 0.0 (默认), 1e-4 (演示) | 0.0 (默认), 1e-5 (演示) |
| QRAMSimulatorTest | 0.0 (默认), 1e-4 (演示) | 0.0 (默认), 1e-4 (演示) |
| ErrorFiltration | 1e-5 | 1e-5 |

---

## 复现检查清单

- [ ] 成功构建所有三个实验目标
- [ ] 运行 QRAM Fidelity 实验并获取保真度数据
- [ ] 完成地址位数扫描 (5-30 bits)
- [ ] 完成噪声强度扫描
- [ ] 运行 Simulator Comparison 实验对比不同架构
- [ ] 运行 Error Filtration 实验验证错误过滤效果
- [ ] 收集所有输出数据进行后处理

---

## 常见问题

### Q: 实验运行时间多长？

A: 取决于参数设置：
- 小参数 (`addrsize=5-10`): 几秒到几分钟
- 中等参数 (`addrsize=15-20`): 几分钟到几十分钟
- 大参数 (`addrsize=25+`): 可能需要数小时

### Q: 如何启用 GPU 加速？

A: 构建时启用 CUDA：
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_CUDA=ON
```

### Q: 输出文件在哪里？

A: 实验结果默认输出到标准输出 (stdout)。可以重定向到文件：
```bash
./Experiment_QRAM_Fidelity [参数] > results.txt
```

---

## 联系

如有复现问题，请通过以下方式联系：
- 提交 GitHub Issue
- 邮件: chenzhaoyun@iai.ustc.edu.cn
