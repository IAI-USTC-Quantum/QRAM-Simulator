系统操作
========

系统操作管理寄存器生命周期、基态的拆分与合并、以及零振幅清理。这些操作虽然不直接对应量子门，但在算法流程中不可或缺。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 系统操作总览
   :header-rows: 1

   * - 算子 / 函数
     - 操作
     - 类型
   * - ``Push``
     - 将寄存器压入栈，保存当前值
     - BaseOperator
   * - ``Pop``
     - 从栈中恢复寄存器值
     - SelfAdjointOperator
   * - ``ClearZero``
     - 清除振幅接近零的基态
     - SelfAdjointOperator
   * - ``split_systems``
     - 按条件拆分基态集合
     - 自由函数
   * - ``combine_systems``
     - 合并基态集合
     - 自由函数

---

寄存器栈管理
------------

Push（压栈）
^^^^^^^^^^^^

.. autoclass:: pysparq.Push
   :members:
   :undoc-members:

**操作**: 将寄存器当前值保存到内部栈中，并将寄存器重置为 ``|0⟩``。

**参数**:

- ``reg`` — 目标寄存器（名称或 ID）
- ``garbage_name`` — 栈中保存用的临时名称（可选）

**用途**: 在需要临时使用一个寄存器作为辅助（ancilla）时，先保存其值，使用完毕后通过 ``Pop`` 恢复。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("data", ps.UnsignedInteger, 4)
   ps.System.add_register("temp", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("data", 7)(state)
   ps.Init_Unsafe("temp", 3)(state)

   # 保存 temp 的当前值，重置为 0
   ps.Push("temp")(state)

   # 现在可以自由使用 temp 作为辅助寄存器
   ps.Add_UInt_UInt("data", "temp", "temp")(state)

Pop（弹栈）
^^^^^^^^^^^

.. autoclass:: pysparq.Pop
   :members:
   :undoc-members:

**操作**: 从栈中恢复寄存器的值。

**参数**: ``reg`` — 要恢复的寄存器（名称或 ID）

.. warning::

   ``Pop`` 要求栈中保存的值与当前其他寄存器无纠缠，否则行为未定义。通常应与 ``Push`` 成对使用。

.. code-block:: python

   # 恢复 temp 到 Push 前的值
   ps.Pop("temp")(state)

Push/Pop 模式
"""""""""""""

``Push`` / ``Pop`` 的典型工作流：

.. code-block:: python

   # 1. 保存辅助寄存器
   ps.Push("ancilla")(state)

   # 2. 使用辅助寄存器进行计算
   ps.Add_UInt_UInt("a", "ancilla", "ancilla")(state)
   # ... 更多操作 ...
   ps.Add_UInt_UInt("a", "ancilla", "ancilla")(state)  # 撤销（SelfAdjoint）

   # 3. 恢复辅助寄存器
   ps.Pop("ancilla")(state)

---

基态清理
--------

ClearZero（清除零振幅）
^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.ClearZero
   :members:
   :undoc-members:

**操作**: 遍历所有基态，移除振幅绝对值低于阈值的项。

**参数**: ``eps``（可选）— 截断阈值，默认为 ``1e-12``。

**用途**: 经过多次运算后，某些基态的振幅可能因浮点误差变得极小但不为零。``ClearZero`` 可清除这些数值噪声，保持稀疏态的紧凑性。

.. code-block:: python

   ps.ClearZero()(state)       # 使用默认阈值
   ps.ClearZero(1e-8)(state)   # 自定义阈值

---

基态拆分与合并
--------------

split_systems（拆分基态）
^^^^^^^^^^^^^^^^^^^^^^^^^

.. autofunction:: pysparq.split_systems

**操作**: 根据 conditioned_by 条件将 ``SparseState`` 中的基态分为满足条件和不满足条件两组。

**参数**:

- ``state`` — 源 ``SparseState``
- ``nonzeros`` / ``all_ones`` / ``by_bit`` / ``by_value`` — 条件参数

**返回**: ``list[System]`` — 满足条件的基态列表。

.. code-block:: python

   # 拆分出 ctrl 寄存器非零的基态
   matching = ps.split_systems(state, nonzeros=["ctrl"])

combine_systems（合并基态）
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autofunction:: pysparq.combine_systems

**操作**: 将拆分出的基态重新合并回 ``SparseState``。

**参数**:

- ``to`` — 目标 ``SparseState``
- ``from`` — 要合并的基态列表

.. code-block:: python

   # 合并回主状态
   ps.combine_systems(state, matching)

拆分-变换-合并模式
""""""""""""""""""

``split_systems`` / ``combine_systems`` 可用于实现手工条件操作：

.. code-block:: python

   # 1. 拆分
   matching = ps.split_systems(state, nonzeros=["ctrl"])

   # 2. 对匹配的基态执行操作
   # ...

   # 3. 合并
   ps.combine_systems(state, matching)
