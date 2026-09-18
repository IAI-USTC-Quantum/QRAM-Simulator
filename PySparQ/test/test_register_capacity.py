import gc

import pysparq as ps


class RegisterModel:
    def __init__(self, basis_values):
        self.values = [list(values) for values in basis_values]
        self.active = [True] * len(self.values[0])
        self.reusable = []

    def add(self):
        if self.reusable:
            register_id = self.reusable.pop()
            self.active[register_id] = True
            for basis in self.values:
                basis[register_id] = 0
            return register_id
        register_id = len(self.active)
        self.active.append(True)
        for basis in self.values:
            basis.append(0)
        return register_id

    def remove(self, register_id):
        self.active[register_id] = False
        self.reusable.append(register_id)
        for basis in self.values:
            basis[register_id] = 0


def assert_matches(state, model):
    assert len(state.basis_states) == len(model.values)
    for basis, expected in zip(state.basis_states, model.values):
        assert [basis.get(index).value for index in range(len(expected))] == expected
    assert [ps.System.status_of(index) for index in range(len(model.active))] == model.active


def test_cpu_system_storage_grows_well_past_the_preallocated_block():
    ps.System.clear()
    state = ps.SparseState()
    register_count = 145

    for index in range(register_count):
        register_id = ps.AddRegister(
            f"r{index}",
            ps.StateStorageType.General,
            1,
        )(state)
        assert register_id == index

    assert len(state.basis_states[0].registers) == register_count

    ps.X_Bool(f"r{register_count - 1}", 0)(state)
    basis = state.basis_states[0]
    assert basis.get(register_count - 1).value == 1
    assert basis.get(0).value == 0


def test_existing_system_lazily_materializes_new_register_slots():
    ps.System.clear()
    basis = ps.System()

    for index in range(145):
        assert (
            ps.System.add_register(
                f"late{index}",
                ps.StateStorageType.General,
                1,
            )
            == index
        )

    assert len(basis.registers) == 0
    first = basis.get(0)
    assert basis.get(144).value == 0
    assert first.value == 0
    assert len(basis.registers) == 145


def test_get_result_keeps_its_system_owner_alive():
    ps.System.clear()
    ps.System.add_register("owned", ps.StateStorageType.General, 1)

    def get_cell():
        return ps.System().get(0)

    cell = get_cell()
    gc.collect()
    assert cell.value == 0


def test_removed_dynamic_slot_is_reused_without_shrinking_other_states():
    ps.System.clear()
    state = ps.SparseState()
    for index in range(145):
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
    assert len(state.basis_states[0].registers) == 145
    assert state.basis_states[0].get(reused).value == 0


def test_generated_sequence_matches_independent_multi_basis_model():
    ps.System.clear()
    state = ps.SparseState()
    ps.AddRegister("seed", ps.StateStorageType.General, 2)(state)
    ps.Hadamard_Int_Full("seed")(state)
    model = RegisterModel([[seed] for seed in range(4)])

    for index in range(1, 145):
        assert ps.AddRegister(
            f"r{index}",
            ps.StateStorageType.General,
            1,
        )(state) == model.add()

    for register_id in (7, 8, 9, 144):
        ps.X_Bool(register_id, 0)(state)
        for basis in model.values:
            basis[register_id] = 1
    assert_matches(state, model)

    removal_order = (8, 1, 144, 7)
    ps.System.remove_register_synchronous(removal_order[0], state)
    model.remove(removal_order[0])
    for register_id in removal_order[1:]:
        ps.RemoveRegister(register_id)(state)
        model.remove(register_id)
    for register_id in reversed(removal_order):
        assert model.add() == register_id
        assert ps.AddRegister(
            f"reuse{register_id}",
            ps.StateStorageType.General,
            1,
        )(state) == register_id
    assert_matches(state, model)

    for register_id in (1, 7):
        ps.RemoveRegister(register_id)(state)
        model.remove(register_id)
    for name in ("again_a", "again_b"):
        expected_id = model.add()
        assert ps.AddRegister(
            name,
            ps.StateStorageType.General,
            1,
        )(state) == expected_id
    assert_matches(state, model)
