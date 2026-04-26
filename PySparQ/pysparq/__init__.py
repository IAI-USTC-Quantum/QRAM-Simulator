"""
PySparQ - Python bindings for QRAM-Simulator's sparse-state quantum circuit simulator.

PySparQ provides a high-performance Python interface to the sparse-state quantum
simulator with native QRAM support and Register Level Programming paradigm.

Basic Usage:
    >>> import pysparq as ps
    >>> ps.System.clear()
    >>> ps.System.add_register("q", ps.UnsignedInteger, 4)
    >>> state = ps.SparseState()
    >>> ps.Init_Unsafe("q", 5)(state)
    >>> ps.print(state)          # prints to stdout (Jupyter-friendly)
    >>> print(state)             # uses __str__ (Detail mode)
    >>> state.to_string(ps.StatePrintDisplay.Default)  # explicit mode

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

# Import version from auto-generated _version.py
try:
    from ._version import __version__, __version_tuple__
except ImportError:
    __version__ = "0.0.0.dev0"
    __version_tuple__ = (0, 0, 0, "dev0")

from ._core import *
from .dynamic_operator import compile_operator, CompilationError, DynamicOperatorError


class StatePrinter:
    """Pythonic state printer with configurable display mode.

    Wraps the C++ StatePrint and provides a clean Python API.
    The ``__call__`` method returns a formatted string (not prints),
    so output is properly captured by Jupyter/IPython.

    Args:
        mode: Display mode (int or StatePrintDisplay enum).
              Defaults to Detail (shows register names and types).
        precision: Number of decimal places for floating-point numbers.
                   Defaults to 0 (uses the state default).

    Example:
        >>> printer = StatePrinter(StatePrintDisplay.Default)
        >>> print(printer(state))       # Default mode
        >>> printer(state, mode=StatePrintDisplay.Binary)  # binary values

        >>> printer = StatePrinter(StatePrintDisplay.Prob)
        >>> print(printer(state))       # show probabilities
    """

    def __init__(
        self,
        mode: int | "StatePrintDisplay" = 1,  # Detail by default
        precision: int = 0,
    ):
        self.mode = mode
        self.precision = precision
        self._cpp_printer = StatePrint(mode, precision)

    def __call__(self, state: SparseState) -> str:
        """Format a SparseState and return the string."""
        return self._cpp_printer(state)

    def __str__(self) -> str:
        return f"StatePrinter(mode={self.mode}, precision={self.precision})"

    def __repr__(self) -> str:
        return self.__str__()

    def to_string(self, state: SparseState, mode: int | None = None) -> str:
        """Format a state with optional mode override.

        Args:
            state: The SparseState to format.
            mode: Optional mode override (uses instance mode if None).
        """
        if mode is not None:
            return StatePrint(mode, self.precision)(state)
        return self(state)


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
