"""
PySparQ - Python bindings for QRAM-Simulator's sparse-state quantum circuit simulator.

PySparQ provides a high-performance Python interface to the sparse-state quantum
simulator with native QRAM support and Register Level Programming paradigm.

Basic Usage:
    >>> import pysparq as ps
    >>> ps.System.clear()
    >>> reg = ps.System.add_register("q", ps.UnsignedInteger, 4)
    >>> state = ps.SparseState()
    >>> ps.Init_Unsafe("q", 5)(state)
    >>> ps.StatePrint()(state)

Key Features:
    - Register Level Programming: Operate on named registers instead of individual qubits
    - Sparse State Simulation: Efficiently simulate states with few non-zero amplitudes
    - Native QRAM Support: Quantum Random Access Memory for efficient data loading
    - High Performance: C++ core with Python bindings

See Also:
    - Documentation: https://iai-ustc-quantum.github.io/QRAM-Simulator/
    - C++ API Docs: https://iai-ustc-quantum.github.io/QRAM-Simulator/api/
    - SparQ Paper: https://arxiv.org/abs/2503.15118
    - QRAM Paper: https://arxiv.org/abs/2503.13832
"""

from __future__ import annotations

from ._core import *


def test_import() -> None:
    """Test that PySparQ imports are working correctly.

    This function tests basic functionality including:
    - Module creation
    - SparseState creation
    - BaseOperator and SelfAdjointOperator behavior

    Raises:
        RuntimeError: If any test fails
        ImportError: If imports are not working correctly

    Example:
        >>> import pysparq as ps
        >>> ps.test_import()
        Test import passed.
    """
    # Test imports
    module1 = ModuleInheritance_Test()
    print("module1 created.")
    sparse_state = SparseState()
    print("sparse_state created.")
    try:
        # this should be OK
        module1(sparse_state)
        print("Test module1 passed.")
    except Exception as e:
        print("Test import failed. Details: ")
        raise e

    try:
        # this should raise RuntimeError
        module1.dag(sparse_state)
        raise ImportError("Test import failed. dag() should raise an error.")
    except RuntimeError as e:
        # Test import passed. dag() raised a RuntimeError as expected.
        print(f"Test module1.dag passed. {e}")
    except ImportError as e:
        print("Test import failed (dag should raise an exception). Details: ")
        raise e
    except Exception as e:
        print("Test import failed. Details: ")
        raise e

    module2 = ModuleInheritance_Test_SelfAdjoint()
    print("module2 created.")
    try:
        # this should be OK
        module2.dag(sparse_state)
        print("Test module2.dag passed.")
        module2(sparse_state)
        print("Test module2 passed.")
    except Exception as e:
        print("Test import failed. Details: ")
        raise e

    print("Test import passed.")
