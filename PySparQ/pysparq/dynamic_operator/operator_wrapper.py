"""
动态算子包装模块

使用 ctypes 调用动态库中的 C++ 算子，创建 Python 代理类。
"""

import ctypes
import os
import warnings
import weakref
from typing import Any, Callable, List, Tuple, Type

# 从 pysparq 导入基类（可选，用于类型提示）
try:
    from pysparq import SparseState
except ImportError:
    # pysparq 未编译时，定义占位符
    SparseState = None


class DynamicOperatorError(Exception):
    """动态算子错误"""
    pass


class DynamicOperatorLoadError(DynamicOperatorError):
    """动态库加载错误"""
    pass


class DynamicOperatorFactoryError(DynamicOperatorError):
    """工厂函数调用错误"""
    pass


# 存储活跃实例用于清理
_active_instances = {}
_instance_counter = [0]


def _register_instance(instance):
    """注册实例用于跟踪"""
    instance_id = _instance_counter[0]
    _instance_counter[0] += 1
    _active_instances[instance_id] = weakref.ref(instance)
    return instance_id


def _unregister_instance(instance_id):
    """注销实例"""
    if instance_id in _active_instances:
        del _active_instances[instance_id]


class CppOperatorWrapper:
    """
    C++ 算子包装器
    
    负责加载动态库、调用工厂函数创建/销毁 C++ 算子对象
    """
    
    def __init__(self, lib_path: str):
        """
        初始化包装器
        
        Args:
            lib_path: 动态库路径
        """
        self.lib_path = lib_path
        self._handle = None
        self._create_func = None
        self._destroy_func = None
        self._get_name_func = None
        self._get_base_class_func = None
        self._apply_func = None
        self._apply_dag_func = None
        self._arg_types = []
        
    def load(self, arg_types: List[str] = None):
        """
        加载动态库
        
        Args:
            arg_types: 构造函数参数类型列表
            
        Raises:
            DynamicOperatorLoadError: 加载失败
        """
        if not os.path.exists(self.lib_path):
            raise DynamicOperatorLoadError(f"动态库不存在: {self.lib_path}")
        
        try:
            # 使用 RTLD_GLOBAL 以便解析符号
            self._handle = ctypes.CDLL(self.lib_path, mode=ctypes.RTLD_GLOBAL)
        except OSError as e:
            raise DynamicOperatorLoadError(f"加载动态库失败: {e}")
        
        # 获取工厂函数
        try:
            self._create_func = self._handle.create_operator
            self._destroy_func = self._handle.destroy_operator
            self._get_name_func = self._handle.get_operator_name
            self._get_base_class_func = self._handle.get_base_class
            
            # Python 增强函数 - 通过 state._cpp_ptr() 获取 C++ SparseState* 指针
            try:
                self._apply_func = self._handle.apply_operator
                self._apply_dag_func = self._handle.apply_operator_dag
            except AttributeError:
                pass  # 旧版本模板没有这些函数
                
        except AttributeError as e:
            raise DynamicOperatorLoadError(f"找不到必需的工厂函数: {e}")
        
        # 设置参数类型
        if arg_types:
            self._arg_types = arg_types
            self._setup_arg_types()
    
    def _setup_arg_types(self):
        """设置函数参数类型"""
        if not self._create_func:
            return
            
        # 根据参数类型设置 argtypes
        type_mapping = {
            'int': ctypes.c_int,
            'size_t': ctypes.c_size_t,
            'unsigned int': ctypes.c_uint,
            'unsigned long': ctypes.c_ulong,
            'unsigned long long': ctypes.c_ulonglong,
            'long': ctypes.c_long,
            'long long': ctypes.c_longlong,
            'float': ctypes.c_float,
            'double': ctypes.c_double,
            'bool': ctypes.c_bool,
            'char': ctypes.c_char,
            'char*': ctypes.c_char_p,
            'const char*': ctypes.c_char_p,
        }
        
        argtypes = []
        for arg_type in self._arg_types:
            ctype = type_mapping.get(arg_type)
            if ctype is None:
                # 默认使用 size_t
                ctype = ctypes.c_size_t
            argtypes.append(ctype)
        
        self._create_func.argtypes = argtypes
        self._create_func.restype = ctypes.c_void_p
        
        self._destroy_func.argtypes = [ctypes.c_void_p]
        self._destroy_func.restype = None
        
        if self._get_name_func:
            self._get_name_func.argtypes = []
            self._get_name_func.restype = ctypes.c_char_p
        
        if self._get_base_class_func:
            self._get_base_class_func.argtypes = []
            self._get_base_class_func.restype = ctypes.c_char_p
            
        if self._apply_func:
            # SparseState* 参数：ctypes.c_void_p 传递指针值
            self._apply_func.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
            self._apply_func.restype = None

        if self._apply_dag_func:
            self._apply_dag_func.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
            self._apply_dag_func.restype = None
    
    def create(self, *args) -> int:
        """
        创建 C++ 算子实例
        
        Args:
            *args: 构造函数参数
            
        Returns:
            C++ 对象地址（作为 Python int）
        """
        if not self._create_func:
            raise DynamicOperatorFactoryError("工厂函数未加载")
        
        try:
            ptr = self._create_func(*args)
            return ptr
        except Exception as e:
            raise DynamicOperatorFactoryError(f"创建算子失败: {e}")
    
    def destroy(self, ptr: int):
        """
        销毁 C++ 算子实例
        
        Args:
            ptr: C++ 对象地址
        """
        if self._destroy_func and ptr:
            try:
                self._destroy_func(ptr)
            except Exception as e:
                warnings.warn(f"Failed to destroy C++ object at {ptr}: {e}")
                raise
    
    def close(self):
        """
        关闭动态库并释放资源
        
        注意：在 Windows 上，必须确保所有 C++ 对象都已销毁
        才能成功删除动态库文件
        """
        # 清理函数引用，帮助垃圾回收
        self._create_func = None
        self._destroy_func = None
        self._get_name_func = None
        self._get_base_class_func = None
        self._apply_func = None
        self._apply_dag_func = None
        
        # 释放动态库句柄
        if self._handle is not None:
            # 在 Windows 上，需要强制垃圾回收以确保句柄释放
            import gc
            gc.collect()
            
            # 删除 handle 引用，让 ctypes 释放库
            handle = self._handle
            self._handle = None
            
            # Windows 特定：强制释放库句柄
            if os.name == 'nt':
                try:
                    import ctypes
                    kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
                    # 获取模块句柄并释放
                    hmodule = ctypes.c_void_p(handle._handle)
                    if hmodule:
                        kernel32.FreeLibrary(hmodule)
                except Exception as e:
                    warnings.warn(f"Failed to FreeLibrary: {e}")
                    raise
            
            # 删除 handle 对象
            del handle
            
            # 再次强制垃圾回收
            gc.collect()
    
    def get_name(self) -> str:
        """获取算子名称"""
        if self._get_name_func:
            result = self._get_name_func()
            if result:
                return result.decode('utf-8')
        return ""
    
    def get_base_class(self) -> str:
        """获取基类名称"""
        if self._get_base_class_func:
            result = self._get_base_class_func()
            if result:
                return result.decode('utf-8')
        return "BaseOperator"
    
    def apply(self, ptr: int, state_cpp_ptr: int):
        """
        应用算子到 SparseState

        Args:
            ptr: 算子对象地址
            state_cpp_ptr: C++ SparseState* 指针（通过 state._cpp_ptr() 获取）
        """
        if self._apply_func and ptr and state_cpp_ptr:
            self._apply_func(ptr, state_cpp_ptr)

    def apply_dag(self, ptr: int, state_cpp_ptr: int):
        """
        应用 dagger 到 SparseState

        Args:
            ptr: 算子对象地址
            state_cpp_ptr: C++ SparseState* 指针（通过 state._cpp_ptr() 获取）
        """
        if self._apply_dag_func and ptr and state_cpp_ptr:
            self._apply_dag_func(ptr, state_cpp_ptr)



def create_operator_class(
    name: str,
    lib_path: str,
    base_class: str = "BaseOperator",
    constructor_args: List[Tuple[str, str]] = None
) -> Type:
    """
    创建动态算子 Python 类
    
    Args:
        name: 算子类名
        lib_path: 动态库路径
        base_class: 基类名 ("BaseOperator" 或 "SelfAdjointOperator")
        constructor_args: 构造函数参数列表 [(type, name), ...]
        
    Returns:
        动态创建的算子类
    """
    constructor_args = constructor_args or []
    
    # 创建 C++ 包装器
    wrapper = CppOperatorWrapper(lib_path)
    arg_types = [arg[0] for arg in constructor_args]
    wrapper.load(arg_types)
    
    # 验证基类
    detected_base = wrapper.get_base_class()
    if detected_base and detected_base != base_class:
        import warnings
        warnings.warn(f"检测到基类为 {detected_base}，但指定为 {base_class}")
        base_class = detected_base
    
    def custom_init(self, **kwargs):
        """
        动态算子构造函数

        Args:
            **kwargs: 构造函数参数（按名称传参）
        """
        # 收集参数值
        args = []
        for arg_type, arg_name in constructor_args:
            if arg_name not in kwargs:
                raise TypeError(f"缺少必需参数: {arg_name}")
            args.append(kwargs[arg_name])

        # 存储参数用于 dag
        self._args = tuple(args)
        # 将 wrapper 和 base_class 存储在类上（而非实例上），
        # 这样所有实例共享同一个 wrapper，避免 __del__ 被调用时误关动态库。
        self._wrapper = DynamicOpClass._wrapper
        self._base_class = DynamicOpClass._base_class
        self._instance_id = _register_instance(self)

        # 创建 C++ 算子实例
        self._cpp_ptr = self._wrapper.create(*args)
    
    def call_method(self, state):
        """
        调用算子

        Args:
            state: SparseState 对象

        Returns:
            返回输入状态（支持链式调用）
        """
        if not self._cpp_ptr:
            raise RuntimeError("算子未初始化或已销毁")

        # 通过 state._cpp_ptr()（在 pysparq._core.SparseState 中暴露）获取 C++ SparseState* 指针
        state_cpp_ptr = state._cpp_ptr()
        self._wrapper.apply(self._cpp_ptr, state_cpp_ptr)

        return state

    def dag_method(self, state):
        """
        调用 dagger 操作

        Args:
            state: SparseState 对象

        Returns:
            返回输入状态
        """
        if not self._cpp_ptr:
            raise RuntimeError("算子未初始化或已销毁")

        state_cpp_ptr = state._cpp_ptr()

        if base_class == "SelfAdjointOperator":
            # 自伴算子 dagger 等于自身
            self._wrapper.apply(self._cpp_ptr, state_cpp_ptr)
        else:
            # BaseOperator 使用 dagger 辅助函数
            self._wrapper.apply_dag(self._cpp_ptr, state_cpp_ptr)

        return state
    
    def repr_method(self) -> str:
        """字符串表示"""
        arg_str = ", ".join(f"{arg_name}={repr(val)}" for (arg_type, arg_name), val in zip(
            constructor_args, self._args
        ))
        return f"{name}({arg_str})"
    
    def del_method(self):
        """析构函数"""
        if hasattr(self, '_cpp_ptr') and self._cpp_ptr:
            self._wrapper.destroy(self._cpp_ptr)
            self._cpp_ptr = 0
        if hasattr(self, '_instance_id'):
            _unregister_instance(self._instance_id)
        # 注意：不要在这里关闭 wrapper，因为 wrapper 由类级别管理，跨所有实例共享。
        # 动态库的关闭由 cleanup_all_instances() 或显式调用负责。
    
    # 创建动态类
    DynamicOpClass = type(
        name,
        (object,),
        {
            '__init__': custom_init,
            '__call__': call_method,
            'dag': dag_method,
            '__repr__': repr_method,
            '__del__': del_method,
            '_is_dynamic_operator': True,
            '_base_class': base_class,
            '_lib_path': lib_path,
        }
    )
    
    # 将 wrapper 和 base_class 存储在类级别（跨实例共享）
    DynamicOpClass._wrapper = wrapper
    DynamicOpClass._base_class = base_class

    # 添加文档字符串
    arg_docs = "\n".join(f"        {arg_name} ({arg_type})" for arg_type, arg_name in constructor_args) if constructor_args else "        (无)"
    DynamicOpClass.__doc__ = f"""
动态生成的算子类: {name}

基类: {base_class}

构造函数参数:
{arg_docs}

使用示例:
    >>> op = {name}({', '.join(f"{arg_name}=..." for _, arg_name in constructor_args) if constructor_args else ''})
    >>> state = op(state)
"""
    
    return DynamicOpClass


def cleanup_all_instances():
    """清理所有活跃的动态算子实例"""
    import gc
    
    # 清理仍然存在的实例
    for instance_id, ref in list(_active_instances.items()):
        instance = ref()
        if instance is not None:
            try:
                # 先销毁 C++ 对象
                if hasattr(instance, '_cpp_ptr') and instance._cpp_ptr:
                    instance._wrapper.destroy(instance._cpp_ptr)
                    instance._cpp_ptr = 0
                # 关闭动态库句柄
                if hasattr(instance, '_wrapper'):
                    instance._wrapper.close()
            except Exception as e:
                warnings.warn(f"Cleanup failed during _cleanup_active_instances: {e}")
                raise
    
    _active_instances.clear()
    
    # 强制垃圾回收确保资源释放
    gc.collect()
