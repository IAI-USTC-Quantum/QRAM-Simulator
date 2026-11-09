"""Sparse logical QRAM words independent of host integer/register widths.

This module is the single authoritative source of the ideal sparse XOR
query semantics shared with SparQSim (installed there as
``pysparq/_qram_words.py`` from this repository's submodule) and OracQ.

The ideal XOR interface emits actual controlled native X operations on
caller-supplied bit coordinates. It is separate from the noisy
bucket-brigade circuit (``QRAMCircuitQubit``/``QRAMCircuitQutrit``, which
route through a physical tree and keep their ``size_t`` address packing)
and never claims to allocate or simulate a dense tree with
``2**address_width`` nodes.

Capacity notions kept apart by design:

- *wide logical words*: ``address_width``/``data_width`` may be any
  positive integer (65, 129, 256, ...); logical registers are carried by
  one or more native machine words and are never truncated to 64 bits;
- *sparse records*: only the non-zero ``cells`` exist as Python objects;
- *nominal address space*: ``2**address_width`` labels, a pure bound on
  address values — no entity, qubit or tree node is allocated for it;
- *allocatable entity capacity*: whatever the native backend actually
  allocates (machine words / qubits); querying this memory allocates
  nothing beyond the caller's own registers.

Native backend protocol (frozen with the SparQSim owner):

- bit coordinates are ``(word_register_name, bit_index)`` pairs — a
  logical register wider than one machine word simply spans several word
  registers;
- ``backend.X_Bool(word_register_name, bit_index)`` returns an operator
  object that can be applied to the state by calling it:
  ``operator(state)``;
- ``operator.conditioned_by_bit(bit_coordinate_list)`` turns the operator
  into an X controlled on every listed bit reading 1. Implementations may
  mutate the operator in place, return ``self`` or return a new
  conditioned operator; all three are supported here;
- conditions passed to :meth:`SparseWordMemory.apply_xor` are
  ``(little_endian_bits, expected_value)`` pairs: the query for a stored
  cell fires only on basis states where the listed bits hold
  ``expected_value`` (little-endian).
"""

from __future__ import annotations

from collections.abc import Mapping, Sequence

BitCoordinate = tuple[str, int]


class SparseWordMemory:
    """A zero-default sparse memory with arbitrary-width unsigned words.

    ``address_width`` and ``data_width`` are logical widths in bits; every
    stored value keeps its exact high bits in arbitrary-precision Python
    integers, so words are never packed into a single 64-bit machine word.
    Cells mapped to zero are dropped: the memory is zero by default.
    """

    def __init__(self, address_width: int, data_width: int, cells: Mapping[int, int]):
        if any(type(width) is not int or width < 1 for width in (address_width, data_width)):
            raise ValueError("memory widths must be positive integers")
        self.address_width = address_width
        self.data_width = data_width
        self.cells: dict[int, int] = {}
        for address, value in cells.items():
            self._check(address, address_width, "address")
            self._check(value, data_width, "data")
            if value:
                self.cells[address] = value

    @staticmethod
    def _check(value: int, width: int, label: str) -> None:
        if type(value) is not int or value < 0 or value.bit_length() > width:
            raise ValueError(f"{label} must fit its declared unsigned width")

    @property
    def nominal_address_space(self) -> int:
        """Number of addressable labels ``2**address_width`` (no allocation)."""
        return 1 << self.address_width

    @property
    def record_count(self) -> int:
        """Number of non-zero stored records (the only existing entities)."""
        return len(self.cells)

    def xor(self, address: int, data: int = 0) -> int:
        """Return ``data`` XOR the stored word at ``address`` (0 if absent)."""
        self._check(address, self.address_width, "address")
        self._check(data, self.data_width, "data")
        return data ^ self.cells.get(address, 0)

    def apply_xor(
        self,
        state,
        address_bits: Sequence[BitCoordinate],
        data_bits: Sequence[BitCoordinate],
        backend,
        *,
        conditions=(),
    ) -> None:
        """Apply the exact coherent query on native machine-word bit coordinates.

        ``address_bits[i]`` / ``data_bits[i]`` are the ``(word_register_name,
        bit_index)`` coordinates of logical bit ``i`` (little-endian) of the
        address / data registers; a logical register wider than one machine
        word spans several word registers. ``backend`` must be the caller's
        real native backend (see the module docstring protocol), not a mock.
        Conditions are ``(little_endian_bits, expected_value)`` pairs; a
        stored cell is queried only on basis states satisfying all of them.
        Shared bits between conditions and the address register are merged —
        a zero control coinciding with an address bit restricts the query to
        cells whose address agrees with it, and never re-addresses a cell.

        For each stored cell the emission is: toggle the zero-valued
        address/control bits to 1, apply one controlled native X per set
        data bit conditioned on all required bits, then restore the
        toggles. This is exact: basis states not matching the merged
        pattern are untouched by the transient toggles.
        """
        if len(address_bits) != self.address_width or len(data_bits) != self.data_width:
            raise ValueError("native bit views do not match logical memory widths")
        if len(set(address_bits)) != len(address_bits) or len(set(data_bits)) != len(data_bits):
            raise ValueError("native bit views must not alias internally")
        if set(address_bits) & set(data_bits):
            raise ValueError("address and data views must not overlap")
        conditions = tuple(conditions)
        for bits, value in conditions:
            if len(set(bits)) != len(bits):
                raise ValueError("condition bit views must not alias internally")
            self._check(value, len(bits), "condition")
            if set(bits) & set(data_bits):
                raise ValueError("conditions cannot overlap modified data bits")
        for address, word in self.cells.items():
            expected = {pair: (address >> bit) & 1 for bit, pair in enumerate(address_bits)}
            conflict = False
            for bits, value in conditions:
                for bit, pair in enumerate(bits):
                    required = (value >> bit) & 1
                    if pair in expected and expected[pair] != required:
                        conflict = True
                    expected[pair] = required
            if conflict:
                continue
            zeros = [pair for pair, value in expected.items() if not value]
            for pair in zeros:
                backend.X_Bool(*pair)(state)
            try:
                remaining = word
                while remaining:
                    low = remaining & -remaining
                    bit = low.bit_length() - 1
                    operator = backend.X_Bool(*data_bits[bit])
                    conditioned = operator.conditioned_by_bit(list(expected))
                    if conditioned is None:
                        conditioned = operator
                    conditioned(state)
                    remaining ^= low
            finally:
                for pair in reversed(zeros):
                    backend.X_Bool(*pair)(state)
