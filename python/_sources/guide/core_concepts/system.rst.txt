System 类
=========

``System`` 类是稀疏态模拟器的基本单元，代表一个计算基态。每个 System 存储一个复数振幅和所有寄存器的值。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

在 PySparQ 中，量子态用稀疏表示：只存储振幅非零的基态。每个基态用一个 ``System`` 对象表示：

.. math::

   |\psi\rangle = \sum_{i \in \text{non-zero}} \alpha_i |i\rangle

其中每个 :math:`|i\rangle` 对应一个 ``System`` 实例。

静态变量：全局寄存器追踪
------------------------

``System`` 类使用静态（类级）变量追踪所有寄存器的元信息。这意味着：

- 所有 ``System`` 实例共享同一套寄存器元数据
- 寄存器 ID 在全局范围内分配
- 程序结束时静态状态不会自动清除

.. warning::

   **重要**：每次运行新程序前，必须调用 ``System.clear()`` 清理静态状态，否则会残留上次运行的信息！

静态成员
^^^^^^^^

.. list-table:: System 静态变量
   :header-rows: 1

   * - 变量名
     - 类型
     - 说明
   * - ``name_register_map``
     - ``list``
     - 寄存器元数据列表，每个元素为 ``(name, type, size, status)`` 元组
   * - ``max_register_count``
     - ``int``
     - 历史最大寄存器数量（统计用）
   * - ``max_system_size``
     - ``int``
     - 历史最大基态数量（统计用）
   * - ``reusable_registers``
     - ``list``
     - 可重用的寄存器 ID 列表
   * - ``temporal_registers``
     - ``list``
     - 临时寄存器栈

实例成员
--------

每个 ``System`` 实例包含：

.. list-table:: System 实例成员
   :header-rows: 1

   * - 成员
     - 类型
     - 说明
   * - ``amplitude``
     - ``complex``
     - 该基态的复数振幅
   * - ``registers``
     - ``list[StateStorage]``
     - 各寄存器的值（按寄存器 ID 索引）

寄存器管理
----------

添加寄存器
^^^^^^^^^^

.. code-block:: python

   import pysparq as ps

   # 清理之前的静态状态（必须！）
   ps.System.clear()

   # 添加寄存器：名称、类型、比特数
   reg_id = ps.System.add_register("counter", ps.UnsignedInteger, 4)
   print(f"寄存器 ID: {reg_id}")  # 0

   # 同步添加：同时初始化所有基态中的寄存器值为 0
   state = ps.SparseState()
   reg_id2 = ps.System.add_register_synchronous("flag", ps.Boolean, 1, state)

   # 添加并施加 Hadamard（创建叠加态）
   ps.AddRegisterWithHadamard("q", ps.UnsignedInteger, 2)(state)
   # 现在 "q" 寄存器处于 |0⟩, |1⟩, |2⟩, |3⟩ 的均匀叠加

删除寄存器
^^^^^^^^^^

.. code-block:: python

   # 按名称删除
   ps.System.remove_register("counter")

   # 按 ID 删除
   ps.System.remove_register(reg_id)

   # 同步删除：同时更新所有基态
   ps.System.remove_register_synchronous("flag", state)

查询方法
--------

获取寄存器信息
^^^^^^^^^^^^^^

.. code-block:: python

   # 获取寄存器 ID
   reg_id = ps.System.get_id("counter")

   # 获取寄存器大小（比特数）
   size = ps.System.size_of("counter")  # 或 ps.System.size_of(reg_id)

   # 获取寄存器类型
   type_ = ps.System.type_of("counter")  # 返回 StateStorageType 枚举

   # 获取寄存器激活状态
   active = ps.System.status_of("counter")  # True 表示已激活

   # 获取完整元信息
   info = ps.System.get_register_info("counter")
   # 返回 (name, type, size, status) 元组

   # 根据 ID 获取名称
   name = ps.System.name_of(reg_id)

统计信息
^^^^^^^^

.. code-block:: python

   # 获取量子比特总数
   n_qubits = ps.System.get_qubit_count()

   # 获取已激活寄存器数量
   n_regs = ps.System.get_activated_register_size()

访问基态中的寄存器值
--------------------

.. code-block:: python

   state = ps.SparseState()

   # 添加寄存器并初始化
   ps.System.add_register_synchronous("x", ps.UnsignedInteger, 4, state)

   # 遍历所有基态
   for system in state.basis_states:
       # 访问振幅
       amp = system.amplitude

       # 访问寄存器值（通过 ID）
       x_id = ps.System.get_id("x")
       value = system.get(x_id).value

       # 字符串表示
       print(system)

   # 访问最后一个激活的寄存器
   last_val = system.last_register()

比较与排序
----------

``System`` 类支持比较运算符，用于排序和去重：

.. code-block:: python

   s1 = ps.System()
   s2 = ps.System()

   # 相等比较（所有寄存器值和振幅都相同）
   if s1 == s2:
       print("相同基态")

   # 小于比较（用于排序）
   if s1 < s2:
       print("s1 排在 s2 前面")

   # 字符串表示
   print(str(system))  # 如: "|x=3, y=5⟩ : (0.5+0j)"

最佳实践
--------

1. **始终在程序开始时调用 ``System.clear()``**

   .. code-block:: python

      import pysparq as ps

      ps.System.clear()  # 第一步！

      # 然后创建寄存器...
      ps.System.add_register("a", ps.UnsignedInteger, 4)

2. **使用 ``add_register_synchronous`` 确保状态一致性**

   当 ``SparseState`` 已存在时，使用同步版本自动更新所有基态。

3. **使用有意义的寄存器名称**

   名称用于调试和 ``StatePrint`` 输出，应具有描述性。

API 参考
--------

.. autoclass:: pysparq.System
   :members:
   :undoc-members:
