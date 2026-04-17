Grover 量子搜索算法
==================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

概述
----

Grover 算法为非结构化搜索问题提供二次加速。给定包含 N 个元素的数据库，算法在 O(√N) 次量子查询内找到目标元素，而经典查询需要 O(N) 次。

本教程演示使用 PySparQ 的寄存器级编程范式实现该算法。

数学背景
--------

算法原理
~~~~~~~~

Grover 算法在两个量子寄存器上操作：

1. **地址寄存器**（n 个量子比特）：编码索引 0 到 N-1，其中 N = 2^n
2. **数据寄存器**：通过 QRAM 存储加载地址处的值

算法由三个主要步骤组成：

1. **初始化**：在所有地址上准备等权叠加态

   .. math::

      |\psi_0\rangle = \frac{1}{\sqrt{N}} \sum_{x=0}^{N-1} |x\rangle

2. **Grover 迭代**：重复应用 G = D · O

   - **预言机 O**：用负相位标记目标状态

     .. math::

        O|x\rangle = (-1)^{f(x)} |x\rangle

     其中 f(x) = 1 当 x 为目标时，否则为 0。

   - **扩散 D**：放大标记状态的振幅

     .. math::

        D = 2|s\rangle\langle s| - I

     其中 |s⟩ 是均匀叠加态。

3. **测量**：读出地址寄存器

振幅放大
~~~~~~~~

经过 k 次迭代后，标记状态的振幅为：

.. math::

   a_k = \sin((2k+1)\theta)

其中 :math:`\theta = \arcsin(\sqrt{M/N})`，M 为标记项数量。

最优迭代次数：

.. math::

   k_{opt} \approx \frac{\pi}{4}\sqrt{\frac{N}{M}}

实现步骤
--------

步骤 1：导入和设置
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import pysparq as ps
   from pysparq.algorithms.grover import GroverOracle, DiffusionOperator, grover_search

   # 清除现有量子态
   ps.System.clear()

步骤 2：创建 QRAM 内存
~~~~~~~~~~~~~~~~~~~~~~

QRAM 存储要搜索的数据。每个地址映射到一个数据值。

.. code-block:: python

   # 定义要搜索的内存
   memory = [5, 12, 3, 8, 15, 7, 2, 9]
   target = 8

   # 计算地址寄存器大小（内存大小的 log2）
   import math
   n_bits = int(math.log2(len(memory))) + 1

   # 创建 QRAM 电路
   # addr_size: 地址比特数
   # data_size: 每个数据值的比特数（64 表示完整整数）
   qram = ps.QRAMCircuit_qutrit(n_bits, 64, memory)

步骤 3：初始化量子态
~~~~~~~~~~~~~~~~~~~

创建量子态和必要的寄存器。

.. code-block:: python

   # 创建量子态
   state = ps.SparseState()

   # 添加寄存器
   addr_reg = ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
   data_reg = ps.AddRegister("data", ps.UnsignedInteger, 64)(state)
   search_reg = ps.AddRegister("search", ps.UnsignedInteger, 64)(state)

   # 将搜索值初始化为目标值
   ps.Init_Unsafe("search", target)(state)

   # 在地址上创建叠加态
   ps.Hadamard_Int_Full("addr")(state)

步骤 4：构建预言机
~~~~~~~~~~~~~~~~~~

预言机标记数据匹配目标的量子态。

.. code-block:: python

   # 创建预言机
   oracle = GroverOracle(qram, "addr", "data", "search")

预言机执行：

1. 加载：``|addr⟩|0⟩ → |addr⟩|data[addr]⟩``（通过 QRAM）
2. 比较：检查 ``data == target``
3. 相位翻转：对匹配状态应用 -1
4. 撤销：逆序执行比较和 QRAM 加载

步骤 5：构建扩散算子
~~~~~~~~~~~~~~~~~~~

扩散算子放大标记状态的振幅。

.. code-block:: python

   # 创建扩散算子
   diffusion = DiffusionOperator("addr")

扩散算子执行：H · P₀ · H

其中 P₀ 对 |0⟩ 状态应用相位翻转。

步骤 6：执行 Grover 迭代
~~~~~~~~~~~~~~~~~~~~~~~~

组合预言机和扩散算子进行振幅放大。

.. code-block:: python

   # 计算最优迭代次数
   n_iterations = int(math.pi / 4 * math.sqrt(len(memory)))

   # 执行迭代
   for _ in range(n_iterations):
       # 为预言机添加临时数据寄存器
       data_id = ps.AddRegister("data_tmp", ps.UnsignedInteger, 64)(state)

       oracle(state)
       diffusion(state)

       # 清理
       ps.RemoveRegister("data_tmp")(state)

步骤 7：测量结果
~~~~~~~~~~~~~~~

通过部分迹提取地址。

.. code-block:: python

   # 通过对数据和搜索寄存器取部分迹来测量
   measured_results, probability = ps.PartialTrace(["data", "search"])(state)

   index = measured_results[0]
   print(f"在索引 {index} 处找到目标")
   print(f"Memory[{index}] = {memory[index]}")

完整示例
--------

.. code-block:: python

   import pysparq as ps
   from pysparq.algorithms.grover import grover_search

   # 定义搜索问题
   memory = [5, 12, 3, 8, 15, 7, 2, 9]
   target = 8

   # 执行 Grover 搜索
   index, prob = grover_search(memory, target)

   print(f"内存: {memory}")
   print(f"目标: {target}")
   print(f"在索引 {index} 处找到")
   print(f"Memory[{index}] = {memory[index]}")
   print(f"概率: {prob:.4f}")

   # 输出:
   # 内存: [5, 12, 3, 8, 15, 7, 2, 9]
   # 目标: 8
   # 在索引 3 处找到
   # Memory[3] = 8
   # 概率: 0.9453

量子计数
--------

``grover_count`` 函数使用相位估计来估计标记项的数量。

.. code-block:: python

   from pysparq.algorithms.grover import grover_count

   # 带重复元素的内存
   memory = [5, 5, 5, 8, 8, 7, 2, 9]
   target = 5

   count, prob = grover_count(memory, target, precision_bits=8)
   print(f"估计 {target} 出现了 {count} 次")

API 参考
---------

.. autoclass:: pysparq.algorithms.grover.GroverOracle
   :members:
   :show-inheritance:

.. autoclass:: pysparq.algorithms.grover.DiffusionOperator
   :members:
   :show-inheritance:

.. autoclass:: pysparq.algorithms.grover.GroverOperator
   :members:
   :show-inheritance:

.. autofunction:: pysparq.algorithms.grover.grover_search

.. autofunction:: pysparq.algorithms.grover.grover_count
