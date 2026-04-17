"""
Grover 量子搜索算法实现
"""

from typing import List, Optional, Tuple, Union

import pysparq as ps


class GroverOracle:
    """Grover 搜索预言机，用于标记目标值。"""

    qram: ps.QRAMCircuit_qutrit
    addr_reg: Union[str, int]
    data_reg: Union[str, int]
    search_reg: Union[str, int]

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        addr_reg: Union[str, int],
        data_reg: Union[str, int],
        search_reg: Union[str, int],
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: Union[str, int, List[Union[str, int]]]
    ) -> "GroverOracle": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class DiffusionOperator:
    """HPH（Hadamard-相位-Hadamard）扩散算子。"""

    addr_reg: Union[str, int]

    def __init__(self, addr_reg: Union[str, int]) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: Union[str, int, List[Union[str, int]]]
    ) -> "DiffusionOperator": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class GroverOperator:
    """组合 Grover 算子：预言机后接扩散。"""

    oracle: GroverOracle
    diffusion: DiffusionOperator

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        addr_reg: Union[str, int],
        data_reg: Union[str, int],
        search_reg: Union[str, int],
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: Union[str, int, List[Union[str, int]]]
    ) -> "GroverOperator": ...
    def __call__(self, state: ps.SparseState) -> None: ...


def grover_search(
    memory: List[int],
    target: int,
    n_iterations: Optional[int] = ...,
    data_size: int = ...,
) -> Tuple[int, float]:
    """执行 Grover 搜索以在内存中找到目标值。"""
    ...


def grover_count(
    memory: List[int],
    target: int,
    precision_bits: int = ...,
    data_size: int = ...,
) -> Tuple[int, float]:
    """Grover 算法的量子计数变体。"""
    ...


def create_grover_demo() -> str:
    """生成 Grover 算法的演示脚本。"""
    ...
