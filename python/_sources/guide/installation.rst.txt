Installation
============

Requirements
------------

- Python 3.9 or higher
- NumPy

Install from PyPI
------------------

.. code-block:: bash

   pip install pysparq

Install from Source
-------------------

.. code-block:: bash

   git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
   cd QRAM-Simulator
   pip install .

Build Requirements
------------------

For building from source, you need:

- CMake 3.15+
- C++17 compatible compiler
- OpenMP (required for parallelization)

Optional:

- CUDA 12+ (for GPU acceleration)

Verify Installation
-------------------

.. code-block:: python

   import pysparq
   pysparq.test_import()

   # Create a quantum system
   system = pysparq.System()
   state = pysparq.SparseState(system)

   print("PySparQ installed successfully!")
