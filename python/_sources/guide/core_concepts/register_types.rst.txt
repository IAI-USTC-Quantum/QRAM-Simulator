寄存器类型
==========

每个量子寄存器具有特定的存储类型，决定了其值的解释方式和有效范围。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

StateStorageType 枚举
---------------------

.. autoclass:: pysparq.StateStorageType
   :members:
   :undoc-members:

类型详解
--------

UnsignedInteger（无符号整数）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- **值域**: :math:`[0, 2^n - 1]`
- **存储**: 直接存储二进制值
- **用途**: 计数器、地址、数组索引

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("addr", ps.UnsignedInteger, 4)  # 0-15 的地址

   # 典型用法
   state = ps.SparseState()
   ps.Init_Unsafe("addr", 5)(state)  # 初始化为 5

SignedInteger（有符号整数）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- **值域**: :math:`[-2^{n-1},\ 2^{n-1}-1]` （二补码表示）
- **存储**: 二补码编码
- **用途**: 带符号的算术运算

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("temp", ps.SignedInteger, 8)  # -128 到 127

   # 支持 Add_UInt_UInt 等算术算子

Boolean（布尔/单量子比特）
^^^^^^^^^^^^^^^^^^^^^^^^^^

- **值域**: {0, 1}
- **大小**: 必须是 1 比特
- **用途**: 标志位、控制比特、单量子比特门操作

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("flag", ps.Boolean, 1)  # 必须是 1 比特

   # 施加单量子比特门
   state = ps.SparseState()
   ps.Xgate_Bool("flag", 0)(state)  # 翻转
   ps.Hadamard_Bool("flag")(state)   # Hadamard

Rational（有理数/定点小数）
^^^^^^^^^^^^^^^^^^^^^^^^^^^

- **值域**: :math:`[0, 1)`
- **存储**: 定点表示，解释为 :math:`value / 2^n`
- **用途**: 角度编码、条件旋转

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("angle", ps.Rational, 16)  # 高精度角度

   # 用于条件旋转
   ps.CondRot_Rational_Bool("angle", "target")(state)

General（通用存储）
^^^^^^^^^^^^^^^^^^^^^^

- **值域**: 任意比特模式
- **用途**: 原始比特存储、特殊用途

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("raw", ps.General, 32)

StateStorage 类
---------------

每个寄存器值存储在 ``StateStorage`` 对象中：

.. autoclass:: pysparq.StateStorage
   :members:
   :undoc-members:

类型约束在算子中的应用
----------------------

算子会检查寄存器类型是否符合要求：

.. code-block:: python

   # 正确：Boolean 寄存器用于单量子比特门
   ps.Xgate_Bool("flag", 0)(state)

   # 错误：UnsignedInteger 不能用于 Boolean 算子
   # ps.Xgate_Bool("counter", 0)(state)  # 类型不匹配！

类型选择建议
------------

.. list-table:: 类型选择指南
   :header-rows: 1

   * - 场景
     - 推荐类型
     - 示例
   * - 地址/索引
     - ``UnsignedInteger``
     - QRAM 地址
   * - 计数器
     - ``UnsignedInteger``
     - 循环变量
   * - 控制标志
     - ``Boolean``
     - 比较结果
   * - 角度参数
     - ``Rational``
     - 旋转角度
   * - 温度/物理量
     - ``SignedInteger``
     - 带符号测量
