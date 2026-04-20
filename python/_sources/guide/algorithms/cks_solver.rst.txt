CKS 线性系统求解器
==================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

CKS（Childs-Kothari-Somma）算法求解线性系统 Ax = b，相比经典迭代方法具有指数级加速。它使用量子游走和酉组合（LCU）实现 κ 条件数矩阵的 O(κ log(κ/ε)) 复杂度。

本教程演示使用 PySparQ 寄存器级编程范式的完整实现。

数学背景
--------

算法原理
~~~~~~~~

CKS 算法的操作步骤：

1. **哈密顿量编码**：通过块编码将矩阵 A 编码为哈密顿量
2. **量子游走**：实现游走算子 W = T† · R · T · Swap
3. **LCU 构建**：使用切比雪夫系数组合游走步数
4. **解提取**：对辅助量子比特后选择得到 ``|x⟩``

关键洞察：解 ``|x⟩`` ∝ A⁻¹``|b⟩`` 可以通过适当系数的量子游走迭代来表达。

量子游走
~~~~~~~~

游走算子为：

.. math::

   W = T^\dagger \cdot R \cdot T \cdot \text{Swap}

其中：

- **T**：态制备算子：``|j⟩|0⟩ → |j⟩|ψⱼ⟩``
- **R**：对特定态的反射（相位翻转）
- **Swap**：交换行和列寄存器

T 算子在非零列上创建叠加态：

.. math::

   |\psi_j\rangle = \sum_{k: A_{jk} \neq 0} \sqrt{|A_{jk}|} |k\rangle

LCU 构建
~~~~~~~~

解构造为：

.. math::

   |x\rangle \propto \sum_{j=0}^{j_0} c_j W^j |b\rangle

其中 c_j 为切比雪夫多项式系数。

实现步骤
--------

步骤 1：导入和设置
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import numpy as np
   import pysparq as ps
   from pysparq.algorithms.cks_solver import (
       cks_solve,
       SparseMatrix,
       ChebyshevPolynomialCoefficient,
       QuantumWalkNSteps,
       LCUContainer,
   )

步骤 2：切比雪夫多项式系数
~~~~~~~~~~~~~~~~~~~~~~~~~~

系数决定游走迭代的权重：

.. code-block:: python

   # 根据条件数和精度计算 b
   kappa = 10.0  # 条件数
   eps = 0.01    # 期望精度
   b = int(kappa * kappa * (np.log(kappa) - np.log(eps)))

   # 创建系数计算器
   cheb = ChebyshevPolynomialCoefficient(b)

   # 访问系数
   for j in range(min(5, cheb.b)):
       coef = cheb.coef(j)
       sign = cheb.sign(j)  # True 表示负数
       step = cheb.step(j)  # 游走步数 = 2j + 1
       print(f"j={j}: 系数={coef:.4f}, 符号={sign}, 步数={step}")

   # 输出:
   # j=0: 系数=1.9922, 符号=False, 步数=1
   # j=1: 系数=1.9766, 符号=True, 步数=3
   # j=2: 系数=1.9531, 符号=False, 步数=5

步骤 3：稀疏矩阵表示
~~~~~~~~~~~~~~~~~~~

将稠密矩阵转换为 QRAM 兼容的稀疏格式：

.. code-block:: python

   # 创建 2x2 矩阵
   A_dense = np.array([[2, 1], [1, 2]], dtype=float)

   # 转换为稀疏表示
   mat = SparseMatrix.from_dense(A_dense, data_size=32)

   print(f"行数: {mat.n_row}")
   print(f"每行最大非零元素: {mat.nnz_col}")
   print(f"数据大小（比特）: {mat.data_size}")
   print(f"仅正数: {mat.positive_only}")

步骤 4：初始化量子态
~~~~~~~~~~~~~~~~~~~

创建寄存器和初始状态：

.. code-block:: python

   ps.System.clear()

   # 创建游走算子管理器
   walk = QuantumWalkNSteps(mat)

   # 初始化量子寄存器
   walk.init_environment()

   # 创建初始状态
   state = walk.create_state()

   # 将行寄存器初始化为叠加态
   init_size = int(np.log2(mat.n_row)) + 1
   ps.Hadamard_Int(walk.j, init_size)(state)
   ps.ClearZero()(state)

步骤 5：应用量子游走步数
~~~~~~~~~~~~~~~~~~~~~~~~

执行游走迭代：

.. code-block:: python

   # 第一步（特殊初始化）
   walk.first_step(state)
   print(f"第一步后状态大小: {state.size()}")

   # 额外步数
   n_steps = 3
   for i in range(n_steps):
       walk.step(state)
       print(f"步数 {i+1}: 状态大小 = {state.size()}")

步骤 6：LCU 构建
~~~~~~~~~~~~~~~

用系数组合游走步数：

.. code-block:: python

   # 创建 LCU 容器
   kappa = np.linalg.cond(A_dense)
   eps = 0.01

   lcu = LCUContainer(mat, kappa, eps)
   lcu.initialize()

   # 用 |b> 态初始化
   def init_b(state):
       ps.Hadamard_Int(lcu.get_input_reg(), init_size)(state)

   lcu.external_input(init_b)

   # 运行 LCU 迭代
   lcu.iterate()

步骤 7：完整求解器
~~~~~~~~~~~~~~~~~

使用高级求解器函数：

.. code-block:: python

   # 定义问题
   A = np.array([[2, 1], [1, 2]], dtype=float)
   b = np.array([1, 1], dtype=float)

   # 使用 CKS 求解
   x = cks_solve(A, b, eps=0.01)

   print(f"解: {x}")
   print(f"验证 Ax = {A @ x}")

   # 输出:
   # 解: [0.33333333 0.33333333]
   # 验证 Ax = [1. 1.]

完整示例
--------

.. code-block:: python

   import numpy as np
   from pysparq.algorithms.cks_solver import cks_solve

   # 示例 1：对角占优矩阵
   A = np.array([
       [4, 1, 0],
       [1, 4, 1],
       [0, 1, 4]
   ], dtype=float)
   b = np.array([1, 2, 1], dtype=float)

   print("求解 Ax = b")
   print(f"A = \n{A}")
   print(f"b = {b}")

   x = cks_solve(A, b, eps=0.01)
   print(f"\n解 x = {x}")
   print(f"验证 Ax = {A @ x}")

   # 与经典解比较
   x_classical = np.linalg.solve(A, b)
   print(f"经典解 = {x_classical}")
   print(f"误差 = {np.linalg.norm(x - x_classical)}")

   # 示例 2：条件数影响
   A_ill = np.array([
       [1, 0.99],
       [0.99, 1]
   ], dtype=float)
   b_ill = np.array([1, 0], dtype=float)

   kappa = np.linalg.cond(A_ill)
   print(f"\n病态矩阵, kappa = {kappa:.2f}")
   x_ill = cks_solve(A_ill, b_ill, kappa=kappa, eps=0.001)
   print(f"解 = {x_ill}")

游走角度函数
------------

对于矩阵元素 A[j,k]，游走应用旋转：

.. code-block:: python

   from pysparq.algorithms.cks_solver import get_coef_positive_only

   # 对于正矩阵元素
   mat_data_size = 8
   v = 128  # 矩阵元素值（量化）

   # 获取旋转矩阵
   rot = get_coef_positive_only(mat_data_size, v, row=0, col=0)
   print(f"旋转矩阵: [[{rot[0]:.4f}, {rot[1]:.4f}]")
   print(f"                 [{rot[2]:.4f}, {rot[3]:.4f}]]")

   # 对于 v=128, Amax=255:
   # x = sqrt(128/255) ≈ 0.71
   # y = sqrt(1 - 128/255) ≈ 0.71
   # 矩阵 = [[0.71, -0.71], [0.71, 0.71]]

复杂度分析
----------

时间复杂度
~~~~~~~~~~

CKS 算法实现：

.. math::

   T = O(\kappa \log(\kappa/\epsilon))

相比经典迭代方法的 O(κ² log(κ/ε))。

空间复杂度
~~~~~~~~~~

对于 n×n 矩阵需要 O(log n) 个量子比特，相比经典的 O(n) 实现指数级空间压缩。

API 参考
---------

.. autoclass:: pysparq.algorithms.cks_solver.ChebyshevPolynomialCoefficient
   :members:

.. autoclass:: pysparq.algorithms.cks_solver.SparseMatrix
   :members:

.. autoclass:: pysparq.algorithms.cks_solver.QuantumWalkNSteps
   :members:

.. autoclass:: pysparq.algorithms.cks_solver.LCUContainer
   :members:

.. autofunction:: pysparq.algorithms.cks_solver.cks_solve
