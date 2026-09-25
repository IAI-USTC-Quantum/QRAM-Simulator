"""Circuit-level QRAM baseline simulator (qubit architecture, 1-2 tree layers).

This subpackage provides an independent, gate-by-gate re-implementation of the
qubit-architecture QRAM loading circuit on the UnifiedQuantum (``uniqc``) QuTiP
density-matrix backend, used as a semi-quantitative cross-check baseline for the
symbolic sparse-tree trajectory engine exported by :class:`qram_simulator.QRAMCircuitQubit`.

The noise model mirrors the TimeStep scheduler of the C++ core: channels are
placed only on the QRAM routing-tree qubits (never on the address register or
the data bus), on every active position ``[0, 2*(2^l - 1))`` of each time slice
whose entangled-layer count is ``l``, with marginal probability ``p`` per
position per step (the channel-level counterpart of the C++ "nerror ~
Binomial(N, p), uniform positions" trajectory sampling).

See ``compare`` for the packaged comparison experiment and the module docstring
of ``circuit_qram`` for the encoding and operator conventions.
"""

from .circuit_qram import CircuitQRAMQubit
from .metrics import classical_fidelity, tvd

__all__ = [
    "CircuitQRAMQubit",
    "classical_fidelity",
    "tvd",
]
