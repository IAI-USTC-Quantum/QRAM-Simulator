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

# QDA (Quantum Discrete Adiabatic) linear solver
from .qda_solver import qda_solve, qda_solve_tridiagonal, qda_solve_via_qram

import numpy as np


def BlockEncoding(A: np.ndarray, **kwargs):
    """Factory function to create appropriate block encoding.

    Auto-detects matrix structure and returns suitable implementation:
    - BlockEncodingTridiagonal for tridiagonal matrices
    - BlockEncodingViaQRAM for general sparse matrices
    """
    from .block_encoding import _is_tridiagonal, _extract_tridiagonal_params
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


__all__ = [
    "BlockEncoding",
    "BlockEncodingTridiagonal",
    "BlockEncodingViaQRAM",
    "StatePreparation",
    "make_tree_and_qram",
    "qda_solve",
    "qda_solve_tridiagonal",
    "qda_solve_via_qram",
]

__version__ = "0.1.0"
