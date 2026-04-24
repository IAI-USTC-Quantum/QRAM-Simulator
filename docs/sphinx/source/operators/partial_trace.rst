部分追迹
========

部分追迹（Partial Trace）操作对指定寄存器执行测量或选择性坍缩。这是量子算法中提取经典信息的关键手段。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 部分追迹算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 返回值
   * - ``PartialTrace``
     - 随机测量指定寄存器
     - ``(measured_values, probability)``
   * - ``PartialTraceSelect``
     - 坍缩到指定值
     - ``probability``
   * - ``PartialTraceSelectRange``
     - 坍缩到指定范围内
     - ``probability``

部分追迹的物理含义是将某个寄存器从量子态中"测量掉"——测量后该寄存器坍缩到确定值，其余寄存器根据测量结果进入相应的条件态。

---

PartialTrace（随机测量）
-----------------------

.. autoclass:: pysparq.PartialTrace
   :members:
   :undoc-members:

**操作**: 对指定寄存器执行随机测量（按概率分布抽样），返回测量结果和对应概率。

**参数**: 寄存器标识（名称字符串、名称列表、ID 或 ID 列表）。

**返回**: ``(measured_values, probability)``

- ``measured_values`` — 测量得到的寄存器值列表
- ``probability`` — 该测量结果的概率

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("addr", ps.UnsignedInteger, 3)
   ps.System.add_register("data", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Hadamard_Int("addr", 3)(state)

   # 随机测量 addr 寄存器
   values, prob = ps.PartialTrace("addr")(state)
   print(f"测量结果: {values}, 概率: {prob:.4f}")
   # 例如: 测量结果: [3], 概率: 0.1250

   # 测量后 addr 已坍缩，state 中仅保留对应基态

测量多个寄存器时，返回值列表按参数顺序排列：

.. code-block:: python

   values, prob = ps.PartialTrace(["addr", "data"])(state)

---

PartialTraceSelect（选择性坍缩）
--------------------------------

.. autoclass:: pysparq.PartialTraceSelect
   :members:
   :undoc-members:

**操作**: 将指定寄存器坍缩到给定的值，而非随机测量。

**参数**: 寄存器-值映射（字典或列表）。

**返回**: ``probability`` — 该坍缩结果的概率。

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("a", ps.UnsignedInteger, 2)
   ps.System.add_register("b", ps.UnsignedInteger, 2)

   state = ps.SparseState()
   ps.Hadamard_Int("a", 2)(state)

   # 将 a 坍缩到值 1
   prob = ps.PartialTraceSelect({"a": 1})(state)
   print(f"坍缩概率: {prob:.4f}")
   # 坍缩概率: 0.2500

   # 也可以用列表形式
   prob = ps.PartialTraceSelect(["a"], [1])(state)

---

PartialTraceSelectRange（范围坍缩）
-----------------------------------

.. autoclass:: pysparq.PartialTraceSelectRange
   :members:
   :undoc-members:

**操作**: 将指定寄存器坍缩到给定值范围内，保留满足条件的基态并重新归一化。

**参数**: 寄存器标识和值范围 ``(min, max)``。

**返回**: ``probability`` — 范围内的总概率。

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("x", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Hadamard_Int("x", 4)(state)

   # 仅保留 x ∈ [2, 5] 的基态
   prob = ps.PartialTraceSelectRange("x", (2, 5))(state)
   print(f"范围内概率: {prob:.4f}")
   # 范围内概率: 0.2500（4/16 个基态）
