"""Regression tests for condition-preserving primitive daggers."""

from __future__ import annotations

import math

import pysparq as ps


def _snapshot(state: ps.SparseState) -> dict[tuple[int, ...], complex]:
    return {
        tuple(
            system.get(index).value for index in range(len(ps.System.name_register_map))
        ): system.amplitude
        for system in state.basis_states
        if abs(system.amplitude) > 1e-12
    }


def _assert_same_state(
    actual: dict[tuple[int, ...], complex],
    expected: dict[tuple[int, ...], complex],
) -> None:
    assert actual.keys() == expected.keys()
    for basis, amplitude in expected.items():
        assert abs(actual[basis] - amplitude) < 1e-10


def _controlled_superposition_state(*, data_width: int = 1, data_value: int = 0):
    ps.System.clear()
    state = ps.SparseState()
    ps.AddRegister("control", ps.Boolean, 1)(state)
    ps.AddRegister("data", ps.UnsignedInteger, data_width)(state)
    for bit in range(data_width):
        if (data_value >> bit) & 1:
            ps.Xgate_Bool("data", bit)(state)
    ps.Hadamard_Bool("control")(state)
    return state


def test_conditioned_rotation_preserves_controls_in_both_round_trip_orders():
    for dagger_first in (False, True):
        state = _controlled_superposition_state()
        expected = _snapshot(state)
        op = ps.RYgate_Bool("data", 0, 0.731).conditioned_by_bit("control", 0)
        if dagger_first:
            op.dag(state)
            op(state)
        else:
            op(state)
            op.dag(state)
        _assert_same_state(_snapshot(state), expected)


def test_conditioned_rz_has_a_control_preserving_dagger():
    for dagger_first in (False, True):
        state = _controlled_superposition_state()
        ps.Hadamard_Bool("data")(state)
        expected = _snapshot(state)
        op = ps.RZgate_Bool("data", 0, 0.417).conditioned_by_bit("control", 0)
        if dagger_first:
            op.dag(state)
            op(state)
        else:
            op(state)
            op.dag(state)
        _assert_same_state(_snapshot(state), expected)


def test_conditioned_shift_preserves_controls_in_dagger():
    for dagger_first in (False, True):
        state = _controlled_superposition_state(data_width=3, data_value=3)
        expected = _snapshot(state)
        op = ps.ShiftLeft_InPlace("data", 1).conditioned_by_bit("control", 0)
        if dagger_first:
            op.dag(state)
            op(state)
        else:
            op(state)
            op.dag(state)
        _assert_same_state(_snapshot(state), expected)


def test_conditioned_general_state_prep_preserves_controls_in_dagger():
    for dagger_first in (False, True):
        state = _controlled_superposition_state()
        expected = _snapshot(state)
        target = [math.sqrt(0.3), -math.sqrt(0.7)]
        op = ps.Rot_GeneralStatePrep("data", target).conditioned_by_bit("control", 0)
        if dagger_first:
            op.dag(state)
            op(state)
        else:
            op(state)
            op.dag(state)
        _assert_same_state(_snapshot(state), expected)


def test_repeated_all_ones_condition_replaces_the_previous_register():
    ps.System.clear()
    state = ps.SparseState()
    ps.AddRegister("a", ps.Boolean, 1)(state)
    ps.AddRegister("b", ps.Boolean, 1)(state)
    ps.AddRegister("data", ps.Boolean, 1)(state)

    op = ps.Xgate_Bool("data", 0)
    op.conditioned_by_all_ones("a")
    op.conditioned_by_all_ones("b")

    assert op.condition_variable_all_ones == [ps.System.get_id("b")]


def test_flip_bools_keeps_values_width_bounded_for_value_controls():
    ps.System.clear()
    state = ps.SparseState()
    ps.AddRegister("flag", ps.Boolean, 1)(state)
    ps.AddRegister("word", ps.UnsignedInteger, 3)(state)
    ps.AddRegister("target", ps.Boolean, 1)(state)

    ps.FlipBools("flag")(state)
    ps.FlipBools("word")(state)

    flag_id = ps.System.get_id("flag")
    word_id = ps.System.get_id("word")
    assert state.basis_states[0].get(flag_id).value == 1
    assert state.basis_states[0].get(word_id).value == 0b111

    ps.Xgate_Bool("target", 0).conditioned_by_value("flag", 1)(state)
    target_id = ps.System.get_id("target")
    assert state.basis_states[0].get(target_id).value == 1
