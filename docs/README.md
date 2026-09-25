# QRAM-Simulator 文档

本目录是 **Sphinx 站点**的源（发布于
[GitHub Pages](https://iai-ustc-quantum.github.io/QRAM-Simulator/)），
单一站点同时承载：

- **使用指南**（安装 / 快速上手 / 架构 / 算子约束 / 命名规范）——Markdown（MyST）
- **C++ API 参考**——由 Doxygen 头文件中文注释经 **breathe** 吸入 Sphinx
- **Python API 参考**——由 pybind11 编译模块经 **pybind11-stubgen** 生成 `.pyi`
  桩后由 **sphinx-autoapi** 构建
- **论文与实验**——论文-代码对照与复现指南

Python 绑定（`qram-simulator` PyPI 包）源码位于本仓库 `bindings/python/`；
pysparq 富绑定位于 [SparQSim 仓库](https://github.com/IAI-USTC-Quantum/SparQSim)。

## 目录结构

```
docs/
├── sphinx/
│   ├── Makefile            # make html
│   ├── requirements.txt    # Sphinx 构建依赖
│   └── source/
│       ├── index.rst       # 站点首页与导航
│       ├── guide/          # 安装/快速上手/架构/算子/命名规范
│       ├── api/            # C++ API（cpp.rst，breathe）+ Python API（autoapi）
│       └── paper/          # 论文复现与理论文档
└── README.md               # 本文件
```

（`Doxyfile` 位于仓库根目录，输出 `docs/api/`——HTML 供直接浏览，
XML 供 breathe 使用；两者均已 gitignore。）

## 本地构建

```bash
# 1. Doxygen XML（breathe 输入；需系统安装 doxygen）
doxygen Doxyfile

# 2. Python 扩展与类型桩（autoapi 输入；仅 Python API 章节需要）
pip install . pybind11-stubgen -r docs/sphinx/requirements.txt
pybind11_stubgen qram_simulator -o docs/sphinx/source/api_stubs

# 3. Sphinx 站点
sphinx-build docs/sphinx/source docs/sphinx/build/html
# 或：cd docs/sphinx && make html
```

> 缺第 1 步时 C++ API 章节构建失败；缺第 2 步时自动跳过 Python API
> 章节（`conf.py` 检测 `api_stubs/` 是否存在）。

## CI（docs.yml）

push/PR 触发 → doxygen → pip install . + stubgen → sphinx-build →
上传 artifact；main 分支 push 时部署 gh-pages（单一 Sphinx 站点全量替换）。
