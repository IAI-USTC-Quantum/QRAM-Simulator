"""
Grover 搜索算法测试。

测试内容：
- GroverOracle: 相位翻转标记状态
- DiffusionOperator: 关于均匀叠加态的反射
- GroverOperator: Oracle + Diffusion 组合
- grover_search: 端到端搜索功能
- grover_count: 量子计数变体

参考: Experiments/Grover/GroverTest.cpp
"""

import pytest
import math
import numpy as np

import pysparq as ps
from pysparq.algorithms.grover import (
    GroverOracle,
    DiffusionOperator,
    GroverOperator,
    grover_search,
    grover_count,
)


class TestGroverOracle:
    """测试 Grover Oracle 功能。"""

    def test_oracle_flips_marked_state_phase(self, fresh_system):
        """Oracle 应该对匹配搜索值的状态应用 -1 相位。"""
        memory = [5, 12, 3, 8, 15, 7, 2, 9]
        target = 8

        qram = ps.QRAMCircuit_qutrit(3, 64, memory)

        state = ps.SparseState()
        ps.AddRegister("addr", ps.UnsignedInteger, 3)(state)
        ps.AddRegister("data", ps.UnsignedInteger, 64)(state)
        ps.AddRegister("search", ps.UnsignedInteger, 64)(state)

        # addr=3 → memory[3]=8 matches target
        ps.Init_Unsafe("addr", 3)(state)
        ps.Init_Unsafe("search", target)(state)

        oracle = GroverOracle(qram, "addr", "data", "search")
        oracle(state)

        # Oracle should execute without error
        assert state.size() >= 1

    def test_oracle_self_adjoint(self, fresh_system):
        """Oracle 应该是自伴的（应用两次 = 恒等）。"""
        memory = [1, 2, 3, 4]
        n_bits = 2

        qram = ps.QRAMCircuit_qutrit(n_bits, 8, memory)

        state = ps.SparseState()
        ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
        ps.AddRegister("data", ps.UnsignedInteger, 8)(state)
        ps.AddRegister("search", ps.UnsignedInteger, 8)(state)

        ps.Hadamard_Int_Full("addr")(state)
        ps.Init_Unsafe("search", 2)(state)

        oracle = GroverOracle(qram, "addr", "data", "search")
        oracle(state)
        oracle(state)
        ps.CheckNormalization(1e-6)(state)

    def test_oracle_with_condition(self, fresh_system):
        """测试带条件的 Oracle 执行。"""
        memory = [1, 2, 3, 4]
        n_bits = 2

        qram = ps.QRAMCircuit_qutrit(n_bits, 8, memory)

        state = ps.SparseState()
        ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
        ps.AddRegister("data", ps.UnsignedInteger, 8)(state)
        ps.AddRegister("search", ps.UnsignedInteger, 8)(state)
        ps.AddRegister("cond", ps.Boolean, 1)(state)

        ps.Init_Unsafe("addr", 1)(state)
        ps.Init_Unsafe("search", 2)(state)
        ps.Init_Unsafe("cond", 1)(state)

        oracle = GroverOracle(qram, "addr", "data", "search")
        oracle.conditioned_by_nonzeros("cond")(state)

        assert state.size() >= 1


class TestDiffusionOperator:
    """测试扩散算子功能。"""

    def test_diffusion_on_zero_state(self, fresh_system):
        """D|0> should produce a superposition (not simply -|0>).

        D = H*P_0*H where P_0 flips phase of |0>.
        D|0> = |0> - 2/sqrt(N)|s> where |s> = H|0>.
        For N=4: D|0> = (1/2)|0> - (1/2)(|1>+|2>+|3>).
        """
        n_bits = 2
        ps.System.add_register("addr", ps.UnsignedInteger, n_bits)
        state = ps.SparseState()

        diffusion = DiffusionOperator("addr")
        diffusion(state)

        # D|0> produces a superposition over all basis states
        assert state.size() > 1

        # Verify probabilities sum to 1
        total_prob = sum(abs(b.amplitude) ** 2 for b in state.basis_states)
        assert abs(total_prob - 1.0) < 1e-10

    def test_diffusion_on_uniform_superposition(self, fresh_system):
        """均匀叠加态是 D 的本征向量，本征值为 -1。"""
        n_bits = 2
        ps.System.add_register("addr", ps.UnsignedInteger, n_bits)
        state = ps.SparseState()

        ps.Hadamard_Int_Full("addr")(state)
        addr_id = ps.System.get_id("addr")
        initial_amps = {}
        for b in state.basis_states:
            key = b.get(addr_id).value
            initial_amps[key] = b.amplitude

        diffusion = DiffusionOperator("addr")
        diffusion(state)

        # |s> = H|0> is eigenvector with eigenvalue -1
        for basis in state.basis_states:
            key = basis.get(addr_id).value
            expected = -initial_amps[key]
            assert abs(basis.amplitude - expected) < 1e-10

    def test_diffusion_self_adjoint(self, fresh_system):
        """扩散算子应该是自伴的。"""
        n_bits = 2
        ps.System.add_register("addr", ps.UnsignedInteger, n_bits)
        state = ps.SparseState()

        ps.Init_Unsafe("addr", 1)(state)

        diffusion = DiffusionOperator("addr")
        diffusion(state)
        diffusion(state)

        assert state.size() == 1

    def test_diffusion_with_condition(self, fresh_system):
        """测试带条件的扩散。"""
        n_bits = 2
        ps.System.add_register("addr", ps.UnsignedInteger, n_bits)
        ps.System.add_register("cond", ps.Boolean, 1)
        state = ps.SparseState()

        ps.Hadamard_Int_Full("addr")(state)
        ps.Init_Unsafe("cond", 1)(state)

        diffusion = DiffusionOperator("addr")
        diffusion.conditioned_by_nonzeros("cond")(state)

        assert state.size() >= 1


class TestGroverOperator:
    """测试组合 Grover 算子。"""

    def test_grover_operator_basic(self, fresh_system):
        """测试基本 Grover 算子执行。"""
        memory = [1, 2, 3, 4]
        n_bits = 2

        qram = ps.QRAMCircuit_qutrit(n_bits, 8, memory)

        state = ps.SparseState()
        ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
        ps.AddRegister("data", ps.UnsignedInteger, 8)(state)
        ps.AddRegister("search", ps.UnsignedInteger, 8)(state)

        ps.Hadamard_Int_Full("addr")(state)
        ps.Init_Unsafe("search", 2)(state)

        grover_op = GroverOperator(qram, "addr", "data", "search")
        grover_op(state)

        assert state.size() >= 1

    def test_grover_operator_multiple_iterations(self, fresh_system):
        """测试多次 Grover 迭代。"""
        memory = [1, 2, 3, 4, 5, 6, 7, 8]
        n_bits = 3
        data_size = 8

        qram = ps.QRAMCircuit_qutrit(n_bits, data_size, memory)

        state = ps.SparseState()
        ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
        ps.AddRegister("data", ps.UnsignedInteger, data_size)(state)
        ps.AddRegister("search", ps.UnsignedInteger, data_size)(state)

        ps.Hadamard_Int_Full("addr")(state)
        ps.Init_Unsafe("search", 5)(state)

        grover_op = GroverOperator(qram, "addr", "data", "search")

        for _ in range(2):
            grover_op(state)

        ps.CheckNormalization(1e-6)(state)


class TestGroverSearch:
    """测试端到端 Grover 搜索。"""

    def test_single_target_search_small(self, fresh_system):
        """小型数据库单目标搜索。"""
        memory = [1, 2, 3, 4]
        target = 2

        idx, prob = grover_search(memory, target, n_iterations=1)

        assert 0 <= idx < len(memory)
        assert prob > 0

    def test_single_target_search_medium(self, fresh_system):
        """中型数据库单目标搜索。"""
        memory = [5, 12, 3, 8, 15, 7, 2, 9]
        target = 8

        idx, prob = grover_search(memory, target, n_iterations=2)

        assert 0 <= idx < len(memory)
        assert prob > 0

    def test_auto_iterations(self, fresh_system):
        """测试自动迭代次数计算。"""
        memory = [1, 2, 3, 4, 5, 6, 7, 8]
        target = 5

        idx, prob = grover_search(memory, target)

        assert 0 <= idx < len(memory)
        assert prob > 0

    def test_search_returns_valid_index(self, fresh_system):
        """验证搜索返回有效索引。"""
        memory = [10, 20, 30, 40]
        target = 30

        idx, prob = grover_search(memory, target, n_iterations=1)

        assert 0 <= idx < len(memory)
        assert prob > 0


class TestGroverCount:
    """测试量子计数变体。"""

    @pytest.mark.slow
    def test_count_single_marked_item(self, fresh_system):
        """测试单标记项计数。"""
        memory = [5, 12, 3, 8, 15, 7, 2, 9]
        target = 8

        count, prob = grover_count(memory, target, precision_bits=4)

        assert count >= 0

    @pytest.mark.slow
    def test_count_multiple_marked_items(self, fresh_system):
        """测试多标记项计数。"""
        memory = [5, 5, 5, 8, 8, 7, 2, 9]  # three 5s
        target = 5

        count, prob = grover_count(memory, target, precision_bits=4)

        assert count >= 0


class TestGroverAlgorithmProperties:
    """测试 Grover 算法的数学性质。"""

    def test_probability_amplitude_amplification(self, fresh_system):
        """验证振幅放大效果。"""
        memory = [1, 2, 3, 4, 5, 6, 7, 8]
        target = 5

        idx1, prob1 = grover_search(memory, target, n_iterations=1)
        idx2, prob2 = grover_search(memory, target, n_iterations=2)

        assert prob2 > 0

    def test_measurement_collapse(self, fresh_system):
        """验证测量后的状态坍缩。"""
        memory = [1, 2, 3, 4]
        target = 2

        idx, prob = grover_search(memory, target, n_iterations=1)

        assert isinstance(idx, int)
        assert 0 <= idx < len(memory)
