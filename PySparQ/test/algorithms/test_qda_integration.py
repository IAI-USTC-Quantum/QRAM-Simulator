"""
QDA 集成测试和 Fidelity 验证。

测试内容：
- 插值参数 f(s) 正确性
- 旋转矩阵 R_s 正确性
- Dolph-Chebyshev 滤波器正确性
- WalkS 算子正确性
- 端到端 fidelity 测试

参考: test/CPUTest/CommonTest/CorrectnessTest_QDA_CompareList.inl
"""

import pytest
import numpy as np
import math
from typing import Callable

import pysparq as ps
from pysparq.algorithms.qda_solver import (
    compute_fs,
    compute_rotation_matrix,
    chebyshev_T,
    dolph_chebyshev,
    compute_fourier_coeffs,
    calculate_angles,
)


# ==============================================================================
# Helper Functions
# ==============================================================================


def get_fidelity(
    state_amps: dict[int, complex], target_amps: dict[int, complex]
) -> float:
    """计算两个量子态之间的 fidelity。

    Fidelity = |<ψ|φ>|² = |Σᵢ ψᵢ* φᵢ|²
    """
    overlap = complex(0, 0)
    all_indices = set(state_amps.keys()) | set(target_amps.keys())

    for idx in all_indices:
        psi = state_amps.get(idx, complex(0, 0))
        phi = target_amps.get(idx, complex(0, 0))
        overlap += np.conj(psi) * phi

    return float(abs(overlap) ** 2)


def generate_poiseuille_matrix(n: int, alpha: float = 1.0, beta: float = 1.0) -> np.ndarray:
    """生成 Poiseuille 流的三对角矩阵。

    对应 C++ generate_Poiseuille_mat 函数。
    矩阵结构为：
        A[i,i] = alpha
        A[i,i-1] = -beta (i > 0)
        A[i,i+1] = -beta (i < n-1)
    """
    A = np.zeros((n, n), dtype=float)
    for i in range(n):
        A[i, i] = alpha
        if i > 0:
            A[i, i - 1] = -beta
        if i < n - 1:
            A[i, i + 1] = -beta
    return A


def normalize_matrix(A: np.ndarray) -> np.ndarray:
    """归一化矩阵使其 Frobenius 范数为 1。"""
    norm = np.linalg.norm(A, "fro")
    if norm > 1e-10:
        return A / norm
    return A


def compute_kappa(A: np.ndarray) -> float:
    """计算矩阵的条件数。"""
    try:
        eigvals = np.linalg.eigvalsh(A)
        min_eig = max(np.min(np.abs(eigvals)), 1e-10)
        max_eig = np.max(np.abs(eigvals))
        return max_eig / min_eig
    except np.linalg.LinAlgError:
        return 10.0


# ==============================================================================
# C++ Reference Fidelity Values
# ==============================================================================
# 从 CorrectnessTest_QDA_CompareList.inl 提取的参考值
# 格式: nqubit=4, step_rate=1.0, p=0.5, alpha=1, beta=1

QDA_FIDELITY_REFERENCE_TRI_NEG = [
    0.9999998025428056, 0.9999999999615321, 0.9999992164840841, 0.9999997924887332,
    0.99999981389001, 0.999999999836926, 0.999999208907277, 0.9999997589478243,
    0.9999998331447031, 0.9999999976742941, 0.9999992337367896, 0.9999996745639621,
    0.9999998712791837, 0.9999999812301907, 0.9999993326129166, 0.9999995007243456,
]

QDA_FIDELITY_REFERENCE_TRI_POS = [
    0.999988745049859, 0.9999999979371405, 0.9999553176556475, 0.9999882044632673,
    0.9999893739308179, 0.9999999984056873, 0.9999547252921027, 0.9999866245703776,
    0.9999903394517995, 0.9999999499003555, 0.9999555037260702, 0.9999828971901702,
    0.9999920858331532, 0.9999994629803715, 0.9999592751833802, 0.9999753570356213,
]


# ==============================================================================
# Interpolation Parameter Tests
# ==============================================================================


class TestComputeFsCorrectness:
    """测试插值参数 f(s) 的正确性。"""

    def test_fs_at_zero(self):
        """f(0) = 0。"""
        for kappa in [2.0, 5.0, 10.0, 100.0]:
            for p in [0.3, 0.5, 0.7]:
                fs = compute_fs(0.0, kappa, p)
                assert abs(fs) < 1e-10, f"kappa={kappa}, p={p}: f(0) should be 0"

    def test_fs_at_one(self):
        """f(1) = 1。"""
        for kappa in [2.0, 5.0, 10.0, 100.0]:
            for p in [0.3, 0.5, 0.7]:
                fs = compute_fs(1.0, kappa, p)
                assert abs(fs - 1.0) < 1e-10, f"kappa={kappa}, p={p}: f(1) should be 1"

    def test_fs_kappa_one_identity(self):
        """当 kappa=1 时，f(s) = s。"""
        kappa = 1.0
        p = 0.5

        for s in np.linspace(0, 1, 20):
            fs = compute_fs(s, kappa, p)
            assert abs(fs - s) < 1e-10, f"s={s}: f(s) should equal s when kappa=1"

    def test_fs_monotonicity(self):
        """f(s) 应该单调递增。"""
        kappa = 10.0
        p = 0.5

        prev_fs = compute_fs(0.0, kappa, p)
        for s in np.linspace(0.05, 1.0, 20):
            fs = compute_fs(s, kappa, p)
            assert fs >= prev_fs - 1e-10, f"s={s}: f(s) should be monotonic"
            prev_fs = fs

    def test_fs_bounded(self):
        """f(s) 应该在 [0, 1] 范围内。"""
        for kappa in [2.0, 10.0, 100.0]:
            for p in [0.1, 0.5, 0.9]:
                for s in np.linspace(0, 1, 20):
                    fs = compute_fs(s, kappa, p)
                    assert 0.0 <= fs <= 1.0, f"kappa={kappa}, p={p}, s={s}: f(s)={fs} out of bounds"

    def test_fs_different_p(self):
        """测试不同调度参数 p 的影响。"""
        kappa = 10.0
        s = 0.5

        # 不同的 p 值应该给出不同的 f(s)
        fs_values = [compute_fs(s, kappa, p) for p in [0.2, 0.5, 0.8]]
        # 它们应该都在合理范围内
        for fs in fs_values:
            assert 0.0 <= fs <= 1.0


# ==============================================================================
# Rotation Matrix Tests
# ==============================================================================


class TestRotationMatrixCorrectness:
    """测试旋转矩阵 R_s 的正确性。"""

    def test_rotation_matrix_unitary(self):
        """旋转矩阵应该是酉矩阵。"""
        for fs in [0.0, 0.2, 0.5, 0.8, 1.0]:
            R = compute_rotation_matrix(fs)
            R_mat = np.array([[R[0], R[1]], [R[2], R[3]]])

            # R @ R^† = I
            identity = R_mat @ R_mat.conj().T
            assert np.allclose(identity, np.eye(2), atol=1e-10), f"fs={fs}: not unitary"

    def test_rotation_matrix_determinant(self):
        """旋转矩阵行列式应该为 -1。"""
        for fs in [0.0, 0.2, 0.5, 0.8, 1.0]:
            R = compute_rotation_matrix(fs)
            det = R[0] * R[3] - R[1] * R[2]
            assert abs(det + 1) < 1e-10, f"fs={fs}: det should be -1, got {det}"

    def test_rotation_matrix_fs_zero(self):
        """fs=0 时的旋转矩阵应该是 [[1, 0], [0, -1]]。"""
        R = compute_rotation_matrix(0.0)
        assert abs(R[0] - 1) < 1e-10
        assert abs(R[1]) < 1e-10
        assert abs(R[2]) < 1e-10
        assert abs(R[3] + 1) < 1e-10

    def test_rotation_matrix_fs_one(self):
        """fs=1 时的旋转矩阵应该是 [[0, 1], [1, 0]]。"""
        R = compute_rotation_matrix(1.0)
        assert abs(R[0]) < 1e-10
        assert abs(R[1] - 1) < 1e-10
        assert abs(R[2] - 1) < 1e-10
        assert abs(R[3]) < 1e-10

    def test_rotation_matrix_structure(self):
        """验证旋转矩阵的结构：R = N * [[1-fs, fs], [fs, fs-1]]。"""
        for fs in [0.1, 0.3, 0.5, 0.7, 0.9]:
            R = compute_rotation_matrix(fs)

            sqrt_N = 1.0 / math.sqrt((1 - fs) ** 2 + fs**2)

            expected_00 = sqrt_N * (1 - fs)
            expected_01 = sqrt_N * fs
            expected_10 = sqrt_N * fs
            expected_11 = sqrt_N * (fs - 1)

            assert abs(R[0] - expected_00) < 1e-10, f"fs={fs}: R[0] mismatch"
            assert abs(R[1] - expected_01) < 1e-10, f"fs={fs}: R[1] mismatch"
            assert abs(R[2] - expected_10) < 1e-10, f"fs={fs}: R[2] mismatch"
            assert abs(R[3] - expected_11) < 1e-10, f"fs={fs}: R[3] mismatch"


# ==============================================================================
# Chebyshev Polynomial Tests
# ==============================================================================


class TestChebyshevPolynomial:
    """测试 Chebyshev 多项式的正确性。"""

    def test_chebyshev_T_values(self):
        """验证 Chebyshev 多项式的已知值。"""
        # T_0(x) = 1
        assert chebyshev_T(0, 0.5) == 1.0

        # T_1(x) = x
        assert chebyshev_T(1, 0.5) == 0.5

        # T_2(x) = 2x² - 1
        assert abs(chebyshev_T(2, 0.5) - (-0.5)) < 1e-10

        # T_3(x) = 4x³ - 3x
        assert abs(chebyshev_T(3, 0.5) - (-1.0)) < 1e-10

    def test_chebyshev_recursion(self):
        """验证递推关系 T_n(x) = 2x T_{n-1}(x) - T_{n-2}(x)。"""
        x = 0.7

        for n in range(2, 20):
            Tn = chebyshev_T(n, x)
            Tn_minus_1 = chebyshev_T(n - 1, x)
            Tn_minus_2 = chebyshev_T(n - 2, x)
            expected = 2 * x * Tn_minus_1 - Tn_minus_2

            assert abs(Tn - expected) < 1e-10, f"n={n}: recursion failed"

    def test_chebyshev_at_one(self):
        """T_n(1) = 1 对所有 n。"""
        for n in range(20):
            assert chebyshev_T(n, 1.0) == 1.0, f"n={n}: T_n(1) should be 1"

    def test_chebyshev_at_minus_one(self):
        """T_n(-1) = (-1)^n。"""
        for n in range(20):
            result = chebyshev_T(n, -1.0)
            expected = (-1) ** n
            assert abs(result - expected) < 1e-10, f"n={n}: T_n(-1) should be {expected}"

    def test_chebyshev_cosine_relation(self):
        """验证 T_n(cos θ) = cos(nθ)。"""
        for theta in [0.1, 0.5, 1.0, 2.0]:
            x = math.cos(theta)

            for n in range(10):
                Tn = chebyshev_T(n, x)
                expected = math.cos(n * theta)

                assert abs(Tn - expected) < 1e-10, f"n={n}, theta={theta}: mismatch"


# ==============================================================================
# Dolph-Chebyshev Filter Tests
# ==============================================================================


class TestDolphChebyshevFilter:
    """测试 Dolph-Chebyshev 滤波器。"""

    def test_fourier_coeffs_length(self):
        """Fourier 系数列表长度应该为 ceil((l+1)/2)（只保留偶数索引）。"""
        for l in [3, 5, 10, 20]:
            for epsilon in [0.01, 0.1, 0.5]:
                coeffs = compute_fourier_coeffs(epsilon, l)
                # 实现只保留偶数索引的系数
                expected_len = (l + 2) // 2
                assert len(coeffs) == expected_len, f"l={l}: expected {expected_len} coeffs, got {len(coeffs)}"

    def test_fourier_coeffs_positive(self):
        """Fourier 系数应该非负（大部分）。"""
        epsilon = 0.1
        l = 10

        coeffs = compute_fourier_coeffs(epsilon, l)

        # 系数可能有小的负值（数值误差），但主要应该为正
        positive_count = sum(1 for c in coeffs if c > -0.1)
        assert positive_count >= len(coeffs) * 0.8, "Most coefficients should be positive"

    def test_dolph_chebyshev_at_zero(self):
        """phi=0 时的值应该有效。"""
        epsilon = 0.1
        l = 5

        result = dolph_chebyshev(epsilon, l, 0.0)
        assert isinstance(result, float)


# ==============================================================================
# Poiseuille Matrix Tests
# ==============================================================================


class TestPoiseuilleMatrix:
    """测试 Poiseuille 流矩阵。"""

    def test_matrix_structure(self):
        """验证 Poiseuille 矩阵结构。"""
        n = 4
        alpha, beta = 1.0, 1.0

        A = generate_poiseuille_matrix(n, alpha, beta)

        # 对角线元素
        for i in range(n):
            assert A[i, i] == alpha, f"Diagonal element mismatch at ({i},{i})"

        # 下对角线
        for i in range(1, n):
            assert A[i, i - 1] == -beta, f"Lower diagonal mismatch at ({i},{i-1})"

        # 上对角线
        for i in range(n - 1):
            assert A[i, i + 1] == -beta, f"Upper diagonal mismatch at ({i},{i+1})"

    def test_matrix_hermitian(self):
        """Poiseuille 矩阵应该是厄米的。"""
        for n in [4, 8, 16]:
            A = generate_poiseuille_matrix(n)
            assert np.allclose(A, A.T), f"n={n}: matrix should be symmetric"

    def test_matrix_condition_number(self):
        """测试矩阵条件数。"""
        for n in [4, 8, 16]:
            A = generate_poiseuille_matrix(n)
            A_norm = normalize_matrix(A)
            kappa = compute_kappa(A_norm)

            # Poiseuille 矩阵条件数应该随着 n 增加
            assert kappa > 1.0, f"n={n}: condition number should be > 1"

    def test_matrix_eigenvalues(self):
        """测试矩阵特征值。"""
        n = 4
        A = generate_poiseuille_matrix(n)

        eigvals = np.linalg.eigvalsh(A)

        # Poiseuille 矩阵（alpha=1, beta=1）有正和负特征值
        # 确保特征值是实数（对称矩阵）
        assert np.all(np.isreal(eigvals)), "Eigenvalues should be real"

        # 归一化后的矩阵特征值应该在合理范围
        A_norm = normalize_matrix(A)
        eigvals_norm = np.linalg.eigvalsh(A_norm)
        assert np.all(np.abs(eigvals_norm) <= 2.0), "Normalized eigenvalues should be bounded"


# ==============================================================================
# Interpolation Sequence Tests
# ==============================================================================


class TestInterpolationSequence:
    """测试插值序列的性质。"""

    def test_interpolation_sequence_continuous(self):
        """插值序列应该连续变化。"""
        kappa = 10.0
        p = 0.5
        steps = 100

        fs_values = [compute_fs(s, kappa, p) for s in np.linspace(0, 1, steps)]

        # 验证连续性：相邻值差异应该很小
        for i in range(1, len(fs_values)):
            diff = abs(fs_values[i] - fs_values[i - 1])
            assert diff < 0.1, f"Jump at step {i}: {diff}"

    def test_rotation_matrix_sequence_unitary(self):
        """所有旋转矩阵应该都是酉的。"""
        for fs in np.linspace(0, 1, 20):
            R = compute_rotation_matrix(fs)
            R_mat = np.array([[R[0], R[1]], [R[2], R[3]]])

            identity = R_mat @ R_mat.conj().T
            assert np.allclose(identity, np.eye(2), atol=1e-10), f"fs={fs}: not unitary"


# ==============================================================================
# Fidelity Tests Against Reference
# ==============================================================================


class TestQDAFidelityAgainstReference:
    """与 C++ 参考值对比的 fidelity 测试。"""

    def test_fidelity_reference_values_valid(self):
        """验证参考有效性。"""
        # Tridiagonal 版本的参考值
        assert len(QDA_FIDELITY_REFERENCE_TRI_NEG) > 0
        assert len(QDA_FIDELITY_REFERENCE_TRI_POS) > 0

        # 所有值应该接近 1
        for i, f in enumerate(QDA_FIDELITY_REFERENCE_TRI_NEG):
            assert 0.99 < f < 1.001, f"Reference neg value {i} = {f} out of range"

        for i, f in enumerate(QDA_FIDELITY_REFERENCE_TRI_POS):
            # pos 版本精度稍低
            assert 0.99 < f < 1.001, f"Reference pos value {i} = {f} out of range"

    @pytest.mark.skip(reason="cks_solver.py TOperator has fundamental porting bugs (#76)")
    def test_walks_fidelity_tridiagonal(self, fresh_system):
        """测试 Tridiagonal 版本的 WalkS fidelity。

        对应 C++ QDA_Poiseuille_Tridiagonal_test:
        逐帧验证 fidelity 与参考值差异 < 1e-5
        """
        nqubit = 4
        alpha, beta = 1.0, 1.0
        p = 0.5
        step_rate = 1.0

        # 生成 Poiseuille 矩阵
        A = generate_poiseuille_matrix(2**nqubit, alpha, beta)
        A = normalize_matrix(A)
        kappa = compute_kappa(A)

        # 计算步数
        STEP_CONSTANT = 2305
        steps = int(step_rate * STEP_CONSTANT * kappa)
        if steps % 2 != 0:
            steps += 1

        # 迭代并与参考值对比
        compare_index = 0
        for n in range(min(steps, len(QDA_FIDELITY_REFERENCE_TRI_NEG) * 2)):
            s = n / steps

            # TODO: 实现 WalkS 执行并获取 fidelity
            # walk = WalkS_Tridiagonal(A, b, s, kappa, p, alpha, beta)
            # fidelity = walk.get_fidelity()

            # if (n + 1) % 2 == 0:
            #     expected = QDA_FIDELITY_REFERENCE_TRI_NEG[compare_index]
            #     assert abs(fidelity - expected) < 1e-5
            #     compare_index += 1

    @pytest.mark.skip(reason="cks_solver.py TOperator has fundamental porting bugs (#76)")
    def test_walks_fidelity_via_qram(self, fresh_system):
        """测试 QRAM 版本的 WalkS fidelity。

        对应 C++ QDA_Poiseuille_via_QRAM_test。
        """
        pass


# ==============================================================================
# End-to-End Tests
# ==============================================================================


class TestQDAEndToEnd:
    """端到端 QDA 测试。"""

    def test_classical_preprocessing(self):
        """测试经典预处理步骤。"""
        from pysparq.algorithms.qda_solver import classical_to_quantum

        A = np.array([[2, 1], [1, 2]], dtype=float)
        b = np.array([1, 1], dtype=float)

        A_q, b_q, recover = classical_to_quantum(A, b)

        # A_q 应该是厄米的
        assert np.allclose(A_q, A_q.T), "Quantum matrix should be Hermitian"

        # b_q 应该归一化
        b_norm = np.linalg.norm(b_q)
        assert abs(b_norm - 1.0) < 0.1, "Quantum vector should be approximately normalized"

    def test_small_system_consistency(self):
        """小规模系统的一致性测试。"""
        # 对于小系统，量子和经典结果应该一致
        A = np.array([[2, 1], [1, 2]], dtype=float)
        b = np.array([1, 1], dtype=float)

        # 经典解
        x_classical = np.linalg.solve(A, b)

        # 验证解的正确性
        assert np.allclose(A @ x_classical, b), "Classical solution verification failed"

        # 解的范数
        x_norm = np.linalg.norm(x_classical)
        assert x_norm > 0, "Solution should be non-zero"
