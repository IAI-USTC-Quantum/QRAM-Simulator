# 贡献指南

首先，感谢你考虑为 QRAM-Simulator 项目做出贡献！🎉

本指南将帮助你了解如何参与项目开发，包括提交 Issue、设置开发环境、遵循编码规范以及提交 Pull Request。

## 目录

- [如何提交 Issue](#如何提交-issue)
- [开发环境设置](#开发环境设置)
- [编码规范](#编码规范)
- [Pull Request 流程](#pull-request-流程)

## 如何提交 Issue

### Bug 报告

如果你发现了 bug，请使用 [Bug 报告模板](.github/ISSUE_TEMPLATE/bug_report.md) 创建 Issue，并尽可能提供以下信息：

- 问题的清晰描述
- 复现步骤
- 期望行为 vs 实际行为
- 环境信息（操作系统、编译器、CMake 版本等）
- 错误日志或截图

### 功能请求

如果你有新功能的想法，请使用 [功能请求模板](.github/ISSUE_TEMPLATE/feature_request.md) 创建 Issue，包括：

- 功能描述
- 使用场景
- 期望的 API/接口设计
- 替代方案（如果有）

## 开发环境设置

### 前置要求

- **CMake**: >= 3.18
- **C++ 编译器**: 支持 C++17 标准
- **Git**: 任何最新版本

### 可选依赖

- **CUDA Toolkit**: 如需 GPU 支持
- **TBB (Intel Threading Building Blocks)**: 用于并行计算优化
- **OpenMP**: 通常已包含在现代编译器中

### 构建步骤

1. **克隆仓库**
   ```bash
   git clone https://github.com/your-username/QRAM-Simulator.git
   cd QRAM-Simulator
   ```

2. **创建构建目录**
   ```bash
   mkdir build && cd build
   ```

3. **配置项目**
   ```bash
   cmake ..
   ```
   
   如需指定特定选项：
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release -DCACHED_REGISTER_SIZE=32
   ```

4. **编译**
   ```bash
   cmake --build . -j$(nproc)
   ```

5. **运行测试**
   ```bash
   ./bin/run_tests
   ```

### Python 绑定开发

如需开发 Python 绑定（PySparQ）：

```bash
cd PySparQ
pip install -e .
```

## 编码规范

### 基本规范

- **C++ 标准**: C++17
- **缩进**: 4 个空格（不使用 Tab）
- **文件编码**: UTF-8

### 命名约定

| 类型 | 命名风格 | 示例 |
|------|---------|------|
| 命名空间 | 小写 + 下划线 | `qram_simulator` |
| 类名 | 大驼峰 (PascalCase) | `QRAMLoad`, `SparseState` |
| 函数名 | 小驼峰 (camelCase) | `noiseFreeImpl`, `getAmplitude` |
| 成员变量 | 小驼峰 + 下划线后缀 | `register_addr_`, `state_vector_` |
| 局部变量 | 小驼峰 | `tempValue`, `index` |
| 宏/常量 | 大写 + 下划线 | `MAX_QUBITS`, `CACHE_SIZE` |
| 模板参数 | 大驼峰 | `typename InputIt` |

### 头文件规范

- 使用 `#pragma once` 作为头文件保护
- 包含顺序：系统头文件 → 第三方库 → 项目内部头文件

```cpp
#pragma once

// 系统头文件
#include <vector>
#include <memory>

// 第三方库
#include <eigen/Dense>

// 项目内部头文件
#include "QRAM/QRAM.h"
```

### 代码格式

项目使用 `.clang-format` 进行代码格式化。在提交代码前，请确保：

```bash
# 格式化所有源文件
find . -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i
```

### 注释规范

- 使用 `//` 进行单行注释
- 使用 `/* */` 进行多行注释
- 函数和类应使用 Doxygen 风格注释

```cpp
/**
 * @brief 执行 QRAM 加载操作
 * @param address 地址寄存器状态
 * @param data 数据寄存器状态
 * @return 加载后的量子态
 */
QState qramLoad(const QState& address, const QState& data);
```

## Pull Request 流程

### 1. Fork 仓库

点击 GitHub 页面的 "Fork" 按钮，将仓库复制到你的账户下。

### 2. 创建功能分支

```bash
git clone https://github.com/your-username/QRAM-Simulator.git
cd QRAM-Simulator
git checkout -b feature/your-feature-name
```

分支命名建议：
- `feature/` - 新功能
- `bugfix/` - Bug 修复
- `docs/` - 文档更新
- `refactor/` - 代码重构

### 3. 提交更改

编写清晰的 commit message：

```bash
git add .
git commit -m "feat: 添加稀疏态模拟器的 GPU 加速支持

- 实现 CUDA 内核用于并行振幅计算
- 添加内存池管理减少分配开销
- 性能提升约 3x 在 V100 上测试"
```

Commit message 格式：
- `feat:` 新功能
- `fix:` Bug 修复
- `docs:` 文档更新
- `style:` 代码格式调整（不影响功能）
- `refactor:` 代码重构
- `perf:` 性能优化
- `test:` 测试相关
- `chore:` 构建/工具相关

### 4. 推送到 Fork

```bash
git push origin feature/your-feature-name
```

### 5. 创建 Pull Request

1. 访问你的 Fork 仓库
2. 点击 "Compare & pull request"
3. 填写 PR 描述，使用提供的 [PR 模板](.github/PULL_REQUEST_TEMPLATE.md)
4. 关联相关 Issue（如有）：`Fixes #123`
5. 提交 PR

### 6. 等待 CI 通过和审阅

- 确保所有 CI 检查通过
- 等待维护者审阅
- 根据反馈进行修改
- 保持 PR 与主分支同步：`git pull upstream main`

### PR 审阅清单

在提交 PR 前，请确认：

- [ ] 代码符合项目编码规范
- [ ] 所有测试通过
- [ ] 新增功能有相应测试覆盖
- [ ] 文档已更新（如需要）
- [ ] Commit message 清晰有意义
- [ ] PR 描述完整，关联相关 Issue

## 获取帮助

如果你有任何问题，可以通过以下方式获取帮助：

- 在 Issue 中提问
- 查看现有文档和代码
- 参考项目 [README.md](README.md)

再次感谢你的贡献！🙏
