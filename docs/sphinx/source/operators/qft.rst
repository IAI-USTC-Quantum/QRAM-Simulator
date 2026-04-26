量子傅里叶变换 (QFT)
===================

量子傅里叶变换是许多量子算法的核心组件。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: QFT 算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正类
   * - ``QFT``
     - 量子傅里叶变换
     - BaseOperator
   * - ``inverseQFT``
     - 逆量子傅里叶变换
     - BaseOperator

数学定义
--------

量子傅里叶变换定义：

.. math::

   QFT|x\rangle = \frac{1}{\sqrt{2^n}} \sum_{y=0}^{2^n-1} e^{2\pi i xy / 2^n} |y\rangle

逆量子傅里叶变换：

.. math::

   QFT^\dagger|y\rangle = \frac{1}{\sqrt{2^n}} \sum_{x=0}^{2^n-1} e^{-2\pi i xy / 2^n} |x\rangle

---

QFT
---

.. autoclass:: pysparq.QFT
   :members:
   :undoc-members:

**操作**: 对整数寄存器执行量子傅里叶变换

**类型约束**: ``UnsignedInteger`` 或 ``SignedInteger``

**位约束**: 寄存器大小决定变换维度

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   # 4 比特寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 4)

   state = ps.SparseState()

   # 初始化为基态 |1⟩
   ps.Init_Unsafe("q", 1)(state)

   print("初始状态:")
   ps.print(state)
   # |q=1⟩ : (1+0j)

   # 应用 QFT
   op = ps.QFT("q")
   op(state)

   print("\nQFT 后:")
   ps.print(state)
   # 产生相位均匀分布的叠加态

   # 撤销（逆变换）
   op.dag(state)

inverseQFT
---------

.. autoclass:: pysparq.inverseQFT
   :members:
   :undoc-members:

**操作**: 对整数寄存器执行逆量子傅里叶变换

**用途**: 撤销 QFT 或解析相位信息

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("q", ps.UnsignedInteger, 4)

   state = ps.SparseState()

   # 创建叠加态
   ps.Hadamard_Int_Full("q")(state)

   # 应用逆 QFT
   ps.inverseQFT("q")(state)

   # 逆 QFT 和 QFT 互为逆运算
   ps.QFT("q")(state)

---

使用场景
--------

周期发现（Shor 算法）
^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   import pysparq as ps
   import numpy as np

   ps.System.clear()

   # 相位寄存器
   n_bits = 8
   ps.System.add_register("phase", ps.UnsignedInteger, n_bits)

   state = ps.SparseState()

   # 模拟相位估计结果（假设周期 r = 5）
   # |phase⟩ 包含周期性相位信息
   ps.Hadamard_Int_Full("phase")(state)

   # 应用逆 QFT 解析周期
   ps.inverseQFT("phase")(state)

   # 观测相位寄存器，确定周期
   # ...

相位估计
^^^^^^^^

.. code-block:: python

   # 假设已有相位估计电路
   # 逆 QFT 将相位信息转换到计算基

   # 1. 制备初态
   ps.Hadamard_Int_Full("estimate")(state)

   # 2. 执行受控幺正操作
   # controlled_U(...)(state)

   # 3. 逆 QFT 提取相位
   ps.inverseQFT("estimate")(state)

   # 4. 测量得到相位估计
   # ...

量子信号处理
^^^^^^^^^^^^

.. code-block:: python

   # QFT 用于频域分析
   ps.System.add_register("signal", ps.UnsignedInteger, 16)

   state = ps.SparseState()
   # 加载信号数据...

   # 变换到频域
   ps.QFT("signal")(state)

   # 频域处理...

   # 变换回时域
   ps.inverseQFT("signal")(state)

性能考虑
--------

- **基态数量**: QFT 作用于叠加态时，会产生 :math:`2^n` 个基态
- **内存限制**: 当 n 较大时（如 n > 20），可能导致内存问题
- **稀疏优化**: PySparQ 只存储非零振幅基态，对于某些输入可能高效

.. note::

   对于非完整的叠加态，QFT 的稀疏实现可能比全态模拟更高效。

实际示例
--------

完整 QFT + inverseQFT 循环
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

   import pysparq as ps
   import numpy as np

   ps.System.clear()

   n = 3
   ps.System.add_register("q", ps.UnsignedInteger, n)

   state = ps.SparseState()

   # 初始值
   initial_value = 5
   ps.Init_Unsafe("q", initial_value)(state)

   print("初始状态:")
   ps.print(state)

   # QFT
   ps.QFT("q")(state)
   print("\nQFT 后:")
   ps.print(state)

   # inverseQFT
   ps.inverseQFT("q")(state)
   print("\ninverseQFT 后:")
   ps.print(state)
   # 恢复到 |q=5⟩