基本量子门
==========

基本量子门实现单量子比特和多量子比特的标准量子门操作。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 基本量子门总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正类
   * - ``Xgate_Bool``
     - Pauli-X（比特翻转）
     - SelfAdjoint
   * - ``Ygate_Bool``
     - Pauli-Y
     - SelfAdjoint
   * - ``Zgate_Bool``
     - Pauli-Z（相位翻转）
     - SelfAdjoint
   * - ``Sgate_Bool``
     - S 门（π/2 相位）
     - SelfAdjoint
   * - ``Tgate_Bool``
     - T 门（π/4 相位）
     - SelfAdjoint
   * - ``Phase_Bool``
     - 任意相位 e^{iλ}
     - BaseOperator
   * - ``RXgate_Bool``
     - X 轴旋转
     - SelfAdjoint
   * - ``RYgate_Bool``
     - Y 轴旋转
     - SelfAdjoint
   * - ``RZgate_Bool``
     - Z 轴旋转
     - SelfAdjoint
   * - ``SXgate_Bool``
     - √X 门
     - SelfAdjoint
   * - ``U2gate_Bool``
     - 通用单量子比特门（2 参数）
     - BaseOperator
   * - ``U3gate_Bool``
     - 通用单量子比特门（3 参数）
     - BaseOperator

类型约束
--------

所有量子门要求：

- 寄存器类型：``Boolean``（单量子比特门）
- 位索引：必须在寄存器大小范围内 [0, size)

.. code-block:: python

   # 正确：Boolean 类型用于单量子比特门
   ps.System.add_register("qubit", ps.Boolean, 1)
   ps.Xgate_Bool("qubit", 0)(state)

   # 错误：位索引超出范围
   # ps.Xgate_Bool("qubit", 1)(state)  # 抛出异常！

---

Pauli 门
--------

Xgate_Bool（Pauli-X / NOT）
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Xgate_Bool
   :members:
   :undoc-members:

**操作**: 比特翻转 |0⟩ ↔ |1⟩

**矩阵**:

.. math::

   X = \begin{pmatrix} 0 & 1 \\ 1 & 0 \end{pmatrix}

.. code-block:: python

   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("q", ps.Boolean, 1)

   state = ps.SparseState()
   # 初始 |q=0⟩

   ps.Xgate_Bool("q", 0)(state)
   # |q=1⟩

   # 再次应用恢复原状态（自伴）
   ps.Xgate_Bool("q", 0)(state)
   # |q=0⟩

Ygate_Bool（Pauli-Y）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Ygate_Bool
   :members:
   :undoc-members:

**矩阵**:

.. math::

   Y = \begin{pmatrix} 0 & -i \\ i & 0 \end{pmatrix}

.. code-block:: python

   ps.Ygate_Bool("q", 0)(state)

Zgate_Bool（Pauli-Z）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Zgate_Bool
   :members:
   :undoc-members:

**操作**: 相位翻转 |1⟩ → -|1⟩

**矩阵**:

.. math::

   Z = \begin{pmatrix} 1 & 0 \\ 0 & -1 \end{pmatrix}

.. code-block:: python

   ps.Zgate_Bool("q", 0)(state)

---

相位门
------

Sgate_Bool（S 门）
^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Sgate_Bool
   :members:
   :undoc-members:

**操作**: 相位旋转 π/2

**矩阵**:

.. math::

   S = \begin{pmatrix} 1 & 0 \\ 0 & i \end{pmatrix}

.. code-block:: python

   ps.Sgate_Bool("q", 0)(state)

Tgate_Bool（T 门）
^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Tgate_Bool
   :members:
   :undoc-members:

**操作**: 相位旋转 π/4

**矩阵**:

.. math::

   T = \begin{pmatrix} 1 & 0 \\ 0 & e^{i\pi/4} \end{pmatrix}

.. code-block:: python

   ps.Tgate_Bool("q", 0)(state)

Phase_Bool（任意相位）
^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Phase_Bool
   :members:
   :undoc-members:

**操作**: 相位旋转 e^{iλ}

**Dagger**: 相位旋转 e^{-iλ}

.. code-block:: python

   # 相位旋转 π/3
   op = ps.Phase_Bool("q", 0, np.pi / 3)
   op(state)

   # 撤销
   op.dag(state)

---

旋转门
------

RXgate_Bool（X 轴旋转）
^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.RXgate_Bool
   :members:
   :undoc-members:

**矩阵**:

.. math::

   R_X(\theta) = \begin{pmatrix} \cos\frac{\theta}{2} & -i\sin\frac{\theta}{2} \\ -i\sin\frac{\theta}{2} & \cos\frac{\theta}{2} \end{pmatrix}

.. code-block:: python

   import numpy as np

   # X 轴旋转 π/2
   ps.RXgate_Bool("q", np.pi / 2)(state)

RYgate_Bool（Y 轴旋转）
^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.RYgate_Bool
   :members:
   :undoc-members:

**矩阵**:

.. math::

   R_Y(\theta) = \begin{pmatrix} \cos\frac{\theta}{2} & -\sin\frac{\theta}{2} \\ \sin\frac{\theta}{2} & \cos\frac{\theta}{2} \end{pmatrix}

.. code-block:: python

   ps.RYgate_Bool("q", np.pi / 2)(state)

RZgate_Bool（Z 轴旋转）
^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.RZgate_Bool
   :members:
   :undoc-members:

**矩阵**:

.. math::

   R_Z(\theta) = \begin{pmatrix} e^{-i\theta/2} & 0 \\ 0 & e^{i\theta/2} \end{pmatrix}

.. code-block:: python

   ps.RZgate_Bool("q", np.pi / 2)(state)

SXgate_Bool（√X 门）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.SXgate_Bool
   :members:
   :undoc-members:

**操作**: X^{1/2}

.. code-block:: python

   ps.SXgate_Bool("q", 0)(state)

---

通用门
------

U2gate_Bool（2 参数通用门）
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.U2gate_Bool
   :members:
   :undoc-members:

**矩阵**:

.. math::

   U_2(\phi, \lambda) = \frac{1}{\sqrt{2}} \begin{pmatrix} 1 & -e^{i\lambda} \\ e^{i\phi} & e^{i(\phi+\lambda)} \end{pmatrix}

U3gate_Bool（3 参数通用门）
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.U3gate_Bool
   :members:
   :undoc-members:

**矩阵**:

.. math::

   U_3(\theta, \phi, \lambda) = \begin{pmatrix} \cos\frac{\theta}{2} & -e^{i\lambda}\sin\frac{\theta}{2} \\ e^{i\phi}\sin\frac{\theta}{2} & e^{i(\phi+\lambda)}\cos\frac{\theta}{2} \end{pmatrix}

.. code-block:: python

   import numpy as np

   op = ps.U3gate_Bool("q", np.pi/4, np.pi/2, 0)
   op(state)

   # 撤销
   op.dag(state)

---

其他门
------

Rot_Bool（通用旋转）
^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Rot_Bool
   :members:
   :undoc-members:

**操作**: 应用任意 2×2 幺正矩阵

.. code-block:: python

   import numpy as np

   # 定义 2x2 矩阵
   matrix = np.array([
       [np.cos(np.pi/4), -np.sin(np.pi/4)],
       [np.sin(np.pi/4), np.cos(np.pi/4)]
   ])

   ps.Rot_Bool("q", matrix)(state)

Reflection_Bool（反射门）
^^^^^^^^^^^^^^^^^^^^^^^^^

.. autoclass:: pysparq.Reflection_Bool
   :members:
   :undoc-members:

**操作**: 反射操作（Grover 算法中的扩散算子）
