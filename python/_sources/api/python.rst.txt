Python API 参考
===============

PySparQ 通过 :mod:`pysparq` 模块将所有量子操作暴露为 Python 类和函数。

.. _api-modules:

核心类
------

.. autoclass:: pysparq._core.System
   :members:
   :show-inheritance:

.. autoclass:: pysparq._core.SparseState
   :members:
   :show-inheritance:

量子算术算子
------------

.. autofunction:: pysparq._core.Add_UInt_UInt

.. autofunction:: pysparq._core.Add_UInt_ConstUInt

.. autofunction:: pysparq._core.Add_ConstUInt

.. autofunction:: pysparq._core.Mult_UInt_ConstUInt

.. autofunction:: pysparq._core.Mod_Mult_UInt_ConstUInt

.. autofunction:: pysparq._core.AddAssign_AnyInt_AnyInt

量子门
------

.. autofunction:: pysparq._core.Hadamard_Int

.. autofunction:: pysparq._core.Hadamard_Bool

.. autofunction:: pysparq._core.QFT

.. autofunction:: pysparq._core.inverseQFT

.. autofunction:: pysparq._core.Xgate_Bool

.. autofunction:: pysparq._core.Ygate_Bool

.. autofunction:: pysparq._core.Zgate_Bool

QRAM 操作
---------

.. autofunction:: pysparq._core.QRAMLoad

.. autofunction:: pysparq._core.QRAMLoadFast

.. autofunction:: pysparq._core.QRAMCircuit_qutrit

状态管理
--------

.. autofunction:: pysparq._core.AddRegister

.. autofunction:: pysparq._core.RemoveRegister

.. autofunction:: pysparq._core.Push

.. autofunction:: pysparq._core.Pop

.. autofunction:: pysparq._core.Normalize

.. autofunction:: pysparq._core.CheckNormalization

完整 API 文档
-------------

完整的 API 文档（所有类、函数和方法），请参见 :ref:`modindex`。
