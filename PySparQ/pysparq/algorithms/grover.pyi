"""
Grover's Quantum Search Algorithm Implementation
"""

from typing import List, Optional, Tuple, Union

import pysparq as ps


class GroverOracle:
    """Oracle for Grover's search that marks target values."""

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
    """HPH (Hadamard-Phase-Hadamard) diffusion operator."""

    addr_reg: Union[str, int]

    def __init__(self, addr_reg: Union[str, int]) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: Union[str, int, List[Union[str, int]]]
    ) -> "DiffusionOperator": ...
    def clear_conditions(self) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class GroverOperator:
    """Combined Grover operator: Oracle followed by Diffusion."""

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
    """Execute Grover's search to find target in memory."""
    ...


def grover_count(
    memory: List[int],
    target: int,
    precision_bits: int = ...,
    data_size: int = ...,
) -> Tuple[int, float]:
    """Quantum counting variant of Grover's algorithm."""
    ...


def create_grover_demo() -> str:
    """Generate a demo script for Grover's algorithm."""
    ...
