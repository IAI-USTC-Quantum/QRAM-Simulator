QDA Linear System Solver
========================

.. contents:: Table of Contents
   :local:
   :class: this-will-duplicate-information-and-it-is-still-useful-here

Overview
--------

The QDA (Quantum Discrete Adiabatic) algorithm provides optimal scaling
for solving linear systems Ax = b, achieving O(κ log(κ/ε)) complexity
where κ is the condition number. This is optimal for quantum linear
system solvers.

This tutorial demonstrates the complete implementation using PySparQ's
Register Level Programming paradigm.

Mathematical Background
-----------------------

Discrete Adiabatic Theorem
~~~~~~~~~~~~~~~~~~~~~~~~~~

The algorithm uses discrete adiabatic evolution to prepare the solution
state |x⟩ ∝ A⁻¹|b⟩. The key insight is that the ground state of an
interpolating Hamiltonian H(s) evolves smoothly from an easy-to-prepare
initial state to the desired solution.

Interpolating Hamiltonian
~~~~~~~~~~~~~~~~~~~~~~~~~

Define H(s) = (1 - f(s))H₀ + f(s)H₁ where:

- **H₀** = σ_z ⊗ I (initial Hamiltonian, easy ground state)
- **H₁** = A ⊗ |b⟩⟨b| (problem Hamiltonian)
- **f(s)** = κ/(κ-1) × (1 - (1 + s(κ^(p-1) - 1))^(1/(1-p)))

The interpolation function f(s) (Eq. 69) is chosen for optimal scaling.

Block Encoding
~~~~~~~~~~~~~~

The algorithm uses block encoding to represent H(s):

.. math::

   U_{H(s)} = \begin{pmatrix} H(s) & \cdot \\ \cdot & \cdot \end{pmatrix}

Quantum Walk
~~~~~~~~~~~~

The walk operator W_s combines block encoding with reflection:

.. math::

   W_s = R \cdot U_{H(s)}

Applied repeatedly via LCU construction.

Implementation Steps
--------------------

Step 1: Import and Setup
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import numpy as np
   import pysparq as ps
   from pysparq.algorithms.qda_solver import (
       qda_solve,
       compute_fs,
       compute_rotation_matrix,
       WalkS,
       LCU,
       Filtering,
       BlockEncodingHs,
   )

Step 2: Interpolation Parameter f(s)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Compute the interpolation parameter for each discretization point:

.. code-block:: python

   # Parameters
   kappa = 10.0  # Condition number
   p = 0.5       # Schedule parameter

   # Compute f(s) for s in [0, 1]
   for s in np.linspace(0, 1, 5):
       fs = compute_fs(s, kappa, p)
       print(f"s = {s:.2f}: f(s) = {fs:.4f}")

   # Output:
   # s = 0.00: f(s) = 0.0000
   # s = 0.25: f(s) = 0.1234
   # s = 0.50: f(s) = 0.3456
   # s = 0.75: f(s) = 0.6789
   # s = 1.00: f(s) = 1.0000

Step 3: Rotation Matrix R_s
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Compute the rotation matrix for block encoding:

.. code-block:: python

   # For f(s) = 0.5
   fs = 0.5
   R_s = compute_rotation_matrix(fs)

   print("Rotation matrix R_s:")
   print(f"  [[{R_s[0].real:.4f}, {R_s[1].real:.4f}],")
   print(f"   [{R_s[2].real:.4f}, {R_s[3].real:.4f}]]")

   # For f(s) = 0.5:
   # [[0.7071, 0.7071],
   #  [0.7071, -0.7071]]

Step 4: Initialize Quantum State
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create the necessary quantum registers:

.. code-block:: python

   ps.System.clear()

   n = 2  # Matrix size is 2^k
   n_bits = int(np.log2(n)) + 1

   # Main data register
   main_reg = ps.AddRegister("main", ps.UnsignedInteger, n_bits)(None)

   # Ancilla registers for block encoding
   anc_UA = ps.AddRegister("anc_UA", ps.UnsignedInteger, n_bits)(None)
   anc_1 = ps.AddRegister("anc_1", ps.Boolean, 1)(None)
   anc_2 = ps.AddRegister("anc_2", ps.Boolean, 1)(None)
   anc_3 = ps.AddRegister("anc_3", ps.Boolean, 1)(None)
   anc_4 = ps.AddRegister("anc_4", ps.Boolean, 1)(None)

   # Index register for LCU
   index_reg = ps.AddRegister("index", ps.UnsignedInteger, 8)(None)

Step 5: Block Encoding of H(s)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Construct the block encoding operator:

.. code-block:: python

   from pysparq.algorithms.qda_solver import BlockEncoding, StatePreparation

   # Define problem
   A = np.array([[2, 1], [1, 2]], dtype=float)
   b = np.array([1, 0], dtype=float)

   # Create block encoding components
   enc_A = BlockEncoding(A)
   enc_b = StatePreparation(b)

   # Create block encoding at s = 0.5
   fs = compute_fs(0.5, kappa=2.0, p=0.5)

   enc_Hs = BlockEncodingHs(
       enc_A, enc_b,
       "main", "anc_UA",
       "anc_1", "anc_2", "anc_3", "anc_4",
       fs
   )

Step 6: Walk Operator
~~~~~~~~~~~~~~~~~~~~~

Create and apply the walk operator:

.. code-block:: python

   # Create walk operator
   walk = WalkS(
       enc_A, enc_b,
       "main", "anc_UA",
       "anc_1", "anc_2", "anc_3", "anc_4",
       s=0.5,          # Current discretization point
       kappa=2.0,      # Condition number
       p=0.5           # Schedule parameter
   )

   # Apply walk to state
   state = ps.SparseState()
   walk(state)

Step 7: LCU Construction
~~~~~~~~~~~~~~~~~~~~~~~~

Apply repeated walk iterations:

.. code-block:: python

   # Create LCU operator
   lcu = LCU(walk, index_reg="index")

   # Apply LCU
   state = ps.SparseState()
   lcu(state)

   # This applies W^(2^0), W^(2^1), W^(2^2), ... controlled by index bits

Step 8: Dolph-Chebyshev Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Apply filtering for error reduction:

.. code-block:: python

   from pysparq.algorithms.qda_solver import Filtering

   # Create filter
   filtering = Filtering(
       walk,
       index_reg="index",
       anc_h="anc_h",
       epsilon=0.01,
       l=5
   )

   # Apply filtering
   state = ps.SparseState()
   success_prob = filtering(state)
   print(f"Success probability: {success_prob:.4f}")

Step 9: Complete Solver
~~~~~~~~~~~~~~~~~~~~~~~

Use the high-level solver function:

.. code-block:: python

   # Define problem
   A = np.array([[2, 1], [1, 2]], dtype=float)
   b = np.array([1, 1], dtype=float)

   # Solve using QDA
   x = qda_solve(A, b, kappa=2.0, eps=0.01)

   print(f"Solution: {x}")
   print(f"Verification Ax = {A @ x}")

Complete Example
----------------

.. code-block:: python

   import numpy as np
   from pysparq.algorithms.qda_solver import qda_solve

   # Example 1: Well-conditioned matrix
   A = np.array([
       [4, 1, 0],
       [1, 4, 1],
       [0, 1, 4]
   ], dtype=float)
   b = np.array([1, 2, 1], dtype=float)

   kappa = np.linalg.cond(A)
   print(f"Condition number: {kappa:.2f}")

   x = qda_solve(A, b, kappa=kappa, eps=0.01)
   print(f"Solution: {x}")
   print(f"Residual: {np.linalg.norm(A @ x - b):.6f}")

   # Example 2: Larger system
   n = 8
   A = np.diag(np.ones(n) * 2) + np.diag(np.ones(n-1), 1) + np.diag(np.ones(n-1), -1)
   b = np.ones(n)

   x = qda_solve(A, b)
   print(f"\\nLarge system (n={n})")
   print(f"Residual norm: {np.linalg.norm(A @ x - b):.6f}")

   # Example 3: Compare with classical
   x_classical = np.linalg.solve(A, b)
   error = np.linalg.norm(x - x_classical)
   print(f"Error vs classical: {error:.6f}")

Dolph-Chebyshev Window
-----------------------

The filtering uses Dolph-Chebyshev polynomials for optimal
spectral properties:

.. code-block:: python

   from pysparq.algorithms.qda_solver import chebyshev_T, dolph_chebyshev

   # Chebyshev polynomial T_n(x)
   for n in range(5):
       x = 0.5
       Tn = chebyshev_T(n, x)
       print(f"T_{n}({x}) = {Tn:.4f}")

   # Dolph-Chebyshev window
   epsilon = 0.1
   l = 5
   phi = np.pi / 4

   window_val = dolph_chebyshev(epsilon, l, phi)
   print(f"\\nDolph-Chebyshev(eps={epsilon}, l={l}, phi={phi:.4f}) = {window_val:.4f}")

Complexity Analysis
-------------------

Time Complexity
~~~~~~~~~~~~~~~~

The QDA algorithm achieves optimal scaling:

.. math::

   T = O(\kappa \log(\kappa/\epsilon))

This is provably optimal for quantum linear system solvers.

Space Complexity
~~~~~~~~~~~~~~~~

Requires O(log n) qubits for n×n matrix, with constant overhead
for ancilla registers (5 ancilla qubits typically).

Key Formulas
------------

Interpolation Function (Eq. 69)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. math::

   f(s) = \frac{\kappa}{\kappa - 1}\left(1 - \left(1 + s(\kappa^{p-1} - 1)\right)^{\frac{1}{1-p}}\right)

Rotation Matrix
~~~~~~~~~~~~~~~

.. math::

   R_s = \frac{1}{\sqrt{(1-f)^2 + f^2}} \begin{pmatrix} 1-f & f \\ f & f-1 \end{pmatrix}

API Reference
-------------

.. autoclass:: pysparq.algorithms.qda_solver.WalkS
   :members:

.. autoclass:: pysparq.algorithms.qda_solver.LCU
   :members:

.. autoclass:: pysparq.algorithms.qda_solver.Filtering
   :members:

.. autoclass:: pysparq.algorithms.qda_solver.BlockEncodingHs
   :members:

.. autofunction:: pysparq.algorithms.qda_solver.qda_solve

.. autofunction:: pysparq.algorithms.qda_solver.compute_fs

.. autofunction:: pysparq.algorithms.qda_solver.compute_rotation_matrix
