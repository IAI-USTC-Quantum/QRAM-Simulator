示例
====

本节提供展示 PySparQ 功能的综合示例。

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

示例 1：基于 QFT 的相位估计
----------------------------

.. code-block:: python

   from pysparq import (
       System, SparseState, AddRegister,
       Hadamard_Int, QFT, inverseQFT
   )

   # 初始化系统
   system = System()
   state = SparseState(system)

   # 创建相位估计所需的寄存器
   n_precision = 4
   AddRegister("precision", pysparq.UnsignedInteger, n_precision)(state)
   AddRegister("eigenstate", pysparq.Boolean, 1)(state)

   # 准备本征态 |1>
   pysparq.Xgate_Bool("eigenstate")(state)

   # 对精度寄存器应用 Hadamard
   Hadamard_Int("precision")(state)

   # 应用受控-U操作（简化版）
   # ... 这里添加自定义受控操作 ...

   # 应用逆 QFT
   inverseQFT("precision")(state)

   # 测量精度寄存器
   print(state)

示例 2：QRAM 数据加载
---------------------

.. code-block:: python

   import numpy as np
   from pysparq import System, SparseState, AddRegister, QRAMLoad, Hadamard_Int

   # 初始化系统
   system = System()
   state = SparseState(system)

   # 创建地址和数据寄存器
   n_address = 3  # 8个内存位置
   n_data = 4     # 4位数据宽度

   AddRegister("addr", pysparq.UnsignedInteger, n_address)(state)
   AddRegister("data", pysparq.UnsignedInteger, n_data)(state)

   # 创建要加载的经典数据
   memory = np.array([1, 3, 5, 7, 2, 4, 6, 8], dtype=np.uint64)

   # 将地址置于叠加态（同时查询所有地址）
   Hadamard_Int("addr")(state)

   # QRAM 加载：data = memory[addr]
   qram = QRAMLoad("addr", "data", memory)
   qram(state)

   # 状态现在包含所有 (addr, memory[addr]) 对的叠加态
   print(state)

示例 3：量子二分搜索
-------------------

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, QuantumBinarySearch

   # 初始化系统
   system = System()
   state = SparseState(system)

   # 创建有序数组（经典预言机）
   sorted_array = [2, 5, 8, 12, 16, 23, 38, 56, 72, 91]

   # 搜索值 23
   target = 23
   n_bits = 4  # log2(len(sorted_array))

   AddRegister("index", pysparq.UnsignedInteger, n_bits)(state)

   # 执行量子二分搜索
   qbs = QuantumBinarySearch(sorted_array, target, "index")
   result_idx = qbs(state)

   print(f"在索引 {result_idx} 处找到 {target}")

示例 4：块编码
-------------

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, Block_Encoding_Tridiagonal

   # 初始化系统
   system = System()
   state = SparseState(system)

   # 块编码三对角矩阵
   # 矩阵: [[a, b, 0], [c, d, e], [0, f, g]]
   alpha = 0.5  # 对角线参数
   beta = 0.3   # 非对角线参数

   n_qubits = 2
   AddRegister("system", pysparq.UnsignedInteger, n_qubits)(state)
   AddRegister("ancilla", pysparq.Boolean, 1)(state)

   # 创建块编码算子
   block_encoder = Block_Encoding_Tridiagonal(alpha, beta, n_qubits)
   block_encoder(state)

   print("块编码应用成功")
