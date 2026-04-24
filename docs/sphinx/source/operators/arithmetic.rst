算术算子
========

算术算子实现量子整数运算，包括加法、乘法、移位、比较等操作。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 算术算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 类型
     - 幺正类
   * - ``Add_UInt_UInt``
     - ``result ^= lhs + rhs``
     - Out-of-place
     - SelfAdjoint
   * - ``Add_UInt_UInt_InPlace``
     - ``rhs += lhs``
     - In-place
     - BaseOperator
   * - ``Add_UInt_ConstUInt``
     - ``result ^= lhs + const``
     - Out-of-place
     - SelfAdjoint
   * - ``Add_ConstUInt``
     - ``reg += const``
     - In-place
     - BaseOperator
   * - ``Mult_UInt_ConstUInt``
     - ``result ^= input * const``
     - Out-of-place
     - SelfAdjoint
   * - ``Add_Mult_UInt_ConstUInt``
     - ``res += lhs * const``
     - In-place
     - BaseOperator
   * - ``Mod_Mult_UInt_ConstUInt``
     - ``y = y * a^(2^x) mod N``
     - In-place
     - BaseOperator
   * - ``ShiftLeft``
     - 循环左移
     - In-place
     - BaseOperator
   * - ``ShiftRight``
     - 循环右移
     - In-place
     - BaseOperator
   * - ``Compare_UInt_UInt``
     - 比较标志
     - Out-of-place
     - SelfAdjoint
   * - ``Less_UInt_UInt``
     - 小于标志
     - Out-of-place
     - SelfAdjoint
   * - ``Assign``
     - ``dst ^= src``
     - Out-of-place
     - SelfAdjoint
   * - ``FlipBools``
     - 按位取反
     - In-place
     - SelfAdjoint
   * - ``Swap_General_General``
     - 交换两寄存器
     - In-place
     - SelfAdjoint
   * - ``GetMid_UInt_UInt``
     - 计算中点
     - Out-of-place
     - SelfAdjoint

---

加法算子
--------

Add_UInt_UInt（外置加法）
^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Add_UInt_UInt
   :members:
   :undoc-members:

**操作**: ``result ^= lhs + rhs``

**幺正保证**: XOR 机制 — 应用两次恢复原值。

**类型约束**: 所有寄存器必须是 ``UnsignedInteger``。

**位约束**: 无特殊要求，结果按输出寄存器大小截断。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   ps.System.add_register("lhs", ps.UnsignedInteger, 4)
   ps.System.add_register("rhs", ps.UnsignedInteger, 4)
   ps.System.add_register("result", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("lhs", 3)(state)
   ps.Init_Unsafe("rhs", 5)(state)

   # result = 0 ^ (3 + 5) = 8
   ps.Add_UInt_UInt("lhs", "rhs", "result")(state)

   ps.StatePrint()(state)
   # 输出: |lhs=3,rhs=5,result=8⟩ : (1+0j)

   # 再次应用 = 撤销
   ps.Add_UInt_UInt("lhs", "rhs", "result")(state)
   # result = 8 ^ 8 = 0

Add_UInt_UInt_InPlace（内置加法）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Add_UInt_UInt_InPlace
   :members:
   :undoc-members:

**操作**: ``rhs = (rhs + lhs) mod 2^n``

**Dagger 实现**: ``rhs = (rhs + 2^n - lhs) mod 2^n``

**类型约束**: 两个寄存器必须是 ``UnsignedInteger``。

**位约束**: 建议两寄存器大小相同，否则较小值被截断。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("lhs", ps.UnsignedInteger, 4)
   ps.System.add_register("rhs", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("lhs", 7)(state)
   ps.Init_Unsafe("rhs", 10)(state)

   # rhs = (10 + 7) % 16 = 1（溢出回绕）
   op = ps.Add_UInt_UInt_InPlace("lhs", "rhs")
   op(state)

   # 撤销: rhs = (1 + 16 - 7) % 16 = 10
   op.dag(state)

Add_UInt_ConstUInt（常量外置加法）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Add_UInt_ConstUInt
   :members:
   :undoc-members:

**操作**: ``result ^= lhs + const``

**类型约束**: 所有寄存器必须是 ``UnsignedInteger``。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("lhs", ps.UnsignedInteger, 4)
   ps.System.add_register("result", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("lhs", 3)(state)

   # result = 0 ^ (3 + 5) = 8
   ps.Add_UInt_ConstUInt("lhs", 5, "result")(state)

Add_ConstUInt（常量内置加法）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Add_ConstUInt
   :members:
   :undoc-members:

**操作**: ``reg = (reg + const) mod 2^n``

**Dagger 实现**: ``reg = (reg + 2^n - const) mod 2^n``

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("counter", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("counter", 10)(state)

   # counter = (10 + 7) % 16 = 1
   op = ps.Add_ConstUInt("counter", 7)
   op(state)

   # 撤销
   op.dag(state)  # counter = 10

---

乘法算子
--------

Mult_UInt_ConstUInt（常量外置乘法）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Mult_UInt_ConstUInt
   :members:
   :undoc-members:

**操作**: ``result ^= input * const``

**幺正保证**: XOR 机制。

.. warning::

   **乘数应为奇数**才能保证双射性。偶数乘数会丢失最低位信息。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("input", ps.UnsignedInteger, 4)
   ps.System.add_register("result", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("input", 3)(state)

   # 好：奇数乘数
   ps.Mult_UInt_ConstUInt("input", 3, "result")(state)
   # result = 0 ^ (3 * 3) = 9

   # 避免：偶数乘数不双射
   # ps.Mult_UInt_ConstUInt("input", 2, "result")(state)  # 丢失 LSB

Add_Mult_UInt_ConstUInt（累加乘法）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Add_Mult_UInt_ConstUInt
   :members:
   :undoc-members:

**操作**: ``result += input * const``

**Dagger 实现**: ``result += (2^n - input * const) mod 2^n``

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("input", ps.UnsignedInteger, 4)
   ps.System.add_register("result", ps.UnsignedInteger, 8)  # 更大防止溢出

   state = ps.SparseState()
   ps.Init_Unsafe("input", 3)(state)
   ps.Init_Unsafe("result", 5)(state)

   # result = 5 + (3 * 4) = 17
   op = ps.Add_Mult_UInt_ConstUInt("input", 4, "result")
   op(state)

   # 撤销
   op.dag(state)

---

模乘算子
--------

Mod_Mult_UInt_ConstUInt（模乘算子）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Mod_Mult_UInt_ConstUInt
   :members:
   :undoc-members:

**操作**: ``y → y * a^(2^x) mod N``（原地模乘）

**Dagger**: ``y → y * a^(-2^x) mod N``（使用扩展欧几里得算法求模逆）

**类型约束**: ``UnsignedInteger``，寄存器大小需 ≥ ⌈log₂(N)⌉。

**条件**: ``a`` 和 ``N`` 必须互质（gcd(a, N) = 1），否则构造时抛出异常。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   reg = ps.System.add_register("y", ps.UnsignedInteger, 4)
   state = ps.SparseState()
   ps.Init_Unsafe("y", 3)(state)

   # y = 3 * 7 mod 15 = 6
   op = ps.Mod_Mult_UInt_ConstUInt("y", 7, 0, 15)
   op(state)

   # 撤销: y = 6 * 13 mod 15 = 3
   op.dag(state)

---

移位算子
--------

ShiftLeft（循环左移）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.ShiftLeft
   :members:
   :undoc-members:

**操作**: 循环左移 ``digit`` 位

**Dagger**: ``ShiftRight(reg, digit)`` 或 ``ShiftLeft(reg, n - digit)``

**类型约束**: ``UnsignedInteger`` 或 ``SignedInteger``。

**位约束**: ``digit <= 寄存器大小``。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("reg", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("reg", 0b1010)(state)  # 10

   ps.ShiftLeft("reg", 1)(state)
   # reg = 0b0101 = 5

   # 撤销
   ps.ShiftLeft("reg", 1).dag(state)
   # reg = 0b1010 = 10

ShiftRight（循环右移）
^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.ShiftRight
   :members:
   :undoc-members:

**操作**: 循环右移 ``digit`` 位

**Dagger**: ``ShiftLeft(reg, digit)``

.. code-block:: python

   ps.Init_Unsafe("reg", 0b1010)(state)  # 10

   ps.ShiftRight("reg", 1)(state)
   # reg = 0b0101 = 5

   # ShiftLeft 和 ShiftRight 互为逆
   ps.ShiftLeft("reg", 1)(state)
   # reg = 0b1010 = 10

---

比较算子
--------

Compare_UInt_UInt（比较）
^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Compare_UInt_UInt
   :members:
   :undoc-members:

**操作**: 设置 ``less_flag`` 和 ``equal_flag`` 基于 ``lhs < rhs`` 和 ``lhs == rhs``。

**类型约束**: 输入 ``UnsignedInteger``，输出 ``Boolean``。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("lhs", ps.UnsignedInteger, 4)
   ps.System.add_register("rhs", ps.UnsignedInteger, 4)
   ps.System.add_register("less", ps.Boolean, 1)
   ps.System.add_register("equal", ps.Boolean, 1)

   state = ps.SparseState()
   ps.Init_Unsafe("lhs", 3)(state)
   ps.Init_Unsafe("rhs", 5)(state)

   ps.Compare_UInt_UInt("lhs", "rhs", "less", "equal")(state)
   # less = 1 (3 < 5), equal = 0

Less_UInt_UInt（小于比较）
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Less_UInt_UInt
   :members:
   :undoc-members:

**操作**: 仅设置 ``less_flag``。

---

其他算子
--------

Assign（赋值）
^^^^^^^^^^^^^^

.. autoclass:: pysparq.Assign
   :members:
   :undoc-members:

**操作**: ``dst ^= src``

**幺正保证**: XOR 机制。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("src", ps.UnsignedInteger, 4)
   ps.System.add_register("dst", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("src", 5)(state)

   # dst = 0 ^ 5 = 5
   ps.Assign("src", "dst")(state)

FlipBools（按位取反）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.FlipBools
   :members:
   :undoc-members:

**操作**: ``reg = ~reg``

**类型约束**: 任意整数类型。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("reg", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("reg", 0b1010)(state)  # 10

   ps.FlipBools("reg")(state)
   # reg = 0b0101 = 5

Swap_General_General（交换）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Swap_General_General
   :members:
   :undoc-members:

**操作**: 交换两个寄存器的值。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("a", ps.UnsignedInteger, 4)
   ps.System.add_register("b", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("a", 3)(state)
   ps.Init_Unsafe("b", 5)(state)

   ps.Swap_General_General("a", "b")(state)
   # a = 5, b = 3

GetMid_UInt_UInt（中点计算）
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.GetMid_UInt_UInt
   :members:
   :undoc-members:

**操作**: ``mid ^= (left + right) // 2``

**用途**: 二分搜索算法。

.. code-block:: python

   ps.System.clear()

   ps.System.add_register("left", ps.UnsignedInteger, 4)
   ps.System.add_register("right", ps.UnsignedInteger, 4)
   ps.System.add_register("mid", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Init_Unsafe("left", 2)(state)
   ps.Init_Unsafe("right", 8)(state)

   ps.GetMid_UInt_UInt("left", "right", "mid")(state)
   # mid = (2 + 8) // 2 = 5
