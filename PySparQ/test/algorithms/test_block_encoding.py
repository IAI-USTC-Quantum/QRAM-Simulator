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

# Use C++ bound PlusOneAndOverflow
PlusOneAndOverflow = ps.PlusOneAndOverflow


class TestUtilityFunctions:
    """测试经典矩阵工具函数。"""

    def test_get_tridiagonal_matrix_diagonal(self):
        """验证三对角矩阵对角线元素。"""
        alpha, beta, dim = 2.0, 1.0, 4
        mat = get_tridiagonal_matrix(alpha, beta, dim)

        # 验证对角线
        for i in range(dim):
            assert mat[i, i] == alpha

    def test_get_tridiagonal_matrix_off_diagonal(self):
        """验证三对角矩阵次对角线元素。"""
        alpha, beta, dim = 2.0, 1.0, 4
        mat = get_tridiagonal_matrix(alpha, beta, dim)

        # 验证次对角线
        for i in range(dim - 1):
            assert mat[i, i + 1] == beta
            assert mat[i + 1, i] == beta

    def test_get_tridiagonal_matrix_zeros(self):
        """验证非三对角位置为零。"""
        alpha, beta, dim = 2.0, 1.0, 4
        mat = get_tridiagonal_matrix(alpha, beta, dim)

        # 验证其他位置为零
        for i in range(dim):
            for j in range(dim):
                if i == j or abs(i - j) == 1:
                    continue
                assert mat[i, j] == 0

    def test_get_u_plus_structure(self):
        """验证 U+ 矩阵结构（下移矩阵）。"""
        n = 4
        u_plus = get_u_plus(n)

        # U+[i, i-1] = 1 for i >= 1
        for i in range(1, n):
            assert u_plus[i, i - 1] == 1

        # 其他元素为零
        for i in range(n):
            for j in range(n):
                if j == i - 1:
                    continue
                assert u_plus[i, j] == 0

    def test_get_u_plus_sum(self):
        """验证 U+ 矩阵非零元素数量。"""
        n = 4
        u_plus = get_u_plus(n)

        # 非零元素数量 = n - 1
        assert np.sum(np.abs(u_plus)) == n - 1

    def test_get_u_minus_structure(self):
        """验证 U- 矩阵结构（上移矩阵）。"""
        n = 4
        u_minus = get_u_minus(n)

        # U-[i, i+1] = 1 for i < n-1
        for i in range(n - 1):
            assert u_minus[i, i + 1] == 1

        # 其他元素为零
        for i in range(n):
            for j in range(n):
                if j == i + 1:
                    continue
                assert u_minus[i, j] == 0

    def test_get_u_minus_sum(self):
        """验证 U- 矩阵非零元素数量。"""
        n = 4
        u_minus = get_u_minus(n)

        # 非零元素数量 = n - 1
        assert np.sum(np.abs(u_minus)) == n - 1

    def test_u_plus_u_minus_adjoint(self):
        """验证 U+ 和 U- 互为共轭转置。"""
        n = 4
        u_plus = get_u_plus(n)
        u_minus = get_u_minus(n)

        # U+ = U-^T
        assert np.allclose(u_plus, u_minus.T)


class TestPlusOneAndOverflow:
    """测试加一和溢出操作。"""

    def test_increment_no_overflow(self, fresh_system):
        """无溢出的加一操作。"""
        ps.System.add_register("main", ps.UnsignedInteger, 2)
        ps.System.add_register("overflow", ps.Boolean, 1)

        state = ps.SparseState()
        ps.Init_Unsafe("main", 1)(state)  # 初始值 1

        op = PlusOneAndOverflow("main", "overflow")
        op(state)

        # 应该变为 2，无溢出
        main_id = ps.System.get_id("main")
        overflow_id = ps.System.get_id("overflow")

        assert state.basis_states[0].get(main_id).value == 2
        assert state.basis_states[0].get(overflow_id).value == 0

    def test_increment_with_overflow(self, fresh_system):
        """有溢出的加一操作（最大值时）。"""
        n_bits = 2
        max_val = (1 << n_bits) - 1  # 3 for 2 bits

        ps.System.add_register("main", ps.UnsignedInteger, n_bits)
        ps.System.add_register("overflow", ps.Boolean, 1)

        state = ps.SparseState()
        ps.Init_Unsafe("main", max_val)(state)  # 初始值最大

        op = PlusOneAndOverflow("main", "overflow")
        op(state)

        main_id = ps.System.get_id("main")
        overflow_id = ps.System.get_id("overflow")

        # 应该回绕到 0，溢出位置位
        assert state.basis_states[0].get(main_id).value == 0
        assert state.basis_states[0].get(overflow_id).value == 1

    def test_dagger_cancels_forward(self, fresh_system):
        """dag 操作应该撤销前向操作。"""
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

        # 应该返回初始状态
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
        overflow_count = 0

        # 加 8 次（2^2 * 2）
        for _ in range(8):
            op(state)
            if state.basis_states[0].get(overflow_id).value == 1:
                overflow_count += 1

        # 应该有 2 次溢出
        assert overflow_count == 2
        # 最终值应该回到 0
        assert state.basis_states[0].get(main_id).value == 0


class TestBlockEncodingTridiagonal:
    """测试三对角矩阵块编码。"""

    def test_basic_encoding_execution(self, fresh_system):
        """基本块编码执行测试。"""
        n_bits = 2
        alpha, beta = 2.0, 0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Init_Unsafe("main_reg", 0)(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)

        # 应该产生更大的状态空间
        assert state.size() > 1

    def test_dagger_returns_to_initial(self, fresh_system):
        """dag 操作应该返回初始状态。"""
        n_bits = 2
        alpha, beta = 2.0, 0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Init_Unsafe("main_reg", 1)(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        # 记录初始状态
        initial_size = state.size()

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)
        block_enc.dag(state)

        # 最终状态大小应该回到初始
        assert state.size() == initial_size

    @pytest.mark.parametrize("alpha,beta", [
        (1.0, 0.0),  # 类单位矩阵
        (0.0, 1.0),  # 纯次对角线
        (2.0, -1.0),  # 负次对角线
        (1.5, 0.5),  # 一般情况
    ])
    def test_various_parameters(self, fresh_system, alpha, beta):
        """测试各种参数组合。"""
        n_bits = 2

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Hadamard_Int_Full("main_reg")(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
        block_enc(state)
        block_enc.dag(state)

        # 前向 + dag 后应该归一化
        ps.CheckNormalization(1e-6)(state)

    def test_identity_like_encoding(self, fresh_system):
        """测试类单位矩阵的块编码（beta=0）。"""
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

        # 主寄存器值应该保持不变
        assert state.basis_states[0].get(main_id).value == initial_val

    def test_negative_beta(self, fresh_system):
        """测试负 beta 参数。"""
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

        # 应该正常执行并归一化
        ps.CheckNormalization(1e-6)(state)


class TestBlockEncodingViaQRAM:
    """测试 QRAM 块编码。"""

    @pytest.mark.slow
    def test_simple_matrix_encoding(self, fresh_system):
        """测试简单矩阵的 QRAM 块编码。"""
        # 这个测试需要完整的 QRAM 设置
        # 标记为慢测试，因为涉及复杂的 QRAM 操作
        pytest.skip("需要完整的 QRAM 数据结构支持")

    def test_block_encoding_structure(self, fresh_system):
        """验证块编码类结构正确。"""
        # 测试类可以正常实例化
        from pysparq.algorithms.block_encoding import BlockEncodingViaQRAM, UR, UL

        # 创建小型 QRAM
        memory = [1, 2, 3, 4]
        qram = ps.QRAMCircuit_qutrit(2, 8, memory)

        ps.System.add_register("column_index", ps.UnsignedInteger, 2)
        ps.System.add_register("row_index", ps.UnsignedInteger, 2)

        # 验证可以创建操作符
        block_enc = BlockEncodingViaQRAM(qram, "column_index", "row_index", 8, 16)
        assert block_enc is not None

        ur = UR(qram, "column_index", 8, 16)
        assert ur is not None

        ul = UL(qram, "row_index", "column_index", 8, 16)
        assert ul is not None


class TestBlockEncodingIntegration:
    """块编码集成测试。"""

    def test_prep_state_computation(self):
        """验证预备态计算正确。"""
        # 对于 alpha=2, beta=0.5, n_bits=2
        # 预备态应该满足特定约束
        n_bits = 2
        alpha, beta = 2.0, 0.5

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)

        # 预备态应该是归一化的
        prep_state = block_enc.prep_state
        norm_sq = sum(abs(p) ** 2 for p in prep_state)
        assert abs(norm_sq - 1.0) < 1e-10

        ps.System.clear()

    def test_multiple_encodings_sequence(self, fresh_system):
        """测试多次块编码序列。"""
        n_bits = 2

        ps.System.add_register("main_reg", ps.UnsignedInteger, n_bits)
        ps.System.add_register("anc_UA", ps.UnsignedInteger, 4)

        state = ps.SparseState()
        ps.Hadamard_Int_Full("main_reg")(state)
        ps.Init_Unsafe("anc_UA", 0)(state)

        # 应用多个不同的块编码
        for alpha, beta in [(1.0, 0.5), (2.0, 0.3), (0.5, 1.0)]:
            block_enc = BlockEncodingTridiagonal("main_reg", "anc_UA", alpha, beta)
            block_enc(state)
            block_enc.dag(state)

        # 最终应该归一化
        ps.CheckNormalization(1e-5)(state)
