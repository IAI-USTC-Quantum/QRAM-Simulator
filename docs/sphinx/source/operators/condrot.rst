条件旋转算子
============

条件旋转算子根据输入寄存器的值对目标寄存器进行旋转操作。

.. contents:: 目录
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

.. list-table:: 条件旋转算子总览
   :header-rows: 1

   * - 算子
     - 操作
     - 幺正类
   * - ``CondRot_Fixed_Bool``
     - 基于 Rational 角度寄存器的固定条件旋转
     - BaseOperator
   * - ``CondRot_General_Bool_fast``
     - 旧的模拟器快速路径，自定义函数直接给出旋转矩阵
     - BaseOperator

旋转机制
--------

条件旋转将输入寄存器的值转换为旋转角度，然后应用于输出寄存器。

对于 Rational 类型输入：

.. math::

   \theta = \frac{value}{2^n} \cdot 2\pi

旋转矩阵：

.. math::

   R(\theta) = \begin{pmatrix} \cos\theta & -\sin\theta \\ \sin\theta & \cos\theta \end{pmatrix}

---

CondRot_Fixed_Bool
------------------

.. autoclass:: pysparq.CondRot_Fixed_Bool
   :members:
   :undoc-members:

**操作**: 根据 Rational 寄存器值旋转 Boolean 寄存器

**类型约束**:
- 输入寄存器: ``Rational``
- 输出寄存器: ``Boolean``（大小必须为 1）

**Dagger**: 使用反向旋转矩阵

.. code-block:: python

   import pysparq as ps
   import numpy as np

   ps.System.clear()

   # Rational 用于角度编码
   ps.System.add_register("angle", ps.Rational, 16)  # 高精度角度
   ps.System.add_register("target", ps.Boolean, 1)

   state = ps.SparseState()

   # 设置角度（如 π/4 → value = 2^16 / 8 = 8192）
   angle_value = int(2**16 * 0.125)  # 0.125 = 1/8 圈
   ps.Init_Unsafe("angle", angle_value)(state)

   # 初始化目标为 |0⟩
   # 初始状态: |angle=8192, target=0⟩

   # 条件旋转
   op = ps.CondRot_Fixed_Bool("angle", "target")
   op(state)

   ps.pprint(state)
   # target 状态被旋转角度 ≈ π/4

   # 撤销
   op.dag(state)

CondRot_General_Bool_fast
-------------------------

.. autoclass:: pysparq.CondRot_General_Bool_fast
   :members:
   :undoc-members:

**操作**: 使用自定义函数直接生成 2x2 旋转矩阵。该接口保留给模拟器快速路径；新算法实现应优先写成“算术 adapter 计算 Rational 角度 → ``CondRot_Fixed_Bool`` → uncompute adapter”。

**类型约束**:
- 输入寄存器: 任意整数类型
- 输出寄存器: ``Boolean``（大小必须为 1）

**自定义函数**: 接受 ``uint64_t`` 值，返回 2×2 旋转矩阵

.. note::

   自定义函数需要返回有效的幺正矩阵。

.. code-block:: python

   import pysparq as ps
   import numpy as np

   ps.System.clear()

   ps.System.add_register("input", ps.UnsignedInteger, 8)
   ps.System.add_register("target", ps.Boolean, 1)

   state = ps.SparseState()

   # 自定义角度函数
   def my_rotation(value: int) -> np.ndarray:
       # 将 8 位整数映射到 [0, 2π)
       theta = value * 2 * np.pi / 256

       # 返回旋转矩阵
       return np.array([
           [np.cos(theta), -np.sin(theta)],
           [np.sin(theta), np.cos(theta)]
       ])

   op = ps.CondRot_General_Bool_fast("input", "target", my_rotation)

   # 测试不同输入值
   ps.Init_Unsafe("input", 64)(state)  # θ = 64/256 * 2π = π/2
   op(state)

   ps.pprint(state)

   # 撤销（需要手动实现逆）
   def my_rotation_inv(value: int) -> np.ndarray:
       theta = value * 2 * np.pi / 256
       return np.array([
           [np.cos(theta), np.sin(theta)],
           [-np.sin(theta), np.cos(theta)]
       ])

   op_inv = ps.CondRot_General_Bool_fast("input", "target", my_rotation_inv)
   op_inv(state)

使用场景
--------

相位估计辅助
^^^^^^^^^^^^

.. code-block:: python

   # 角度寄存器存储估计相位
   ps.System.add_register("phase", ps.Rational, 16)
   ps.System.add_register("ancilla", ps.Boolean, 1)

   # 初始化 ancilla 为 |+⟩
   ps.Hadamard_Bool("ancilla")(state)

   # 条件旋转编码相位信息
   ps.CondRot_Fixed_Bool("phase", "ancilla")(state)

哈密顿模拟
^^^^^^^^^^

.. code-block:: python

   # 时间步长角度
   dt = 0.01
   angle_val = int(dt * 2**16)

   ps.Init_Unsafe("dt_angle", angle_val)(state)
   ps.CondRot_Fixed_Bool("dt_angle", "qubit")(state)

量子振幅编码
^^^^^^^^^^^^^^

.. code-block:: python

   # 根据数据值旋转目标比特
   def amplitude_encoding(value: int) -> np.ndarray:
       theta = np.arccos(value / 255.0)  # 假设 8 位数据
       return np.array([
           [np.cos(theta), -np.sin(theta)],
           [np.sin(theta), np.cos(theta)]
       ])

   # Preferred pattern:
   # 1. compute angle into a Rational register using an arithmetic adapter
   # 2. rotate with CondRot_Fixed_Bool
   # 3. apply the same adapter again to uncompute
   ps.CondRot_Fixed_Bool("angle", "qubit")(state)
