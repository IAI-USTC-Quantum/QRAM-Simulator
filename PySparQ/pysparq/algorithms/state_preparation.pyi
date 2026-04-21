"""基于 QRAM 二叉树分解的量子态制备。"""

from __future__ import annotations

from typing import Union


class StatePrepViaQRAM:
    """基于 QRAM 二叉树分解的态制备量子算子。"""

    qram: object
    work_qubit: str
    addr_size: int
    data_size: int
    rational_size: int

    def __init__(
        self,
        qram: object,
        work_qubit: str,
        data_size: int,
        rational_size: int,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: Union[str, int, list[Union[str, int]]]
    ) -> "StatePrepViaQRAM": ...
    def conditioned_by_all_ones(
        self, cond: Union[str, int, list[Union[str, int]]]
    ) -> "StatePrepViaQRAM": ...
    def conditioned_by_bit(
        self, reg: Union[str, int], bit: int
    ) -> "StatePrepViaQRAM": ...
    def clear_conditions(self) -> "StatePrepViaQRAM": ...
    def __call__(self, state: "SparseState") -> None: ...
    def dag(self, state: "SparseState") -> None: ...


class StatePreparation:
    """管理完整态制备流水线的高级封装。"""

    qubit_number: int
    data_size: int
    data_range: int
    rational_size: int
    dist: list[int]
    tree: list[int]
    qram: Union[object, None]

    def __init__(self, qubit_number: int, data_size: int, data_range: int) -> None: ...
    def random_distribution(self) -> None:
        """生成随机振幅分布。"""
        ...
    def show_distribution(self) -> None:
        """打印分布的原始值和归一化振幅。"""
        ...
    def get_real_dist(self) -> list[float]:
        """返回归一化振幅分布的浮点数列表。"""
        ...
    def make_tree(self) -> None:
        """从当前分布构建二叉树。"""
        ...
    def show_tree(self) -> None:
        """逐层打印二叉树。"""
        ...
    def make_qram(self) -> None:
        """创建适合树数据大小的 QRAM 电路。"""
        ...
    def set_qram(self) -> None:
        """将二叉树数据加载到 QRAM 电路中。"""
        ...
    def get_fidelity(self) -> float:
        """计算制备态与目标态之间的保真度。"""
        ...
    def run(self) -> None:
        """执行完整的态制备流水线。"""
        ...


def create_state_preparation_demo() -> str:
    """返回态制备的演示脚本字符串。"""
    ...
