#!/usr/bin/env python3
"""
动态算子量子电路示例

演示如何定义和编译用于量子计算的动态算子。

运行前请确保：
1. QRAM-Simulator 已正确构建
2. PySparQ 模块可导入
3. g++ 编译器可用

使用方法：
    python examples/dynamic_operator_quantum.py

注意：
    本示例演示算子的编译和实例创建。
    实际应用于量子态需要确保 ABI 兼容性。
"""

import sys
import os
import math

# 添加项目根目录到路径
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, project_root)

print("=" * 70)
print("PySparQ 动态算子量子电路示例")
print("=" * 70)
print()

# 导入 PySparQ
try:
    import pysparq as ps
    from pysparq.dynamic_operator import compile_operator, get_cache_info, clear_cache
    print("成功导入 pysparq 和 compile_operator")
except ImportError as e:
    print(f"导入失败: {e}")
    print("请确保 PySparQ 已正确安装: pip install .")
    sys.exit(1)

print()

# 清除缓存以演示完整流程
print("清除编译缓存...")
clear_cache()
print()

# ============================================================================
# 示例 1: 受控相位门 (CU(1))
# ============================================================================
print("-" * 70)
print("示例 1: 受控相位门 (CU(1))")
print("-" * 70)
print()
print("受控相位门在控制位和目标位都为 |1> 时应用相位旋转。")
print("这是量子计算中常用的受控门之一。")
print()

controlled_phase_code = """
class ControlledPhase : public BaseOperator {
    size_t control_reg;
    size_t target_reg;
    double phase;
public:
    ControlledPhase(size_t c, size_t t, double theta)
        : control_reg(c), target_reg(t), phase(theta) {}

    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            // 当控制位和目标位都为 |1> 时应用相位
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

print("编译受控相位门...")
try:
    ControlledPhase = compile_operator(
        name="ControlledPhase",
        cpp_code=controlled_phase_code,
        base_class="BaseOperator",
        constructor_args=[
            ("size_t", "control_reg"),
            ("size_t", "target_reg"),
            ("double", "phase")
        ],
        verbose=True
    )
    print()
    print("编译成功!")
    print(f"  类名: {ControlledPhase.__name__}")
    print(f"  基类: {ControlledPhase._base_class}")

    # 创建实例
    op = ControlledPhase(control_reg=0, target_reg=1, phase=math.pi/4)
    print(f"  实例: {repr(op)}")
    print()
except Exception as e:
    print(f"编译失败: {e}")
    print()
    ControlledPhase = None

# ============================================================================
# 示例 2: 多寄存器纠缠门
# ============================================================================
print("-" * 70)
print("示例 2: 多寄存器纠缠门")
print("-" * 70)
print()
print("三重 XOR 纠缠门，用于创建多寄存器纠缠态。")
print()

entangle_code = """
class MultiEntangleOp : public SelfAdjointOperator {
    size_t reg_a;
    size_t reg_b;
    size_t reg_c;
public:
    MultiEntangleOp(size_t a, size_t b, size_t c)
        : reg_a(a), reg_b(b), reg_c(c) {}

    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            // 三重 XOR：纠缠三个寄存器
            uint64_t val = s.get(reg_a).value ^ s.get(reg_b).value ^ s.get(reg_c).value;
            s.get(reg_a).value = val;
        }
    }
};
"""

print("编译多寄存器纠缠门...")
try:
    MultiEntangleOp = compile_operator(
        name="MultiEntangleOp",
        cpp_code=entangle_code,
        base_class="SelfAdjointOperator",
        constructor_args=[
            ("size_t", "reg_a"),
            ("size_t", "reg_b"),
            ("size_t", "reg_c")
        ],
        verbose=True
    )
    print()
    print("编译成功!")
    print(f"  类名: {MultiEntangleOp.__name__}")
    print(f"  基类: {MultiEntangleOp._base_class}")

    op = MultiEntangleOp(reg_a=0, reg_b=1, reg_c=2)
    print(f"  实例: {repr(op)}")
    print()
except Exception as e:
    print(f"编译失败: {e}")
    print()
    MultiEntangleOp = None

# ============================================================================
# 示例 3: Grover 搜索 Oracle
# ============================================================================
print("-" * 70)
print("示例 3: Grover 搜索 Oracle")
print("-" * 70)
print()
print("标记 Oracle 是 Grover 搜索算法的核心组件。")
print("它通过相位翻转标记目标状态。")
print()

oracle_code = """
class MarkOracle : public SelfAdjointOperator {
    size_t data_reg;
    uint64_t target_value;
public:
    MarkOracle(size_t d, uint64_t t) : data_reg(d), target_value(t) {}

    void operator()(std::vector<System>& state) const override {
        // 标记目标状态：当数据寄存器等于目标值时，振幅乘以 -1
        for (auto& s : state) {
            if (s.get(data_reg).value == target_value) {
                s.amplitude *= -1.0;
            }
        }
    }
};
"""

print("编译标记 Oracle...")
try:
    MarkOracle = compile_operator(
        name="MarkOracle",
        cpp_code=oracle_code,
        base_class="SelfAdjointOperator",
        constructor_args=[
            ("size_t", "data_reg"),
            ("uint64_t", "target_value")
        ],
        verbose=True
    )
    print()
    print("编译成功!")
    print(f"  类名: {MarkOracle.__name__}")
    print(f"  基类: {MarkOracle._base_class}")

    op = MarkOracle(data_reg=0, target_value=5)
    print(f"  实例: {repr(op)}")
    print()
except Exception as e:
    print(f"编译失败: {e}")
    print()
    MarkOracle = None

# ============================================================================
# 示例 4: 哈密顿量演化
# ============================================================================
print("-" * 70)
print("示例 4: 哈密顿量演化")
print("-" * 70)
print()
print("哈密顿量演化算子用于模拟量子系统的时间演化。")
print()

hamiltonian_code = """
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
            // 根据寄存器值应用相位旋转
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

print("编译哈密顿量演化算子...")
try:
    HamiltonianEvolution = compile_operator(
        name="HamiltonianEvolution",
        cpp_code=hamiltonian_code,
        base_class="BaseOperator",
        constructor_args=[
            ("size_t", "reg_id"),
            ("double", "coupling_strength"),
            ("double", "time")
        ],
        verbose=True
    )
    print()
    print("编译成功!")
    print(f"  类名: {HamiltonianEvolution.__name__}")
    print(f"  基类: {HamiltonianEvolution._base_class}")

    op = HamiltonianEvolution(reg_id=0, coupling_strength=0.5, time=1.0)
    print(f"  实例: {repr(op)}")
    print()
except Exception as e:
    print(f"编译失败: {e}")
    print()
    HamiltonianEvolution = None

# ============================================================================
# 示例 5: 量子游走算子
# ============================================================================
print("-" * 70)
print("示例 5: 量子游走步进算子")
print("-" * 70)
print()
print("量子游走是经典随机游走的量子类比。")
print("步进算子根据硬币状态移动位置。")
print()

quantum_walk_code = """
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

            // 硬币为 0 向右移动，为 1 向左移动
            if (coin_val == 0 && pos < n_positions - 1) {
                s.get(position_reg).value = pos + 1;
            } else if (coin_val == 1 && pos > 0) {
                s.get(position_reg).value = pos - 1;
            }
        }
    }
};
"""

print("编译量子游走步进算子...")
try:
    QuantumWalkStep = compile_operator(
        name="QuantumWalkStep",
        cpp_code=quantum_walk_code,
        base_class="SelfAdjointOperator",
        constructor_args=[
            ("size_t", "position_reg"),
            ("size_t", "coin_reg"),
            ("size_t", "n_positions")
        ],
        verbose=True
    )
    print()
    print("编译成功!")
    print(f"  类名: {QuantumWalkStep.__name__}")
    print(f"  基类: {QuantumWalkStep._base_class}")

    op = QuantumWalkStep(position_reg=0, coin_reg=1, n_positions=8)
    print(f"  实例: {repr(op)}")
    print()
except Exception as e:
    print(f"编译失败: {e}")
    print()
    QuantumWalkStep = None

# ============================================================================
# 缓存信息
# ============================================================================
print("-" * 70)
print("缓存信息")
print("-" * 70)
print()

info = get_cache_info()
print(f"缓存目录: {info['cache_dir']}")
print(f"缓存文件数: {info['so_count']}")
print(f"缓存大小: {info['total_size_mb']:.2f} MB")
print()

# ============================================================================
# 总结
# ============================================================================
print("=" * 70)
print("示例完成")
print("=" * 70)
print()

print("""
总结:
-----
本示例演示了以下量子计算相关的动态算子:

1. 受控相位门 (ControlledPhase):
   - 使用 BaseOperator 实现 dagger
   - 当控制位和目标位都为 |1> 时应用相位
   - 可用于受控酉门序列

2. 多寄存器纠缠门 (MultiEntangleOp):
   - 使用 SelfAdjointOperator
   - 通过三重 XOR 创建纠缠
   - dagger 自动等于自身

3. Grover 搜索 Oracle (MarkOracle):
   - 使用 SelfAdjointOperator
   - 通过相位翻转标记目标状态
   - 是 Grover 算法的核心组件

4. 哈密顿量演化 (HamiltonianEvolution):
   - 使用 BaseOperator 实现可逆演化
   - 支持自定义耦合强度和时间
   - 可用于量子模拟

5. 量子游走步进 (QuantumWalkStep):
   - 使用 SelfAdjointOperator
   - 根据硬币状态移动位置
   - 是量子游走算法的基础

更多信息请参考:
- docs/sphinx/source/guide/dynamic_operators.rst (完整用户指南)
- docs/sphinx/source/api/dynamic_operator.rst (API 参考)
- PySparQ/test/test_dynamic_operator.py (单元测试)
""")