黑魔法操作
==========

.. raw:: html

   <div class="admonition warning">
   <p class="admonition-title">警告</p>
   <p>本节操作绕过正常的量子力学约束，<strong>不保证幺正性</strong>。仅在调试、初始化或明确了解后果时使用。</p>
   </div>

黑魔法操作提供直接修改量子态内部表示的能力，不经过正常的量子门变换。这些操作不对应任何物理量子操作。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 黑魔法操作总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正性
   * - ``Normalize``
     - 归一化量子态
     - 非幺正
   * - ``Init_Unsafe``
     - 直接设置寄存器值
     - 非幺正

---

Normalize（归一化）
------------------

.. autoclass:: pysparq.Normalize
   :members:
   :undoc-members:

**操作**: 将整个 ``SparseState`` 的振幅归一化，使得 :math:`\sum_i |\alpha_i|^2 = 1`。

**参数**: 无。

**用途**: 当手工操作（如 ``Init_Unsafe``）或数值误差导致量子态未归一化时，使用此算子修复。

.. code-block:: python

   import pysparq as ps

   # 假设某些操作导致量子态未归一化
   ps.Normalize()(state)

   # 可以通过 ViewNormalization 检查
   ps.ViewNormalization()(state)

---

Init_Unsafe（不安全初始化）
---------------------------

.. autoclass:: pysparq.Init_Unsafe
   :members:
   :undoc-members:

**操作**: 直接将指定寄存器的值设置为给定常数，**不经过任何量子门**。

**参数**:

- ``reg`` — 目标寄存器（名称或 ID）
- ``value`` — 要设置的值（整数）

**行为**: 遍历所有基态，将指定寄存器的值强制设为 ``value``。如果多个基态在设置后键值重复，它们的振幅会相加。

.. warning::

   此操作 **不保证幺正性**。它会破坏叠加态信息——如果寄存器原本处于叠加态，所有非 ``value`` 的分量将被覆盖。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("a", ps.UnsignedInteger, 4)
   ps.System.add_register("b", ps.UnsignedInteger, 4)

   state = ps.SparseState()

   # 直接设置初始值
   ps.Init_Unsafe("a", 3)(state)
   ps.Init_Unsafe("b", 5)(state)

   ps.pprint(state)
   # |a=3,b=5⟩ : (1+0j)

.. note::

   ``Init_Unsafe`` 最常见的用途是在算法开始时设置输入寄存器的初值。在叠加态上使用时需格外小心。
