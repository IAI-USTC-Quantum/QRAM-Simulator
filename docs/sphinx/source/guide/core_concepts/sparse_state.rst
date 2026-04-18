SparseState 类
==============

``SparseState`` 类是 PySparQ 的核心数据结构，表示一个稀疏量子态。它封装了 ``System`` 对象的列表，只存储振幅非零的基态。

.. contents:: 目录
   :local:

稀疏表示原理
------------

传统全态矢量模拟器存储 :math:`2^n` 个振幅（:math:`n` 为量子比特数），无论大多数振幅是否为零。PySparQ 采用稀疏表示：

- 只存储 ``amplitude ≠ 0`` 的基态
- 每个 ``System`` 代表一个计算基态
- 叠加态数量有限时，存储复杂度为多项式级

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   # 添加一个 10 比特的寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 10)

   # 初始状态：只有一个基态 |q=0⟩
   state = ps.SparseState()
   print(f"基态数量: {state.size()}")  # 1

   # 施加 Hadamard 创建 2^10 = 1024 个基态
   ps.Hadamard_Int_Full("q")(state)
   print(f"基态数量: {state.size()}")  # 1024

基本用法
--------

创建 SparseState
^^^^^^^^^^^^^^^^

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   # 默认构造：创建单个 |0...0⟩ 基态
   state = ps.SparseState()

   # 指定初始基态数量
   state = ps.SparseState(size=4)

   # 从 System 列表构造
   systems = [ps.System(), ps.System()]
   state = ps.SparseState(systems)

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

   # 创建 2 比特寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 2)

   # 初始状态
   state = ps.SparseState()

   print("初始状态:")
   ps.StatePrint()(state)
   # 输出:
   # [1 basis state]
   # |q=0⟩ : (1+0j)

   # 施加 Hadamard
   ps.Hadamard_Int("q")(state)

   print("\nHadamard 后:")
   ps.StatePrint()(state)
   # 输出:
   # [2 basis states]
   # |q=0⟩ : (0.707+0j)
   # |q=2⟩ : (0.707+0j)

   # 完整 Hadamard（所有输出状态）
   ps.Hadamard_Int_Full("q")(state)

   print("\n完整 Hadamard 后:")
   ps.StatePrint()(state)
   # 输出:
   # [4 basis states]
   # |q=0⟩ : (0.5+0j)
   # |q=1⟩ : (0.5+0j)
   # |q=2⟩ : (0.5+0j)
   # |q=3⟩ : (0.5+0j)

状态打印模式
------------

``StatePrint`` 算子支持多种显示模式：

.. list-table:: StatePrintDisplay 枚举
   :header-rows: 1

   * - 模式
     - 值
     - 说明
     - 示例输出
   * - ``Default``
     - 0
     - 默认模式，十进制值
     - ``|q=5⟩ : (0.5+0j)``
   * - ``Detail``
     - 1
     - 详细振幅显示
     - 显示实部/虚部分量
   * - ``Binary``
     - 2
     - 二进制表示
     - ``|q=101⟩ : (0.5+0j)``
   * - ``Prob``
     - 4
     - 概率视图
     - ``|q=5⟩ : P=0.25``

.. code-block:: python

   # 不同显示模式
   ps.StatePrint(ps.Default)(state)
   ps.StatePrint(ps.Binary)(state)
   ps.StatePrint(ps.Prob)(state)

   # 指定精度
   ps.StatePrint(ps.Default, precision=4)(state)

状态操作函数
------------

拆分与合并寄存器
^^^^^^^^^^^^^^^^

.. code-block:: python

   ps.System.clear()

   # 创建 8 比特寄存器
   ps.System.add_register("full", ps.UnsignedInteger, 8)

   state = ps.SparseState()
   ps.Init_Unsafe("full", 0b10110011)(state)

   # 拆分为两个 4 比特寄存器
   ps.SplitRegister("high", "low", 4)(state)
   # "high" = 0b1011, "low" = 0b0011

   # 合并两个寄存器
   ps.CombineRegister("combined", "high", "low")(state)
   # "combined" = 0b10110011

清除接近零的振幅
^^^^^^^^^^^^^^^^

.. code-block:: python

   # 清除 |amplitude|² < epsilon 的基态
   ps.ClearZero(epsilon=1e-10)(state)

   # 归一化状态
   ps.Normalize()(state)

合并重复基态
^^^^^^^^^^^^

当多个基态具有相同的寄存器值时，它们的振幅会自动合并：

.. code-block:: python

   # merge_system 函数：合并两个相同基态的振幅
   # 通常由算子内部自动调用

寄存器栈操作
^^^^^^^^^^^^

.. code-block:: python

   # Push：添加临时寄存器
   ps.Push("temp", ps.UnsignedInteger, 4)(state)

   # 使用临时寄存器...

   # Pop：删除最后一个寄存器
   ps.Pop()(state)

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
