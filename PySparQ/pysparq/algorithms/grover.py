"""
Grover's Quantum Search Algorithm Implementation

This module demonstrates Grover's search algorithm using PySparQ's
Register Level Programming paradigm. Grover's algorithm provides a
quadratic speedup for unstructured search problems.

Mathematical Background:
    Given an unsorted database of N items with M marked items,
    Grover's algorithm finds a marked item in O(sqrt(N/M)) queries.

    The algorithm iteratively applies:
    1. Oracle: Mark target states with negative phase
    2. Diffusion: Amplify amplitude of marked states

    After k iterations, the amplitude of marked states is:
    sin((2k+1) * theta) where theta = arcsin(sqrt(M/N))

Reference:
    - L.K. Grover, "A fast quantum mechanical algorithm for database search"
    - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

from __future__ import annotations

import math

import pysparq as ps


class GroverOracle:
    """Oracle for Grover's search that marks target values.

    The oracle performs:
    1. QRAM load: |addr>|0> -> |addr>|data[addr]>
    2. Compare: Check if data matches search value
    3. Phase flip: Apply -1 phase to matching states
    4. Uncompute: Reverse comparison and QRAM load

    Attributes:
        qram: QRAMCircuit_qutrit instance containing the search data
        addr_reg: Name or ID of the address register
        data_reg: Name or ID of the data register
        search_reg: Name or ID of the search value register

    Example:
        >>> memory = [5, 12, 3, 8, 15, 7, 2, 9]
        >>> qram = ps.QRAMCircuit_qutrit(3, 64, memory)
        >>> oracle = GroverOracle(qram, "addr", "data", "search")
        >>> oracle(state)  # Apply oracle to state
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        addr_reg: str | int,
        data_reg: str | int,
        search_reg: str | int,
    ):
        self.qram = qram
        self.addr_reg = addr_reg
        self.data_reg = data_reg
        self.search_reg = search_reg

        self._condition_regs: list[str | int] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "GroverOracle":
        """Set condition registers for conditional execution."""
        if isinstance(cond, list):
            self._condition_regs = cond
        else:
            self._condition_regs = [cond]
        return self

    def conditioned_by_all_ones(self, conds) -> "GroverOracle":
        """Set condition registers for conditional execution (all-ones alias)."""
        self._condition_regs = [conds] if isinstance(conds, (str, int)) else list(conds)
        return self

    def conditioned_by_bit(self, reg: str | int, pos: int) -> "GroverOracle":
        """Add a single-bit condition."""
        self._condition_bits.append((reg, pos))
        return self

    def clear_conditions(self) -> None:
        """Clear all conditions."""
        self._condition_regs = []
        self._condition_bits = []

    def dag(self, state: ps.SparseState) -> None:
        """Apply the inverse oracle (self-inverse reflection)."""
        self(state)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply the oracle operation to the quantum state."""
        # Step 1: Load data from QRAM into data register
        ps.QRAMLoad(self.qram, self.addr_reg, self.data_reg)(state)

        # Step 2: Create comparison flag registers
        compare_less = ps.AddRegister("compare_less", ps.Boolean, 1)(state)
        compare_equal = ps.AddRegister("compare_equal", ps.Boolean, 1)(state)

        # Step 3: Compare loaded data with search target value
        ps.Compare_UInt_UInt(
            self.data_reg, self.search_reg, compare_less, compare_equal
        )(state)

        # Step 4: Apply phase flip on match (marked states)
        phase_flip = ps.ZeroConditionalPhaseFlip([compare_equal])
        if self._condition_regs:
            phase_flip.conditioned_by_nonzeros(self._condition_regs)(state)
        else:
            phase_flip(state)

        # Step 5: Uncompute comparison (reverse comparison)
        ps.Compare_UInt_UInt(
            self.data_reg, self.search_reg, compare_less, compare_equal
        )(state)

        # Step 6: Remove temporary registers
        ps.RemoveRegister(compare_equal)(state)
        ps.RemoveRegister(compare_less)(state)

        # Step 7: Uncompute QRAM load (self-adjoint, same operation)
        ps.QRAMLoad(self.qram, self.addr_reg, self.data_reg)(state)


class DiffusionOperator:
    """HPH (Hadamard-Phase-Hadamard) diffusion operator.

    The diffusion operator performs reflection about the equal
    superposition state:
        D = 2|s><s| - I

    where |s> = (1/sqrt(N)) * sum_x |x> is the uniform superposition.

    Implementation:
        D = H * (2|0><0| - I) * H
          = H * P_0 * H

    where P_0 applies a -1 phase to |0> state.

    Attributes:
        addr_reg: Name or ID of the address register to apply diffusion

    Example:
        >>> diffusion = DiffusionOperator("addr")
        >>> diffusion(state)  # Apply diffusion operator
    """

    def __init__(self, addr_reg: str | int):
        self.addr_reg = addr_reg
        self._condition_regs: list[str | int] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "DiffusionOperator":
        """Set condition registers for conditional execution."""
        if isinstance(cond, list):
            self._condition_regs = cond
        else:
            self._condition_regs = [cond]
        return self

    def conditioned_by_all_ones(self, conds) -> "DiffusionOperator":
        """Set condition registers for conditional execution (all-ones alias)."""
        self._condition_regs = [conds] if isinstance(conds, (str, int)) else list(conds)
        return self

    def conditioned_by_bit(self, reg: str | int, pos: int) -> "DiffusionOperator":
        """Add a single-bit condition."""
        self._condition_bits.append((reg, pos))
        return self

    def clear_conditions(self) -> None:
        """Clear all conditions."""
        self._condition_regs = []
        self._condition_bits = []

    def dag(self, state: ps.SparseState) -> None:
        """Apply the inverse diffusion (self-inverse H-P-PhaseFlip-P-H)."""
        self(state)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply the diffusion operator."""
        # H (create superposition if not already)
        ps.Hadamard_Int_Full(self.addr_reg)(state)

        # P_0 (phase flip on |0>)
        phase_flip = ps.ZeroConditionalPhaseFlip([self.addr_reg])
        if self._condition_regs:
            phase_flip.conditioned_by_nonzeros(self._condition_regs)(state)
        else:
            phase_flip(state)

        # H (return to computational basis)
        ps.Hadamard_Int_Full(self.addr_reg)(state)


class GroverOperator:
    """Combined Grover operator: Oracle followed by Diffusion.

    The full Grover iteration is:
        G = D * O

    where O is the oracle and D is the diffusion operator.

    Attributes:
        qram: QRAMCircuit_qutrit instance
        addr_reg: Address register
        data_reg: Data register (temporary)
        search_reg: Search value register

    Example:
        >>> grover_op = GroverOperator(qram, "addr", "data", "search")
        >>> grover_op(state)  # Apply one Grover iteration
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        addr_reg: str | int,
        data_reg: str | int,
        search_reg: str | int,
    ):
        self.oracle = GroverOracle(qram, addr_reg, data_reg, search_reg)
        self.diffusion = DiffusionOperator(addr_reg)
        self._condition_regs: list[str | int] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    def conditioned_by_nonzeros(
        self, cond: str | int | list[str | int]
    ) -> "GroverOperator":
        """Set condition registers for conditional execution."""
        if isinstance(cond, list):
            self._condition_regs = cond
        else:
            self._condition_regs = [cond]
        return self

    def conditioned_by_all_ones(self, conds) -> "GroverOperator":
        """Set condition registers for conditional execution (all-ones alias)."""
        self._condition_regs = [conds] if isinstance(conds, (str, int)) else list(conds)
        return self

    def conditioned_by_bit(self, reg: str | int, pos: int) -> "GroverOperator":
        """Add a single-bit condition."""
        self._condition_bits.append((reg, pos))
        return self

    def clear_conditions(self) -> None:
        """Clear all conditions."""
        self._condition_regs = []
        self._condition_bits = []

    def dag(self, state: ps.SparseState) -> None:
        """Apply the inverse Grover operator (delegates to sub-operators' dag)."""
        if self._condition_regs:
            self.diffusion.conditioned_by_nonzeros(self._condition_regs)
            self.oracle.conditioned_by_nonzeros(self._condition_regs)
        self.diffusion.dag(state)
        self.oracle.dag(state)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply one complete Grover iteration."""
        # Apply oracle with conditions
        if self._condition_regs:
            self.oracle.conditioned_by_nonzeros(self._condition_regs)(state)
            self.diffusion.conditioned_by_nonzeros(self._condition_regs)(state)
        else:
            self.oracle(state)
            self.diffusion(state)


def grover_search(
    memory: list[int],
    target: int,
    n_iterations: int | None = None,
    data_size: int = 64,
) -> tuple[int, float]:
    """Execute Grover's search to find target in memory.

    Args:
        memory: List of integers to search through
        target: Value to search for
        n_iterations: Number of Grover iterations (auto-computed if None)
        data_size: Bit size for data register (default 64)

    Returns:
        Tuple of (index, probability) where memory[index] matches target

    Raises:
        ValueError: If memory is empty or target not found

    Example:
        >>> memory = [5, 12, 3, 8, 15, 7, 2, 9]
        >>> idx, prob = grover_search(memory, 8)
        >>> print(f"Found {memory[idx]} at index {idx} with prob {prob:.4f}")

    Note:
        The probability distribution peaks at marked states.
        Multiple runs may be needed to find the correct index.
    """
    # Clear previous state
    ps.System.clear()

    # Compute address register size
    n = len(memory)
    n_bits = int(math.log2(n)) + 1 if n > 0 else 1

    # Ensure n is power of 2 for QRAM
    actual_n = 2**n_bits
    if actual_n != n:
        # Pad memory with distinct values not equal to target
        extended_memory = memory + [
            target + i + 1 for i in range(actual_n - n)
        ]
        memory = extended_memory

    # Auto-compute optimal iterations
    # For single target: k = pi/4 * sqrt(N)
    if n_iterations is None:
        n_iterations = max(1, int(math.pi / 4 * math.sqrt(len(memory))))

    # Create QRAM circuit
    qram = ps.QRAMCircuit_qutrit(n_bits, data_size, memory)

    # Create quantum state
    state = ps.SparseState()

    # Add registers
    addr_reg = ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
    data_reg = ps.AddRegister("data", ps.UnsignedInteger, data_size)(state)
    search_reg = ps.AddRegister("search", ps.UnsignedInteger, data_size)(state)

    # Initialize search value register to target
    ps.Init_Unsafe("search", target)(state)

    # Initialize address register to equal superposition
    ps.Hadamard_Int_Full("addr")(state)

    # Create Grover operator
    grover_op = GroverOperator(qram, "addr", "data", "search")

    # Apply Grover iterations
    for _ in range(n_iterations):
        # Add temporary data register for this iteration
        data_id = ps.AddRegister("data_temp", ps.UnsignedInteger, data_size)(state)

        # Apply Grover operator
        grover_op(state)

        # Remove temporary data register
        ps.RemoveRegister("data_temp")(state)

    # Measure: apply partial trace to get address
    measured_results, prob = ps.PartialTrace(["data", "search"])(state)

    return measured_results[0], prob


def grover_count(
    memory: list[int],
    target: int,
    precision_bits: int = 8,
    data_size: int = 64,
) -> tuple[int, float]:
    """Quantum counting variant of Grover's algorithm.

    Uses phase estimation to estimate the number of marked items (M)
    in the memory, which determines the optimal number of Grover iterations.

    Args:
        memory: List of integers to search
        target: Value to search for
        precision_bits: Number of bits in precision register
        data_size: Bit size for data register

    Returns:
        Tuple of (estimated_count, probability)

    Example:
        >>> memory = [5, 5, 5, 8, 8, 7, 2, 9]  # Three 5s
        >>> count, prob = grover_count(memory, 5)
        >>> print(f"Estimated {count} marked items")
    """
    ps.System.clear()

    n = len(memory)
    n_bits = int(math.log2(n)) + 1 if n > 0 else 1

    # Ensure power of 2
    actual_n = 2**n_bits
    if actual_n != n:
        extended_memory = memory + [target + i + 100 for i in range(actual_n - n)]
        memory = extended_memory

    # Create QRAM
    qram = ps.QRAMCircuit_qutrit(n_bits, data_size, memory)

    state = ps.SparseState()

    # Add registers
    count_reg = ps.AddRegister("count", ps.UnsignedInteger, precision_bits)(state)
    addr_reg = ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
    data_reg = ps.AddRegister("data", ps.UnsignedInteger, data_size)(state)
    search_reg = ps.AddRegister("search", ps.UnsignedInteger, data_size)(state)

    # Initialize search value
    ps.Init_Unsafe("search", target)(state)

    # Initialize superposition on both count and address
    ps.Hadamard_Int_Full("count")(state)
    ps.Hadamard_Int_Full("addr")(state)

    # Create Grover operator for phase estimation
    grover_op = GroverOperator(qram, "addr", "data", "search")

    # Controlled Grover operations
    # Apply 2^j Grover iterations controlled by bit j of count register
    for i in range(precision_bits):
        for _ in range(2**i):
            grover_op.conditioned_by_bit("count", i)(state)

    # Apply inverse QFT on count register
    ps.inverseQFT("count")(state)

    # Measure count register
    measured_results, prob = ps.PartialTrace(["addr", "data", "search"])(state)

    return measured_results[0], prob


def create_grover_demo() -> str:
    """Generate a demo script for Grover's algorithm.

    Returns:
        Python code as a string demonstrating Grover's search

    Example:
        >>> print(create_grover_demo())
    """
    return '''
import pysparq as ps
from pysparq.algorithms.grover import grover_search, grover_count

# Example 1: Basic Grover search
memory = [5, 12, 3, 8, 15, 7, 2, 9]
target = 8

print(f"Searching for {target} in {memory}")
idx, prob = grover_search(memory, target)
print(f"Found at index {idx} with probability {prob:.4f}")

# Example 2: Quantum counting
memory_with_duplicates = [5, 5, 5, 8, 8, 7, 2, 9]
target = 5

print(f"\\nCounting occurrences of {target}")
count, prob = grover_count(memory_with_duplicates, target)
print(f"Estimated {count} marked items")
'''
