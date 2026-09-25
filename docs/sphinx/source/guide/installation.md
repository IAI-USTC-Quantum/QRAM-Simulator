# 安装

QRAM-Simulator 提供两种使用方式：**Python 包安装**（推荐给外部库与
脚本用户）与 **C++ 源码构建**（推荐给需要深度定制的开发者）。

## Python 安装（pip）

```bash
pip install qram-simulator
```

发布范围：CPython 3.10–3.13，Linux x86_64（manylinux）与 Windows
AMD64 平台。包内含 pybind11 编译扩展 `_core`，无纯 Python 依赖。

```python
import qram_simulator as qs

print(qs.__version__)
print(qs.QRAMCircuitQubit, qs.QRAMCircuitQutrit, qs.QRAMFullAmp)
```

### 从源码安装

```bash
git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
cd QRAM-Simulator
pip install .
```

构建依赖（自动进入 pip 构建隔离环境）：CMake ≥ 3.18、C++17 编译器、
OpenMP。Linux 需要 `libgomp`，Windows 使用 MSVC 自带的 OpenMP。

### 运行测试

```bash
pip install .[dev]
pytest bindings/python/test
```

## C++ 构建（CMake）

### 独立构建

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

构建开关：

| 开关 | 默认 | 说明 |
|------|------|------|
| `QRAM_BUILD_TESTS` | ON | 构建 C++ 测试目标（`test/`） |
| `QRAM_BUILD_EXPERIMENTS` | ON | 构建论文实验程序（`Experiments/`） |
| `QRAM_BUILD_PYTHON_BINDINGS` | OFF | 构建 pybind11 绑定（`bindings/python/`，需 pybind11） |
| `QRAM_ENABLE_CUDA` | OFF | 构建 CUDA GPU 后端（需 CUDA 工具链；生产 CondRot 原语的 GPU 内核待补，期间走 CPU 回退） |

运行 CI 回归对拍：

```bash
cd build && ctest --output-on-failure
```

### 作为子项目消费（SparQSim 的方式）

```cmake
set(QRAM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(QRAM_BUILD_EXPERIMENTS OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/qram-simulator)   # git submodule

target_link_libraries(your_target PRIVATE SparQ_QRAMSimulator SparQ_Common)
```

两个库目标（`SparQ_QRAMSimulator` / `SparQ_Common`，命名沿用历史）通过
`BUILD_INTERFACE` 暴露平铺头文件布局；安装态亦可经
`find_package(QRAMSimulator)` 消费（`QRAMSimulator::` 命名空间）。

## 依赖说明

全部第三方依赖 vendor 于 `ThirdParty/`，构建期不联网：

| 依赖 | 用途 |
|------|------|
| Eigen 3.4.0 | 线性代数（matrix.h / 线性求解器） |
| fmt | 格式化与日志输出 |
| argparse | 实验程序命令行解析 |
| googletest | 已 vendor、暂未接线（测试用自制 TEST 宏框架） |

系统级依赖：OpenMP（必需）、TBB（可选，找不到自动回退 OpenMP）。
