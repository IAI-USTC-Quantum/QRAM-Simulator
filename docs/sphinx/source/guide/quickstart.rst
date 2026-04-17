Quick Start
===========

Register Level Programming
--------------------------

PySparQ uses a "Register Level Programming" paradigm. Instead of composing circuits from individual gates, you operate directly on named quantum registers.

Basic Workflow
--------------

1. Create a quantum system
2. Create a sparse quantum state
3. Allocate registers (integers, booleans)
4. Apply quantum operations
5. Read measurement results

Example: Quantum Addition
-------------------------

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, Add_UInt_UInt, Hadamard_Int

   # Create system and state
   system = System()
   state = SparseState(system)

   # Allocate two 4-bit unsigned integer registers
   AddRegister("a", pysparq.UnsignedInteger, 4)(state)
   AddRegister("b", pysparq.UnsignedInteger, 4)(state)

   # Put both registers in superposition
   Hadamard_Int("a")(state)
   Hadamard_Int("b")(state)

   # Quantum addition: a += b
   Add_UInt_UInt("b", "a")(state)

   # State now contains all possible sums in superposition
   print(state)

Conditional Operations
----------------------

Operations can be conditioned on other registers:

.. code-block:: python

   # Allocate a control register
   AddRegister("control", pysparq.Boolean, 1)(state)

   # Apply operation only when control is |1>
   Add_UInt_UInt("a", "b").conditioned_by_nonzeros("control")(state)

Control Types
^^^^^^^^^^^^^

- :meth:`conditioned_by_nonzeros(reg)` - Condition on register being non-zero
- :meth:`conditioned_by_all_ones(reg)` - Condition on register being all ones
- :meth:`conditioned_by_bit(reg, pos)` - Condition on specific bit
- :meth:`conditioned_by_value(reg, pos)` - Condition on value at position
