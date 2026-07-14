# PySparQ 动态算子扩展模块

提供运行时编译和加载自定义 C++ 算子的功能。

> **⚠️ 酉性声明 / QCFD 支持路径限制**
>
> `compile_operator()` 编译得到的算子只经过 C++ 类型检查，**编译成功不代表、
> 也不能证明该算子是酉的**（unitary），也不代表其 `dag()` 确实是 `operator()`
> 的逆。历史上已经出现过 `dag()` 直接清零寄存器、`operator()` 覆盖而非
> XOR-写入输出寄存器等破坏可逆性的实现，且这些实现都能顺利通过编译。
>
> 因此，本仓库支持的 QCFD 路径（QECC.Lang 驱动的 qfvm/qnls/qham）**禁止**
> 使用 `compile_operator` 生成的动态算子表达其语义。所有 QCFD 语义必须通过
> 具名、可静态检查的 PySparQ 内建算子（或由内建算子组合而成的 Python 组合
> 算子）表达，并通过 `pysparq.conformance` 提供的可复用一致性测试矩阵
> （任意非零输出目标、穷举/抽样基态覆盖、输出碰撞检测、叠加态线性性、
> 正/负/多重寄存器控制、forward+dagger 与 dagger+forward 恒等）验证。
> `compile_operator` 本身仍然是通用的运行时算子编译工具，可继续用于原型
> 验证、教学或与 QCFD 无关的实验。

## 功能特性

- **运行时 C++ 编译**: 将自定义 C++ 算子代码编译为共享库
- **Python 类动态生成**: 使用 `type()` 创建 Python 算子类
- **ctypes 集成**: 通过 ctypes 调用动态库中的工厂函数
- **缓存机制**: 基于代码哈希的编译缓存，避免重复编译
- **生命周期管理**: 正确处理 C++ 对象的创建和销毁

## API 参考

### `compile_operator()`

编译 C++ 代码为动态算子类。

```python
def compile_operator(
    name: str,
    cpp_code: str,
    base_class: str = "BaseOperator",
    extra_includes: List[str] = None,
    extra_libs: List[str] = None,
    constructor_args: List[Tuple[str, str]] = None,
    cache_dir: Optional[str] = None,
    verbose: bool = False,
) -> Type
```

**参数:**
- `name`: 算子类名
- `cpp_code`: C++ 源代码（仅类定义）
- `base_class`: 基类名 ("BaseOperator" 或 "SelfAdjointOperator")
- `extra_includes`: 额外头文件搜索路径列表
- `extra_libs`: 额外链接库列表
- `constructor_args`: 构造函数参数列表 `[(type, name), ...]`
- `cache_dir`: 缓存目录
- `verbose`: 是否输出详细日志

**返回:**
动态生成的算子类

**示例:**

```python
from PySparQ.dynamic_operator import compile_operator

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

# 像原生算子一样使用
op = MyFlipOp(reg_id=0)
state = op(state)
```

### `compile_cpp_code()`

底层编译函数，编译 C++ 代码为共享库。

```python
def compile_cpp_code(
    cpp_code: str,
    class_name: str,
    cache_dir: Optional[str] = None,
    ctor_params: str = "",
    ctor_args: str = "",
    config: Optional[CompilerConfig] = None,
    project_root: Optional[str] = None,
    verbose: bool = False,
) -> str
```

### `CompilerConfig`

编译器配置类。

```python
config = CompilerConfig(
    cxx="g++",              # C++ 编译器
    std="c++17",            # C++ 标准
    opt_level="O2",         # 优化级别
    include_paths=[],       # 额外头文件路径
    lib_paths=[],           # 额外库文件路径
    libraries=[],           # 链接的库
    extra_flags=[],         # 额外编译器标志
)
```

### 缓存管理

```python
from PySparQ.dynamic_operator import get_cache_info, clear_cache

# 查看缓存信息
info = get_cache_info()
print(f"缓存文件数: {info['file_count']}")

# 清除缓存
clear_cache()
```

## 实现架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Python 层                                 │
│  ┌─────────────────┐      ┌──────────────────────────────┐  │
│  │ compile_operator│─────▶│  DynamicOpClass (type创建)   │  │
│  └─────────────────┘      └──────────────────────────────┘  │
│           │                              │                  │
│           ▼                              ▼                  │
│  ┌─────────────────┐      ┌──────────────────────────────┐  │
│  │ compile_cpp_code│      │  CppOperatorWrapper (ctypes) │  │
│  └─────────────────┘      └──────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    C++ 动态库层                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  extern "C" BaseOperator* create_operator(...)       │  │
│  │  extern "C" void destroy_operator(BaseOperator*)     │  │
│  │  extern "C" void apply_operator(...)                 │  │
│  │  extern "C" void apply_operator_dag(...)             │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## 技术细节

### 工厂函数

动态库导出 C 风格工厂函数:

```cpp
// 创建算子实例
extern "C" BaseOperator* create_operator(...);

// 销毁算子实例
extern "C" void destroy_operator(BaseOperator* op);

// 获取算子名称
extern "C" const char* get_operator_name();

// 获取基类名称
extern "C" const char* get_base_class();

// Python 调用辅助函数
extern "C" void apply_operator(BaseOperator* op, SparseState* state);
extern "C" void apply_operator_dag(BaseOperator* op, SparseState* state);
```

### Python 类动态创建

使用 Python 的 `type()` 函数动态创建类:

```python
DynamicOpClass = type(
    name,  # 类名
    (object,),  # 基类
    {
        '__init__': custom_init,
        '__call__': call_method,
        'dag': dag_method,
        '__repr__': repr_method,
        '__del__': del_method,
    }
)
```

### 生命周期管理

1. **创建**: Python `__init__` 调用 C++ `create_operator()`
2. **使用**: Python `__call__` 调用 C++ `apply_operator()`
3. **销毁**: Python `__del__` 调用 C++ `destroy_operator()`

## 依赖要求

- Python 3.8+
- C++ 编译器 (g++ 或 clang++)
- PySparQ 核心库 (用于实际运行算子)

## 文件结构

```
PySparQ/dynamic_operator/
├── __init__.py          # 模块入口，导出主要 API
├── compiler.py          # C++ 代码编译系统
├── operator_wrapper.py  # ctypes 包装和动态类创建
└── README.md            # 本文档
```

## 已知限制

1. **需要 PySparQ 核心库**: 动态算子需要在 PySparQ 运行时环境中执行
2. **符号解析**: 动态库需要链接到 PySparQ 核心库以解析符号
3. **虚函数调用**: 通过 ctypes 直接调用 C++ 虚函数有限制，需要辅助函数

## 未来改进

1. 支持更多参数类型（复杂类型、字符串等）
2. 实现完整的 dag() 支持（非自伴算子）
3. 添加性能分析和调试功能
4. 支持 CUDA 算子动态编译
