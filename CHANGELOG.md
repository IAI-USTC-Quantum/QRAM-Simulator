# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- 项目架构文档 ARCHITECTURE.md
- 变更日志 CHANGELOG.md
- 完善的 README.md，包含复现说明

### Changed
- 文档结构优化

### Fixed
- 无

---

## [1.0.0] - 2025-04-15

### Added

#### 核心模拟器
- **稀疏态量子模拟器核心（SparQ）**
  - 基于稀疏向量表示的量子态存储
  - 高效的门操作执行引擎
  - 支持任意数量的量子比特
  - 内置归一化验证和状态检查

#### QRAM 实现
- **Qutrit-based QRAM 电路**
  - 层次化的树形结构表示
  - 支持可配置的地址线和数据线位数
  - 时步逻辑生成量子操作序列
  - 分支概率追踪和采样
  
- **Qubit-based QRAM 电路**
  - 硬件兼容的实现方式
  - 与 qutrit 版本共享相同 API

- **CUDA GPU 加速支持**
  - GPU 内存管理（`thrust::device_vector`）
  - 并行门操作内核
  - 异步执行支持
  - CPU-GPU 混合执行策略

#### 基础量子门
- **单比特门**：H（Hadamard）、X、Y、Z、S、T
- **旋转门**：RX、RY、RZ、通用旋转门
- **多比特门**：CNOT、Toffoli、多控制门
- **条件门**：条件旋转、条件交换
- **相位门**：S、T、相位旋转、并行相位操作

#### 高级量子算法
- **量子傅里叶变换（QFT）**
- **Grover 搜索算法**
- **量子算术运算**
  - 量子加法器
  - 量子乘法器
  - 量子比较器
- **块编码（Block Encoding）**
- **态制备（State Preparation）**
- **量子随机漫步**
- **哈密顿量模拟**

#### 噪声模型
- **去极化噪声（Depolarizing）**
- **振幅阻尼噪声（Amplitude Damping）**
- **噪声参数可配置**
- **噪声影响分析工具**

#### Python 绑定（PySparQ）
- **完整的 Python API**
  - 所有核心类暴露到 Python
  - 类型提示支持（`_core.pyi`）
- **Pybind11 封装**
  - 高效的 C++-Python 互操作
  - 自动内存管理
- **pip 安装支持**
  - `pyproject.toml` 配置
  - 预编译 wheel 分发

#### 实验代码
- **QRAM 保真度测试**
  - `QRAMFidelityTest.cpp`：主模拟 + 性能分析
  - `QRAMSimulatorTest.cpp`：完整 vs 普通模拟器对比
- **误差过滤实验**
  - `testMultiEFQRAM.cpp`：多误差过滤 QRAM
- **算法验证**
  - QFT、Grover、Shor 算法
  - 量子卷积神经网络（QCNN）
  - 量子微分算法（QDA）
  - GHZ 态制备

#### 基础设施
- **CMake 构建系统**
  - 跨平台支持（Linux、Windows、macOS）
  - 自动依赖检测
  - CUDA 可选编译
- **Eigen 集成**
  - 头文件形式嵌入（ThirdParty/eigen-3.4.0）
  - 稀疏/稠密矩阵运算支持
- **日志系统**
  - 分级日志（DEBUG、INFO、WARN、ERROR）
  - 性能计时支持
- **单元测试框架**
  - 核心模块测试覆盖
  - CI/CD 集成（GitHub Actions）

### Changed
- 无（初始版本）

### Deprecated
- 无

### Removed
- 无

### Fixed
- 无（初始版本）

### Security
- 无

---

## 版本历史概览

| 版本 | 日期 | 主要更新 |
|------|------|----------|
| 1.0.0 | 2025-04-15 | 首次发布，包含完整的 QRAM 模拟器、稀疏态模拟核心、Python 绑定和实验代码 |

---

## 未来计划

### [1.1.0] - 计划

#### 预期新增功能
- [ ] 更多噪声模型（相位阻尼、比特翻转等）
- [ ] 可视化工具（量子态演化图）
- [ ] 性能优化（向量化、更高效的稀疏操作）
- [ ] 更多算法实现（VQE、QAOA）
- [ ] 交互式 Jupyter Notebook 教程

#### 预期改进
- [ ] Python API 文档完善
- [ ] 更多使用示例
- [ ] 性能基准测试套件

---

## 参考

- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)
- 项目文档：[README.md](README.md) | [ARCHITECTURE.md](ARCHITECTURE.md)
