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

# Version from installed dist-info metadata. setuptools-scm's write_to file
# is excluded from wheels by scikit-build-core's gitignore filtering (a latent
# bug that shipped pysparq<=0.1.1 with __version__ == "0.0.0.dev0"), so read
# the metadata instead, which always carries the setuptools-scm version.
try:
    from importlib.metadata import PackageNotFoundError, version as _dist_version

    __version__: str = _dist_version("qram-simulator")
except PackageNotFoundError:  # source tree without metadata
    __version__ = "0.0.0.dev0"
