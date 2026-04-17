CKS Linear System Solver
=========================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

Overview
--------

The CKS (Childs-Kothari-Somma) algorithm solves linear systems Ax = b
with exponential speedup over classical iterative methods. It uses
quantum walk and Linear Combination of Unitaries (LCU) to achieve
O(κ log(κ/ε)) complexity for κ-conditioned matrices.

This tutorial demonstrates the complete implementation using PySparQ's
Register Level Programming paradigm.

Mathematical Background
-----------------------

The Algorithm
~~~~~~~~~~~~~

The CKS algorithm operates by:

1. **Hamiltonian Encoding**: Encode matrix A as a Hamiltonian via block encoding
2. **Quantum Walk**: Implement walk operator W = T† · R · T · Swap
3. **LCU Construction**: Combine walk steps with Chebyshev coefficients
4. **Solution Extraction**: Post-select on ancilla to get |x⟩

Key insight: The solution |x⟩ ∝ A⁻¹|b⟩ can be expressed via quantum walk
iterations with appropriate coefficients.

Quantum Walk
~~~~~~~~~~~~

The walk operator is:

.. math::

   W = T^\dagger \cdot R \cdot T \cdot \text{Swap}

where:

- **T**: State preparation operator: |j⟩|0⟩ → |j⟩|ψⱼ⟩
- **R**: Reflection (phase flip) on certain states
- **Swap**: Exchange row and column registers

The T operator creates superposition over non-zero columns:

.. math::

   |\psi_j\rangle = \sum_{k: A_{jk} \neq 0} \sqrt{|A_{jk}|} |k\rangle

LCU Construction
~~~~~~~~~~~~~~~~~

The solution is constructed as:

.. math::

   |x\rangle \propto \sum_{j=0}^{j_0} c_j W^j |b\rangle

where c_j are Chebyshev polynomial coefficients.

Implementation Steps
--------------------

Step 1: Import and Setup
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import numpy as np
   import pysparq as ps
   from pysparq.algorithms.cks_solver import (
       cks_solve,
       SparseMatrix,
       ChebyshevPolynomialCoefficient,
       QuantumWalkNSteps,
       LCUContainer,
   )

Step 2: Chebyshev Polynomial Coefficients
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The coefficients determine the walk iteration weights:

.. code-block:: python

   # Compute b from condition number and precision
   kappa = 10.0  # Condition number
   eps = 0.01    # Desired precision
   b = int(kappa * kappa * (np.log(kappa) - np.log(eps)))

   # Create coefficient calculator
   cheb = ChebyshevPolynomialCoefficient(b)

   # Access coefficients
   for j in range(min(5, cheb.b)):
       coef = cheb.coef(j)
       sign = cheb.sign(j)  # True if negative
       step = cheb.step(j)  # Walk steps = 2j + 1
       print(f"j={j}: coef={coef:.4f}, sign={sign}, steps={step}")

   # Output:
   # j=0: coef=1.9922, sign=False, steps=1
   # j=1: coef=1.9766, sign=True, steps=3
   # j=2: coef=1.9531, sign=False, steps=5

Step 3: Sparse Matrix Representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Convert dense matrix to QRAM-compatible sparse format:

.. code-block:: python

   # Create 2x2 matrix
   A_dense = np.array([[2, 1], [1, 2]], dtype=float)

   # Convert to sparse representation
   mat = SparseMatrix.from_dense(A_dense, data_size=32)

   print(f"Rows: {mat.n_row}")
   print(f"Max non-zeros per row: {mat.nnz_col}")
   print(f"Data size (bits): {mat.data_size}")
   print(f"Positive only: {mat.positive_only}")

Step 4: Initialize Quantum State
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create registers and initial state:

.. code-block:: python

   ps.System.clear()

   # Create walk operator manager
   walk = QuantumWalkNSteps(mat)

   # Initialize quantum registers
   walk.init_environment()

   # Create initial state
   state = walk.create_state()

   # Initialize row register to superposition
   init_size = int(np.log2(mat.n_row)) + 1
   ps.Hadamard_Int(walk.j, init_size)(state)
   ps.ClearZero()(state)

Step 5: Apply Quantum Walk Steps
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Execute the walk iterations:

.. code-block:: python

   # First step (special initialization)
   walk.first_step(state)
   print(f"State size after first step: {state.size()}")

   # Additional steps
   n_steps = 3
   for i in range(n_steps):
       walk.step(state)
       print(f"Step {i+1}: state size = {state.size()}")

Step 6: LCU Construction
~~~~~~~~~~~~~~~~~~~~~~~~

Combine walk steps with coefficients:

.. code-block:: python

   # Create LCU container
   kappa = np.linalg.cond(A_dense)
   eps = 0.01

   lcu = LCUContainer(mat, kappa, eps)
   lcu.initialize()

   # Initialize with |b> state
   def init_b(state):
       ps.Hadamard_Int(lcu.get_input_reg(), init_size)(state)

   lcu.external_input(init_b)

   # Run LCU iterations
   lcu.iterate()

Step 7: Complete Solver
~~~~~~~~~~~~~~~~~~~~~~~

Use the high-level solver function:

.. code-block:: python

   # Define problem
   A = np.array([[2, 1], [1, 2]], dtype=float)
   b = np.array([1, 1], dtype=float)

   # Solve using CKS
   x = cks_solve(A, b, eps=0.01)

   print(f"Solution: {x}")
   print(f"Verification Ax = {A @ x}")

   # Output:
   # Solution: [0.33333333 0.33333333]
   # Verification Ax = [1. 1.]

Complete Example
----------------

.. code-block:: python

   import numpy as np
   from pysparq.algorithms.cks_solver import cks_solve

   # Example 1: Diagonal dominant matrix
   A = np.array([
       [4, 1, 0],
       [1, 4, 1],
       [0, 1, 4]
   ], dtype=float)
   b = np.array([1, 2, 1], dtype=float)

   print("Solving Ax = b")
   print(f"A = \n{A}")
   print(f"b = {b}")

   x = cks_solve(A, b, eps=0.01)
   print(f"\nSolution x = {x}")
   print(f"Verification Ax = {A @ x}")

   # Compare with classical solution
   x_classical = np.linalg.solve(A, b)
   print(f"Classical solution = {x_classical}")
   print(f"Error = {np.linalg.norm(x - x_classical)}")

   # Example 2: Condition number effect
   A_ill = np.array([
       [1, 0.99],
       [0.99, 1]
   ], dtype=float)
   b_ill = np.array([1, 0], dtype=float)

   kappa = np.linalg.cond(A_ill)
   print(f"\nIll-conditioned matrix, kappa = {kappa:.2f}")
   x_ill = cks_solve(A_ill, b_ill, kappa=kappa, eps=0.001)
   print(f"Solution = {x_ill}")

Walk Angle Functions
--------------------

For matrix element A[j,k], the walk applies a rotation:

.. code-block:: python

   from pysparq.algorithms.cks_solver import get_coef_positive_only

   # For positive matrix elements
   mat_data_size = 8
   v = 128  # Matrix element value (quantized)

   # Get rotation matrix
   rot = get_coef_positive_only(mat_data_size, v, row=0, col=0)
   print(f"Rotation matrix: [[{rot[0]:.4f}, {rot[1]:.4f}]")
   print(f"                 [{rot[2]:.4f}, {rot[3]:.4f}]]")

   # For v=128, Amax=255:
   # x = sqrt(128/255) ≈ 0.71
   # y = sqrt(1 - 128/255) ≈ 0.71
   # Matrix = [[0.71, -0.71], [0.71, 0.71]]

Complexity Analysis
-------------------

Time Complexity
~~~~~~~~~~~~~~~

The CKS algorithm achieves:

.. math::

   T = O(\kappa \log(\kappa/\epsilon))

compared to O(κ² log(κ/ε)) for classical iterative methods.

Space Complexity
~~~~~~~~~~~~~~~~

Requires O(log n) qubits for n×n matrix, achieving exponential
space compression compared to classical O(n).

API Reference
-------------

.. autoclass:: pysparq.algorithms.cks_solver.ChebyshevPolynomialCoefficient
   :members:

.. autoclass:: pysparq.algorithms.cks_solver.SparseMatrix
   :members:

.. autoclass:: pysparq.algorithms.cks_solver.QuantumWalkNSteps
   :members:

.. autoclass:: pysparq.algorithms.cks_solver.LCUContainer
   :members:

.. autofunction:: pysparq.algorithms.cks_solver.cks_solve
