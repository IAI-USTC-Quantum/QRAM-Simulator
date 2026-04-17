Shor's Quantum Factorization Algorithm
=======================================

.. contents:: Table of Contents
   :local:

Overview
--------

Shor's algorithm provides exponential speedup for integer factorization,
breaking RSA encryption theoretically. Given a composite number N, it
finds factors in polynomial time using quantum period finding.

This tutorial demonstrates two implementations using PySparQ:

1. **Semi-classical Shor**: Practical iterative measurement approach
2. **Full quantum Shor**: Textbook quantum phase estimation

Mathematical Background
-----------------------

The Algorithm
~~~~~~~~~~~~~

Shor's algorithm factors N by:

1. **Classical pre-processing**: Pick random a coprime to N
2. **Quantum period finding**: Find r where a^r ≡ 1 (mod N)
3. **Classical post-processing**: Compute gcd(a^(r/2) ± 1, N)

Period Finding
~~~~~~~~~~~~~~

The quantum part finds the period r of the function:

.. math::

   f(x) = a^x \\mod N

Using quantum phase estimation on unitary:

.. math::

   U|x\\rangle = |x \\oplus 1\\rangle

The eigenvalues of U are e^(2πik/r), giving the period.

Continued Fractions
~~~~~~~~~~~~~~~~~~~

The measurement result y/Q is related to c/r via:

.. math::

   \\frac{y}{Q} \\approx \\frac{c}{r}

where Q = 2^size (the precision register size).

We extract r using continued fraction expansion (Farey sequence).

Implementation Steps
--------------------

Step 1: Import and Setup
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import pysparq as ps
   from pysparq.algorithms.shor import factor, SemiClassicalShor, general_expmod

   import math
   import random

Step 2: Classical Helper Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The modular exponentiation function:

.. code-block:: python

   def general_expmod(a: int, x: int, N: int) -> int:
       """Compute a^x mod N using square-and-multiply."""
       if x == 0:
           return 1
       if x == 1:
           return a % N
       if x & 1:  # odd
           return (general_expmod(a, x - 1, N) * a) % N
       else:  # even
           half = general_expmod(a, x // 2, N)
           return (half * half) % N

   # Example usage
   result = general_expmod(7, 5, 15)  # 7^5 mod 15 = 7
   print(f"7^5 mod 15 = {result}")

Step 3: Continued Fraction Expansion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Extract period from measurement:

.. code-block:: python

   from fractions import Fraction

   def find_best_fraction(y: int, Q: int, N: int):
       """Find best fraction c/r ≈ y/Q using Farey sequence."""
       target = y / Q

       # Binary search through Farey sequence
       low_num, low_den = 0, 1
       high_num, high_den = 1, 1
       best_num, best_den = 0, 1

       while low_den + high_den <= N:
           mediant_num = low_num + high_num
           mediant_den = low_den + high_den

           if mediant_num / mediant_den < target:
               low_num, low_den = mediant_num, mediant_den
           else:
               high_num, high_den = mediant_num, mediant_den

           # Track best approximation
           if abs(mediant_num/mediant_den - target) < abs(best_num/best_den - target):
               best_num, best_den = mediant_num, mediant_den

       return best_den, best_num  # (r, c)

Step 4: Semi-classical Shor Circuit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Build the iterative measurement circuit:

.. code-block:: python

   class SemiClassicalShor:
       def __init__(self, a: int, N: int):
           if math.gcd(a, N) != 1:
               raise ValueError("a and N must be coprime")

           self.a = a
           self.N = N
           self.n = int(math.log2(N)) + 1
           self.size = self.n * 2  # Precision register size

       def run(self):
           ps.System.clear()
           state = ps.SparseState()

           # Create ancilla initialized to |1>
           anc_reg = ps.AddRegister("anc_reg", ps.UnsignedInteger, self.n)(state)
           ps.Xgate_Bool("anc_reg", 0)(state)

           results = []

           # Iterative phase estimation
           for x in range(self.size):
               # Work qubit in superposition
               work_reg = ps.AddRegisterWithHadamard(
                   "work_reg", ps.UnsignedInteger, 1
               )(state)

               # Controlled modular multiplication
               opnum = general_expmod(self.a, 2**(self.size - 1 - x), self.N)

               modmul = ps.CustomArithmetic(["anc_reg"], 64, 64,
                   lambda v: (v * opnum) % self.N
               )
               modmul.conditioned_by_all_ones("work_reg")(state)

               # Phase corrections from previous measurements
               for i, result in enumerate(reversed(results)):
                   if result == 1:
                       phase = -2 * math.pi / (2 ** (i + 2))
                       ps.Phase_Bool("work_reg", phase)(state)

               # Measure work qubit
               ps.Hadamard_Bool("work_reg")(state)
               measured, _ = ps.PartialTrace(["work_reg"])(state)
               results.append(measured[0])

               ps.RemoveRegister("work_reg")(state)

           # Convert bit results to integer
           meas_result = sum(bit * (2**i) for i, bit in enumerate(results))

           return meas_result, results

Step 5: Classical Post-processing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Extract factors from measurement:

.. code-block:: python

   def shor_postprocess(meas: int, size: int, a: int, N: int):
       """Extract factors from measurement result."""
       Q = 2 ** size
       r, c = find_best_fraction(meas, Q, N)

       # Validate period
       if r > N:
           raise ValueError("Period too large")
       if r % 2 == 1:
           raise ValueError("Odd period")

       # Check a^(r/2) ≠ -1 mod N
       a_r_half = general_expmod(a, r // 2, N)
       if a_r_half == N - 1:
           raise ValueError("a^(r/2) = -1 mod N")

       # Compute factors
       p = math.gcd(a_r_half + 1, N)
       q = math.gcd(a_r_half - 1, N)

       return p, q

Step 6: Complete Factorization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   def factor(N: int, a: int = None):
       """Factor N using Shor's algorithm."""
       if N % 2 == 0:
           return (2, N // 2)

       if a is None:
           a = random.randint(2, N - 1)

       # Check if already a factor
       g = math.gcd(a, N)
       if g != 1:
           return (g, N // g)

       shor = SemiClassicalShor(a, N)
       p, q = shor.run()

       return p, q

Complete Example
----------------

.. code-block:: python

   from pysparq.algorithms.shor import factor

   # Factor 15
   p, q = factor(15)
   print(f"Factors of 15: {p} and {q}")
   print(f"Verification: {p} × {q} = {p * q}")

   # Output:
   # Factoring N = 15 with a = 2
   # Found period: 4, c: 1
   # a^(r/2) mod N = 4
   # p = 5, q = 3, p * q = 15
   # Factors of 15: 5 and 3
   # Verification: 5 × 3 = 15

Multiple Attempts
~~~~~~~~~~~~~~~~~

.. code-block:: python

   import random

   N = 21

   # May need multiple attempts
   for attempt in range(5):
       a = random.randint(2, N - 1)
       if math.gcd(a, N) == 1:
           try:
               p, q = factor(N, a)
               if p != 1 and q != 1 and p * q == N:
                   print(f"Success! {N} = {p} × {q}")
                   break
           except ValueError as e:
               print(f"Attempt {attempt+1} failed: {e}")

Understanding the Quantum Circuit
----------------------------------

Register Layout
~~~~~~~~~~~~~~~

For factoring N:

- **Ancilla register** (n = ceil(log2(N)) bits): Holds y → y * a^(2^x) mod N
- **Work register** (1 bit): Single iterative measurement qubit

The circuit uses 2n iterations of controlled modular multiplication.

Circuit Diagram
~~~~~~~~~~~~~~~

For each iteration x in [0, 2n):

::

   work: |0>--H--[ctrl]--H--measure--> b_x
                     |
   anc:  |1>--[ModMul(a, 2^x, N)]-->

The ModMul operation computes:

.. math::

   |y\\rangle \\rightarrow |y \\cdot a^{2^x} \\mod N\\rangle

controlled by the work qubit.

Phase Corrections
~~~~~~~~~~~~~~~~~

After measuring bit x, we apply phase corrections for bits x+1, x+2, ...:

.. math::

   R_z\\left(-\\frac{2\\pi b_j}{2^{j-i}}\\right)

This implements the iterative phase estimation correctly.

API Reference
-------------

.. autoclass:: pysparq.algorithms.shor.SemiClassicalShor
   :members:

.. autoclass:: pysparq.algorithms.shor.ModMul
   :members:

.. autoclass:: pysparq.algorithms.shor.Shor
   :members:

.. autofunction:: pysparq.algorithms.shor.factor

.. autofunction:: pysparq.algorithms.shor.general_expmod

.. autofunction:: pysparq.algorithms.shor.find_best_fraction
