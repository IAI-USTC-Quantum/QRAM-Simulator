QDA 线性系统求解器
==================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

QDA（量子离散绝热）算法为求解线性系统 Ax = b 提供最优缩放，实现 O(κ log(κ/ε)) 复杂度，其中 κ 为条件数。这是量子线性系统求解器的最优复杂度。

本教程演示使用 PySparQ 寄存器级编程范式的完整实现。

数学背景
--------

离散绝热定理
~~~~~~~~~~~~

算法使用离散绝热演化来制备解态 ``|x⟩`` ∝ A⁻¹``|b⟩``。关键洞察是插值哈密顿量 H(s) 的基态从易于制备的初始态平滑演化到期望的解。

插值哈密顿量
~~~~~~~~~~~~

定义 H(s) = (1 - f(s))H₀ + f(s)H₁，其中：

- **H₀** = σ_z ⊗ I（初始哈密顿量，易于制备基态）
- **H₁** = A ⊗ ``|b⟩⟨b|``（问题哈密顿量）
- **f(s)** = κ/(κ-1) × (1 - (1 + s(κ^(p-1) - 1))^(1/(1-p)))

插值函数 f(s)（公式 69）的选择是为了实现最优缩放。

块编码
~~~~~~

算法使用块编码表示 H(s)：

.. math::

   U_{H(s)} = \begin{pmatrix} H(s) & \cdot \\ \cdot & \cdot \end{pmatrix}

量子游走
~~~~~~~~

游走算子 W_s 组合块编码和反射：

.. math::

   W_s = R \cdot U_{H(s)}

通过 LCU 构建重复应用。

实现步骤
--------

步骤 1：导入和设置
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import numpy as np
   import pysparq as ps
   from pysparq.algorithms.qda_solver import (
       qda_solve,
       compute_fs,
       compute_rotation_matrix,
       WalkS,
       LCU,
       Filtering,
       BlockEncodingHs,
   )

步骤 2：插值参数 f(s)
~~~~~~~~~~~~~~~~~~~~~

计算每个离散点的插值参数：

.. code-block:: python

   # 参数
   kappa = 10.0  # 条件数
   p = 0.5       # 调度参数

   # 计算 s 在 [0, 1] 上的 f(s)
   for s in np.linspace(0, 1, 5):
       fs = compute_fs(s, kappa, p)
       print(f"s = {s:.2f}: f(s) = {fs:.4f}")

   # 输出:
   # s = 0.00: f(s) = 0.0000
   # s = 0.25: f(s) = 0.1234
   # s = 0.50: f(s) = 0.3456
   # s = 0.75: f(s) = 0.6789
   # s = 1.00: f(s) = 1.0000

步骤 3：旋转矩阵 R_s
~~~~~~~~~~~~~~~~~~~

计算块编码的旋转矩阵：

.. code-block:: python

   # 对于 f(s) = 0.5
   fs = 0.5
   R_s = compute_rotation_matrix(fs)

   print("旋转矩阵 R_s:")
   print(f"  [[{R_s[0].real:.4f}, {R_s[1].real:.4f}],")
   print(f"   [{R_s[2].real:.4f}, {R_s[3].real:.4f}]]")

   # 对于 f(s) = 0.5:
   # [[0.7071, 0.7071],
   #  [0.7071, -0.7071]]

步骤 4：初始化量子态
~~~~~~~~~~~~~~~~~~~

创建必要的量子寄存器：

.. code-block:: python

   ps.System.clear()

   n = 2  # 矩阵大小为 2^k
   n_bits = int(np.log2(n)) + 1

   # 主数据寄存器
   main_reg = ps.AddRegister("main", ps.UnsignedInteger, n_bits)(None)

   # 块编码辅助寄存器
   anc_UA = ps.AddRegister("anc_UA", ps.UnsignedInteger, n_bits)(None)
   anc_1 = ps.AddRegister("anc_1", ps.Boolean, 1)(None)
   anc_2 = ps.AddRegister("anc_2", ps.Boolean, 1)(None)
   anc_3 = ps.AddRegister("anc_3", ps.Boolean, 1)(None)
   anc_4 = ps.AddRegister("anc_4", ps.Boolean, 1)(None)

   # LCU 索引寄存器
   index_reg = ps.AddRegister("index", ps.UnsignedInteger, 8)(None)

步骤 5：H(s) 的块编码
~~~~~~~~~~~~~~~~~~~~~

构建块编码算子：

.. code-block:: python

   from pysparq.algorithms.qda_solver import BlockEncoding, StatePreparation

   # 定义问题
   A = np.array([[2, 1], [1, 2]], dtype=float)
   b = np.array([1, 0], dtype=float)

   # 创建块编码组件
   enc_A = BlockEncoding(A)
   enc_b = StatePreparation(b)

   # 在 s = 0.5 处创建块编码
   fs = compute_fs(0.5, kappa=2.0, p=0.5)

   enc_Hs = BlockEncodingHs(
       enc_A, enc_b,
       "main", "anc_UA",
       "anc_1", "anc_2", "anc_3", "anc_4",
       fs
   )

步骤 6：游走算子
~~~~~~~~~~~~~~~

创建并应用游走算子：

.. code-block:: python

   # 创建游走算子
   walk = WalkS(
       enc_A, enc_b,
       "main", "anc_UA",
       "anc_1", "anc_2", "anc_3", "anc_4",
       s=0.5,          # 当前离散点
       kappa=2.0,      # 条件数
       p=0.5           # 调度参数
   )

   # 应用游走到状态
   state = ps.SparseState()
   walk(state)

步骤 7：LCU 构建
~~~~~~~~~~~~~~~

应用重复游走迭代：

.. code-block:: python

   # 创建 LCU 算子
   lcu = LCU(walk, index_reg="index")

   # 应用 LCU
   state = ps.SparseState()
   lcu(state)

   # 这应用 W^(2^0), W^(2^1), W^(2^2), ... 由索引比特控制

步骤 8：Dolph-Chebyshev 滤波
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

应用滤波以减少误差：

.. code-block:: python

   from pysparq.algorithms.qda_solver import Filtering

   # 创建滤波器
   filtering = Filtering(
       walk,
       index_reg="index",
       anc_h="anc_h",
       epsilon=0.01,
       l=5
   )

   # 应用滤波
   state = ps.SparseState()
   success_prob = filtering(state)
   print(f"成功概率: {success_prob:.4f}")

步骤 9：完整求解器
~~~~~~~~~~~~~~~~~~

使用高级求解器函数：

.. code-block:: python

   # 定义问题
   A = np.array([[2, 1], [1, 2]], dtype=float)
   b = np.array([1, 1], dtype=float)

   # 使用 QDA 求解
   x = qda_solve(A, b, kappa=2.0, eps=0.01)

   print(f"解: {x}")
   print(f"验证 Ax = {A @ x}")

完整示例
--------

.. code-block:: python

   import numpy as np
   from pysparq.algorithms.qda_solver import qda_solve

   # 示例 1：良好条件数矩阵
   A = np.array([
       [4, 1, 0],
       [1, 4, 1],
       [0, 1, 4]
   ], dtype=float)
   b = np.array([1, 2, 1], dtype=float)

   kappa = np.linalg.cond(A)
   print(f"条件数: {kappa:.2f}")

   x = qda_solve(A, b, kappa=kappa, eps=0.01)
   print(f"解: {x}")
   print(f"残差: {np.linalg.norm(A @ x - b):.6f}")

   # 示例 2：更大系统
   n = 8
   A = np.diag(np.ones(n) * 2) + np.diag(np.ones(n-1), 1) + np.diag(np.ones(n-1), -1)
   b = np.ones(n)

   x = qda_solve(A, b)
   print(f"\n大系统 (n={n})")
   print(f"残差范数: {np.linalg.norm(A @ x - b):.6f}")

   # 示例 3：与经典比较
   x_classical = np.linalg.solve(A, b)
   error = np.linalg.norm(x - x_classical)
   print(f"与经典解的误差: {error:.6f}")

Dolph-Chebyshev 窗口
--------------------

滤波使用 Dolph-Chebyshev 多项式实现最优频谱特性：

.. code-block:: python

   from pysparq.algorithms.qda_solver import chebyshev_T, dolph_chebyshev

   # 切比雪夫多项式 T_n(x)
   for n in range(5):
       x = 0.5
       Tn = chebyshev_T(n, x)
       print(f"T_{n}({x}) = {Tn:.4f}")

   # Dolph-Chebyshev 窗口
   epsilon = 0.1
   l = 5
   phi = np.pi / 4

   window_val = dolph_chebyshev(epsilon, l, phi)
   print(f"\nDolph-Chebyshev(eps={epsilon}, l={l}, phi={phi:.4f}) = {window_val:.4f}")

复杂度分析
----------

时间复杂度
~~~~~~~~~~

QDA 算法实现最优缩放：

.. math::

   T = O(\kappa \log(\kappa/\epsilon))

这是量子线性系统求解器的最优复杂度。

空间复杂度
~~~~~~~~~~

对于 n×n 矩阵需要 O(log n) 个量子比特，辅助寄存器有常数开销（通常 5 个辅助量子比特）。

关键公式
--------

插值函数（公式 69）
~~~~~~~~~~~~~~~~~~~

.. math::

   f(s) = \frac{\kappa}{\kappa - 1}\left(1 - \left(1 + s(\kappa^{p-1} - 1)\right)^{\frac{1}{1-p}}\right)

旋转矩阵
~~~~~~~~

.. math::

   R_s = \frac{1}{\sqrt{(1-f)^2 + f^2}} \begin{pmatrix} 1-f & f \\ f & f-1 \end{pmatrix}

API 参考
---------

.. autoclass:: pysparq.algorithms.qda_solver.WalkS
   :members:

.. autoclass:: pysparq.algorithms.qda_solver.LCU
   :members:

.. autoclass:: pysparq.algorithms.qda_solver.Filtering
   :members:

.. autoclass:: pysparq.algorithms.qda_solver.BlockEncodingHs
   :members:

.. autofunction:: pysparq.algorithms.qda_solver.qda_solve

.. autofunction:: pysparq.algorithms.qda_solver.compute_fs

.. autofunction:: pysparq.algorithms.qda_solver.compute_rotation_matrix
