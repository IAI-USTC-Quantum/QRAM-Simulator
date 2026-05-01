"""
运行时 C++ 代码编译系统

提供动态编译用户自定义 C++ 算子的功能，支持：
- 自动生成代码框架
- 调用 g++ 编译为共享库 (.so)
- 代码哈希缓存机制，避免重复编译
- 编译错误捕获和格式化
"""

import hashlib
import os
import re
import shutil
import subprocess
import tempfile
import warnings
from pathlib import Path
from typing import Optional, Tuple


class CompilationError(Exception):
    """编译错误异常"""

    def __init__(self, message: str, stderr: str = "", returncode: int = 0):
        super().__init__(message)
        self.stderr = stderr
        self.returncode = returncode

    def __str__(self) -> str:
        msg = super().__str__()
        if self.stderr:
            msg += f"\n\n编译器输出:\n{self.stderr}"
        return msg


class CompilerConfig:
    """编译器配置"""

    # 默认代码框架模板
    DEFAULT_TEMPLATE = """#include "basic_components.h"
#include <vector>
#include <complex>

using namespace qram_simulator;

{USER_CPP_CODE}

extern "C" BaseOperator* create_operator({CTOR_PARAMS}) {{
    return new {CLASS_NAME}({CTOR_ARGS});
}}

extern "C" void destroy_operator(BaseOperator* op) {{
    delete op;
}}

extern "C" const char* get_operator_name() {{
    return "{CLASS_NAME}";
}}
"""

    # Python 增强模板 - 包含支持 ctypes 调用的辅助函数
    # 关键：通过 state._cpp_ptr()（pysparq._core.SparseState 中暴露）获取 C++ SparseState* 指针，
    # ctypes 将其作为 c_void_p 传递，确保指针值正确传递且 ABI 一致。
    PYTHON_TEMPLATE = """#include "basic_components.h"
#include <vector>
#include <complex>

using namespace qram_simulator;

{USER_CPP_CODE}

extern "C" BaseOperator* create_operator({CTOR_PARAMS}) {{
    return new {CLASS_NAME}({CTOR_ARGS});
}}

extern "C" void destroy_operator(BaseOperator* op) {{
    delete op;
}}

extern "C" const char* get_operator_name() {{
    return "{CLASS_NAME}";
}}

// Python 调用辅助函数 - 应用算子到 SparseState
// Python 侧通过 state._cpp_ptr() 获取 C++ SparseState* 指针，
// ctypes 将其作为 ctypes.c_void_p 传递。
extern "C" void apply_operator(BaseOperator* op, SparseState* state) {{
    if (op && state) {{
        (*op)(*state);
    }}
}}

// Python 调用辅助函数 - 应用 dagger
extern "C" void apply_operator_dag(BaseOperator* op, SparseState* state) {{
    if (op && state) {{
        op->dag(*state);
    }}
}}

// 获取基类类型
extern "C" const char* get_base_class() {{
    return "{BASE_CLASS}";
}}
"""

    def __init__(
        self,
        cxx: str = "g++",
        std: str = "c++17",
        opt_level: str = "O2",
        include_paths: Optional[list] = None,
        lib_paths: Optional[list] = None,
        libraries: Optional[list] = None,
        extra_flags: Optional[list] = None,
        template: Optional[str] = None,
    ):
        """
        初始化编译器配置

        Args:
            cxx: C++ 编译器命令（默认 g++）
            std: C++ 标准版本（默认 c++17）
            opt_level: 优化级别（默认 O2）
            include_paths: 额外的头文件搜索路径
            lib_paths: 额外的库文件搜索路径
            libraries: 需要链接的库
            extra_flags: 额外的编译器标志
            template: 自定义代码模板
        """
        self.cxx = cxx
        self.std = std
        self.opt_level = opt_level
        self.include_paths = include_paths or []
        self.lib_paths = lib_paths or []
        self.libraries = libraries or []
        self.extra_flags = extra_flags or []
        self.template = template or self.DEFAULT_TEMPLATE

    def get_compile_flags(self) -> list:
        """生成编译器标志列表"""
        flags = [
            f"-std={self.std}",
            f"-{self.opt_level}",
            "-fPIC",  # 位置无关代码（共享库必需）
            "-shared",  # 生成共享库
        ]

        # 添加头文件搜索路径
        for path in self.include_paths:
            flags.append(f"-I{path}")

        # 添加库文件搜索路径
        for path in self.lib_paths:
            flags.append(f"-L{path}")

        # 添加链接的库
        for lib in self.libraries:
            flags.append(f"-l{lib}")

        # 添加额外标志
        flags.extend(self.extra_flags)

        return flags


def compute_code_hash(cpp_code: str, class_name: str, config: CompilerConfig) -> str:
    """
    计算代码哈希值，用于缓存

    哈希包括：代码内容、类名、编译器版本和配置

    Args:
        cpp_code: 用户 C++ 代码
        class_name: 算子类名
        config: 编译器配置

    Returns:
        16 字符的十六进制哈希字符串
    """
    # 获取编译器版本信息（影响 ABI）
    compiler_version = ""
    try:
        result = subprocess.run(
            [config.cxx, "--version"],
            capture_output=True,
            text=True,
            timeout=5,
        )
        if result.returncode == 0:
            compiler_version = result.stdout.strip()[:100]  # 取前100字符
    except Exception:
        pass

    # 组合哈希内容
    hash_content = "|".join([
        cpp_code,
        class_name,
        config.std,
        config.opt_level,
        ",".join(sorted(config.include_paths)),
        compiler_version,
    ])

    return hashlib.sha256(hash_content.encode("utf-8")).hexdigest()[:16]


def find_project_root() -> Optional[Path]:
    """
    查找项目根目录或已安装的包目录

    对于已安装的包，目录结构为:
    - site-packages/pysparq/ (Python包)
    - site-packages/include/ (头文件，包含 basic_components.h)

    对于源代码目录:
    - 项目根目录包含 SparQ/ 和 PySparQ/

    Returns:
        项目根目录路径或已安装的包目录，未找到返回 None
    """
    current = Path(__file__).resolve().parent

    # 检查是否在已安装的包中 (site-packages/pysparq/dynamic_operator)
    # 在这种情况下，头文件在 site-packages/include/
    for parent in [current] + list(current.parents):
        # 已安装的包的情况：检查 include/basic_components.h 是否存在
        # 这确保是完整的头文件目录，而不是 PySparQ/include（只有绑定头文件）
        if (parent / "include" / "basic_components.h").exists() and (parent / "pysparq").exists():
            return parent
        # 源代码目录的情况：检查大写的 SparQ/ 和 PySparQ/
        if (parent / "SparQ").exists() and (parent / "PySparQ").exists():
            return parent
    return None


def generate_cpp_source(
    cpp_code: str,
    class_name: str,
    ctor_params: str = "",
    ctor_args: str = "",
    config: Optional[CompilerConfig] = None,
) -> str:
    """
    生成完整的 C++ 源文件

    Args:
        cpp_code: 用户提供的 C++ 代码（包含类定义）
        class_name: 算子类名
        ctor_params: 构造函数参数声明（如 "int n, double theta"）
        ctor_args: 构造函数参数调用（如 "n, theta"）
        config: 编译器配置（使用模板）

    Returns:
        完整的 C++ 源代码字符串
    """
    cfg = config or CompilerConfig()
    template = cfg.template

    # 替换模板变量
    source = template.format(
        USER_CPP_CODE=cpp_code,
        CLASS_NAME=class_name,
        CTOR_PARAMS=ctor_params,
        CTOR_ARGS=ctor_args,
    )

    return source


def compile_cpp_code(
    cpp_code: str,
    class_name: str,
    cache_dir: Optional[str] = None,
    ctor_params: str = "",
    ctor_args: str = "",
    config: Optional[CompilerConfig] = None,
    project_root: Optional[str] = None,
    verbose: bool = False,
) -> str:
    """
    编译 C++ 代码为共享库

    Args:
        cpp_code: 用户提供的 C++ 代码（包含类定义）
        class_name: 算子类名
        cache_dir: 缓存目录（默认使用系统临时目录）
        ctor_params: 构造函数参数声明
        ctor_args: 构造函数参数调用
        config: 编译器配置
        project_root: 项目根目录（自动检测）
        verbose: 是否输出详细日志

    Returns:
        编译后的共享库路径 (.so 文件)

    Raises:
        CompilationError: 编译失败
        FileNotFoundError: 找不到编译器
    """
    cfg = config or CompilerConfig()

    # 自动检测项目根目录
    if project_root is None:
        detected_root = find_project_root()
        if detected_root is None:
            raise RuntimeError(
                "无法自动检测项目根目录，请手动指定 project_root 参数"
            )
        project_root = str(detected_root)

    project_root_path = Path(project_root)

    # 检查是否是已安装的包（头文件在 include/ 目录下）
    installed_include = project_root_path / "include"
    if installed_include.exists():
        # 已安装的包情况：头文件已经在统一的 include/ 目录下
        if str(installed_include) not in cfg.include_paths:
            cfg.include_paths.insert(0, str(installed_include))
    else:
        # 源代码目录情况：头文件分散在多个子目录
        sparq_include = project_root_path / "SparQ" / "include"
        if sparq_include.exists() and str(sparq_include) not in cfg.include_paths:
            cfg.include_paths.insert(0, str(sparq_include))

        qram_include = project_root_path / "QRAM" / "include"
        if qram_include.exists() and str(qram_include) not in cfg.include_paths:
            cfg.include_paths.insert(0, str(qram_include))

        common_include = project_root_path / "Common" / "include"
        if common_include.exists() and str(common_include) not in cfg.include_paths:
            cfg.include_paths.insert(0, str(common_include))

        # 添加 Eigen 头文件路径
        eigen_include = project_root_path / "ThirdParty" / "eigen-3.4.0"
        if eigen_include.exists() and str(eigen_include) not in cfg.include_paths:
            cfg.include_paths.insert(0, str(eigen_include))

        # 添加 fmt 头文件路径
        fmt_include = project_root_path / "ThirdParty" / "fmt" / "include"
        if fmt_include.exists() and str(fmt_include) not in cfg.include_paths:
            cfg.include_paths.insert(0, str(fmt_include))

    # 计算哈希值
    code_hash = compute_code_hash(cpp_code, class_name, cfg)

    # 确定缓存目录
    if cache_dir is None:
        cache_dir = os.path.join(tempfile.gettempdir(), "pysparq_dynamic_ops")

    os.makedirs(cache_dir, exist_ok=True)

    # 生成库文件名
    lib_filename = f"{class_name}_{code_hash}.so"
    lib_path = os.path.join(cache_dir, lib_filename)

    # 检查缓存
    if os.path.exists(lib_path):
        if verbose:
            print(f"[compiler] 使用缓存: {lib_path}")
        return lib_path

    # 生成完整源代码
    full_source = generate_cpp_source(cpp_code, class_name, ctor_params, ctor_args, cfg)

    # 创建临时源文件
    source_filename = f"{class_name}_{code_hash}.cpp"
    source_path = os.path.join(cache_dir, source_filename)

    with open(source_path, "w", encoding="utf-8") as f:
        f.write(full_source)

    if verbose:
        print(f"[compiler] 源文件: {source_path}")
        print(f"[compiler] 目标库: {lib_path}")

    # 检查编译器
    if not shutil.which(cfg.cxx):
        raise FileNotFoundError(f"找不到 C++ 编译器: {cfg.cxx}")

    # 构建编译命令
    cmd = [cfg.cxx] + cfg.get_compile_flags() + ["-o", lib_path, source_path]

    if verbose:
        print(f"[compiler] 编译命令: {' '.join(cmd)}")

    # 执行编译
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=120,  # 2分钟超时
        )
    except subprocess.TimeoutExpired:
        raise CompilationError("编译超时（超过2分钟）")
    except Exception as e:
        raise CompilationError(f"编译器执行失败: {e}")

    # 检查编译结果
    if result.returncode != 0:
        # 格式化错误信息
        formatted_error = format_compile_error(result.stderr, source_path)
        raise CompilationError(
            f"编译失败 (返回码: {result.returncode})",
            stderr=formatted_error,
            returncode=result.returncode,
        )

    # 编译成功，删除源文件（可选，保留用于调试）
    # os.remove(source_path)

    if verbose:
        print(f"[compiler] 编译成功: {lib_path}")

    return lib_path


def format_compile_error(stderr: str, source_path: str) -> str:
    """
    格式化编译错误输出

    - 简化文件路径
    - 高亮错误行
    - 提取关键错误信息

    Args:
        stderr: 编译器标准错误输出
        source_path: 源文件路径

    Returns:
        格式化后的错误信息
    """
    if not stderr:
        return "未知编译错误"

    lines = stderr.strip().split("\n")
    formatted_lines = []

    # 错误模式匹配
    error_patterns = [
        r"(.*?):(\d+):(\d+):\s*(error|warning):\s*(.*)",  # GCC/Clang 格式
        r"(.*?):\s*(error|warning)\s*\w*:\s*(.*)",  # 另一种格式
    ]

    for line in lines:
        line = line.strip()
        if not line:
            continue

        # 简化路径
        if source_path in line:
            line = line.replace(source_path, "<source>")

        formatted_lines.append(line)

    # 提取错误摘要
    errors = []
    warnings = []

    for line in formatted_lines:
        if "error:" in line.lower():
            errors.append(line)
        elif "warning:" in line.lower():
            warnings.append(line)

    # 构建输出
    output = []

    if errors:
        output.append(f"错误 ({len(errors)} 个):")
        for err in errors[:5]:  # 只显示前5个错误
            output.append(f"  - {err}")
        if len(errors) > 5:
            output.append(f"  ... 还有 {len(errors) - 5} 个错误")

    if warnings:
        output.append(f"\n警告 ({len(warnings)} 个):")
        for warn in warnings[:3]:  # 只显示前3个警告
            output.append(f"  - {warn}")

    if not errors and not warnings:
        output.append("编译器输出:")
        output.extend(formatted_lines[:20])  # 最多20行

    return "\n".join(output)


def clear_cache(cache_dir: Optional[str] = None) -> int:
    """
    清除编译缓存

    Args:
        cache_dir: 缓存目录（默认使用系统临时目录）

    Returns:
        删除的文件数量
    """
    if cache_dir is None:
        cache_dir = os.path.join(tempfile.gettempdir(), "pysparq_dynamic_ops")

    if not os.path.exists(cache_dir):
        return 0

    count = 0
    for filename in os.listdir(cache_dir):
        filepath = os.path.join(cache_dir, filename)
        try:
            if os.path.isfile(filepath):
                os.remove(filepath)
                count += 1
        except Exception as e:
            warnings.warn(f"Failed to remove cached file {filepath}: {e}")
            raise

    return count


def get_cache_info(cache_dir: Optional[str] = None) -> dict:
    """
    获取缓存信息

    Args:
        cache_dir: 缓存目录

    Returns:
        包含缓存统计信息的字典
    """
    if cache_dir is None:
        cache_dir = os.path.join(tempfile.gettempdir(), "pysparq_dynamic_ops")

    info = {
        "cache_dir": cache_dir,
        "exists": os.path.exists(cache_dir),
        "file_count": 0,
        "so_count": 0,
        "cpp_count": 0,
        "total_size_mb": 0.0,
    }

    if not info["exists"]:
        return info

    total_size = 0
    for filename in os.listdir(cache_dir):
        filepath = os.path.join(cache_dir, filename)
        if os.path.isfile(filepath):
            info["file_count"] += 1
            total_size += os.path.getsize(filepath)

            if filename.endswith(".so"):
                info["so_count"] += 1
            elif filename.endswith(".cpp"):
                info["cpp_count"] += 1

    info["total_size_mb"] = round(total_size / (1024 * 1024), 2)

    return info


# ========== 便捷函数 ==========

def quick_compile(
    class_code: str,
    class_name: str,
    verbose: bool = False,
) -> str:
    """
    快速编译 C++ 算子代码

    Args:
        class_code: 包含类定义的 C++ 代码
        class_name: 类名
        verbose: 是否输出详细日志

    Returns:
        共享库文件路径
    """
    return compile_cpp_code(
        cpp_code=class_code,
        class_name=class_name,
        verbose=verbose,
    )
