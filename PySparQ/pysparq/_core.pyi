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
__all__: list[str] = ['AddAssign_AnyInt_AnyInt', 'AddRegister', 'AddRegisterWithHadamard', 'Add_ConstUInt', 'Add_Mult_UInt_ConstUInt', 'Add_UInt_ConstUInt', 'Add_UInt_UInt', 'Add_UInt_UInt_InPlace', 'Assign', 'BaseOperator', 'Binary', 'Boolean', 'CheckDuplicateKey', 'CheckNan', 'CheckNormalization', 'ClearZero', 'CombineRegister', 'Compare_UInt_UInt', 'CondRot_Rational_Bool', 'CustomArithmetic', 'Default', 'DenseMatrix_complex', 'DenseMatrix_float64', 'Detail', 'Div_Sqrt_Arccos_Int_Int', 'FlipBools', 'General', 'GetMid_UInt_UInt', 'GetRotateAngle_Int_Int', 'GlobalPhase_Int', 'Hadamard_Bool', 'Hadamard_Int', 'Hadamard_Int_Full', 'Hadamard_PartialQubit', 'Init_Unsafe', 'Less_UInt_UInt', 'ModuleInheritance_Test', 'ModuleInheritance_Test_SelfAdjoint', 'MoveBackRegister', 'Mult_UInt_ConstUInt', 'Normalize', 'PartialTrace', 'PartialTraceSelect', 'PartialTraceSelectRange', 'Phase_Bool', 'Pop', 'Prob', 'Push', 'QFT', 'QRAMCircuit_qutrit', 'QRAMLoad', 'QRAMLoadFast', 'RXgate_Bool', 'RYgate_Bool', 'RZgate_Bool', 'Rational', 'Reflection_Bool', 'RemoveRegister', 'Rot_Bool', 'Rot_GeneralStatePrep', 'Rot_GeneralUnitary', 'SXgate_Bool', 'SelfAdjointOperator', 'Sgate_Bool', 'ShiftLeft', 'ShiftRight', 'SignedInteger', 'SortByAmplitude', 'SortByKey', 'SortByKey2', 'SortExceptBit', 'SortExceptKey', 'SortExceptKeyHadamard', 'SortUnconditional', 'SparseMatrix', 'SparseState', 'SplitRegister', 'Sqrt_Div_Arccos_Int_Int', 'StateEqualExceptKey', 'StateEqualExceptQubits', 'StateHashExceptKey', 'StateHashExceptQubits', 'StateLessExceptKey', 'StateLessExceptQubits', 'StatePrint', 'StatePrintDisplay', 'StateStorage', 'StateStorageType', 'Swap_Bool_Bool', 'Swap_General_General', 'System', 'TestRemovable', 'Tgate_Bool', 'U2gate_Bool', 'U3gate_Bool', 'UnsignedInteger', 'ViewNormalization', 'Xgate_Bool', 'Ygate_Bool', 'ZeroConditionalPhaseFlip', 'Zgate_Bool', 'combine_systems', 'inverseQFT', 'merge_system', 'remove_system', 'split_systems', 'stateprep_unitary_build_schmidt']
class AddAssign_AnyInt_AnyInt(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> AddAssign_AnyInt_AnyInt:
        """
        Condition this operation on registers holding a specific value.
        
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
class Add_ConstUInt(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Add_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
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
class Add_Mult_UInt_ConstUInt(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Add_Mult_UInt_ConstUInt:
        """
        Condition this operation on registers holding a specific value.
        
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
class CondRot_Rational_Bool(BaseOperator):
    @typing.overload
    def __init__(self, arg0: str, arg1: str) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CustomArithmetic(SelfAdjointOperator):
    def __init__(self, input_registers: list, input_size: int, output_size: int, func: collections.abc.Callable) -> None:
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
class Div_Sqrt_Arccos_Int_Int(SelfAdjointOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Div_Sqrt_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
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
class GlobalPhase_Int(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> GlobalPhase_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> GlobalPhase_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GlobalPhase_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> GlobalPhase_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> GlobalPhase_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> GlobalPhase_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> GlobalPhase_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> GlobalPhase_Int:
        """
        Condition this operation on registers holding a specific value.
        
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
class Hadamard_PartialQubit(SelfAdjointOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_PartialQubit:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_PartialQubit:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_PartialQubit:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_PartialQubit:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Hadamard_PartialQubit:
        """
        Condition this operation on registers holding a specific value.
        
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
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
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
        inverseQFT("data")(state)  # Apply inverse QFT
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
class RXgate_Bool(Rot_Bool):
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
class RYgate_Bool(Rot_Bool):
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
class RZgate_Bool(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> RZgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> RZgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> RZgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> RZgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> RZgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> RZgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> RZgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> RZgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> RZgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> RZgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> RZgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> RZgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> RZgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> RZgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> RZgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> RZgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
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
class SXgate_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class SelfAdjointOperator(BaseOperator):
    def dag(self, arg0: SparseState) -> None:
        ...
class Sgate_Bool(Phase_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class ShiftLeft(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> ShiftLeft:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> ShiftLeft:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftLeft:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> ShiftLeft:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> ShiftLeft:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftLeft:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftLeft:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftLeft:
        """
        Condition this operation on registers holding a specific value.
        
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
class ShiftRight(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> ShiftRight:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> ShiftRight:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftRight:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> ShiftRight:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> ShiftRight:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> ShiftRight:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> ShiftRight:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> ShiftRight:
        """
        Condition this operation on registers holding a specific value.
        
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
    @typing.overload
    def __init__(self, key1: str, key2: str) -> None:
        ...
    @typing.overload
    def __init__(self, key1_id: typing.SupportsInt | typing.SupportsIndex, key2_id: typing.SupportsInt | typing.SupportsIndex) -> None:
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
    @typing.overload
    def __init__(self, key: str, qubit_ids: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, key_id: typing.SupportsInt | typing.SupportsIndex, qubit_ids: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class SortUnconditional(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class SparseMatrix:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex], arg1: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], arg2: typing.SupportsInt | typing.SupportsIndex, arg3: typing.SupportsInt | typing.SupportsIndex, arg4: typing.SupportsInt | typing.SupportsIndex, arg5: bool) -> None:
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
    def empty(self) -> bool:
        ...
    def size(self) -> int:
        ...
    @property
    def basis_states(self) -> list[System]:
        ...
class SplitRegister:
    def __call__(self, arg0: SparseState) -> int:
        ...
    def __init__(self, first: str, second: str, size: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Sqrt_Div_Arccos_Int_Int(SelfAdjointOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Sqrt_Div_Arccos_Int_Int:
        """
        Condition this operation on registers holding a specific value.
        
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
    def registers(self) -> typing.Annotated[list[StateStorage], "FixedSize(40)"]:
        ...
class TestRemovable(SelfAdjointOperator):
    @typing.overload
    def __init__(self, register_name: str) -> None:
        ...
    @typing.overload
    def __init__(self, register_id: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Tgate_Bool(Phase_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class U2gate_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class U3gate_Bool(Rot_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg: str, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, theta: typing.SupportsFloat | typing.SupportsIndex, phi: typing.SupportsFloat | typing.SupportsIndex, lambda: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class ViewNormalization(SelfAdjointOperator):
    def __init__(self) -> None:
        ...
class Xgate_Bool(SelfAdjointOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Xgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Xgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Xgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Xgate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Xgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Xgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Xgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Xgate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Xgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Xgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Xgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Xgate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Xgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Xgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Xgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Xgate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
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
class Ygate_Bool(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> Ygate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> Ygate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Ygate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Ygate_Bool:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Ygate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Ygate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Ygate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Ygate_Bool:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> Ygate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> Ygate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> Ygate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Ygate_Bool:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> Ygate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> Ygate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> Ygate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> Ygate_Bool:
        """
        Condition this operation on registers holding a specific value.
        
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
class Zgate_Bool(Phase_Bool):
    @typing.overload
    def __init__(self, reg: str, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
    @typing.overload
    def __init__(self, reg_id: typing.SupportsInt | typing.SupportsIndex, digit: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        ...
class inverseQFT(BaseOperator):
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
    def conditioned_by_all_ones(self, cond: str) -> inverseQFT:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[str]) -> inverseQFT:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, cond: typing.SupportsInt | typing.SupportsIndex) -> inverseQFT:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_all_ones(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> inverseQFT:
        """
        Condition this operation on registers where all bits are 1.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> inverseQFT:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> inverseQFT:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> inverseQFT:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_bit(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> inverseQFT:
        """
        Condition this operation on a specific bit position.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Bit position to check (0-indexed).
            conds: List of (register, position) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: str) -> inverseQFT:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[str]) -> inverseQFT:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, cond: typing.SupportsInt | typing.SupportsIndex) -> inverseQFT:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_nonzeros(self, conds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> inverseQFT:
        """
        Condition this operation on registers with nonzero values.
        
        Args:
            cond: Register name (str) or ID (int) to condition on.
            conds: List of register names or IDs for multi-condition.
        
        Returns:
            Self, for method chaining.
        
        Example:
            op.conditioned_by_nonzeros('control_reg')(state)
        """
    @typing.overload
    def conditioned_by_value(self, cond: str, pos: typing.SupportsInt | typing.SupportsIndex) -> inverseQFT:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[str, typing.SupportsInt | typing.SupportsIndex]]) -> inverseQFT:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, cond: typing.SupportsInt | typing.SupportsIndex, pos: typing.SupportsInt | typing.SupportsIndex) -> inverseQFT:
        """
        Condition this operation on registers holding a specific value.
        
        Args:
            cond: Register name (str) or ID (int).
            pos: Value to match.
            conds: List of (register, value) pairs.
        
        Returns:
            Self, for method chaining.
        """
    @typing.overload
    def conditioned_by_value(self, conds: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> inverseQFT:
        """
        Condition this operation on registers holding a specific value.
        
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
def combine_systems(to: SparseState, from: SparseState) -> None:
    ...
def merge_system(arg0: System, arg1: System) -> None:
    ...
def remove_system(arg0: System) -> bool:
    ...
def split_systems(state: SparseState, condition_variable_nonzeros: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], condition_variable_all_ones: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], condition_variable_by_bit: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]], condition_variable_by_value: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]]) -> SparseState:
    ...
def stateprep_unitary_build_schmidt(state_vector: collections.abc.Sequence[typing.SupportsComplex | typing.SupportsFloat | typing.SupportsIndex]) -> DenseMatrix_complex:
    """
    Build unitary for state preparation
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
