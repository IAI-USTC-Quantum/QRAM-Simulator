"""PySparQ type stubs - re-exports all public API from _core module."""
from __future__ import annotations

# Re-export all public API from the C++ extension module
from pysparq._core import *
from pysparq._core import __all__ as __all__


# Python-only function (defined in __init__.py)
def test_import() -> None:
    """Test that PySparQ imports are working correctly."""
    ...
