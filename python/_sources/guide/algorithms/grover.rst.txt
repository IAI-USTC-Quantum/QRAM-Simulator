Grover's Quantum Search Algorithm
==================================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

Overview
--------

Grover's algorithm provides quadratic speedup for unstructured search
problems. Given a database of N items, the algorithm finds a marked
item in O(√N) quantum queries, compared to O(N) classical queries.

This tutorial demonstrates the implementation using PySparQ's
Register Level Programming paradigm.

Mathematical Background
-----------------------

The Algorithm
~~~~~~~~~~~~~

Grover's algorithm operates on two quantum registers:

1. **Address register** (n qubits): Encodes indices 0 to N-1 where N = 2^n
2. **Data register**: Stores the value at the loaded address via QRAM

The algorithm consists of three main steps:

1. **Initialization**: Prepare equal superposition over all addresses

   .. math::

      |\psi_0\rangle = \frac{1}{\sqrt{N}} \sum_{x=0}^{N-1} |x\rangle

2. **Grover Iteration**: Repeatedly apply G = D · O

   - **Oracle O**: Marks target states with a negative phase

     .. math::

        O|x\rangle = (-1)^{f(x)} |x\rangle

     where f(x) = 1 if x is marked, 0 otherwise.

   - **Diffusion D**: Amplifies amplitude of marked states

     .. math::

        D = 2|s\rangle\langle s| - I

     where |s⟩ is the uniform superposition.

3. **Measurement**: Read out the address register

Amplitude Amplification
~~~~~~~~~~~~~~~~~~~~~~~

After k iterations, the amplitude of marked states is:

.. math::

   a_k = \sin((2k+1)\theta)

where :math:`\theta = \arcsin(\sqrt{M/N})` and M is the number of marked items.

Optimal number of iterations:

.. math::

   k_{opt} \approx \frac{\pi}{4}\sqrt{\frac{N}{M}}

Implementation Steps
--------------------

Step 1: Import and Setup
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import pysparq as ps
   from pysparq.algorithms.grover import GroverOracle, DiffusionOperator, grover_search

   # Clear any existing quantum state
   ps.System.clear()

Step 2: Create QRAM Memory
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The QRAM stores the data to be searched. Each address maps to a data value.

.. code-block:: python

   # Define memory to search
   memory = [5, 12, 3, 8, 15, 7, 2, 9]
   target = 8

   # Calculate address register size (log2 of memory size)
   import math
   n_bits = int(math.log2(len(memory))) + 1

   # Create QRAM circuit
   # addr_size: number of address bits
   # data_size: number of bits per data value (64 for full integers)
   qram = ps.QRAMCircuit_qutrit(n_bits, 64, memory)

Step 3: Initialize Quantum State
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create the quantum state and necessary registers.

.. code-block:: python

   # Create quantum state
   state = ps.SparseState()

   # Add registers
   addr_reg = ps.AddRegister("addr", ps.UnsignedInteger, n_bits)(state)
   data_reg = ps.AddRegister("data", ps.UnsignedInteger, 64)(state)
   search_reg = ps.AddRegister("search", ps.UnsignedInteger, 64)(state)

   # Initialize search value to target
   ps.Init_Unsafe("search", target)(state)

   # Create superposition over addresses
   ps.Hadamard_Int_Full("addr")(state)

Step 4: Build Oracle
~~~~~~~~~~~~~~~~~~~~

The oracle marks states where data matches the target.

.. code-block:: python

   # Create oracle
   oracle = GroverOracle(qram, "addr", "data", "search")

The oracle performs:

1. Load: ``|addr⟩|0⟩ → |addr⟩|data[addr]⟩`` via QRAM
2. Compare: Check if ``data == target``
3. Phase flip: Apply -1 to matching states
4. Uncompute: Reverse comparison and QRAM load

Step 5: Build Diffusion Operator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The diffusion operator amplifies marked states.

.. code-block:: python

   # Create diffusion operator
   diffusion = DiffusionOperator("addr")

The diffusion operator performs: H · P₀ · H

Where P₀ applies a phase flip to the |0⟩ state.

Step 6: Apply Grover Iterations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Combine oracle and diffusion for amplitude amplification.

.. code-block:: python

   # Calculate optimal iterations
   n_iterations = int(math.pi / 4 * math.sqrt(len(memory)))

   # Apply iterations
   for _ in range(n_iterations):
       # Add temporary data register for oracle
       data_id = ps.AddRegister("data_tmp", ps.UnsignedInteger, 64)(state)

       oracle(state)
       diffusion(state)

       # Clean up
       ps.RemoveRegister("data_tmp")(state)

Step 7: Measure Result
~~~~~~~~~~~~~~~~~~~~~~~

Extract the address via partial trace.

.. code-block:: python

   # Measure by tracing out data and search registers
   measured_results, probability = ps.PartialTrace(["data", "search"])(state)

   index = measured_results[0]
   print(f"Found target at index {index}")
   print(f"Memory[{index}] = {memory[index]}")

Complete Example
----------------

.. code-block:: python

   import pysparq as ps
   from pysparq.algorithms.grover import grover_search

   # Define search problem
   memory = [5, 12, 3, 8, 15, 7, 2, 9]
   target = 8

   # Execute Grover search
   index, prob = grover_search(memory, target)

   print(f"Memory: {memory}")
   print(f"Target: {target}")
   print(f"Found at index {index}")
   print(f"Memory[{index}] = {memory[index]}")
   print(f"Probability: {prob:.4f}")

   # Output:
   # Memory: [5, 12, 3, 8, 15, 7, 2, 9]
   # Target: 8
   # Found at index 3
   # Memory[3] = 8
   # Probability: 0.9453

Quantum Counting
----------------

The ``grover_count`` function uses phase estimation to estimate
the number of marked items.

.. code-block:: python

   from pysparq.algorithms.grover import grover_count

   # Memory with duplicates
   memory = [5, 5, 5, 8, 8, 7, 2, 9]
   target = 5

   count, prob = grover_count(memory, target, precision_bits=8)
   print(f"Estimated {count} occurrences of {target}")

API Reference
-------------

.. autoclass:: pysparq.algorithms.grover.GroverOracle
   :members:
   :show-inheritance:

.. autoclass:: pysparq.algorithms.grover.DiffusionOperator
   :members:
   :show-inheritance:

.. autoclass:: pysparq.algorithms.grover.GroverOperator
   :members:
   :show-inheritance:

.. autofunction:: pysparq.algorithms.grover.grover_search

.. autofunction:: pysparq.algorithms.grover.grover_count
