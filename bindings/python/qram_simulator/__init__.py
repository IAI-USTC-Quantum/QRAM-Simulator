"""qram_simulator —— QRAM-Simulator C++ 核心的 Python 绑定。

导出 qubit / qutrit 两种 QRAM 架构的 QRAMCircuit、全振幅桥接器
QRAMFullAmp、时序与噪声调度器 TimeStep 以及全局随机种子控制。
详见 ``_core`` 模块的 docstring 与项目文档。
"""

from ._core import *

from importlib import metadata as _metadata

# __version__ 从 dist-info 读取而非 setuptools-scm 生成的 _version.py：
# scikit-build-core 会按 .gitignore 过滤 wheel 内文件，pyqsparse<=0.1.1
# 曾因此丢失写入的 _version.py（qram-simulator 沿用 dist-info 方案）。
try:
    __version__ = _metadata.version("qram-simulator")
except _metadata.PackageNotFoundError:  # 未安装（如直接在源码树中运行）
    __version__ = "0.0.0"

del _metadata
