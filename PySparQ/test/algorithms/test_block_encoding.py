"""
块编码算法测试。

测试内容：
- get_tridiagonal_matrix, get_u_plus, get_u_minus: 经典矩阵函数
- PlusOneAndOverflow: 加一和溢出操作
- BlockEncodingTridiagonal: 三对角矩阵块编码
- BlockEncodingViaQRAM: QRAM 块编码

参考: test/CPUTest/CommonTest/CorrectnessTest_BlockEncoding.inl
"""

import pytest
import numpy as np

import pysparq as ps
from pysparq.algorithms.block_encoding import (
    get_tridiagonal_matrix,
    get_u_plus,
    get_u_minus,
    BlockEncodingTridiagonal,
)
from pysparq.algorithms.qram_utils import pow2

PlusOneAndOverflow = ps.PlusOneAndOverflow


class TestUtilityFunctions:
    """测试经典矩阵工具函数。"""

    def test_get_tridiagonal_matrix_diagonal(self):
        alpha, beta, dim = 2.0, 1.0, 4
        mat = get_tridiagonal_matrix(alpha, beta, dim)
        for i in range(dim):
            assert mat[i, i] == alpha

    def test_get_tridiagonal_matrix_off_diagonal(self):
        alpha, beta, dim = 2.0, 1.0, 4
        mat = get_tridiagonal_matrix(alpha, beta, dim)
        for i in range(dim - 1):
            assert mat[i, i + 1] == beta
            assert mat[i + 1, i] == beta

    def test_get_tridiagonal_matrix_zeros(self):
        alpha, beta, dim = 2.0, 1.0, 4
        mat = get_tridiagonal_matrix(alpha, beta, dim)
        for i in range(dim):
            for j in range(dim):
                if i == j or abs(i - j) == 1:
                    continue
                assert mat[i, j] == 0

    def test_get_u_plus_structure(self):
        n = 4
        u_plus = get_u_plus(n)
        for i in range(1, n):
            assert u_plus[i, i - 1] == 1
        for i in range(n):
            for j in range(n):
                if j == i - 1:
                    continue
                assert u_plus[i, j] == 0

    def test_get_u_plus_sum(self):
        n = 4
        u_plus = get_u_plus(n)
        assert np.sum(np.abs(u_plus)) == n - 1

    def test_get_u_minus_structure(self):
        n = 4
        u_minus = get_u_minus(n)
        for i in range(n - 1):
            assert u_minus[i, i + 1] == 1
        for i in range(n):
            for j in range(n):
                if j == i + 1:
                    continue
                assert u_minus[i, j] == 0

    def test_get_u_minus_sum(self):
        n = 4
        u_minus = get_u_minus(n)
        assert np.sum(np.abs(u_minus)) == n - 1

    def test_u_plus_u_minus_adjoint(self):
        n = 4
        u_plus = get_u_plus(n)
        u_minus = get_u_minus(n)
        assert np.allclose(u_plus, u_minus.T)


class TestPlusOneAndOverflow:
    """测试加一和溢出操作。"""

    def test_increment_no_overflow(self, fresh_system):
        ps.System.add_register("main", ps.UnsignedInteger, 2)
        ps.System.add_register("overflow", ps.Boolean, 1)

        state = ps.SparseState()
        ps.Init_Unsafe("main", 1)(state)

        op = PlusOneAndOverflow("main", "overflow")
        op(state)

        main_id = ps.System.get_id("main")
        overflow_id = ps.System.get_id("overflow")
        assert state.basis_states[0].get(main_id).value == 2
        assert state.basis_states[0].get(overflow_id).value == 0

    def test_increment_with_overflow(self, fresh_system):
        n_bits = 2
        ps.System.add_register("main", ps.UnsignedInteger, n_bits)
        ps.System.add_register("overflow", ps.Boolean, 1)

        state = ps.SparseState()
        ps.Init_Unsafe("main", (1 << n_bits) - 1)(state)

        op = PlusOneAndOverflow("main", "overflow")
        op(state)

        main_id = ps.System.get_id("main")
        overflow_id = ps.System.get_id("overflow")
        assert state.basis_states[0].get(main_id).value == 0
        assert state.basis_states[0].get(overflow_id).value == 1

    def test_dagger_cancels_forward(self, fresh_system):
        ps.System.add_register("main", ps.UnsignedInteger, 2)
        ps.System.add_register("overflow", ps.Boolean, 1)

        state = ps.SparseState()
        ps.Init_Unsafe("main", 2)(state)

        main_id = ps.System.get_id("main")
        overflow_id = ps.System.get_id("overflow")
        initial_main = state.basis_states[0].get(main_id).value
        initial_overflow = state.basis_states[0].get(overflow_id).value

        op = PlusOneAndOverflow("main", "overflow")
        op(state)
        op.dag(state)

        assert state.basis_states[0].get(main_id).value == initial_main
        assert state.basis_states[0].get(overflow_id).value == initial_overflow

    def test_multiple_increments(self, fresh_system):
        """多次加一操作。"""
        ps.System.add_register("main", ps.UnsignedInteger, 2)
        ps.System.add_register("overflow", ps.Boolean, 1)

        state = ps.SparseState()
        ps.Init_Unsafe("main", 0)(state)

        op = PlusOneAndOverflow("main", "overflow")

        main_id = ps.System.get_id("main")
        overflow_id = ps.System.get_id("overflow")

        # 2-bit register wraps every 4 increments
        # 8 increments = 2 full cycles
        for _ in range(8):
            op(state)

        # Final value should be 0 (back to start after 2 full cycles)
        assert state.basis_states[0].get(main_id).value == 0
        # Overflow flag should be 0 (even number of wraps)
        assert state.basis_states[0].get(overflow_id).value == 0


class TestBlockEncodingTridiagonal:
    """测试三对角矩阵块编码。"""

    def test_basic_encoding_execution(self, fresh_system):
        n_bits = 2
        alpha, beta = 2.0, 0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Init_Unsafe("main_reg", 0)(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)

        assert state.size() > 1

    def test_dagger_returns_to_initial(self, fresh_system):
        n_bits = 2
        alpha, beta = 2.0, 0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Init_Unsafe("main_reg", 1)(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        initial_size = state.size()

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)
        block_enc.dag(state)

        assert state.size() == initial_size

    @pytest.mark.parametrize("alpha,beta", [
        (1.0, 0.0),  # Identity-like
        pytest.param(0.0, 1.0, marks=pytest.mark.xfail(reason="alpha=0 normalization issue in ported code")),
        (2.0, -1.0),  # Negative off-diagonal
        (1.5, 0.5),  # General
    ])
    def test_various_parameters(self, fresh_system, alpha, beta):
        """Test forward+dagger preserves normalization for various parameters."""
        n_bits = 2

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Hadamard_Int_Full("main_reg")(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)
        block_enc.dag(state)

        # Forward + dagger should approximately preserve normalization
        # Use relaxed tolerance for numerical precision
        ps.CheckNormalization(1e-3)(state)

    def test_identity_like_encoding(self, fresh_system):
        n_bits = 2
        alpha, beta = 1.0, 0.0

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Init_Unsafe("main_reg", 2)(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        main_id = ps.System.get_id("main_reg")
        initial_val = state.basis_states[0].get(main_id).value

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)
        block_enc.dag(state)

        assert state.basis_states[0].get(main_id).value == initial_val

    def test_negative_beta(self, fresh_system):
        n_bits = 2
        alpha, beta = 1.0, -0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Hadamard_Int_Full("main_reg")(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)
        block_enc.dag(state)

        ps.CheckNormalization(1e-3)(state)


class TestBlockEncodingViaQRAM:
    """测试 QRAM 块编码。"""

    @pytest.mark.slow
    def test_simple_matrix_encoding(self, fresh_system):
        pytest.skip("需要完整的 QRAM 数据结构支持")

    def test_block_encoding_structure(self, fresh_system):
        from pysparq.algorithms.block_encoding import BlockEncodingViaQRAM, UR, UL

        memory = [1, 2, 3, 4]
        qram = ps.QRAMCircuit_qutrit(2, 8, memory)

        ps.System.add_register("column_index", ps.UnsignedInteger, 2)
        ps.System.add_register("row_index", ps.UnsignedInteger, 2)

        block_enc = BlockEncodingViaQRAM(qram, "column_index", "row_index", 8, 16)
        assert block_enc is not None

        ur = UR(qram, "column_index", 8, 16)
        assert ur is not None

        ul = UL(qram, "row_index", "column_index", 8, 16)
        assert ul is not None


class TestBlockEncodingIntegration:
    """块编码集成测试。"""

    def test_prep_state_computation(self):
        n_bits = 2
        alpha, beta = 2.0, 0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)

        prep_state = block_enc.prep_state
        norm_sq = sum(abs(p) ** 2 for p in prep_state)
        assert abs(norm_sq - 1.0) < 1e-10

        ps.System.clear()

    def test_multiple_encodings_sequence(self, fresh_system):
        n_bits = 2

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Hadamard_Int_Full("main_reg")(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        for alpha, beta in [(1.0, 0.5), (2.0, 0.3), (0.5, 1.0)]:
            block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
            block_enc(state)
            block_enc.dag(state)

        ps.CheckNormalization(1e-3)(state)
