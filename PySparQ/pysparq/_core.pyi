"""

PySparQ - Sparse-state quantum circuit simulator with native QRAM support.

This module provides a Register Level Programming paradigm for quantum algorithm
development. Instead of composing circuits from individual gates, operate directly
on named registers using high-level arithmetic operations.

Key classes:
    System: Quantum system managing registers
    SparseState: Sparse quantum state representation
    BaseOperator: Base class for all quantum operators

Example:
    from pysparq import System, SparseState, AddRegister, Hadamard_Int

    system = System()
    state = SparseState()
    AddRegister("q", UnsignedInteger, 4)(state)
    Hadamard_Int("q")(state)
    print(state)
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['Abs_SInt', 'Add_AnyInt_AnyInt_InPlace', 'AddRegister', 'AddRegisterWithHadamard', 'Add_ConstUInt_InPlace', 'Add_Mult_UInt_ConstUInt_InPlace', 'Add_UInt_ConstUInt', 'Add_UInt_UInt', 'Add_UInt_UInt_InPlace', 'And_UInt_UInt', 'Assign', 'BaseOperator', 'Binary', 'Boolean', 'Carry_UInt_UInt', 'CheckDuplicateKey', 'CheckNan', 'CheckNormalization', 'ClearZero', 'CombineRegister', 'Compare_UInt_UInt', 'CondRot_Fixed_Bool', 'CondRot_Rational_Bool', 'CustomArithmetic', 'Default', 'DenseMatrix_complex', 'DenseMatrix_float64', 'Detail', 'Div_Sqrt_Arccos_UInt_UInt', 'Div_UInt_UInt', 'FlipBools', 'General', 'GetDataAddr', 'GetMid_UInt_UInt', 'GetQWRotateAngle_Int_Int_Int', 'GetRotateAngle_Int_Int', 'GetRowAddr', 'GlobalPhase', 'Hadamard_Bool', 'Hadamard_Int', 'Hadamard_Int_Full', 'Hadamard_Partial', 'Init_Unsafe', 'IsZero_UInt', 'Less_SInt_SInt', 'Less_UInt_UInt', 'MeasureZ', 'Mod_Mult_UInt_ConstUInt_InPlace', 'ModuleInheritance_Test', 'ModuleInheritance_Test_SelfAdjoint', 'MoveBackRegister', 'MulOverflow_UInt_UInt', 'Mul_UInt_UInt', 'Mult_UInt_ConstUInt', 'Neg_UInt', 'Negative_SInt', 'Normalize', 'Or_UInt_UInt', 'Overflow_SInt_SInt', 'PartialTrace', 'PartialTraceSelect', 'PartialTraceSelectRange', 'Phase_Bool', 'PlusOneAndOverflow', 'Pop', 'Prob', 'Probability', 'Push', 'QFT', 'QRAMCircuit_qutrit', 'QRAMLoad', 'QRAMLoadFast', 'QuantumBinarySearch_Fast', 'RX_Bool', 'RY_Bool', 'RZ_Bool', 'Rational', 'Reflection_Bool', 'RemoveRegister', 'Reset', 'Rot_Bool', 'Rot_GeneralStatePrep', 'Rot_GeneralUnitary', 'SX_Bool', 'Select_Bool_UInt_UInt', 'SelfAdjointOperator', 'S_Bool', 'ShiftLeft_InPlace', 'ShiftRight_InPlace', 'SignedInteger', 'SortByAmplitude', 'SortByKey', 'SortByKey2', 'SortExceptBit', 'SortExceptKey', 'SortExceptKeyHadamard', 'SortUnconditional', 'SparseMatrix', 'SparseState', 'SplitRegister', 'Sqrt_Div_Arccos_Int_UInt', 'Sqrt_UInt', 'StateEqualExceptKey', 'StateEqualExceptQubits', 'StateHashExceptKey', 'StateHashExceptQubits', 'StateLessExceptKey', 'StateLessExceptQubits', 'StatePrint', 'StatePrintDisplay', 'StateStorage', 'StateStorageType', 'Sub_UInt_UInt', 'Swap_Bool_Bool', 'Swap_General_General', 'System', 'TestRemovable', 'T_Bool', 'U2_Bool', 'U3_Bool', 'UnsignedInteger', 'ViewNormalization', 'X_Bool', 'Xor_UInt_UInt', 'Y_Bool', 'ZeroConditionalPhaseFlip', 'Z_Bool', 'combine_systems', 'get_seed', 'InverseQFT', 'merge_system', 'print', 'remove_system', 'reseed', 'set_seed', 'split_systems', 'stateprep_unitary_build_schmidt', 'time_seed']
class Abs_SInt(SelfAdjointOperator):
    """
    Absolute value of a signed integer register.

    Computes: res ^= |reg|, where reg is sign-extended from its two's complement
    bit pattern, truncated to mod 2^res_width before being XORed into res (SInt
    in, UInt out).

    Args:
        reg: Name/ID of the SignedInteger input register.
        res: Name/ID of the UnsignedInteger output register (result is XORed in).

    Example:
        Abs_SInt("a", "result")(state)  # result ^= |a|
    """
    @typing.overload
    def __init__(self, reg: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Abs_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Abs_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Abs_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Abs_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Abs_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Abs_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Abs_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Abs_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Abs_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Abs_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Abs_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Abs_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Abs_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Abs_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Abs_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Abs_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Add_AnyInt_AnyInt_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, input_reg: str, output_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, input_reg: typing.SupportsInt | typing.SupportsIndex, output_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_AnyInt_AnyInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class AddRegister:
    def __call__(self, arg0: SparseState) -> int:
        ...
    def __init__(self, name: str, type: StateStorageType, size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class AddRegisterWithHadamard:
    def __call__(self, arg0: SparseState) -> int:
        ...
    def __init__(self, name: str, type: StateStorageType, size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Add_ConstUInt_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, input_reg: str, add: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, input_reg: typing.SupportsInt | typing.SupportsIndex, add: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Add_Mult_UInt_ConstUInt_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, input_reg: str, multiplier: typing.SupportsInt | typing.SupportsIndex, output_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, input_reg: typing.SupportsInt | typing.SupportsIndex, multiplier: typing.SupportsInt | typing.SupportsIndex, output_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Add_UInt_ConstUInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, input_reg: str, add: typing.SupportsInt | typing.SupportsIndex, output_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, input_reg: typing.SupportsInt | typing.SupportsIndex, add: typing.SupportsInt | typing.SupportsIndex, output_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Add_UInt_UInt(SelfAdjointOperator):
    """

    Add two unsigned integer registers.

    Computes: |a⟩|b⟩|0⟩ → |a⟩|b⟩|a+b⟩ (mod 2^n)

    Args:
        input_reg1: Name/ID of the first input register (addend).
        input_reg2: Name/ID of the second input register (addend).
        output_reg: Name/ID of the output register (accumulates sum).

    Example:
        Add_UInt_UInt("a", "b", "result")(state)  # result = a + b
    """
    @typing.overload
    def __init__(self, input_reg1: str, input_reg2: str, output_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, input_id1: typing.SupportsInt | typing.SupportsIndex, input_id2: typing.SupportsInt | typing.SupportsIndex, output_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Add_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Add_UInt_UInt_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, input_reg: str, output_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, input_reg: typing.SupportsInt | typing.SupportsIndex, output_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_UInt_UInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class And_UInt_UInt(SelfAdjointOperator):
    """
    Bitwise AND of two unsigned integer registers.

    Computes: res ^= lhs & rhs, operands zero-extended, result truncated to
    mod 2^res_width before being XORed into res.

    Args:
        lhs: Name/ID of the first input register.
        rhs: Name/ID of the second input register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        And_UInt_UInt("a", "b", "result")(state)  # result ^= a & b
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> And_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> And_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> And_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> And_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> And_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> And_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> And_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> And_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> And_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> And_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> And_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> And_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> And_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> And_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> And_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> And_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Assign(SelfAdjointOperator):
    @typing.overload
    def __init__(self, src: str, dst: str) -> None:
        ...
    @typing.overload
    def __init__(self, src_id: typing.SupportsInt | typing.SupportsIndex, dst_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Assign:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Assign:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Assign:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Assign:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Assign:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Assign:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Assign:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Assign:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Assign:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Assign:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Assign:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Assign:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Assign:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Assign:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Assign:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Assign:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class BaseOperator:
    def __call__(self, arg0: SparseState) -> None:
        ...
    def dag(self, arg0: SparseState) -> None:
        ...
class Carry_UInt_UInt(SelfAdjointOperator):
    """
    Carry-out flag of an unsigned addition at the res width.

    Computes: flag ^= carry_out(lhs + rhs) relative to the width w of res,
    i.e. whether the full-precision sum is >= 2^w (at w = 64 the predicate is
    64-bit wraparound). out/res 参数仅提供宽度,不读其值.

    Args:
        lhs: Name/ID of the first input register.
        rhs: Name/ID of the second input register.
        res: Name/ID of the register providing the target width w (not read).
        flag: Name/ID of the width-1 Boolean flag register (result is XORed in).

    Example:
        Carry_UInt_UInt("a", "b", "result", "flag")(state)  # flag ^= carry of a+b at result width
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str, flag: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex, flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Carry_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Carry_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Carry_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Carry_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Carry_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Carry_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Carry_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Carry_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Carry_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Carry_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Carry_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Carry_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Carry_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Carry_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Carry_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Carry_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class CheckDuplicateKey(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class CheckNan(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class CheckNormalization(SelfAdjointOperator):
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, threshold: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class ClearZero(SelfAdjointOperator):
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, epsilon: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class CombineRegister:
    def __call__(self, arg0: SparseState) -> int:
        ...
    def __init__(self, first: str, second: str) -> None:
        ...
class Compare_UInt_UInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, left_reg: str, right_reg: str, less_flag_reg: str, equal_flag_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, left_id: typing.SupportsInt | typing.SupportsIndex, right_id: typing.SupportsInt | typing.SupportsIndex, less_flag_id: typing.SupportsInt | typing.SupportsIndex, equal_flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Compare_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Compare_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Compare_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Compare_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Compare_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Compare_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Compare_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Compare_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Compare_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Compare_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Compare_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Compare_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Compare_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Compare_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Compare_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Compare_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class CondRot_Fixed_Bool(BaseOperator):
    @typing.overload
    def __init__(self, arg0: str, arg1: str) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CondRot_Rational_Bool(BaseOperator):
    def __init__(self, arg0: str, arg1: str) -> None:
        ...
class CustomArithmetic(SelfAdjointOperator):
    def __init__(self, input_registers: list, input_size: int, output_size: int, func: collections.abc.Callable) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> CustomArithmetic:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> CustomArithmetic:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> CustomArithmetic:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> CustomArithmetic:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> CustomArithmetic:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> CustomArithmetic:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> CustomArithmetic:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> CustomArithmetic:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> CustomArithmetic:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> CustomArithmetic:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> CustomArithmetic:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> CustomArithmetic:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> CustomArithmetic:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> CustomArithmetic:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> CustomArithmetic:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> CustomArithmetic:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class DenseMatrix_complex:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class DenseMatrix_float64:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Div_Sqrt_Arccos_UInt_UInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, lhs_reg: str, rhs_reg: str, out_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_reg: typing.SupportsInt | typing.SupportsIndex, rhs_reg: typing.SupportsInt | typing.SupportsIndex, out_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Div_UInt_UInt(SelfAdjointOperator):
    """
    Integer-divide two unsigned integer registers.

    Computes: res ^= lhs / rhs (floor division). Total-domain convention: a zero
    divisor yields quotient 0 instead of raising (overflow/domain information is
    reported by dedicated flag operators); the quotient is truncated to
    mod 2^res_width before being XORed into res.

    Args:
        lhs: Name/ID of the dividend register.
        rhs: Name/ID of the divisor register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Div_UInt_UInt("a", "b", "result")(state)  # result ^= a / b (0 if b == 0)
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Div_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Div_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Div_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Div_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Div_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Div_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Div_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Div_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Div_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Div_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Div_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Div_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class FlipBools(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> FlipBools:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> FlipBools:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> FlipBools:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> FlipBools:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> FlipBools:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> FlipBools:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> FlipBools:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> FlipBools:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> FlipBools:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> FlipBools:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> FlipBools:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> FlipBools:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> FlipBools:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> FlipBools:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> FlipBools:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> FlipBools:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class GetDataAddr(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg_offset: str, reg_row: str, reg_col_sparse: str, row_size: typing.SupportsInt | typing.SupportsIndex, reg_data_offset: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_offset: typing.SupportsInt | typing.SupportsIndex, reg_row: typing.SupportsInt | typing.SupportsIndex, reg_col_sparse: typing.SupportsInt | typing.SupportsIndex, row_size: typing.SupportsInt | typing.SupportsIndex, reg_data_offset: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class GetMid_UInt_UInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, left_reg: str, right_reg: str, mid_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, left_id: typing.SupportsInt | typing.SupportsIndex, right_id: typing.SupportsInt | typing.SupportsIndex, mid_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GetMid_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GetMid_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GetMid_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GetMid_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GetMid_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class GetQWRotateAngle_Int_Int_Int(SelfAdjointOperator):
    @typing.overload
    def __init__(self, data: str, row: str, col: str, out: str, mat: SparseMatrix) -> None:
        ...
    @typing.overload
    def __init__(self, data: typing.SupportsInt | typing.SupportsIndex, row: typing.SupportsInt | typing.SupportsIndex, col: typing.SupportsInt | typing.SupportsIndex, out: typing.SupportsInt | typing.SupportsIndex, mat: SparseMatrix) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GetQWRotateAngle_Int_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class GetRotateAngle_Int_Int(SelfAdjointOperator):
    @typing.overload
    def __init__(self, lhs_reg: str, rhs_reg: str, out_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_reg: typing.SupportsInt | typing.SupportsIndex, rhs_reg: typing.SupportsInt | typing.SupportsIndex, out_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GetRotateAngle_Int_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class GetRowAddr(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg_offset: str, reg_row: str, row_size: typing.SupportsInt | typing.SupportsIndex, reg_row_offset: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_offset: typing.SupportsInt | typing.SupportsIndex, reg_row: typing.SupportsInt | typing.SupportsIndex, row_size: typing.SupportsInt | typing.SupportsIndex, reg_row_offset: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class GlobalPhase(BaseOperator):
    def __init__(self, phase: typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> GlobalPhase:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> GlobalPhase:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GlobalPhase:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> GlobalPhase:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> GlobalPhase:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GlobalPhase:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Hadamard_Bool(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg_in: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_in: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Hadamard_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Hadamard_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Hadamard_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Hadamard_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Hadamard_Int(SelfAdjointOperator):
    """

    Apply Hadamard transform to an integer register.

    Creates an equal superposition over all integer values from 0 to 2^n - 1
    for the specified number of digits.

    Args:
        reg_in: Name/ID of the input register.
        n_digits: Number of digits (qubits) to apply Hadamard to.

    Example:
        Hadamard_Int("q", 4)(state)  # Superpose q over 0..15
    """
    @typing.overload
    def __init__(self, reg_in: str, n_digits: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_in: typing.SupportsInt | typing.SupportsIndex, n_digits: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Hadamard_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Hadamard_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Int:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Hadamard_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Hadamard_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Int:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Hadamard_Int_Full(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg_in: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_in: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Hadamard_Int_Full:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Hadamard_Int_Full:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int_Full:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Int_Full:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int_Full:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int_Full:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int_Full:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int_Full:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Hadamard_Int_Full:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Hadamard_Int_Full:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int_Full:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Int_Full:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int_Full:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int_Full:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Int_Full:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Int_Full:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Hadamard_Partial(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg_in: str, qubit_positions: set) -> None:
        ...
    @typing.overload
    def __init__(self, reg_in: typing.SupportsInt | typing.SupportsIndex, qubit_positions: set) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Hadamard_Partial:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Hadamard_Partial:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Partial:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Partial:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Partial:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Partial:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Partial:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Partial:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Hadamard_Partial:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Hadamard_Partial:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Partial:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_Partial:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Partial:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Partial:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_Partial:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_Partial:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Init_Unsafe(SelfAdjointOperator):
    """

    Initialize a register to a specific value (unsafe).

    Sets the register to a classical value without checking normalization.
    Use with caution as it modifies amplitudes directly.

    Args:
        reg: Register name (str) or ID (int).
        value: Classical value to set.

    Example:
        Init_Unsafe("q", 5)(state)  # Set register q to value 5
    """
    @typing.overload
    def __init__(self, reg: str, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, id: typing.SupportsInt | typing.SupportsIndex, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class IsZero_UInt(SelfAdjointOperator):
    """
    Zero test of an unsigned integer register.

    Computes: flag ^= (reg == 0), with the operand zero-extended to the
    full-precision domain before comparing.

    Args:
        reg: Name/ID of the UnsignedInteger input register.
        flag: Name/ID of the width-1 Boolean flag register (result is XORed in).

    Example:
        IsZero_UInt("a", "flag")(state)  # flag ^= (a == 0)
    """
    @typing.overload
    def __init__(self, reg: str, flag: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> IsZero_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> IsZero_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> IsZero_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> IsZero_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> IsZero_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> IsZero_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> IsZero_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> IsZero_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> IsZero_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> IsZero_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> IsZero_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> IsZero_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> IsZero_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> IsZero_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> IsZero_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> IsZero_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Less_SInt_SInt(SelfAdjointOperator):
    """
    Signed less-than comparison of two signed integer registers.

    Computes: flag ^= (lhs < rhs), where both operands are sign-extended to the
    full-precision comparison domain (64-bit) before comparing.

    Args:
        lhs: Name/ID of the SignedInteger left operand register.
        rhs: Name/ID of the SignedInteger right operand register.
        flag: Name/ID of the width-1 Boolean flag register (result is XORed in).

    Example:
        Less_SInt_SInt("a", "b", "flag")(state)  # flag ^= (a < b)
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, flag: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Less_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Less_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Less_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Less_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Less_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Less_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Less_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Less_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Less_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Less_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Less_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Less_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Less_UInt_UInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, left_reg: str, right_reg: str, less_flag_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, left_id: typing.SupportsInt | typing.SupportsIndex, right_id: typing.SupportsInt | typing.SupportsIndex, less_flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Less_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Less_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Less_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Less_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Less_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Less_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Less_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Less_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Less_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Less_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Less_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Less_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Less_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class MeasureZ:
    """

    Projective Z-basis (computational basis) measurement.

    Samples an outcome for one or more registers according to the Born rule,
    using the seedable global random engine (see set_seed()). Collapses the
    state onto the sampled branch and renormalizes it in place.

    This operation is non-unitary and irreversible (no dag()).

    Example:
        ps.set_seed(0)
        outcome, prob = ps.MeasureZ("q")(state)
    """
    def __call__(self, state: SparseState) -> tuple[list[int], float]:
        ...
    @typing.overload
    def __init__(self, register_names: collections.abc.Sequence[str]) -> None:
        ...
    @typing.overload
    def __init__(self, register_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_name: str) -> None:
        ...
    @typing.overload
    def __init__(self, register_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def registers(self) -> list[int]:
        ...
class Mod_Mult_UInt_ConstUInt_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, a: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex, N: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
                     Create a modular multiplication operator.

                     Computes: |y⟩ → |y * a^(2^x) mod N⟩

                     Args:
                         reg: Name of the operand register
                         a: Base for exponentiation
                         x: Power of 2 exponent (computes a^(2^x))
                         N: Modulus
        """
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, a: typing.SupportsInt | typing.SupportsIndex, x: typing.SupportsInt | typing.SupportsIndex, N: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Mod_Mult_UInt_ConstUInt_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class ModuleInheritance_Test(BaseOperator):
    def __init__(self) -> None:
        ...
class ModuleInheritance_Test_SelfAdjoint(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class MoveBackRegister:
    def __call__(self, arg0: SparseState) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class MulOverflow_UInt_UInt(SelfAdjointOperator):
    """
    Multiplication overflow flag relative to the res width.

    Computes: flag ^= (lhs * rhs does not fit in w bits), where w is the width of
    res; the product is evaluated at full precision (128-bit, via 64-bit hi/lo
    decomposition). out/res 参数仅提供宽度,不读其值.

    Args:
        lhs: Name/ID of the first input register.
        rhs: Name/ID of the second input register.
        res: Name/ID of the register providing the target width w (not read).
        flag: Name/ID of the width-1 Boolean flag register (result is XORed in).

    Example:
        MulOverflow_UInt_UInt("a", "b", "result", "flag")(state)  # flag ^= (a*b overflows result width)
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str, flag: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex, flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> MulOverflow_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Mul_UInt_UInt(SelfAdjointOperator):
    """
    Multiply two unsigned integer registers.

    Computes: res ^= lhs * rhs, taking the low 64 bits of the full-precision
    (128-bit) product, truncated to mod 2^res_width before being XORed into res.

    Args:
        lhs: Name/ID of the first input register.
        rhs: Name/ID of the second input register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Mul_UInt_UInt("a", "b", "result")(state)  # result ^= a * b
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Mul_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Mul_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Mul_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Mul_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Mul_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Mul_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Mul_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Mul_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Mul_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Mul_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Mul_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Mul_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Mul_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Mul_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Mul_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Mul_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Mult_UInt_ConstUInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, input_reg: str, multiplier: typing.SupportsInt | typing.SupportsIndex, output_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, input_id: typing.SupportsInt | typing.SupportsIndex, multiplier: typing.SupportsInt | typing.SupportsIndex, output_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Neg_UInt(SelfAdjointOperator):
    """
    Negate an unsigned integer register.

    Computes: res ^= 0 - reg (two's complement negation on the unsigned 64-bit
    wraparound domain), truncated to mod 2^res_width before being XORed into res.

    Args:
        reg: Name/ID of the input register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Neg_UInt("a", "result")(state)  # result ^= -a
    """
    @typing.overload
    def __init__(self, reg: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Neg_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Neg_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Neg_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Neg_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Neg_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Neg_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Neg_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Neg_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Neg_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Neg_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Neg_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Neg_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Neg_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Neg_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Neg_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Neg_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Negative_SInt(SelfAdjointOperator):
    """
    Negativity test of a signed integer register.

    Computes: flag ^= (reg < 0), where reg is sign-extended from its two's
    complement bit pattern before comparing against 0.

    Args:
        reg: Name/ID of the SignedInteger input register.
        flag: Name/ID of the width-1 Boolean flag register (result is XORed in).

    Example:
        Negative_SInt("a", "flag")(state)  # flag ^= (a < 0)
    """
    @typing.overload
    def __init__(self, reg: str, flag: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Negative_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Negative_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Negative_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Negative_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Negative_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Negative_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Negative_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Negative_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Negative_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Negative_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Negative_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Negative_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Negative_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Negative_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Negative_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Negative_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Normalize(SelfAdjointOperator):
    """

    Normalize the quantum state.

    Ensures the state vector has unit norm by dividing all amplitudes
    by the total norm. Call after operations that may leave the state
    unnormalized.

    Example:
        Normalize()(state)
    """
    def __init__(self) -> None:
        ...
class Or_UInt_UInt(SelfAdjointOperator):
    """
    Bitwise OR of two unsigned integer registers.

    Computes: res ^= lhs | rhs, operands zero-extended, result truncated to
    mod 2^res_width before being XORed into res.

    Args:
        lhs: Name/ID of the first input register.
        rhs: Name/ID of the second input register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Or_UInt_UInt("a", "b", "result")(state)  # result ^= a | b
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Or_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Or_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Or_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Or_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Or_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Or_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Or_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Or_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Or_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Or_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Or_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Or_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Or_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Or_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Or_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Or_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Overflow_SInt_SInt(SelfAdjointOperator):
    """
    Signed-addition overflow flag at the res width.

    Computes: flag ^= overflow(lhs + rhs) at width w of res: operands are
    sign-extended, truncated to w bits, and the same-sign/addends/result-sign-
    flip rule is applied. out/res 参数仅提供宽度,不读其值.

    Args:
        lhs: Name/ID of the SignedInteger left operand register.
        rhs: Name/ID of the SignedInteger right operand register.
        res: Name/ID of the register providing the target width w (not read).
        flag: Name/ID of the width-1 Boolean flag register (result is XORed in).

    Example:
        Overflow_SInt_SInt("a", "b", "result", "flag")(state)  # flag ^= signed overflow of a+b at result width
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str, flag: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex, flag_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Overflow_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Overflow_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Overflow_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Overflow_SInt_SInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Overflow_SInt_SInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class PartialTrace:
    def __call__(self, state: SparseState) -> tuple[list[int], float]:
        ...
    @typing.overload
    def __init__(self, partial_trace_register_names: collections.abc.Sequence[str]) -> None:
        ...
    @typing.overload
    def __init__(self, partial_trace_register_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, single_register_name: str) -> None:
        ...
    @typing.overload
    def __init__(self, single_register_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class PartialTraceSelect:
    def __call__(self, state: SparseState) -> float:
        ...
    @typing.overload
    def __init__(self, name_value_map: collections.abc.Mapping[str, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, id_value_map: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, reg_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], select_values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class PartialTraceSelectRange:
    def __call__(self, state: SparseState) -> float:
        ...
    @typing.overload
    def __init__(self, register_name: str, select_range: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_id: typing.SupportsInt | typing.SupportsIndex, select_range: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class Phase_Bool(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Phase_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Phase_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Phase_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Phase_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Phase_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Phase_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Phase_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Phase_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Phase_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Phase_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Phase_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Phase_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Phase_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Phase_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Phase_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Phase_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class PlusOneAndOverflow(BaseOperator):
    def __init__(self, main_reg: str, overflow: str) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> PlusOneAndOverflow:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> PlusOneAndOverflow:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> PlusOneAndOverflow:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> PlusOneAndOverflow:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> PlusOneAndOverflow:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> PlusOneAndOverflow:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> PlusOneAndOverflow:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> PlusOneAndOverflow:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> PlusOneAndOverflow:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> PlusOneAndOverflow:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> PlusOneAndOverflow:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> PlusOneAndOverflow:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> PlusOneAndOverflow:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> PlusOneAndOverflow:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> PlusOneAndOverflow:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> PlusOneAndOverflow:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Pop(BaseOperator):
    @typing.overload
    def __init__(self, reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Probability:
    """

    Read-only Born-rule probability query (does not modify the state).

    Computes the probability that the given register(s) hold the given
    value(s). Useful for QIF/QWHILE-style dynamic branching conditions and
    for Born-rule conformance checks against a dense-state reference.

    Example:
        p = ps.Probability("q", 5)(state)
        dist = ps.Probability.distribution(state, "q")  # full outcome distribution
    """
    @staticmethod
    @typing.overload
    def distribution(state: SparseState, register_id: typing.SupportsInt | typing.SupportsIndex) -> dict[int, float]:
        ...
    @staticmethod
    @typing.overload
    def distribution(state: SparseState, register_name: str) -> dict[int, float]:
        ...
    def __call__(self, state: SparseState) -> float:
        ...
    @typing.overload
    def __init__(self, name_value_map: collections.abc.Mapping[str, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, id_value_map: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_names: collections.abc.Sequence[str], target_values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], target_values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_name: str, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, register_id: typing.SupportsInt | typing.SupportsIndex, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def registers(self) -> list[int]:
        ...
    @property
    def values(self) -> list[int]:
        ...
class Push(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, garbage: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, garbage: str) -> None:
        ...
class QFT(BaseOperator):
    """

    Quantum Fourier Transform on a register.

    Applies the QFT to transform between computational and Fourier bases.
    Commonly used in phase estimation and Shor's algorithm.

    Args:
        reg_name: Name of the register to transform (str) or register ID (int).

    Example:
        QFT("data")(state)  # Apply QFT
        # ... computation ...
        InverseQFT("data")(state)  # Apply inverse QFT
    """
    @typing.overload
    def __init__(self, reg_name: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> QFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> QFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> QFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> QFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> QFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> QFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> QFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> QFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> QFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> QFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> QFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> QFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> QFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> QFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> QFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> QFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class QRAMCircuit_qutrit:
    @typing.overload
    def __init__(self, addr_size: typing.SupportsInt | typing.SupportsIndex, data_size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, addr_size: typing.SupportsInt | typing.SupportsIndex, data_size: typing.SupportsInt | typing.SupportsIndex, memory: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, addr_size: typing.SupportsInt | typing.SupportsIndex, data_size: typing.SupportsInt | typing.SupportsIndex, memory: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @property
    def address_size(self) -> int:
        ...
    @property
    def data_size(self) -> int:
        ...
class QRAMLoad(SelfAdjointOperator):
    """

    Load classical data into quantum superposition via QRAM.

    Performs the QRAM load operation, creating a superposition where each
    basis state is entangled with its corresponding data value.

    Args:
        qram: QRAMCircuit_qutrit instance containing the memory.
        addr_reg: Name/ID of the address register.
        data_reg: Name/ID of the data register.

    Example:
        qram = QRAMCircuit_qutrit(addr_size=3, data_size=4, memory=data)
        QRAMLoad(qram, "address", "data")(state)

    Note:
        Use QRAMLoadFast for optimized execution when address distribution
        is uniform.
    """
    version: typing.ClassVar[str] = ''
    @typing.overload
    def __init__(self, qram: QRAMCircuit_qutrit, addr_reg: str, data_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, qram: QRAMCircuit_qutrit, addr_reg_id: typing.SupportsInt | typing.SupportsIndex, data_reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> QRAMLoad:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> QRAMLoad:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoad:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> QRAMLoad:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoad:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoad:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoad:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoad:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> QRAMLoad:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> QRAMLoad:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoad:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> QRAMLoad:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoad:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoad:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoad:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoad:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
    @property
    def qram_circuit(self) -> QRAMCircuit_qutrit:
        ...
class QRAMLoadFast(SelfAdjointOperator):
    @typing.overload
    def __init__(self, qram: QRAMCircuit_qutrit, addr_reg: str, data_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, qram: QRAMCircuit_qutrit, addr_reg_id: typing.SupportsInt | typing.SupportsIndex, data_reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> QRAMLoadFast:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> QRAMLoadFast:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoadFast:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> QRAMLoadFast:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoadFast:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoadFast:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoadFast:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoadFast:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> QRAMLoadFast:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> QRAMLoadFast:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoadFast:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> QRAMLoadFast:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoadFast:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoadFast:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> QRAMLoadFast:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> QRAMLoadFast:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class QuantumBinarySearch_Fast(SelfAdjointOperator):
    @typing.overload
    def __init__(self, qram: QRAMCircuit_qutrit, address_offset_register: str, total_length: typing.SupportsInt | typing.SupportsIndex, target_register: str, result_register: str) -> None:
        ...
    @typing.overload
    def __init__(self, qram: QRAMCircuit_qutrit, address_offset_register: typing.SupportsInt | typing.SupportsIndex, total_length: typing.SupportsInt | typing.SupportsIndex, target_register: typing.SupportsInt | typing.SupportsIndex, result_register: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class RX_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class RY_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class RZ_Bool(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> RZ_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> RZ_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> RZ_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> RZ_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> RZ_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> RZ_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> RZ_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> RZ_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> RZ_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> RZ_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> RZ_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> RZ_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> RZ_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> RZ_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> RZ_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> RZ_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Reflection_Bool(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg: str, inverse: bool = False) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, inverse: bool = False) -> None:
        ...
    @typing.overload
    def __init__(self, reg_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], inverse: bool = False) -> None:
        ...
    @typing.overload
    def __init__(self, regs: collections.abc.Sequence[str], inverse: bool = False) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Reflection_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Reflection_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Reflection_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Reflection_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Reflection_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Reflection_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Reflection_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Reflection_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Reflection_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Reflection_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Reflection_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Reflection_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Reflection_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Reflection_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Reflection_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Reflection_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class RemoveRegister:
    def __call__(self, arg0: SparseState) -> None:
        ...
    @typing.overload
    def __init__(self, name: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Reset:
    """

    Reset one or more registers to a definite classical value (default 0).

    Implemented as measurement (collapse + renormalize) followed by a
    deterministic classical correction, matching hardware active-reset and
    OriginIR-ext RESET semantics. Returns the pre-reset measured outcome.

    Example:
        ps.set_seed(0)
        measured = ps.Reset("q")(state)   # reset "q" to 0
        measured = ps.Reset("q", 3)(state)  # reset "q" to 3
    """
    def __call__(self, state: SparseState) -> list[int]:
        ...
    @typing.overload
    def __init__(self, register_names: collections.abc.Sequence[str]) -> None:
        ...
    @typing.overload
    def __init__(self, register_names: collections.abc.Sequence[str], targets: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], targets: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, register_name: str, target: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, register_id: typing.SupportsInt | typing.SupportsIndex, target: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @property
    def registers(self) -> list[int]:
        ...
    @property
    def target_values(self) -> list[int]:
        ...
class Rot_Bool(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, matrix: ...) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, matrix: ...) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, matrix: ...) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, matrix: ...) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, matrix: typing.Annotated[collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex], "FixedSize(4)"]) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, matrix: typing.Annotated[collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex], "FixedSize(4)"]) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, matrix: typing.Annotated[collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex], "FixedSize(4)"]) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, matrix: typing.Annotated[collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex], "FixedSize(4)"]) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Rot_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Rot_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Rot_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Rot_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Rot_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Rot_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Rot_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Rot_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Rot_GeneralStatePrep(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, state_vector: collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, state_vector: collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralStatePrep:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralStatePrep:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralStatePrep:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Rot_GeneralUnitary(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, unitary_matrix: DenseMatrix_complex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, unitary_matrix: DenseMatrix_complex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralUnitary:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralUnitary:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralUnitary:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralUnitary:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Rot_GeneralUnitary:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class SX_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class Select_Bool_UInt_UInt(SelfAdjointOperator):
    """
    Select between two unsigned integer registers by a Boolean condition.

    Computes: res ^= (cond ? lhs : rhs), where cond is a width-1 Boolean register
    read at bit 0; the selected value is truncated to mod 2^res_width before
    being XORed into res.

    Args:
        cond: Name/ID of the width-1 Boolean condition register.
        lhs: Name/ID of the register selected when cond == 1.
        rhs: Name/ID of the register selected when cond == 0.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Select_Bool_UInt_UInt("c", "a", "b", "result")(state)  # result ^= a if c else b
    """
    @typing.overload
    def __init__(self, cond: str, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, cond_id: typing.SupportsInt | typing.SupportsIndex, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Select_Bool_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class SelfAdjointOperator(BaseOperator):
    def dag(self, arg0: SparseState) -> None:
        ...
class S_Bool(Phase_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class ShiftLeft_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, shift_bits: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, shift_bits: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class ShiftRight_InPlace(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, shift_bits: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, shift_bits: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> ShiftRight_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> ShiftRight_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftRight_InPlace:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight_InPlace:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> ShiftRight_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> ShiftRight_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftRight_InPlace:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight_InPlace:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    def dag(self, state: SparseState) -> None:
        """
        Apply the adjoint (inverse) of this operation.

        Args:
            state: The quantum state to operate on.

        Note: Only available for self-adjoint operators.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class SortByAmplitude(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class SortByKey(SelfAdjointOperator):
    @typing.overload
    def __init__(self, key: str) -> None:
        ...
    @typing.overload
    def __init__(self, key_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class SortByKey2(SelfAdjointOperator):
    def __init__(self, key1: str, key2: str) -> None:
        ...
class SortExceptBit(SelfAdjointOperator):
    @typing.overload
    def __init__(self, key: str, digit: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, key_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class SortExceptKey(SelfAdjointOperator):
    @typing.overload
    def __init__(self, key: str) -> None:
        ...
    @typing.overload
    def __init__(self, key_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class SortExceptKeyHadamard(SelfAdjointOperator):
    def __init__(self, key: str, qubit_ids: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class SortUnconditional(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class SparseMatrix:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, elements: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex], sparsity: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], data_size: typing.SupportsInt | typing.SupportsIndex, nnz_col: typing.SupportsInt | typing.SupportsIndex, n_row: typing.SupportsInt | typing.SupportsIndex, positive_only: bool) -> None:
        ...
    def get_data(self) -> list[int]:
        ...
    def get_sparsity_offset(self) -> int:
        ...
    @property
    def data_size(self) -> int:
        ...
    @property
    def elements(self) -> list[int]:
        ...
    @property
    def n_row(self) -> int:
        ...
    @property
    def nnz_col(self) -> int:
        ...
    @property
    def positive_only(self) -> bool:
        ...
    @property
    def sparsity(self) -> list[int]:
        ...
class SparseState:
    """

    Sparse quantum state representation.

    Stores only non-zero amplitude entries, making it efficient for states
    with limited superposition. Works with the global System registry.

    Example:
        state = SparseState()
        AddRegister("q", UnsignedInteger, 4)(state)
        Hadamard_Int("q")(state)

    Note:
        The sparse representation is memory-efficient but may be slower
        for dense superposition states.
    """
    def __init__(self) -> None:
        """
        Create an empty sparse quantum state
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def _cpp_ptr(self) -> int:
        """
        Return the raw C++ SparseState* address as uintptr_t.
        """
    def empty(self) -> bool:
        ...
    def size(self) -> int:
        ...
    def to_string(self, display: typing.SupportsInt | typing.SupportsIndex = 0, precision: typing.SupportsInt | typing.SupportsIndex = 0) -> str:
        """
        Return a formatted string representation of the state.

        Args:
            display: Display mode flags (StatePrintDisplay values).
            precision: Number of decimal places for floating-point numbers.
        """
    @property
    def basis_states(self) -> list[System]:
        ...
class SplitRegister:
    def __call__(self, arg0: SparseState) -> int:
        ...
    def __init__(self, first: str, second: str, size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Sqrt_Div_Arccos_Int_UInt(SelfAdjointOperator):
    @typing.overload
    def __init__(self, lhs_reg: str, rhs_reg: str, out_reg: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_reg: typing.SupportsInt | typing.SupportsIndex, rhs_reg: typing.SupportsInt | typing.SupportsIndex, out_reg: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Sqrt_UInt(SelfAdjointOperator):
    """
    Integer square root of an unsigned integer register.

    Computes: res ^= isqrt(reg) = floor(sqrt(reg)) via an integer-only bitwise
    algorithm (CPU and CUDA agree bit-for-bit), truncated to mod 2^res_width
    before being XORed into res.

    Args:
        reg: Name/ID of the input register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Sqrt_UInt("a", "result")(state)  # result ^= floor(sqrt(a))
    """
    @typing.overload
    def __init__(self, reg: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Sqrt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Sqrt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sqrt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Sqrt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Sqrt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sqrt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class StateEqualExceptKey:
    def __call__(self, arg0: System, arg1: System) -> int:
        ...
    def __init__(self, excluded_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class StateEqualExceptQubits:
    def __call__(self, arg0: System, arg1: System) -> int:
        ...
    def __init__(self, target_id: typing.SupportsInt | typing.SupportsIndex, excluded_qubits: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class StateHashExceptKey:
    def __call__(self, arg0: System) -> int:
        ...
    def __init__(self, excluded_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class StateHashExceptQubits:
    def __call__(self, arg0: System) -> int:
        ...
    def __init__(self, target_id: typing.SupportsInt | typing.SupportsIndex, excluded_qubits: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class StateLessExceptKey:
    def __call__(self, arg0: System, arg1: System) -> int:
        ...
    def __init__(self, excluded_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class StateLessExceptQubits:
    def __call__(self, arg0: System, arg1: System) -> int:
        ...
    def __init__(self, target_id: typing.SupportsInt | typing.SupportsIndex, excluded_qubits: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class StatePrint(SelfAdjointOperator):
    on: typing.ClassVar[bool] = True
    def __call__(self, state: typing.Any) -> str:
        """
        Return formatted state string for the given SparseState.
        """
    @typing.overload
    def __init__(self, disp: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, disp: typing.SupportsInt | typing.SupportsIndex, precision: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, disp: StatePrintDisplay) -> None:
        ...
class StatePrintDisplay:
    """
    Members:

      Default

      Detail

      Binary

      Prob
    """
    Binary: typing.ClassVar[StatePrintDisplay]  # value = <StatePrintDisplay.Binary: 2>
    Default: typing.ClassVar[StatePrintDisplay]  # value = <StatePrintDisplay.Default: 0>
    Detail: typing.ClassVar[StatePrintDisplay]  # value = <StatePrintDisplay.Detail: 1>
    Prob: typing.ClassVar[StatePrintDisplay]  # value = <StatePrintDisplay.Prob: 4>
    __members__: typing.ClassVar[dict[str, StatePrintDisplay]]  # value = {'Default': <StatePrintDisplay.Default: 0>, 'Detail': <StatePrintDisplay.Detail: 1>, 'Binary': <StatePrintDisplay.Binary: 2>, 'Prob': <StatePrintDisplay.Prob: 4>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class StateStorage:
    def __init__(self) -> None:
        ...
    @property
    def value(self) -> int:
        ...
class StateStorageType:
    """
    Members:

      General

      UnsignedInteger

      SignedInteger

      Boolean

      Rational
    """
    Boolean: typing.ClassVar[StateStorageType]  # value = <StateStorageType.Boolean: 3>
    General: typing.ClassVar[StateStorageType]  # value = <StateStorageType.General: 0>
    Rational: typing.ClassVar[StateStorageType]  # value = <StateStorageType.Rational: 4>
    SignedInteger: typing.ClassVar[StateStorageType]  # value = <StateStorageType.SignedInteger: 2>
    UnsignedInteger: typing.ClassVar[StateStorageType]  # value = <StateStorageType.UnsignedInteger: 1>
    __members__: typing.ClassVar[dict[str, StateStorageType]]  # value = {'General': <StateStorageType.General: 0>, 'UnsignedInteger': <StateStorageType.UnsignedInteger: 1>, 'SignedInteger': <StateStorageType.SignedInteger: 2>, 'Boolean': <StateStorageType.Boolean: 3>, 'Rational': <StateStorageType.Rational: 4>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Sub_UInt_UInt(SelfAdjointOperator):
    """
    Subtract two unsigned integer registers.

    Computes: res ^= lhs - rhs, evaluated on the unsigned 64-bit wraparound
    domain, then truncated to mod 2^res_width before being XORed into res.

    Args:
        lhs: Name/ID of the minuend register.
        rhs: Name/ID of the subtrahend register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Sub_UInt_UInt("a", "b", "result")(state)  # result ^= a - b
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Sub_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Sub_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sub_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sub_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sub_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sub_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sub_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sub_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Sub_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Sub_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sub_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sub_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sub_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sub_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sub_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sub_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Swap_Bool_Bool(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg1: str, digit1: typing.SupportsInt | typing.SupportsIndex, reg2: str, digit2: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg1_id: typing.SupportsInt | typing.SupportsIndex, digit1: typing.SupportsInt | typing.SupportsIndex, reg2_id: typing.SupportsInt | typing.SupportsIndex, digit2: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Swap_Bool_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Swap_Bool_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Swap_Bool_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Swap_Bool_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_Bool_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_Bool_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_Bool_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_Bool_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Swap_Bool_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Swap_Bool_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Swap_Bool_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Swap_Bool_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_Bool_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_Bool_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_Bool_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_Bool_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Swap_General_General(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg1: str, reg2: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg1_id: typing.SupportsInt | typing.SupportsIndex, reg2_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Swap_General_General:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Swap_General_General:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Swap_General_General:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Swap_General_General:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_General_General:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_General_General:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_General_General:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_General_General:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Swap_General_General:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Swap_General_General:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Swap_General_General:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Swap_General_General:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_General_General:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_General_General:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Swap_General_General:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Swap_General_General:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class System:
    """

    Quantum system managing named registers.

    The System class provides the foundation for register management. It tracks
    register names, types, and sizes via a global registry shared by all
    SparseState instances.

    Example:
        system = System()
        state = SparseState()

    Attributes:
        registers: Dict mapping register names to their metadata.
        amplitude: Amplitude coefficient for this system instance.
    """
    __hash__: typing.ClassVar[None] = None
    max_register_count: typing.ClassVar[int] = 0
    max_register_map: typing.ClassVar[int] = 0
    max_system_size: typing.ClassVar[int] = 0
    name_register_map: typing.ClassVar[list] = list()
    reusable_registers: typing.ClassVar[list] = list()
    temporal_registers: typing.ClassVar[list] = list()
    @staticmethod
    def add_register(arg0: str, arg1: StateStorageType, arg2: typing.SupportsInt | typing.SupportsIndex) -> int:
        ...
    @staticmethod
    @typing.overload
    def add_register_synchronous(arg0: str, arg1: StateStorageType, arg2: typing.SupportsInt | typing.SupportsIndex, arg3: SparseState) -> int:
        ...
    @staticmethod
    @typing.overload
    def add_register_synchronous(arg0: str, arg1: StateStorageType, arg2: typing.SupportsInt | typing.SupportsIndex, arg3: collections.abc.Sequence[System]) -> int:
        ...
    @staticmethod
    def clear() -> None:
        ...
    @staticmethod
    def get_activated_register_size() -> int:
        ...
    @staticmethod
    def get_id(arg0: str) -> int:
        ...
    @staticmethod
    def get_qubit_count() -> int:
        ...
    @staticmethod
    def get_register_info(arg0: str) -> tuple[str, StateStorageType, int, bool]:
        ...
    @staticmethod
    def name_of(arg0: typing.SupportsInt | typing.SupportsIndex) -> str:
        ...
    @staticmethod
    @typing.overload
    def remove_register(arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @staticmethod
    @typing.overload
    def remove_register(arg0: str) -> None:
        ...
    @staticmethod
    @typing.overload
    def remove_register_synchronous(arg0: typing.SupportsInt | typing.SupportsIndex, arg1: collections.abc.Sequence[System]) -> None:
        ...
    @staticmethod
    @typing.overload
    def remove_register_synchronous(arg0: str, arg1: collections.abc.Sequence[System]) -> None:
        ...
    @staticmethod
    def set_register_type(arg0: str, arg1: StateStorageType) -> None:
        ...
    @staticmethod
    @typing.overload
    def size_of(arg0: str) -> int:
        ...
    @staticmethod
    @typing.overload
    def size_of(arg0: typing.SupportsInt | typing.SupportsIndex) -> int:
        ...
    @staticmethod
    @typing.overload
    def status_of(arg0: str) -> bool:
        ...
    @staticmethod
    @typing.overload
    def status_of(arg0: typing.SupportsInt | typing.SupportsIndex) -> bool:
        ...
    @staticmethod
    @typing.overload
    def type_of(arg0: str) -> StateStorageType:
        ...
    @staticmethod
    @typing.overload
    def type_of(arg0: typing.SupportsInt | typing.SupportsIndex) -> StateStorageType:
        ...
    @staticmethod
    def update_max_size(arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __eq__(self, arg0: System) -> bool:
        ...
    def __init__(self) -> None:
        """
        Create an empty quantum system
        """
    def __less__(self, arg0: System) -> bool:
        ...
    def __ne__(self, arg0: System) -> bool:
        ...
    def __str__(self) -> str:
        ...
    @typing.overload
    def get(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> StateStorage:
        ...
    @typing.overload
    def get(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> StateStorage:
        ...
    @typing.overload
    def last_register(self) -> StateStorage:
        ...
    @typing.overload
    def last_register(self) -> StateStorage:
        ...
    @typing.overload
    def to_string(self) -> str:
        ...
    @typing.overload
    def to_string(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> str:
        ...
    @property
    def amplitude(self) -> complex:
        ...
    @property
    def registers(self) -> typing.Annotated[list[StateStorage], "FixedSize(64)"]:
        ...
class TestRemovable(SelfAdjointOperator):
    @typing.overload
    def __init__(self, register_name: str) -> None:
        ...
    @typing.overload
    def __init__(self, register_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class T_Bool(Phase_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class U2_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class U3_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda_: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class ViewNormalization(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class X_Bool(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> X_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> X_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> X_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> X_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> X_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> X_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> X_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> X_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> X_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> X_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> X_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> X_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> X_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> X_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> X_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> X_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Xor_UInt_UInt(SelfAdjointOperator):
    """
    Bitwise XOR of two unsigned integer registers.

    Computes: res ^= lhs ^ rhs, operands zero-extended, result truncated to
    mod 2^res_width before being XORed into res.

    Args:
        lhs: Name/ID of the first input register.
        rhs: Name/ID of the second input register.
        res: Name/ID of the output register (result is XORed in).

    Example:
        Xor_UInt_UInt("a", "b", "result")(state)  # result ^= a ^ b
    """
    @typing.overload
    def __init__(self, lhs: str, rhs: str, res: str) -> None:
        ...
    @typing.overload
    def __init__(self, lhs_id: typing.SupportsInt | typing.SupportsIndex, rhs_id: typing.SupportsInt | typing.SupportsIndex, res_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Xor_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Xor_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Xor_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Xor_UInt_UInt:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Xor_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Xor_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Xor_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Xor_UInt_UInt:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Xor_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Xor_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Xor_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Xor_UInt_UInt:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Xor_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Xor_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Xor_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Xor_UInt_UInt:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Y_Bool(BaseOperator):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> Y_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Y_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Y_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Y_Bool:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Y_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Y_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Y_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Y_Bool:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Y_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Y_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Y_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Y_Bool:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Y_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Y_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Y_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Y_Bool:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class ZeroConditionalPhaseFlip(SelfAdjointOperator):
    @typing.overload
    def __init__(self, reg_ids: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, regs: collections.abc.Sequence[str]) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ZeroConditionalPhaseFlip:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
class Z_Bool(Phase_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class InverseQFT(BaseOperator):
    """

    Inverse Quantum Fourier Transform on a register.

    Applies the inverse QFT to transform from Fourier basis back to
    computational basis.

    Args:
        reg_name: Name of the register (str) or register ID (int).
    """
    @typing.overload
    def __init__(self, reg_name: str) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def clear_control_all_ones(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_bit(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_by_value(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    def clear_control_nonzeros(self) -> None:
        """
        Clear all control conditions of the specified type.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: str) -> InverseQFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> InverseQFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> InverseQFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> InverseQFT:
        """
        Condition this operation on registers where all bits are 1.

        Calling this method replaces prior all-ones conditions. Pass the list
        overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> InverseQFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> InverseQFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> InverseQFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> InverseQFT:
        """
        Condition this operation on a specific bit position.

        Calling this method replaces prior bit conditions. Pass the list-of-pairs
        overload to require several bits simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> InverseQFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> InverseQFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> InverseQFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> InverseQFT:
        """
        Condition this operation on registers with nonzero values.

        Calling this method replaces prior nonzero-register conditions. Pass the
        list overload to require several registers simultaneously; conditions of
        different kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.

        Returns:
            Self, for method chaining.

        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> InverseQFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> InverseQFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> InverseQFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> InverseQFT:
        """
        Condition this operation on registers holding a specific value.

        Calling this method replaces prior value conditions. Pass the list-of-pairs
        overload to require several values simultaneously; conditions of different
        kinds are combined with logical AND.

        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.

        Returns:
            Self, for method chaining.
        """
    @property
    def condition_variable_all_ones(self) -> list[int]:
        ...
    @property
    def condition_variable_by_bit(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_by_value(self) -> list[tuple[int, int]]:
        ...
    @property
    def condition_variable_nonzeros(self) -> list[int]:
        ...
def combine_systems(to: SparseState, from_: SparseState) -> None:
    ...
def get_seed() -> int:
    """
    Return the current seed of the global random engine.
    """
def merge_system(arg0: System, arg1: System) -> None:
    ...
def print(state: SparseState) -> None:
    """
    Print a SparseState to stdout in detail mode.

    Uses Detail display mode (shows register names and types).
    Output is captured by Jupyter/IPython notebooks.
    """
def remove_system(arg0: System) -> bool:
    ...
def reseed() -> int:
    """
    Reseed the global random engine from its own randomness and return the new seed.
    """
def set_seed(seed: typing.SupportsInt | typing.SupportsIndex) -> None:
    """
    Seed the global random engine used by measurement/reset/PartialTrace.

    Call before MeasureZ/Reset (or PartialTrace/PartialTraceSelect*) to make
    their sampled outcomes reproducible, which is required for deterministic
    replay/testing of a dynamic executor.

    Example:
        ps.set_seed(12345)
        outcome, prob = ps.MeasureZ('q')(state)
    """
def split_systems(state: SparseState, condition_variable_nonzeros: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], condition_variable_all_ones: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], condition_variable_by_bit: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]], condition_variable_by_value: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> SparseState:
    ...
def stateprep_unitary_build_schmidt(state_vector: collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex]) -> DenseMatrix_complex:
    """
    Build unitary for state preparation
    """
def time_seed() -> int:
    """
    Seed the global random engine from the current wall-clock time and return the seed.

    Use set_seed() instead when reproducibility is required.
    """
Binary: StatePrintDisplay  # value = <StatePrintDisplay.Binary: 2>
Boolean: StateStorageType  # value = <StateStorageType.Boolean: 3>
Default: StatePrintDisplay  # value = <StatePrintDisplay.Default: 0>
Detail: StatePrintDisplay  # value = <StatePrintDisplay.Detail: 1>
General: StateStorageType  # value = <StateStorageType.General: 0>
Prob: StatePrintDisplay  # value = <StatePrintDisplay.Prob: 4>
Rational: StateStorageType  # value = <StateStorageType.Rational: 4>
SignedInteger: StateStorageType  # value = <StateStorageType.SignedInteger: 2>
UnsignedInteger: StateStorageType  # value = <StateStorageType.UnsignedInteger: 1>
