"""PySparQ 类型存根 - 从 _core 模块重新导出所有公共 API。"""
from __future__ import annotations

# Re-export all public API from the C++ extension module
from pysparq._core import *
from pysparq._core import __all__ as __all__


# Python-only function (defined in __init__.py)
def test_import() -> None:
    """测试 PySparQ 导入是否正常工作。"""
    ...


class StatePrinter:
    """Pythonic state printer with configurable display mode."""

    def __init__(
        self,
        mode: int | "StatePrintDisplay" = 1,
        precision: int = 0,
    ) -> None:
        ...
    def __call__(self, state: SparseState) -> str: ...
    def __str__(self) -> str: ...
    def __repr__(self) -> str: ...
    def to_string(self, state: SparseState, mode: int | None = None) -> str: ...
    mode: int | StatePrintDisplay
    precision: int
