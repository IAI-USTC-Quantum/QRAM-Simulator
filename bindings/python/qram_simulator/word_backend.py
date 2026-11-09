"""Reference native backend for the ideal sparse wide-word query chain.

Executes the frozen ``SparseWordMemory.apply_xor`` operator protocol (see
:mod:`qram_simulator.word_memory`) on an exact dict-backed statevector
whose basis indices are arbitrary-precision Python ints. Registers are
named machine words of ``MACHINE_WORD_BITS`` bits, so logical registers
wider than 64 bits genuinely span several words — the same layout
discipline SparQSim's production native backend follows.

This module is QRAM-Simulator's own verification and product-demo
executor. Production-scale native execution is provided by the caller's
backend (SparQSim); both are validated against the shared fixture
``bindings/python/test/fixtures/word_query_fixture.json``.
"""

from __future__ import annotations

MACHINE_WORD_BITS = 64


class SparseStateVector:
    """Exact amplitudes over arbitrary-width basis indices (dict-backed)."""

    def __init__(self):
        self.amps: dict[int, complex] = {}

    def amplitude(self, basis: int) -> complex:
        return self.amps.get(basis, 0j)


class WordXOperator:
    """A native X on one machine-word bit, optionally controlled on |1>.

    Follows the frozen protocol: ``conditioned_by_bit`` accepts the bit
    coordinate list, mutates the operator and returns ``self``.
    """

    def __init__(self, backend: "ReferenceWordBackend", position: int):
        self._backend = backend
        self._position = position
        self._control_mask = 0

    def conditioned_by_bit(self, pairs):
        for pair in pairs:
            self._control_mask |= 1 << self._backend.position(pair)
        return self

    def __call__(self, state: SparseStateVector) -> None:
        target = 1 << self._position
        moved: dict[int, complex] = {}
        for basis, amplitude in state.amps.items():
            if basis & self._control_mask == self._control_mask:
                index = basis ^ target
                moved[index] = moved.get(index, 0j) + amplitude
            else:
                moved[basis] = moved.get(basis, 0j) + amplitude
        state.amps = moved


class ReferenceWordBackend:
    """Named machine-word register file with exact X / controlled-X execution.

    Every operation is an exact unitary permutation of the amplitudes —
    a genuine statevector execution, not a call recorder. Bit indices are
    confined to one machine word; wider logical registers span several
    word registers (see :func:`word_layout`).
    """

    def __init__(self, machine_word_bits: int = MACHINE_WORD_BITS):
        if type(machine_word_bits) is not int or machine_word_bits < 1:
            raise ValueError("machine_word_bits must be a positive integer")
        self.machine_word_bits = machine_word_bits
        self._word_slots: dict[str, int] = {}
        self._positions: dict[tuple[str, int], int] = {}

    def word_slot(self, name: str) -> int:
        if name not in self._word_slots:
            self._word_slots[name] = len(self._word_slots)
        return self._word_slots[name]

    def position(self, pair: tuple[str, int]) -> int:
        name, bit_index = pair
        if type(bit_index) is not int or not 0 <= bit_index < self.machine_word_bits:
            raise ValueError(
                f"bit index outside one {self.machine_word_bits}-bit machine word: {pair}")
        if pair not in self._positions:
            self._positions[pair] = self.word_slot(name) * self.machine_word_bits + bit_index
        return self._positions[pair]

    def register(self, *views) -> None:
        """Touch every coordinate so later state creation sees the layout."""
        for view in views:
            for pair in view:
                self.position(pair)

    def make_state(self) -> SparseStateVector:
        if not self._positions:
            raise ValueError("register the bit views before creating states")
        return SparseStateVector()

    def X_Bool(self, name: str, bit_index: int) -> WordXOperator:
        return WordXOperator(self, self.position((name, bit_index)))


class InplaceWordBackend(ReferenceWordBackend):
    """Protocol variant whose ``conditioned_by_bit`` returns None (pure mutation)."""

    class _InplaceX(WordXOperator):
        def conditioned_by_bit(self, pairs):
            WordXOperator.conditioned_by_bit(self, pairs)
            return None

    def X_Bool(self, name, bit_index):
        return self._InplaceX(self, self.position((name, bit_index)))


def word_layout(width: int, prefix: str,
                machine_word_bits: int = MACHINE_WORD_BITS) -> list[tuple[str, int]]:
    """Little-endian coordinates of one logical register over machine words."""
    if type(width) is not int or width < 1:
        raise ValueError("width must be a positive integer")
    return [(f"{prefix}w{n}", bit) for n in range((width - 1) // machine_word_bits + 1)
            for bit in range(machine_word_bits) if n * machine_word_bits + bit < width]


def encode_basis(basis: int, pairs, value: int, backend: ReferenceWordBackend) -> int:
    """OR the little-endian ``value`` into ``basis`` at the given coordinates."""
    for bit, pair in enumerate(pairs):
        if (value >> bit) & 1:
            basis |= 1 << backend.position(pair)
    return basis
