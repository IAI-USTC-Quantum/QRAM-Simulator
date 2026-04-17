#!/usr/bin/env python3
"""
文档示例测试

验证所有文档中的示例代码可正常编译和运行。
"""

import pytest
import sys
import os
import math

# 添加项目根目录到路径
project_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, project_root)

from pysparq.dynamic_operator import (
    compile_operator,
    CompilationError,
    clear_cache,
    get_cache_info,
    DynamicOperatorLoadError,
)


class TestControlledPhaseExample:
    """测试受控相位门示例"""

    def setup_method(self):
        """每个测试前清理缓存"""
        clear_cache()

    def test_compile_controlled_phase(self):
        """测试受控相位门编译"""
        cpp_code = """
        class ControlledPhase : public BaseOperator {
            size_t control_reg;
            size_t target_reg;
            double phase;
        public:
            ControlledPhase(size_t c, size_t t, double theta)
                : control_reg(c), target_reg(t), phase(theta) {}

            void operator()(std::vector<System>& state) const override {
                for (auto& s : state) {
                    if (s.get(control_reg).value && s.get(target_reg).value) {
                        s.amplitude *= std::exp(std::complex<double>(0, phase));
                    }
                }
            }

            void dag(std::vector<System>& state) const override {
                for (auto& s : state) {
                    if (s.get(control_reg).value && s.get(target_reg).value) {
                        s.amplitude *= std::exp(std::complex<double>(0, -phase));
                    }
                }
            }
        };
        """

        ControlledPhase = compile_operator(
            name="ControlledPhase",
            cpp_code=cpp_code,
            base_class="BaseOperator",
            constructor_args=[
                ("size_t", "control_reg"),
                ("size_t", "target_reg"),
                ("double", "phase")
            ]
        )

        assert ControlledPhase is not None
        assert ControlledPhase.__name__ == "ControlledPhase"

        # 测试实例创建
        op = ControlledPhase(control_reg=0, target_reg=1, phase=math.pi/4)
        assert op is not None
        assert "ControlledPhase" in repr(op)


class TestQuantumWalkExample:
    """测试量子游走示例"""

    def setup_method(self):
        clear_cache()

    def test_compile_quantum_walk_step(self):
        """测试量子游走步进算子编译"""
        cpp_code = """
        class QuantumWalkStep : public SelfAdjointOperator {
            size_t position_reg;
            size_t coin_reg;
            size_t n_positions;
        public:
            QuantumWalkStep(size_t pos, size_t coin, size_t n)
                : position_reg(pos), coin_reg(coin), n_positions(n) {}

            void operator()(std::vector<System>& state) const override {
                for (auto& s : state) {
                    size_t coin_val = s.get(coin_reg).value;
                    size_t pos = s.get(position_reg).value;

                    if (coin_val == 0 && pos < n_positions - 1) {
                        s.get(position_reg).value = pos + 1;
                    } else if (coin_val == 1 && pos > 0) {
                        s.get(position_reg).value = pos - 1;
                    }
                }
            }
        };
        """

        QuantumWalkStep = compile_operator(
            name="QuantumWalkStep",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[
                ("size_t", "position_reg"),
                ("size_t", "coin_reg"),
                ("size_t", "n_positions")
            ]
        )

        assert QuantumWalkStep is not None
        assert QuantumWalkStep.__name__ == "QuantumWalkStep"


class TestGroverOracleExample:
    """测试 Grover Oracle 示例"""

    def setup_method(self):
        clear_cache()

    def test_compile_mark_oracle(self):
        """测试标记 Oracle 编译"""
        cpp_code = """
        class MarkOracle : public SelfAdjointOperator {
            size_t data_reg;
            uint64_t target_value;
        public:
            MarkOracle(size_t d, uint64_t t) : data_reg(d), target_value(t) {}

            void operator()(std::vector<System>& state) const override {
                for (auto& s : state) {
                    if (s.get(data_reg).value == target_value) {
                        s.amplitude *= -1.0;
                    }
                }
            }
        };
        """

        MarkOracle = compile_operator(
            name="MarkOracle",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[
                ("size_t", "data_reg"),
                ("uint64_t", "target_value")
            ]
        )

        assert MarkOracle is not None
        assert MarkOracle.__name__ == "MarkOracle"

        # 测试实例创建
        op = MarkOracle(data_reg=0, target_value=5)
        assert op is not None


class TestHamiltonianEvolutionExample:
    """测试哈密顿量演化示例"""

    def setup_method(self):
        clear_cache()

    def test_compile_hamiltonian_evolution(self):
        """测试哈密顿量演化算子编译"""
        cpp_code = """
        class HamiltonianEvolution : public BaseOperator {
            size_t reg_id;
            double coupling_strength;
            double time;
        public:
            HamiltonianEvolution(size_t r, double g, double t)
                : reg_id(r), coupling_strength(g), time(t) {}

            void operator()(std::vector<System>& state) const override {
                double phase = coupling_strength * time;
                for (auto& s : state) {
                    double value_phase = phase * s.get(reg_id).value;
                    s.amplitude *= std::exp(std::complex<double>(0, value_phase));
                }
            }

            void dag(std::vector<System>& state) const override {
                double phase = -coupling_strength * time;
                for (auto& s : state) {
                    double value_phase = phase * s.get(reg_id).value;
                    s.amplitude *= std::exp(std::complex<double>(0, value_phase));
                }
            }
        };
        """

        HamiltonianEvolution = compile_operator(
            name="HamiltonianEvolution",
            cpp_code=cpp_code,
            base_class="BaseOperator",
            constructor_args=[
                ("size_t", "reg_id"),
                ("double", "coupling_strength"),
                ("double", "time")
            ]
        )

        assert HamiltonianEvolution is not None
        assert HamiltonianEvolution.__name__ == "HamiltonianEvolution"

        # 测试实例创建
        op = HamiltonianEvolution(reg_id=0, coupling_strength=0.5, time=1.0)
        assert op is not None


class TestMultiEntangleExample:
    """测试多寄存器纠缠门示例"""

    def setup_method(self):
        clear_cache()

    def test_compile_multi_entangle(self):
        """测试多寄存器纠缠门编译"""
        cpp_code = """
        class MultiEntangleOp : public SelfAdjointOperator {
            size_t reg_a;
            size_t reg_b;
            size_t reg_c;
        public:
            MultiEntangleOp(size_t a, size_t b, size_t c)
                : reg_a(a), reg_b(b), reg_c(c) {}

            void operator()(std::vector<System>& state) const override {
                for (auto& s : state) {
                    uint64_t val = s.get(reg_a).value ^ s.get(reg_b).value ^ s.get(reg_c).value;
                    s.get(reg_a).value = val;
                }
            }
        };
        """

        MultiEntangleOp = compile_operator(
            name="MultiEntangleOp",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[
                ("size_t", "reg_a"),
                ("size_t", "reg_b"),
                ("size_t", "reg_c")
            ]
        )

        assert MultiEntangleOp is not None
        assert MultiEntangleOp.__name__ == "MultiEntangleOp"


class TestCaching:
    """测试缓存机制"""

    def setup_method(self):
        clear_cache()

    def teardown_method(self):
        clear_cache()

    def test_cache_hit(self):
        """测试缓存命中"""
        cpp_code = """
        class CachedOp : public SelfAdjointOperator {
            size_t reg_id;
        public:
            CachedOp(size_t r) : reg_id(r) {}
            void operator()(std::vector<System>& state) const override {}
        };
        """

        # 第一次编译
        OpClass1 = compile_operator(
            name="CachedOp",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[("size_t", "reg_id")]
        )
        lib_path1 = OpClass1._lib_path

        # 第二次编译相同代码
        OpClass2 = compile_operator(
            name="CachedOp",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[("size_t", "reg_id")]
        )
        lib_path2 = OpClass2._lib_path

        # 应该使用相同的缓存库
        assert lib_path1 == lib_path2

    def test_cache_info(self):
        """测试缓存信息获取"""
        info = get_cache_info()
        assert "cache_dir" in info
        assert "so_count" in info
        assert "total_size_mb" in info


class TestErrorHandling:
    """测试错误处理"""

    def setup_method(self):
        clear_cache()

    def test_compilation_error(self):
        """测试编译错误处理"""
        bad_cpp_code = """
        class BadOp : public BaseOperator {
            void operator()(std::vector<System>& state) const override {
                undefined_function();  // 未定义函数
            }
        };
        """

        with pytest.raises(CompilationError):
            compile_operator(
                name="BadOp",
                cpp_code=bad_cpp_code,
                base_class="BaseOperator"
            )

    def test_invalid_base_class(self):
        """测试无效基类"""
        with pytest.raises(ValueError) as exc_info:
            compile_operator(
                name="TestOp",
                cpp_code="class TestOp : public InvalidBase {};",
                base_class="InvalidBase"
            )
        assert "base_class" in str(exc_info.value)

    def test_empty_name(self):
        """测试空名称"""
        with pytest.raises(ValueError):
            compile_operator(
                name="",
                cpp_code="class TestOp : public BaseOperator {};"
            )

    def test_empty_code(self):
        """测试空代码"""
        with pytest.raises(ValueError):
            compile_operator(
                name="TestOp",
                cpp_code=""
            )


class TestDocstringExamples:
    """测试文档中的代码示例"""

    def setup_method(self):
        clear_cache()

    def test_basic_flip_operator(self):
        """测试基本翻转算子示例（来自文档）"""
        cpp_code = """
        class FlipOp : public SelfAdjointOperator {
            size_t reg_id;
        public:
            FlipOp(size_t r) : reg_id(r) {}
            void operator()(std::vector<System>& state) const override {
                for (auto& s : state) {
                    s.get(reg_id).value ^= 1;
                }
            }
        };
        """

        FlipOp = compile_operator(
            name="FlipOp",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[("size_t", "reg_id")]
        )

        assert FlipOp is not None
        op = FlipOp(reg_id=0)
        assert "FlipOp" in repr(op)

    def test_phase_gate_operator(self):
        """测试相位门示例（来自文档）"""
        cpp_code = """
        class PhaseGate : public BaseOperator {
            size_t reg_id;
            double phase;
        public:
            PhaseGate(size_t r, double p) : reg_id(r), phase(p) {}
            void operator()(std::vector<System>& state) const override {
                for (auto& s : state) {
                    s.amplitude *= std::exp(std::complex<double>(0, phase));
                }
            }
            void dag(std::vector<System>& state) const override {
                for (auto& s : state) {
                    s.amplitude *= std::exp(std::complex<double>(0, -phase));
                }
            }
        };
        """

        PhaseGate = compile_operator(
            name="PhaseGate",
            cpp_code=cpp_code,
            base_class="BaseOperator",
            constructor_args=[
                ("size_t", "reg_id"),
                ("double", "phase")
            ]
        )

        assert PhaseGate is not None
        op = PhaseGate(reg_id=0, phase=math.pi/4)
        assert op is not None


# ============ 运行测试 ============

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
