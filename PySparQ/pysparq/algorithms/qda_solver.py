"""
QDA (Quantum Discrete Adiabatic) Linear System Solver Implementation

This module implements the optimal scaling quantum linear system solver
using discrete adiabatic evolution. It achieves O(κ log(κ/ε)) scaling
for condition number κ and precision ε.

Mathematical Background:
    The QDA algorithm solves Ax = b by:
    1. Constructing interpolating Hamiltonian H(s) = (1-f(s))H₀ + f(s)H₁
    2. Applying quantum walk at each discretization point s
    3. Using LCU and filtering to construct the solution
    4. Post-selecting on ancilla to extract |x⟩

    Key formula (Eq. 69):
        f(s) = κ/(κ-1) * (1 - (1 + s(κ^(p-1) - 1))^(1/(1-p)))

Reference:
    - L. Lin and Y. Tong, "Optimal quantum linear system solver via
      discrete adiabatic theorem", PRX Quantum 3, 040303 (2022)
    - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

from __future__ import annotations

import math
import warnings
from typing import Callable

import numpy as np

import pysparq as ps
from pysparq.operators import ControllableOperatorMixin


def compute_fs(s: float, kappa: float, p: float) -> float:
    """Compute the interpolation parameter f(s).

    Uses the formula from Eq. (69) of the PRX Quantum paper:
        f(s) = κ/(κ-1) * (1 - (1 + s(κ^(p-1) - 1))^(1/(1-p)))

    Args:
        s: Discretization parameter (0 ≤ s ≤ 1)
        kappa: Condition number
        p: Schedule parameter (p < 1 for optimal scaling)

    Returns:
        Interpolation parameter f(s)

    Example:
        >>> fs = compute_fs(0.5, kappa=10, p=0.5)
        >>> print(f"f(0.5) = {fs:.4f}")
    """
    if kappa == 1:
        return s

    kappa_p_minus_1 = kappa ** (p - 1)
    inner = 1 + s * (kappa_p_minus_1 - 1)

    if inner <= 0:
        return 1.0

    exponent = 1 / (1 - p)
    result = kappa / (kappa - 1) * (1 - inner**exponent)

    return max(0.0, min(1.0, result))


def compute_rotation_matrix(fs: float) -> list[complex]:
    """Compute the rotation matrix R_s for block encoding.

    The rotation matrix normalizes the linear interpolation:
        R_s = [[√N*(1-f_s), √N*f_s],
               [√N*f_s, √N*(f_s-1)]]

    where √N = 1/√((1-f_s)² + f_s²)

    Args:
        fs: Interpolation parameter f(s)

    Returns:
        [u00, u01, u10, u11] representing 2x2 matrix
    """
    sqrt_N = 1.0 / math.sqrt((1 - fs) ** 2 + fs**2)
    u00 = sqrt_N * (1 - fs)
    u01 = sqrt_N * fs
    u10 = sqrt_N * fs
    u11 = sqrt_N * (fs - 1)
    return [complex(u00, 0), complex(u01, 0), complex(u10, 0), complex(u11, 0)]


def chebyshev_T(n: int, x: float) -> float:
    """Compute Chebyshev polynomial T_n(x).

    Args:
        n: Polynomial degree
        x: Input value

    Returns:
        T_n(x)
    """
    if n == 0:
        return 1
    if n == 1:
        return x

    T0, T1 = 1, x
    for _ in range(2, n + 1):
        Tn = 2 * x * T1 - T0
        T0, T1 = T1, Tn
    return Tn


def dolph_chebyshev(epsilon: float, l: int, phi: float) -> float:
    """Compute Dolph-Chebyshev window function.

    Args:
        epsilon: Error tolerance
        l: Window length parameter
        phi: Phase angle

    Returns:
        Window function value
    """
    beta = math.cosh(math.acosh(1.0 / epsilon) / l)
    return epsilon * chebyshev_T(l, beta * math.cos(phi))


def compute_fourier_coeffs(epsilon: float, l: int) -> list[float]:
    """Compute Fourier coefficients for Dolph-Chebyshev filter.

    Args:
        epsilon: Error tolerance
        l: Filter length

    Returns:
        List of Fourier coefficients
    """
    coeffs = []
    P = 2 * math.pi
    delta_phi = P / 10000.0

    for j in range(l + 1):
        integral = 0.0
        phi = 0.0
        while phi <= P / 2:
            cos_term = math.cos(2 * math.pi * j * phi / P)
            func_value = dolph_chebyshev(epsilon, l, phi)
            integral += func_value * cos_term
            phi += delta_phi

        coeff = integral * delta_phi / P
        if j % 2 == 0:
            coeffs.append(2 * coeff)

    return coeffs


def calculate_angles(coeffs: list[float]) -> list[float]:
    """Calculate rotation angles from coefficients for state preparation.

    Args:
        coeffs: Positive coefficients

    Returns:
        Rotation angles for each coefficient
    """
    angles = []
    for i, s in enumerate(coeffs):
        if s < 0:
            raise ValueError("Coefficients must be non-negative")
        total = sum(coeffs[i:])
        if total > 0:
            cos_theta_2 = math.sqrt(s / total)
            angles.append(2 * math.acos(cos_theta_2))
        else:
            angles.append(0.0)
    return angles


class BlockEncodingHs(ControllableOperatorMixin):
    """Block encoding of the interpolating Hamiltonian H(s).

    H(s) = (1 - f(s)) * H₀ + f(s) * H₁

    where H₀ is the initial Hamiltonian and H₁ is the problem Hamiltonian
    encoded from matrix A.

    Attributes:
        enc_A: Block encoding of matrix A
        enc_b: State preparation for vector b
        main_reg: Main data register
        anc_UA: Ancilla for block encoding
        anc_1, anc_2, anc_3, anc_4: Additional ancilla registers
        fs: Interpolation parameter
        R_s: Rotation matrix for f(s)
    """

    def __init__(
        self,
        enc_A: "BlockEncoding",
        enc_b: "StatePreparation",
        main_reg: str,
        anc_UA: str,
        anc_1: str,
        anc_2: str,
        anc_3: str,
        anc_4: str,
        fs: float,
    ):
        super().__init__()
        self.enc_A = enc_A
        self.enc_b = enc_b
        self.main_reg = main_reg
        self.anc_UA = anc_UA
        self.anc_1 = anc_1
        self.anc_2 = anc_2
        self.anc_3 = anc_3
        self.anc_4 = anc_4
        self.fs = fs
        self.R_s = compute_rotation_matrix(fs)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply block encoding of H(s)."""
        # Simplified implementation of the circuit from C++ code
        # Full implementation would use SPLIT_BY_CONDITIONS pattern

        # Hadamard on anc_3
        ps.Hadamard_Bool(self.anc_3)(state)

        # State preparation inverse
        self.enc_b.dag(state)

        # X gate on anc_1
        ps.Xgate_Bool(self.anc_1, 0)(state)

        # Reflection on main register
        ps.Reflection_Bool(self.main_reg, True).conditioned_by_all_ones(
            [self.anc_1, self.anc_3, self.anc_4]
        )(state)

        # X gate on anc_1
        ps.Xgate_Bool(self.anc_1, 0)(state)

        # State preparation forward
        self.enc_b(state)

        # Rotation sequence on anc_2
        ps.Xgate_Bool(self.anc_4, 0)(state)
        ps.Rot_Bool(self.anc_2, self.R_s).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)

        # Hadamard on anc_2
        ps.Hadamard_Bool(self.anc_2).conditioned_by_all_ones(self.anc_4)(state)

        # Apply block encoding of A (first pass)
        self.enc_A.conditioned_by_all_ones([self.anc_1, self.anc_2])(state)
        ps.Xgate_Bool(self.anc_1, 0).conditioned_by_all_ones(self.anc_2)(state)
        ps.Reflection_Bool(self.anc_2, True).conditioned_by_all_ones(self.anc_1)(state)

        # Block encoding dagger
        self.enc_A.conditioned_by_all_ones([self.anc_1, self.anc_2]).dag(state)

        # Final operations
        ps.Xgate_Bool(self.anc_4, 0)(state)
        ps.Hadamard_Bool(self.anc_2).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)
        ps.Rot_Bool(self.anc_2, self.R_s).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)

        # Second state preparation sequence
        self.enc_b.dag(state)
        ps.Xgate_Bool(self.anc_1, 0)(state)
        ps.Reflection_Bool(self.main_reg, True).conditioned_by_all_ones(
            [self.anc_1, self.anc_3, self.anc_4]
        )(state)
        ps.Xgate_Bool(self.anc_1, 0)(state)
        self.enc_b(state)

        # Final Hadamard
        ps.Hadamard_Bool(self.anc_3)(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse block encoding — mirrors C++ Block_Encoding_Hs::impl_dag.

        C++ impl_dag (qda_fundamental.h:98-130):
        Hadamard → enc_b.dag → reflection → enc_b → anc_4 sequence →
        enc_A → reflection(anc_2) → X(anc_1) → enc_A.dag →
        anc_4 sequence → enc_b.dag → reflection → enc_b → Hadamard
        """
        ps.Hadamard_Bool(self.anc_3)(state)
        self.enc_b.dag(state)
        ps.Xgate_Bool(self.anc_1, 0)(state)
        ps.Reflection_Bool(self.main_reg, True).conditioned_by_all_ones(
            [self.anc_1, self.anc_3, self.anc_4]
        )(state)
        ps.Xgate_Bool(self.anc_1, 0)(state)
        self.enc_b(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)
        ps.Rot_Bool(self.anc_2, self.R_s).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)
        ps.Hadamard_Bool(self.anc_2).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)
        self.enc_A.conditioned_by_all_ones([self.anc_1, self.anc_2])(state)
        ps.Reflection_Bool(self.anc_2, True).conditioned_by_all_ones(self.anc_1)(state)
        ps.Xgate_Bool(self.anc_1, 0).conditioned_by_all_ones(self.anc_2)(state)
        self.enc_A.conditioned_by_all_ones([self.anc_1, self.anc_2]).dag(state)
        ps.Hadamard_Bool(self.anc_2).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)
        ps.Rot_Bool(self.anc_2, self.R_s).conditioned_by_all_ones(self.anc_4)(state)
        ps.Xgate_Bool(self.anc_4, 0)(state)
        self.enc_b.dag(state)
        ps.Xgate_Bool(self.anc_1, 0)(state)
        ps.Reflection_Bool(self.main_reg, True).conditioned_by_all_ones(
            [self.anc_1, self.anc_3, self.anc_4]
        )(state)
        ps.Xgate_Bool(self.anc_1, 0)(state)
        self.enc_b(state)
        ps.Hadamard_Bool(self.anc_3)(state)


class BlockEncodingHsPD:
    """Positive-definite version of block encoding H(s).

    Simplified version for positive-definite matrices with fewer ancillas.
    """

    def __init__(
        self,
        enc_A: "BlockEncoding",
        enc_b: "StatePreparation",
        main_reg: str,
        anc_UA: str,
        anc_1: str,
        anc_2: str,
        anc_3: str,
        anc_4: str,
        fs: float,
    ):
        self.enc_A = enc_A
        self.enc_b = enc_b
        self.main_reg = main_reg
        self.anc_UA = anc_UA
        self.anc_1 = anc_1
        self.anc_2 = anc_2
        self.anc_3 = anc_3
        self.anc_4 = anc_4
        self.fs = fs
        self.R_s = compute_rotation_matrix(fs)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply positive-definite block encoding."""
        ps.Hadamard_Bool(self.anc_2)(state)
        self.enc_b.dag(state)
        ps.Reflection_Bool(self.main_reg, True).conditioned_by_all_ones(
            [self.anc_2, self.anc_3]
        )(state)
        self.enc_b(state)

        ps.Xgate_Bool(self.anc_3, 0)(state)
        ps.Rot_Bool(self.anc_1, self.R_s).conditioned_by_all_ones(self.anc_3)(state)
        ps.Xgate_Bool(self.anc_3, 0)(state)

        ps.Hadamard_Bool(self.anc_1).conditioned_by_all_ones(self.anc_3)(state)
        self.enc_A.conditioned_by_all_ones([self.anc_1, self.anc_3])(state)
        ps.Xgate_Bool(self.anc_3, 0)(state)
        self.enc_A.conditioned_by_all_ones([self.anc_1, self.anc_3]).dag(state)
        ps.Hadamard_Bool(self.anc_1).conditioned_by_all_ones(self.anc_3)(state)
        ps.Xgate_Bool(self.anc_3, 0)(state)

        ps.Rot_Bool(self.anc_1, self.R_s).conditioned_by_all_ones(self.anc_3)(state)
        ps.Xgate_Bool(self.anc_3, 0)(state)

        self.enc_b.dag(state)
        ps.Reflection_Bool(self.main_reg, True).conditioned_by_all_ones(
            [self.anc_2, self.anc_3]
        )(state)
        self.enc_b(state)
        ps.Hadamard_Bool(self.anc_2)(state)


class WalkS(ControllableOperatorMixin):
    """Quantum walk operator at parameter s.

    Combines block encoding of H(s) with reflection operations to implement
    one step of the discrete adiabatic evolution.

    W_s = R · H_s

    where R is a reflection operator.
    """

    def __init__(
        self,
        enc_A: "BlockEncoding",
        enc_b: "StatePreparation",
        main_reg: str,
        anc_UA: str,
        anc_1: str,
        anc_2: str,
        anc_3: str,
        anc_4: str,
        s: float,
        kappa: float,
        p: float,
        is_positive_definite: bool = False,
    ):
        super().__init__()
        self.main_reg = main_reg
        self.anc_UA = anc_UA
        self.anc_1 = anc_1
        self.anc_2 = anc_2
        self.anc_3 = anc_3
        self.anc_4 = anc_4
        self.s = s
        self.kappa = kappa
        self.p = p
        self.is_positive_definite = is_positive_definite

        # Compute fs from s, kappa, p
        self.fs = compute_fs(s, kappa, p)

        # Phase factor
        self.phase = complex(0, 1)

        # Create block encoding
        if is_positive_definite:
            self.enc_Hs = BlockEncodingHsPD(
                enc_A, enc_b, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4, self.fs
            )
        else:
            self.enc_Hs = BlockEncodingHs(
                enc_A, enc_b, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4, self.fs
            )

    def __call__(self, state: ps.SparseState) -> None:
        """Apply walk operator."""
        # Apply block encoding
        if self._condition_regs:
            self.enc_Hs.conditioned_by_all_ones(self._condition_regs)(state)
        else:
            self.enc_Hs(state)

        # Apply reflection
        if not self.is_positive_definite:
            ps.Reflection_Bool([self.anc_UA, self.anc_2, self.anc_3], False)(state)
        else:
            ps.Reflection_Bool([self.anc_UA, self.anc_1, self.anc_2], False)(state)

        # Apply global phase
        ps.GlobalPhase_Int(self.phase)(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse walk operator."""
        # Inverse global phase
        ps.GlobalPhase_Int(-self.phase)(state)

        # Inverse reflection
        if not self.is_positive_definite:
            ps.Reflection_Bool([self.anc_UA, self.anc_2, self.anc_3], False)(state)
        else:
            ps.Reflection_Bool([self.anc_UA, self.anc_1, self.anc_2], False)(state)

        # Inverse block encoding
        self.enc_Hs.dag(state)


class LCU:
    """Linear Combination of Unitaries for QDA.

    Applies repeated walk operators according to the index register,
    implementing the LCU construction for solution preparation.

    The LCU constructs:
        Σᵢ cᵢ W^(2^i)

    where W is the walk operator and cᵢ are coefficients.
    """

    def __init__(self, walk: WalkS, index_reg: str, log_file: str = ""):
        self.walk = walk
        self.index_reg = index_reg
        self.log_file = log_file
        self.index_size = ps.System.size_of(index_reg)

    def __call__(self, state: ps.SparseState) -> None:
        """Apply LCU combination."""
        for i in range(self.index_size):
            print(f"LCU step {i} / {self.index_size}")

            # Get conditioned walk
            self.walk.clear_conditions()
            walk_cond = self.walk.conditioned_by_bit(self.index_reg, i)

            if self.walk._condition_regs:
                walk_cond = walk_cond.conditioned_by_all_ones(
                    self.walk._condition_regs
                )

            # Apply 2^(i+1) walk iterations
            for _ in range(2 ** (i + 1)):
                walk_cond(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse LCU."""
        for i in range(self.index_size):
            print(f"LCUdag step {i} / {self.index_size}")

            self.walk.clear_conditions()
            walk_cond = self.walk.conditioned_by_bit(self.index_reg, i)

            if self.walk._condition_regs:
                walk_cond = walk_cond.conditioned_by_all_ones(
                    self.walk._condition_regs
                )

            for _ in range(2 ** (i + 1)):
                walk_cond.dag(state)


class Filtering:
    """Dolph-Chebyshev filtering for QDA.

    Applies a filter function to reduce error in the solution,
    using the Dolph-Chebyshev window for optimal spectral properties.
    """

    def __init__(
        self,
        walk: WalkS,
        index_reg: str,
        anc_h: str,
        epsilon: float = 0.01,
        l: int = 5,
    ):
        self.walk = walk
        self.index_reg = index_reg
        self.anc_h = anc_h
        self.epsilon = epsilon
        self.l = l

        # Compute Fourier coefficients
        self.coeffs = compute_fourier_coeffs(epsilon, l)

    def __call__(self, state: ps.SparseState) -> float:
        """Apply filtering and return success probability."""
        # Hadamard on anc_h
        ps.Hadamard_Int_Full(self.anc_h)(state)

        # Apply LCU
        lcu = LCU(self.walk, self.index_reg)
        lcu.conditioned_by_all_ones(self.anc_h)(state)

        # X gate
        ps.Xgate_Bool(self.anc_h, 0)(state)

        # Inverse LCU would go here
        # For now, simplified implementation

        # Hadamard on anc_h
        ps.Hadamard_Int_Full(self.anc_h)(state)

        # Return probability (simplified)
        return 1.0


def classical_to_quantum(
    A: np.ndarray, b: np.ndarray
) -> tuple[np.ndarray, np.ndarray, Callable[[np.ndarray], np.ndarray]]:
    """Convert classical linear system to quantum-compatible form.

    Args:
        A: Input matrix
        b: Right-hand side vector

    Returns:
        Tuple of (quantum_A, quantum_b, recovery_function)
    """
    hermitian_done = False

    # Hermitian extension if not already Hermitian
    if not np.allclose(A, A.T.conj()):
        n = A.shape[0]
        A_ext = np.zeros((2 * n, 2 * n), dtype=complex)
        A_ext[:n, n:] = A
        A_ext[n:, :n] = A.T.conj()
        A = A_ext
        b_ext = np.zeros(2 * n, dtype=complex)
        b_ext[:n] = b
        b = b_ext
        hermitian_done = True

    # Pad to power of 2
    n = A.shape[0]
    n_padded = 1 << (n - 1).bit_length()

    if n_padded != n:
        A_padded = np.eye(n_padded, dtype=complex)
        A_padded[:n, :n] = A
        b_padded = np.zeros(n_padded, dtype=complex)
        b_padded[:n] = b
    else:
        A_padded = A
        b_padded = b

    # Normalize
    b_padded = b_padded / (np.linalg.norm(b_padded) + 1e-10)
    A_padded = A_padded / (np.linalg.norm(A_padded, ord=2) + 1e-10)

    def recover(x_q: np.ndarray) -> np.ndarray:
        """Recover classical solution from quantum output."""
        x = x_q[:n]
        if hermitian_done:
            # Extract from extended solution
            orig_n = n // 2
            x = x[orig_n : orig_n + orig_n]
        return x.real

    return A_padded, b_padded, recover


# ==============================================================================
# Block Encoding - Import from modules
# ==============================================================================

from .block_encoding import BlockEncodingTridiagonal, BlockEncodingViaQRAM

# Import StatePreparation from state_preparation module
from .state_preparation import StatePreparation, StatePrepViaQRAM
from .qram_utils import make_vector_tree, scale_and_convert_vector


# ==============================================================================
# Main Solver
# ==============================================================================


def qda_solve_tridiagonal(
    A: np.ndarray,
    b: np.ndarray,
    kappa: float | None = None,
    p: float = 1.3,
    eps: float = 0.01,
    step_rate: float = 1.0,
) -> np.ndarray:
    """Solve Ax = b using QDA algorithm with tridiagonal block encoding.

    Args:
        A: Input matrix (must be tridiagonal)
        b: Right-hand side vector
        kappa: Condition number estimate (computed if None)
        p: Schedule parameter (p < 1 for optimal scaling)
        eps: Desired precision
        step_rate: Multiplier for step count

    Returns:
        Solution vector x

    Note:
        This uses BlockEncodingTridiagonal for efficient encoding of
        tridiagonal matrices.
    """
    from .block_encoding import _is_tridiagonal, _extract_tridiagonal_params

    A_arr = np.asarray(A, dtype=np.float64)
    b_arr = np.asarray(b, dtype=np.float64)

    if not _is_tridiagonal(A_arr):
        raise ValueError(
            "qda_solve_tridiagonal requires a tridiagonal matrix. "
            "Use qda_solve_via_qram for general matrices."
        )

    alpha, beta = _extract_tridiagonal_params(A_arr)

    ps.System.clear()

    A_q, b_q, recover = classical_to_quantum(A_arr, b_arr)
    if kappa is None:
        try:
            kappa = float(np.linalg.cond(A_q))
        except np.linalg.LinAlgError:
            kappa = 10.0

    n = A_q.shape[0]
    n_bits = int(math.log2(n))
    main_reg = "main"
    anc_UA = "anc_UA"
    anc_1, anc_2, anc_3, anc_4 = "anc_1", "anc_2", "anc_3", "anc_4"

    state = ps.SparseState()
    ps.AddRegister(main_reg, ps.UnsignedInteger, n_bits)(state)
    ps.AddRegister(anc_UA, ps.UnsignedInteger, 2)(state)
    ps.AddRegister(anc_1, ps.Boolean, 1)(state)
    ps.AddRegister(anc_2, ps.Boolean, 1)(state)
    ps.AddRegister(anc_3, ps.Boolean, 1)(state)
    ps.AddRegister(anc_4, ps.Boolean, 1)(state)

    enc_A = BlockEncodingTridiagonal(main_reg=main_reg, anc_UA=anc_UA, alpha=alpha, beta=beta)
    prep_data_size = 32
    prep_rational_size = min(50, prep_data_size * 2)
    b_encoded = scale_and_convert_vector(
        b_q.real, prep_data_size - 2, prep_data_size, from_matrix=False
    )
    qram_b = ps.QRAMCircuit_qutrit(
        n_bits + 1, prep_data_size, make_vector_tree(b_encoded, prep_data_size)
    )
    enc_b = StatePrepViaQRAM(qram_b, main_reg, prep_data_size, prep_rational_size)
    steps = max(1, int(step_rate * max(1.0, kappa) * max(1.0, math.log(max(kappa / eps, math.e)))))

    for i in range(steps):
        s = i / steps
        WalkS(
            enc_A, enc_b, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4,
            s, kappa, p,
        )(state)
        ps.ClearZero()(state)

    raise RuntimeError(
        "qda_solve_tridiagonal: quantum walk simulation ran but measurement-based "
        "solution extraction is not yet implemented"
    )


def qda_solve_via_qram(
    A: np.ndarray,
    b: np.ndarray,
    kappa: float | None = None,
    p: float = 1.3,
    eps: float = 0.01,
    step_rate: float = 1.0,
) -> np.ndarray:
    """Solve Ax = b using QDA algorithm.

    The QDA algorithm provides optimal κ scaling for solving linear
    systems, achieving O(κ log(κ/ε)) complexity.

    Args:
        A: Input matrix (converted to Hermitian if needed)
        b: Right-hand side vector
        kappa: Condition number estimate (computed if None)
        p: Schedule parameter (p < 1 for optimal scaling)
        eps: Desired precision
        step_rate: Multiplier for step count

    Returns:
        Solution vector x

    Example:
        >>> A = np.array([[2, 1], [1, 2]], dtype=float)
        >>> b = np.array([1, 1], dtype=float)
        >>> x = qda_solve(A, b, kappa=2.0)
        >>> print(f"Solution: {x}")

    Note:
        This implementation uses classical post-processing for
        the solution extraction. A full quantum implementation
        would use measurement and post-selection.
    """
    A_arr = np.asarray(A, dtype=np.float64)
    b_arr = np.asarray(b, dtype=np.float64)

    ps.System.clear()

    A_q, b_q, recover = classical_to_quantum(A_arr, b_arr)
    if kappa is None:
        try:
            kappa = float(np.linalg.cond(A_q))
        except np.linalg.LinAlgError:
            kappa = 10.0

    n = A_q.shape[0]
    n_bits = int(math.log2(n))
    main_reg = "main"
    anc_UA = "anc_UA"
    anc_1, anc_2, anc_3, anc_4 = "anc_1", "anc_2", "anc_3", "anc_4"

    state = ps.SparseState()
    ps.AddRegister(main_reg, ps.UnsignedInteger, n_bits)(state)
    ps.AddRegister(anc_UA, ps.UnsignedInteger, 2)(state)
    ps.AddRegister(anc_1, ps.Boolean, 1)(state)
    ps.AddRegister(anc_2, ps.Boolean, 1)(state)
    ps.AddRegister(anc_3, ps.Boolean, 1)(state)
    ps.AddRegister(anc_4, ps.Boolean, 1)(state)

    enc_A = BlockEncodingViaQRAM(main_reg, anc_UA, A_q.real, 32)
    prep_data_size = 32
    prep_rational_size = min(50, prep_data_size * 2)
    b_encoded = scale_and_convert_vector(
        b_q.real, prep_data_size - 2, prep_data_size, from_matrix=False
    )
    qram_b = ps.QRAMCircuit_qutrit(
        n_bits + 1, prep_data_size, make_vector_tree(b_encoded, prep_data_size)
    )
    enc_b = StatePrepViaQRAM(qram_b, main_reg, prep_data_size, prep_rational_size)
    steps = max(1, int(step_rate * max(1.0, kappa) * max(1.0, math.log(max(kappa / eps, math.e)))))

    for i in range(steps):
        s = i / steps
        WalkS(
            enc_A, enc_b, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4,
            s, kappa, p,
        )(state)
        ps.ClearZero()(state)

    # Quantum simulation must succeed; this framework is for testing quantum
    # circuits, not for falling back to classical solvers that mask bugs.
    raise RuntimeError(
        "qda_solve_via_qram: quantum walk simulation ran but measurement-based "
        "solution extraction is not yet implemented"
    )


def qda_solve(
    A: np.ndarray,
    b: np.ndarray,
    kappa: float | None = None,
    p: float = 1.3,
    eps: float = 0.01,
    step_rate: float = 1.0,
) -> np.ndarray:
    """Solve Ax = b using QDA algorithm (auto-detects matrix type).

    This is a convenience dispatcher that selects the appropriate QDA variant:

    - ``qda_solve_tridiagonal`` for tridiagonal matrices (efficient,
      uses BlockEncodingTridiagonal)
    - ``qda_solve_via_qram`` for general sparse matrices (uses
      BlockEncodingViaQRAM)

    Args:
        A: Input matrix
        b: Right-hand side vector
        kappa: Condition number estimate (computed if None)
        p: Schedule parameter (p < 1 for optimal scaling)
        eps: Desired precision
        step_rate: Multiplier for step count

    Returns:
        Solution vector x

    Example:
        >>> A = np.array([[2, 1], [1, 2]], dtype=float)
        >>> b = np.array([1, 1], dtype=float)
        >>> x = qda_solve(A, b, kappa=2.0)
    """
    from .block_encoding import _is_tridiagonal

    if _is_tridiagonal(A):
        return qda_solve_tridiagonal(A, b, kappa=kappa, p=p, eps=eps, step_rate=step_rate)
    else:
        return qda_solve_via_qram(A, b, kappa=kappa, p=p, eps=eps, step_rate=step_rate)


def create_qda_demo() -> str:
    """Generate a demo script for QDA solver."""
    return '''
import numpy as np
from pysparq.algorithms.qda_solver import (
    qda_solve,
    compute_fs,
    compute_rotation_matrix,
    chebyshev_T,
    dolph_chebyshev,
)

# Example 1: Simple system
A = np.array([[2, 1], [1, 2]], dtype=float)
b = np.array([1, 0], dtype=float)

print("Solving Ax = b with QDA")
print(f"A = \\n{A}")
print(f"b = {b}")

x = qda_solve(A, b, kappa=2.0)
print(f"Solution x = {x}")
print(f"Verification Ax = {A @ x}")

# Example 2: Interpolation parameter
print("\\nInterpolation parameter f(s):")
kappa, p = 10.0, 0.5
for s in [0.0, 0.25, 0.5, 0.75, 1.0]:
    fs = compute_fs(s, kappa, p)
    print(f"  f({s:.2f}) = {fs:.4f}")

# Example 3: Rotation matrices
print("\\nRotation matrices R_s:")
for fs in [0.2, 0.5, 0.8]:
    R = compute_rotation_matrix(fs)
    print(f"  f={fs}: [[{R[0]:.3f}, {R[1]:.3f}]")
    print(f"         [{R[2]:.3f}, {R[3]:.3f}]]")

# Example 4: Dolph-Chebyshev filter
print("\\nDolph-Chebyshev filter coefficients:")
coeffs = compute_fourier_coeffs(epsilon=0.1, l=5)
for i, c in enumerate(coeffs[:5]):
    print(f"  w_{i} = {c:.4f}")
'''
