C++ API 参考
============

本章由 `Doxygen <https://www.doxygen.nl>`_ + `Breathe
<https://breathe.readthedocs.io>`_ 从源码头文件的中文注释自动生成，
按核心工作类组织。完整头文件（含自由函数与工具）见仓库
``QRAM/include/`` 与 ``Common/include/``。

QRAM 模块
---------

电路（qubit 架构）
~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::qram_qubit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

电路（qutrit 架构）
~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

时序与噪声调度
~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::TimeStep
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::TimeSlices
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::OperationPack
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::Operation
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::ContinuousRange
   :project: QRAM-Simulator
   :members:

分支结构（qubit 架构）
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::qram_qubit::BranchGroup
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::qram_qubit::Branch
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::qram_qubit::SystemState
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::qram_qubit::State
   :project: QRAM-Simulator
   :members:

分支结构（qutrit 架构）
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::qram_qutrit::Branch
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::qram_qutrit::SubBranch
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMState
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMNode
   :project: QRAM-Simulator
   :members:

Common 模块
-----------

全振幅桥接
~~~~~~~~~~

.. doxygenclass:: qram_simulator::QRAMFullAmp
   :project: QRAM-Simulator
   :members:

全振幅电路原语
~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::quantum_simulator::MeasureResult
   :project: QRAM-Simulator
   :members:

矩阵
~~~~

.. doxygenstruct:: qram_simulator::SparseMatrix
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::DenseMatrix
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::DenseVector
   :project: QRAM-Simulator
   :members:

随机引擎
~~~~~~~~

.. doxygenstruct:: qram_simulator::random_engine
   :project: QRAM-Simulator
   :members:

日志与剖析
~~~~~~~~~~

.. doxygenstruct:: qram_simulator::Logger
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::profiler
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::Statistic
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::Outputter
   :project: QRAM-Simulator
   :members:

迭代工具
~~~~~~~~

.. doxygenclass:: qram_simulator::range
   :project: QRAM-Simulator
   :members:

.. doxygenclass:: qram_simulator::product
   :project: QRAM-Simulator
   :members:
