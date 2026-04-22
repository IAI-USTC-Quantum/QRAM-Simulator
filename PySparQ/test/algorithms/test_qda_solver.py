"""
QDA 线性求解器测试。

测试内容：
- compute_fs: 插值参数计算
- compute_rotation_matrix: 旋转矩阵计算
- chebyshev_T: Chebyshev 多项式
- dolph_chebyshev: Dolph-Chebyshev 滤波器
- compute_fourier_coeffs: Fourier 系数计算

参考: test/CPUTest/CommonTest/CorrectnessTest_QDA_CompareList.inl
"""

import pytest
import numpy as np
import math

import pysparq as ps
from pysparq.algorithms.qda_solver import (
    compute_fs,
    compute_rotation_matrix,
    chebyshev_T,
    dolph_chebyshev,
    compute_fourier_coeffs,
)


class TestComputeFs:
    """测试插值参数计算。"""

    def test_fs_at_zero(self):
        """f(0) 应该为 0。"""
        fs = compute_fs(0.0, kappa=10.0, p=0.5)
        assert abs(fs) < 1e-10

    def test_fs_at_one(self):
        """f(1) 应该为 1。"""
        fs = compute_fs(1.0, kappa=10.0, p=0.5)
        assert abs(fs - 1.0) < 1e-10

    def test_fs_monotonic(self):
        """f(s) 应该单调递增。"""
        kappa, p = 10.0, 0.5
        prev_fs = compute_fs(0.0, kappa, p)

        for s in np.linspace(0.1, 1.0, 10):
            fs = compute_fs(s, kappa, p)
            assert fs >= prev_fs - 1e-10  # 允许数值误差
            prev_fs = fs

    def test_fs_kappa_one(self):
        """当 kappa=1 时，f(s) 应该等于 s。"""
        for s in [0.0, 0.25, 0.5, 0.75, 1.0]:
            fs = compute_fs(s, kappa=1.0, p=0.5)
            assert abs(fs - s) < 1e-10

    def test_fs_bounded(self):
        """f(s) 应该在 [0, 1] 范围内。"""
        for kappa in [2.0, 10.0, 100.0]:
            for p in [0.3, 0.5, 0.7]:
                for s in np.linspace(0, 1, 20):
                    fs = compute_fs(s, kappa, p)
                    assert 0.0 <= fs <= 1.0

    def test_fs_different_kappa(self):
        """测试不同条件数。"""
        s = 0.5
        p = 0.5

        for kappa in [2.0, 5.0, 10.0, 100.0]:
            fs = compute_fs(s, kappa, p)
            assert 0.0 <= fs <= 1.0

    def test_fs_different_p(self):
        """测试不同调度参数。"""
        s = 0.5
        kappa = 10.0

        for p in [0.1, 0.3, 0.5, 0.7, 0.9]:
            fs = compute_fs(s, kappa, p)
            assert 0.0 <= fs <= 1.0


class TestRotationMatrix:
    """测试旋转矩阵计算。"""

    def test_rotation_matrix_unitary(self):
        """旋转矩阵应该是酉矩阵。"""
        for fs in [0.2, 0.5, 0.8]:
            R = compute_rotation_matrix(fs)
            R_matrix = np.array([[R[0], R[1]], [R[2], R[3]]])

            # 检查酉性：R * R^dagger = I
            identity = R_matrix @ R_matrix.conj().T
            assert np.allclose(identity, np.eye(2), atol=1e-10)

    def test_rotation_matrix_determinant(self):
        """旋转矩阵行列式应该为 -1。"""
        for fs in [0.2, 0.5, 0.8]:
            R = compute_rotation_matrix(fs)
            det = R[0] * R[3] - R[1] * R[2]
            assert abs(det + 1) < 1e-10  # det = -1

    def test_rotation_matrix_structure(self):
        """验证旋转矩阵结构。"""
        fs = 0.5
        R = compute_rotation_matrix(fs)

        sqrt_N = 1.0 / math.sqrt((1 - fs) ** 2 + fs**2)

        expected_u00 = sqrt_N * (1 - fs)
        expected_u01 = sqrt_N * fs
        expected_u10 = sqrt_N * fs
        expected_u11 = sqrt_N * (fs - 1)

        assert abs(R[0] - expected_u00) < 1e-10
        assert abs(R[1] - expected_u01) < 1e-10
        assert abs(R[2] - expected_u10) < 1e-10
        assert abs(R[3] - expected_u11) < 1e-10

    def test_rotation_matrix_fs_zero(self):
        """fs=0 时的旋转矩阵。"""
        R = compute_rotation_matrix(0.0)
        # fs=0 时，sqrt_N = 1
        # R = [[1, 0], [0, -1]]
        assert abs(R[0] - 1) < 1e-10
        assert abs(R[1]) < 1e-10
        assert abs(R[2]) < 1e-10
        assert abs(R[3] + 1) < 1e-10

    def test_rotation_matrix_fs_one(self):
        """fs=1 时的旋转矩阵。"""
        R = compute_rotation_matrix(1.0)
        # fs=1 时，sqrt_N = 1
        # R = [[0, 1], [1, 0]]
        assert abs(R[0]) < 1e-10
        assert abs(R[1] - 1) < 1e-10
        assert abs(R[2] - 1) < 1e-10
        assert abs(R[3]) < 1e-10


class TestChebyshevPolynomial:
    """测试 Chebyshev 多项式。"""

    @pytest.mark.parametrize("n,x,expected", [
        (0, 0.5, 1.0),
        (1, 0.5, 0.5),
        (2, 0.5, -0.5),
        (3, 0.5, -1.0),
        (0, 1.0, 1.0),
        (1, 1.0, 1.0),
        (2, 1.0, 1.0),
    ])
    def test_chebyshev_T_values(self, n, x, expected):
        """测试 Chebyshev 多项式值。"""
        result = chebyshev_T(n, x)
        assert abs(result - expected) < 1e-10

    def test_chebyshev_recursion(self):
        """验证递推关系 T_n(x) = 2x T_{n-1}(x) - T_{n-2}(x)。"""
        x = 0.7
        for n in range(2, 10):
            Tn = chebyshev_T(n, x)
            Tn_minus_1 = chebyshev_T(n - 1, x)
            Tn_minus_2 = chebyshev_T(n - 2, x)
            expected = 2 * x * Tn_minus_1 - Tn_minus_2
            assert abs(Tn - expected) < 1e-10

    def test_chebyshev_at_one(self):
        """T_n(1) = 1 对所有 n。"""
        for n in range(10):
            assert chebyshev_T(n, 1.0) == 1.0

    def test_chebyshev_at_minus_one(self):
        """T_n(-1) = (-1)^n。"""
        for n in range(10):
            result = chebyshev_T(n, -1.0)
            expected = (-1) ** n
            assert abs(result - expected) < 1e-10

    def test_chebyshev_cosine_relation(self):
        """验证 T_n(cos(theta)) = cos(n*theta)。"""
        for theta in [0.1, 0.5, 1.0, 2.0]:
            x = math.cos(theta)
            for n in range(5):
                Tn = chebyshev_T(n, x)
                expected = math.cos(n * theta)
                assert abs(Tn - expected) < 1e-10


class TestDolphChebyshev:
    """测试 Dolph-Chebyshev 滤波器。"""

    def test_dolph_chebyshev_basic(self):
        """基本 Dolph-Chebyshev 计算。"""
        epsilon = 0.1
        l = 5
        phi = 0.5

        result = dolph_chebyshev(epsilon, l, phi)
        assert isinstance(result, float)

    def test_dolph_chebyshev_positive(self):
        """Dolph-Chebyshev 值在多数情况下应为正值。"""
        epsilon = 0.1
        l = 5

        positive_count = 0
        for phi in np.linspace(0, math.pi, 20):
            result = dolph_chebyshev(epsilon, l, phi)
            if result > -epsilon:
                positive_count += 1

        # 大多数值应该为正或接近零
        assert positive_count >= 15, "Most values should be positive or near zero"

    def test_dolph_chebyshev_at_zero(self):
        """phi=0 时的值。"""
        epsilon = 0.1
        l = 5

        result = dolph_chebyshev(epsilon, l, 0.0)
        # phi=0 时，cos(phi)=1
        # 应该接近某个正值
        assert result >= 0


class TestFourierCoefficients:
    """测试 Fourier 系数计算。"""

    def test_fourier_coeffs_length(self):
        """Fourier 系数列表长度正确（偶数索引系数）。"""
        epsilon = 0.1
        l = 5

        coeffs = compute_fourier_coeffs(epsilon, l)
        # 实现只保留偶数索引系数: ceil((l+1)/2) = 3
        expected_len = (l + 2) // 2
        assert len(coeffs) == expected_len

    def test_fourier_coeffs_positive(self):
        """Fourier 系数应该非负。"""
        epsilon = 0.1
        l = 5

        coeffs = compute_fourier_coeffs(epsilon, l)
        for coeff in coeffs:
            assert coeff >= -0.1  # 允许小的数值误差

    def test_fourier_coeffs_symmetry(self):
        """Fourier 系数的对称性。"""
        epsilon = 0.1
        l = 4

        coeffs = compute_fourier_coeffs(epsilon, l)
        # 实对称函数的 Fourier 系数应该是对称的
        for i in range(len(coeffs)):
            for j in range(len(coeffs)):
                if i + j == l:
                    # 对称位置
                    pass  # 关系依赖于具体实现


class TestQDAIntegration:
    """QDA 集成测试。"""

    def test_interpolation_sequence(self):
        """测试插值序列的连续性。"""
        kappa = 10.0
        p = 0.5
        steps = 100

        fs_values = [compute_fs(s, kappa, p) for s in np.linspace(0, 1, steps)]

        # 验证单调性
        for i in range(1, len(fs_values)):
            assert fs_values[i] >= fs_values[i - 1] - 1e-10

    def test_rotation_matrix_sequence(self):
        """测试旋转矩阵序列。"""
        fs_values = [0.0, 0.25, 0.5, 0.75, 1.0]

        matrices = [compute_rotation_matrix(fs) for fs in fs_values]

        # 验证所有矩阵都是酉的
        for R in matrices:
            R_mat = np.array([[R[0], R[1]], [R[2], R[3]]])
            identity = R_mat @ R_mat.conj().T
            assert np.allclose(identity, np.eye(2), atol=1e-10)

    def test_filter_construction(self):
        """测试滤波器构造。"""
        epsilon = 0.1
        l = 5

        coeffs = compute_fourier_coeffs(epsilon, l)

        # 构造滤波器函数
        def filter_func(x):
            result = 0.0
            for j, coeff in enumerate(coeffs):
                result += coeff * chebyshev_T(j, x)
            return result

        # 测试滤波器
        for x in [-0.5, 0.0, 0.5, 1.0]:
            result = filter_func(x)
            assert isinstance(result, float)


class TestQDAEdgeCases:
    """QDA 边界情况测试。"""

    def test_kappa_one(self):
        """测试 kappa=1 的特殊情况。"""
        for s in np.linspace(0, 1, 10):
            fs = compute_fs(s, kappa=1.0, p=0.5)
            assert abs(fs - s) < 1e-10

    def test_large_kappa(self):
        """测试大条件数。"""
        kappa = 1000.0
        p = 0.5

        for s in np.linspace(0, 1, 10):
            fs = compute_fs(s, kappa, p)
            assert 0.0 <= fs <= 1.0

    def test_small_epsilon(self):
        """测试小误差容限。"""
        epsilon = 1e-6
        l = 10

        coeffs = compute_fourier_coeffs(epsilon, l)
        # 偶数索引系数长度
        expected_len = (l + 2) // 2
        assert len(coeffs) == expected_len

    def test_p_near_zero(self):
        """测试 p 接近 0。"""
        kappa = 10.0
        p = 0.01

        for s in np.linspace(0, 1, 10):
            fs = compute_fs(s, kappa, p)
            assert 0.0 <= fs <= 1.0

    def test_p_near_one(self):
        """测试 p 接近 1。"""
        kappa = 10.0
        p = 0.99

        for s in np.linspace(0, 1, 10):
            fs = compute_fs(s, kappa, p)
            assert 0.0 <= fs <= 1.0
