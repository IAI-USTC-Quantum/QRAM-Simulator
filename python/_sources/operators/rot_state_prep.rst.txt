旋转与态制备
============

旋转与态制备算子提供任意维幺正旋转和从 ``|0⟩`` 制备目标量子态的能力。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 旋转与态制备算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正类
   * - ``Rot_GeneralUnitary``
     - 作用任意维幺正矩阵
     - BaseOperator
   * - ``Rot_GeneralStatePrep``
     - 从 ``|0⟩`` 制备目标量子态
     - BaseOperator

---

Rot_GeneralUnitary（通用幺正旋转）
----------------------------------

.. autoclass:: pysparq.Rot_GeneralUnitary
   :members:
   :undoc-members:

**操作**: 对寄存器作用一个任意 :math:`2^n \times 2^n` 幺正矩阵。

**参数**:

- ``reg`` — 目标寄存器（名称或 ID）
- ``matrix`` — 幺正矩阵（``DenseMatrix_complex`` 或 NumPy 数组）

**Dagger**: 作用矩阵的共轭转置。

**用途**: 当标准门库无法直接表达所需的变换时，可以通过矩阵形式直接指定。

.. note::

   矩阵维度必须与寄存器的希尔伯特空间维度 :math:`2^n` 匹配，其中 :math:`n` 为寄存器比特数。

.. code-block:: python

   import numpy as np
   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("q", ps.UnsignedInteger, 2)

   state = ps.SparseState()

   # 定义 4x4 幺正矩阵
   matrix = np.eye(4, dtype=complex)
   matrix[0, 0] = 0
   matrix[0, 3] = 1
   matrix[3, 3] = 0
   matrix[3, 0] = 1

   op = ps.Rot_GeneralUnitary("q", matrix)
   op(state)

   # 撤销
   op.dag(state)

---

Rot_GeneralStatePrep（量子态制备）
----------------------------------

.. autoclass:: pysparq.Rot_GeneralStatePrep
   :members:
   :undoc-members:

**操作**: 将寄存器从 ``|0⟩`` 态制备为目标量子态。

**参数**:

- ``reg`` — 目标寄存器（名称或 ID）
- ``state_vector`` — 目标态矢量（复数列表或 NumPy 数组）

**Dagger**: 将目标态映射回 ``|0⟩``。

**数学表示**: 给定目标态 :math:`|\psi\rangle = \sum_i \alpha_i |i\rangle`，构造幺正 :math:`U` 使得 :math:`U|0\rangle = |\psi\rangle`。

.. warning::

   该算子要求寄存器当前处于 ``|0⟩`` 态。如果寄存器已有非零值，行为未定义。

.. code-block:: python

   import numpy as np
   import pysparq as ps

   ps.System.clear()
   ps.System.add_register("q", ps.UnsignedInteger, 2)

   state = ps.SparseState()

   # 制备 Bell 态的简化版：均匀叠加
   target = np.array([0.5, 0.5, 0.5, 0.5], dtype=complex)

   op = ps.Rot_GeneralStatePrep("q", target)
   op(state)

   ps.StatePrint()(state)
   # |q=0⟩ : (0.5+0j)
   # |q=1⟩ : (0.5+0j)
   # |q=2⟩ : (0.5+0j)
   # |q=3⟩ : (0.5+0j)

辅助函数
--------

stateprep_unitary_build_schmidt
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autofunction:: pysparq.stateprep_unitary_build_schmidt

**操作**: 使用 Schmidt 分解构造态制备所需的幺正矩阵。

**用途**: 当需要手动构建或检查 ``Rot_GeneralStatePrep`` 的内部矩阵时使用。
