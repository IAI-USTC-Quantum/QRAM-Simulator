# Tests of the circuit-level baseline (qram_simulator.baseline) against the
# QRAM-Simulator core through the same binding surface used by the packaged
# comparison experiment.  Skipped automatically when the optional uniqc
# (UnifiedQuantum) + qutip stack is not installed.

import pytest

import qram_simulator as qs

# Skip before touching qs.baseline (its __init__ eagerly imports
# circuit_qram, which needs numpy) and before importing numpy directly:
# CI runs on bare `pip install . pytest` environments.
pytest.importorskip("uniqc.simulator.qutip_sim_impl")

import numpy as np

qs.baseline  # lazy import guard (fails here, not deep inside, if broken)

from qram_simulator.baseline import CircuitQRAMQubit, classical_fidelity, tvd
from qram_simulator.baseline.compare import run_circuit_side, run_qram_side

MEMORY = [0, 1, 1, 0]


def _density_in_sdk_order(reference):
    """Encode a QuTiP reference tensor using the real SDK's observed bit order."""
    from uniqc.simulator.qutip_sim_impl import DensityOperatorSimulatorQutip

    backend = DensityOperatorSimulatorQutip()
    backend.init_n_qubit(2)
    backend.x(0)
    flipped = int(np.argmax(np.real(backend.density_matrix.diag())))
    assert flipped in (1, 2), "backend X(0) did not identify a basis convention"
    if flipped == 1:
        # Reference axes start with qubit0; SDK matrix index has qubit0 last.
        reference = reference.permute(list(reversed(range(len(reference.dims[0])))))
    return reference.full()


def test_physical_partial_trace_agrees_with_qutip_and_keeps_complex_coherence():
    import qutip

    circuit = CircuitQRAMQubit(1, 1, [0, 1])
    # Reference tensor axes: address, bus, then the two router bits.
    # Reorder visible MSB tensor indices to address + (bus << addr_size).
    visible = qutip.Qobj([0, 1j*np.sqrt(0.7), np.sqrt(0.3), 0], dims=[[2, 2], [1, 1]])
    tree = np.sqrt(0.4)*qutip.basis(4, 0)+1j*np.sqrt(0.6)*qutip.basis(4, 3)
    tree.dims = [[2, 2], [1, 1]]
    joint = qutip.tensor(visible, tree).proj()
    order = [0, 2, 1, 3]
    expected = joint.ptrace([0, 1]).full()[np.ix_(order, order)]
    actual = circuit.logical_density_matrix(_density_in_sdk_order(joint))
    np.testing.assert_allclose(actual, expected, rtol=0, atol=1e-12)
    assert abs(actual[1, 2].imag) > 0.4  # Populations cannot pass this test.
    assert np.trace(actual) == pytest.approx(1)


def test_router_excitation_and_which_path_information_are_distinguished():
    import qutip

    circuit = CircuitQRAMQubit(1, 1, [0, 1])
    bell = (qutip.tensor(qutip.basis(2, 0), qutip.basis(2, 0)) +
            qutip.tensor(qutip.basis(2, 1), qutip.basis(2, 1)))/np.sqrt(2)
    tree = qutip.tensor(qutip.basis(2, 1), qutip.basis(2, 1))
    joint = _density_in_sdk_order(qutip.tensor(bell, tree).proj())
    assert circuit.fidelity_with_ideal(joint) == pytest.approx(0, abs=1e-12)
    assert circuit.logical_fidelity_with_ideal(joint) == pytest.approx(1, abs=1e-12)
    which_path = (qutip.tensor(qutip.basis(2, 0), qutip.basis(2, 0),
                              qutip.basis(2, 0), qutip.basis(2, 0)) +
                  qutip.tensor(qutip.basis(2, 1), qutip.basis(2, 1),
                              qutip.basis(2, 1), qutip.basis(2, 1)))/np.sqrt(2)
    assert circuit.logical_fidelity_with_ideal(_density_in_sdk_order(which_path.proj())) == pytest.approx(0.5, abs=1e-12)


@pytest.mark.parametrize("channel,parameter", [("BitFlip", "bit_flip"),
    ("PhaseFlip", "phase_flip"), ("Depolarizing", "depolarizing"), ("Damping", "damping")])
def test_logical_query_fidelity_matches_independent_complete_channel(channel, parameter, tmp_path):
    import json
    import math

    probability, samples, delta = 0.06, 4096, 1e-9
    circuit = CircuitQRAMQubit(2, 1, MEMORY, **{parameter: probability})
    rho = circuit.simulate_density("faithful")
    expected = circuit.logical_fidelity_with_ideal(rho)
    full_overlap = circuit.fidelity_with_ideal(rho)
    assert np.trace(rho) == pytest.approx(1, abs=1e-11)
    assert expected+1e-12 >= full_overlap
    qram = qs.QRAMCircuitQubit(2, 1, MEMORY)
    qram.set_input_zerobus()
    qram.set_noise_models({getattr(qs.OperationType, channel): probability})
    values = []
    for trajectory in range(samples):
        qs.set_seed(20261109+trajectory)
        qram.run_normal()
        value = qram.get_logical_query_fidelity()
        assert -1e-12 <= value <= 1+1e-12
        values.append(value)
    mean = float(np.mean(values))
    # A declared, fixed bounded-variable concentration bound, not a fitted
    # tolerance or a retry-until-pass Monte-Carlo sample selection.
    radius = math.sqrt(math.log(2/delta)/(2*samples))
    report = {"channel": channel, "probability": probability, "trajectories": samples,
              "seed_start": 20261109, "density_reference": expected,
              "full_circuit_overlap": full_overlap, "trajectory_mean": mean,
              "standard_error": float(np.std(values, ddof=1)/math.sqrt(samples)),
              "confidence_radius": radius, "delta": delta}
    (tmp_path/"logical-channel.json").write_text(json.dumps(report, indent=2)+"\n")
    assert abs(mean-expected) <= radius


@pytest.mark.parametrize("addr,data,memory", [
    (1, 1, [0, 1]),
    (2, 1, MEMORY),
    (2, 2, [0, 1, 2, 3]),
])
def test_noise_free_bridge_bitwise(addr, data, memory):
    """Circuit-level ideal distribution == QRAM noise-free run, key for key."""
    circuit = CircuitQRAMQubit(addr, data, memory)
    ideal_dist = circuit.marginal_distribution(circuit.ideal_density_matrix())

    qram = qs.QRAMCircuitQubit(addr, data, memory)
    qram.set_input_zerobus()
    qram.run_full()
    qram_dist = qram.get_output_distribution()

    keys = set(ideal_dist) | set(qram_dist)
    assert keys
    for key in keys:
        assert ideal_dist.get(key, 0.0) == pytest.approx(qram_dist.get(key, 0.0), abs=1e-12), key


def test_constructor_validation():
    with pytest.raises(ValueError):
        CircuitQRAMQubit(3, 1, [0] * 8)  # density-matrix budget
    with pytest.raises(ValueError):
        CircuitQRAMQubit(2, 1, [0, 1])  # memory length
    with pytest.raises(ValueError):
        CircuitQRAMQubit(2, 1, [0, 2, 1, 0])  # value exceeds data_size bits
    with pytest.raises(ValueError):
        CircuitQRAMQubit(2, 1, MEMORY, depolarizing=1.5)
    with pytest.raises(ValueError):
        CircuitQRAMQubit(2, 1, MEMORY, damping=1.0)


def test_noise_free_fidelity_is_one():
    circuit = CircuitQRAMQubit(2, 1, MEMORY)
    result = circuit.run()
    assert result["trace"] == pytest.approx(1.0, abs=1e-12)
    assert result["fidelity"] == pytest.approx(1.0, abs=1e-12)


@pytest.mark.parametrize("depol", [0.02, 0.1])
def test_depol_semi_quantitative(depol):
    """Channel ensemble vs trajectory ensemble: F_cls high, F_quantum within MC error."""
    config = {"depol": depol}
    circuit = run_circuit_side(2, 1, MEMORY, config, "faithful")
    assert circuit["trace"] == pytest.approx(1.0, abs=1e-9)  # depolarizing is TP

    qram = run_qram_side(2, 1, MEMORY, config, runs=300, seed=20260925)
    from qram_simulator.baseline.metrics import normalize
    f_cls = classical_fidelity(normalize(qram["avg_dist"]), normalize(circuit["dist"]))
    assert f_cls > 0.99
    assert tvd(normalize(qram["avg_dist"]), normalize(circuit["dist"])) < 0.05

    overlap = qram["avg_overlap_fid"]
    assert abs(circuit["fidelity"] - overlap["mean"]) < max(3.0 * overlap["mc_error"], 0.03)


@pytest.mark.parametrize("damp", [0.01, 0.05])
def test_damping_current_whole_tree_channel(damp):
    """Independent gate-level Kraus channel agrees with current C++ trajectories."""
    config = {"damp": damp}
    qram = run_qram_side(2, 1, MEMORY, config, runs=300, seed=20260925)

    faithful = run_circuit_side(2, 1, MEMORY, config, "faithful")
    textbook = run_circuit_side(2, 1, MEMORY, config, "textbook")

    # The complete channel is TP, not a no-jump or discarded-weight estimator.
    assert qram["trajectory_convention"] == "joint_auxiliary_whole_tree_v1"
    assert abs(faithful["trace"] - qram["survival"]) < 0.03
    for trace in (qram["survival"], faithful["trace"], textbook["trace"]):
        assert trace == pytest.approx(1.0, abs=1e-12)
    assert faithful["fidelity"] == pytest.approx(textbook["fidelity"], abs=1e-12)

    overlap = qram["avg_overlap_fid"]
    assert abs(faithful["fidelity"] - overlap["mean"]) < max(3.0 * overlap["mc_error"], 0.05)
    # Keep the existing normalized-distribution metric and threshold.
    from qram_simulator.baseline.metrics import normalize
    assert classical_fidelity(normalize(qram["avg_dist"]),
                              normalize(faithful["dist"])) > 0.99


def test_one_layer_damping_does_not_disappear_with_empty_pauli_front():
    """Root occupation is damped even when layer_entangle_max is always zero."""
    config = {"damp": 0.2}
    qram = run_qram_side(1, 1, [0, 1], config, runs=2000, seed=20260925)
    circuit = run_circuit_side(1, 1, [0, 1], config, "faithful")
    assert circuit["trace"] == pytest.approx(1.0, abs=1e-12)
    assert qram["survival"] == pytest.approx(1.0, abs=1e-12)
    assert circuit["fidelity"] < 0.99
    overlap = qram["avg_overlap_fid"]
    assert abs(circuit["fidelity"] - overlap["mean"]) < max(3*overlap["mc_error"], 0.02)


def test_old_non_tp_mirror_requires_explicit_legacy_mode():
    circuit = CircuitQRAMQubit(1, 1, [0, 1], damping=0.05)
    legacy = circuit.simulate_density("legacy")
    current = circuit.simulate_density("faithful")
    nojump = circuit.simulate_density("nojump")
    assert float(np.trace(current).real) == pytest.approx(1.0, abs=1e-12)
    assert float(np.trace(legacy).real) < 0.99
    assert float(np.trace(nojump).real) < 0.99


def test_metrics_basics():
    p = {"0:0": 0.5, "1:1": 0.5}
    assert classical_fidelity(p, p) == pytest.approx(1.0)
    assert tvd(p, p) == pytest.approx(0.0)
    q = {"0:0": 1.0}
    assert tvd(p, q) == pytest.approx(0.5)
    assert classical_fidelity(p, q) == pytest.approx(0.5)


def test_rho_shapes_and_purity():
    circuit = CircuitQRAMQubit(2, 1, MEMORY)
    rho_ideal = circuit.ideal_density_matrix()
    assert rho_ideal.shape == (2 ** circuit.encoding.num_qubits,) * 2
    assert np.real(np.trace(rho_ideal @ rho_ideal)) == pytest.approx(1.0, abs=1e-12)
    rho = circuit.simulate_density("faithful")
    assert rho.shape == rho_ideal.shape
    assert 0.0 < np.real(np.trace(rho)) <= 1.0 + 1e-12
