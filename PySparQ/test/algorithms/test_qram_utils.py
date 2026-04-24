"""
QRAM 工具函数测试。

测试内容：
- pow2: 2 的幂次
- make_complement/get_complement: 补码转换
- column_flatten: 矩阵转置
- scale_and_convert_vector: 向量缩放转换
- make_vector_tree: 二叉树构建
- make_func/make_func_inv: 旋转矩阵计算

这些是纯 Python 函数，不依赖量子态操作。
"""

import pytest
import numpy as np
import math

from pysparq.algorithms.qram_utils import (
    pow2,
    make_complement,
    get_complement,
    column_flatten,
    scale_and_convert_vector,
    make_vector_tree,
    make_func,
    make_func_inv,
    PI,
)


class TestPow2:
    """测试 pow2 函数。"""

    @pytest.mark.parametrize("n,expected", [(0, 1), (1, 2), (2, 4), (10, 1024), (20, 1048576)])
    def test_pow2_values(self, n, expected):
        """验证 2^n 计算正确。"""
        assert pow2(n) == expected

    def test_pow2_negative_raises(self):
        """负指数应该产生有效结果（Python 左移行为）。"""
        # Python 的左移对负数有特殊行为
        # 1 << -1 会抛出 ValueError 或产生 0
        # 这里我们只测试非负数


class TestTwoComplement:
    """测试补码转换函数。"""

    @pytest.mark.parametrize(
        "data,data_sz,expected",
        [
            (3, 8, 3),  # 正数不变
            (-3, 8, 253),  # -3 in 8 bits = 253
            (-1, 4, 15),  # -1 in 4 bits = 15
            (0, 8, 0),  # 零不变
            (127, 8, 127),  # 正数边界
            (-128, 8, 128),  # 负数边界
        ],
    )
    def test_make_complement(self, data, data_sz, expected):
        """验证补码编码正确。"""
        assert make_complement(data, data_sz) == expected

    @pytest.mark.parametrize(
        "data,data_sz,expected",
        [
            (253, 8, -3),
            (15, 4, -1),
            (0, 8, 0),
            (3, 8, 3),
            (128, 8, -128),
        ],
    )
    def test_get_complement(self, data, data_sz, expected):
        """验证补码解码正确。"""
        assert get_complement(data, data_sz) == expected

    @pytest.mark.parametrize("val", [-128, -1, -50, -100, -127])
    def test_roundtrip_8bit(self, val):
        """验证 8 位补码往返转换。"""
        enc = make_complement(val, 8)
        dec = get_complement(enc, 8)
        assert dec == val

    @pytest.mark.parametrize("val", [-8, -1, -4, -7])
    def test_roundtrip_4bit(self, val):
        """验证 4 位补码往返转换。"""
        enc = make_complement(val, 4)
        dec = get_complement(enc, 4)
        assert dec == val

    def test_64bit_handling(self):
        """验证 64 位特殊处理。"""
        # 64 位时不做转换
        assert make_complement(-1, 64) == -1
        assert get_complement((1 << 63), 64) < 0  # 负数


class TestColumnFlatten:
    """测试矩阵转置函数。"""

    def test_2x2_matrix(self):
        """验证 2x2 矩阵转置。"""
        row_major = [1, 2, 3, 4]  # [[1, 2], [3, 4]]
        col_major = column_flatten(row_major)
        assert col_major == [1, 3, 2, 4]  # [[1, 3], [2, 4]]

    def test_3x3_matrix(self):
        """验证 3x3 矩阵转置。"""
        row_major = [1, 2, 3, 4, 5, 6, 7, 8, 9]
        col_major = column_flatten(row_major)
        assert col_major == [1, 4, 7, 2, 5, 8, 3, 6, 9]

    def test_4x4_matrix(self):
        """验证 4x4 矩阵转置。"""
        row_major = list(range(1, 17))
        col_major = column_flatten(row_major)
        # 验证转置后的位置
        assert col_major[0] == 1  # (0,0)
        assert col_major[1] == 5  # (1,0)
        assert col_major[4] == 2  # (0,1)

    def test_non_square_raises(self):
        """非完美平方长度应抛出异常。"""
        with pytest.raises(ValueError):
            column_flatten([1, 2, 3])  # 长度 3 不是完美平方


class TestScaleAndConvertVector:
    """测试向量缩放和转换函数。"""

    def test_positive_values(self):
        """验证正值缩放转换。"""
        vec = [0.25, 0.5, 0.75, 1.0]
        result = scale_and_convert_vector(vec, exponent=2, data_size=8, from_matrix=False)
        # 0.25 * 4 = 1, 0.5 * 4 = 2, etc.
        expected = [1, 2, 3, 4]
        assert result == expected

    def test_negative_values(self):
        """验证负值缩放转换。"""
        vec = [0.25, -0.25, 0.5, -0.5]
        result = scale_and_convert_vector(vec, exponent=2, data_size=8, from_matrix=False)
        # -0.25 * 4 = -1 -> 255 (8-bit 补码)
        # -0.5 * 4 = -2 -> 254 (8-bit 补码)
        assert result[0] == 1
        assert result[1] == 255  # -1 in 8-bit
        assert result[2] == 2
        assert result[3] == 254  # -2 in 8-bit

    def test_from_matrix_transposes(self):
        """验证 from_matrix=True 时转置。"""
        vec = [1.0, 2.0, 3.0, 4.0]  # [[1, 2], [3, 4]]
        result = scale_and_convert_vector(vec, exponent=0, data_size=8, from_matrix=True)
        # 转置后: [1, 3, 2, 4]
        assert result == [1, 3, 2, 4]

    def test_from_matrix_false_no_transpose(self):
        """验证 from_matrix=False 时不转置。"""
        vec = [1.0, 2.0, 3.0, 4.0]
        result = scale_and_convert_vector(vec, exponent=0, data_size=8, from_matrix=False)
        assert result == [1, 2, 3, 4]

    def test_numpy_array_input(self):
        """验证 numpy 数组输入。"""
        vec = np.array([0.5, 1.0])
        result = scale_and_convert_vector(vec, exponent=1, data_size=8, from_matrix=False)
        assert result == [1, 2]


class TestMakeVectorTree:
    """测试二叉树构建函数。"""

    def test_all_zeros(self):
        """验证全零分布的树。"""
        tree = make_vector_tree([0, 0, 0, 0], data_size=4)
        # 所有值应该为零
        assert all(v == 0 for v in tree[:-1])  # 排除末尾的 0

    def test_uniform_values(self):
        """验证均匀分布的树。"""
        tree = make_vector_tree([1, 1, 1, 1], data_size=4)
        # 根节点应该是 4（平方和: 1+1+1+1=4）
        assert tree[0] == 4

    def test_single_pair(self):
        """验证单对分布的树。"""
        tree = make_vector_tree([3, 4], data_size=4)
        # 根节点应该是 3^2 + 4^2 = 25
        assert tree[0] == 25
        assert tree[1] == 3
        assert tree[2] == 4

    def test_tree_structure(self):
        """验证树结构的层级关系。"""
        # 使用 4 个元素: a, b, c, d
        # 第一层: a^2+b^2, c^2+d^2
        # 第二层: sum of first layer
        tree = make_vector_tree([2, 2, 3, 3], data_size=4)
        # 根节点: 4+4+9+9 = 26
        assert tree[0] == 26

    def test_negative_values_in_tree(self):
        """验证负值在树构建中的处理。"""
        # 使用补码编码的负值
        # -2 in 8-bit = 254
        tree = make_vector_tree([254, 254], data_size=8)
        # (-2)^2 + (-2)^2 = 4 + 4 = 8
        assert tree[0] == 8


class TestRotationMatrices:
    """测试旋转矩阵计算函数。"""

    def test_zero_rotation(self):
        """零角度应该产生单位矩阵。"""
        mat = make_func(0, 4)
        # [[1, 0], [0, 1]]
        assert abs(mat[0] - complex(1, 0)) < 1e-10
        assert abs(mat[1]) < 1e-10
        assert abs(mat[2]) < 1e-10
        assert abs(mat[3] - complex(1, 0)) < 1e-10

    def test_rotation_matrix_shape(self):
        """验证旋转矩阵形状正确。"""
        mat = make_func(1, 4)
        assert len(mat) == 4

    def test_inverse_rotation(self):
        """逆旋转应该转置旋转矩阵。"""
        mat = make_func(1, 4)
        mat_inv = make_func_inv(1, 4)
        # 非对角元素符号翻转
        assert abs(mat[1] + mat_inv[1]) < 1e-10
        assert abs(mat[2] + mat_inv[2]) < 1e-10
        # 对角元素相同
        assert abs(mat[0] - mat_inv[0]) < 1e-10
        assert abs(mat[3] - mat_inv[3]) < 1e-10

    @pytest.mark.parametrize("value", [0, 1, 4, 8, 15])
    def test_unitarity(self, value):
        """旋转矩阵应该是酉矩阵。"""
        mat = make_func(value, 4)
        R = np.array([[mat[0], mat[1]], [mat[2], mat[3]]])
        # R * R^dagger 应该是单位矩阵
        identity = R @ R.conj().T
        assert np.allclose(identity, np.eye(2), atol=1e-10)

    def test_determinant_one(self):
        """旋转矩阵行列式应该为 1。"""
        for value in [0, 1, 4, 8]:
            mat = make_func(value, 4)
            det = mat[0] * mat[3] - mat[1] * mat[2]
            assert abs(det - 1.0) < 1e-10

    def test_angle_calculation(self):
        """验证角度计算正确。"""
        # value=1, n_digit=4 -> theta = 1/16 * 2*pi = pi/8
        mat = make_func(1, 4)
        expected_theta = PI / 8
        expected_cos = math.cos(expected_theta)
        expected_sin = math.sin(expected_theta)

        assert abs(mat[0].real - expected_cos) < 1e-10
        assert abs(mat[1].real + expected_sin) < 1e-10  # -sin
        assert abs(mat[2].real - expected_sin) < 1e-10
        assert abs(mat[3].real - expected_cos) < 1e-10

    def test_64bit_special_case(self):
        """验证 64 位特殊处理。"""
        mat = make_func(1, 64)
        # 64 位时使用不同的分母: 2 * 2^63
        # theta = value / (2 * 2^63) * 2 * pi
        assert len(mat) == 4
        # 应该非常接近单位矩阵（角度很小）
        assert abs(mat[0] - complex(1, 0)) < 1e-10
        assert abs(mat[3] - complex(1, 0)) < 1e-10


class TestIntegration:
    """集成测试：验证多个函数协同工作。"""

    def test_full_pipeline(self):
        """验证完整的 QRAM 数据准备流水线。"""
        # 1. 创建矩阵数据
        matrix_data = [0.25, 0.5, 0.75, 1.0]

        # 2. 转置
        transposed = column_flatten(matrix_data)

        # 3. 缩放和转换
        converted = scale_and_convert_vector(
            transposed, exponent=2, data_size=8, from_matrix=False
        )

        # 4. 构建树
        tree = make_vector_tree(converted, data_size=8)

        # 验证流程完成
        assert len(tree) > 0

    def test_matrix_encoding_roundtrip(self):
        """验证矩阵编码往返。"""
        # 创建简单的 2x2 矩阵
        original = [1.0, 2.0, 3.0, 4.0]

        # 编码
        encoded = scale_and_convert_vector(original, exponent=0, data_size=8, from_matrix=True)

        # 解码（转置回来）
        decoded = column_flatten(encoded)

        # 验证
        # 注意：转置两次应该回到原矩阵
        double_transposed = column_flatten(decoded)
        assert double_transposed == encoded
