SparseState 类
==============

``SparseState`` 类是 PySparQ 的核心数据结构，表示一个稀疏量子态。它内部托管了 ``std::vector<System>``，只存储振幅非零的基态。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

稀疏表示原理
------------

传统全态矢量模拟器存储 :math:`2^n` 个振幅（:math:`n` 为量子比特数），无论大多数振幅是否为零。PySparQ 采用稀疏表示：

- 只存储 ``amplitude ≠ 0`` 的基态
- 每个 ``System`` 代表一个计算基态
- 叠加态数量有限时，存储复杂度为多项式级

唯一性规则
----------

``SparseState`` 中的一个核心不变量是：**所有 ``System`` 的寄存器值组合必须唯一**。

如果对 ``SparseState`` 中的基态进行操作后产生了两个具有相同寄存器值的 ``System``，这意味着发生了量子干涉——此时两个 ``System`` 的振幅应当相加，合并为一个 ``System``。这一过程通常由算子内部的 ``sort-merge-unique`` 机制自动完成。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("q", ps.UnsignedInteger, 2)

   # SparseState() 默认构造：创建单个 |q=0⟩ 基态
   state = ps.SparseState()
   print(f"基态数量: {state.size()}")  # 1

   # 施加 Hadamard 创建 2^2 = 4 个基态
   ps.Hadamard_Int_Full("q")(state)
   print(f"基态数量: {state.size()}")  # 4

基本用法
--------

创建 SparseState
^^^^^^^^^^^^^^^^

.. important::

   ``SparseState()`` 的默认构造函数会自动创建一个 ``|0...0⟩`` 的初态（即一个所有寄存器值为 0、振幅为 1 的 ``System``）。通常**不需要**手动构造 ``System`` 对象来创建 ``SparseState``。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("q", ps.UnsignedInteger, 10)

   # 默认构造：创建单个 |q=0⟩ 基态（振幅为 1）
   state = ps.SparseState()

访问基态
^^^^^^^^

.. code-block:: python

   # 获取基态列表
   for system in state.basis_states:
       print(f"振幅: {system.amplitude}")

   # 按索引访问
   first = state[0]
   last = state.basis_states[-1]

   # 获取基态数量
   n = state.size()

   # 检查是否为空
   if state.empty():
       print("状态为空")

状态演化示例
------------

下面的示例展示 ``SparseState`` 如何随算子操作演化：

.. code-block:: python
   :caption: 示例：Hadamard 创建叠加态

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("q", ps.UnsignedInteger, 2)

   # SparseState 默认创建 |q=0⟩ 初态
   state = ps.SparseState()

   print("初始状态:")
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt2 |
   # 1.000000+0.000000i  q=|0>

   # 施加 Hadamard（部分叠加：对第 0 位施 H）
   ps.Hadamard_Int("q", 1)(state)

   print("\nHadamard 后:")
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt2 |
   # 0.707107+0.000000i  q=|0>
   # 0.707107+0.000000i  q=|2>

   # 完整 Hadamard（所有输出状态）
   ps.Hadamard_Int_Full("q")(state)

   print("\n完整 Hadamard 后:")
   ps.print(state)
   # 输出：
   # StatePrint (mode=Detail)
   # |(0)q : UInt2 |
   # 0.500000+0.000000i  q=|0>
   # 0.500000+0.000000i  q=|1>
   # 0.500000+0.000000i  q=|2>
   # 0.500000+0.000000i  q=|3>

状态打印模式
------------

``ps.StatePrint(state, mode)`` 和 ``ps.print(state, mode)`` 支持多种显示模式：

.. list-table:: StatePrintDisplay 枚举
   :header-rows: 1

   * - 模式
     - 值
     - 说明
     - 示例输出
   * - ``Default``
     - 0
     - 默认模式，十进制值
     - ``0.5+0.000000i |5>``
   * - ``Detail``
     - 1
     - 详细模式，含寄存器头和振幅
     - ``0.500000+0.000000i  q=|5>``
   * - ``Binary``
     - 2
     - 二进制表示
     - ``0.5+0.000000i |0101>``
   * - ``Prob``
     - 4
     - 概率视图
     - ``0.5+0.000000i (p = 0.25) |5>``

.. code-block:: python

   # 不同显示模式
   ps.print(state)                                                    # Detail（默认）
   print(ps.StatePrint(state, mode=ps.StatePrintDisplay.Default))      # Default
   print(ps.StatePrint(state, mode=ps.StatePrintDisplay.Binary))       # Binary
   print(ps.StatePrint(state, mode=ps.StatePrintDisplay.Prob))         # Prob

   # 指定精度
   print(ps.StatePrint(state, mode=ps.StatePrintDisplay.Default, precision=15))
   print(ps.StatePrint(state, mode=ps.StatePrintDisplay.Default, precision=4))

清除接近零的振幅
----------------

.. code-block:: python

   # 清除 |amplitude|² < epsilon 的基态
   ps.ClearZero(epsilon=1e-10)(state)

   # 归一化状态
   ps.Normalize()(state)

合并重复基态
------------

当多个基态具有相同的寄存器值时，它们的振幅会自动合并：

.. code-block:: python

   # merge_system 函数：合并两个相同基态的振幅
   # 通常由算子内部自动调用

迭代器支持
----------

``SparseState`` 支持标准 Python 迭代器协议：

.. code-block:: python

   # 正向迭代
   for system in state.basis_states:
       print(system.amplitude)

   # 反向迭代
   for system in reversed(state.basis_states):
       print(system.amplitude)

   # 索引访问
   first = state.basis_states[0]
   last = state.basis_states[-1]

   # 切片
   first_three = state.basis_states[:3]

API 参考
--------

.. autoclass:: pysparq.SparseState
   :members:
   :undoc-members:

.. autofunction:: pysparq.split_systems

.. autofunction:: pysparq.combine_systems

.. autofunction:: pysparq.merge_system

.. autofunction:: pysparq.remove_system
