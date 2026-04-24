"""
Shor's Quantum Factorization Algorithm Implementation
"""

import pysparq as ps


class ShorExecutionFailed(Exception):
    """Exception raised when Shor's algorithm fails to find factors."""
    ...


def general_expmod(a: int, x: int, N: int) -> int:
    """Compute a^x mod N efficiently using square-and-multiply."""
    ...


def find_best_fraction(y: int, Q: int, N: int) -> tuple[int, int]:
    """Find the best fraction c/r approximating y/Q using Farey sequence."""
    ...


def compute_period(meas_result: int, size: int, N: int) -> int:
    """Compute the period from measurement result."""
    ...


def check_period(period: int, a: int, N: int) -> None:
    """Check if period is valid for factoring."""
    ...


def shor_postprocess(meas: int, size: int, a: int, N: int) -> tuple[int, int]:
    """Classical post-processing for Shor's algorithm."""
    ...


class ModMul:
    """Controlled modular multiplication operation."""

    reg: str
    a: int
    x: int
    N: int
    opnum: int
    _condition_bits: list[tuple[str | int, int]]
    _condition_regs: list[str | int]

    def __init__(self, reg: str, a: int, x: int, N: int) -> None: ...
    def conditioned_by_all_ones(self, cond: str) -> "ModMul": ...
    def conditioned_by_nonzeros(self, cond: str | int) -> "ModMul": ...
    def clear_conditions(self) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...


class SemiClassicalShor:
    """Semi-classical implementation of Shor's algorithm."""

    a: int
    N: int
    n: int
    size: int
    meas_result: int
    period: int
    p: int
    q: int

    def __init__(self, a: int, N: int) -> None: ...
    def run(self) -> tuple[int, int]: ...


class ExpMod:
    """Modular exponentiation operation."""

    input_reg: str
    output_reg: str
    a: int
    N: int
    period: int
    axmodn: list[int]

    def __init__(
        self, input_reg: str, output_reg: str, a: int, N: int, period: int
    ) -> None: ...
    def __call__(self, state: ps.SparseState) -> None: ...
    def dag(self, state: ps.SparseState) -> None: ...


class Shor:
    """Full quantum Shor's algorithm."""

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


def factor(N: int, a: int | None = ...) -> tuple[int, int]:
    """Factor N using Shor's algorithm."""
    ...


def factor_full_quantum(N: int, a: int | None = ...) -> tuple[int, int]:
    """Factor N using full quantum Shor's algorithm."""
    ...


def create_shor_demo() -> str:
    """Generate a demo script for Shor's algorithm."""
    ...
