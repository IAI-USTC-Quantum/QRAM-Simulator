#!/usr/bin/env python3
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'PySparQ'))

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
    print("=" * 60)
    print("Test 1: Basic Compile")
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
        
        print(f"\nCompiled: {lib_path}")
        print(f"Exists: {os.path.exists(lib_path)}")
        print(f"Size: {os.path.getsize(lib_path)} bytes")
        return True, lib_path
    except Exception as e:
        print(f"Error: {e}")
        return False, None


def test_compile_operator():
    print("\n" + "=" * 60)
    print("Test 2: compile_operator")
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
        
        print(f"\nClass created: {MyTestOp}")
        print(f"Class name: {MyTestOp.__name__}")
        
        op = MyTestOp(reg_id=0)
        print(f"Instance created: {repr(op)}")
        print(f"_is_dynamic_operator: {op._is_dynamic_operator}")
        print(f"_base_class: {op._base_class}")
        print(f"_cpp_ptr: {op._cpp_ptr}")
        
        return True
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_caching():
    print("\n" + "=" * 60)
    print("Test 3: Caching")
    print("=" * 60)
    
    info_before = get_cache_info()
    print(f"Cache before: {info_before}")
    
    cpp_code = """
class CacheTestOp : public SelfAdjointOperator {
public:
    CacheTestOp() {}
    void operator()(std::vector<System>& state) const override {}
};
"""
    
    try:
        print("\nFirst compile...")
        lib_path1 = compile_cpp_code(
            cpp_code=cpp_code,
            class_name="CacheTestOp",
            verbose=True,
        )
        
        print("\nSecond compile (should use cache)...")
        lib_path2 = compile_cpp_code(
            cpp_code=cpp_code,
            class_name="CacheTestOp",
            verbose=True,
        )
        
        assert lib_path1 == lib_path2, "Cache miss!"
        print("\nCache hit verified!")
        
        info_after = get_cache_info()
        print(f"Cache after: {info_after}")
        return True
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_wrapper():
    print("\n" + "=" * 60)
    print("Test 4: CppOperatorWrapper")
    print("=" * 60)
    
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
        
        print(f"Library: {lib_path}")
        
        wrapper = CppOperatorWrapper(lib_path)
        wrapper.load(arg_types=["int"])
        
        name = wrapper.get_name()
        print(f"Operator name: {name}")
        
        base = wrapper.get_base_class()
        print(f"Base class: {base}")
        
        ptr = wrapper.create(42)
        print(f"C++ object pointer: {ptr}")
        
        wrapper.destroy(ptr)
        print("Object destroyed")
        
        return True
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    print("Dynamic Operator Extension Tests")
    print("=" * 60)
    
    results = []
    results.append(("basic_compile", test_basic_compile()[0]))
    results.append(("compile_operator", test_compile_operator()))
    results.append(("caching", test_caching()))
    results.append(("wrapper", test_wrapper()))
    
    print("\n" + "=" * 60)
    print("Results")
    print("=" * 60)
    
    for name, passed in results:
        status = "PASS" if passed else "FAIL"
        print(f"  {name}: {status}")
    
    total = len(results)
    passed = sum(1 for _, p in results if p)
    print(f"\nTotal: {passed}/{total} tests passed")
    
    return all(p for _, p in results)


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
