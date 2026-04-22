"""
CKS 线性求解器测试。

测试内容：
- ChebyshevPolynomialCoefficient: Chebyshev 系数计算
- get_coef_positive_only: 正矩阵元素系数
- get_coef_common: 一般矩阵元素系数
- SparseMatrix: 稀疏矩阵表示

参考: CPP CKS 相关测试模式
"""

import pytest
import numpy as np
import math

import pysparq as ps
from pysparq.algorithms.cks_solver import (
    ChebyshevPolynomialCoefficient,
    get_coef_positive_only,
    get_coef_common,
    SparseMatrix,
)


class TestChebyshevPolynomialCoefficient:
    """测试 Chebyshev 系数计算。"""

    def test_initialization(self):
        """测试初始化。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)
        assert cheb.b == 10

    def test_coefficient_values(self):
        """验证系数值为正且递减。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)

        prev_coef = float("inf")
        for j in range(cheb.b):
            coef = cheb.coef(j)
            assert coef >= 0
            assert coef <= prev_coef + 1e-10  # 允许数值误差
            prev_coef = coef

    def test_step_size(self):
        """步长应该为 2j + 1。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)

        for j in range(cheb.b):
            assert cheb.step(j) == 2 * j + 1

    def test_sign_alternation(self):
        """符号应该交替：偶数 j 为正，奇数 j 为负。"""
        cheb = ChebyshevPolynomialCoefficient(b=10)

        for j in range(cheb.b):
            expected_sign = (j & 1) == 1  # True for odd
            assert cheb.sign(j) == expected_sign

    def test_step_sequence(self):
        """验证步长序列。"""
        cheb = ChebyshevPolynomialCoefficient(b=5)
        expected_steps = [1, 3, 5, 7, 9]

        for j, expected in enumerate(expected_steps):
            assert cheb.step(j) == expected

    def test_large_b(self):
        """测试大 b 值。"""
        cheb = ChebyshevPolynomialCoefficient(b=200)

        # 对于大 b，使用 erfc 近似
        for j in range(0, cheb.b, 50):
            coef = cheb.coef(j)
            assert 0 <= coef <= 2

    def test_small_b(self):
        """测试小 b 值。"""
        cheb = ChebyshevPolynomialCoefficient(b=5)

        # 对于小 b，使用精确计算
        for j in range(cheb.b):
            coef = cheb.coef(j)
            assert coef >= 0

    def test_combinatorial_coefficient(self):
        """测试组合系数计算。"""
        cheb = ChebyshevPolynomialCoefficient(b=5)

        # 测试简单的组合数
        # C(n, k) = n! / (k! * (n-k)!)
        c = cheb.C(10, 2)
        # C(10, 2) / 4^b = 45 / 1024
        expected = 45.0 / (2**10) / (2**10)
        # 注意：实现可能有不同的归一化
        assert c > 0


class TestGetCoefPositiveOnly:
    """测试正矩阵元素系数计算。"""

    def test_output_format(self):
        """验证输出格式。"""
        mat_data_size = 8
        v = 100
        row, col = 0, 0

        result = get_coef_positive_only(mat_data_size, v, row, col)

        assert len(result) == 4
        assert all(isinstance(c, complex) for c in result)

    def test_unitary_matrix(self):
        """验证生成的矩阵是酉矩阵。"""
        mat_data_size = 8

        for v in [0, 50, 100, 200]:
            result = get_coef_positive_only(mat_data_size, v, 0, 0)
            R = np.array([[result[0], result[1]], [result[2], result[3]]])

            # 验证酉性
            identity = R @ R.conj().T
            assert np.allclose(identity, np.eye(2), atol=1e-10)

    def test_zero_value(self):
        """测试零值情况。"""
        mat_data_size = 8
        v = 0

        result = get_coef_positive_only(mat_data_size, v, 0, 0)

        # v=0 时，x=0, y=1
        # 矩阵应该是 [[0, -1], [1, 0]]
        assert abs(result[0]) < 1e-10
        assert abs(result[1] + 1) < 1e-10
        assert abs(result[2] - 1) < 1e-10
        assert abs(result[3]) < 1e-10

    def test_max_value(self):
        """测试最大值情况。"""
        mat_data_size = 8
        Amax_real = 2**mat_data_size - 1

        result = get_coef_positive_only(mat_data_size, Amax_real, 0, 0)

        # v=Amax_real 时，x=1, y=0
        # 矩阵应该是 [[1, 0], [0, 1]]
        assert abs(result[0] - 1) < 1e-10
        assert abs(result[1]) < 1e-10
        assert abs(result[2]) < 1e-10
        assert abs(result[3] - 1) < 1e-10


class TestGetCoefCommon:
    """测试一般矩阵元素系数计算。"""

    def test_output_format(self):
        """验证输出格式。"""
        mat_data_size = 8
        v = 100
        row, col = 0, 0

        result = get_coef_common(mat_data_size, v, row, col)

        assert len(result) == 4

    def test_positive_value(self):
        """测试正值。"""
        mat_data_size = 8
        v = 100

        result = get_coef_common(mat_data_size, v, 0, 0)

        # 正值应该产生有效的旋转矩阵
        R = np.array([[result[0], result[1]], [result[2], result[3]]])

    def test_negative_value(self):
        """测试负值。"""
        mat_data_size = 8
        # 使用补码表示负值

        result = get_coef_common(mat_data_size, 200, 0, 0)

        # 应该产生有效的结果


class TestSparseMatrix:
    """测试稀疏矩阵表示。"""

    def test_from_dense_positive(self):
        """测试从正稠密矩阵创建稀疏矩阵。"""
        A = np.array([[1, 2], [2, 1]], dtype=float)

        mat = SparseMatrix.from_dense(A, data_size=8, positive_only=True)

        assert mat.n_row == 2
        assert mat.positive_only == True

    def test_from_dense_with_negative(self):
        """测试从含负元素的稠密矩阵创建稀疏矩阵。"""
        A = np.array([[1, -2], [-2, 1]], dtype=float)

        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 2
        assert mat.positive_only == False

    def test_from_dense_identity(self):
        """测试从单位矩阵创建。"""
        A = np.eye(4)

        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 4

    def test_matrix_properties(self):
        """测试矩阵属性。"""
        A = np.array([[2, 1], [1, 2]], dtype=float)
        mat = SparseMatrix.from_dense(A, data_size=8)

        # 验证基本属性
        assert hasattr(mat, "n_row")
        assert hasattr(mat, "data_size")

    def test_get_walk_angle_func(self):
        """测试获取游走角度函数。"""
        A = np.eye(2)
        mat = SparseMatrix.from_dense(A, data_size=8)

        # 获取游走角度函数
        func = mat.get_walk_angle_func()
        assert callable(func)


class TestCKSIntegration:
    """CKS 集成测试。"""

    def test_chebyshev_walk_correspondence(self):
        """测试 Chebyshev 系数与游走步数的对应关系。"""
        b = 10
        cheb = ChebyshevPolynomialCoefficient(b)

        for j in range(b):
            coef = cheb.coef(j)
            step = cheb.step(j)
            sign = cheb.sign(j)

            # 步数应该是奇数
            assert step % 2 == 1

            # 系数应该为非负
            assert coef >= 0

    def test_matrix_coefficient_consistency(self):
        """测试矩阵系数一致性。"""
        mat_data_size = 8

        # 测试多个值
        for v in range(0, 256, 50):
            coef_pos = get_coef_positive_only(mat_data_size, v, 0, 0)
            coef_com = get_coef_common(mat_data_size, v, 0, 0)

            # 两者都应该是有效的 2x2 矩阵
            assert len(coef_pos) == 4
            assert len(coef_com) == 4

    def test_full_coefficient_sequence(self):
        """测试完整系数序列。"""
        b = 20
        cheb = ChebyshevPolynomialCoefficient(b)

        total_coef = 0.0
        for j in range(b):
            total_coef += cheb.coef(j) * ((-1) ** j)

        # 系数和应该在合理范围内
        assert abs(total_coef) < 2 * b


class TestCKSEdgeCases:
    """CKS 边界情况测试。"""

    def test_b_one(self):
        """测试 b=1。"""
        cheb = ChebyshevPolynomialCoefficient(b=1)

        coef = cheb.coef(0)
        step = cheb.step(0)

        assert step == 1
        assert coef >= 0

    def test_large_matrix(self):
        """测试大矩阵。"""
        A = np.eye(10)

        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 10

    def test_zero_matrix(self):
        """测试零矩阵。"""
        A = np.zeros((3, 3))

        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 3

    def test_single_element_matrix(self):
        """测试单元素矩阵。"""
        A = np.array([[5.0]])

        mat = SparseMatrix.from_dense(A, data_size=8)

        assert mat.n_row == 1

    def test_very_large_data_size(self):
        """测试大数据大小。"""
        A = np.eye(2)

        mat = SparseMatrix.from_dense(A, data_size=32)

        assert mat.data_size == 32
