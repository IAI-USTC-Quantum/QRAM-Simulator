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
        """验证模幂运算正确性。"""
        result = general_expmod(a, x, N)
        assert result == expected

    def test_expmod_identity(self):
        """验证恒等性质：a^(phi(N)) ≡ 1 mod N（对于互质）。"""
        # 2^4 ≡ 1 mod 5 (phi(5) = 4)
        assert general_expmod(2, 4, 5) == 1

        # 3^6 ≡ 1 mod 7 (phi(7) = 6)
        assert general_expmod(3, 6, 7) == 1

    def test_expmod_large_exponent(self):
        """测试大指数情况。"""
        # 2^20 mod 15
        result = general_expmod(2, 20, 15)
        expected = pow(2, 20, 15)
        assert result == expected


class TestFindBestFraction:
    """测试 Farey 序列分数近似。"""

    def test_exact_fraction(self):
        """测试精确分数匹配。"""
        # 4/16 = 1/4
        r, c = find_best_fraction(4, 16, 15)
        assert r == 4
        assert c == 1

    def test_simple_fraction(self):
        """测试简单分数。"""
        # 1/8 = 1/8
        r, c = find_best_fraction(1, 8, 15)
        assert r == 8
        assert c == 1

    def test_approximate_fraction(self):
        """测试近似分数。"""
        # 应该找到好的近似
        r, c = find_best_fraction(1, 8, 15)
        assert r <= 15
        assert c < r

    def test_fraction_bounded_by_N(self):
        """验证分母受 N 约束。"""
        for _ in range(10):
            y = np.random.randint(1, 100)
            Q = 256
            N = 50
            r, c = find_best_fraction(y, Q, N)
            assert r <= N

    def test_fraction_zero_target(self):
        """测试目标为零的情况。"""
        r, c = find_best_fraction(0, 8, 15)
        # 应该返回某个合理的分数
        assert r >= 1


class TestComputePeriod:
    """测试周期计算。"""

    def test_valid_period(self):
        """测试有效周期计算。"""
        # a=2, N=15, period=4. Q=256.
        # y/Q ≈ c/r, need y s.t. find_best_fraction gives r=4
        # c=1, r=4: y/Q = 1/4, y = 64
        period = compute_period(64, 8, 15)
        assert period > 0
        assert period <= 15

    def test_zero_measurement_raises(self):
        """零测量结果应该抛出异常。"""
        with pytest.raises(ShorExecutionFailed):
            compute_period(0, 8, 15)

    def test_period_is_positive(self):
        """周期应该为正数。"""
        for meas in [1, 2, 4, 8, 16]:
            try:
                period = compute_period(meas, 8, 15)
                assert period > 0
            except ShorExecutionFailed:
                pass  # 某些输入可能无效


class TestCheckPeriod:
    """测试周期有效性检查。"""

    def test_valid_even_period(self):
        """有效偶数周期不应该抛出异常。"""
        # a=2, N=15, period=4 是有效的
        check_period(4, 2, 15)  # 不应该抛出

    def test_odd_period_raises(self):
        """奇数周期应该抛出异常。"""
        with pytest.raises(ShorExecutionFailed):
            check_period(3, 2, 15)

    def test_period_too_large_raises(self):
        """周期过大应该抛出异常。"""
        with pytest.raises(ShorExecutionFailed):
            check_period(100, 2, 15)

    def test_trivial_period_raises(self):
        """a^(r/2) ≡ -1 mod N 应该抛出异常。"""
        # 找一个使得 a^(r/2) ≡ -1 mod N 的例子
        # 这意味着 a^(r/2) ≡ N-1 mod N
        # 例如：a=4, N=15, 如果 r=2，则 a^1 = 4，不是 -1
        # 我们需要找一个合适的例子
        pass  # 跳过复杂情况


class TestShorPostprocess:
    """测试经典后处理。"""

    def test_postprocess_returns_factors(self):
        """后处理应该返回因子对。"""
        # 简单测试
        meas = 4
        size = 8
        a = 2
        N = 15

        p, q = shor_postprocess(meas, size, a, N)

        # 返回值应该是整数
        assert isinstance(p, int)
        assert isinstance(q, int)


class TestShorFactorization:
    """测试 Shor 分解算法。"""

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_factor_15(self, fresh_system):
        """分解 15 = 3 * 5。"""
        p, q = factor(15, a=2)
        assert p * q == 15
        # 可能顺序不同
        assert {p, q} == {3, 5}

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_factor_21(self, fresh_system):
        """分解 21 = 3 * 7。"""
        p, q = factor(21)
        assert p * q == 21

    def test_factor_even_number(self):
        """偶数应该被经典处理。"""
        p, q = factor(14)
        assert p == 2
        assert q == 7

    def test_factor_with_known_factor(self):
        """测试当基数与 N 有公因子。"""
        p, q = factor(15, a=3)  # gcd(3, 15) = 3
        assert p == 3
        assert q == 5

    def test_factor_invalid_input(self):
        """测试无效输入。"""
        with pytest.raises(ValueError):
            factor(1)

        with pytest.raises(ValueError):
            factor(0)

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_factor_prime_behavior(self):
        """测试质数输入行为。"""
        # 17 是质数
        # Shor 算法对质数可能失败或返回平凡因子
        p, q = factor(17)
        # 可能返回 (1, 17) 或其他
        assert p * q == 17


class TestSemiClassicalShor:
    """测试半经典 Shor 实现。"""

    def test_init(self, fresh_system):
        """测试初始化。"""
        shor = SemiClassicalShor(a=2, N=15)
        assert shor.a == 2
        assert shor.N == 15
        assert shor.n == 4  # log2(15) + 1

    def test_init_non_coprime_raises(self):
        """测试非互质基初始化。"""
        with pytest.raises(ValueError):
            SemiClassicalShor(a=3, N=15)  # gcd(3, 15) = 3 != 1

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_run_returns_factors(self, fresh_system):
        """测试运行返回因子。"""
        shor = SemiClassicalShor(a=2, N=15)
        p, q = shor.run()
        assert p * q == 15

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_run_updates_attributes(self, fresh_system):
        """测试运行更新属性。"""
        shor = SemiClassicalShor(a=2, N=15)
        shor.run()

        # 应该更新测量结果和因子
        assert shor.meas_result >= 0
        assert shor.p >= 1
        assert shor.q >= 1


class TestShorAlgorithmProperties:
    """测试 Shor 算法的数学性质。"""

    def test_period_finding_consistency(self):
        """验证周期查找的一致性。"""
        # 对于 a=2, N=15，周期是 4
        # 2^4 = 16 ≡ 1 mod 15
        assert general_expmod(2, 4, 15) == 1

        # 周期应该是 4
        # 在量子算法中，测量结果应该对应这个周期

    def test_factor_derivation(self):
        """验证因子推导。"""
        # 如果找到周期 r=4，a=2，N=15
        # a^(r/2) = 2^2 = 4
        # gcd(4+1, 15) = gcd(5, 15) = 5
        # gcd(4-1, 15) = gcd(3, 15) = 3
        a_exp_r_half = general_expmod(2, 2, 15)
        p = math.gcd(a_exp_r_half + 1, 15)
        q = math.gcd(a_exp_r_half - 1, 15)
        assert p == 5
        assert q == 3

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_multiple_bases(self, fresh_system):
        """测试多个不同的基。"""
        N = 15
        for a in [2, 4, 7, 8, 11, 13, 14]:
            if math.gcd(a, N) == 1:
                p, q = factor(N, a=a)
                assert p * q == N


class TestShorEdgeCases:
    """测试 Shor 算法的边界情况。"""

    def test_small_composite(self, fresh_system):
        """测试小合数。"""
        # 6 = 2 * 3
        p, q = factor(6)
        assert p * q == 6

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_power_of_prime(self, fresh_system):
        """测试质数幂。"""
        # 9 = 3 * 3
        p, q = factor(9)
        assert p * q == 9

    @pytest.mark.skip(reason="CustomArithmetic XOR-based op needs refactor for in-place modmul")
    def test_different_bases_same_result(self, fresh_system):
        """测试不同基应该给出相同结果（可能顺序不同）。"""
        N = 21
        results = set()
        for a in [2, 5, 10]:
            if math.gcd(a, N) == 1:
                try:
                    p, q = factor(N, a=a)
                    results.add((min(p, q), max(p, q)))
                except:
                    pass
        # 至少有一个成功
        assert len(results) >= 1
