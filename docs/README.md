# QRAM-Simulator 文档

本文档目录包含 QRAM-Simulator 项目的各类文档资源。

## 目录结构

```
docs/
├── README.md          # 本文档
└── api/               # API 文档输出目录（由 Doxygen 生成）
    └── html/          # HTML 格式的 API 文档
```

## API 文档

API 文档使用 [Doxygen](https://www.doxygen.nl/) 生成，涵盖以下模块：

- **SparQ** - 稀疏量子态模拟器核心库
- **QRAM** - 量子随机存取存储器实现
- **PySparQ** - Python 绑定接口

### 生成 API 文档

```bash
# 安装 Doxygen
sudo apt-get install doxygen  # Ubuntu/Debian

# 生成文档
doxygen Doxyfile

# 查看生成的文档
open docs/api/html/index.html
```

### 文档配置

Doxygen 配置文件位于项目根目录的 `Doxyfile`，主要配置包括：

- **输入目录**: `SparQ/include/`, `QRAM/include/`, `PySparQ/include/`, `PySparQ/`
- **输出目录**: `docs/api/`
- **递归解析**: 启用（RECURSIVE = YES）
- **提取所有代码**: 启用（EXTRACT_ALL = YES）
- **生成格式**: HTML（GENERATE_HTML = YES）
- **排除目录**: `ThirdParty/`, `build/`, `Experiments/`

## 持续集成

API 文档生成已集成到 CI 流程中，每次提交到 main/develop 分支时自动构建。

查看 CI 配置: `.github/workflows/cmake-multi-platform.yml`
