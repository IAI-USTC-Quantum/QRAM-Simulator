C++ API Reference
=================

This chapter is generated automatically from the comments in the source
header files by `Doxygen <https://www.doxygen.nl>`_ + `Breathe
<https://breathe.readthedocs.io>`_, organized by core working class. The complete
headers (including free functions and utilities) live in the repository under
`QRAM/include/ <https://github.com/IAI-USTC-Quantum/QRAM-Simulator/tree/main/QRAM/include>`__
and `Common/include/ <https://github.com/IAI-USTC-Quantum/QRAM-Simulator/tree/main/Common/include>`__.

.. note::

   The ``cppapi-*`` explicit anchors above each entry exist so that other
   documentation pages can deep-link to a single class; they do not conflict
   with the automatically generated Doxygen anchors.

QRAM Module
-----------

Circuits (qubit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _cppapi-qramcircuit-qubit:

.. doxygenstruct:: qram_simulator::qram_qubit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

Circuits (qutrit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _cppapi-qramcircuit-qutrit:

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

Timing and noise scheduling
~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Branch structures (qubit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Branch structures (qutrit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Common Module
-------------

Full-amplitude bridge
~~~~~~~~~~~~~~~~~~~~~

.. _cppapi-qramfullamp:

.. doxygenclass:: qram_simulator::QRAMFullAmp
   :project: QRAM-Simulator
   :members:

Full-amplitude circuit primitives
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _cppapi-measureresult:

.. doxygenstruct:: qram_simulator::quantum_simulator::MeasureResult
   :project: QRAM-Simulator
   :members:

Matrices
~~~~~~~~

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

Random engine
~~~~~~~~~~~~~

.. _cppapi-random-engine:

.. doxygenstruct:: qram_simulator::random_engine
   :project: QRAM-Simulator
   :members:

Logging and profiling
~~~~~~~~~~~~~~~~~~~~~

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

Iteration utilities
~~~~~~~~~~~~~~~~~~~

.. _cppapi-range:

.. doxygenclass:: qram_simulator::range
   :project: QRAM-Simulator
   :members:

.. _cppapi-product:

.. doxygenclass:: qram_simulator::product
   :project: QRAM-Simulator
   :members:
