"""PySparQ 类型存根 - 从 _core 模块重新导出所有公共 API。"""
from __future__ import annotations

# Re-export all public API from the C++ extension module
from pysparq._core import *
from pysparq._core import __all__ as __all__


# Python-only function (defined in __init__.py)
def test_import() -> None:
    """测试 PySparQ 导入是否正常工作。"""
    ...
