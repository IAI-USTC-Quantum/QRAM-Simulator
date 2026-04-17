"""
CKS（Childs-Kothari-Somma）线性系统求解器实现
"""

from dataclasses import dataclass
from typing import Callable, List, Optional

import numpy as np
import pysparq as ps


class ChebyshevPolynomialCoefficient:
    """计算量子游走的切比雪夫多项式系数。"""

    b: int

    def __init__(self, b: int) -> None: ...
    def C(self, Big: int, Small: int) -> float: ...
    def coef(self, j: int) -> float: ...
    def sign(self, j: int) -> bool: ...
    def step(self, j: int) -> int: ...


def get_coef_positive_only(
    mat_data_size: int, v: int, row: int, col: int
) -> List[complex]:
    """获取仅正矩阵元素的旋转矩阵系数。"""
    ...


def get_coef_common(
    mat_data_size: int, v: int, row: int, col: int
) -> List[complex]:
    """获取一般（带符号）矩阵元素的旋转矩阵系数。"""
    ...


def make_walk_angle_func(
    mat_data_size: int, positive_only: bool
) -> Callable[[int, int, int], List[complex]]:
    """为矩阵创建游走角度函数。"""
    ...


@dataclass
class SparseMatrixData:
    """量子模拟的稀疏矩阵数据。"""

    n_row: int
    nnz_col: int
    data: List[int]
    data_size: int
    positive_only: bool
    sparsity_offset: int


class SparseMatrix:
    """CKS 算法的稀疏矩阵表示。"""

    n_row: int
    nnz_col: int
    data: List[int]
    data_size: int
    positive_only: bool
    sparsity_offset: int

    def __init__(
        self,
        n_row: int,
        nnz_col: int,
        data: List[int],
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
    def get_data(self) -> List[int]: ...
    def get_sparsity_offset(self) -> int: ...
    def get_walk_angle_func(self) -> Callable[[int, int, int], List[complex]]: ...


class QuantumBinarySearch:
    """用于稀疏矩阵访问的量子二分搜索。"""

    qram: ps.QRAMCircuit_qutrit
    address_offset_reg: str
    total_length: int
    target_reg: str
    result_reg: str
    max_step: int

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        address_offset_reg: str,
        total_length: int,
        target_reg: str,
        result_reg: str,
    ) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class CondRotQW:
    """量子游走的条件旋转。"""

    j_reg: str
    k_reg: str
    data_reg: str
    output_reg: str
    mat: SparseMatrix

    def __init__(
        self,
        j_reg: str,
        k_reg: str,
        data_reg: str,
        output_reg: str,
        mat: SparseMatrix,
    ) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class TOperator:
    """CKS 算法的 T 算子。"""

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
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class QuantumWalk:
    """CKS 算法的量子游走算子。"""

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
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class QuantumWalkNSteps:
    """CKS 算法的多步量子游走。"""

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

    def __init__(
        self, mat: SparseMatrix, qram: Optional[ps.QRAMCircuit_qutrit] = ...
    ) -> None: ...
    def init_environment(self) -> None: ...
    def create_state(self) -> ps.SparseState: ...
    def first_step(self, state: ps.SparseState) -> None: ...
    def step(self, state: ps.SparseState) -> None: ...
    def make_n_step_state(self, n_steps: int) -> ps.SparseState: ...


class LCUContainer:
    """CKS 的酉组合（LCU）容器。"""

    kappa: float
    eps: float
    b: int
    j0: int
    chebyshev: ChebyshevPolynomialCoefficient
    walk: QuantumWalkNSteps
    current_state: Optional[ps.SparseState]
    step_state: Optional[ps.SparseState]

    def __init__(
        self,
        mat: SparseMatrix,
        kappa: float,
        eps: float,
        qram: Optional[ps.QRAMCircuit_qutrit] = ...,
    ) -> None: ...
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
    """使用 CKS 量子线性求解器求解 Ax = b。"""
    ...


def create_cks_demo() -> str:
    """生成 CKS 求解器的演示脚本。"""
    ...
