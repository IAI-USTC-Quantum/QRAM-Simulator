"""
Shor 分解算法测试。

测试内容：
- general_expmod: 模幂运算
- find_best_fraction: Farey 序列分数近似
- compute_period: 从测量结果计算周期
- check_period: 周期有效性检查
- factor: 端到端分解功能

参考: CPP examples 中的 Shor 模式
"""

import pytest
import math
import numpy as np

import pysparq as ps
from pysparq.algorithms.shor import (
    general_expmod,
    find_best_fraction,
    compute_period,
    check_period,
    shor_postprocess,
    factor,
    SemiClassicalShor,
    ShorExecutionFailed,
)


class TestGeneralExpmod:
    """测试模幂运算。"""

    @pytest.mark.parametrize(
        "a,x,N,expected",
        [
            (2, 10, 15, 4),  # 2^10 mod 15 = 1024 mod 15 = 4
            (7, 3, 15, 13),  # 7^3 mod 15 = 343 mod 15 = 13
            (2, 0, 15, 1),  # a^0 = 1
            (5, 1, 21, 5),  # a^1 = a
            (3, 4, 10, 1),  # 3^4 = 81 mod 10 = 1
            (2, 8, 17, 1),  # 2^8 = 256 mod 17 = 1
        ],
    )
    def test_expmod_values(self, a, x, N, expected):
        result = general_expmod(a, x, N)
        assert result == expected

    def test_expmod_identity(self):
        assert general_expmod(2, 4, 5) == 1
        assert general_expmod(3, 6, 7) == 1

    def test_expmod_large_exponent(self):
        result = general_expmod(2, 20, 15)
        expected = pow(2, 20, 15)
        assert result == expected


class TestFindBestFraction:
    """测试连分数分数近似。"""

    def test_exact_fraction(self):
        # 64/256 = 1/4
        r, c = find_best_fraction(64, 256, 15)
        assert r == 4
        assert c == 1

    def test_simple_fraction(self):
        # 1/8
        r, c = find_best_fraction(1, 8, 15)
        assert r == 8
        assert c == 1

    def test_approximate_fraction(self):
        r, c = find_best_fraction(1, 8, 15)
        assert r <= 15
        assert c < r

    def test_fraction_bounded_by_N(self):
        for _ in range(10):
            y = np.random.randint(1, 100)
            Q = 256
            N = 50
            r, c = find_best_fraction(y, Q, N)
            assert r <= N

    def test_fraction_zero_target(self):
        r, c = find_best_fraction(0, 8, 15)
        # 0/Q = 0 → convergent 0/1
        assert r >= 1

    def test_fraction_3_over_4(self):
        # 192/256 = 3/4
        r, c = find_best_fraction(192, 256, 15)
        assert r == 4
        assert c == 3


class TestComputePeriod:
    """测试周期计算。"""

    def test_valid_period_from_measurement(self):
        # For a=2, N=15, period r=4: measurement y = 64 (c=1)
        # y=64, Q=256, y/Q = 1/4
        period = compute_period(64, 8, 15)
        assert period == 4

    def test_period_from_another_measurement(self):
        # y=192, Q=256, y/Q = 3/4 → r=4
        period = compute_period(192, 8, 15)
        assert period == 4

    def test_zero_measurement_raises(self):
        with pytest.raises(ShorExecutionFailed):
            compute_period(0, 8, 15)

    def test_period_is_positive(self):
        for meas in [64, 128, 192]:
            try:
                period = compute_period(meas, 8, 15)
                assert period > 0
            except ShorExecutionFailed:
                pass


class TestCheckPeriod:
    """测试周期有效性检查。"""

    def test_valid_even_period(self):
        check_period(4, 2, 15)  # a=2, r=4 valid

    def test_odd_period_raises(self):
        with pytest.raises(ShorExecutionFailed):
            check_period(3, 2, 15)

    def test_period_too_large_raises(self):
        with pytest.raises(ShorExecutionFailed):
            check_period(100, 2, 15)

    def test_trivial_period_raises(self):
        # a^(r/2) = -1 mod N → a=4, N=5, r=2: 4^1=4=(-1 mod 5)
        with pytest.raises(ShorExecutionFailed):
            check_period(2, 4, 5)


class TestShorPostprocess:
    """测试经典后处理。"""

    def test_postprocess_returns_factors(self):
        p, q = shor_postprocess(64, 8, 2, 15)
        assert isinstance(p, int)
        assert isinstance(q, int)
        # For y=64, r=4: a^(r/2)=4, gcd(5,15)=5, gcd(3,15)=3
        assert {p, q} == {3, 5}


class TestShorFactorization:
    """测试 Shor 分解算法。"""

    def test_factor_15(self, fresh_system):
        """分解 15 = 3 * 5。"""
        # Semi-classical Shor result depends on quantum measurement
        p, q = factor(15, a=2)
        assert p * q == 15

    def test_factor_21(self, fresh_system):
        p, q = factor(21)
        assert p * q == 21

    def test_factor_even_number(self):
        p, q = factor(14)
        assert p == 2
        assert q == 7

    def test_factor_with_known_factor(self):
        p, q = factor(15, a=3)
        assert p == 3
        assert q == 5

    def test_factor_invalid_input(self):
        with pytest.raises(ValueError):
            factor(1)
        with pytest.raises(ValueError):
            factor(0)


class TestSemiClassicalShor:
    """测试半经典 Shor 实现。"""

    def test_init(self, fresh_system):
        shor = SemiClassicalShor(a=2, N=15)
        assert shor.a == 2
        assert shor.N == 15
        assert shor.n == 4  # log2(15) + 1

    def test_init_non_coprime_raises(self):
        with pytest.raises(ValueError):
            SemiClassicalShor(a=3, N=15)

    def test_run_returns_factors(self, fresh_system):
        """Semi-classical Shor is probabilistic — may return trivial factors."""
        shor = SemiClassicalShor(a=2, N=15)
        p, q = shor.run()
        # Result is probabilistic; may return (1, 15) or (3, 5) or (5, 3)
        assert p * q == 15 or (p == 1 and q == 1)

    def test_run_updates_attributes(self, fresh_system):
        shor = SemiClassicalShor(a=2, N=15)
        shor.run()
        assert shor.meas_result >= 0
        assert shor.p >= 1
        assert shor.q >= 1


class TestShorAlgorithmProperties:
    """测试 Shor 算法的数学性质。"""

    def test_period_finding_consistency(self):
        assert general_expmod(2, 4, 15) == 1

    def test_factor_derivation(self):
        # r=4, a=2, N=15: a^(r/2) = 2^2 = 4
        a_exp_r_half = general_expmod(2, 2, 15)
        p = math.gcd(a_exp_r_half + 1, 15)
        q = math.gcd(a_exp_r_half - 1, 15)
        assert p == 5
        assert q == 3
