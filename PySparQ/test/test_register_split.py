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

        ps.Xgate_Bool("filter", 0)(state)
        ps.Xgate_Bool("bold", 3)(state)
        for name, _ in reversed(parts):
            ps.CombineRegister("state", name)(state)

        state_id = ps.System.get_id("state")
        expected = 1 | (8 << 18)
        assert state.basis_states[0].get(state_id).value == expected
    finally:
        ps.System.clear()
