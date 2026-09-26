C++ API Reference
=================

This chapter is generated automatically from the comments in the source
header files by `Doxygen <https://www.doxygen.nl>`_ + `Breathe
<https://breathe.readthedocs.io>`_, organized by core working class. The complete
headers (including free functions and utilities) live in the repository under
``QRAM/include/`` and ``Common/include/``.

QRAM Module
-----------

Circuits (qubit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::qram_qubit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

Circuits (qutrit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::qram_qutrit::QRAMCircuit
   :project: QRAM-Simulator
   :members:

Timing and noise scheduling
~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Branch structures (qubit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Branch structures (qutrit architecture)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Common Module
-------------

Full-amplitude bridge
~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: qram_simulator::QRAMFullAmp
   :project: QRAM-Simulator
   :members:

Full-amplitude circuit primitives
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::quantum_simulator::MeasureResult
   :project: QRAM-Simulator
   :members:

Matrices
~~~~~~~~

.. doxygenstruct:: qram_simulator::SparseMatrix
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::DenseMatrix
   :project: QRAM-Simulator
   :members:

.. doxygenstruct:: qram_simulator::DenseVector
   :project: QRAM-Simulator
   :members:

Random engine
~~~~~~~~~~~~~

.. doxygenstruct:: qram_simulator::random_engine
   :project: QRAM-Simulator
   :members:

Logging and profiling
~~~~~~~~~~~~~~~~~~~~~

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

Iteration utilities
~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: qram_simulator::range
   :project: QRAM-Simulator
   :members:

.. doxygenclass:: qram_simulator::product
   :project: QRAM-Simulator
   :members:
