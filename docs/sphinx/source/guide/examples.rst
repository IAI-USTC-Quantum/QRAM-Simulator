Examples
========

This section provides comprehensive examples demonstrating PySparQ's capabilities.

.. contents:: Table of Contents
   :local:

Example 1: QFT-based Phase Estimation
--------------------------------------

.. code-block:: python

   from pysparq import (
       System, SparseState, AddRegister,
       Hadamard_Int, QFT, inverseQFT
   )

   # Initialize system
   system = System()
   state = SparseState(system)

   # Create registers for phase estimation
   n_precision = 4
   AddRegister("precision", pysparq.UnsignedInteger, n_precision)(state)
   AddRegister("eigenstate", pysparq.Boolean, 1)(state)

   # Prepare eigenstate |1>
   pysparq.Xgate_Bool("eigenstate")(state)

   # Apply Hadamard to precision register
   Hadamard_Int("precision")(state)

   # Apply controlled-U operations (simplified)
   # ... custom controlled operations here ...

   # Apply inverse QFT
   inverseQFT("precision")(state)

   # Measure the precision register
   print(state)

Example 2: QRAM Data Loading
-----------------------------

.. code-block:: python

   import numpy as np
   from pysparq import System, SparseState, AddRegister, QRAMLoad, Hadamard_Int

   # Initialize system
   system = System()
   state = SparseState(system)

   # Create address and data registers
   n_address = 3  # 8 memory locations
   n_data = 4     # 4-bit data width

   AddRegister("addr", pysparq.UnsignedInteger, n_address)(state)
   AddRegister("data", pysparq.UnsignedInteger, n_data)(state)

   # Create classical data to load
   memory = np.array([1, 3, 5, 7, 2, 4, 6, 8], dtype=np.uint64)

   # Put address in superposition (query all addresses simultaneously)
   Hadamard_Int("addr")(state)

   # QRAM load: data = memory[addr]
   qram = QRAMLoad("addr", "data", memory)
   qram(state)

   # State now contains superposition of all (addr, memory[addr]) pairs
   print(state)

Example 3: Quantum Binary Search
---------------------------------

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, QuantumBinarySearch

   # Initialize system
   system = System()
   state = SparseState(system)

   # Create sorted array (classical oracle)
   sorted_array = [2, 5, 8, 12, 16, 23, 38, 56, 72, 91]

   # Search for value 23
   target = 23
   n_bits = 4  # log2(len(sorted_array))

   AddRegister("index", pysparq.UnsignedInteger, n_bits)(state)

   # Perform quantum binary search
   qbs = QuantumBinarySearch(sorted_array, target, "index")
   result_idx = qbs(state)

   print(f"Found {target} at index {result_idx}")

Example 4: Block Encoding
-------------------------

.. code-block:: python

   from pysparq import System, SparseState, AddRegister, Block_Encoding_Tridiagonal

   # Initialize system
   system = System()
   state = SparseState(system)

   # Block encode a tridiagonal matrix
   # Matrix: [[a, b, 0], [c, d, e], [0, f, g]]
   alpha = 0.5  # diagonal parameter
   beta = 0.3   # off-diagonal parameter

   n_qubits = 2
   AddRegister("system", pysparq.UnsignedInteger, n_qubits)(state)
   AddRegister("ancilla", pysparq.Boolean, 1)(state)

   # Create block encoding operator
   block_encoder = Block_Encoding_Tridiagonal(alpha, beta, n_qubits)
   block_encoder(state)

   print("Block encoding applied successfully")
