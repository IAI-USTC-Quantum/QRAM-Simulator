"""
Shor 量子因式分解算法实现
"""

from typing import List, Optional, Tuple

import pysparq as ps


class ShorExecutionFailed(Exception):
    """Shor 算法未能找到因子时抛出的异常。"""
    ...


def general_expmod(a: int, x: int, N: int) -> int:
    """使用平方-乘法高效计算 a^x mod N。"""
    ...


def find_best_fraction(y: int, Q: int, N: int) -> Tuple[int, int]:
    """使用法里序列找到近似 y/Q 的最佳分数 c/r。"""
    ...


def compute_period(meas_result: int, size: int, N: int) -> int:
    """从测量结果计算周期。"""
    ...


def check_period(period: int, a: int, N: int) -> None:
    """检查周期是否对因式分解有效。"""
    ...


def shor_postprocess(meas: int, size: int, a: int, N: int) -> Tuple[int, int]:
    """Shor 算法的经典后处理。"""
    ...


class ModMul:
    """受控模乘运算。"""

    reg: str
    a: int
    x: int
    N: int
    opnum: int

    def __init__(self, reg: str, a: int, x: int, N: int) -> None: ...
    def conditioned_by_all_ones(self, cond: str) -> "ModMul": ...
    def __call__(self, state: ps.SparseState) -> None: ...


class SemiClassicalShor:
    """Shor 算法的半经典实现。"""

    a: int
    N: int
    n: int
    size: int
    meas_result: int
    period: int
    p: int
    q: int

    def __init__(self, a: int, N: int) -> None: ...
    def run(self) -> Tuple[int, int]: ...


class ExpMod:
    """模幂运算。"""

    input_reg: str
    output_reg: str
    a: int
    N: int
    period: int

    def __init__(
        self, input_reg: str, output_reg: str, a: int, N: int, period: int
    ) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class Shor:
    """全量子 Shor 算法。"""

    work_reg: str
    ancilla_reg: str
    expmod: ExpMod

    def __init__(
        self,
        work_reg: str,
        ancilla_reg: str,
        a: int,
        N: int,
        period: int,
    ) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


def factor(N: int, a: Optional[int] = ...) -> Tuple[int, int]:
    """使用 Shor 算法分解 N。"""
    ...


def factor_full_quantum(N: int, a: Optional[int] = ...) -> Tuple[int, int]:
    """使用全量子 Shor 算法分解 N。"""
    ...


def create_shor_demo() -> str:
    """生成 Shor 算法的演示脚本。"""
    ...
