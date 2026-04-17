Python API Reference
====================

PySparQ exposes all quantum operations as Python classes and functions through the :mod:`pysparq` module.

.. _api-modules:

Core Classes
------------

.. autoapisummary::

   pysparq.System
   pysparq.SparseState
   pysparq.BaseOperator
   pysparq.SelfAdjointOperator

Quantum Arithmetic Operators
----------------------------

.. autoapisummary::

   pysparq.Add_UInt_UInt
   pysparq.Add_UInt_ConstUInt
   pysparq.Add_ConstUInt
   pysparq.Mult_UInt_ConstUInt
   pysparq.AddAssign_AnyInt_AnyInt

Quantum Gates
-------------

.. autoapisummary::

   pysparq.Hadamard_Int
   pysparq.Hadamard_Bool
   pysparq.QFT
   pysparq.inverseQFT
   pysparq.Xgate_Bool
   pysparq.Ygate_Bool
   pysparq.Zgate_Bool

QRAM Operations
---------------

.. autoapisummary::

   pysparq.QRAMLoad
   pysparq.QRAMLoadFast
   pysparq.QRAMCircuit_qutrit

State Management
----------------

.. autoapisummary::

   pysparq.AddRegister
   pysparq.RemoveRegister
   pysparq.Push
   pysparq.Pop
   pysparq.Normalize
   pysparq.CheckNormalization

Full API Documentation
----------------------

For complete API documentation with all classes, functions, and methods, see the :doc:`API Reference </autoapi/pysparq/index>`.