"""
算法专用 fixtures 和 helper 函数。

提供：
- tridiagonal_matrix: 创建三对角矩阵
- random_unitary: 创建随机酉矩阵
- assert_probability_distribution: 验证概率分布
"""

import pytest
import numpy as np


@pytest.fixture
def tridiagonal_matrix():
    """创建三对角矩阵用于块编码测试。

    Returns:
        Callable: 接受 (alpha, beta, dim) 参数，返回归一化的三对角矩阵
    """

    def _create(alpha: float, beta: float, dim: int) -> np.ndarray:
        mat = np.zeros((dim, dim))
        for i in range(dim):
            mat[i, i] = alpha
            if i > 0:
                mat[i - 1, i] = beta
            if i < dim - 1:
                mat[i + 1, i] = beta
        # 归一化
        norm = np.linalg.norm(mat, "fro")
        if norm > 0:
            mat = mat / norm
        return mat

    return _create


@pytest.fixture
def random_unitary():
    """生成随机酉矩阵用于测试。

    Returns:
        Callable: 接受 (dim, seed) 参数，返回酉矩阵
    """

    def _create(dim: int, seed: int = 42) -> np.ndarray:
        np.random.seed(seed)
        z = np.random.randn(dim, dim) + 1j * np.random.randn(dim, dim)
        q, r = np.linalg.qr(z)
        return q

    return _create


@pytest.fixture
def simple_linear_system():
    """创建简单线性系统用于求解器测试。

    Returns:
        Callable: 接受 (n, kappa) 参数，返回 (A, b, x_expected)
    """

    def _create(n: int = 2, kappa: float = 2.0) -> tuple:
        # 创建条件数为 kappa 的对称正定矩阵
        # A = I + (kappa-1)/n * ones
        # 这样条件数约为 kappa
        A = np.eye(n) + (kappa - 1) / n * np.ones((n, n))
        b = np.ones(n)
        x_expected = np.linalg.solve(A, b)
        return A, b, x_expected

    return _create


@pytest.fixture
def fixed_seed_system(fresh_system):
    """Fresh system with fixed random seed for deterministic regression tests.

    Mirrors the C++ ``random_engine::set_seed(seed)`` pattern from
    test/CPUTest/CommonTest/CorrectnessTest_Common.inl.

    All quantum simulation in the returned context uses np.random.seed(42),
    making tests reproducible across runs.  Resets the C++ SparseState system
    between tests via the ``fresh_system`` fixture from the root conftest.py.
    """
    import pysparq as ps

    np.random.seed(42)
    yield fresh_system
    ps.System.clear()
    np.random.seed(None)  # Restore random state
