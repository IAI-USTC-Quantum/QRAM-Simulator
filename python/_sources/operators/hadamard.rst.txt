Hadamard 操作
=============

Hadamard 算子在寄存器上创建量子叠加态，是量子算法的基础操作。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: Hadamard 算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 类型约束
     - 幺正类
   * - ``Hadamard_Int``
     - 整数寄存器 Hadamard
     - 整数类型
     - SelfAdjoint
   * - ``Hadamard_Int_Full``
     - 完整 Hadamard（所有输出状态）
     - 整数类型
     - SelfAdjoint
   * - ``Hadamard_Bool``
     - 单量子比特 Hadamard
     - Boolean（size=1）
     - SelfAdjoint
   * - ``Hadamard_PartialQubit``
     - 部分量子比特 Hadamard
     - 整数类型
     - SelfAdjoint

---

Hadamard_Int
------------

.. autoclass:: pysparq.Hadamard_Int
   :members:
   :undoc-members:

**操作**: 对整数寄存器的指定量子比特应用 Hadamard

**数学定义**:

.. math::

   H|x\rangle = \frac{1}{\sqrt{2}}(|0\rangle + |1\rangle) \quad \text{for each qubit}

**类型约束**: ``UnsignedInteger`` 或 ``SignedInteger``

.. code-block:: python

   import pysparq as ps

   ps.System.clear()

   # 4 比特寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 4)

   state = ps.SparseState()
   print("初始状态:")
   ps.StatePrint()(state)
   # [1 basis state]
   # |q=0⟩ : (1+0j)

   # 对前 2 比特应用 Hadamard
   ps.Hadamard_Int("q", 2)(state)

   print("\nHadamard 后:")
   ps.StatePrint()(state)
   # [2 basis states]
   # |q=0⟩ : (0.707+0j)
   # |q=2⟩ : (0.707+0j)

---

Hadamard_Int_Full
-----------------

.. autoclass:: pysparq.Hadamard_Int_Full
   :members:
   :undoc-members:

**操作**: 对整个寄存器应用完整 Hadamard，创建所有 :math:`2^n` 个状态的均匀叠加

**数学定义**:

.. math::

   H^n|x\rangle = \frac{1}{\sqrt{2^n}}\sum_{y=0}^{2^n-1}|y\rangle

**类型约束**: 整数类型

**注意**: 会创建 :math:`2^n` 个基态，当 n 较大时可能导致内存问题。

.. code-block:: python

   ps.System.clear()

   # 2 比特寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 2)

   state = ps.SparseState()

   # 完整 Hadamard：创建 2^2 = 4 个状态
   ps.Hadamard_Int_Full("q")(state)

   ps.StatePrint()(state)
   # [4 basis states]
   # |q=0⟩ : (0.5+0j)
   # |q=1⟩ : (0.5+0j)
   # |q=2⟩ : (0.5+0j)
   # |q=3⟩ : (0.5+0j)

   # 再次应用 = 撤销（自伴）
   ps.Hadamard_Int_Full("q")(state)
   # 回到单基态

---

Hadamard_Bool
-------------

.. autoclass:: pysparq.Hadamard_Bool
   :members:
   :undoc-members:

**操作**: 单量子比特 Hadamard

**矩阵**:

.. math::

   H = \frac{1}{\sqrt{2}}\begin{pmatrix} 1 & 1 \\ 1 & -1 \end{pmatrix}

**类型约束**: ``Boolean``（寄存器大小必须为 1）

**位约束**: 无（默认操作第 0 位）

.. code-block:: python

   ps.System.clear()

   # 单量子比特寄存器
   ps.System.add_register("qubit", ps.Boolean, 1)  # 必须是 1 比特！

   state = ps.SparseState()

   ps.Hadamard_Bool("qubit")(state)

   ps.StatePrint()(state)
   # [2 basis states]
   # |qubit=0⟩ : (0.707+0j)
   # |qubit=1⟩ : (0.707+0j)

---

Hadamard_PartialQubit
---------------------

.. autoclass:: pysparq.Hadamard_PartialQubit
   :members:
   :undoc-members:

**操作**: 对寄存器中的指定量子比特应用 Hadamard

**类型约束**: 整数类型

**位约束**: 指定位置必须在寄存器范围内

.. code-block:: python

   ps.System.clear()

   # 4 比特寄存器
   ps.System.add_register("q", ps.UnsignedInteger, 4)

   state = ps.SparseState()

   # 只对第 1 和第 3 位应用 Hadamard
   positions = {1, 3}
   ps.Hadamard_PartialQubit("q", positions)(state)

   # 创建 2^2 = 4 个叠加状态（仅翻转位置 1 和 3）

---

使用场景
--------

均匀叠加态
^^^^^^^^^^

用于量子搜索、量子采样等算法：

.. code-block:: python

   # 创建地址寄存器的均匀叠加
   ps.System.add_register("addr", ps.UnsignedInteger, n)
   state = ps.SparseState()
   ps.Hadamard_Int_Full("addr")(state)

   # 现在 addr 在所有可能值上均匀分布

单个量子比特初始化
^^^^^^^^^^^^^^^^^^

用于初始化控制比特：

.. code-block:: python

   ps.System.add_register("ctrl", ps.Boolean, 1)
   ps.Hadamard_Bool("ctrl")(state)

   # ctrl 处于 |+⟩ = (|0⟩ + |1⟩)/√2 状态

部分叠加态
^^^^^^^^^^

用于选择性叠加：

.. code-block:: python

   # 只叠加低 2 比特
   ps.Hadamard_Int("reg", 2)(state)

   # 或叠加特定位置
   ps.Hadamard_PartialQubit("reg", {0, 2})(state)