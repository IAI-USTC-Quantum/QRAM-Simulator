"""Regression tests for reversible split/combine register semantics."""

import pysparq as ps


def test_combine_masks_one_bit_sentinel_values():
    ps.System.clear()
    try:
        ps.System.add_register("state", ps.StateStorageType.General, 22)
        state = ps.SparseState()
        parts = (
            ("filter", 1),
            ("h", 1),
            ("q_lcu", 1),
            ("dil_anc", 14),
            ("q_b", 1),
            ("bold", 4),
        )
        for name, width in parts:
            ps.SplitRegister("state", name, width)(state)

        ps.X_Bool("filter", 0)(state)
        ps.X_Bool("bold", 3)(state)
        for name, _ in reversed(parts):
            ps.CombineRegister("state", name)(state)

        state_id = ps.System.get_id("state")
        expected = 1 | (8 << 18)
        assert state.basis_states[0].get(state_id).value == expected
    finally:
        ps.System.clear()


def test_split_combine_masks_sentinels_for_multiple_basis_states():
    ps.System.clear()
    try:
        state = ps.SparseState()
        ps.AddRegister("seed", ps.StateStorageType.General, 2)(state)
        ps.Hadamard_Int_Full("seed")(state)
        ps.AddRegister("packed", ps.StateStorageType.General, 12)(state)
        for digit in (0, 1, 5, 7, 11):
            ps.X_Bool("packed", digit)(state)

        packed_id = ps.System.get_id("packed")
        low_id = ps.SplitRegister("packed", "low", 5)(state)
        for basis in state.basis_states:
            assert basis.get(packed_id).value == 0b1000101
            assert basis.get(low_id).value == 0b00011

        ps.CombineRegister("packed", "low")(state)
        for basis in state.basis_states:
            assert basis.get(packed_id).value == 0b100010100011

        assert ps.AddRegister(
            "recycled_low",
            ps.StateStorageType.General,
            5,
        )(state) == low_id
        for basis in state.basis_states:
            assert basis.get(low_id).value == 0
    finally:
        ps.System.clear()
