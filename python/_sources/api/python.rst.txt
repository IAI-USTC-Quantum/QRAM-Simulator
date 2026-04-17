Python API 参考
===============

PySparQ 通过 :mod:`pysparq` 模块将所有量子操作暴露为 Python 类和函数。

.. _api-modules:

核心类
------

- :class:`~pysparq._core.System` - 量子寄存器的系统管理
- :class:`~pysparq._core.SparseState` - 稀疏量子态表示
- :class:`~pysparq._core.BaseOperator` - 量子算子的基类
- :class:`~pysparq._core.SelfAdjointOperator` - 自伴量子算子

量子算术算子
------------

- :func:`~pysparq._core.Add_UInt_UInt` - 两个无符号整数寄存器相加
- :func:`~pysparq._core.Add_UInt_ConstUInt` - 无符号整数寄存器加常数
- :func:`~pysparq._core.Add_ConstUInt` - 加常数无符号整数
- :func:`~pysparq._core.Mult_UInt_ConstUInt` - 无符号整数乘以常数
- :func:`~pysparq._core.AddAssign_AnyInt_AnyInt` - 加法赋值操作

量子门
------

- :func:`~pysparq._core.Hadamard_Int` - 整数寄存器上的 Hadamard 门
- :func:`~pysparq._core.Hadamard_Bool` - 布尔寄存器上的 Hadamard 门
- :func:`~pysparq._core.QFT` - 量子傅里叶变换
- :func:`~pysparq._core.inverseQFT` - 逆量子傅里叶变换
- :func:`~pysparq._core.Xgate_Bool` - 布尔寄存器上的 Pauli X 门
- :func:`~pysparq._core.Ygate_Bool` - 布尔寄存器上的 Pauli Y 门
- :func:`~pysparq._core.Zgate_Bool` - 布尔寄存器上的 Pauli Z 门

QRAM 操作
---------

- :func:`~pysparq._core.QRAMLoad` - QRAM 加载操作
- :func:`~pysparq._core.QRAMLoadFast` - 快速 QRAM 加载操作
- :func:`~pysparq._core.QRAMCircuit_qutrit` - 使用 qutrit 的 QRAM 电路

状态管理
--------

- :func:`~pysparq._core.AddRegister` - 添加新的量子寄存器
- :func:`~pysparq._core.RemoveRegister` - 移除量子寄存器
- :func:`~pysparq._core.Push` - Push 操作
- :func:`~pysparq._core.Pop` - Pop 操作
- :func:`~pysparq._core.Normalize` - 归一化量子态
- :func:`~pysparq._core.CheckNormalization` - 检查态归一化

完整 API 文档
-------------

完整的 API 文档（所有类、函数和方法），请参见 :doc:`API 参考 </autoapi/pysparq/index>`。
