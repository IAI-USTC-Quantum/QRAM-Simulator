"""
PySparQ Algorithm Examples

This package contains educational implementations of quantum algorithms
using PySparQ's Register Level Programming paradigm. These implementations
are provided as examples and tutorials, not as core library exports.

Available Algorithms:
    - grover: Grover's quantum search algorithm
    - shor: Shor's integer factorization algorithm
    - cks_solver: CKS (Childs-Kothari-Somma) linear system solver
    - qda_solver: QDA (Quantum Discrete Adiabatic) linear system solver
    - state_preparation: QRAM-based state preparation
    - block_encoding: Block encoding (tridiagonal and via QRAM)

Utilities:
    - qram_utils: Classical helpers for QRAM data construction

Note:
    These algorithms are not exported from the main pysparq module.
    Import them directly: ``from pysparq.algorithms import grover``

See Also:
    - :doc:`/guide/algorithms/index` for step-by-step tutorials
    - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

# Block encoding is a core component, export it
from .block_encoding import (
    BlockEncodingTridiagonal,
    BlockEncodingViaQRAM,
)

# StatePreparation from separate module
from .state_preparation import StatePreparation, make_tree_and_qram

# CKS v2
from .cks_solver import cks_solve_v2

import numpy as np


def BlockEncoding(A: np.ndarray, **kwargs):
    """Factory function to create appropriate block encoding.

    Auto-detects matrix structure and returns suitable implementation:
    - BlockEncodingTridiagonal for tridiagonal matrices
    - BlockEncodingViaQRAM for general sparse matrices
    """
    if _is_tridiagonal(A):
        alpha, beta = _extract_tridiagonal_params(A)
        return BlockEncodingTridiagonal(
            kwargs.get("main_reg", "main"),
            kwargs.get("anc_UA", "anc_UA"),
            alpha,
            beta,
        )
    else:
        return BlockEncodingViaQRAM(
            kwargs.get("main_reg", "main"),
            kwargs.get("anc_UA", "anc_UA"),
            A,
            kwargs.get("data_size", 32),
        )


def _is_tridiagonal(A: np.ndarray, tol: float = 1e-10) -> bool:
    """Check if matrix is tridiagonal."""
    A = np.asarray(A, dtype=float)
    n = A.shape[0]
    for i in range(n):
        for j in range(n):
            if abs(i - j) > 1 and abs(A[i, j]) > tol:
                return False
    return True


def _extract_tridiagonal_params(A: np.ndarray) -> tuple:
    """Extract alpha (diagonal) and beta (off-diagonal) from tridiagonal matrix."""
    A = np.asarray(A, dtype=float)
    n = A.shape[0]
    alpha = A[0, 0]

    off_diag_sum = 0.0
    off_diag_count = 0
    for i in range(n - 1):
        off_diag_sum += abs(A[i, i + 1]) + abs(A[i + 1, i])
        off_diag_count += 2

    beta = off_diag_sum / off_diag_count if off_diag_count > 0 else 0.0
    return alpha, beta


__all__ = [
    "BlockEncoding",
    "BlockEncodingTridiagonal",
    "BlockEncodingViaQRAM",
    "StatePreparation",
    "cks_solve_v2",
    "make_tree_and_qram",
]

__version__ = "0.1.0"
