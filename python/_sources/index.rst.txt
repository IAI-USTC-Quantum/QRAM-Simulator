PySparQ Documentation
=====================

PySparQ is a sparse-state quantum circuit simulator with native QRAM support and Register Level Programming paradigm.

.. raw:: html

   <div class="badges" style="display: flex; gap: 10px; flex-wrap: wrap; margin-bottom: 20px;">
     <a href="https://arxiv.org/abs/2503.13832"><img src="https://img.shields.io/badge/QRAM_Simulator-arXiv%3A2503%2E13832-b31b1b.svg" alt="arXiv"></a>
     <a href="https://arxiv.org/abs/2503.15118"><img src="https://img.shields.io/badge/SparQ-arXiv%3A2503%2E15118-6f42c1.svg" alt="arXiv"></a>
     <a href="https://pypi.org/project/pysparq/"><img src="https://img.shields.io/pypi/v/pysparq.svg" alt="PyPI"></a>
     <a href="https://github.com/IAI-USTC-Quantum/QRAM-Simulator"><img src="https://img.shields.io/badge/GitHub-Repo-181717?logo=github" alt="GitHub"></a>
     <a href="https://iai-ustc-quantum.github.io/QRAM-Simulator/"><img src="https://img.shields.io/badge/docs-GitHub%20Pages-4D6AE4" alt="Documentation"></a>
   </div>

Quick Links
-----------

* `GitHub Repository <https://github.com/IAI-USTC-Quantum/QRAM-Simulator>`_ - Source code and issues
* `Landing Page <https://iai-ustc-quantum.github.io/QRAM-Simulator/>`_ - Project overview and C++ API docs

.. toctree::
   :maxdepth: 2
   :caption: User Guide

   guide/installation
   guide/quickstart
   guide/examples
   guide/algorithms/index

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   api/index
   autoapi/pysparq/index

Getting Started
---------------

Install PySparQ via pip:

.. code-block:: bash

   pip install pysparq

Quick example:

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, Hadamard_Int

   system = System()
   state = SparseState(system)
   AddRegister("q", pysparq.UnsignedInteger, 4)(state)
   Hadamard_Int("q")(state)
   print(state)

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
