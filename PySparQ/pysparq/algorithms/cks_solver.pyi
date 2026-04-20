"""
CKS (Childs-Kothari-Somma) Linear System Solver Implementation
"""

from typing import Callable, Optional

import numpy as np
import pysparq as ps


class ChebyshevPolynomialCoefficient:
    """Computes Chebyshev polynomial coefficients for quantum walk."""

    b: int

    def __init__(self, b: int) -> None: ...
    def C(self, Big: int, Small: int) -> float: ...
    def coef(self, j: int) -> float: ...
    def sign(self, j: int) -> bool: ...
    def step(self, j: int) -> int: ...


def get_coef_positive_only(
    mat_data_size: int, v: int, row: int, col: int
) -> list[complex]:
    """Get rotation matrix coefficients for positive-only matrix elements."""
    ...


def get_coef_common(
    mat_data_size: int, v: int, row: int, col: int
) -> list[complex]:
    """Get rotation matrix coefficients for general (signed) matrix elements."""
    ...


def make_walk_angle_func(
    mat_data_size: int, positive_only: bool
) -> Callable[[int, int, int], list[complex]]:
    """Create walk angle function for a matrix."""
    ...


class SparseMatrixData:
    """Sparse matrix data for quantum simulation."""

    n_row: int
    nnz_col: int
    data: list[int]
    data_size: int
    positive_only: bool
    sparsity_offset: int

    def __init__(
        self,
        n_row: int,
        nnz_col: int,
        data: list[int],
        data_size: int,
        positive_only: bool = ...,
        sparsity_offset: int = ...,
    ) -> None: ...


class SparseMatrix:
    """Sparse matrix representation for CKS algorithm."""

    n_row: int
    nnz_col: int
    data: list[int]
    data_size: int
    positive_only: bool
    sparsity_offset: int

    def __init__(
        self,
        n_row: int,
        nnz_col: int,
        data: list[int],
        data_size: int,
        positive_only: bool = ...,
    ) -> None: ...
    @classmethod
    def from_dense(
        cls,
        matrix: np.ndarray,
        data_size: int = ...,
        positive_only: Optional[bool] = ...,
    ) -> "SparseMatrix": ...
    def get_data(self) -> list[int]: ...
    def get_sparsity_offset(self) -> int: ...
    def get_walk_angle_func(self) -> Callable[[int, int, int], list[complex]]: ...


class QuantumBinarySearch:
    """Quantum binary search for sparse matrix access."""

    qram: ps.QRAMCircuit_qutrit
    address_offset_reg: str
    total_length: int
    target_reg: str
    result_reg: str
    max_step: int
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        address_offset_reg: str,
        total_length: int,
        target_reg: str,
        result_reg: str,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "QuantumBinarySearch": ...
    def conditioned_by_all_ones(
        self, conds: str | int | list[str | int]
    ) -> "QuantumBinarySearch": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "QuantumBinarySearch": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class CondRotQW:
    """Conditional rotation for quantum walk."""

    j_reg: str
    k_reg: str
    data_reg: str
    output_reg: str
    mat: SparseMatrix
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        j_reg: str,
        k_reg: str,
        data_reg: str,
        output_reg: str,
        mat: SparseMatrix,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "CondRotQW": ...
    def conditioned_by_all_ones(
        self, conds: str | int | list[str | int]
    ) -> "CondRotQW": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "CondRotQW": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class TOperator:
    """T operator for CKS algorithm."""

    qram: ps.QRAMCircuit_qutrit
    data_offset_reg: str
    sparse_offset_reg: str
    j_reg: str
    b1_reg: str
    k_reg: str
    b2_reg: str
    search_result_reg: str
    nnz_col: int
    data_size: int
    mat: SparseMatrix
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        data_offset_reg: str,
        sparse_offset_reg: str,
        j_reg: str,
        b1_reg: str,
        k_reg: str,
        b2_reg: str,
        search_result_reg: str,
        nnz_col: int,
        data_size: int,
        mat: SparseMatrix,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "TOperator": ...
    def conditioned_by_all_ones(
        self, conds: str | int | list[str | int]
    ) -> "TOperator": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "TOperator": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class QuantumWalk:
    """Quantum walk operator for CKS algorithm."""

    qram: ps.QRAMCircuit_qutrit
    j_reg: str
    b1_reg: str
    k_reg: str
    b2_reg: str
    j_comp_reg: str
    k_comp_reg: str
    data_offset_reg: str
    sparse_offset_reg: str
    mat: SparseMatrix
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        j_reg: str,
        b1_reg: str,
        k_reg: str,
        b2_reg: str,
        j_comp_reg: str,
        k_comp_reg: str,
        data_offset_reg: str,
        sparse_offset_reg: str,
        mat: SparseMatrix,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "QuantumWalk": ...
    def conditioned_by_all_ones(
        self, conds: str | int | list[str | int]
    ) -> "QuantumWalk": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "QuantumWalk": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class QuantumWalkNSteps:
    """Multiple quantum walk steps for CKS algorithm."""

    mat: SparseMatrix
    qram: ps.QRAMCircuit_qutrit
    addr_size: int
    data_size: int
    nnz_col: int
    n_row: int
    default_reg_size: int
    data_offset: str
    sparse_offset: str
    j: str
    b1: str
    k: str
    b2: str
    j_comp: str
    k_comp: str
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self, mat: SparseMatrix, qram: Optional[ps.QRAMCircuit_qutrit] = ...
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "QuantumWalkNSteps": ...
    def conditioned_by_all_ones(
        self, conds: str | int | list[str | int]
    ) -> "QuantumWalkNSteps": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "QuantumWalkNSteps": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def init_environment(self, state: ps.SparseState) -> None: ...
    def create_state(self) -> ps.SparseState: ...
    def first_step(self, state: ps.SparseState) -> None: ...
    def step(self, state: ps.SparseState) -> None: ...
    def make_n_step_state(self, n_steps: int) -> ps.SparseState: ...


class LCUContainer:
    """LCU (Linear Combination of Unitaries) container for CKS."""

    kappa: float
    eps: float
    b: int
    j0: int
    chebyshev: ChebyshevPolynomialCoefficient
    walk: QuantumWalkNSteps
    current_state: Optional[ps.SparseState]
    step_state: Optional[ps.SparseState]
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        mat: SparseMatrix,
        kappa: float,
        eps: float,
        qram: Optional[ps.QRAMCircuit_qutrit] = ...,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "LCUContainer": ...
    def conditioned_by_all_ones(
        self, conds: str | int | list[str | int]
    ) -> "LCUContainer": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "LCUContainer": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def get_input_reg(self) -> str: ...
    def initialize(self) -> None: ...
    def external_input(self, init_op: Callable[[ps.SparseState], None]) -> None: ...
    def iterate(self) -> bool: ...


def cks_solve(
    A: np.ndarray,
    b: np.ndarray,
    kappa: Optional[float] = ...,
    eps: float = ...,
    data_size: int = ...,
) -> np.ndarray:
    """Solve Ax = b using CKS quantum linear solver."""
    ...


def create_cks_demo() -> str:
    """Generate a demo script for CKS solver."""
    ...
