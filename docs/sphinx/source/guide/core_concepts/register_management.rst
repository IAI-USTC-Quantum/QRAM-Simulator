寄存器管理
==========

寄存器是 PySparQ "寄存器级编程"（Register Level Programming）的核心抽象。与传统的量子比特级编程不同，PySparQ 将操作层面提升到量子寄存器，使算法开发更加直观。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

寄存器的本质
------------

系统中的寄存器托管了 ``n`` 个 ``uint64_t`` 的存储。每个寄存器拥有名称、类型和比特宽度，其值以 ``uint64_t`` 存储。这种设计允许我们将量子态编码为类似 :math:`|a\rangle|b\rangle|c\rangle` 的多寄存器形式，而无需关心底层量子比特的编码方式。

例如，QRAM 访问 :math:`|i\rangle|0\rangle \to |i\rangle|d[i]\rangle` 只需要一个地址寄存器 ``i`` 和一个数据寄存器 ``d``，``QRAMLoad`` 算子直接在寄存器级别完成映射，完全不需要管理量子比特。

.. math::

   |\psi\rangle = \sum_j \alpha_j \, |a_j\rangle |b_j\rangle |c_j\rangle

其中每个 :math:`|a_j\rangle |b_j\rangle |c_j\rangle` 对应一个 ``System`` 中的 ``registers`` 数组。

添加寄存器：AddRegister
------------------------

添加寄存器等价于与 :math:`|0\rangle` 寄存器做直积（tensor product）。新寄存器在所有现有基态中的初始值为 0。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("a", ps.UnsignedInteger, 4)
   state = ps.SparseState()

   # 添加寄存器 "b"：等价于 |a⟩ ⊗ |0⟩
   ps.AddRegister("b", ps.UnsignedInteger, 4)(state)

   # state 中所有基态的 "b" 值为 0
   ps.StatePrint()(state)

``AddRegister`` 会同时更新静态元数据（``name_register_map``）和所有现有基态的寄存器值。

也可以使用 ``AddRegisterWithHadamard`` 在添加寄存器的同时施加 Hadamard，直接创建均匀叠加态：

.. code-block:: python

   # 添加 "q" 并创建 |0⟩, |1⟩, ..., |2^n - 1⟩ 的均匀叠加
   ps.AddRegisterWithHadamard("q", ps.UnsignedInteger, 2)(state)

删除寄存器：RemoveRegister
--------------------------

删除寄存器等价于对该寄存器做 PartialTrace（偏迹），即消除该寄存器对量子态的贡献。在模拟中，这相当于测量该寄存器后丢弃测量结果的效果。

.. code-block:: python

   # 删除寄存器 "b"：等价于对 "b" 做 PartialTrace
   ps.RemoveRegister("b")(state)

.. important::

   ``RemoveRegister`` 在执行前会检查该寄存器是否与剩余寄存器存在纠缠（通过 ``TestRemovable``）。如果存在纠缠，删除操作会抛出异常，因为此时对单个寄存器的 PartialTrace 不能简单地等价于丢弃。

拆分寄存器：SplitRegister
--------------------------

``SplitRegister`` 将一个寄存器拆分为两个：原寄存器保留高位，新寄存器获取低位。

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("full", ps.UnsignedInteger, 8)
   state = ps.SparseState()
   ps.Init_Unsafe("full", 0b10110011)(state)

   # 拆分：原 "full" 保留高 4 位，新 "low" 获取低 4 位
   ps.SplitRegister("full", "low", 4)(state)
   # "full" = 0b1011（高 4 位），"low" = 0b0011（低 4 位）

拆分过程：

1. 添加新寄存器（获取低位）
2. 缩小原寄存器的比特宽度
3. 在所有基态中，将原值的高位保留在原寄存器，低位写入新寄存器

合并寄存器：CombineRegister
---------------------------

``CombineRegister`` 将两个寄存器合并为一个：第一个寄存器的值左移后拼接第二个寄存器的值。

.. code-block:: python

   # 合并："full" = (full << 4) + low
   ps.CombineRegister("full", "low")(state)
   # "full" = 0b10110011

合并过程：

1. 扩展第一个寄存器的比特宽度（加上第二个的宽度）
2. 在所有基态中，将第一个寄存器的值左移第二个的宽度后加上第二个的值
3. 删除第二个寄存器

PartialTrace：测量
------------------

在 PySparQ 中，``PartialTrace`` 被等价视为测量操作。这在模拟层面是合理的——对某些寄存器进行偏迹等同于对它们进行测量并丢弃结果。

``PartialTrace`` 提供三种模式：

.. list-table:: PartialTrace 变体
   :header-rows: 1

   * - 类
     - 行为
     - 返回值
   * - ``PartialTrace``
     - 随机测量：按概率分布随机选择测量结果，然后坍缩态
     - ``(measured_values, probability)``
   * - ``PartialTraceSelect``
     - 选择性坍缩：保留指定寄存器值为指定值的基态，归一化其余基态
     - 归一化概率
   * - ``PartialTraceSelectRange``
     - 范围坍缩：保留指定寄存器值在给定范围内的基态
     - 归一化概率

.. code-block:: python
   :caption: PartialTrace 测量示例

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("a", ps.UnsignedInteger, 2)
   ps.System.add_register("b", ps.UnsignedInteger, 2)
   state = ps.SparseState()

   ps.Hadamard_Int("a")(state)
   ps.Hadamard_Int("b")(state)

   # 随机测量寄存器 "a"
   measured_values, prob = ps.PartialTrace("a")(state)
   print(f"测量结果: {measured_values}, 概率: {prob}")
   # state 中 "a" 的值坍缩为测量结果

   # 选择性坍缩：强制 "b" = 2
   prob = ps.PartialTraceSelect("b", [2])(state)

.. code-block:: python
   :caption: PartialTraceSelect 确定性坍缩示例

   # 只保留 "a" = 1 的基态并归一化
   prob = ps.PartialTraceSelect("a", [1])(state)

寄存器栈操作：Push / Pop
-------------------------

``Push`` 和 ``Pop`` 提供临时寄存器的栈管理，适用于算法中需要临时变量的场景：

.. code-block:: python

   # Push：将临时寄存器入栈
   ps.Push("temp", ps.UnsignedInteger, 4)(state)

   # 使用临时寄存器进行计算...

   # Pop：弹出临时寄存器
   ps.Pop()(state)

寄存器操作与量子态的关系总结
----------------------------

.. list-table:: 寄存器操作的物理意义
   :header-rows: 1

   * - 操作
     - 物理意义
     - 对量子态的影响
   * - ``AddRegister``
     - 与 :math:`|0\rangle` 直积
     - 所有基态增加一个值为 0 的寄存器
   * - ``RemoveRegister``
     - PartialTrace（测量后丢弃）
     - 消除该寄存器，前提是无纠缠
   * - ``SplitRegister``
     - 将一个子系统拆分为两个
     - 基态数量不变，寄存器数加 1
   * - ``CombineRegister``
     - 将两个子系统合并为一个
     - 基态数量不变，寄存器数减 1
   * - ``PartialTrace``
     - 测量
     - 按概率坍缩态，基态数量减少
   * - ``Push`` / ``Pop``
     - 临时寄存器入栈/出栈
     - 辅助操作，保存/恢复寄存器状态

API 参考
--------

.. autoclass:: pysparq.AddRegister
   :members:
   :undoc-members:

.. autoclass:: pysparq.RemoveRegister
   :members:
   :undoc-members:

.. autoclass:: pysparq.SplitRegister
   :members:
   :undoc-members:

.. autoclass:: pysparq.CombineRegister
   :members:
   :undoc-members:

.. autoclass:: pysparq.PartialTrace
   :members:
   :undoc-members:

.. autoclass:: pysparq.PartialTraceSelect
   :members:
   :undoc-members:

.. autoclass:: pysparq.PartialTraceSelectRange
   :members:
   :undoc-members:

.. autoclass:: pysparq.AddRegisterWithHadamard
   :members:
   :undoc-members:

.. autoclass:: pysparq.Push
   :members:
   :undoc-members:

.. autoclass:: pysparq.Pop
   :members:
   :undoc-members:

.. autoclass:: pysparq.ClearZero
   :members:
   :undoc-members:
