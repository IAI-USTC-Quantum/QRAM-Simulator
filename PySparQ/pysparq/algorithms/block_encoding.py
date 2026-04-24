"""Block encoding algorithms for quantum simulation.

This module implements block encoding of matrices as quantum unitary
operators, following the Register Level Programming paradigm.  Two
approaches are provided:

- **Tridiagonal block encoding**: encodes a tridiagonal matrix
  :math:`\\alpha I + \\beta T` using a compact state-preparation
  circuit with ancilla qubits.
- **QRAM-based block encoding**: encodes an arbitrary sparse matrix
  stored in a QRAM data structure via the :math:`U_L / U_R` quantum
  walk decomposition.

Utility functions for constructing the corresponding classical matrices
are also included.

Ported from:
  - ``SparQ_Algorithm/include/BlockEncoding/block_encoding_tridiagonal.h``
  - ``SparQ_Algorithm/include/BlockEncoding/block_encoding_via_QRAM.h``

Reference:
  - SparQ Paper: https://arxiv.org/abs/2503.15118
"""

from __future__ import annotations

import math

import numpy as np

import pysparq as ps

from pysparq.algorithms.qram_utils import make_func, make_func_inv, pow2


# ==============================================================================
# Utility matrix functions (pure numpy)
# ==============================================================================


def get_tridiagonal_matrix(alpha: float, beta: float, dim: int) -> np.ndarray:
    """Return the *dim* x *dim* tridiagonal matrix alpha*I + beta*T.

    The off-diagonal matrix *T* has ones on the first sub- and
    super-diagonal.

    Args:
        alpha: Diagonal coefficient.
        beta: Off-diagonal coefficient.
        dim: Matrix dimension.

    Returns:
        Dense numpy array of shape ``(dim, dim)``.
    """
    mat = np.full((dim, dim), 0.0)
    for i in range(dim):
        mat[i, i] = alpha
        if i > 0:
            mat[i - 1, i] = beta
        if i < dim - 1:
            mat[i + 1, i] = beta
    return mat


def get_u_plus(size: int) -> np.ndarray:
    """Return the *size* x *size* shift-down (sub-diagonal) matrix.

    ``U_plus[i, i-1] = 1`` for ``i >= 1``, all other entries zero.

    Args:
        size: Matrix dimension.

    Returns:
        Dense numpy array.
    """
    mat = np.zeros((size, size))
    for i in range(1, size):
        mat[i, i - 1] = 1
    return mat


def get_u_minus(size: int) -> np.ndarray:
    """Return the *size* x *size* shift-up (super-diagonal) matrix.

    ``U_minus[i, i+1] = 1`` for ``i < size-1``, all other entries zero.

    Args:
        size: Matrix dimension.

    Returns:
        Dense numpy array.
    """
    mat = np.zeros((size, size))
    for i in range(size - 1):
        mat[i, i + 1] = 1
    return mat


# ==============================================================================
# BlockEncodingTridiagonal
# ==============================================================================


class BlockEncodingTridiagonal:
    """Block encoding of a tridiagonal matrix alpha*I + beta*T.

    The operator prepares a 4-element ancilla state, applies a
    conditional increment/decrement on the main register, and then
    uncomputes the ancilla.  When ``beta < 0``, additional reflections
    are inserted to handle the sign.

    Ported from ``block_encoding_tridiagonal.h:33-108``.
    """

    def __init__(self, main_reg: str, anc_UA: str, alpha: float, beta: float):
        self.main_reg = main_reg
        self.anc_UA = anc_UA
        self.alpha = alpha
        self.beta = beta

        # Compute state-preparation vector (4-element complex).
        # Ported from the C++ constructor in block_encoding_tridiagonal.cpp:66-93.
        n = pow2(ps.System.size_of(main_reg))
        sum_val = n * abs(alpha) ** 2 + 2 * (n - 1) * abs(beta) ** 2
        norm_f = math.sqrt(sum_val)

        abs_alpha = abs(alpha)
        abs_beta = abs(beta)

        p0 = math.sqrt(abs_alpha) / math.sqrt(norm_f) if norm_f > 0 else 0.0
        p1 = math.sqrt(abs_beta) / math.sqrt(norm_f) if norm_f > 0 else 0.0
        p2 = p1
        p3_sq = max(0.0, 1.0 - (abs_alpha + 2 * abs_beta) / norm_f) if norm_f > 0 else 1.0
        p3 = math.sqrt(p3_sq)

        self.prep_state: list[complex] = [
            complex(p0, 0),
            complex(p1, 0),
            complex(p2, 0),
            complex(p3, 0),
        ]

        self._condition_regs: list[str] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    # -- conditioning helpers --------------------------------------------------

    def conditioned_by_nonzeros(
        self, conds: str | list[str]
    ) -> BlockEncodingTridiagonal:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_all_ones(
        self, conds: str | list[str]
    ) -> BlockEncodingTridiagonal:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_bit(
        self, reg: str | int, pos: int
    ) -> BlockEncodingTridiagonal:
        self._condition_bits = [(reg, pos)]
        return self

    def clear_conditions(self) -> None:
        self._condition_regs = []
        self._condition_bits = []

    # -- internal helper -------------------------------------------------------

    def _do_split(self, state: ps.SparseState) -> ps.SparseState | None:
        if not self._condition_regs and not self._condition_bits:
            return None
        nonzero_ids = [ps.System.get_id(r) for r in self._condition_regs]
        by_bit = [(ps.System.get_id(r), v) for r, v in self._condition_bits]
        return ps.split_systems(state, nonzero_ids, [], by_bit, [])

    def _do_merge(self, state: ps.SparseState, split_info: ps.SparseState | None) -> None:
        if split_info is not None:
            ps.combine_systems(state, split_info)

    # -- forward ---------------------------------------------------------------

    def __call__(self, state: ps.SparseState) -> None:
        split_info = self._do_split(state)

        ps.SplitRegister(self.anc_UA, "overflow", 1)(state)
        ps.SplitRegister(self.anc_UA, "other", 1)(state)
        stateprep = ps.Rot_GeneralStatePrep(self.anc_UA, self.prep_state)
        stateprep(state)

        ps.PlusOneAndOverflow(self.main_reg, "overflow").conditioned_by_value(
            self.anc_UA, 1
        )(state)

        if self.beta < 0:
            ps.Reflection_Bool([self.main_reg, "overflow"], False).conditioned_by_value(
                self.anc_UA, 1
            )(state)
            ps.Reflection_Bool([self.main_reg, "overflow"], False).conditioned_by_value(
                self.anc_UA, 2
            )(state)

        ps.PlusOneAndOverflow(self.main_reg, "overflow").conditioned_by_value(
            self.anc_UA, 2
        ).dag(state)

        ps.Xgate_Bool("other", 0).conditioned_by_all_ones(self.anc_UA)(state)

        stateprep.dag(state)
        ps.CombineRegister(self.anc_UA, "other")(state)
        ps.CombineRegister(self.anc_UA, "overflow")(state)

        self._do_merge(state, split_info)

    # -- dagger ----------------------------------------------------------------

    def dag(self, state: ps.SparseState) -> None:
        split_info = self._do_split(state)

        ps.SplitRegister(self.anc_UA, "overflow", 1)(state)
        ps.SplitRegister(self.anc_UA, "other", 1)(state)
        stateprep = ps.Rot_GeneralStatePrep(self.anc_UA, self.prep_state)
        stateprep(state)

        ps.PlusOneAndOverflow(self.main_reg, "overflow").conditioned_by_value(
            self.anc_UA, 2
        )(state)

        if self.beta < 0:
            ps.Reflection_Bool([self.main_reg, "overflow"], False).conditioned_by_value(
                self.anc_UA, 2
            )(state)
            ps.Reflection_Bool([self.main_reg, "overflow"], False).conditioned_by_value(
                self.anc_UA, 1
            )(state)

        ps.PlusOneAndOverflow(self.main_reg, "overflow").conditioned_by_value(
            self.anc_UA, 1
        ).dag(state)

        ps.Xgate_Bool("other", 0).conditioned_by_all_ones(self.anc_UA)(state)

        stateprep.dag(state)
        ps.CombineRegister(self.anc_UA, "other")(state)
        ps.CombineRegister(self.anc_UA, "overflow")(state)

        self._do_merge(state, split_info)


# ==============================================================================
# UR
# ==============================================================================


class UR:
    """Right-multiplication operator for QRAM-based block encoding.

    Implements the :math:`U_R` unitary that encodes the column norms of
    the target matrix.  The operator iterates over the address bits of
    the column index, performing QRAM loads, division, conditional
    rotation, and uncomputation at each step.

    Ported from ``block_encoding_via_QRAM.h:15-136``.
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        column_index: str,
        data_size: int,
        rational_size: int,
    ):
        self.qram = qram
        self.column_index = column_index
        self.data_size = data_size
        self.rational_size = rational_size
        self.addr_size = ps.System.size_of(column_index)

        self._condition_regs: list[str] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    # -- conditioning helpers --------------------------------------------------

    def conditioned_by_nonzeros(self, conds: str | list[str]) -> UR:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_all_ones(self, conds: str | list[str]) -> UR:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_bit(self, reg: str | int, pos: int) -> UR:
        self._condition_bits = [(reg, pos)]
        return self

    def clear_conditions(self) -> None:
        self._condition_regs = []
        self._condition_bits = []

    # -- forward ---------------------------------------------------------------

    def __call__(self, state: ps.SparseState) -> None:
        ps.AddRegister("addr_child", ps.UnsignedInteger, self.addr_size + 1)(state)
        ps.AddRegister("data_parent", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("data_child", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("temp_bit", ps.Boolean, 1)(state)
        ps.AddRegister("div_result", ps.Rational, self.rational_size)(state)

        n_digit = ps.System.size_of("div_result")
        func = lambda value, _n=n_digit: make_func(value, _n)

        for k in range(self.addr_size):
            ps.SplitRegister(self.column_index, "rotation", 1)(state)
            ps.CombineRegister(self.column_index, "temp_bit")(state)
            ps.ShiftRight(self.column_index, 1)(state)
            ps.Add_ConstUInt(self.column_index, pow2(k) - 1)(state)
            ps.Mult_UInt_ConstUInt(self.column_index, 2, "addr_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)
            ps.QRAMLoad(self.qram, self.column_index, "data_parent")(state)
            ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state)

            ps.CondRot_General_Bool("div_result", "rotation", func)(state)

            ps.ClearZero()(state)

            # Uncompute division and QRAM loads
            ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state)
            ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            ps.QRAMLoad(self.qram, self.column_index, "data_parent")(state)
            ps.Xgate_Bool("addr_child", 0)(state)
            ps.Mult_UInt_ConstUInt(self.column_index, 2, "addr_child")(state)
            ps.Add_ConstUInt(self.column_index, pow2(self.addr_size) - pow2(k) + 1)(state)
            ps.ShiftLeft(self.column_index, 1)(state)
            ps.SplitRegister(self.column_index, "temp_bit", 1)(state)
            ps.CombineRegister(self.column_index, "rotation")(state)
            ps.ShiftLeft(self.column_index, 1)(state)

        ps.ShiftRight(self.column_index, 1)(state)

        ps.RemoveRegister("data_parent")(state)
        ps.RemoveRegister("addr_child")(state)
        ps.RemoveRegister("data_child")(state)
        ps.RemoveRegister("temp_bit")(state)
        ps.RemoveRegister("div_result")(state)

        ps.ClearZero()(state)

    # -- dagger ----------------------------------------------------------------

    def dag(self, state: ps.SparseState) -> None:
        ps.AddRegister("addr_child", ps.UnsignedInteger, self.addr_size + 1)(state)
        ps.AddRegister("data_parent", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("data_child", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("temp_bit", ps.Boolean, 1)(state)
        ps.AddRegister("div_result", ps.Rational, self.rational_size)(state)

        n_digit = ps.System.size_of("div_result")
        func_inv = lambda value, _n=n_digit: make_func_inv(value, _n)

        ps.ShiftLeft(self.column_index, 1)(state)

        for k in range(self.addr_size):
            ps.ShiftRight(self.column_index, 1)(state)
            ps.SplitRegister(self.column_index, "rotation", 1)(state)
            ps.CombineRegister(self.column_index, "temp_bit")(state)
            ps.ShiftRight(self.column_index, 1)(state)
            ps.Add_ConstUInt(self.column_index, pow2(self.addr_size - 1 - k) - 1)(state)
            ps.Mult_UInt_ConstUInt(self.column_index, 2, "addr_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)
            ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            ps.QRAMLoad(self.qram, self.column_index, "data_parent")(state)
            ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state)

            ps.CondRot_General_Bool("div_result", "rotation", func_inv)(state)

            ps.ClearZero()(state)

            # Uncompute
            ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(state)
            ps.QRAMLoad(self.qram, self.column_index, "data_parent")(state)
            ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)
            ps.Mult_UInt_ConstUInt(self.column_index, 2, "addr_child")(state)
            ps.Add_ConstUInt(
                self.column_index,
                pow2(self.addr_size) - pow2(self.addr_size - 1 - k) + 1,
            )(state)
            ps.ShiftLeft(self.column_index, 1)(state)
            ps.SplitRegister(self.column_index, "temp_bit", 1)(state)
            ps.CombineRegister(self.column_index, "rotation")(state)

        ps.RemoveRegister("data_parent")(state)
        ps.RemoveRegister("addr_child")(state)
        ps.RemoveRegister("data_child")(state)
        ps.RemoveRegister("temp_bit")(state)
        ps.RemoveRegister("div_result")(state)

        ps.ClearZero()(state)


# ==============================================================================
# UL
# ==============================================================================


class UL:
    """Left-multiplication operator for QRAM-based block encoding.

    Implements the :math:`U_L` unitary that encodes the row structure of
    the target matrix.  The operator iterates over the upper address bits
    of the row index, constructing parent/child addresses, loading data
    from QRAM, computing rotation angles, and applying conditional
    rotations.

    Ported from ``block_encoding_via_QRAM.h:138-307``.
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        row_index: str,
        column_index: str,
        data_size: int,
        rational_size: int,
    ):
        self.qram = qram
        self.row_index = row_index
        self.column_index = column_index
        self.data_size = data_size
        self.rational_size = rational_size
        self.addr_size = ps.System.size_of(row_index)

        self._condition_regs: list[str] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    # -- conditioning helpers --------------------------------------------------

    def conditioned_by_nonzeros(self, conds: str | list[str]) -> UL:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_all_ones(self, conds: str | list[str]) -> UL:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_bit(self, reg: str | int, pos: int) -> UL:
        self._condition_bits = [(reg, pos)]
        return self

    def clear_conditions(self) -> None:
        self._condition_regs = []
        self._condition_bits = []

    # -- forward ---------------------------------------------------------------

    def __call__(self, state: ps.SparseState) -> None:
        ps.AddRegister("addr_parent", ps.UnsignedInteger, 2 * self.addr_size + 1)(state)
        ps.AddRegister("addr_child", ps.UnsignedInteger, 2 * self.addr_size + 1)(state)
        ps.AddRegister("data_parent", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("data_child", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("div_result", ps.Rational, self.rational_size)(state)

        n_digit = ps.System.size_of("div_result")
        func = lambda value, _n=n_digit: make_func(value, _n)

        for k in range(self.addr_size, 2 * self.addr_size):
            ps.SplitRegister(self.row_index, "rotation", 1)(state)
            ps.Add_ConstUInt("addr_parent", pow2(k) - 1)(state)
            ps.Add_Mult_UInt_ConstUInt(
                self.column_index, pow2(k - self.addr_size), "addr_parent"
            )(state)
            ps.Add_UInt_UInt_InPlace(self.row_index, "addr_parent")(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)

            if k != 2 * self.addr_size - 1:
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(
                    state
                )

                ps.CondRot_General_Bool("div_result", "rotation", func)(state)

                # Uncompute
                ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(
                    state
                )
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
            else:
                # Last iteration: use GetRotateAngle instead of Div
                ps.ShiftLeft("addr_parent", 1)(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.Add_ConstUInt("addr_child", 1)(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)

                ps.CondRot_General_Bool("div_result", "rotation", func)(state)

                # Uncompute
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.Add_ConstUInt("addr_child", 1).dag(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.ShiftRight("addr_parent", 1)(state)

            # Uncompute address computation
            ps.Xgate_Bool("addr_child", 0)(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Add_UInt_UInt_InPlace(self.row_index, "addr_parent").dag(state)
            ps.Add_Mult_UInt_ConstUInt(
                self.column_index, pow2(k - self.addr_size), "addr_parent"
            ).dag(state)
            ps.Add_ConstUInt("addr_parent", pow2(k) - 1).dag(state)
            ps.CombineRegister(self.row_index, "rotation")(state)
            ps.ShiftLeft(self.row_index, 1)(state)

        ps.ShiftRight(self.row_index, 1)(state)

        ps.RemoveRegister("addr_parent")(state)
        ps.RemoveRegister("data_parent")(state)
        ps.RemoveRegister("addr_child")(state)
        ps.RemoveRegister("data_child")(state)
        ps.RemoveRegister("div_result")(state)
        ps.ClearZero()(state)

    # -- dagger ----------------------------------------------------------------

    def dag(self, state: ps.SparseState) -> None:
        ps.AddRegister("addr_parent", ps.UnsignedInteger, 2 * self.addr_size + 1)(state)
        ps.AddRegister("addr_child", ps.UnsignedInteger, 2 * self.addr_size + 1)(state)
        ps.AddRegister("data_parent", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("data_child", ps.SignedInteger, self.data_size)(state)
        ps.AddRegister("div_result", ps.Rational, self.rational_size)(state)

        n_digit = ps.System.size_of("div_result")
        func_inv = lambda value, _n=n_digit: make_func_inv(value, _n)

        ps.ShiftLeft(self.row_index, 1)(state)

        for k in range(2 * self.addr_size - 1, self.addr_size - 1, -1):
            ps.ShiftRight(self.row_index, 1)(state)
            ps.SplitRegister(self.row_index, "rotation", 1)(state)
            ps.Add_ConstUInt("addr_parent", pow2(k) - 1)(state)
            ps.Add_Mult_UInt_ConstUInt(
                self.column_index, pow2(k - self.addr_size), "addr_parent"
            )(state)
            ps.Add_UInt_UInt_InPlace(self.row_index, "addr_parent")(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Xgate_Bool("addr_child", 0)(state)

            if k != 2 * self.addr_size - 1:
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(
                    state
                )

                ps.CondRot_General_Bool("div_result", "rotation", func_inv)(state)

                ps.ClearZero()(state)

                # Uncompute
                ps.Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(
                    state
                )
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
            else:
                # Last iteration
                ps.ShiftLeft("addr_parent", 1)(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.Add_ConstUInt("addr_child", 1)(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)

                ps.CondRot_General_Bool("div_result", "rotation", func_inv)(state)

                ps.ClearZero()(state)

                # Uncompute
                ps.GetRotateAngle_Int_Int(
                    "data_parent", "data_child", "div_result"
                )(state)
                ps.QRAMLoad(self.qram, "addr_child", "data_child")(state)
                ps.QRAMLoad(self.qram, "addr_parent", "data_parent")(state)
                ps.Add_ConstUInt("addr_child", 1).dag(state)
                ps.Xgate_Bool("addr_parent", 0)(state)
                ps.ShiftRight("addr_parent", 1)(state)

            # Uncompute address computation
            ps.Xgate_Bool("addr_child", 0)(state)
            ps.Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(state)
            ps.Add_UInt_UInt_InPlace(self.row_index, "addr_parent").dag(state)
            ps.Add_Mult_UInt_ConstUInt(
                self.column_index, pow2(k - self.addr_size), "addr_parent"
            ).dag(state)
            ps.Add_ConstUInt("addr_parent", pow2(k) - 1).dag(state)
            ps.CombineRegister(self.row_index, "rotation")(state)

        ps.RemoveRegister("addr_parent")(state)
        ps.RemoveRegister("data_parent")(state)
        ps.RemoveRegister("addr_child")(state)
        ps.RemoveRegister("data_child")(state)
        ps.RemoveRegister("div_result")(state)

        ps.ClearZero()(state)


# ==============================================================================
# BlockEncodingViaQRAM
# ==============================================================================


class BlockEncodingViaQRAM:
    """Block encoding of an arbitrary matrix via QRAM.

    The block encoding is composed as:

    .. math::

        U_A = \\text{SWAP}(\\text{row}, \\text{col}) \\cdot
              U_R^\\dagger(\\text{col}) \\cdot U_L(\\text{row}, \\text{col})

    satisfying ``<i|_col <0|_row U_A |j>_col |0>_row = A_{ij}``.

    Ported from ``block_encoding_via_QRAM.h:320-369``.
    """

    def __init__(
        self,
        qram: ps.QRAMCircuit_qutrit,
        column_index: str,
        row_index: str,
        data_size: int,
        rational_size: int,
    ):
        self.qram = qram
        self.column_index = column_index
        self.row_index = row_index
        self.data_size = data_size
        self.rational_size = rational_size

        self._condition_regs: list[str] = []
        self._condition_bits: list[tuple[str | int, int]] = []

    # -- conditioning helpers --------------------------------------------------

    def conditioned_by_nonzeros(
        self, conds: str | list[str]
    ) -> BlockEncodingViaQRAM:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_all_ones(
        self, conds: str | list[str]
    ) -> BlockEncodingViaQRAM:
        if isinstance(conds, list):
            self._condition_regs = conds
        else:
            self._condition_regs = [conds]
        return self

    def conditioned_by_bit(
        self, reg: str | int, pos: int
    ) -> BlockEncodingViaQRAM:
        self._condition_bits = [(reg, pos)]
        return self

    def clear_conditions(self) -> None:
        self._condition_regs = []
        self._condition_bits = []

    # -- internal helper -------------------------------------------------------

    def _do_split(self, state: ps.SparseState) -> ps.SparseState | None:
        if not self._condition_regs and not self._condition_bits:
            return None
        nonzero_ids = [ps.System.get_id(r) for r in self._condition_regs]
        by_bit = [(ps.System.get_id(r), v) for r, v in self._condition_bits]
        return ps.split_systems(state, nonzero_ids, [], by_bit, [])

    def _do_merge(self, state: ps.SparseState, split_info: ps.SparseState | None) -> None:
        if split_info is not None:
            ps.combine_systems(state, split_info)

    # -- forward ---------------------------------------------------------------

    def __call__(self, state: ps.SparseState) -> None:
        split_info = self._do_split(state)

        UL(
            self.qram,
            self.row_index,
            self.column_index,
            self.data_size,
            self.rational_size,
        )(state)
        UR(
            self.qram,
            self.column_index,
            self.data_size,
            self.rational_size,
        ).dag(state)
        ps.Swap_General_General(self.row_index, self.column_index)(state)

        self._do_merge(state, split_info)

    # -- dagger ----------------------------------------------------------------

    def dag(self, state: ps.SparseState) -> None:
        split_info = self._do_split(state)

        ps.Swap_General_General(self.row_index, self.column_index)(state)
        UR(
            self.qram,
            self.column_index,
            self.data_size,
            self.rational_size,
        )(state)
        UL(
            self.qram,
            self.row_index,
            self.column_index,
            self.data_size,
            self.rational_size,
        ).dag(state)

        self._do_merge(state, split_info)


# ==============================================================================
# Demo
# ==============================================================================


def create_block_encoding_demo() -> str:
    """Return a demo script string illustrating block encoding usage.

    Returns:
        A multi-line Python source string that exercises the tridiagonal
        block encoding and the classical utility functions.
    """
    return '''\
"""Demo: block encoding algorithms."""

import numpy as np
import pysparq as ps
from pysparq.algorithms.block_encoding import (
    get_tridiagonal_matrix,
    get_u_plus,
    get_u_minus,
    BlockEncodingTridiagonal,
)

# --- Classical utility functions ---
alpha, beta, dim = 0.5, 0.3, 4
A = get_tridiagonal_matrix(alpha, beta, dim)
print(f"Tridiagonal matrix (alpha={alpha}, beta={beta}):\\n{A}")

U_plus = get_u_plus(dim)
print(f"\\nU_plus (shift-down):\\n{U_plus}")

U_minus = get_u_minus(dim)
print(f"\\nU_minus (shift-up):\\n{U_minus}")

# --- Quantum tridiagonal block encoding ---
ps.System.clear()
ps.System.add_register("main_reg", ps.UnsignedInteger, 2)
ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

state = ps.SparseState()
ps.Init_Unsafe("main_reg", 0)(state)
ps.Init_Unsafe("anc_UA", 0)(state)
print(f"\\nInitial state size: {state.size()}")

block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
block_enc(state)
print(f"State size after block encoding: {state.size()}")

block_enc.dag(state)
print(f"State size after block encoding dagger: {state.size()}")

ps.System.clear()
print("\\nDone.")
'''
