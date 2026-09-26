QRAM-Simulator 文档
===================

.. rubric:: QRAM 电路模拟核心——qubit / qutrit 两种架构的噪声稀疏分支仿真

QRAM-Simulator 是面向 QRAM（量子随机存储器）装载电路的高性能 C++ 模拟核心：

- **两种架构**：qubit 架构（物理比特树 + good/bad 分支剪枝）与
  qutrit 架构（三能级节点树），共享同一时序与噪声调度器 ``TimeStep``；
- **噪声模型**：比特翻转 / 相位翻转 / 联合翻转 / 去极化 / 振幅衰减
  （Damping），可按时间片注入任意组合；
- **保真度评估**：基于输出采样的装载保真度，支持剪枝运行与
  不剪枝 ground-truth 基准对拍；
- **Python 绑定**：``pip install qram-simulator`` 即可获得全部核心
  工作类（QRAMCircuitQubit / QRAMCircuitQutrit / QRAMFullAmp / TimeStep）。

.. toctree::
   :maxdepth: 2
   :caption: 使用指南

   guide/installation
   guide/quickstart
   guide/architecture
   guide/operators
   guide/naming_conventions

.. toctree::
   :maxdepth: 2
   :caption: API 参考

   api/index

.. toctree::
   :maxdepth: 2
   :caption: 论文与实验

   paper/README
   paper/reproduction
   paper/exactness_validation
   paper/joint_damping
   paper/qubit_qram_pruning
   paper/qubit_error_propagation

索引
----

* :ref:`genindex`
* :ref:`search`
