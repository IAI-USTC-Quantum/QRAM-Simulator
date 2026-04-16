"""PySparQ 动态算子扩展模块 - 提供运行时编译和加载自定义 C++ 算子的功能。"""

from typing import List, Tuple, Type, Optional

from .compiler import (
    CompilerConfig,
    CompilationError,
    compile_cpp_code,
    compute_code_hash,
    find_project_root,
    generate_cpp_source,
    format_compile_error,
    clear_cache,
    get_cache_info,
    quick_compile,
)

from .operator_wrapper import (
    CppOperatorWrapper,
    DynamicOperatorError,
    DynamicOperatorLoadError,
    DynamicOperatorFactoryError,
    create_operator_class,
    cleanup_all_instances,
)

__all__ = [
    # 编译相关
    "CompilerConfig",
    "CompilationError",
    "compile_cpp_code",
    "compute_code_hash",
    "find_project_root",
    "generate_cpp_source",
    "format_compile_error",
    "clear_cache",
    "get_cache_info",
    "quick_compile",
    # 动态算子相关
    "compile_operator",
    "CppOperatorWrapper",
    "DynamicOperatorError",
    "DynamicOperatorLoadError",
    "DynamicOperatorFactoryError",
    "create_operator_class",
    "cleanup_all_instances",
]

__version__ = "0.2.0"


def compile_operator(
    name: str,
    cpp_code: str,
    base_class: str = "BaseOperator",
    extra_includes: List[str] = None,
    extra_libs: List[str] = None,
    constructor_args: List[Tuple[str, str]] = None,
    cache_dir: Optional[str] = None,
    verbose: bool = False,
) -> Type:
    """编译 C++ 代码为动态算子类。

    这是一个高级函数，将 C++ 代码编译并包装为可直接在 Python 中使用的算子类。

    Args:
        name: 算子类名
        cpp_code: C++ 源代码（仅类定义）
        base_class: 基类名 ("BaseOperator" 或 "SelfAdjointOperator")
        extra_includes: 额外头文件搜索路径列表
        extra_libs: 额外链接库列表
        constructor_args: 构造函数参数列表 [(type, name), ...]
        cache_dir: 缓存目录（默认使用系统临时目录）
        verbose: 是否输出详细日志

    Returns:
        动态生成的算子类

    Raises:
        CompilationError: 编译失败
        DynamicOperatorLoadError: 动态库加载失败
        ValueError: 参数错误
    """
    if extra_includes is None:
        extra_includes = []
    if extra_libs is None:
        extra_libs = []
    if constructor_args is None:
        constructor_args = []

    # 参数验证
    if not name or not isinstance(name, str):
        raise ValueError("name 必须是有效的字符串")
    if not cpp_code or not isinstance(cpp_code, str):
        raise ValueError("cpp_code 必须是有效的 C++ 代码字符串")

    valid_base_classes = ["BaseOperator", "SelfAdjointOperator"]
    if base_class not in valid_base_classes:
        raise ValueError(f"base_class 必须是 {valid_base_classes} 之一")

    # 构造构造函数参数字符串
    ctor_params = ", ".join(f"{arg_type} {arg_name}" for arg_type, arg_name in constructor_args)
    ctor_args = ", ".join(arg_name for _, arg_name in constructor_args)

    # 使用 Python 增强模板
    config = CompilerConfig(
        include_paths=extra_includes,
        libraries=extra_libs,
        template=CompilerConfig.PYTHON_TEMPLATE.replace("{BASE_CLASS}", base_class),
    )

    if verbose:
        print(f"[compile_operator] 编译算子: {name}")
        print(f"[compile_operator] 基类: {base_class}")
        print(f"[compile_operator] 参数: {ctor_params}")

    # 编译 C++ 代码
    lib_path = compile_cpp_code(
        cpp_code=cpp_code,
        class_name=name,
        cache_dir=cache_dir,
        ctor_params=ctor_params,
        ctor_args=ctor_args,
        config=config,
        verbose=verbose,
    )

    if verbose:
        print(f"[compile_operator] 编译成功: {lib_path}")
        print(f"[compile_operator] 创建 Python 类...")

    # 创建 Python 类
    OpClass = create_operator_class(
        name=name,
        lib_path=lib_path,
        base_class=base_class,
        constructor_args=constructor_args,
    )

    if verbose:
        print(f"[compile_operator] 算子类 {name} 已创建")

    return OpClass
