"""矩阵块编码算法（三对角和基于 QRAM）。"""

from __future__ import annotations

from typing import Union

import numpy as np


def get_tridiagonal_matrix(alpha: float, beta: float, dim: int) -> np.ndarray:
    """返回 dim x dim 的三对角矩阵 alpha*I + beta*T。"""
    ...


def get_u_plus(size: int) -> np.ndarray:
    """返回 size x size 的下移（次对角线）矩阵。"""
    ...


def get_u_minus(size: int) -> np.ndarray:
    """返回 size x size 的上移（超对角线）矩阵。"""
    ...


class PlusOneAndOverflow:
    """将寄存器加 1 并跟踪溢出。"""

    main_reg: str
    overflow: str

    def __init__(self, main_reg: str, overflow: str) -> None: ...
    def conditioned_by_nonzeros(
        self, conds: Union[str, list[str]]
    ) -> "PlusOneAndOverflow": ...
    def conditioned_by_all_ones(
        self, conds: Union[str, list[str]]
    ) -> "PlusOneAndOverflow": ...
    def conditioned_by_bit(
        self, reg: Union[str, int], pos: int
    ) -> "PlusOneAndOverflow": ...
    def conditioned_by_value(
        self, reg: Union[str, int], value: int
    ) -> "PlusOneAndOverflow": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: "SparseState") -> None: ...
    def dag(self, state: "SparseState") -> None: ...


class BlockEncodingTridiagonal:
    """三对角矩阵 alpha*I + beta*T 的块编码。"""

    main_reg: str
    anc_UA: str
    alpha: float
    beta: float
    prep_state: list[complex]

    def __init__(
        self, main_reg: str, anc_UA: str, alpha: float, beta: float
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, conds: Union[str, list[str]]
    ) -> "BlockEncodingTridiagonal": ...
    def conditioned_by_all_ones(
        self, conds: Union[str, list[str]]
    ) -> "BlockEncodingTridiagonal": ...
    def conditioned_by_bit(
        self, reg: Union[str, int], pos: int
    ) -> "BlockEncodingTridiagonal": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: "SparseState") -> None: ...
    def dag(self, state: "SparseState") -> None: ...


class UR:
    """基于 QRAM 块编码的右乘算子。"""

    qram: "QRAMCircuit_qutrit"
    column_index: str
    data_size: int
    rational_size: int
    addr_size: int

    def __init__(
        self,
        qram: "QRAMCircuit_qutrit",
        column_index: str,
        data_size: int,
        rational_size: int,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, conds: Union[str, list[str]]
    ) -> "UR": ...
    def conditioned_by_all_ones(
        self, conds: Union[str, list[str]]
    ) -> "UR": ...
    def conditioned_by_bit(
        self, reg: Union[str, int], pos: int
    ) -> "UR": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: "SparseState") -> None: ...
    def dag(self, state: "SparseState") -> None: ...


class UL:
    """基于 QRAM 块编码的左乘算子。"""

    qram: "QRAMCircuit_qutrit"
    row_index: str
    column_index: str
    data_size: int
    rational_size: int
    addr_size: int

    def __init__(
        self,
        qram: "QRAMCircuit_qutrit",
        row_index: str,
        column_index: str,
        data_size: int,
        rational_size: int,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, conds: Union[str, list[str]]
    ) -> "UL": ...
    def conditioned_by_all_ones(
        self, conds: Union[str, list[str]]
    ) -> "UL": ...
    def conditioned_by_bit(
        self, reg: Union[str, int], pos: int
    ) -> "UL": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: "SparseState") -> None: ...
    def dag(self, state: "SparseState") -> None: ...


class BlockEncodingViaQRAM:
    """通过 QRAM 对任意矩阵进行块编码。"""

    qram: "QRAMCircuit_qutrit"
    column_index: str
    row_index: str
    data_size: int
    rational_size: int

    def __init__(
        self,
        qram: "QRAMCircuit_qutrit",
        column_index: str,
        row_index: str,
        data_size: int,
        rational_size: int,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, conds: Union[str, list[str]]
    ) -> "BlockEncodingViaQRAM": ...
    def conditioned_by_all_ones(
        self, conds: Union[str, list[str]]
    ) -> "BlockEncodingViaQRAM": ...
    def conditioned_by_bit(
        self, reg: Union[str, int], pos: int
    ) -> "BlockEncodingViaQRAM": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: "SparseState") -> None: ...
    def dag(self, state: "SparseState") -> None: ...


def create_block_encoding_demo() -> str:
    """返回块编码用法的演示脚本字符串。"""
    ...
