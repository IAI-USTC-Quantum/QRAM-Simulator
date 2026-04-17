快速入门
========

寄存器级编程
------------

PySparQ 采用"寄存器级编程"范式。与从单个门组合电路不同，您直接对命名的量子寄存器进行操作。

基本工作流程
------------

1. 创建量子系统
2. 创建稀疏量子态
3. 分配寄存器（整数、布尔值）
4. 应用量子操作
5. 读取测量结果

示例：量子加法
--------------

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, Add_UInt_UInt, Hadamard_Int

   # 创建系统和状态
   system = System()
   state = SparseState(system)

   # 分配两个4位无符号整数寄存器
   AddRegister("a", pysparq.UnsignedInteger, 4)(state)
   AddRegister("b", pysparq.UnsignedInteger, 4)(state)

   # 将两个寄存器置于叠加态
   Hadamard_Int("a")(state)
   Hadamard_Int("b")(state)

   # 量子加法：a += b
   Add_UInt_UInt("b", "a")(state)

   # 状态现在包含所有可能求和结果的叠加态
   print(state)

条件操作
--------

操作可以根据其他寄存器的值进行条件控制：

.. code-block:: python

   # 分配控制寄存器
   AddRegister("control", pysparq.Boolean, 1)(state)

   # 仅当 control 为 |1> 时应用操作
   Add_UInt_UInt("a", "b").conditioned_by_nonzeros("control")(state)

控制类型
^^^^^^^^

- :meth:`conditioned_by_nonzeros(reg)` - 当寄存器非零时执行
- :meth:`conditioned_by_all_ones(reg)` - 当寄存器全为1时执行
- :meth:`conditioned_by_bit(reg, pos)` - 当特定位为1时执行
- :meth:`conditioned_by_value(reg, pos)` - 当指定位置的值为特定值时执行
