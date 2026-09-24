"""Smoke tests for the thin qram_simulator binding.

The exhaustive operator/conformance suites live in the SparQSim repository
(pysparq); this file only pins the thin binding's compile surface and the
core happy path.
"""

import pytest

import qram_simulator as qs


@pytest.fixture(autouse=True)
def _clean_system():
    qs.System.clear()
    yield
    qs.System.clear()


def test_register_lifecycle():
    qs.System.add_register("q", qs.StateStorageType.UnsignedInteger, 4)
    assert qs.System.get_qubit_count() == 4
    assert qs.System.name_of(qs.System.get_id("q")) == "q"
    assert qs.System.size_of("q") == 4
    assert qs.System.type_of("q") == qs.StateStorageType.UnsignedInteger
    assert qs.System.status_of("q") is True  # add_register activates the register


def test_init_hadamard_measure_roundtrip():
    qs.System.add_register("q", qs.StateStorageType.UnsignedInteger, 4)
    state = qs.SparseState()
    qs.Init_Unsafe("q", 5)(state)
    assert state.size() == 1

    qs.Hadamard_Int("q", 4)(state)
    qs.CheckNormalization()(state)
    assert state.size() == 16

    qs.set_seed(0)
    outcome, prob = qs.MeasureZ("q")(state)
    assert isinstance(outcome, list) and 0 <= outcome[0] < 16
    assert 0.0 < prob <= 1.0


def test_arithmetic_add():
    qs.System.add_register("a", qs.StateStorageType.UnsignedInteger, 4)
    qs.System.add_register("b", qs.StateStorageType.UnsignedInteger, 4)
    qs.System.add_register("c", qs.StateStorageType.UnsignedInteger, 4)
    state = qs.SparseState()
    qs.Init_Unsafe("a", 3)(state)
    qs.Init_Unsafe("b", 5)(state)
    qs.Init_Unsafe("c", 0)(state)
    qs.Add_UInt_UInt("a", "b", "c")(state)
    # 3 + 5 = 8, c was zero-initialized so XOR == assignment
    assert qs.Probability("c", 8)(state) == pytest.approx(1.0)


def test_probability_distribution():
    qs.System.add_register("q", qs.StateStorageType.UnsignedInteger, 2)
    state = qs.SparseState()
    qs.Init_Unsafe("q", 0)(state)
    qs.Hadamard_Int("q", 2)(state)
    dist = qs.Probability.distribution(state, "q")
    assert set(dist.keys()) == {0, 1, 2, 3}
    assert sum(dist.values()) == pytest.approx(1.0)


def test_qram_load():
    memory = [1, 2, 4, 8, 16, 32, 64, 128]
    qram = qs.QRAMCircuit_qutrit(addr_size=3, data_size=8, memory=memory)
    qs.System.add_register("addr", qs.StateStorageType.UnsignedInteger, 3)
    qs.System.add_register("data", qs.StateStorageType.UnsignedInteger, 8)
    state = qs.SparseState()
    qs.Init_Unsafe("addr", 5)(state)
    qs.QRAMLoad(qram, "addr", "data")(state)
    assert qs.Probability("data", memory[5])(state) == pytest.approx(1.0)


def test_state_printing():
    qs.System.add_register("q", qs.StateStorageType.UnsignedInteger, 4)
    state = qs.SparseState()
    qs.Init_Unsafe("q", 5)(state)
    text = qs.StatePrint(qs.StatePrintDisplay.Detail)(state)
    assert "q" in text
    assert str(state) == state.to_string(1)
