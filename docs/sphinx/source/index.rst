PySparQ Documentation
=====================

PySparQ is a sparse-state quantum circuit simulator with native QRAM support and Register Level Programming paradigm.

.. toctree::
   :maxdepth: 2
   :caption: User Guide

   guide/installation
   guide/quickstart
   guide/examples

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
