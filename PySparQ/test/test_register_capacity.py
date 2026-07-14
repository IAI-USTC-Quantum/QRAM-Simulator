import pysparq as ps


def test_cpu_system_storage_grows_well_past_the_preallocated_block():
    state = ps.SparseState()
    register_count = 1024

    for index in range(register_count):
        register_id = ps.AddRegister(
            f"r{index}",
            ps.StateStorageType.General,
            1,
        )(state)
        assert register_id == index

    assert len(state.basis_states[0].registers) == register_count

    ps.Xgate_Bool(f"r{register_count - 1}", 0)(state)
    basis = state.basis_states[0]
    assert basis.get(register_count - 1).value == 1
    assert basis.get(0).value == 0


def test_existing_system_lazily_materializes_new_register_slots():
    basis = ps.System()

    for index in range(300):
        assert (
            ps.System.add_register(
                f"late{index}",
                ps.StateStorageType.General,
                1,
            )
            == index
        )

    assert len(basis.registers) == 0
    assert basis.get(299).value == 0
    assert len(basis.registers) == 300


def test_removed_dynamic_slot_is_reused_without_shrinking_other_states():
    state = ps.SparseState()
    for index in range(300):
        ps.AddRegister(
            f"r{index}",
            ps.StateStorageType.General,
            1,
        )(state)

    ps.RemoveRegister("r127")(state)
    reused = ps.AddRegister(
        "reused",
        ps.StateStorageType.General,
        1,
    )(state)

    assert reused == 127
    assert len(state.basis_states[0].registers) == 300
    assert state.basis_states[0].get(reused).value == 0
