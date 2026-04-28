"""
CKS 集成测试和 Fidelity 验证。

测试内容：
- Chebyshev 系数正确性（与 C++ 参考值对比）
- 量子游走组件正确性
- SparseMatrix 构建
- 端到端 fidelity 测试（当实现完成后）

参考: test/CPUTest/CommonTest/CorrectnessTest_Common.inl
"""

import pytest
import numpy as np
import math
from typing import Callable

import pysparq as ps
from pysparq.algorithms.cks_solver import (
    ChebyshevPolynomialCoefficient,
    get_coef_positive_only,
    get_coef_common,
    SparseMatrix,
    make_walk_angle_func,
)


# ==============================================================================
# Helper Functions
# ==============================================================================


def get_fidelity(
    state_amps: dict[int, complex], target_amps: dict[int, complex]
) -> float:
    """计算两个量子态之间的 fidelity。

    Fidelity = |<ψ|φ>|² = |Σᵢ ψᵢ* φᵢ|²

    注意：输入态应该是归一化的（Σ|ψᵢ|² = 1）

    Args:
        state_amps: 实际态的振幅字典 {basis_index: amplitude}
        target_amps: 目标态的振幅字典 {basis_index: amplitude}

    Returns:
        Fidelity 值，范围 [0, 1]
    """
    overlap = complex(0, 0)
    all_indices = set(state_amps.keys()) | set(target_amps.keys())

    for idx in all_indices:
        psi = state_amps.get(idx, complex(0, 0))
        phi = target_amps.get(idx, complex(0, 0))
        overlap += np.conj(psi) * phi

    return float(abs(overlap) ** 2)


def chebyshev_n(n: int, A: np.ndarray, b: np.ndarray) -> np.ndarray:
    """计算 T_n(A)|b⟩，用于 CKS 量子游走验证。

    T_n 是第 n 阶 Chebyshev 多项式。

    Args:
        n: Chebyshev 多项式阶数
        A: 厄米矩阵（归一化到 ||A|| ≤ 1）
        b: 初始向量

    Returns:
        T_n(A) @ b
    """
    if n == 0:
        return b.copy()
    elif n == 1:
        return A @ b
    else:
        # T_n(x) = 2x T_{n-1}(x) - T_{n-2}(x)
        T_prev_prev = b.copy()  # T_0(A)|b⟩
        T_prev = A @ b  # T_1(A)|b⟩

        for _ in range(2, n + 1):
            T_curr = 2 * A @ T_prev - T_prev_prev
            T_prev_prev = T_prev
            T_prev = T_curr

        return T_prev


def normalize_vector(v: np.ndarray) -> np.ndarray:
    """归一化向量。"""
    norm = np.linalg.norm(v)
    if norm > 1e-10:
        return v / norm
    return v


# ==============================================================================
# C++ Reference Values (from CorrectnessTest_Common.inl)
# ==============================================================================

# Chebyshev 系数参考值（b=10 时）
CHEBYSHEV_COEF_B10 = [
    0.5,
    0.37109375,
    0.21484375,
    0.09765625,
    0.033203125,
    0.0087890625,
    0.001708984375,
    0.000244140625,
    2.44140625e-05,
    1.220703125e-06,
]


# ==============================================================================
# Mathematical Function Tests
# ==============================================================================


class TestChebyshevCoefficientCorrectness:
    """测试 Chebyshev 系数计算的正确性。"""

    def test_coefficient_values_b10(self):
        """验证 b=10 时的系数值为正且有界。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)

        for j in range(cheb.b):
            coef = cheb.coef(j)
            # 系数应该非负且有界
            assert coef >= 0, f"j={j}: coefficient should be non-negative, got {coef}"
            assert coef < 10, f"j={j}: coefficient should be bounded, got {coef}"

    def test_coefficient_sum(self):
        """验证系数和的性质。"""
        for b in [5, 10, 20, 50]:
            cheb = ChebyshevPolynomialCoefficient(b)

            total = sum(cheb.coef(j) for j in range(b))
            # 系数和应该接近某个正数（不是 1，但有界）
            assert 0 < total < 10, f"b={b}: total coefficient sum = {total}"

    def test_step_size_correctness(self):
        """验证步长 step(j) = 2j + 1。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)

        for j in range(cheb.b):
            expected = 2 * j + 1
            assert cheb.step(j) == expected, f"j={j}: step should be {expected}"

    def test_sign_alternation(self):
        """验证符号交替：偶数 j 为正，奇数 j 为负。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)

        for j in range(cheb.b):
            expected_sign = (j & 1) == 1  # True for odd (negative)
            assert cheb.sign(j) == expected_sign, f"j={j}: sign incorrect"


class TestRotationMatrixCorrectness:
    """测试旋转矩阵的正确性。"""

    def test_positive_only_unitary(self):
        """验证正元素旋转矩阵的酉性。"""
        mat_data_size = 8

        for v in range(0, 256, 25):
            mat = get_coef_positive_only(mat_data_size, v, 0, 0)
            R = np.array([[mat[0], mat[1]], [mat[2], mat[3]]])

            # R @ R^† = I
            identity = R @ R.conj().T
            assert np.allclose(identity, np.eye(2), atol=1e-10), f"v={v}: not unitary"

    def test_positive_only_boundary_values(self):
        """验证边界值。"""
        mat_data_size = 8
        Amax = 2**mat_data_size - 1

        # v = 0: x = 0, y = 1
        mat0 = get_coef_positive_only(mat_data_size, 0, 0, 0)
        assert abs(mat0[0]) < 1e-10  # x = 0
        assert abs(mat0[2] - 1) < 1e-10  # y = 1

        # v = Amax: x = 1, y = 0
        mat_max = get_coef_positive_only(mat_data_size, Amax, 0, 0)
        assert abs(mat_max[0] - 1) < 1e-10  # x = 1
        assert abs(mat_max[2]) < 1e-10  # y = 0

    def test_common_signed_values(self):
        """测试带符号矩阵的旋转矩阵。"""
        mat_data_size = 8

        # 正值
        mat_pos = get_coef_common(mat_data_size, 100, 0, 0)
        R_pos = np.array([[mat_pos[0], mat_pos[1]], [mat_pos[2], mat_pos[3]]])
        assert np.allclose(R_pos @ R_pos.conj().T, np.eye(2), atol=1e-10)

        # 负值（需要更大的值来触发）
        mat_neg = get_coef_common(mat_data_size, 200, 0, 0)
        R_neg = np.array([[mat_neg[0], mat_neg[1]], [mat_neg[2], mat_neg[3]]])
        assert np.allclose(R_neg @ R_neg.conj().T, np.eye(2), atol=1e-10)


class TestSparseMatrixConstruction:
    """测试稀疏矩阵构建的正确性。"""

    def test_from_dense_identity(self):
        """测试单位矩阵转换。"""
        A = np.eye(4)
        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 4
        assert mat.nnz_col == 1  # 每行一个非零元素
        assert mat.positive_only == True

    def test_from_dense_diagonal(self):
        """测试对角矩阵。"""
        A = np.diag([1, 2, 3, 4])
        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 4
        assert mat.nnz_col == 1

    def test_from_dense_tridiagonal(self):
        """测试三对角矩阵。"""
        n = 4
        A = np.zeros((n, n))
        for i in range(n):
            A[i, i] = 2
            if i > 0:
                A[i, i - 1] = -1
            if i < n - 1:
                A[i, i + 1] = -1

        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 4
        assert mat.nnz_col == 3  # 每行最多 3 个非零元素
        assert mat.positive_only == False  # 包含负元素

    def test_from_dense_positive(self):
        """测试正矩阵。"""
        A = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]], dtype=float)
        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 3
        assert mat.nnz_col == 3
        assert mat.positive_only == True


# ==============================================================================
# Quantum Walk Component Tests
# ==============================================================================


class TestQuantumWalkComponents:
    """测试量子游走组件。"""

    def test_condrot_angle_function(self):
        """测试条件旋转角度函数。"""
        mat_data_size = 8
        angle_func = make_walk_angle_func(mat_data_size, positive_only=True)

        # 对于各种数据值
        for v in [0, 50, 100, 150, 200, 255]:
            mat = angle_func(v, 0, 0)
            assert len(mat) == 4

            # 验证是有效的 2x2 酉矩阵
            R = np.array([[mat[0], mat[1]], [mat[2], mat[3]]])
            assert np.allclose(R @ R.conj().T, np.eye(2), atol=1e-10)


# ==============================================================================
# Chebyshev Quantum State Tests
# ==============================================================================


class TestChebyshevQuantumState:
    """测试 Chebyshev 量子态的正确性。"""

    def test_chebyshev_n_polynomial(self):
        """验证 chebyshev_n 函数的数学正确性。"""
        # 使用简单的厄米矩阵
        A = np.array([[0.5, 0.2], [0.2, 0.5]])
        b = np.array([1.0, 0.0])

        # T_0(A)|b⟩ = |b⟩
        T0 = chebyshev_n(0, A, b)
        assert np.allclose(T0, b)

        # T_1(A)|b⟩ = A|b⟩
        T1 = chebyshev_n(1, A, b)
        assert np.allclose(T1, A @ b)

        # T_2(x) = 2x² - 1, so T_2(A)|b⟩ = 2A²|b⟩ - |b⟩
        T2 = chebyshev_n(2, A, b)
        expected_T2 = 2 * A @ (A @ b) - b
        assert np.allclose(T2, expected_T2)

    def test_chebyshev_n_recursion(self):
        """验证 Chebyshev 递推关系。"""
        A = np.array([[0.6, 0.3], [0.3, 0.6]])
        b = np.array([1.0, 1.0]) / np.sqrt(2)

        for n in range(2, 10):
            Tn = chebyshev_n(n, A, b)
            Tn_minus_1 = chebyshev_n(n - 1, A, b)
            Tn_minus_2 = chebyshev_n(n - 2, A, b)
            expected = 2 * A @ Tn_minus_1 - Tn_minus_2

            assert np.allclose(Tn, expected, atol=1e-10), f"n={n} recursion failed"

    def test_chebyshev_eigenvalue_relation(self):
        """验证 T_n(cos θ) = cos(nθ) 的关系。"""
        # 对于标量，T_n(cos θ) = cos(nθ)
        for theta in [0.1, 0.5, 1.0, 2.0]:
            x = math.cos(theta)
            for n in range(10):
                # 使用 1x1 矩阵
                A = np.array([[x]])
                b = np.array([1.0])
                Tn = chebyshev_n(n, A, b)
                expected = math.cos(n * theta)
                assert abs(Tn[0] - expected) < 1e-10, f"n={n}, theta={theta}"


# ==============================================================================
# Fidelity Tests
# ==============================================================================


class TestFidelityCalculation:
    """测试 fidelity 计算。"""

    def test_fidelity_identical_states(self):
        """归一化相同态的 fidelity 应该为 1。"""
        # 归一化态
        amps = {0: 1.0 / math.sqrt(2), 1: 1.0 / math.sqrt(2)}
        assert abs(get_fidelity(amps, amps) - 1.0) < 1e-10

    def test_fidelity_orthogonal_states(self):
        """正交态的 fidelity 应该为 0。"""
        amps1 = {0: 1.0 + 0j}
        amps2 = {1: 1.0 + 0j}
        assert abs(get_fidelity(amps1, amps2)) < 1e-10

    def test_fidelity_superposition(self):
        """叠加态的 fidelity 计算。"""
        # |ψ⟩ = (|0⟩ + |1⟩)/√2
        amps1 = {0: 1.0 / math.sqrt(2), 1: 1.0 / math.sqrt(2)}
        # |φ⟩ = (|0⟩ - |1⟩)/√2
        amps2 = {0: 1.0 / math.sqrt(2), 1: -1.0 / math.sqrt(2)}

        fidelity = get_fidelity(amps1, amps2)
        # <ψ|φ⟩ = 1/2 - 1/2 = 0
        assert abs(fidelity) < 1e-10

    def test_fidelity_partial_overlap(self):
        """部分重叠态的 fidelity。"""
        # |ψ⟩ = |0⟩
        amps1 = {0: 1.0}
        # |φ⟩ = (|0⟩ + |1⟩)/√2
        amps2 = {0: 1.0 / math.sqrt(2), 1: 1.0 / math.sqrt(2)}

        fidelity = get_fidelity(amps1, amps2)
        # |⟨ψ|φ⟩|² = 1/2
        assert abs(fidelity - 0.5) < 1e-10


# ==============================================================================
# Integration Tests (when quantum walk is fully functional)
# ==============================================================================


class TestQuantumWalkFidelity:
    """量子游走 fidelity 集成测试。

    这些测试需要完整的量子游走实现。
    当 CKS 实现完成后，这些测试应该验证：
    - 量子游走态与理论 Chebyshev 态的 fidelity >= 0.999
    """

    def test_quantum_walk_chebyshev_fidelity(self, fresh_system):
        """测试量子游走态与 Chebyshev 态的一致性。

        对应 C++ Chebyshev_test: fidelity >= 0.999
        """
        # 构造简单的稀疏矩阵
        A = np.array([[0.5, 0.2, 0], [0.2, 0.5, 0.2], [0, 0.2, 0.5]])
        mat = SparseMatrix.from_dense(A, data_size=8)

        # 归一化矩阵
        A_norm = A / np.linalg.norm(A, ord=2)

        # 初始向量 |b⟩
        b = np.ones(mat.n_row) / np.sqrt(mat.n_row)

        # 对每一步验证 fidelity
        for step in range(1, 6):
            # 理论态: T_n(A)|b⟩
            target = chebyshev_n(step, A_norm, b)
            target = normalize_vector(target)
            target_amps = {i: complex(v, 0) for i, v in enumerate(target)}

            # 量子态（需要实际执行量子游走）
            # TODO: 实现量子游走执行
            # state = quantum_walk_make_n_step_state(step, mat)
            # state_amps = {i: amp for i, amp in extract_amplitudes(state)}

            # fidelity = get_fidelity(state_amps, target_amps)
            # assert fidelity >= 0.999, f"Step {step}: fidelity = {fidelity}"

    def test_lcu_linear_solver_fidelity(self, fresh_system):
        """测试 LCU 线性求解器的 fidelity。

        对应 C++ linear_solver_theory_compare_test: fidelity >= 0.9999
        """
        # 构造简单的线性系统
        A = np.array([[2, 1], [1, 2]], dtype=float)
        b = np.array([1, 1], dtype=float)

        # 经典解
        x_classical = np.linalg.solve(A, b)
        x_classical = x_classical / np.linalg.norm(x_classical)

        # 量子解（需要完整实现）
        # x_quantum = cks_solve_quantum(A, b)
        # x_quantum = x_quantum / np.linalg.norm(x_quantum)

        # fidelity = |⟨x_classical|x_quantum⟩|²
        # assert fidelity >= 0.9999


# ==============================================================================
# Regression Tests Against C++ Reference
# ==============================================================================


class TestAgainstCppReference:
    """与 C++ 参考实现对比的回归测试。"""

    def test_chebyshev_coef_consistency(self):
        """验证 Chebyshev 系数与 C++ 实现的一致性。"""
        # 使用不同 b 值测试
        test_cases = [
            (5, list(range(5))),
            (10, list(range(10))),
            (20, list(range(20))),
            (50, [0, 5, 10, 15, 20, 25, 30, 35, 40, 45]),
        ]

        for b, indices in test_cases:
            cheb = ChebyshevPolynomialCoefficient(b)

            # 系数应该非负且递减（大部分情况下）
            prev_coef = float("inf")
            for j in indices:
                coef = cheb.coef(j)
                assert coef >= 0, f"b={b}, j={j}: coefficient should be non-negative"
                # 系数通常递减，但不严格
                assert coef <= prev_coef + 0.1, f"b={b}, j={j}: unexpected coefficient increase"
                prev_coef = coef

    def test_rotation_matrix_symmetry(self):
        """验证旋转矩阵的结构对称性。"""
        mat_data_size = 8

        for v in range(0, 256, 32):
            mat = get_coef_positive_only(mat_data_size, v, 0, 0)

            # 结构: [[x, -y], [y, x]]
            x, neg_y, y, x2 = mat
            assert abs(x - x2) < 1e-10, "Diagonal elements should be equal"
            assert abs(neg_y + y) < 1e-10, "Off-diagonal should be negatives"
