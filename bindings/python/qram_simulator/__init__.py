"""qram_simulator -- Python bindings for the QRAM-Simulator C++ core.

Exports QRAMCircuit for both QRAM architectures (qubit / qutrit), the
full-amplitude bridge QRAMFullAmp, the timing & noise scheduler TimeStep,
and global random-seed control. See the ``_core`` module docstring and
the project documentation for details.
"""

from ._core import *

from importlib import metadata as _metadata

# __version__ is read from dist-info instead of a setuptools-scm generated
# _version.py: scikit-build-core filters wheel contents by .gitignore, and
# pyqsparse<=0.1.1 once lost its written _version.py because of this
# (qram-simulator keeps the dist-info approach).
try:
    __version__ = _metadata.version("qram-simulator")
except _metadata.PackageNotFoundError:  # not installed (e.g. running from the source tree)
    __version__ = "0.0.0"

del _metadata

# The circuit-level baseline subpackage pulls in the optional uniqc/qutip
# dependencies, so it is imported lazily on first attribute access
# (`qs.baseline.CircuitQRAMQubit`) instead of at package import time.
_LAZY = ("baseline",)


def __getattr__(name: str):
    if name in _LAZY:
        from importlib import import_module

        return import_module(f"{__name__}.{name}")
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")


def __dir__() -> list:
    return sorted(set(globals()) | set(_LAZY))
