"""
动态算子包装模块

使用 ctypes 调用动态库中的 C++ 算子，创建 Python 代理类。
"""

import ctypes
import os
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
            
            # Python 增强函数（可选，如果存在）
            try:
                self._apply_func = self._handle.apply_operator
                self._apply_dag_func = self._handle.apply_operator_dag
            except AttributeError:
                # 旧版本模板没有这些函数
                pass
                
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
            except Exception:
                pass  # 忽略销毁错误
    
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
    
    def apply(self, ptr: int, state_ptr: int):
        """
        应用算子到状态
        
        Args:
            ptr: 算子对象地址
            state_ptr: 状态对象地址
        """
        if self._apply_func and ptr and state_ptr:
            self._apply_func(ptr, state_ptr)
    
    def apply_dag(self, ptr: int, state_ptr: int):
        """
        应用 dagger 到状态
        
        Args:
            ptr: 算子对象地址
            state_ptr: 状态对象地址
        """
        if self._apply_dag_func and ptr and state_ptr:
            self._apply_dag_func(ptr, state_ptr)


def _get_state_ptr(state):
    """
    获取状态对象的内部指针
    
    这个函数依赖于 pybind11 对象的内部结构。
    我们使用 __int__ 方法（如果存在）或通过 id() 获取地址。
    
    Args:
        state: SparseState 对象
        
    Returns:
        状态对象地址
    """
    # pybind11 对象通常可以通过 __int__() 获取指针
    # 如果没有，我们使用 id() 作为备选
    if hasattr(state, '__int__'):
        return int(state)
    return id(state)


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
        self._wrapper = wrapper
        self._base_class = base_class
        self._instance_id = _register_instance(self)
        
        # 创建 C++ 算子实例
        self._cpp_ptr = wrapper.create(*args)
    
    def call_method(self, state):
        """
        调用算子
        
        Args:
            state: SparseState 或 basis_states 列表
            
        Returns:
            返回输入状态（支持链式调用）
        """
        if not self._cpp_ptr:
            raise RuntimeError("算子未初始化或已销毁")
        
        # 获取状态指针并调用 C++ 算子
        if hasattr(self._wrapper, '_apply_func') and self._wrapper._apply_func:
            # 使用辅助函数调用
            state_ptr = _get_state_ptr(state)
            self._wrapper.apply(self._cpp_ptr, state_ptr)
        else:
            # 回退方案：通过 pysparq 的绑定调用
            # 这需要重新加载库并创建绑定对象
            _call_via_pybind11(self, state)
        
        return state
    
    def dag_method(self, state):
        """
        调用 dagger 操作
        
        Args:
            state: SparseState 或 basis_states 列表
            
        Returns:
            返回输入状态
        """
        if not self._cpp_ptr:
            raise RuntimeError("算子未初始化或已销毁")
        
        if base_class == "SelfAdjointOperator":
            # 自伴算子 dagger 等于自身
            state_ptr = _get_state_ptr(state)
            if hasattr(self._wrapper, '_apply_func') and self._wrapper._apply_func:
                self._wrapper.apply(self._cpp_ptr, state_ptr)
            else:
                _call_via_pybind11(self, state)
        else:
            # BaseOperator 使用 dagger 辅助函数
            if hasattr(self._wrapper, '_apply_dag_func') and self._wrapper._apply_dag_func:
                state_ptr = _get_state_ptr(state)
                self._wrapper.apply_dag(self._cpp_ptr, state_ptr)
            else:
                _call_dag_via_pybind11(self, state)
        
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


def _call_via_pybind11(operator_instance, state):
    """
    通过 pybind11 回退调用算子
    
    这是一个备用方案，当 ctypes 辅助函数不可用时使用。
    这需要重新加载库并通过 pysparq 创建绑定对象。
    """
    raise NotImplementedError(
        "动态算子调用需要 PySparQ 支持。"
        "请确保使用的是支持动态算子的 PySparQ 版本。"
    )


def _call_dag_via_pybind11(operator_instance, state):
    """通过 pybind11 调用 dagger"""
    raise NotImplementedError(
        "动态算子 dagger 调用需要 PySparQ 支持。"
        "请确保使用的是支持动态算子的 PySparQ 版本。"
    )


def cleanup_all_instances():
    """清理所有活跃的动态算子实例"""
    # 由于使用了弱引用，实例会被自动清理
    # 这里我们强制进行垃圾回收
    import gc
    gc.collect()
    
    # 清理仍然存在的实例
    for instance_id, ref in list(_active_instances.items()):
        instance = ref()
        if instance is not None:
            try:
                if hasattr(instance, '_cpp_ptr') and instance._cpp_ptr:
                    instance._wrapper.destroy(instance._cpp_ptr)
                    instance._cpp_ptr = 0
            except:
                pass
    
    _active_instances.clear()
