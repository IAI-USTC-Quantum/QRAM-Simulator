# Tests for the baseline-facing binding surface added to QRAMCircuitQubit /
# TimeStep (no uniqc dependency): zerobus input, output distribution, the
# no-post-selection fidelity conventions, and the readable schedule.

import math

import pytest

import qram_simulator as qs


MEMORY = [0, 1, 1, 0]


def test_module_surface_gained_baseline_entries():
    for name in ("set_input_zerobus", "get_output_distribution", "get_fidelity_conventions"):
        assert hasattr(qs.QRAMCircuitQubit, name)
    assert hasattr(qs.TimeStep, "layer_entangle_max")
    assert hasattr(qs.OperationPack, "operations")


def test_layer_entangle_max_matches_schedule_formula():
    ts = qs.TimeStep(2, 1)
    full = ts.full_step()
    for step in range(1, full):
        out = full - step
        if step <= 3 * 2 - 2:
            expect = step // 3
        elif out <= 3 * 2 - 2:
            expect = out // 3
        else:
            expect = 1
        assert ts.layer_entangle_max(step) == expect


def test_addr1_has_no_position_noise():
    ts = qs.TimeStep(1, 1)
    assert all(ts.layer_entangle_max(s) == 0 for s in range(1, ts.full_step()))


def test_schedule_operations_readable():
    ts = qs.TimeStep(2, 1)
    slices = ts.generate({}, qs.ARCH_QUBIT)
    ops = [op for pack in slices.time_slices for op in pack.operations]
    assert ops, "noise-free schedule must contain logical operations"
    kinds = {op.type for op in ops}
    assert kinds <= {
        qs.OperationType.FirstCopy, qs.OperationType.CopyIn, qs.OperationType.CopyOut,
        qs.OperationType.SwapInternal, qs.OperationType.ControlSwap, qs.OperationType.FetchData,
    }
    assert qs.OperationType.FetchData in kinds
    assert qs.OperationType.ControlSwap in kinds


def test_zerobus_noise_free_distribution():
    qs.set_seed(7)
    qram = qs.QRAMCircuitQubit(2, 1, MEMORY)
    qram.set_input_zerobus()
    qram.run_full()
    dist = qram.get_output_distribution()
    # zerobus: bus_out = memory[a]; each of the four addresses with weight 1/4
    assert set(dist) == {"0:0", "1:1", "2:1", "3:0"}
    for value in dist.values():
        assert value == pytest.approx(0.25, abs=1e-12)
    assert sum(dist.values()) == pytest.approx(1.0, abs=1e-12)


def test_fidelity_conventions_noise_free_are_one():
    qs.set_seed(7)
    qram = qs.QRAMCircuitQubit(2, 1, MEMORY)
    qram.set_input_zerobus()
    qram.run_full()
    conv = qram.get_fidelity_conventions()
    assert conv["overlap_fid"] == pytest.approx(1.0, abs=1e-12)
    assert conv["fid_nopost"] == pytest.approx(1.0, abs=1e-12)
    assert conv["fid_incoh"] == pytest.approx(1.0, abs=1e-12)


def test_fidelity_conventions_track_noise_monotonically():
    last = 1.0
    for p in (0.02, 0.1, 0.4):
        qs.set_seed(20260925)
        qram = qs.QRAMCircuitQubit(2, 1, MEMORY)
        qram.set_noise_models({qs.OperationType.Depolarizing: p})
        qram.set_input_zerobus()
        qram.run_full()
        conv = qram.get_fidelity_conventions()
        assert 0.0 <= conv["overlap_fid"] <= last + 1e-12
        last = conv["overlap_fid"]


def test_trajectories_are_independent_realizations():
    # repeated run_full must re-sample the noise schedule: with depolarizing,
    # per-trajectory overlap fidelity fluctuates; the averaged distribution
    # stays normalized (survival 1 for depolarizing)
    qs.set_seed(20260925)
    qram = qs.QRAMCircuitQubit(2, 1, MEMORY)
    qram.set_noise_models({qs.OperationType.Depolarizing: 0.1})
    qram.set_input_zerobus()
    values = set()
    for _ in range(8):
        qram.run_full()
        values.add(qram.get_fidelity_conventions()["overlap_fid"])
    assert len(values) > 1
    assert math.isclose(sum(qram.get_output_distribution().values()), 1.0, abs_tol=1e-9)


def test_memory_out_of_range_bits_are_never_fetched():
    # FetchData fetches bit-by-bit (digit < data_size), so bits of an
    # out-of-range memory value beyond data_size simply never reach the bus,
    # and the fidelity expectation masks to the bus width -- such memories are
    # silently degraded, which is why the baseline constructor rejects them
    qs.set_seed(7)
    qram = qs.QRAMCircuitQubit(2, 1, [0, 2, 1, 0])  # cell 1 exceeds 1 bit
    qram.set_input_zerobus()
    qram.run_full()
    dist = qram.get_output_distribution()
    assert dist["1:0"] == pytest.approx(0.25, abs=1e-12)  # bit 0 of 2 is 0
    assert sum(dist.values()) == pytest.approx(1.0, abs=1e-9)
    assert qram.get_fidelity_conventions()["overlap_fid"] == pytest.approx(1.0, abs=1e-12)
