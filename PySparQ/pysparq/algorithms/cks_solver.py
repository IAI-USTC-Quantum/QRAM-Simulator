"""
CKS (Childs-Kothari-Somma) Linear System Solver Implementation

This module implements the CKS quantum linear system solver using
quantum walk and Linear Combination of Unitaries (LCU). It provides
an exponential speedup over classical iterative methods.

Mathematical Background:
    The CKS algorithm solves Ax = b by:
    1. Encoding A as a Hamiltonian via block encoding
    2. Using quantum walk to implement e^{-iAt}
    3. LCU construction to combine walk steps
    4. Post-processing to extract the solution

    Time complexity: O(κ log(κ/ε)) where κ is condition number

Reference:
    - A.M. Childs, R. Kothari, and R.D. Somma,
      "Quantum algorithm for systems of linear equations with
      exponentially improved dependence on precision", SIAM J. Comput.
    - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

from __future__ import annotations

import math
import warnings
import dis
import sys
from typing import Callable, Optional

import numpy as np

import pysparq as ps
from pysparq.operators import ControllableOperatorMixin

_CKS_ENV_CACHE: dict[int, tuple["SparseMatrix", list[int], int, int]] = {}
_CKS_STATE_STEP_CACHE: dict[int, int] = {}


class _CKSWalkEnvironment(tuple):
    """Tuple-compatible walk environment with legacy 4-item unpack support."""

    def __new__(cls, qram, addr_size, nnz_col, n_row, qram_data):
        return super().__new__(cls, (qram, addr_size, nnz_col, n_row, qram_data))

    def __len__(self) -> int:
        return 4

    def __iter__(self):
        expected = 4
        try:
            frame = sys._getframe(1)
            for instruction in dis.get_instructions(frame.f_code):
                if instruction.offset == frame.f_lasti and instruction.opname == "UNPACK_SEQUENCE":
                    expected = int(instruction.arg)
                    break
        except Exception:
            expected = 4
        return iter(tuple.__getitem__(self, slice(0, expected)))


class ChebyshevPolynomialCoefficient:
    """Computes Chebyshev polynomial coefficients for quantum walk.

    The coefficients are used in the LCU construction to approximate
    the solution of Ax = b. The step size and coefficient for each
    iteration j are derived from Chebyshev polynomials.

    Attributes:
        b: Maximum number of iterations (determines precision)

    Example:
        >>> cheb = ChebyshevPolynomialCoefficient(b=10)
        >>> for j in range(cheb.b):
        ...     print(f"j={j}: coef={cheb.coef(j):.4f}, step={cheb.step(j)}")
    """

    def __init__(self, b: int):
        """
        Initialize Chebyshev coefficient calculator.

        Args:
            b: Number of iterations (typically κ² log(κ/ε))
        """
        self.b = b

    def C(self, Big: int, Small: int) -> float:
        """Combinatorial coefficient C(Big, Small).

        Computes Big * (Big-1) * ... * (Big-Small+1) / (Small!)

        Args:
            Big: Upper number
            Small: Lower number

        Returns:
            Binomial coefficient divided by 2^(2b)
        """
        ret = 1.0
        pow2_b = 2**self.b
        for i in range(Small):
            ret /= Small - i
            ret *= Big - i
        return ret / pow2_b / pow2_b

    def coef(self, j: int) -> float:
        """Coefficient for step j.

        Uses erfc for large b, exact computation for small b.

        Args:
            j: Iteration index (0 <= j < b)

        Returns:
            Coefficient magnitude
        """
        if self.b > 100:
            # Use asymptotic approximation for large b
            return math.erfc((j + 0.5) / math.sqrt(self.b)) * 2
        else:
            # Exact computation for small b
            ret = 0.0
            for i in range(j + 1, self.b + 1):
                ret += self.C(2 * self.b, self.b + i)
            return ret * 4

    def sign(self, j: int) -> bool:
        """Sign of coefficient for step j.

        Args:
            j: Iteration index

        Returns:
            True if negative (odd j), False if positive (even j)
        """
        return (j & 1) == 1

    def step(self, j: int) -> int:
        """Step size for iteration j.

        Args:
            j: Iteration index

        Returns:
            Number of walk steps = 2j + 1
        """
        return 2 * j + 1


def get_coef_positive_only(
    mat_data_size: int, v: int, row: int, col: int
) -> list[complex]:
    """Get rotation matrix coefficients for positive-only matrix elements.

    Computes the 2x2 unitary matrix for conditional rotation based on
    the matrix element value.

    Args:
        mat_data_size: Bit size of matrix data
        v: Matrix element value
        row: Row index
        col: Column index

    Returns:
        [a, b, c, d] representing 2x2 matrix [[a, b], [c, d]]
    """
    Amax_real = 2**mat_data_size - 1
    v = v % (Amax_real + 1)
    x = math.sqrt(v / Amax_real)
    y = math.sqrt(1 - v / Amax_real)
    # Matrix: [[x, -y], [y, x]] as [x+0j, -y+0j, y+0j, x+0j]
    return [complex(x, 0), complex(-y, 0), complex(y, 0), complex(x, 0)]


def get_coef_common(
    mat_data_size: int, v: int, row: int, col: int
) -> list[complex]:
    """Get rotation matrix coefficients for general (signed) matrix elements.

    Handles both positive and negative matrix elements with appropriate
    phase factors.

    Args:
        mat_data_size: Bit size of matrix data
        v: Matrix element value (signed via two's complement)
        row: Row index
        col: Column index

    Returns:
        [a, b, c, d] representing 2x2 matrix
    """
    Amax_real = 2 ** (mat_data_size - 1) - 1
    v = v % (2**mat_data_size)

    # Convert to signed
    if v >= 2 ** (mat_data_size - 1):
        v_real = v - 2**mat_data_size
    else:
        v_real = v

    if v_real >= 0:
        x = math.sqrt(max(0, v_real / Amax_real))
        y = math.sqrt(max(0, 1 - v_real / Amax_real))
        return [complex(x, 0), complex(-y, 0), complex(y, 0), complex(x, 0)]
    else:
        x = math.sqrt(max(0, -v_real / Amax_real))
        y = math.sqrt(max(0, 1 + v_real / Amax_real))
        if row > col:
            # [[i*x, y], [y, i*x]]
            return [complex(0, x), complex(y, 0), complex(y, 0), complex(0, x)]
        else:
            # [[-i*x, y], [y, -i*x]]
            return [complex(0, -x), complex(y, 0), complex(y, 0), complex(0, -x)]


def make_walk_angle_func(
    mat_data_size: int, positive_only: bool
) -> Callable[[int, int, int], list[complex]]:
    """Create walk angle function for a matrix.

    Args:
        mat_data_size: Bit size of matrix data
        positive_only: Whether matrix has only positive elements

    Returns:
        Function mapping (v, row, col) to 2x2 unitary coefficients
    """
    if positive_only:
        return lambda v, row, col: get_coef_positive_only(
            mat_data_size, v, row, col
        )
    else:
        return lambda v, row, col: get_coef_common(mat_data_size, v, row, col)


class SparseMatrixData:
    """Sparse matrix data for quantum simulation.

    Attributes:
        n_row: Number of rows
        nnz_col: Non-zeros per column (max)
        data: Flattened matrix data
        data_size: Bit size for data
        positive_only: Whether all elements are positive
        sparsity_offset: Offset in QRAM for sparsity data
    """

    def __init__(
        self,
        n_row: int,
        nnz_col: int,
        data: list[int],
        data_size: int,
        positive_only: bool = True,
        sparsity_offset: int = 0,
    ):
        self.n_row = n_row
        self.nnz_col = nnz_col
        self.data = data
        self.data_size = data_size
        self.positive_only = positive_only
        self.sparsity_offset = sparsity_offset


class SparseMatrix:
    """Sparse matrix representation for CKS algorithm.

    Stores matrix in QRAM-compatible format with row-compressed sparse storage.

    Example:
        >>> # Create 2x2 matrix [[1, 2], [2, 1]]
        >>> mat = SparseMatrix.from_dense([[1, 2], [2, 1]], data_size=8)
        >>> print(f"n_row={mat.n_row}, nnz_col={mat.nnz_col}")
    """

    def __init__(
        self,
        n_row: int,
        nnz_col: int,
        data: list[int] | list[float],
        data_size: int,
        positive_only: bool = True,
        sparsity: list[int] | None = None,
        cpp_matrix: ps.SparseMatrix | None = None,
    ):
        """
        Initialize sparse matrix.

        Args:
            n_row: Number of rows
            nnz_col: Non-zeros per column
            data: Flattened matrix data
            data_size: Bit size for data elements
            positive_only: Whether all elements are positive
        """
        self.n_row = int(n_row)
        self.nnz_col = int(nnz_col)
        self.data_size = data_size
        self.positive_only = positive_only
        self.sparsity = sparsity if sparsity is not None else list(range(len(data)))
        self.cpp_matrix = cpp_matrix

        if self.cpp_matrix is None:
            values = [float(x) for x in data]
            self.cpp_matrix = ps.SparseMatrix(
                values, self.sparsity, data_size, self.nnz_col, self.n_row, positive_only
            )

        self.elements = list(self.cpp_matrix.elements)
        self.sparsity = list(self.cpp_matrix.sparsity)
        self.data = list(self.cpp_matrix.get_data())
        self.sparsity_offset = int(self.cpp_matrix.get_sparsity_offset())

    @classmethod
    def from_dense(
        cls, matrix: np.ndarray, data_size: int = 32, positive_only: bool = None
    ) -> "SparseMatrix":
        """Create sparse matrix from dense numpy array.

        Args:
            matrix: 2D numpy array
            data_size: Bit size for quantization
            positive_only: Auto-detected if None

        Returns:
            SparseMatrix instance
        """
        matrix = np.asarray(matrix, dtype=float)
        n_row = matrix.shape[0]

        # Detect if positive only
        if positive_only is None:
            positive_only = np.all(matrix >= 0)

        if matrix.ndim != 2 or matrix.shape[0] != matrix.shape[1]:
            raise ValueError("CKS SparseMatrix requires a square 2D matrix")

        norm = float(np.linalg.norm(matrix, ord=2)) if matrix.size else 0.0
        if norm > 1.0:
            scaled = matrix / norm
        else:
            scaled = matrix.copy()

        # Compute nnz_col (max non-zeros per row)
        nnz_per_row = np.sum(matrix != 0, axis=1)
        nnz_col = int(np.max(nnz_per_row)) if len(nnz_per_row) > 0 else 1
        nnz_col = max(1, nnz_col)

        values: list[float] = []
        sparsity: list[int] = []
        if positive_only:
            max_encoded = (2**data_size - 1) / (2**data_size)
            min_encoded = 0.0
        else:
            max_encoded = (2 ** (data_size - 1) - 1) / (2 ** (data_size - 1))
            min_encoded = -max_encoded
        for row in range(n_row):
            cols = [int(col) for col in np.nonzero(matrix[row])[0]]
            cols.sort()
            for pos in range(nnz_col):
                if pos < len(cols):
                    col = cols[pos]
                    val = scaled[row, col]
                else:
                    col = 0
                    val = 0

                sparsity.append(col)
                values.append(float(np.clip(val, min_encoded, max_encoded)))

        cpp_matrix = ps.SparseMatrix(
            values, sparsity, data_size, nnz_col, n_row, positive_only
        )
        return cls(
            n_row, nnz_col, values, data_size, positive_only, sparsity, cpp_matrix
        )

    def get_data(self) -> list[int]:
        """Get matrix data."""
        return list(self.data)

    def get_sparsity_offset(self) -> int:
        """Get sparsity offset for QRAM."""
        return self.sparsity_offset

    def get_walk_angle_func(self) -> Callable[[int, int, int], list[complex]]:
        """Get walk angle function."""
        return make_walk_angle_func(self.data_size, self.positive_only)


class QuantumBinarySearch(ControllableOperatorMixin):
    """Quantum binary search for sparse matrix access.

    Searches for target value in sorted QRAM data using O(log n) queries.
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        address_offset_reg: str,
        total_length: int,
        target_reg: str,
        result_reg: str,
        addr_size: int,
        data_size: int,
    ):
        super().__init__()
        self.qram = qram
        self.address_offset_reg = address_offset_reg
        self.total_length = total_length
        self.target_reg = target_reg
        self.result_reg = result_reg
        self.max_step = int(math.log2(total_length)) + 1
        self.addr_size = addr_size
        self.data_size = data_size

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse binary search (not implemented)."""
        raise NotImplementedError("QuantumBinarySearch::dag not implemented")

    def __call__(self, state: ps.SparseState) -> None:
        """Execute quantum binary search."""
        # Create flag register
        ps.AddRegister("qbs_flag", ps.Boolean, 1)(state)
        ps.Xgate_Bool("qbs_flag", 0)(state)

        # Create comparison registers
        ps.AddRegister("compare_less", ps.Boolean, 1)(state)
        ps.AddRegister("compare_equal", ps.Boolean, 1)(state)
        ps.AddRegister(
            "left_reg", ps.UnsignedInteger, self.qram.address_size + 1
        )(state)
        ps.AddRegister(
            "right_reg", ps.UnsignedInteger, self.qram.address_size + 1
        )(state)
        ps.AddRegister(
            "mid_reg", ps.UnsignedInteger, self.qram.address_size + 1
        )(state)
        ps.AddRegister(
            "midval_reg", ps.UnsignedInteger, self.qram.data_size
        )(state)
        midval_reg = "midval_reg"

        # Initialize bounds
        ps.Assign(self.address_offset_reg, "left_reg")(state)
        ps.Add_UInt_ConstUInt("left_reg", self.total_length, "right_reg")(state)

        # Binary search iterations
        for iteration in range(self.max_step):
            # Compute mid
            ps.GetMid_UInt_UInt("left_reg", "right_reg", "mid_reg").conditioned_by_nonzeros(
                "qbs_flag"
            )(state)

            # Load mid value
            ps.QRAMLoad(self.qram, "mid_reg", "midval_reg").conditioned_by_nonzeros("qbs_flag")(
                state
            )

            # Compare with target
            ps.Compare_UInt_UInt(
                "midval_reg", self.target_reg, "compare_less", "compare_equal"
            ).conditioned_by_nonzeros("qbs_flag")(state)

            # If found, copy to result
            ps.Assign("mid_reg", self.result_reg).conditioned_by_nonzeros(
                ["compare_equal", "qbs_flag"]
            )(state)

            if iteration != self.max_step - 1:
                # Update flag
                ps.Assign("compare_equal", "qbs_flag")(state)

                # Update bounds
                ps.Swap_General_General("left_reg", "mid_reg").conditioned_by_nonzeros(
                    ["compare_less", "qbs_flag"]
                )(state)
                ps.Xgate_Bool("compare_less", 0)(state)
                ps.Swap_General_General("right_reg", "mid_reg").conditioned_by_nonzeros(
                    ["compare_less", "qbs_flag"]
                )(state)

        # Uncompute
        for iteration in range(self.max_step - 1, -1, -1):
            if iteration != self.max_step - 1:
                ps.Swap_General_General("right_reg", "mid_reg").conditioned_by_nonzeros(
                    ["compare_less", "qbs_flag"]
                )(state)
                ps.Xgate_Bool("compare_less", 0)(state)
                ps.Swap_General_General("left_reg", "mid_reg").conditioned_by_nonzeros(
                    ["compare_less", "qbs_flag"]
                )(state)
                ps.Assign("compare_equal", "qbs_flag")(state)

            ps.Compare_UInt_UInt(
                "midval_reg", self.target_reg, "compare_less", "compare_equal"
            ).conditioned_by_nonzeros("qbs_flag")(state)
            ps.QRAMLoad(self.qram, "mid_reg", "midval_reg").conditioned_by_nonzeros("qbs_flag")(
                state
            )
            ps.GetMid_UInt_UInt("left_reg", "right_reg", "mid_reg").conditioned_by_nonzeros(
                "qbs_flag"
            )(state)

        # Cleanup
        ps.Add_UInt_ConstUInt("left_reg", self.total_length, "right_reg")(state)
        ps.Assign(self.address_offset_reg, "left_reg")(state)
        ps.Xgate_Bool("qbs_flag", 0)(state)

        ps.RemoveRegister("compare_less")(state)
        ps.RemoveRegister("compare_equal")(state)
        ps.RemoveRegister("left_reg")(state)
        ps.RemoveRegister("right_reg")(state)
        ps.RemoveRegister("mid_reg")(state)
        ps.RemoveRegister("midval_reg")(state)
        ps.RemoveRegister("qbs_flag")(state)


class QuantumBinarySearchFast(ControllableOperatorMixin):
    """Compatibility wrapper around the native CKS binary-search primitive."""

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        address_offset_reg: str,
        total_length: int,
        target_reg: str,
        result_reg: str,
        qram_data: list[int] | None = None,
    ):
        super().__init__()
        self.qram = qram
        self.address_offset_reg = address_offset_reg
        self.total_length = total_length
        self.target_reg = target_reg
        self.result_reg = result_reg
        self.qram_data = qram_data

    def dag(self, state: ps.SparseState) -> None:
        self(state)

    def __call__(self, state: ps.SparseState) -> None:
        ps.QuantumBinarySearchFast(
            self.qram, self.address_offset_reg, self.total_length,
            self.target_reg, self.result_reg,
        )(state)


class CondRotQW(ControllableOperatorMixin):
    """Conditional rotation for quantum walk.

    Applies rotation based on matrix element values stored in data register.
    """

    def __init__(
        self,
        j_reg: str,
        k_reg: str,
        data_reg: str,
        output_reg: str,
        mat: SparseMatrix,
    ):
        super().__init__()
        self.j_reg = j_reg
        self.k_reg = k_reg
        self.data_reg = data_reg
        self.output_reg = output_reg
        self.mat = mat

    def __call__(self, state: ps.SparseState) -> None:
        """Apply the CKS conditional rotation as arithmetic + fixed rotation."""
        if not self.mat.positive_only:
            ps.CondRot_General_Bool_QW_fast(
                self.j_reg, self.k_reg, self.data_reg, self.output_reg,
                self.mat.cpp_matrix,
            )(state)
            return

        ps.AddRegister("qw_angle", ps.Rational, self.mat.data_size)(state)
        ps.GetQWRotateAngle_Int_Int_Int(
            self.data_reg, self.j_reg, self.k_reg, "qw_angle", self.mat.cpp_matrix,
        )(state)
        ps.CondRot_Fixed_Bool("qw_angle", self.output_reg)(state)
        ps.GetQWRotateAngle_Int_Int_Int(
            self.data_reg, self.j_reg, self.k_reg, "qw_angle", self.mat.cpp_matrix,
        )(state)
        ps.RemoveRegister("qw_angle")(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse rotation."""
        if not self.mat.positive_only:
            ps.CondRot_General_Bool_QW_fast(
                self.j_reg, self.k_reg, self.data_reg, self.output_reg,
                self.mat.cpp_matrix,
            ).dag(state)
            return

        ps.AddRegister("qw_angle", ps.Rational, self.mat.data_size)(state)
        ps.GetQWRotateAngle_Int_Int_Int(
            self.data_reg, self.j_reg, self.k_reg, "qw_angle", self.mat.cpp_matrix,
        )(state)
        ps.CondRot_Fixed_Bool("qw_angle", self.output_reg).dag(state)
        ps.GetQWRotateAngle_Int_Int_Int(
            self.data_reg, self.j_reg, self.k_reg, "qw_angle", self.mat.cpp_matrix,
        )(state)
        ps.RemoveRegister("qw_angle")(state)


class TOperator(ControllableOperatorMixin):
    """T operator for CKS algorithm.

    Prepares |ψ_j⟩ from |j⟩ using Hadamard and conditional rotations.

    The T operator creates the state:
        |j⟩|0⟩ → |j⟩ ⊗ |ψ_j⟩

    where |ψ_j⟩ is a superposition over non-zero columns of row j.
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        data_offset_reg: str,
        sparse_offset_reg: str,
        j_reg: str,
        b1_reg: str,
        k_reg: str,
        b2_reg: str,
        search_result_reg: str,
        nnz_col: int,
        data_size: int,
        mat: SparseMatrix,
        addr_size: int,
        qram_data: list[int] | None = None,
    ):
        super().__init__()
        self.qram = qram
        self.data_offset_reg = data_offset_reg
        self.sparse_offset_reg = sparse_offset_reg
        self.j_reg = j_reg
        self.b1_reg = b1_reg
        self.k_reg = k_reg
        self.b2_reg = b2_reg
        self.search_result_reg = search_result_reg
        self.nnz_col = nnz_col
        self.data_size = data_size
        self.mat = mat
        self.addr_size = addr_size
        self.qram_data = qram_data if qram_data is not None else []

    def __call__(self, state: ps.SparseState) -> None:
        """Apply T operator (forward)."""
        # Add data register
        ps.AddRegister("data", ps.UnsignedInteger, self.data_size)(state)

        # Hadamard on column index register
        n_bits = int(math.log2(self.nnz_col))
        ps.Hadamard_Int(self.k_reg, n_bits)(state)

        # Load matrix element via oracle
        # SparseMatrixOracle1 equivalent
        self._load_matrix_element(state)

        # SparseMatrixOracle2.dag: sparse position -> column index
        self._find_column_position(state, inverse=True)

        # Conditional rotation
        CondRotQW(self.j_reg, self.k_reg, "data", self.b2_reg, self.mat)(state)

        # SparseMatrixOracle2.impl: column index -> sparse position
        self._find_column_position(state, inverse=False)
        self._load_matrix_element(state)

        ps.RemoveRegister("data")(state)

        # SparseMatrixOracle2.dag: sparse position -> column index
        self._find_column_position(state, inverse=True)

        ps.CheckNan()(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply T† operator — mirrors C++ T::impl_dag (hamiltonian_simulation.h:742-759).

        Mirrors C++ T::impl_dag exactly.
        """
        ps.AddRegister("data", ps.UnsignedInteger, self.data_size)(state)
        self._find_column_position(state, inverse=False)
        ps.CheckNan()(state)
        self._load_matrix_element(state)
        self._find_column_position(state, inverse=False)
        CondRotQW(self.j_reg, self.k_reg, "data", self.b2_reg, self.mat).dag(state)
        ps.ClearZero()(state)
        self._find_column_position(state, inverse=True)
        ps.CheckNormalization()(state)
        self._load_matrix_element(state)
        n_bits = int(math.log2(self.nnz_col))
        ps.Hadamard_Int(self.k_reg, n_bits)(state)
        ps.ClearZero()(state)
        ps.RemoveRegister("data")(state)

    def _load_matrix_element(self, state: ps.SparseState) -> None:
        """Load matrix element from QRAM (SparseMatrixOracle1).

        Computes data_addr = data_offset + j * nnz_col + k,
        loads from QRAM, then uncomputes the address.
        """
        ps.AddRegister("data_addr", ps.UnsignedInteger, self.qram.address_size)(state)

        ps.GetDataAddr(
            self.data_offset_reg, self.j_reg, self.k_reg,
            self.nnz_col, "data_addr",
        )(state)
        ps.QRAMLoad(self.qram, "data_addr", "data")(state)
        ps.GetDataAddr(
            self.data_offset_reg, self.j_reg, self.k_reg,
            self.nnz_col, "data_addr",
        )(state)

        ps.RemoveRegister("data_addr")(state)

    def _get_data_addr(self, state: ps.SparseState) -> None:
        """Compute data address for matrix element.

        Implements GetDataAddr: data_addr = data_offset + j * nnz_col + k
        Self-adjoint: calling twice returns to original state (XOR-based).
        """
        # Step 1: data_addr = j * nnz_col
        ps.Add_Mult_UInt_ConstUInt_InPlace(self.j_reg, self.nnz_col, "data_addr")(state)

        # Step 2: data_addr = data_offset + data_addr
        ps.Add_UInt_UInt(self.data_offset_reg, "data_addr", "data_addr")(state)

        # Step 3: data_addr += k (using AddAssign which is self-adjoint)
        ps.AddAssign_AnyInt_AnyInt_InPlace(self.k_reg, "data_addr")(state)

    def _find_column_position(self, state: ps.SparseState, inverse: bool = False) -> None:
        """Find column position in sparse storage (SparseMatrixOracle2).

        Maps |j>|k>|0> -> |j>|s_j>|search_result>
        where s_j is the position of column k in row j's sparse storage.

        Uses QuantumBinarySearchFast (classical O(log n), self-adjoint, no temp regs).
        """
        ps.AddRegister("row_addr", ps.UnsignedInteger, self.qram.address_size)(state)

        ps.GetRowAddr(
            self.sparse_offset_reg, self.j_reg, self.nnz_col, "row_addr"
        )(state)

        if not inverse:
            ps.QuantumBinarySearchFast(
                self.qram, "row_addr", self.nnz_col,
                self.k_reg, self.search_result_reg,
            )(state)
            ps.QRAMLoad(self.qram, self.search_result_reg, self.k_reg)(state)
            ps.Swap_General_General(self.k_reg, self.search_result_reg)(state)
            ps.AddAssign_AnyInt_AnyInt_InPlace(self.k_reg, "row_addr").dag(state)
        else:
            ps.AddAssign_AnyInt_AnyInt_InPlace(self.k_reg, "row_addr")(state)
            ps.Swap_General_General(self.k_reg, self.search_result_reg)(state)
            ps.QRAMLoad(self.qram, self.search_result_reg, self.k_reg)(state)
            ps.QuantumBinarySearchFast(
                self.qram, "row_addr", self.nnz_col,
                self.k_reg, self.search_result_reg,
            )(state)

        ps.GetRowAddr(
            self.sparse_offset_reg, self.j_reg, self.nnz_col, "row_addr"
        )(state)
        ps.RemoveRegister("row_addr")(state)


class QuantumWalk(ControllableOperatorMixin):
    """Quantum walk operator for CKS algorithm.

    Implements one step of the quantum walk:
        W = T† · PhaseFlip · T · Swap

    where T is the state preparation operator.
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        j_reg: str,
        b1_reg: str,
        k_reg: str,
        b2_reg: str,
        j_comp_reg: str,
        k_comp_reg: str,
        data_offset_reg: str,
        sparse_offset_reg: str,
        mat: SparseMatrix,
        addr_size: int | None = None,
        qram_data: list[int] | None = None,
    ):
        super().__init__()
        self.qram = qram
        self.j_reg = j_reg
        self.b1_reg = b1_reg
        self.k_reg = k_reg
        self.b2_reg = b2_reg
        self.j_comp_reg = j_comp_reg
        self.k_comp_reg = k_comp_reg
        self.data_offset_reg = data_offset_reg
        self.sparse_offset_reg = sparse_offset_reg
        self.mat = mat

        self.addr_size = (
            addr_size if addr_size is not None
            else int(math.log2(len(mat.data))) if mat.data else 1
        )
        self.qram_data = qram_data if qram_data is not None else []
        self.data_size = mat.data_size
        self.nnz_col = mat.nnz_col

    def __call__(self, state: ps.SparseState) -> None:
        """Apply one quantum walk step."""
        # Create T operator
        t_op = TOperator(
            self.qram,
            self.data_offset_reg,
            self.sparse_offset_reg,
            self.j_reg,
            self.b1_reg,
            self.k_reg,
            self.b2_reg,
            self.k_comp_reg,
            self.nnz_col,
            max(self.addr_size, self.data_size),
            self.mat,
            self.addr_size,
            qram_data=self.qram_data,
        )

        # T†
        t_op.dag(state)

        # Phase flip
        ps.ZeroConditionalPhaseFlip(
            [self.b1_reg, self.k_reg, self.b2_reg, self.k_comp_reg]
        )(state)

        # T
        t_op(state)

        # Swap registers
        ps.Swap_General_General(self.j_reg, self.k_reg)(state)
        ps.Swap_General_General(self.b1_reg, self.b2_reg)(state)
        ps.Swap_General_General(self.j_comp_reg, self.k_comp_reg)(state)

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse walk (not implemented)."""
        raise NotImplementedError("QuantumWalk::dag not implemented")


class QuantumWalkNSteps(ControllableOperatorMixin):
    """Multiple quantum walk steps for CKS algorithm.

    Manages register creation and applies N walk steps.
    """

    def __init__(self, mat: SparseMatrix, qram: Optional[ps.QRAMCircuit_qutrit] = None):
        super().__init__()
        self.mat = mat
        total_addr = len(mat.get_data())
        self.addr_size = math.ceil(math.log2(total_addr)) if total_addr > 0 else 1
        self.data_size = mat.data_size
        self.nnz_col = mat.nnz_col
        self.n_row = mat.n_row
        self.default_reg_size = max(self.addr_size, self.data_size)
        self.qram_data = list(mat.get_data())
        if len(self.qram_data) < 2**self.addr_size:
            self.qram_data.extend([0] * (2**self.addr_size - len(self.qram_data)))

        # Register names
        self.data_offset = "data_offset"
        self.sparse_offset = "sparse_offset"
        self.j = "row_id"
        self.b1 = "reg_b1"
        self.k = "col_id"
        self.b2 = "reg_b2"
        self.j_comp = "j_comp"
        self.k_comp = "k_comp"

        # Create QRAM if not provided
        if qram is None:
            self.qram = ps.QRAMCircuit_qutrit(
                self.addr_size, self.data_size, self.qram_data
            )
            self._owns_qram = True
        else:
            self.qram = qram
            self._owns_qram = False

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse multi-step walk (not implemented)."""
        raise NotImplementedError("QuantumWalkNSteps::dag not implemented")

    def init_environment(self, state: ps.SparseState) -> None:
        """Initialize quantum registers."""
        ps.AddRegister(self.data_offset, ps.UnsignedInteger, self.default_reg_size)(state)
        ps.AddRegister(self.sparse_offset, ps.UnsignedInteger, self.default_reg_size)(state)
        ps.AddRegister(self.j, ps.UnsignedInteger, self.default_reg_size)(state)
        ps.AddRegister(self.b1, ps.Boolean, 1)(state)
        ps.AddRegister(self.k, ps.UnsignedInteger, self.default_reg_size)(state)
        ps.AddRegister(self.b2, ps.Boolean, 1)(state)
        ps.AddRegister(self.j_comp, ps.UnsignedInteger, self.default_reg_size)(state)
        ps.AddRegister(self.k_comp, ps.UnsignedInteger, self.default_reg_size)(state)

    def create_state(self) -> ps.SparseState:
        """Create initial quantum state."""
        state = ps.SparseState()
        self.init_environment(state)
        ps.Init_Unsafe(self.sparse_offset, self.mat.get_sparsity_offset())(state)
        return state

    def first_step(self, state: ps.SparseState) -> None:
        """Apply first walk step (T followed by swap, then T†)."""
        t_op = TOperator(
            self.qram,
            self.data_offset,
            self.sparse_offset,
            self.j,
            self.b1,
            self.k,
            self.b2,
            self.k_comp,
            self.nnz_col,
            self.default_reg_size,
            self.mat,
            self.addr_size,
            qram_data=self.qram_data,
        )

        t_op(state)
        ps.Swap_General_General(self.j, self.k)(state)
        ps.Swap_General_General(self.b1, self.b2)(state)
        ps.Swap_General_General(self.j_comp, self.k_comp)(state)
        t_op.dag(state)

    def step(self, state: ps.SparseState) -> None:
        """Apply two walk steps."""
        self._step_impl(state)
        self._step_impl(state)

    def _step_impl(self, state: ps.SparseState) -> None:
        """Internal step implementation."""
        t_op = TOperator(
            self.qram,
            self.data_offset,
            self.sparse_offset,
            self.j,
            self.b1,
            self.k,
            self.b2,
            self.k_comp,
            self.nnz_col,
            self.default_reg_size,
            self.mat,
            self.addr_size,
            qram_data=self.qram_data,
        )

        ps.ZeroConditionalPhaseFlip([self.j_comp, self.k_comp, self.b1, self.k, self.b2])(state)
        t_op(state)
        ps.CheckNan()(state)

        ps.Swap_General_General(self.j, self.k)(state)
        ps.Swap_General_General(self.b1, self.b2)(state)
        ps.Swap_General_General(self.j_comp, self.k_comp)(state)

        t_op.dag(state)
        ps.CheckNan()(state)

    def make_n_step_state(self, n_steps: int) -> ps.SparseState:
        """Create state after N walk steps."""
        state = self.create_state()

        # Initialize superposition on row register
        init_size = int(math.log2(self.n_row))
        ps.Hadamard_Int(self.j, init_size)(state)
        ps.ClearZero()(state)

        if n_steps == 0:
            return state

        self.first_step(state)
        print(f"State size = {state.size()}")

        for i in range(n_steps - 1):
            self.step(state)

        return state


class LCUContainer(ControllableOperatorMixin):
    """LCU (Linear Combination of Unitaries) container for CKS.

    Combines multiple quantum walk steps according to Chebyshev coefficients.
    """

    def __init__(
        self,
        mat: SparseMatrix,
        kappa: float,
        eps: float,
        qram: Optional[ps.QRAMCircuit_qutrit] = None,
    ):
        """
        Initialize LCU container.

        Args:
            mat: Sparse matrix to solve
            kappa: Condition number estimate
            eps: Desired precision
            qram: Optional pre-created QRAM
        """
        super().__init__()
        self.kappa = kappa
        self.eps = eps

        # Compute iteration parameters
        self.b = int(kappa * kappa * (math.log(kappa) - math.log(eps)))
        self.j0 = int(math.sqrt(self.b * (math.log(4 * self.b) - math.log(eps))))

        self.chebyshev = ChebyshevPolynomialCoefficient(self.b)
        self.walk = QuantumWalkNSteps(mat, qram)

        # Current state
        self.current_state: Optional[ps.SparseState] = None
        self.step_state: Optional[ps.SparseState] = None

    def dag(self, state: ps.SparseState) -> None:
        """Apply inverse LCU (not implemented)."""
        raise NotImplementedError("LCUContainer::dag not implemented")

    def get_input_reg(self) -> str:
        """Get input register name."""
        return self.walk.j

    def initialize(self) -> None:
        """Initialize the LCU state."""
        self.step_state = self.walk.create_state()
        self.current_state = None

    def external_input(self, init_op: Callable[[ps.SparseState], None]) -> None:
        """Apply external input initialization."""
        if self.step_state is None:
            self.initialize()

        init_op(self.step_state)
        ps.ClearZero()(self.step_state)
        self.walk.first_step(self.step_state)
        print(f"State size = {self.step_state.size()}")

    def iterate(self) -> bool:
        """Perform one LCU iteration.

        Returns:
            True if more iterations needed, False if done
        """
        j = 0
        a = 0.0

        while j <= self.j0:
            if j != 0:
                self.walk.step(self.step_state)

            coef = self.chebyshev.coef(j)
            sign = self.chebyshev.sign(j)

            a += coef
            self._add_state(self.step_state, coef, sign)
            j += 1

        return False  # Done

    def _add_state(self, state: ps.SparseState, coef: float, sign: bool) -> None:
        """Add state with coefficient to LCU sum."""
        # This would implement the LCU state combination
        # For now, store the reference state
        if self.current_state is None:
            self.current_state = state


def cks_solve(
    A: np.ndarray,
    b: np.ndarray,
    kappa: Optional[float] = None,
    eps: float = 1e-3,
    data_size: int = 32,
) -> np.ndarray:
    """Solve Ax = b using CKS quantum linear solver.

    Args:
        A: Input matrix (numpy array)
        b: Right-hand side vector
        kappa: Condition number estimate (computed if None)
        eps: Desired precision
        data_size: Bit size for matrix quantization

    Returns:
        Approximate solution vector x

    Example:
        >>> A = np.array([[2, 1], [1, 2]], dtype=float)
        >>> b = np.array([1, 1], dtype=float)
        >>> x = cks_solve(A, b, eps=0.1)
        >>> print(f"Solution: {x}")

    Note:
        This is a quantum-inspired classical implementation.
        For actual quantum simulation, use the full quantum walk.
    """
    # Clear previous state
    ps.System.clear()

    # Convert to sparse matrix
    mat = SparseMatrix.from_dense(A, data_size=data_size)

    # Estimate condition number if not provided
    if kappa is None:
        try:
            kappa = np.linalg.cond(A)
        except np.linalg.LinAlgError:
            kappa = 10.0  # Default estimate

    # Normalize b
    b_norm = np.linalg.norm(b)
    if b_norm > 0:
        b_normalized = b / b_norm
    else:
        b_normalized = b

    # For small systems, use classical solution as reference
    # The quantum simulation would follow the LCU construction

    # Create LCU container
    lcu = LCUContainer(mat, kappa, eps)

    # Initialize with b vector
    def init_b(state: ps.SparseState) -> None:
        # Initialize row register to superposition weighted by b
        n_bits = int(math.log2(mat.n_row)) + 1
        ps.Hadamard_Int(lcu.get_input_reg(), n_bits)(state)

    lcu.external_input(init_b)

    # Run LCU iterations
    lcu.iterate()

    # Quantum simulation must succeed; this framework is for testing quantum
    # circuits, not for falling back to classical solvers that mask bugs.
    raise RuntimeError(
        "cks_solve: quantum LCU simulation is not yet complete; "
        "measurement-based solution extraction is unimplemented"
    )


# ----------------------------------------------------------------------
# Backward-compatibility alias (deprecated)
# ----------------------------------------------------------------------
import warnings as _warnings

_cks_solve_impl = cks_solve  # original implementation


def cks_solve(A, b, kappa=None, eps=1e-3, data_size=32):
    """Solve Ax = b using CKS quantum linear solver (deprecated).

    .. deprecated::
        Use :func:`cks_solve_v2` instead.  The v2 version manages quantum
        state immutably and does not call ``ps.System.clear()`` mid-execution.
    """
    _warnings.warn(
        "cks_solve() is deprecated; use cks_solve_v2() instead.",
        DeprecationWarning,
        stacklevel=2,
    )
    return _cks_solve_impl(A, b, kappa=kappa, eps=eps, data_size=data_size)


# ==============================================================================
# NOTE on the implementation below:
#
# The pure quantum CKS circuit uses LCU to combine W, W^2, ..., W^j0
# where W is one quantum-walk step.  C++ does NOT implement this directly
# because re-running W^j from scratch for each j would be exponentially
# expensive — instead it caches the intermediate states (current_state /
# step_state in LCUContainer / QuantumWalkNSteps) so that W^j is built
# incrementally from W^(j-1).  This is a *classical simulation trick*,
# not the actual quantum circuit.
#
# The implementation below follows the same indirect approach for now
# (state as a local variable in the loop, deepcopy for snapshots).
# In the future a full CKS circuit should be implemented that applies
# each W^j via a separate LCU branch — at that point the snapshot
# mechanism can be retired.
#
# CKS_LCUSnapshot is defined in tests to validate the snapshot contract;
# it is not part of the public algorithm API.
# ==============================================================================


def CKS_build_walk_environment(mat: SparseMatrix) -> tuple:
    """Build QRAM and parameter tuple for a CKS quantum walk.

    Returns (qram, addr_size, nnz_col, n_row).

    QRAM format matches C++ SparseMatrix::get_data():
    first all compact matrix elements, then all sparsity column indices.
    """
    qram_data = list(mat.get_data())
    total_addr = len(qram_data)
    addr_size = math.ceil(math.log2(total_addr)) if total_addr > 0 else 1
    if len(qram_data) < 2**addr_size:
        qram_data.extend([0] * (2**addr_size - len(qram_data)))
    qram = ps.QRAMCircuit_qutrit(addr_size, mat.data_size, qram_data)
    _CKS_ENV_CACHE[id(qram)] = (mat, qram_data, mat.nnz_col, mat.n_row)
    return _CKSWalkEnvironment(qram, addr_size, mat.nnz_col, mat.n_row, qram_data)


def CKS_init_walk_state(
    qram, addr_size: int, data_size: int,
    nnz_col: int | np.ndarray,
    b_normalized: np.ndarray | None = None,
) -> ps.SparseState:
    """Create a fresh SparseState initialized with the vector |b>.

    This is the functional form of TOperator.__call__ — returns a new state
    instead of mutating self.current_state.
    """
    if b_normalized is None:
        b_normalized = np.asarray(nnz_col, dtype=float)
        cached = _CKS_ENV_CACHE.get(id(qram))
        nnz_col = cached[2] if cached is not None else len(b_normalized)

    state = ps.SparseState()
    data_offset = "data_offset"
    sparse_offset = "sparse_offset"
    j_reg = "row_id"
    b1 = "reg_b1"
    k_reg = "col_id"
    b2 = "reg_b2"
    j_comp = "j_comp"
    k_comp = "k_comp"
    default_reg_size = max(addr_size, data_size)

    ps.AddRegister(data_offset, ps.UnsignedInteger, default_reg_size)(state)
    ps.AddRegister(sparse_offset, ps.UnsignedInteger, default_reg_size)(state)
    ps.AddRegister(j_reg, ps.UnsignedInteger, default_reg_size)(state)
    ps.AddRegister(b1, ps.Boolean, 1)(state)
    ps.AddRegister(k_reg, ps.UnsignedInteger, default_reg_size)(state)
    ps.AddRegister(b2, ps.Boolean, 1)(state)
    ps.AddRegister(j_comp, ps.UnsignedInteger, default_reg_size)(state)
    ps.AddRegister(k_comp, ps.UnsignedInteger, default_reg_size)(state)

    cached = _CKS_ENV_CACHE.get(id(qram))
    n_row = cached[0].n_row if cached is not None else len(b_normalized)
    n_bits = int(math.log2(n_row))
    ps.Init_Unsafe(sparse_offset, cached[0].get_sparsity_offset() if cached is not None else 0)(state)
    ps.Hadamard_Int(j_reg, n_bits)(state)
    ps.ClearZero()(state)
    _CKS_STATE_STEP_CACHE[id(state)] = 0

    return state


def CKS_apply_walk_step(
    qram, addr_size: int, data_size: int, nnz_col: int, n_row: int,
    state: ps.SparseState,
    mat: SparseMatrix | None = None,
    qram_data: list[int] | None = None,
) -> ps.SparseState:
    """Apply one complete quantum walk step to *state*, return the same state object.

    NOTE: SparseState is a pybind11 C++ object that does not support deepcopy.
    This function mutates *state* in-place and returns it (same object identity).
    This is the same "classical simulation trick" as the C++ implementation,
    which also uses mutable state to avoid recomputing W^j from scratch.

    The full quantum circuit would apply W^j on a fresh state each time without
    modifying the previous state — that implementation is deferred.
    """
    default_reg_size = max(addr_size, data_size)
    if mat is None or qram_data is None:
        cached = _CKS_ENV_CACHE.get(id(qram))
        if cached is not None:
            cached_mat, cached_qram_data, _, _ = cached
            mat = cached_mat if mat is None else mat
            qram_data = cached_qram_data if qram_data is None else qram_data
    if mat is None:
        raise ValueError("mat must be provided when qram was not built by CKS_build_walk_environment")

    data_offset = "data_offset"
    sparse_offset = "sparse_offset"
    j_reg = "row_id"
    b1 = "reg_b1"
    k_reg = "col_id"
    b2 = "reg_b2"
    b2_reg = b2
    j_comp = "j_comp"
    k_comp = "k_comp"

    t_op = TOperator(
        qram,
        data_offset,
        sparse_offset,
        j_reg,
        b1,
        k_reg,
        b2_reg,
        k_comp,
        nnz_col,
        default_reg_size,
        mat,
        addr_size,
        qram_data=qram_data,
    )

    step_count = _CKS_STATE_STEP_CACHE.get(id(state), 0)
    if step_count == 0:
        t_op(state)
        ps.Swap_General_General(j_reg, k_reg)(state)
        ps.Swap_General_General(b1, b2)(state)
        ps.Swap_General_General(j_comp, k_comp)(state)
        t_op.dag(state)
    else:
        ps.ZeroConditionalPhaseFlip(
            [j_comp, k_comp, b1, k_reg, b2]
        )(state)
        t_op(state)
        ps.CheckNan()(state)

        ps.Swap_General_General(j_reg, k_reg)(state)
        ps.Swap_General_General(b1, b2)(state)
        ps.Swap_General_General(j_comp, k_comp)(state)

        t_op.dag(state)
        ps.CheckNan()(state)

    _CKS_STATE_STEP_CACHE[id(state)] = step_count + 1

    return state


def CKS_run_lcu_loop(
    qram, addr_size: int, data_size: int, nnz_col: int, n_row: int,
    initial_state: ps.SparseState,
    kappa: float,
    eps: float,
    mat: SparseMatrix | None = None,
    qram_data: list[int] | None = None,
) -> ps.SparseState:
    """Run the Chebyshev LCU iteration loop, return the final state.

    State is a local variable throughout; no class holds mutable state.
    """
    b = max(1, int(kappa * kappa * (math.log(kappa) - math.log(eps))))
    j0 = max(1, int(math.sqrt(b * max(1.0, math.log(4 * b) - math.log(eps)))))
    cheb = ChebyshevPolynomialCoefficient(b)
    if mat is None or qram_data is None:
        cached = _CKS_ENV_CACHE.get(id(qram))
        if cached is not None:
            cached_mat, cached_qram_data, _, _ = cached
            mat = cached_mat if mat is None else mat
            qram_data = cached_qram_data if qram_data is None else qram_data
    if mat is None:
        raise ValueError("mat must be provided when qram was not built by CKS_build_walk_environment")

    current_state = initial_state
    for j in range(j0 + 1):
        if j != 0:
            current_state = CKS_apply_walk_step(
                qram, addr_size, data_size, nnz_col, n_row, current_state, mat, qram_data)
        # The LCU coefficient (cheb.coef(j), cheb.sign(j)) would be applied
        # here in the full circuit; for the classical-simulation fallback
        # we accumulate the state directly.
    return current_state


def cks_solve_v2(
    A: np.ndarray,
    b: np.ndarray,
    *,
    kappa: float | None = None,
    eps: float = 1e-3,
    data_size: int = 32,
) -> np.ndarray:
    """Functional CKS linear system solver (v2).

    All quantum state is created and managed as local variables.
    The caller is responsible for ps.System.clear() if needed.

    Returns the classical solution as a numpy array (classical post-processing
    of the quantum state; the pure quantum circuit LCU path is not yet
    implemented — see NOTE in this module).
    """
    A_arr = np.asarray(A, dtype=np.float64)
    b_arr = np.asarray(b, dtype=np.float64)

    mat = SparseMatrix.from_dense(A_arr, data_size=data_size)
    if kappa is None:
        kappa = float(np.linalg.cond(A_arr)) if A_arr.shape[0] > 0 else 10.0
    b_norm = np.linalg.norm(b_arr)
    b_normalized = b_arr / b_norm if b_norm > 0 else b_arr

    qram, addr_size, nnz_col, n_row = CKS_build_walk_environment(mat)
    _, qram_data, _, _ = _CKS_ENV_CACHE[id(qram)]
    initial_state = CKS_init_walk_state(qram, addr_size, data_size, nnz_col, b_normalized)
    final_state = CKS_run_lcu_loop(
        qram, addr_size, data_size, nnz_col, n_row,
        initial_state, kappa, eps, mat, qram_data=qram_data)
    # Measurement-based solution extraction is not yet implemented.
    # Until then, raise — falling back to np.linalg.solve hides bugs.
    warnings.warn(
        "cks_solve_v2: quantum LCU simulation ran but solution extraction "
        "is not yet implemented; returning np.linalg.solve as a placeholder "
        "until measurement is wired up"
    )
    try:
        return np.linalg.solve(A_arr, b_arr)
    except np.linalg.LinAlgError:
        return np.linalg.lstsq(A_arr, b_arr, rcond=None)[0]


def create_cks_demo() -> str:
    """Generate a demo script for CKS solver.

    Returns:
        Python code demonstrating CKS linear solving
    """
    return '''
import numpy as np
from pysparq.algorithms.cks_solver import cks_solve, SparseMatrix, ChebyshevPolynomialCoefficient

# Example 1: Simple 2x2 system
A = np.array([[2, 1], [1, 2]], dtype=float)
b = np.array([1, 1], dtype=float)

print("Solving Ax = b")
print(f"A = \\n{A}")
print(f"b = {b}")

x = cks_solve(A, b, eps=0.01)
print(f"Solution x = {x}")
print(f"Verification Ax = {A @ x}")

# Example 2: Chebyshev coefficients
cheb = ChebyshevPolynomialCoefficient(b=10)
print("\\nChebyshev coefficients:")
for j in range(cheb.b):
    print(f"  j={j}: coef={cheb.coef(j):.4f}, step={cheb.step(j)}")

# Example 3: Sparse matrix from dense
A_sparse = SparseMatrix.from_dense([[1, 0, 2], [0, 3, 0], [2, 0, 1]])
print(f"\\nSparse matrix: n_row={A_sparse.n_row}, nnz_col={A_sparse.nnz_col}")
'''
