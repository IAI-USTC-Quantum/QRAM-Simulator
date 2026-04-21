"""QRAM 电路构建的经典辅助函数。"""

from __future__ import annotations

from typing import Union

import numpy as np

PI: float
"""与 C++ ``constexpr double pi`` 匹配的 Pi 常量。"""


def pow2(n: int) -> int:
    """通过左移返回 ``2**n``，匹配 ``basic.h`` 语义。"""
    ...


def make_complement(data: int, data_sz: int) -> int:
    """将有符号整数转换为其二进制补码表示。"""
    ...


def get_complement(data: int, data_sz: int) -> int:
    """反向二进制补码：将无符号值符号扩展为有符号整数。"""
    ...


def column_flatten(row_vec: list[float]) -> list[float]:
    """将行优先方阵转置为列优先表示。"""
    ...


def scale_and_convert_vector(
    input_vec: Union[list[float], np.ndarray],
    exponent: int,
    data_size: int,
    from_matrix: bool = ...,
) -> list[int]:
    """缩放浮点值并转换为二进制补码整数。"""
    ...


def make_vector_tree(dist: list[int], data_size: int) -> list[int]:
    """从叶分布数据构建用于 QRAM 电路的二叉树。"""
    ...


def make_func(value: int, n_digit: int) -> list[complex]:
    """从有理寄存器值计算 2x2 旋转矩阵。"""
    ...


def make_func_inv(value: int, n_digit: int) -> list[complex]:
    """从有理寄存器值计算逆 2x2 旋转矩阵。"""
    ...


def create_qram_utils_demo() -> str:
    """返回展示本模块典型用法的演示脚本字符串。"""
    ...
