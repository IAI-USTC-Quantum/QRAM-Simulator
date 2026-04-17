核心概念
========

本节介绍 PySparQ 稀疏态量子模拟的基础构建模块。理解这些概念是使用 Register Level Programming 范式的前提。

.. toctree::
   :maxdepth: 2

   system
   sparse_state
   register_types

核心抽象
--------

PySparQ 基于三个核心抽象构建：

1. **System** — 单个计算基态（稀疏表示的基本单元）
2. **SparseState** — System 的集合（稀疏量子态）
3. **Registers** — 具有类型存储的命名量子变量

与传统全态矢量模拟器存储 :math:`2^n` 个振幅不同，PySparQ 仅存储非零基态，使得叠加态数量有限的量子算法可以在多项式资源内模拟。
