"""Logical address/data words must not inherit a uint64 packing boundary.

Classical coverage: exact wide words, high-bit read/write, XOR involution,
capacity notions and invalid-value rejection.

Quantum-chain coverage (:class:`ReferenceWordBackend`): ``apply_xor``
emits real native X / controlled-X operations on ``(word_register_name,
bit_index)`` coordinates and the resulting exact statevector matches an
independent classical oracle for every occupied basis state, including
superpositions with phases, entanglement between address and data,
shared zero controls that coincide with address bits, external zero/one
controls and double application (query involution).

The shared cross-repository fixture ``fixtures/word_query_fixture.json``
(word-query-fixture.v1) is consumed by QRAM-Simulator and SparQSim alike,
so the same source data verifies the query chain against two independent
native backends.
"""

from __future__ import annotations

import json
import math
from pathlib import Path

import pytest

from qram_simulator.word_backend import (
    MACHINE_WORD_BITS,
    InplaceWordBackend,
    ReferenceWordBackend,
    encode_basis,
    word_layout,
)
from qram_simulator.word_memory import SparseWordMemory

MACHINE_WORD = MACHINE_WORD_BITS


# ---------------------------------------------------------------------------
# Independent classical oracle (deliberately separate from the chain)
# ---------------------------------------------------------------------------


def classical_query(memory: SparseWordMemory, address, data, control, conditions,
                    address_bits, control_bits) -> int | None:
    """Independent oracle: output data word, or None when gated off."""
    values = {pair: (address >> bit) & 1 for bit, pair in enumerate(address_bits)}
    values.update({pair: (control >> bit) & 1 for bit, pair in enumerate(control_bits)})
    for bits, value in conditions:
        for bit, pair in enumerate(bits):
            if values[pair] != (value >> bit) & 1:
                return None
    return data ^ memory.cells.get(address, 0)


def run_chain(memory, backend, address_bits, data_bits, control_bits,
              basis_fields, amplitudes, conditions=()):
    """Execute apply_xor on a real state and derive the oracle expectation."""
    backend.register(address_bits, data_bits, control_bits)
    state = backend.make_state()
    for (address, data, control), amplitude in zip(basis_fields, amplitudes):
        basis = encode_basis(encode_basis(encode_basis(0, address_bits, address, backend),
                              data_bits, data, backend),
                       control_bits, control, backend)
        state.amps[basis] = state.amps.get(basis, 0j) + amplitude
    memory.apply_xor(state, address_bits, data_bits, backend, conditions=conditions)
    expected: dict[int, complex] = {}
    for (address, data, control), amplitude in zip(basis_fields, amplitudes):
        out = classical_query(memory, address, data, control, conditions,
                              address_bits, control_bits)
        out_data = data if out is None else out
        basis = encode_basis(encode_basis(encode_basis(0, address_bits, address, backend),
                              data_bits, out_data, backend),
                       control_bits, control, backend)
        expected[basis] = expected.get(basis, 0j) + amplitude
    return state, expected


def address_purity(state, address_bits, backend) -> float:
    """Purity of the reduced address state (1 for product, <1 entangled)."""
    address_mask = sum(1 << backend.position(pair) for pair in address_bits)
    reduced: dict[tuple[int, int], complex] = {}
    for basis, amplitude in state.amps.items():
        key = (basis & address_mask, basis & ~address_mask)
        reduced[key] = reduced.get(key, 0j) + amplitude
    addresses = {addr for addr, _ in reduced}
    environments = {env for _, env in reduced}
    purity = 0.0
    for a in addresses:
        for b in addresses:
            rho_ab = sum(reduced.get((a, env), 0j) * reduced.get((b, env), 0j).conjugate()
                         for env in environments)
            purity += abs(rho_ab) ** 2
    return purity


# ---------------------------------------------------------------------------
# Classical wide-word semantics
# ---------------------------------------------------------------------------


def test_sparse_high_words_and_xor_involution():
    memory = SparseWordMemory(129, 193, {1 << 128: (1 << 192)+7, 0: 1})
    assert len(memory.cells) == 2
    data = (1 << 130)+9
    output = memory.xor(1 << 128, data)
    assert output == data ^ ((1 << 192)+7)
    assert memory.xor(1 << 128, output) == data
    assert memory.xor((1 << 128)+1, data) == data


@pytest.mark.parametrize("address,value",
                   [(True, 1), (-1, 1), (1 << 129, 1), (0, True), (0, -1), (0, 1 << 193)])
def test_invalid_values_do_not_truncate(address, value):
    with pytest.raises(ValueError):
        SparseWordMemory(129, 193, {address: value})


@pytest.mark.parametrize("width", [64, 65, 127, 128, 129, 256])
def test_wide_high_bits_and_involution(width):
    high_address = (1 << (width - 1)) | 0b101
    high_word = (1 << (width - 1)) | (1 << (width // 2)) | 3
    memory = SparseWordMemory(width, width, {high_address: high_word, 0: 0, 1: 0})
    assert memory.record_count == 1, "zero cells must not be materialized"
    assert memory.nominal_address_space == 1 << width
    data = (1 << (width - 2)) + 7
    assert memory.xor(high_address, data) == data ^ high_word
    assert memory.xor(high_address, memory.xor(high_address, data)) == data
    assert memory.xor(0, data) == data, "default cell reads exact zero"


def test_xor_argument_validation():
    memory = SparseWordMemory(8, 8, {1: 2})
    with pytest.raises(ValueError):
        memory.xor(1 << 8)
    with pytest.raises(ValueError):
        memory.xor(0, 1 << 8)
    with pytest.raises(ValueError):
        memory.xor(-1)
    with pytest.raises(ValueError):
        memory.xor(True)  # bool is not an accepted integer


def test_constructor_width_validation():
    for bad in (0, -1, True, 2.0):
        with pytest.raises(ValueError):
            SparseWordMemory(bad, 8, {})
        with pytest.raises(ValueError):
            SparseWordMemory(8, bad, {})


def test_narrow_native_boundary_is_explicit_not_silent():
    """The noisy bucket-brigade path rejects >64 combined bits instead of truncating.

    That native circuit packs address+bus into one size_t branch index; its
    boundary is an explicit exception. The ideal sparse XOR path in this
    module is the supported route beyond that boundary.
    """
    qram_simulator = pytest.importorskip("qram_simulator")
    narrow = qram_simulator.QRAMCircuitQubit(4, 61)  # 65 combined bits
    with pytest.raises((ValueError, RuntimeError)):
        narrow.set_input_uniform(4)
    with pytest.raises((TypeError, OverflowError)):
        qram_simulator.QRAMCircuitQubit(4, 2).set_input_addresses([1 << 70])
    wide = SparseWordMemory(129, 129, {(1 << 128) + 5: (1 << 128) + 9})
    assert wide.xor((1 << 128) + 5) == (1 << 128) + 9


# ---------------------------------------------------------------------------
# Quantum chain on the real reference native backend
# ---------------------------------------------------------------------------

CHAIN_WIDTHS = [64, 65, 127, 128, 129, 256]


@pytest.mark.parametrize("address_width", CHAIN_WIDTHS)
def test_chain_superposition_matches_classical_oracle(address_width):
    data_width = min(address_width + 1, 257)
    addresses = [(1 << (address_width - 1)) | 0b11, 0b10, (1 << (address_width - 2)) | 1]
    memory = SparseWordMemory(address_width, data_width, {
        addresses[0]: (1 << (data_width - 1)) | 5,
        addresses[2]: (1 << (data_width - 1)) | (1 << (data_width - 2)),
    })
    backend = ReferenceWordBackend()
    address_bits = word_layout(address_width, "a")
    data_bits = word_layout(data_width, "d")
    quarter = 0.5
    amplitudes = [quarter, quarter * 1j, -quarter]
    fields = [(address, 0, 0) for address in addresses]
    state, expected = run_chain(memory, backend, address_bits, data_bits, [],
                                fields, amplitudes)
    assert state.amps == expected


def test_chain_entangles_address_and_data_and_keeps_phase():
    memory = SparseWordMemory(4, 3, {0b1011: 0b111})
    backend = ReferenceWordBackend()
    address_bits = word_layout(4, "a")
    data_bits = word_layout(3, "d")
    phase = complex(3, 4) / 5
    fields = [(0b1011, 0, 0), (0b0100, 0, 0)]
    state, expected = run_chain(memory, backend, address_bits, data_bits, [],
                                fields, [phase / math.sqrt(2), phase.conjugate() / math.sqrt(2)])
    assert state.amps == expected
    # the queried branch keeps its relative phase and the branches entangle:
    # the reduced address state has purity 1/2, not the 1 of a product state
    assert address_purity(state, address_bits, backend) == pytest.approx(0.5)


def test_chain_shared_zero_address_control():
    memory = SparseWordMemory(5, 4, {0b0101: 0b1001, 0b0001: 0b0110})
    backend = ReferenceWordBackend()
    address_bits = word_layout(5, "a")
    data_bits = word_layout(4, "d")
    # a zero control placed on address bit 2: the cell whose address holds a
    # 1 there (0b0101) is gated off, the cell agreeing with 0 (0b0001) and
    # every untouched address keep their exact data — no wrong-address query
    zero_control = ([address_bits[2]], 0)
    fields = [(0b0101, 0b0011, 0), (0b0001, 0b0011, 0), (0b0011, 0b0011, 0)]
    thirds = [1 / 3, 1 / 3, 1 / 3]
    state, expected = run_chain(memory, backend, address_bits, data_bits, [],
                                fields, thirds, conditions=(zero_control,))
    assert state.amps == expected
    one_control = ([address_bits[2]], 1)  # now 0b0001 contradicts and 0b0101 agrees
    state2, expected2 = run_chain(memory, backend, address_bits, data_bits, [],
                                  fields, thirds, conditions=(one_control,))
    assert state2.amps == expected2
    assert state2.amps != state.amps, "the two gate polarities address different cells"


def test_chain_external_controls_zero_and_one():
    memory = SparseWordMemory(3, 3, {0b101: 0b110})
    backend = ReferenceWordBackend()
    address_bits = word_layout(3, "a")
    data_bits = word_layout(3, "d")
    control_bits = word_layout(2, "c")
    # little-endian condition value 0b01 over (c1, a1): c1 must read 1 and
    # address bit 1 must read 0 — mixing an external one-control and a
    # shared zero address control in a single condition
    conditions = (([control_bits[1], address_bits[1]], 0b01),)
    fields = [(0b101, 0b010, 0b10), (0b101, 0b010, 0b01), (0b111, 0b010, 0b10)]
    amplitudes = [0.5, 0.5, 1 / math.sqrt(2)]
    state, expected = run_chain(memory, backend, address_bits, data_bits, control_bits,
                                fields, amplitudes, conditions=conditions)
    assert state.amps == expected


@pytest.mark.parametrize("address_width", CHAIN_WIDTHS)
def test_chain_double_application_is_identity(address_width):
    memory = SparseWordMemory(address_width, address_width,
                              {(1 << (address_width - 1)) | 2: (1 << (address_width - 1)) | 1})
    backend = ReferenceWordBackend()
    address_bits = word_layout(address_width, "a")
    data_bits = word_layout(address_width, "d")
    backend.register(address_bits, data_bits)
    state = backend.make_state()
    address = (1 << (address_width - 1)) | 2
    basis = encode_basis(encode_basis(0, address_bits, address, backend), data_bits, 0b101, backend)
    state.amps[basis] = 1.0
    memory.apply_xor(state, address_bits, data_bits, backend)
    assert state.amps != {basis: 1.0}
    memory.apply_xor(state, address_bits, data_bits, backend)
    assert state.amps == {basis: 1.0}


def test_chain_inplace_conditioned_protocol_variant():
    memory = SparseWordMemory(3, 2, {0b110: 0b11})
    backend = InplaceWordBackend()
    address_bits = word_layout(3, "a")
    data_bits = word_layout(2, "d")
    fields = [(0b110, 0b01, 0), (0b001, 0b01, 0)]
    state, expected = run_chain(memory, backend, address_bits, data_bits, [],
                                fields, [1 / math.sqrt(2), 1 / math.sqrt(2)])
    assert state.amps == expected


def test_chain_input_validation():
    memory = SparseWordMemory(3, 2, {0b110: 0b11})
    backend = ReferenceWordBackend()
    address_bits = word_layout(3, "a")
    data_bits = word_layout(2, "d")
    backend.register(address_bits, data_bits)
    state = backend.make_state()
    with pytest.raises(ValueError):
        memory.apply_xor(state, address_bits[:2], data_bits, backend)
    with pytest.raises(ValueError):
        memory.apply_xor(state, address_bits, data_bits[:1], backend)
    with pytest.raises(ValueError):
        memory.apply_xor(state, address_bits + [address_bits[0]], data_bits, backend)
    aliased = [address_bits[0], address_bits[0], address_bits[2]]
    with pytest.raises(ValueError):
        memory.apply_xor(state, aliased, data_bits, backend)
    with pytest.raises(ValueError):
        memory.apply_xor(state, data_bits + address_bits[:1], address_bits, backend)
    with pytest.raises(ValueError):
        memory.apply_xor(state, address_bits, data_bits, backend,
                         conditions=(([data_bits[0]], 1),))
    with pytest.raises(ValueError):
        memory.apply_xor(state, address_bits, data_bits, backend,
                         conditions=(([address_bits[0], address_bits[0]], 1),))
    with pytest.raises(ValueError):
        memory.apply_xor(state, address_bits, data_bits, backend,
                         conditions=(([address_bits[0]], 2),))


def test_chain_high_data_bits_span_machine_words():
    memory = SparseWordMemory(70, 130, {(1 << 69): (1 << 129) | (1 << 64) | 1})
    backend = ReferenceWordBackend()
    address_bits = word_layout(70, "a")
    data_bits = word_layout(130, "d")
    fields = [((1 << 69), (1 << 65), 0), (0, 0, 0)]
    state, expected = run_chain(memory, backend, address_bits, data_bits, [],
                                fields, [1 / math.sqrt(2), 1 / math.sqrt(2)])
    assert state.amps == expected
    data_positions = [backend.position(pair) for pair in data_bits]
    assert min(data_positions[64:]) >= MACHINE_WORD, \
        "the data register must genuinely span several machine words"


# ---------------------------------------------------------------------------
# Shared cross-repository fixture (word-query-fixture.v1)
# ---------------------------------------------------------------------------

FIXTURE = Path(__file__).parent / "fixtures" / "word_query_fixture.json"


def test_shared_fixture_matches_this_backend():
    payload = json.loads(FIXTURE.read_text())
    assert payload["schema"] == "qram.word-query-fixture.v1"
    memory = SparseWordMemory(payload["address_width"], payload["data_width"],
                              {int(k): int(v) for k, v in payload["cells"].items()})
    backend = ReferenceWordBackend()
    address_bits = [tuple(pair) for pair in payload["address_bits"]]
    data_bits = [tuple(pair) for pair in payload["data_bits"]]
    control_bits = [tuple(pair) for pair in payload["control_bits"]]
    conditions = tuple(([tuple(pair) for pair in cond["bits"]], int(cond["value"]))
                       for cond in payload["conditions"])
    fields = [(int(row["address"]), int(row["data"]), int(row["control"]))
              for row in payload["input"]]
    amplitudes = [complex(row["amplitude"][0], row["amplitude"][1])
                  for row in payload["input"]]
    state, expected = run_chain(memory, backend, address_bits, data_bits, control_bits,
                                fields, amplitudes, conditions)
    recorded: dict[int, complex] = {}
    for row in payload["expected_output"]:
        basis = encode_basis(0, address_bits, int(row["address"]), backend)
        basis = encode_basis(basis, data_bits, int(row["data"]), backend)
        basis = encode_basis(basis, control_bits, int(row["control"]), backend)
        recorded[basis] = recorded.get(basis, 0j) + complex(row["amplitude"][0],
                                                            row["amplitude"][1])
    assert state.amps == recorded == expected
