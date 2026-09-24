"""
qram_simulator - Thin Python bindings for the qram-simulator C++ core.

Exposes the core sparse-state simulator primitives: register management
(System), SparseState, basic arithmetic/gate operators, measurement, and
native QRAM loading.

This is a deliberately minimal API surface. The full-featured Register
Level Programming API (algorithms, RIR interpreter, dynamic operators,
operator conditioning) is published separately as the `pysparq` package
from the SparQSim repository.

Basic Usage:
    >>> from qram_simulator import System, SparseState, StateStorageType
    >>> from qram_simulator import Init_Unsafe, Hadamard_Int
    >>> System.clear()
    >>> System.add_register("q", StateStorageType.UnsignedInteger, 4)
    >>> state = SparseState()
    >>> Init_Unsafe("q", 5)(state)
    >>> Hadamard_Int("q", 4)(state)
    >>> print(state)
"""

from ._core import *  # noqa: F401,F403

# Import version from auto-generated _version.py
try:
    from ._version import __version__, __version_tuple__
except ImportError:
    __version__ = "0.0.0.dev0"
    __version_tuple__ = (0, 0, 0, "dev0")

del __version_tuple__  # only __version__ is part of the public surface
