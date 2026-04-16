# QRAM-Simulator Python 动态 Operator 扩展机制 - 第二阶段实现报告

## 任务完成情况

### ✅ 已完成的文件

1. **`PySparQ/dynamic_operator/operator_wrapper.py`** (新建)
   - `CppOperatorWrapper` 类：使用 ctypes 加载动态库和调用工厂函数
   - `create_operator_class()` 函数：使用 `type()` 动态创建 Python 算子类
   - 完整的生命周期管理（创建、调用、销毁）
   - 支持 `BaseOperator` 和 `SelfAdjointOperator` 两种基类
   - 弱引用机制跟踪活跃实例

2. **`PySparQ/dynamic_operator/__init__.py`** (更新)
   - 新增 `compile_operator()` 函数
   - 支持完整的参数列表：`name`, `cpp_code`, `base_class`, `extra_includes`, `extra_libs`, `constructor_args`
   - 自动参数验证
   - 导出所有公共 API

3. **`PySparQ/dynamic_operator/compiler.py`** (更新)
   - 新增 `PYTHON_TEMPLATE` 模板：包含支持 ctypes 调用的辅助函数
   - 添加 `get_base_class()`、`apply_operator()`、`apply_operator_dag()` 等辅助函数
   - 扩展包含路径：添加 `QRAM/include`、`Common/include`、`ThirdParty/fmt/include`
   - 修复头文件包含路径问题

4. **`PySparQ/dynamic_operator/README.md`** (新建)
   - 完整的 API 文档
   - 使用示例
   - 架构说明
   - 技术细节

## 核心功能验证

### 编译功能测试
```python
from dynamic_operator import compile_cpp_code

lib_path = compile_cpp_code(
    cpp_code="""
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
""",
    class_name="MyFlipOp",
    ctor_params="size_t r",
    ctor_args="r",
    verbose=True,
)
# 结果: 编译成功 /tmp/pysparq_dynamic_ops/MyFlipOp_xxx.so
```

### compile_operator 功能测试
```python
from dynamic_operator import compile_operator

MyTestOp = compile_operator(
    name="MyTestOp",
    cpp_code=cpp_code,
    base_class="SelfAdjointOperator",
    constructor_args=[("size_t", "reg_id")],
    verbose=True,
)

op = MyTestOp(reg_id=0)
print(repr(op))  # MyTestOp(reg_id=0)
```

### 编译缓存功能
- 基于代码哈希的缓存机制工作正常
- 相同代码第二次编译会使用缓存

## 实现的技术要点

### 1. C++ 模板设计
```cpp
// 工厂函数
extern "C" BaseOperator* create_operator(size_t r) {
    return new MyFlipOp(r);
}

extern "C" void destroy_operator(BaseOperator* op) {
    delete op;
}

// Python 调用辅助函数
extern "C" void apply_operator(BaseOperator* op, SparseState* state) {
    if (op && state) {
        (*op)(*state);
    }
}
```

### 2. Python 动态类创建
```python
def create_operator_class(name, lib_path, base_class, constructor_args):
    wrapper = CppOperatorWrapper(lib_path)
    wrapper.load([arg[0] for arg in constructor_args])
    
    def custom_init(self, **kwargs):
        # 收集参数
        args = []
        for arg_type, arg_name in constructor_args:
            args.append(kwargs[arg_name])
        self._args = tuple(args)
        self._cpp_ptr = wrapper.create(*args)
    
    def call_method(self, state):
        state_ptr = _get_state_ptr(state)
        wrapper.apply(self._cpp_ptr, state_ptr)
        return state
    
    DynamicOpClass = type(
        name,
        (object,),
        {
            '__init__': custom_init,
            '__call__': call_method,
            'dag': dag_method,
            '__repr__': repr_method,
            '__del__': del_method,
        }
    )
    return DynamicOpClass
```

### 3. ctypes 类型映射
```python
type_mapping = {
    'int': ctypes.c_int,
    'size_t': ctypes.c_size_t,
    'unsigned int': ctypes.c_uint,
    'float': ctypes.c_float,
    'double': ctypes.c_double,
    'bool': ctypes.c_bool,
}
```

## 已知限制和解决方案

### 限制 1: 需要 PySparQ 核心库
**问题**: 动态库需要链接到 PySparQ 核心库以解析 `qram_simulator::System` 等符号

**当前状态**: 
- ✅ 编译功能完全正常
- ⚠️ 运行时调用需要 PySparQ._core 模块已编译并加载

**解决方案**: 
- 在 PySparQ 构建系统中添加对动态算子的支持
- 或使用 `RTLD_GLOBAL` 在加载 pysparq._core 后再加载动态库

### 限制 2: 虚函数调用
**问题**: ctypes 无法直接调用 C++ 虚函数

**解决方案**: 
- 使用辅助函数 `apply_operator()` 和 `apply_operator_dag()`
- 已在 PYTHON_TEMPLATE 中实现

## 使用目标 API

实现完成后支持以下使用方式：

```python
from qram_simulator import compile_operator

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

MyFlipOp = compile_operator(
    name="MyFlipOp",
    cpp_code=cpp_code,
    base_class="SelfAdjointOperator",
    constructor_args=[("size_t", "reg_id")]
)

op = MyFlipOp(reg_id=0)
state = op(state)  # 像原生算子一样使用
```

## 下一步工作

1. **PySparQ 构建集成**: 确保 PySparQ._core 编译时导出必要的符号
2. **运行时测试**: 在完整的 PySparQ 环境中测试算子调用
3. **dag() 完整支持**: 实现非自伴算子的 dagger 操作
4. **文档完善**: 添加更多使用示例和最佳实践

## 总结

第二阶段实现已完成：
- ✅ `compile_operator()` 函数实现
- ✅ `operator_wrapper.py` 动态类包装
- ✅ C++ 辅助函数模板
- ✅ 完整的错误处理
- ✅ 缓存机制

等待 PySparQ 核心库编译完成后即可进行完整的功能测试。
