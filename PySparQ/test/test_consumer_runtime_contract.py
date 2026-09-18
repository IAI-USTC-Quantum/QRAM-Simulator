"""Independent mathematical models for the frozen QECC.Lang/QFVM slice."""

from __future__ import annotations

import itertools
import json
import math
from pathlib import Path

import pysparq as ps
import pytest


def test_frozen_inventory_has_unique_classified_operations():
    inventory_path = Path(__file__).parents[1] / "consumer_runtime_inventory.json"
    inventory = json.loads(inventory_path.read_text())
    operations = inventory["accepted_reversible_operations"]
    symbols = [item["symbol"] for item in operations]
    assert len(symbols) == len(set(symbols))
    assert {item["contract"] for item in operations} == {
        "xor_into",
        "inplace_bijective",
    }
    assert "compile_operator" not in symbols
    assert inventory["compile_operator_boundary"]["qecc_lang_src_matches"] == 0
    assert inventory["compile_operator_boundary"]["supported_qfvm_scope_matches"] == 0


def _declare(specs):
    ps.System.clear()
    for name, storage, width in specs:
        ps.System.add_register(name, storage, width)


def _basis(values):
    state = ps.SparseState()
    for name, value in values.items():
        ps.Init_Unsafe(name, value)(state)
    return state


def _snapshot(state, names):
    result = {}
    for branch in state.basis_states:
        key = tuple(branch.get(ps.System.get_id(name)).value for name in names)
        result[key] = result.get(key, 0j) + branch.amplitude
    return {key: amp for key, amp in result.items() if abs(amp) > 1e-12}


def _assert_state(actual, expected):
    assert actual.keys() == expected.keys()
    for key, amplitude in expected.items():
        assert abs(actual[key] - amplitude) < 1e-10


@pytest.mark.parametrize(
    ("specs", "names", "make_op", "model"),
    [
        (
            [("a", ps.UnsignedInteger, 2), ("b", ps.UnsignedInteger, 2), ("out", ps.UnsignedInteger, 2)],
            ("a", "b", "out"),
            lambda: ps.Add_UInt_UInt("a", "b", "out"),
            lambda a, b, out: (a, b, out ^ ((a + b) & 3)),
        ),
        (
            [("src", ps.UnsignedInteger, 2), ("dst", ps.UnsignedInteger, 2)],
            ("src", "dst"),
            lambda: ps.Assign("src", "dst"),
            lambda src, dst: (src, dst ^ src),
        ),
        (
            [
                ("left", ps.UnsignedInteger, 2),
                ("right", ps.UnsignedInteger, 2),
                ("less", ps.Boolean, 1),
                ("equal", ps.Boolean, 1),
            ],
            ("left", "right", "less", "equal"),
            lambda: ps.Compare_UInt_UInt("left", "right", "less", "equal"),
            lambda left, right, less, equal: (
                left,
                right,
                less ^ int(left < right),
                equal ^ int(left == right),
            ),
        ),
        (
            [("addend", ps.UnsignedInteger, 2), ("acc", ps.UnsignedInteger, 2)],
            ("addend", "acc"),
            lambda: ps.Add_UInt_UInt_InPlace("addend", "acc"),
            lambda addend, acc: (addend, (acc + addend) & 3),
        ),
        (
            [("x", ps.UnsignedInteger, 2), ("y", ps.UnsignedInteger, 2)],
            ("x", "y"),
            lambda: ps.Swap_General_General("x", "y"),
            lambda x, y: (y, x),
        ),
        (
            [("x", ps.Boolean, 2), ("y", ps.Boolean, 2)],
            ("x", "y"),
            lambda: ps.Swap_Bool_Bool("x", 1, "y", 0),
            lambda x, y: (
                (x & 1) | ((y & 1) << 1),
                (y & 2) | ((x >> 1) & 1),
            ),
        ),
        (
            [("word", ps.Boolean, 2)],
            ("word",),
            lambda: ps.X_Bool("word", 1),
            lambda word: (word ^ 2,),
        ),
        (
            [("word", ps.Boolean, 2)],
            ("word",),
            lambda: ps.FlipBools("word"),
            lambda word: (word ^ 3,),
        ),
        (
            [("a", ps.UnsignedInteger, 2), ("b", ps.UnsignedInteger, 2), ("out", ps.UnsignedInteger, 2)],
            ("a", "b", "out"),
            lambda: ps.Sub_UInt_UInt("a", "b", "out"),
            lambda a, b, out: (a, b, out ^ ((a - b) & 3)),
        ),
        (
            [("num", ps.UnsignedInteger, 2), ("den", ps.UnsignedInteger, 2), ("q", ps.UnsignedInteger, 2)],
            ("num", "den", "q"),
            lambda: ps.Div_UInt_UInt("num", "den", "q"),
            lambda num, den, q: (num, den, q ^ (((num // den) if den else 0) & 3)),
        ),
        (
            [("s", ps.Boolean, 1), ("x", ps.UnsignedInteger, 2), ("y", ps.UnsignedInteger, 2), ("out", ps.UnsignedInteger, 2)],
            ("s", "x", "y", "out"),
            lambda: ps.Select_Bool_UInt_UInt("s", "x", "y", "out"),
            lambda s, x, y, out: (s, x, y, out ^ ((x if s else y) & 3)),
        ),
    ],
)
def test_consumed_operations_match_independent_basis_models(
    specs, names, make_op, model
):
    """Exhaust every small-width input, including arbitrary nonzero targets."""
    _declare(specs)
    widths = {name: width for name, _, width in specs}
    for values in itertools.product(*(range(1 << widths[name]) for name in names)):
        state = _basis(dict(zip(names, values)))
        make_op()(state)
        assert tuple(_snapshot(state, names)) == (model(*values),)


@pytest.mark.parametrize(
    ("specs", "names", "make_op"),
    [
        (
            [("a", ps.UnsignedInteger, 2), ("b", ps.UnsignedInteger, 2), ("out", ps.UnsignedInteger, 2)],
            ("a", "b", "out"),
            lambda: ps.Add_UInt_UInt("a", "b", "out"),
        ),
        (
            [("src", ps.UnsignedInteger, 2), ("dst", ps.UnsignedInteger, 2)],
            ("src", "dst"),
            lambda: ps.Assign("src", "dst"),
        ),
        (
            [
                ("left", ps.UnsignedInteger, 2),
                ("right", ps.UnsignedInteger, 2),
                ("less", ps.Boolean, 1),
                ("equal", ps.Boolean, 1),
            ],
            ("left", "right", "less", "equal"),
            lambda: ps.Compare_UInt_UInt("left", "right", "less", "equal"),
        ),
        (
            [("addend", ps.UnsignedInteger, 2), ("acc", ps.UnsignedInteger, 2)],
            ("addend", "acc"),
            lambda: ps.Add_UInt_UInt_InPlace("addend", "acc"),
        ),
        (
            [("x", ps.UnsignedInteger, 2), ("y", ps.UnsignedInteger, 2)],
            ("x", "y"),
            lambda: ps.Swap_General_General("x", "y"),
        ),
        (
            [("x", ps.Boolean, 2), ("y", ps.Boolean, 2)],
            ("x", "y"),
            lambda: ps.Swap_Bool_Bool("x", 1, "y", 0),
        ),
        (
            [("word", ps.Boolean, 2)],
            ("word",),
            lambda: ps.X_Bool("word", 1),
        ),
        (
            [("word", ps.Boolean, 2)],
            ("word",),
            lambda: ps.FlipBools("word"),
        ),
        (
            [("a", ps.UnsignedInteger, 2), ("b", ps.UnsignedInteger, 2), ("out", ps.UnsignedInteger, 2)],
            ("a", "b", "out"),
            lambda: ps.Sub_UInt_UInt("a", "b", "out"),
        ),
        (
            [("num", ps.UnsignedInteger, 2), ("den", ps.UnsignedInteger, 2), ("q", ps.UnsignedInteger, 2)],
            ("num", "den", "q"),
            lambda: ps.Div_UInt_UInt("num", "den", "q"),
        ),
        (
            [("s", ps.Boolean, 1), ("x", ps.UnsignedInteger, 2), ("y", ps.UnsignedInteger, 2), ("out", ps.UnsignedInteger, 2)],
            ("s", "x", "y", "out"),
            lambda: ps.Select_Bool_UInt_UInt("s", "x", "y", "out"),
        ),
    ],
)
def test_consumed_operations_preserve_superpositions_and_both_round_trips(
    specs, names, make_op
):
    _declare(specs)
    state = _basis({name: index % (1 << width) for index, (name, _, width) in enumerate(specs)})
    ps.Hadamard_Int(names[0], specs[0][2])(state)
    initial = _snapshot(state, names)

    op = make_op()
    op(state)
    op.dag(state)
    _assert_state(_snapshot(state, names), initial)

    state = _basis({name: index % (1 << width) for index, (name, _, width) in enumerate(specs)})
    ps.Hadamard_Int(names[0], specs[0][2])(state)
    op = make_op()
    op.dag(state)
    op(state)
    _assert_state(_snapshot(state, names), initial)


@pytest.mark.parametrize(
    ("method", "active", "inactive"),
    [
        ("conditioned_by_nonzeros", {"cw": 2, "c": 1}, {"cw": 0, "c": 1}),
        ("conditioned_by_all_ones", {"cw": 3, "c": 1}, {"cw": 2, "c": 1}),
        ("conditioned_by_bit", {"cw": 2, "c": 1}, {"cw": 1, "c": 1}),
        ("conditioned_by_value", {"cw": 2, "c": 1}, {"cw": 3, "c": 1}),
    ],
)
def test_all_consumed_native_control_wrappers_follow_conjunction_model(
    method, active, inactive
):
    _declare(
        [
            ("addend", ps.UnsignedInteger, 2),
            ("acc", ps.UnsignedInteger, 2),
            ("cw", ps.Boolean, 2),
            ("c", ps.Boolean, 1),
        ]
    )

    def controlled():
        op = ps.Add_UInt_UInt_InPlace("addend", "acc")
        if method in {"conditioned_by_nonzeros", "conditioned_by_all_ones"}:
            return getattr(op, method)(["cw", "c"])
        if method == "conditioned_by_bit":
            return op.conditioned_by_bit([("cw", 1), ("c", 0)])
        return op.conditioned_by_value([("cw", 2), ("c", 1)])

    for controls, expected in ((active, 3), (inactive, 2)):
        state = _basis({"addend": 1, "acc": 2, **controls})
        controlled()(state)
        assert next(iter(_snapshot(state, ("acc",)))) == (expected,)


def test_controlled_dagger_is_coherent_in_both_orders():
    _declare(
        [
            ("addend", ps.UnsignedInteger, 2),
            ("acc", ps.UnsignedInteger, 2),
            ("control", ps.Boolean, 1),
        ]
    )
    for dagger_first in (False, True):
        state = _basis({"addend": 1, "acc": 3})
        ps.Hadamard_Bool("control")(state)
        initial = _snapshot(state, ("addend", "acc", "control"))
        op = ps.Add_UInt_UInt_InPlace("addend", "acc").conditioned_by_bit("control", 0)
        if dagger_first:
            op.dag(state)
            op(state)
        else:
            op(state)
            op.dag(state)
        _assert_state(_snapshot(state, ("addend", "acc", "control")), initial)


@pytest.mark.parametrize("load", [ps.QRAMLoad, ps.QRAMLoadFast])
def test_qram_is_persistent_controlled_xor_memory(load):
    memory = [1, 6, 3, 4]
    qram = ps.QRAMCircuit_qutrit(2, 3, memory)
    _declare(
        [
            ("addr", ps.UnsignedInteger, 2),
            ("data", ps.UnsignedInteger, 3),
            ("control", ps.Boolean, 1),
        ]
    )

    for initial_data in (0, 5, 7):
        state = _basis({"data": initial_data, "control": 1})
        ps.Hadamard_Int("addr", 2)(state)
        op = load(qram, "addr", "data").conditioned_by_value("control", 1)
        op(state)
        expected_amp = 0.5
        expected = {
            (addr, initial_data ^ memory[addr], 1): expected_amp for addr in range(4)
        }
        _assert_state(_snapshot(state, ("addr", "data", "control")), expected)
        op(state)
        expected = {(addr, initial_data, 1): expected_amp for addr in range(4)}
        _assert_state(_snapshot(state, ("addr", "data", "control")), expected)

    off = _basis({"addr": 2, "data": 5, "control": 0})
    load(qram, "addr", "data").conditioned_by_value("control", 1)(off)
    assert tuple(_snapshot(off, ("addr", "data", "control"))) == ((2, 5, 0),)

    reused = _basis({"addr": 1, "data": 2, "control": 1})
    load(qram, "addr", "data")(reused)
    assert tuple(_snapshot(reused, ("addr", "data"))) == ((1, 2 ^ memory[1]),)


def test_nonunitary_aliases_and_bit_range_errors_are_rejected():
    _declare(
        [
            ("a", ps.UnsignedInteger, 2),
            ("b", ps.UnsignedInteger, 2),
            ("wide", ps.UnsignedInteger, 3),
            ("f1", ps.Boolean, 1),
            ("f2", ps.Boolean, 1),
        ]
    )
    qram = ps.QRAMCircuit_qutrit(2, 2, [0, 1, 2, 3])
    factories = [
        lambda: ps.Add_UInt_UInt("a", "b", "a"),
        lambda: ps.Assign("a", "a"),
        lambda: ps.Add_UInt_UInt_InPlace("a", "a"),
        lambda: ps.Compare_UInt_UInt("a", "b", "f1", "f1"),
        lambda: ps.Swap_General_General("a", "a"),
        lambda: ps.QRAMLoad(qram, "a", "a"),
        lambda: ps.QRAMLoadFast(qram, "a", "a"),
        lambda: ps.X_Bool("f1", 1),
    ]
    for factory in factories:
        with pytest.raises((ValueError, RuntimeError)):
            factory()


def test_inplace_add_supports_consumed_unequal_width_translation():
    _declare(
        [
            ("bit", ps.UnsignedInteger, 1),
            ("address", ps.UnsignedInteger, 3),
        ]
    )
    for bit, address in itertools.product(range(2), range(8)):
        state = _basis({"bit": bit, "address": address})
        op = ps.Add_UInt_UInt_InPlace("bit", "address")
        op(state)
        assert tuple(_snapshot(state, ("bit", "address"))) == (
            (bit, (address + bit) & 7),
        )
        op.dag(state)
        assert tuple(_snapshot(state, ("bit", "address"))) == ((bit, address),)


def test_control_superposition_matches_positive_and_negative_branches():
    _declare([("control", ps.Boolean, 1), ("target", ps.Boolean, 1)])
    state = ps.SparseState()
    ps.Hadamard_Bool("control")(state)
    ps.X_Bool("target", 0).conditioned_by_bit("control", 0)(state)
    amp = 1 / math.sqrt(2)
    _assert_state(_snapshot(state, ("control", "target")), {(0, 0): amp, (1, 1): amp})
