"""PySparQ 类型存根 - 从 _core 模块重新导出所有公共 API。"""
from __future__ import annotations

import pysparq._core as _core
from pysparq._core import __all__ as __all__

from pysparq._core import (
    SparseState,
    System,
    StateStorage,
    StateStorageType,
    UnsignedInteger,
    SignedInteger,
    Boolean,
    Rational,
    SparseMatrix,
    DenseMatrix_complex,
    DenseMatrix_float64,
    BaseOperator,
    SelfAdjointOperator,
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
    ShiftLeft_InPlace,
    ShiftRight_InPlace,
    Mult_UInt_ConstUInt,
    Add_Mult_UInt_ConstUInt_InPlace,
    Mod_Mult_UInt_ConstUInt_InPlace,
    Add_UInt_UInt,
    Add_UInt_UInt_InPlace,
    Add_UInt_ConstUInt,
    Add_ConstUInt_InPlace,
    Div_Sqrt_Arccos_Int_Int,
    Sqrt_Div_Arccos_Int_Int,
    GetRotateAngle_Int_Int,
    AddAssign_AnyInt_AnyInt_InPlace,
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
    CondRot_Fixed_Bool,
    CondRot_General_Bool_QW_fast,
    GetQWRotateAngle_Int_Int_Int,
    PlusOneAndOverflow,
    split_systems,
    combine_systems,
    merge_system,
    remove_system,
    StatePrintDisplay,
)


# Python-only functions and classes
def StatePrint(
    state: SparseState,
    mode: int | "StatePrintDisplay" = 1,
    precision: int = 0,
) -> str:
    """Return a formatted string representation of a SparseState."""
    ...


def to_string(
    state: SparseState,
    mode: int | "StatePrintDisplay" = 1,
    precision: int = 0,
) -> str:
    """Return a formatted string representation of a SparseState (alias for StatePrint)."""
    ...


def print(state: SparseState, mode: int | "StatePrintDisplay" = 1, precision: int = 0) -> None:
    """Print a SparseState to stdout in Detail mode."""
    ...


def test_import() -> None:
    """测试 PySparQ 导入是否正常工作。"""
    ...


class StatePrinter:
    """Reusable state formatter with configurable default mode."""

    def __init__(
        self,
        mode: int | "StatePrintDisplay" = 1,
        precision: int = 0,
    ) -> None:
        ...
    def __call__(
        self,
        state: SparseState,
        mode: int | "StatePrintDisplay" | None = None,
    ) -> str:
        """Format a state using this printer's mode (or override it)."""
        ...
    def __str__(self) -> str: ...
    def __repr__(self) -> str: ...
    mode: int
    precision: int
