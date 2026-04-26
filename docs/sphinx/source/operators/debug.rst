调试工具
========

调试工具提供量子态检查、归一化验证、NaN 检测和状态打印等功能。这些算子不改变量子态（除 ``CheckNormalization_Renormalize``），用于开发和调试阶段。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 调试工具总览
   :header-rows: 1

   * - 算子
     - 操作
     - 是否修改态
   * - ``StatePrint``
     - 打印量子态
     - 否
   * - ``CheckNormalization``
     - 检查归一化
     - 否
   * - ``CheckNan``
     - 检测 NaN
     - 否
   * - ``ViewNormalization``
     - 显示归一化值
     - 否
   * - ``TestRemovable``
     - 测试寄存器可移除性
     - 否
   * - ``CheckDuplicateKey``
     - 检测重复键
     - 否

---

状态打印
--------

StatePrint（状态打印）
^^^^^^^^^^^^^^^^^^^^^^

.. autofunction:: pysparq.StatePrint
.. autofunction:: pysparq.print
.. autofunction:: pysparq.to_string
.. autoclass:: pysparq.StatePrinter
   :members:

**操作**: 以指定格式返回 ``SparseState`` 中所有基态的信息字符串。

**参数**:

- ``state`` — SparseState 实例
- ``mode`` — 显示模式（可选，默认 ``Detail``）
- ``precision`` — 浮点精度（可选）

**显示模式**:

.. list-table:: StatePrint 显示模式
   :header-rows: 1

   * - 模式
     - 说明
   * - ``Default``
     - 标准格式 ``|reg=val⟩ : (α+βi)``
   * - ``Detail``
     - 详细信息，包含寄存器元数据
   * - ``Binary``
     - 以二进制显示寄存器值
   * - ``Prob``
     - 显示概率而非振幅

.. code-block:: python

   import pysparq as ps

   # 打印到 stdout（Detail 模式）
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)addr : UInt4 | |(1)data : UInt8 |
   # 0.250000+0.000000i  addr=|0> data=|0>
   # 0.250000+0.000000i  addr=|1> data=|2>
   # ...

   # 返回字符串（Detail 模式）
   ps.StatePrint(state)
   # 返回相同的 Detail 格式字符串

   # 概率格式（返回字符串）
   ps.StatePrint(state, mode=ps.StatePrintDisplay.Prob)
   # 输出：
   # StatePrint (mode=Prob)
   # 0.250000+0.000000i (p = 0.0625) |0>|0>
   # ...

   # 高精度
   ps.StatePrint(state, mode=ps.StatePrintDisplay.Default, precision=15)

   # 二进制格式
   ps.StatePrint(state, mode=ps.StatePrintDisplay.Binary)
   # 寄存器值以二进制显示

---

归一化检查
----------

CheckNormalization（归一化检查）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.CheckNormalization
   :members:
   :undoc-members:

**操作**: 断言 ``SparseState`` 的归一化值 :math:`\sum_i |\alpha_i|^2` 接近 1。如果偏离超过阈值，抛出异常。

**参数**: ``threshold`` — 允许的偏差阈值（可选，默认 ``1e-6``）。

.. code-block:: python

   # 严格检查
   ps.CheckNormalization(1e-10)(state)

   # 宽松检查
   ps.CheckNormalization(1e-3)(state)

ViewNormalization（显示归一化值）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.ViewNormalization
   :members:
   :undoc-members:

**操作**: 计算并打印 ``SparseState`` 的归一化值 :math:`\sum_i |\alpha_i|^2`，不抛出异常。

**参数**: 无。

.. code-block:: python

   ps.ViewNormalization()(state)
   # 输出: Norm = 0.9999999...

---

数据完整性检查
--------------

CheckNan（NaN 检测）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.CheckNan
   :members:
   :undoc-members:

**操作**: 遍历所有基态，检查振幅是否包含 NaN 值。如果发现 NaN，抛出异常。

**参数**: 无。

**用途**: 经过大量算术运算后，某些振幅可能因数值溢出或除零变为 NaN。此算子用于早期发现这类问题。

.. code-block:: python

   ps.CheckNan()(state)

TestRemovable（寄存器可移除性测试）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.TestRemovable
   :members:
   :undoc-members:

**操作**: 测试指定寄存器是否可以从 ``SparseState`` 中安全移除（即该寄存器是否与其他寄存器纠缠）。

**参数**: ``reg`` — 目标寄存器（名称或 ID）。

**返回**: 通过内部断言报告结果。

.. code-block:: python

   # 测试 "ancilla" 是否可以安全移除
   ps.TestRemovable("ancilla")(state)

CheckDuplicateKey（重复键检测）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.CheckDuplicateKey
   :members:
   :undoc-members:

**操作**: 检查 ``SparseState`` 中是否存在键值重复的基态。正常情况下不应存在重复。

**参数**: 无。

**用途**: 如果怀疑某个非幺正操作引入了重复键，可用此算子检测。

.. code-block:: python

   ps.CheckDuplicateKey()(state)
