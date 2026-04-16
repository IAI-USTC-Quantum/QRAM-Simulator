#!/usr/bin/env python3
"""
测试动态算子扩展机制 - 仅测试编译功能
"""

import sys
import os

# 添加到路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'PySparQ'))

# 从 dynamic_operator 导入
from dynamic_operator import (
    compile_cpp_code,
    compile_operator,
    CompilerConfig,
    CompilationError,
    get_cache_info,
    clear_cache,
    CppOperatorWrapper,
    create_operator_class,
)


def test_basic_compile():
    """测试基本编译"""
    print("=" * 60)
    print("测试基本编译")
    print("=" * 60)
    
    cpp_code = """
class MyFlipOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    MyFlipOp(size_t r) : reg_id(r) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.get(reg_id).value ^= 1;
        }
    }
};
"""
    
    try:
        lib_path = compile_cpp_code(
            cpp_code=cpp_code,
            class_name="MyFlipOp",
            ctor_params="size_t r",
            ctor_args="r",
            verbose=True,
        )
        
        print(f"\n编译成功: {lib_path}")
        print(f"文件存在: {os.path.exists(lib_path)}")
        print(f"文件大小: {os.path.getsize(lib_path)} bytes")
        
        return True, lib_path
        
    except CompilationError as e:
        print(f"编译错误: {e}")
        return False, None
    except Exception as e:
        print(f"错误: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
        return False, None


def test_compile_operator():
    """测试 compile_operator 函数"""
    print("\n" + "=" * 60)
    print("测试 compile_operator")
    print("=" * 60)
    
    cpp_code = """
class MyTestOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    MyTestOp(size_t r) : reg_id(r) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.get(reg_id).value ^= 1;
        }
    }
};
"""
    
    try:
        MyTestOp = compile_operator(
            name="MyTestOp",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[("size_t", "reg_id")],
            verbose=True,
        )
        
        print(f"\n算子类创建成功: {MyTestOp}")
        print(f"类名: {MyTestOp.__name__}")
        print(f"文档: {MyTestOp.__doc__[:300]}...")
        
        # 创建实例
        op = MyTestOp(reg_id=0)
        print(f"\n算子实例创建成功: {repr(op)}")
        
        # 检查属性
        print(f"\n实例属性:")
        print(f"  _is_dynamic_operator: {op._is_dynamic_operator}")
        print(f"  _base_class: {op._base_class}")
        print(f"  _cpp_ptr: {op._cpp_ptr}")
        print(f"  _args: {op._args}")
        
        return True
        
    except Exception as e:
        print(f"错误: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_caching():
    """测试缓存"""
    print("\n" + "=" * 60)
    print("测试编译缓存")
    print("=" * 60)
    
    # 获取初始缓存状态
    info_before = get_cache_info()
    print(f"初始缓存信息: {info_before}")
    
    cpp_code = """
class CacheTestOp : public SelfAdjointOperator {
public:
    CacheTestOp() {}
    void operator()(std::vector<System>& state) const override {}
};
"""
    
    try:
        # 第一次编译
        print("\n第一次编译...")
        lib_path1 = compile_cpp_code(
            cpp_code=cpp_code,
            class_name="CacheTestOp",
            verbose=True,
        )
        print(f"库路径: {lib_path1}")
        
        # 第二次编译（应该使用缓存）
        print("\n第二次编译（应使用缓存）...")
        lib_path2 = compile_cpp_code(
            cpp_code=cpp_code,
            class_name="CacheTestOp",
            verbose=True,
        )
        print(f"库路径: {lib_path2}")
        
        # 验证路径相同
        assert lib_path1 == lib_path2, "缓存未命中！"
        print("\n✓ 缓存命中验证通过")
        
        # 获取最终缓存状态
        info_after = get_cache_info()
        print(f"\n最终缓存信息: {info_after}")
        
        return True
        
    except Exception as e:
        print(f"错误: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_cpp_operator_wrapper():
    """测试 CppOperatorWrapper"""
    print("\n" + "=" * 60)
    print("测试 CppOperatorWrapper")
    print("=" * 60)
    
    # 先编译一个库
    cpp_code = """
class WrapperTestOp : public SelfAdjointOperator {
    int value;
public:
    WrapperTestOp(int v) : value(v) {}
    void operator()(std::vector<System>& state) const override {}
};
"""
    
    try:
        lib_path = compile_cpp_code(
            cpp_code=cpp_code,
            class_name="WrapperTestOp",
            ctor_params="int v",
            ctor_args="v",
            verbose=False,
        )
        
        print(f"库路径: {lib_path}")
        
        # 创建 wrapper
        wrapper = CppOperatorWrapper(lib_path)
        wrapper.load(arg_types=["int"])
        
        # 测试获取名称
        name = wrapper.get_name()
        print(f"算子名称: {name}")
        
        # 测试获取基类
        base = wrapper.get_base_class()
        print(f"基类: {base}")
        
        # 测试创建对象
        ptr = wrapper.create(42)
        print(f"C++ 对象指针: {ptr}")
        
        # 测试销毁对象
        wrapper.destroy(ptr)
        print(f"对象已销毁")
        
        return True
        
    except Exception as e:
        print(f"错误: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_multiple_constructor_args():
    """测试多个构造函数参数"""
    print("\n" + "=" * 60)
    print("测试多个构造函数参数")
    print("=" * 60)
    
    cpp_code = """
class MultiArgOp : public SelfAdjointOperator {
    size_t reg_a;
    size_t reg_b;
    int factor;
public:
    MultiArgOp(size_t a, size_t b, int f) : reg_a(a), reg_b(b), factor(f) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.get(reg_a).value = s.get(reg_b).value * factor;
        }
    }
};
"""
    
    try:
        MultiArgOp = compile_operator(
            name="MultiArgOp",
            cpp_code=cpp_code,
            base_class="SelfAdjointOperator",
            constructor_args=[("size_t", "reg_a"), ("size_t", "reg_b"), ("int", "factor")],
            verbose=True,
        )
        
        print(f"\n算子类创建成功: {MultiArgOp}")
        
        # 创建实例
        op = MultiArgOp(reg_a=0, reg_b=1, factor=2)
        print(f"\n算子实例创建成功: {repr(op)}")
        print(f"参数: {op._args}")
        
        return True
        
    except Exception as e:
        print(f"错误: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    """主函数"""
    print("动态算子扩展测试（仅编译）")
    print("=" * 60)
    
    results = []
    
    # 测试1: 基本编译
    results.append(("basic_compile", test_basic_compile()[0]))
    
    # 测试2: compile_operator
    results.append(("compile_operator", test_compile_operator()))
    
    # 测试3: 缓存
    results.append(("caching", test_caching()))
    
    # 测试4: CppOperatorWrapper
    results.append(("cpp_operator_wrapper", test_cpp_operator_wrapper()))
    
    # 测试5: 多个参数
    results.append(("multiple_args", test_multiple_constructor_args()))
    
    # 汇总结果
    print("\n" + "=" * 60)
    print("测试结果汇总")
    print("=" * 60)
    
    for name, passed in results:
        status = "✓ 通过" if passed else "✗ 失败"
        print(f"  {name}: {status}")
    
    total = len(results)
    passed = sum(1 for _, p in results if p)
    print(f"\n总计: {passed}/{total} 测试通过")
    
    return all(p for _, p in results)


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
