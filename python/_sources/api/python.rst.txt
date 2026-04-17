Python API Reference
====================

PySparQ exposes all quantum operations as Python classes and functions through the :mod:`pysparq` module.

.. _api-modules:

Core Classes
------------

- :class:`~pysparq._core.System` - System management for quantum registers
- :class:`~pysparq._core.SparseState` - Sparse quantum state representation
- :class:`~pysparq._core.BaseOperator` - Base class for quantum operators
- :class:`~pysparq._core.SelfAdjointOperator` - Self-adjoint quantum operators

Quantum Arithmetic Operators
----------------------------

- :func:`~pysparq._core.Add_UInt_UInt` - Add two unsigned integer registers
- :func:`~pysparq._core.Add_UInt_ConstUInt` - Add constant to unsigned integer register
- :func:`~pysparq._core.Add_ConstUInt` - Add constant unsigned integer
- :func:`~pysparq._core.Mult_UInt_ConstUInt` - Multiply unsigned integer by constant
- :func:`~pysparq._core.AddAssign_AnyInt_AnyInt` - Add and assign operation

Quantum Gates
-------------

- :func:`~pysparq._core.Hadamard_Int` - Hadamard gate on integer register
- :func:`~pysparq._core.Hadamard_Bool` - Hadamard gate on boolean register
- :func:`~pysparq._core.QFT` - Quantum Fourier Transform
- :func:`~pysparq._core.inverseQFT` - Inverse Quantum Fourier Transform
- :func:`~pysparq._core.Xgate_Bool` - Pauli X gate on boolean
- :func:`~pysparq._core.Ygate_Bool` - Pauli Y gate on boolean
- :func:`~pysparq._core.Zgate_Bool` - Pauli Z gate on boolean

QRAM Operations
---------------

- :func:`~pysparq._core.QRAMLoad` - QRAM load operation
- :func:`~pysparq._core.QRAMLoadFast` - Fast QRAM load operation
- :func:`~pysparq._core.QRAMCircuit_qutrit` - QRAM circuit using qutrits

State Management
----------------

- :func:`~pysparq._core.AddRegister` - Add a new quantum register
- :func:`~pysparq._core.RemoveRegister` - Remove a quantum register
- :func:`~pysparq._core.Push` - Push operation
- :func:`~pysparq._core.Pop` - Pop operation
- :func:`~pysparq._core.Normalize` - Normalize quantum state
- :func:`~pysparq._core.CheckNormalization` - Check state normalization

Full API Documentation
----------------------

For complete API documentation with all classes, functions, and methods, see the :doc:`API Reference </autoapi/pysparq/index>`.