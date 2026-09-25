# Functional smoke tests for the qram_simulator binding layer.
# Run: pytest bindings/python/test (requires `pip install .` first, locally or in CI)
#
# Semantics note (kept consistent with the C++ implementation):
# - QRAMFullAmp::apply internally calls qutrit QRAMCircuit::run(version),
#   which raises when no noise is set; therefore all QRAMFullAmp test cases
#   inject near-zero noise.

import pytest

import qram_simulator as qs

# Near-zero noise: satisfies the "must be non-empty" constraint of
# run(version) while being physically close to noiseless evolution
TINY_NOISE = {qs.OperationType.Depolarizing: 1e-15}


def test_module_surface():
    # All core exports present (regression guard for the binding surface)
    for name in (
        "QRAMCircuitQubit",
        "QRAMCircuitQutrit",
        "QRAMFullAmp",
        "TimeStep",
        "Operation",
        "OperationPack",
        "TimeSlices",
        "OperationType",
        "set_seed",
        "get_seed",
        "ARCH_QUBIT",
        "ARCH_QUTRIT",
    ):
        assert hasattr(qs, name), f"missing export: {name}"
    assert isinstance(qs.__version__, str)


def test_seed_roundtrip_and_reproducibility():
    qs.set_seed(12345)
    assert qs.get_seed() == 12345

    def run_once():
        qs.set_seed(12345)
        qram = qs.QRAMCircuitQutrit(2, 2)
        qram.set_memory_random()
        qram.set_noise_models({qs.OperationType.Depolarizing: 1e-3})
        qram.set_input_random(8)
        qram.run_normal()
        return qram.sample_and_get_fidelity()

    assert run_once() == run_once()


def test_qubit_noise_free_fidelity():
    qs.set_seed(42)
    qram = qs.QRAMCircuitQubit(3, 2)
    qram.set_memory_random()
    qram.set_input_uniform(20)
    qram.run_normal()
    # With no noise, the sampled fidelity should return to 1
    assert qram.sample_and_get_fidelity() == pytest.approx(1.0, abs=1e-9)


def test_qutrit_noise_free_fidelity():
    # run_normal is called directly and bypasses the noise gate of
    # run(version), so noiseless operation is allowed
    qs.set_seed(7)
    qram = qs.QRAMCircuitQutrit(3, 2)
    qram.set_memory_random()
    qram.set_input_uniform(20)
    qram.run_normal()
    assert qram.sample_and_get_fidelity() == pytest.approx(1.0, abs=1e-9)


def test_noise_model_validation():
    qram = qs.QRAMCircuitQubit(2, 1)
    with pytest.raises(RuntimeError):
        qram.set_noise_models({qs.OperationType.Depolarizing: 1.5})
    with pytest.raises(RuntimeError):
        qram.set_noise_models({qs.OperationType.Damping: 1.0})
    assert qram.is_noise_free()

    qram.set_noise_models({qs.OperationType.Depolarizing: 1e-3})
    assert not qram.is_noise_free()
    assert not qram.has_damping()

    qram.set_noise_models({qs.OperationType.Depolarizing: 1e-3,
                           qs.OperationType.Damping: 1e-4})
    assert qram.has_damping()


def test_qubit_noisy_fidelity_in_range():
    qs.set_seed(11)
    qram = qs.QRAMCircuitQubit(3, 2)
    qram.set_memory_random()
    qram.set_noise_models({
        qs.OperationType.Depolarizing: 1e-3,
        qs.OperationType.Damping: 1e-4,
    })
    qram.set_input_uniform(20)
    qram.run_normal()
    fidelity = qram.sample_and_get_fidelity()
    assert 0.0 <= fidelity <= 1.0 + 1e-9


def test_qubit_run_versions_consistent():
    # Pruned and unpruned runs should agree in the noiseless case
    def run(fn):
        qs.set_seed(2024)
        qram = qs.QRAMCircuitQubit(2, 2)
        qram.set_memory_random()
        qram.set_input_uniform(10)
        fn(qram)
        return qram.sample_and_get_fidelity()

    assert run(lambda q: q.run_normal()) == pytest.approx(
        run(lambda q: q.run_full()), abs=1e-12)
    assert run(lambda q: q.run("normal")) == pytest.approx(
        run(lambda q: q.run("full")), abs=1e-12)


def test_time_step_schedule():
    ts = qs.TimeStep(2, 1)
    assert ts.full_step() > 0

    noise_free = ts.generate({}, qs.ARCH_QUBIT)
    assert len(noise_free) > 0
    assert str(noise_free)  # printable

    noisy = ts.generate({qs.OperationType.Depolarizing: 1e-3}, qs.ARCH_QUTRIT)
    assert len(noisy) > 0
    assert len(noisy.time_slices) == len(noisy)

    pack = ts.generate_step(1)
    assert isinstance(pack, qs.OperationPack)
    assert ts.is_bad_branch(0) in (True, False)
    lo, hi = ts.get_bad_range_qubit(0)
    assert 0 <= lo <= hi


def test_full_amp_zero_address_identity():
    # Loading address |0...0> with mem[0]=0 leaves the state unchanged
    # (an exact assertion independent of the bit-ordering convention)
    qs.set_seed(5)
    manipulator = qs.QRAMFullAmp(2, 2, [0, 0, 0, 0])
    manipulator.set_noise_models(TINY_NOISE)
    state = [0.0j] * 16
    state[0] = 1.0 + 0.0j
    out = manipulator.apply(state, [0, 1], [2, 3], [], version="normal")
    assert len(out) == 16
    for got, expect in zip(out, state):
        assert got == pytest.approx(expect, abs=1e-6)


def test_full_amp_uniform_address_loading():
    # Uniform superposition of |a>|00> (addr qubits {0,1}, data qubits
    # {2,3}, little-endian convention) + memory=[0,1,2,3] -> after loading,
    # each of the 4 basis states has probability 1/4 with distinct data words
    qs.set_seed(5)
    manipulator = qs.QRAMFullAmp(2, 2, [0, 1, 2, 3])
    manipulator.set_noise_models(TINY_NOISE)
    state = [0.0j] * 16
    for addr in range(4):
        state[addr] = 0.5 + 0.0j  # data qubits (high bits) are 0
    out = manipulator.apply(state, [0, 1], [2, 3], [], version="normal")

    probs = [abs(c) ** 2 for c in out]
    assert sum(probs) == pytest.approx(1.0, abs=1e-9)
    nonzero = sorted(p for p in probs if p > 1e-6)
    assert nonzero == pytest.approx([0.25, 0.25, 0.25, 0.25], abs=1e-6)
    nonzero_idx = [i for i, p in enumerate(probs) if p > 1e-6]
    data_words = {i >> 2 for i in nonzero_idx}
    assert len(data_words) == 4  # four addresses load four distinct data words


def test_full_amp_deterministic_given_seed():
    # Same seed + fresh instance -> bit-identical apply results
    # (internal sampling goes through the global random engine)
    state = [0.0j] * 16
    for addr in range(4):
        state[addr] = 0.5 + 0.0j

    outs = []
    for _ in range(2):
        qs.set_seed(99)
        manipulator = qs.QRAMFullAmp(2, 2, [1, 0, 3, 2])
        manipulator.set_noise_models({qs.OperationType.Depolarizing: 1e-3})
        outs.append(manipulator.apply(state, [0, 1], [2, 3], [], version="normal"))

    assert outs[0] == outs[1]
