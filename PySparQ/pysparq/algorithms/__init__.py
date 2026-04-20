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

__all__ = []  # Intentionally empty - algorithms are examples, not exports

__version__ = "0.1.0"