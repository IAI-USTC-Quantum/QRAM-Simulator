量子算子
========

算子是 PySparQ 中量子操作的构建模块。所有操作都以算子对象的形式实现，它们接受 ``SparseState`` 并对其进行变换。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

.. toctree::
   :maxdepth: 2
   :caption: 算子分类

   arithmetic
   gates
   hadamard
   qram_ops
   condrot
   qft

什么是算子？
------------

**算子**是一个可调用对象，它对 ``SparseState`` 进行变换，实现量子操作同时保证幺正性。

基本用法
^^^^^^^^

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   # 创建寄存器
   ps.System.add_register("a", ps.UnsignedInteger, 4)
   ps.System.add_register("b", ps.UnsignedInteger, 4)
   ps.System.add_register("result", ps.UnsignedInteger, 4)

   state = ps.SparseState()

   # 初始化输入
   ps.Init_Unsafe("a", 3)(state)
   ps.Init_Unsafe("b", 5)(state)

   # 创建并应用算子
   add_op = ps.Add_UInt_UInt("a", "b", "result")
   add_op(state)  # 应用算子

   # 对于非自伴算子，使用 dag() 撤销操作
   add_op.dag(state)  # 撤销（恢复原状态）

算子属性
--------

幺正性质
^^^^^^^^

所有量子算子必须满足幺正条件：

.. math::

   U^\dagger U = I

PySparQ 通过两种机制保证幺正性：

.. list-table:: 幺正性机制
   :header-rows: 1

   * - 类型
     - 机制
     - 示例
   * - Out-of-place（外置）
     - XOR 写入：``result ^= f(inputs)``
     - ``Add_UInt_UInt``, ``Mult_UInt_ConstUInt``
   * - In-place（内置）
     - 显式 dagger 实现
     - ``Add_UInt_UInt_InPlace``, ``ShiftLeft``

SelfAdjointOperator vs BaseOperator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: 算子基类
   :header-rows: 1

   * - 基类
     - 特点
     - ``dag()`` 行为
     - 典型算子
   * - ``SelfAdjointOperator``
     - :math:`U^\dagger = U`
     - ``dag()`` 等同于 ``operator()``
     - ``Add_UInt_UInt``, ``Xgate_Bool``
   * - ``BaseOperator``
     - 一般幺正算子
     - 需要显式实现 ``dag()``
     - ``Add_UInt_UInt_InPlace``, ``ShiftLeft``

.. code-block:: python

   # SelfAdjointOperator：两次应用恢复原状态
   op = ps.Add_UInt_UInt("a", "b", "result")
   op(state)  # 应用
   op(state)  # 再次应用 = 撤销（因为 XOR 自逆）

   # BaseOperator：需要 dag() 撤销
   op = ps.ShiftLeft("reg", 2)
   op(state)      # 左移 2 位
   op.dag(state)  # 右移 2 位（撤销）

类型约束
^^^^^^^^

算子对寄存器类型有严格要求：

.. list-table:: 寄存器类型
   :header-rows: 1

   * - 类型
     - 说明
     - 有效范围
   * - ``UnsignedInteger``
     - 无符号整数
     - :math:`[0, 2^n-1]`
   * - ``SignedInteger``
     - 有符号整数（二补码）
     - :math:`[-2^{n-1}, 2^{n-1}-1]`
   * - ``Boolean``
     - 单量子比特
     - {0, 1}
   * - ``Rational``
     - 定点小数
     - :math:`[0, 1)`
   * - ``General``
     - 原始比特存储
     - 任意比特模式

.. code-block:: python

   # 正确：Boolean 用于单量子比特门
   ps.System.add_register("qubit", ps.Boolean, 1)
   ps.Xgate_Bool("qubit", 0)(state)

   # 错误：类型不匹配
   # ps.System.add_register("counter", ps.UnsignedInteger, 4)
   # ps.Xgate_Bool("counter", 0)(state)  # 抛出异常！

位约束
^^^^^^

许多算子验证：

- 寄存器大小匹配预期维度
- 比特索引在寄存器范围内
- 输出寄存器有足够容量

条件操作
--------

所有算子支持条件执行，实现受控操作。

条件方法
^^^^^^^^

.. list-table:: 条件方法
   :header-rows: 1

   * - 方法
     - 条件
     - 示例
   * - ``conditioned_by_nonzeros(reg)``
     - 寄存器值 ≠ 0
     - 任意非零状态控制
   * - ``conditioned_by_all_ones(reg)``
     - 所有比特为 1
     - 多量子比特控制
   * - ``conditioned_by_bit(reg, pos)``
     - 指定比特为 1
     - 单量子比特控制
   * - ``conditioned_by_value(reg, val)``
     - 寄存器等于特定值
     - 经典控制

.. code-block:: python

   op = ps.Add_UInt_UInt("a", "b", "result")

   # 当 control 寄存器非零时应用
   op.conditioned_by_nonzeros("control")(state)

   # 当 flag 的第 0 位为 1 时应用
   op.conditioned_by_bit("flag", 0)(state)

   # 当 mode 等于 1 时应用
   op.conditioned_by_value("mode", 1)(state)

   # 多个条件
   op.conditioned_by_nonzeros(["ctrl1", "ctrl2"])(state)

清理控制条件
^^^^^^^^^^^^

.. code-block:: python

   op.clear_control_nonzeros()
   op.clear_control_by_bit()
   op.clear_control_by_value()
   op.clear_control_all_ones()

查看控制变量
^^^^^^^^^^^^

.. code-block:: python

   # 获取当前控制变量
   print(op.condition_variable_nonzeros)
   print(op.condition_variable_by_bit)  # list[tuple[int, int]]
   print(op.condition_variable_by_value)  # list[tuple[int, int]]

API 参考
--------

.. autoclass:: pysparq.BaseOperator
   :members:
   :undoc-members:

.. autoclass:: pysparq.SelfAdjointOperator
   :members:
   :undoc-members:
