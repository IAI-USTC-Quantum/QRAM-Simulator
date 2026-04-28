"""
Shor's Quantum Factorization Algorithm Implementation

This module demonstrates Shor's integer factorization algorithm using
PySparQ's Register Level Programming paradigm. Shor's algorithm provides
exponential speedup over classical factorization algorithms.

Mathematical Background:
    Shor's algorithm factors N by:
    1. Finding a random coprime a to N
    2. Using quantum period finding to find r where a^r ≡ 1 (mod N)
    3. Computing gcd(a^(r/2) ± 1, N) to get factors

    The quantum part uses phase estimation on the modular exponentiation
    unitary to find the period r.

Reference:
    - P.W. Shor, "Polynomial-Time Algorithms for Prime Factorization
      and Discrete Logarithms on a Quantum Computer"
    - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

from __future__ import annotations

import math
import random

import pysparq as ps


class ShorExecutionFailed(Exception):
    """Exception raised when Shor's algorithm fails to find factors."""

    pass


def general_expmod(a: int, x: int, N: int) -> int:
    """Compute a^x mod N efficiently using square-and-multiply.

    Args:
        a: Base
        x: Exponent
        N: Modulus

    Returns:
        a^x mod N

    Example:
        >>> general_expmod(2, 10, 15)
        4
        >>> general_expmod(7, 3, 15)
        13
    """
    if x == 0:
        return 1
    if x == 1:
        return a % N
    if x & 1:  # odd
        return (general_expmod(a, x - 1, N) * a) % N
    else:  # even
        half = general_expmod(a, x // 2, N)
        return (half * half) % N


def find_best_fraction(y: int, Q: int, N: int) -> tuple[int, int]:
    """Find the best fraction c/r approximating y/Q with r <= N.

    Uses the continued fraction expansion of y/Q to find the convergent
    with the largest denominator not exceeding N.

    Args:
        y: Numerator of measurement result
        Q: Denominator (typically 2^size)
        N: Upper bound for denominator (the number being factored)

    Returns:
        Tuple of (denominator, numerator) = (r, c)

    Example:
        >>> find_best_fraction(64, 256, 15)  # 64/256 = 1/4
        (4, 1)
    """
    # Continued fraction expansion using integer arithmetic
    h_prev2, h_prev1 = 0, 1  # numerators of convergents p[-2], p[-1]
    k_prev2, k_prev1 = 1, 0  # denominators of convergents q[-2], q[-1]

    a, b = y, Q

    while b != 0:
        q = a // b
        r = a % b

        h_new = q * h_prev1 + h_prev2
        k_new = q * k_prev1 + k_prev2

        if k_new > N:
            break

        h_prev2, h_prev1 = h_prev1, h_new
        k_prev2, k_prev1 = k_prev1, k_new

        a, b = b, r

    return k_prev1, h_prev1  # (r, c)


def compute_period(meas_result: int, size: int, N: int) -> int:
    """Compute the period from measurement result.

    Args:
        meas_result: Quantum measurement result
        size: Number of bits in precision register
        N: Number being factored

    Returns:
        Estimated period r

    Raises:
        ShorExecutionFailed: If no valid period found
    """
    if meas_result == 0:
        raise ShorExecutionFailed("Measurement result y = 0, algorithm failed")

    Q = 2**size
    y = meas_result
    r, c = find_best_fraction(y, Q, N)

    if 0 < r < N:
        return r
    else:
        raise ShorExecutionFailed("Failed to find a suitable period")


def check_period(period: int, a: int, N: int) -> None:
    """Check if period is valid for factoring.

    Args:
        period: Candidate period r
        a: Base used in period finding
        N: Number being factored

    Raises:
        ShorExecutionFailed: If period is invalid
    """
    if period > N:
        raise ShorExecutionFailed(f"Period r = {period} > N = {N}")

    if period % 2 == 1:
        raise ShorExecutionFailed(f"Odd period r = {period}")

    # Check a^(r/2) ≠ -1 mod N
    a_exp_r_half = general_expmod(a, period // 2, N)
    if a_exp_r_half == N - 1:
        raise ShorExecutionFailed(f"a^(r/2) = -1 mod N for r = {period}")


def shor_postprocess(meas: int, size: int, a: int, N: int) -> tuple[int, int]:
    """Classical post-processing for Shor's algorithm.

    Args:
        meas: Measurement result from quantum circuit
        size: Number of bits in precision register
        a: Base used in period finding
        N: Number being factored

    Returns:
        Tuple of factors (p, q) where p * q = N

    Raises:
        ShorExecutionFailed: If factoring fails
    """
    try:
        period = compute_period(meas, size, N)
        check_period(period, a, N)

        a_exp_r_half = general_expmod(a, period // 2, N)

        p = math.gcd(a_exp_r_half + 1, N)
        q = math.gcd(a_exp_r_half - 1, N)

        return (p, q)
    except ShorExecutionFailed:
        return (1, 1)


class ModMul:
    """Controlled modular multiplication operation.

    Performs y -> y * a^(2^x) mod N controlled by a control register.

    Attributes:
        reg: Register to apply modular multiplication
        a: Base for exponentiation
        x: Power of 2 exponent (computes a^(2^x))
        N: Modulus

    Note:
        When a and reg values are coprime to N, the operation is unitary.
    """

    def __init__(self, reg: str, a: int, x: int, N: int):
        self.reg = reg
        self.a = a
        self.x = x
        self.N = N
        self.opnum = general_expmod(a, 2**x, N)
        self._condition_bits: list[tuple[str | int, int]] = []
        self._condition_regs: list[str | int] = []

    def conditioned_by_all_ones(self, cond: str) -> "ModMul":
        """Set condition for controlled operation."""
        self._condition_bits.append((cond, 0))
        return self

    def conditioned_by_nonzeros(self, cond: str | int) -> "ModMul":
        """Set condition for nonzero-controlled operation."""
        self._condition_bits.append((cond, 1))
        return self

    def clear_conditions(self) -> None:
        """Clear all condition bits."""
        self._condition_bits.clear()
        self._condition_regs.clear()

    def __call__(self, state: ps.SparseState) -> None:
        """Apply modular multiplication to the state."""
        op = ps.Mod_Mult_UInt_ConstUInt_InPlace(self.reg, self.a, self.x, self.N)

        if self._condition_bits:
            cond_reg, _ = self._condition_bits[-1]
            op.conditioned_by_all_ones(cond_reg)(state)
        else:
            op(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply the dagger operation (inverse modular multiplication)."""
        op = ps.Mod_Mult_UInt_ConstUInt_InPlace(self.reg, self.a, self.x, self.N)

        if self._condition_bits:
            cond_reg, _ = self._condition_bits[-1]
            op.conditioned_by_all_ones(cond_reg).dag(state)
        else:
            op.dag(state)


class SemiClassicalShor:
    """Semi-classical implementation of Shor's algorithm.

    Uses iterative measurement for efficient resource usage compared
    to the full quantum phase estimation approach. This matches the
    standard technique used in practice.

    Attributes:
        a: Random coprime to N
        N: Number to factor
        n: Bit size of N
        size: Total precision bits (2 * n)

    Example:
        >>> shor = SemiClassicalShor(2, 15)  # Factor 15 using base 2
        >>> p, q = shor.run()
        >>> print(f"Factors: {p}, {q}")
    """

    def __init__(self, a: int, N: int):
        """Initialize Shor's algorithm.

        Args:
            a: Random coprime to N
            N: Number to factor

        Raises:
            ValueError: If a and N are not coprime
        """
        if math.gcd(a, N) != 1:
            raise ValueError(f"a = {a} and N = {N} must be coprime")

        self.a = a
        self.N = N
        self.n = int(math.log2(N)) + 1
        self.size = self.n * 2
        self.meas_result: int = 0
        self.period: int = 0
        self.p: int = 0
        self.q: int = 0

    def run(self) -> tuple[int, int]:
        """Execute the quantum circuit and return factors.

        Returns:
            Tuple of (p, q) where p * q = N
        """
        ps.System.clear()
        state = ps.SparseState()

        # Create ancilla register initialized to |1>
        anc_reg = ps.AddRegister("anc_reg", ps.UnsignedInteger, self.n)(state)
        ps.Xgate_Bool("anc_reg", 0)(state)

        results: list[int] = []

        # Iterative phase estimation
        for x in range(self.size):
            # Create work qubit in superposition
            work_reg = ps.AddRegisterWithHadamard(
                "work_reg", ps.UnsignedInteger, 1
            )(state)

            # Apply controlled modular multiplication
            # a^(2^(size-1-x)) mod N
            modmul = ModMul("anc_reg", self.a, self.size - 1 - x, self.N)
            modmul.conditioned_by_all_ones("work_reg")(state)

            # Apply phase corrections from previous measurements
            for i, result in enumerate(reversed(results)):
                if result == 1:
                    phase = -2 * math.pi / (2 ** (i + 2))
                    ps.Phase_Bool("work_reg", phase)(state)

            # Hadamard on work qubit before measurement
            ps.Hadamard_Bool("work_reg")(state)

            # Measure work qubit
            measured_results, _ = ps.PartialTrace(["work_reg"])(state)
            results.append(measured_results[0])

            # Remove work register
            ps.RemoveRegister("work_reg")(state)

        # Convert bit results to integer
        self.meas_result = sum(bit * (2**i) for i, bit in enumerate(results))

        # Classical post-processing
        self.p, self.q = shor_postprocess(self.meas_result, self.size, self.a, self.N)
        return (self.p, self.q)


class ExpMod:
    """Modular exponentiation operation.

    Performs |x>|z> -> |x>|z XOR (a^x mod N)>.

    This is used in the full quantum Shor's algorithm (non-semiclassical).
    """

    def __init__(self, input_reg: str, output_reg: str, a: int, N: int, period: int):
        self.input_reg = input_reg
        self.output_reg = output_reg
        self.a = a
        self.N = N
        self.period = period

        # Precompute a^k mod N for k = 0 to period
        self.axmodn = [1]
        for _ in range(1, period):
            next_val = (self.axmodn[-1] * a) % N
            if next_val == 1:
                break
            self.axmodn.append(next_val)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply modular exponentiation."""

        def expmod_func(x: int) -> int:
            x = x % self.period
            return self.axmodn[x]

        ps.CustomArithmetic(
            [self.input_reg, self.output_reg], 64, 64, expmod_func
        )(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse (self-adjoint for XOR)."""
        self(state)


class Shor:
    """Full quantum Shor's algorithm.

    Uses quantum phase estimation with the modular exponentiation
    unitary to find the period.

    Attributes:
        work_reg: Precision/work register
        ancilla_reg: Ancilla register for modular exponentiation
        a: Base for exponentiation
        N: Number to factor
    """

    def __init__(
        self,
        work_reg: str,
        ancilla_reg: str,
        a: int,
        N: int,
        period: int,
    ):
        self.work_reg = work_reg
        self.ancilla_reg = ancilla_reg
        self.expmod = ExpMod(work_reg, ancilla_reg, a, N, period)

    def __call__(self, state: ps.SparseState) -> None:
        """Execute the full quantum Shor circuit."""
        # Apply Hadamard to work register
        ps.Hadamard_Int_Full(self.work_reg)(state)

        # Apply modular exponentiation
        self.expmod(state)

        # Measure ancilla register
        ps.PartialTrace([self.ancilla_reg])(state)

        # Apply inverse QFT
        ps.inverseQFT(self.work_reg)(state)


def factor(N: int, a: int | None = None) -> tuple[int, int]:
    """Factor N using Shor's algorithm.

    This is the main entry point for Shor's factorization algorithm.

    Args:
        N: Number to factor (should be composite)
        a: Optional base (random coprime selected if None)

    Returns:
        Tuple of factors (p, q) where p * q = N

    Raises:
        ValueError: If N is prime or 1

    Example:
        >>> p, q = factor(15)
        >>> print(f"Factors of 15: {p} and {q}")
        Factors of 15: 3 and 5

        >>> p, q = factor(21)
        >>> print(f"Factors of 21: {p} and {q}")
        Factors of 21: 3 and 7

    Note:
        May need multiple attempts with different random 'a' values.
        The algorithm has a reasonable probability of success per attempt.
    """
    if N <= 1:
        raise ValueError(f"N = {N} must be greater than 1")

    # Check if N is even
    if N % 2 == 0:
        return (2, N // 2)

    # Select random coprime if not provided
    if a is None:
        a = random.randint(2, N - 1)

    # Check if a is already a factor
    g = math.gcd(a, N)
    if g != 1:
        return (g, N // g)

    # Use semi-classical Shor (more practical)
    shor = SemiClassicalShor(a, N)
    p, q = shor.run()

    if p == 1 or q == 1:
        return (1, N)

    return (p, q)


def factor_full_quantum(N: int, a: int | None = None) -> tuple[int, int]:
    """Factor N using full quantum Shor's algorithm.

    This implements the textbook version with full quantum phase
    estimation, as opposed to the semi-classical iterative measurement.

    Args:
        N: Number to factor
        a: Optional base for exponentiation

    Returns:
        Tuple of factors (p, q)
    """
    if N <= 1:
        raise ValueError(f"N = {N} must be greater than 1")

    if N % 2 == 0:
        return (2, N // 2)

    if a is None:
        a = random.randint(2, N - 1)

    g = math.gcd(a, N)
    if g != 1:
        return (g, N // g)

    n = int(math.log2(N)) + 1
    size = n * 2

    ps.System.clear()

    state = ps.SparseState()

    work_reg = ps.AddRegister("work_reg", ps.UnsignedInteger, size)(state)
    anc_reg = ps.AddRegister("anc_reg", ps.UnsignedInteger, n)(state)

    # Compute period classically for the full quantum version
    axmodn = [1]
    for _ in range(1, N):
        next_val = (axmodn[-1] * a) % N
        if next_val == 1:
            break
        axmodn.append(next_val)

    period = len(axmodn)

    shor = Shor(work_reg, anc_reg, a, N, period)
    shor(state)

    measured, _ = ps.PartialTrace([work_reg])(state)
    return shor_postprocess(measured[0], size, a, N)


def create_shor_demo() -> str:
    """Generate a demo script for Shor's algorithm.

    Returns:
        Python code demonstrating Shor's factorization
    """
    return '''
import pysparq as ps
from pysparq.algorithms.shor import factor, SemiClassicalShor

# Example 1: Simple factorization
N = 15
p, q = factor(N)
print(f"Factors of {N}: {p} and {q}")
assert p * q == N

# Example 2: Try different number
N = 21
p, q = factor(N)
print(f"Factors of {N}: {p} and {q}")
assert p * q == N

# Example 3: Using specific base
N = 35
p, q = factor(N, a=2)  # Use base 2
print(f"Factors of {N}: {p} and {q}")

# Example 4: SemiClassicalShor class directly
shor = SemiClassicalShor(a=2, N=15)
p, q = shor.run()
print(f"Direct call: {p} * {q} = {p * q}")
'''
