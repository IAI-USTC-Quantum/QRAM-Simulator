"""Native logical-query fidelity without requiring a density-matrix backend."""

import pytest
import qram_simulator as qs


@pytest.mark.parametrize("bus", [1, 2])
def test_noiseless_reduced_fidelity_with_selected_addresses(bus):
    qram = qs.QRAMCircuitQubit(3, bus, [index % 2**bus for index in range(8)])
    qram.set_input_addresses([0, 2, 5, 7])
    qram.run_normal()
    assert qram.get_logical_query_fidelity() == pytest.approx(1, abs=1e-12)
    assert qram.get_logical_query_fidelity() == pytest.approx(1, abs=1e-12)


@pytest.mark.parametrize("channel", ["BitFlip", "PhaseFlip", "Depolarizing", "Damping"])
def test_reduced_fidelity_matches_full_without_consuming_measurement_rng(channel):
    memory = [0, 1, 0, 1, 1, 0, 0, 1]
    for seed in range(12):
        reports = []
        for pruned in (False, True):
            qram = qs.QRAMCircuitQubit(3, 1, memory)
            qram.set_input_addresses([0, 2, 5, 7])
            qram.set_noise_models({getattr(qs.OperationType, channel): 0.04})
            qs.set_seed(20261109 + seed)
            (qram.run_normal if pruned else qram.run_full)()
            logical = qram.get_logical_query_fidelity()
            assert -1e-12 <= logical <= 1 + 1e-12
            assert qram.get_logical_query_fidelity() == pytest.approx(logical, abs=1e-12)
            physical = qram.get_fidelity_conventions()["overlap_fid"]
            assert logical + 1e-12 >= physical
            reports.append((logical, physical, qram.sample_and_get_fidelity()))
        assert reports[0] == pytest.approx(reports[1], abs=1e-12)


def test_unsupported_multi_bus_columns_and_missing_run_are_rejected():
    qram = qs.QRAMCircuitQubit(2, 1, [0, 1, 1, 0])
    with pytest.raises(ValueError):
        qram.get_logical_query_fidelity()
    qram.set_input_addresses([0, 1])
    with pytest.raises(ValueError):
        qram.get_logical_query_fidelity()
    qram.set_input_uniform(8)  # Every address has both bus columns.
    qram.run_full()
    with pytest.raises(ValueError):
        qram.get_logical_query_fidelity()
