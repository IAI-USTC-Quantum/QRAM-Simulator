相位与反射算子
==============

相位与反射算子实现条件相位翻转、全局相位和 Grover 反射等操作，广泛用于量子搜索和振幅放大算法。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 相位算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正类
   * - ``ZeroConditionalPhaseFlip``
     - 指定寄存器全为零时翻转相位
     - SelfAdjoint
   * - ``Reflection_Bool``
     - 关于 ``|0⟩`` 态的反射（Grover 扩散）
     - SelfAdjoint
   * - ``GlobalPhase_Int``
     - 全局相位乘以复数因子
     - BaseOperator

---

ZeroConditionalPhaseFlip（零条件相位翻转）
------------------------------------------

.. autoclass:: pysparq.ZeroConditionalPhaseFlip
   :members:
   :undoc-members:

**操作**: 当所有指定寄存器的值均为零时，对基态施加 :math:`-1` 相位翻转。

**参数**: 寄存器标识列表（名称列表或 ID 列表）。

**数学表示**:

.. math::

   |x_1, x_2, \ldots, x_n\rangle \rightarrow \begin{cases} -|x_1, x_2, \ldots, x_n\rangle & \text{if } x_1 = x_2 = \cdots = x_n = 0 \\ |x_1, x_2, \ldots, x_n\rangle & \text{otherwise} \end{cases}

**用途**: 构造 Grover 算法中的 Oracle。当目标态为 ``|0...0⟩`` 时，该算子即为相位 Oracle。

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("addr", ps.UnsignedInteger, 3)
   ps.System.add_register("data", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   ps.Hadamard_Int("addr", 3)(state)
   # 加载后，当 addr 对应的 data 为 0 时翻转相位
   ps.ZeroConditionalPhaseFlip(["data"])(state)

   # 也可以用寄存器名称列表
   ps.ZeroConditionalPhaseFlip(["addr", "data"])(state)

---

Reflection_Bool（反射）
----------------------

.. autoclass:: pysparq.Reflection_Bool
   :members:
   :undoc-members:

**操作**: 关于 ``|0⟩`` 态的反射操作，即 Grover 扩散算子中的核心组件。

**参数**:

- ``reg`` — 目标寄存器（名称或 ID，或列表）
- ``inverse`` — 是否使用逆反射（可选，默认 ``False``）

**数学表示**:

.. math::

   R = 2|0\rangle\langle 0| - I

**用途**: Grover 算法的扩散算子（Diffusion Operator）由 Hadamard + Reflection + Hadamard 组合而成。

.. code-block:: python

   ps.System.clear()
   ps.System.add_register("q", ps.Boolean, 1)

   state = ps.SparseState()
   ps.Hadamard_Bool("q")(state)

   # 反射操作
   ps.Reflection_Bool("q")(state)

---

GlobalPhase_Int（全局相位）
--------------------------

.. autoclass:: pysparq.GlobalPhase_Int
   :members:
   :undoc-members:

**操作**: 对整个量子态乘以一个复数全局相位因子。

**参数**: ``c`` — 复数相位因子。

**Dagger**: 乘以共轭复数。

**注意**: 全局相位在量子力学中不可观测，但在某些算法（如振幅编码、QDA）中需要精确追踪。

.. code-block:: python

   import numpy as np

   # 全局相位旋转 e^{iπ/4}
   op = ps.GlobalPhase_Int(np.exp(1j * np.pi / 4))
   op(state)

   # 撤销
   op.dag(state)
