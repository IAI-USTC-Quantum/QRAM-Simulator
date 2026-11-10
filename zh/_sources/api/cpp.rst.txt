C++ API 参考
============

本章由 `Doxygen <https://www.doxygen.nl>`_ + `Breathe
<https://breathe.readthedocs.io>`_ 从源码头文件的注释自动生成，
按核心工作类组织。完整头文件（含自由函数与工具）见仓库
`QRAM/include/ <https://github.com/IAI-USTC-Quantum/QRAM-Simulator/tree/main/QRAM/include>`__
与 `Common/include/ <https://github.com/IAI-USTC-Quantum/QRAM-Simulator/tree/main/Common/include>`__。

.. note::

   各条目上方的 ``cppapi-*`` 显式锚点供其他文档页深链使用，
   与自动生成的 Doxygen 锚点互不冲突。

QRAM 模块
---------

电路（qubit 架构）
~~~~~~~~~~~~~~~~~~

.. _cppapi-qramcircuit-qubit:

.. doxygenstruct:: qram_simulator::qram_qubit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

电路（qutrit 架构）
~~~~~~~~~~~~~~~~~~~

.. _cppapi-qramcircuit-qutrit:

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

时序与噪声调度
~~~~~~~~~~~~~~

.. _cppapi-timestep:

.. doxygenstruct:: qram_simulator::TimeStep
   :project: QRAM-Simulator
   :members:

.. _cppapi-timeslices:

.. doxygenstruct:: qram_simulator::TimeSlices
   :project: QRAM-Simulator
   :members:

.. _cppapi-operationpack:

.. doxygenstruct:: qram_simulator::OperationPack
   :project: QRAM-Simulator
   :members:

.. _cppapi-operation:

.. doxygenstruct:: qram_simulator::Operation
   :project: QRAM-Simulator
   :members:

.. _cppapi-continuousrange:

.. doxygenstruct:: qram_simulator::ContinuousRange
   :project: QRAM-Simulator
   :members:

分支结构（qubit 架构）
~~~~~~~~~~~~~~~~~~~~~~

.. _cppapi-branchgroup:

.. doxygenstruct:: qram_simulator::qram_qubit::BranchGroup
   :project: QRAM-Simulator
   :members:

.. _cppapi-branch-qubit:

.. doxygenstruct:: qram_simulator::qram_qubit::Branch
   :project: QRAM-Simulator
   :members:

.. _cppapi-systemstate:

.. doxygenstruct:: qram_simulator::qram_qubit::SystemState
   :project: QRAM-Simulator
   :members:

.. _cppapi-state:

.. doxygenstruct:: qram_simulator::qram_qubit::State
   :project: QRAM-Simulator
   :members:

分支结构（qutrit 架构）
~~~~~~~~~~~~~~~~~~~~~~~

.. _cppapi-branch-qutrit:

.. doxygenstruct:: qram_simulator::qram_qutrit::Branch
   :project: QRAM-Simulator
   :members:

.. _cppapi-subbranch:

.. doxygenstruct:: qram_simulator::qram_qutrit::SubBranch
   :project: QRAM-Simulator
   :members:

.. _cppapi-qramstate:

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMState
   :project: QRAM-Simulator
   :members:

.. _cppapi-qramnode:

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMNode
   :project: QRAM-Simulator
   :members:

Common 模块
-----------

全振幅桥接
~~~~~~~~~~

.. _cppapi-qramfullamp:

.. doxygenclass:: qram_simulator::QRAMFullAmp
   :project: QRAM-Simulator
   :members:

全振幅电路原语
~~~~~~~~~~~~~~

.. _cppapi-measureresult:

.. doxygenstruct:: qram_simulator::quantum_simulator::MeasureResult
   :project: QRAM-Simulator
   :members:

矩阵
~~~~

.. _cppapi-sparsematrix:

.. doxygenstruct:: qram_simulator::SparseMatrix
   :project: QRAM-Simulator
   :members:

.. _cppapi-densematrix:

.. doxygenstruct:: qram_simulator::DenseMatrix
   :project: QRAM-Simulator
   :members:

.. _cppapi-densevector:

.. doxygenstruct:: qram_simulator::DenseVector
   :project: QRAM-Simulator
   :members:

随机引擎
~~~~~~~~

.. _cppapi-random-engine:

.. doxygenstruct:: qram_simulator::random_engine
   :project: QRAM-Simulator
   :members:

日志与剖析
~~~~~~~~~~

.. _cppapi-logger:

.. doxygenstruct:: qram_simulator::Logger
   :project: QRAM-Simulator
   :members:

.. _cppapi-profiler:

.. doxygenstruct:: qram_simulator::profiler
   :project: QRAM-Simulator
   :members:

.. _cppapi-statistic:

.. doxygenstruct:: qram_simulator::Statistic
   :project: QRAM-Simulator
   :members:

.. _cppapi-outputter:

.. doxygenstruct:: qram_simulator::Outputter
   :project: QRAM-Simulator
   :members:

迭代工具
~~~~~~~~

.. _cppapi-range:

.. doxygenclass:: qram_simulator::range
   :project: QRAM-Simulator
   :members:

.. _cppapi-product:

.. doxygenclass:: qram_simulator::product
   :project: QRAM-Simulator
   :members:
