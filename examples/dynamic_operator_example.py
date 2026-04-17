#!/usr/bin/env python3
"""
动态算子扩展示例

演示如何使用 compile_operator 功能在运行时创建自定义 C++ 算子。

运行前请确保：
1. QRAM-Simulator 已正确构建
2. PySparQ 模块可导入

使用方法：
    python examples/dynamic_operator_example.py
"""

import sys
import os

# 将项目根目录添加到路径
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, project_root)

print("=" * 70)
print("PySparQ 动态算子扩展示例")
print("=" * 70)
print()

# 尝试从 PySparQ 导入
try:
    from pysparq import compile_operator
    print("✓ 成功从 pysparq 导入 compile_operator")
except ImportError:
    print("! pysparq 未完全安装，尝试直接导入 dynamic_operator...")
    from pysparq.dynamic_operator import compile_operator
    print("✓ 成功从 pysparq.dynamic_operator 导入")

print()

# ============ 示例 1: SelfAdjointOperator ============
print("-" * 70)
print("示例 1: SelfAdjointOperator (自伴算子)")
print("-" * 70)
print()
print("SelfAdjointOperator 用于厄米算子，其 dagger 操作自动等于自身。")
print("典型应用：Pauli 门（X、Z）、控制非门（CNOT）等。")
print()

cpp_code_1 = """
class MyFlipOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    MyFlipOp(size_t r) : reg_id(r) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.get(reg_id).value ^= 1;  // XOR 翻转
        }
    }
};
"""

try:
    MyFlipOp = compile_operator(
        name="MyFlipOp",
        cpp_code=cpp_code_1,
        base_class="SelfAdjointOperator",
        constructor_args=[("size_t", "reg_id")],
        verbose=True,
    )
    print()
    print("✓ SelfAdjointOperator 创建成功")
    print(f"  类名: {MyFlipOp.__name__}")
    print(f"  基类: {MyFlipOp._base_class}")
    print(f"  说明: dagger() 自动等于 operator()")
    print()
except Exception as e:
    print(f"✗ 创建失败: {e}")
    print()

# ============ 示例 2: BaseOperator with dagger ============
print("-" * 70)
print("示例 2: BaseOperator (带 dagger 实现)")
print("-" * 70)
print()
print("BaseOperator 需要手动实现 dag() 方法，用于非厄米算子。")
print("典型应用：相位门、受控相位门、一般酉门等。")
print()

cpp_code_2 = """
class MyPhaseOp : public BaseOperator {
    size_t reg_id;
    double phase;
public:
    MyPhaseOp(size_t r, double p) : reg_id(r), phase(p) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            if (s.get(reg_id).value != 0) {
                s.amplitude *= std::exp(std::complex<double>(0, phase));
            }
        }
    }
    void dag(std::vector<System>& state) const override {
        for (auto& s : state) {
            if (s.get(reg_id).value != 0) {
                s.amplitude *= std::exp(std::complex<double>(0, -phase));
            }
        }
    }
};
"""

try:
    MyPhaseOp = compile_operator(
        name="MyPhaseOp",
        cpp_code=cpp_code_2,
        base_class="BaseOperator",
        constructor_args=[("size_t", "reg_id"), ("double", "phase")],
        verbose=True,
    )
    print()
    print("✓ BaseOperator 创建成功")
    print(f"  类名: {MyPhaseOp.__name__}")
    print(f"  基类: {MyPhaseOp._base_class}")
    print(f"  说明: 需要实现 dag() 方法，用于逆操作")
    print()
except Exception as e:
    print(f"✗ 创建失败: {e}")
    print()

# ============ 示例 3: 多参数算子 ============
print("-" * 70)
print("示例 3: 多参数复杂算子")
print("-" * 70)
print()
print("动态算子支持多个构造函数参数，可用于复杂量子操作。")
print()

cpp_code_3 = """
class MyControlledOp : public SelfAdjointOperator {
    size_t control_reg;
    size_t target_reg;
    double angle;
public:
    MyControlledOp(size_t c, size_t t, double a)
        : control_reg(c), target_reg(t), angle(a) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            if (s.get(control_reg).value != 0) {
                s.amplitude *= std::exp(std::complex<double>(0, angle));
            }
        }
    }
};
"""

try:
    MyControlledOp = compile_operator(
        name="MyControlledOp",
        cpp_code=cpp_code_3,
        base_class="SelfAdjointOperator",
        constructor_args=[
            ("size_t", "control_reg"),
            ("size_t", "target_reg"),
            ("double", "angle"),
        ],
        verbose=True,
    )
    print()
    print("✓ 多参数算子创建成功")
    print(f"  类名: {MyControlledOp.__name__}")
    print(f"  参数列表:")
    print(f"    - control_reg (size_t): 控制寄存器 ID")
    print(f"    - target_reg (size_t): 目标寄存器 ID")
    print(f"    - angle (double): 相位角度（弧度）")
    print()
except Exception as e:
    print(f"✗ 创建失败: {e}")
    print()

# ============ 示例 4: 创建实例和使用 ============
print("-" * 70)
print("示例 4: 创建算子实例")
print("-" * 70)
print()

try:
    # 使用前面定义的算子类创建实例
    if 'MyFlipOp' in dir():
        flip_op = MyFlipOp(reg_id=0)
        print(f"MyFlipOp 实例: {repr(flip_op)}")

    if 'MyPhaseOp' in dir():
        import math
        phase_op = MyPhaseOp(reg_id=0, phase=math.pi/4)
        print(f"MyPhaseOp 实例: {repr(phase_op)}")

    if 'MyControlledOp' in dir():
        ctrl_op = MyControlledOp(control_reg=0, target_reg=1, angle=math.pi/2)
        print(f"MyControlledOp 实例: {repr(ctrl_op)}")

    print()
    print("注意: 实际应用于量子态需要编译完整的 PySparQ 模块。")
    print("编译后可使用: op(state) 或 op.dag(state)")

except Exception as e:
    print(f"✗ 创建实例失败: {e}")

print()

# ============ 示例 5: 缓存管理 ============
print("-" * 70)
print("示例 5: 编译缓存管理")
print("-" * 70)
print()

try:
    from pysparq.dynamic_operator import get_cache_info, clear_cache

    info = get_cache_info()
    print(f"缓存目录: {info['cache_dir']}")
    print(f"缓存文件数: {info['so_count']}")
    print(f"缓存大小: {info['total_size_mb']:.2f} MB")
    print()
    print("编译结果会自动缓存，避免重复编译相同代码。")
    print("如需清除缓存，可调用: clear_cache()")

except ImportError:
    print("无法导入缓存管理函数")

print()

# ============ 总结 ============
print("=" * 70)
print("示例完成")
print("=" * 70)

print("""
总结:
-----
本示例演示了动态算子的三种基本用法:

1. SelfAdjointOperator (自伴算子):
   - dagger() 自动等于 operator()
   - 适用于 Pauli 门、CNOT 等厄米算子
   - 实现更简单，只需写一个 operator() 方法

2. BaseOperator (一般算子):
   - 需要手动实现 dag() 方法
   - 适用于相位门、一般酉门等非厄米算子
   - 支持自定义逆操作

3. 多参数算子:
   - 支持多种参数类型: size_t, int, double, float, bool, uint64_t
   - 使用 constructor_args 指定参数列表
   - 创建实例时使用关键字参数

更多信息请参考:
- docs/sphinx/source/guide/dynamic_operators.rst (完整用户指南)
- docs/sphinx/source/api/dynamic_operator.rst (API 参考)
- examples/dynamic_operator_quantum.py (端到端量子电路示例)
- PySparQ/test/test_dynamic_operator.py (单元测试)
""")
