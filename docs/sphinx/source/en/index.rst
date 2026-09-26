QRAM-Simulator Documentation
============================

.. rubric:: The core QRAM circuit simulator — noisy sparse-branch simulation for both the qubit and qutrit architectures

QRAM-Simulator is a high-performance C++ simulation core for QRAM (quantum random access
memory) loading circuits:

- **Two architectures**: the qubit architecture (physical-qubit tree + good/bad branch
  pruning) and the qutrit architecture (three-level node tree), sharing a single timing
  and noise scheduler, ``TimeStep``;
- **Noise models**: bit flip / phase flip / combined flip / depolarizing / amplitude
  damping, which can be injected per time slice in arbitrary combinations;
- **Fidelity evaluation**: loading fidelity estimated from output sampling, with support
  for cross-checking pruned runs against unpruned ground-truth baselines;
- **Python bindings**: ``pip install qram-simulator`` gives you all the core working
  classes (QRAMCircuitQubit / QRAMCircuitQutrit / QRAMFullAmp / TimeStep).

.. toctree::
   :maxdepth: 2
   :caption: User Guide

   guide/installation
   guide/quickstart
   guide/architecture
   guide/operators
   guide/naming_conventions

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   api/index

.. toctree::
   :maxdepth: 2
   :caption: Papers and Experiments

   paper/README
   paper/reproduction
   paper/exactness_validation
   paper/qubit_qram_pruning
   paper/qubit_error_propagation

Index
-----

* :ref:`genindex`
* :ref:`search`
