"""
Grover's Quantum Search Algorithm Implementation
"""

import pysparq as ps


class GroverOracle:
    """Oracle for Grover's search that marks target values."""

    qram: ps.QRAMCircuit_qutrit
    addr_reg: str | int
    data_reg: str | int
    search_reg: str | int
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        addr_reg: str | int,
        data_reg: str | int,
        search_reg: str | int,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "GroverOracle": ...
    def conditioned_by_all_ones(self, conds: str | int | list[str | int]) -> "GroverOracle": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "GroverOracle": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class DiffusionOperator:
    """HPH (Hadamard-Phase-Hadamard) diffusion operator."""

    addr_reg: str | int
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(self, addr_reg: str | int) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "DiffusionOperator": ...
    def conditioned_by_all_ones(self, conds: str | int | list[str | int]) -> "DiffusionOperator": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "DiffusionOperator": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class GroverOperator:
    """Combined Grover operator: Oracle followed by Diffusion."""

    oracle: GroverOracle
    diffusion: DiffusionOperator
    _condition_regs: list[str | int]
    _condition_bits: list[tuple[str | int, int]]

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        addr_reg: str | int,
        data_reg: str | int,
        search_reg: str | int,
    ) -> None: ...
    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "GroverOperator": ...
    def conditioned_by_all_ones(self, conds: str | int | list[str | int]) -> "GroverOperator": ...
    def conditioned_by_bit(self, reg: str | int, pos: int) -> "GroverOperator": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


def grover_search(
    memory: list[int],
    target: int,
    n_iterations: int | None = ...,
    data_size: int = ...,
) -> tuple[int, float]:
    """Execute Grover's search to find target in memory."""
    ...


def grover_count(
    memory: list[int],
    target: int,
    precision_bits: int = ...,
    data_size: int = ...,
) -> tuple[int, float]:
    """Quantum counting variant of Grover's algorithm."""
    ...


def create_grover_demo() -> str:
    """Generate a demo script for Grover's algorithm."""
    ...
