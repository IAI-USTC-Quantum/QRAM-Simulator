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
    >>> ps.pprint(state)                         # print to stdout (Detail mode)
    >>> str(state)                               # __str__ → Detail mode string
    >>> repr(state)                              # __repr__ → Detail mode string
    >>> ps.StatePrint(state)                     # returns Detail mode string
    >>> ps.StatePrint(state, ps.StatePrintDisplay.Default)  # explicit mode

Key Features:
    - Register Level Programming: Operate on named registers instead of individual qubits
    - Sparse State Simulation: Efficiently simulate states with few non-zero amplitudes
    - Native QRAM Support: Quantum Random Access Memory for efficient data loading
    - High Performance: C++ core with Python bindings

Display Modes (StatePrintDisplay):
    - Default (0):  Brief format — amplitude and register values only
    - Detail  (1):  Register header + amplitude + named register values
    - Binary  (2):  Register values shown in binary
    - Prob    (4):  Amplitude + probability for each basis state
    Modes can be OR'd together: Detail | Prob

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

# Import everything from _core, then shadow StatePrint/print with Python functions.
# Using a targeted import + re-bind pattern (rather than "from ._core import *")
# so that we can override StatePrint without name collision.
import pysparq._core as _core
from pysparq._core import (
    SparseState,
    System,
    StateStorage,
    StateStorageType,
    # Expose StateStorageType values at module level for convenience
    UnsignedInteger,
    SignedInteger,
    Boolean,
    Rational,
    General as StateStorageType_General,
    SparseMatrix,
    DenseMatrix_complex,
    DenseMatrix_float64,
    BaseOperator,
    SelfAdjointOperator,
    # Operators
    AddRegister,
    AddRegisterWithHadamard,
    RemoveRegister,
    MoveBackRegister,
    SplitRegister,
    CombineRegister,
    Push,
    Pop,
    ClearZero,
    Normalize,
    Init_Unsafe,
    ModuleInheritance_Test,
    ModuleInheritance_Test_SelfAdjoint,
    CheckNormalization,
    CheckNan,
    ViewNormalization,
    TestRemovable,
    CheckDuplicateKey,
    Hadamard_Int,
    Hadamard_Int_Full,
    Hadamard_Bool,
    Hadamard_PartialQubit,
    ZeroConditionalPhaseFlip,
    Reflection_Bool,
    GlobalPhase_Int,
    PartialTrace,
    PartialTraceSelect,
    PartialTraceSelectRange,
    QFT,
    inverseQFT,
    QRAMCircuit_qutrit,
    QRAMLoad,
    QRAMLoadFast,
    Xgate_Bool,
    FlipBools,
    Swap_Bool_Bool,
    ShiftLeft,
    ShiftRight,
    Mult_UInt_ConstUInt,
    Add_Mult_UInt_ConstUInt,
    Mod_Mult_UInt_ConstUInt,
    Add_UInt_UInt,
    Add_UInt_UInt_InPlace,
    Add_UInt_ConstUInt,
    Add_ConstUInt,
    Div_Sqrt_Arccos_Int_Int,
    Sqrt_Div_Arccos_Int_Int,
    GetRotateAngle_Int_Int,
    AddAssign_AnyInt_AnyInt,
    Assign,
    Compare_UInt_UInt,
    Less_UInt_UInt,
    Swap_General_General,
    GetMid_UInt_UInt,
    CustomArithmetic,
    StateHashExceptKey,
    StateHashExceptQubits,
    StateEqualExceptKey,
    StateEqualExceptQubits,
    StateLessExceptKey,
    StateLessExceptQubits,
    Rot_GeneralUnitary,
    Rot_GeneralStatePrep,
    stateprep_unitary_build_schmidt,
    SortExceptKey,
    SortByKey,
    SortExceptBit,
    SortExceptKeyHadamard,
    SortUnconditional,
    SortByAmplitude,
    SortByKey2,
    Phase_Bool,
    Rot_Bool,
    Ygate_Bool,
    Zgate_Bool,
    Sgate_Bool,
    Tgate_Bool,
    RXgate_Bool,
    RYgate_Bool,
    RZgate_Bool,
    SXgate_Bool,
    U2gate_Bool,
    U3gate_Bool,
    CondRot_Rational_Bool,
    CondRot_General_Bool,
    PlusOneAndOverflow,
    # Utilities
    split_systems,
    combine_systems,
    merge_system,
    remove_system,
    # Enums
    StatePrintDisplay,
    # Note: StatePrint (C++ class) is NOT imported here — we shadow it below
)
from .dynamic_operator import compile_operator, CompilationError, DynamicOperatorError


# --------------------------------------------------------------------
# State string formatting (shadows _core.StatePrint)
# --------------------------------------------------------------------

def StatePrint(
    state: SparseState,
    mode: int | StatePrintDisplay = 1,
    precision: int = 0,
) -> str:
    """Return a formatted string representation of a SparseState.

    This is the primary way to convert a SparseState to a string.
    ``ps.StatePrint(state)`` is equivalent to ``str(state)`` but allows
    explicit control over display mode and precision.

    Args:
        state: The SparseState to format.
        mode:  Display mode (int or StatePrintDisplay enum).
               Defaults to Detail (1) — shows register names and types.
               See ``StatePrintDisplay`` for available modes.
        precision: Number of decimal places for floating-point numbers.
                   Defaults to 0 (uses the state's default).

    Returns:
        A formatted string describing the quantum state.

    Example:
        >>> ps.System.clear()
        >>> ps.AddRegister("q", ps.UnsignedInteger, 2)(ps.SparseState())
        >>> ps.Init_Unsafe("q", 1)(ps.SparseState())
        >>> state = ps.SparseState()
        >>> ps.Hadamard_Int("q", 2)(state)
        >>> ps.StatePrint(state)       # Detail mode (default)
        >>> ps.StatePrint(state, mode=ps.StatePrintDisplay.Default)
        >>> ps.StatePrint(state, mode=ps.StatePrintDisplay.Binary)
        >>> ps.StatePrint(state, mode=ps.StatePrintDisplay.Prob)
    """
    return state.to_string(int(mode), precision)


def to_string(
    state: SparseState,
    mode: int | StatePrintDisplay = 1,
    precision: int = 0,
) -> str:
    """Return a formatted string representation of a SparseState.

    Alias for ``StatePrint(state, mode, precision)`` with Detail mode default.

    Args:
        state: The SparseState to format.
        mode:  Display mode. Defaults to Detail (1).
        precision: Decimal places for floating-point numbers.

    Returns:
        A formatted string describing the quantum state.
    """
    return state.to_string(int(mode), precision)


def pprint(state: SparseState, mode: int | StatePrintDisplay = 1, precision: int = 0) -> None:
    """Print a SparseState to stdout in the specified display mode.

    This function prints directly to standard output, which is captured
    correctly by Jupyter and IPython notebooks.

    Args:
        state: The SparseState to print.
        mode: Display mode. Defaults to Detail (1).
        precision: Decimal places for floating-point numbers.

    Example:
        >>> ps.System.clear()
        >>> ps.AddRegister("q", ps.UnsignedInteger, 2)(ps.SparseState())
        >>> state = ps.SparseState()
        >>> ps.Hadamard_Int("q", 2)(state)
        >>> ps.pprint(state)   # prints to stdout in Detail mode
        >>> ps.pprint(state, mode=ps.StatePrintDisplay.Prob)   # Prob mode
    """
    result = state.to_string(int(mode), precision)
    import sys
    sys.stdout.write(result + '\n')


class StatePrinter:
    """Reusable state formatter with configurable default mode.

    Provides a stateful formatter that holds a persistent mode and
    precision setting. Useful when printing many states in the same
    format.

    Args:
        mode: Default display mode. Defaults to Detail (1).
        precision: Default decimal precision. Defaults to 0.

    Example:
        >>> printer = StatePrinter(ps.StatePrintDisplay.Prob)
        >>> printer(state1)   # uses Prob mode
        >>> printer(state2)   # uses Prob mode
        >>> printer(state3, mode=ps.StatePrintDisplay.Binary)  # override
    """

    def __init__(
        self,
        mode: int | StatePrintDisplay = 1,
        precision: int = 0,
    ):
        self.mode = int(mode)
        self.precision = precision

    def __call__(
        self,
        state: SparseState,
        mode: int | StatePrintDisplay | None = None,
    ) -> str:
        """Format a state using this printer's mode (or override it).

        Args:
            state: The SparseState to format.
            mode: Optional mode override for this call.

        Returns:
            Formatted string representation.
        """
        effective_mode = int(mode) if mode is not None else self.mode
        return state.to_string(effective_mode, self.precision)

    def __str__(self) -> str:
        return f"StatePrinter(mode={self.mode}, precision={self.precision})"

    def __repr__(self) -> str:
        return self.__str__()


# --------------------------------------------------------------------
# Test utilities
# --------------------------------------------------------------------

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
