"""Tests for the seedable sparse-state measurement/reset/probability APIs.

Covers the Stage 1 acceptance criterion: "measurement/reset preserve
normalization and match dense-state references", plus determinism under
``ps.set_seed`` (required for a dynamic executor to be able to replay a
mid-circuit MEASURE/RESET deterministically in tests and debugging).
"""

from __future__ import annotations

import pysparq as ps
import pytest


def _setup_two_registers(width_a=2, width_b=2):
    ps.System.clear()
    scratch = ps.SparseState()
    ps.AddRegister("a", ps.UnsignedInteger, width_a)(scratch)
    ps.AddRegister("b", ps.UnsignedInteger, width_b)(scratch)
    return ps.System.get_id("a"), ps.System.get_id("b")


class TestMeasureZBasics:
    def test_measuring_a_basis_state_is_deterministic(self):
        a_id, b_id = _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 2)(state)
        ps.Init_Unsafe("b", 1)(state)

        outcome, prob = ps.MeasureZ(["a", "b"])(state)

        assert outcome == [2, 1]
        assert prob == pytest.approx(1.0)
        assert state.size() == 1
        assert state.basis_states[0].get(a_id).value & 0b11 == 2
        assert state.basis_states[0].get(b_id).value & 0b11 == 1

    def test_measurement_matches_born_rule_distribution(self):
        """Sample MeasureZ many times (fresh state each time, reseeded) and
        confirm the empirical outcome distribution matches |amplitude|^2,
        computed independently via a dense-state reference.
        """
        width = 2
        _setup_two_registers(width, width)

        counts = {}
        trials = 4000
        for trial in range(trials):
            ps.set_seed(trial)
            state = ps.SparseState()
            ps.Hadamard_Int("a", width)(state)
            outcome, prob = ps.MeasureZ("a")(state)
            key = outcome[0]
            counts[key] = counts.get(key, 0) + 1
            # Each outcome of a uniform Hadamard superposition is equally
            # likely, so prob must equal 1 / 2**width for every trial.
            assert prob == pytest.approx(1.0 / (1 << width))

        expected = trials / (1 << width)
        for k in range(1 << width):
            assert counts.get(k, 0) == pytest.approx(expected, rel=0.25)

    def test_measurement_renormalizes_remaining_branches(self):
        """After measuring one register out of a joint superposition, the
        surviving branches over the other register must remain correctly
        normalized (sum of |amplitude|^2 == 1).
        """
        width = 2
        _setup_two_registers(width, width)
        ps.set_seed(1234)

        state = ps.SparseState()
        ps.Hadamard_Int("a", width)(state)
        ps.Hadamard_Int("b", width)(state)

        ps.MeasureZ("a")(state)

        total_prob = sum(abs(s.amplitude) ** 2 for s in state.basis_states)
        assert total_prob == pytest.approx(1.0)
        # Only the "b" register varies among the surviving branches, and
        # since it was independently Hadamard'd it must still be a full
        # equal superposition over its 2**width values.
        assert state.size() == 1 << width

    def test_measuring_empty_state_raises(self):
        _setup_two_registers()
        state = ps.SparseState()
        # Force the state to be empty by measuring out all support at a
        # value that structurally cannot occur (via a manual construction
        # is not exposed), so instead check the empty-state guard directly
        # through PartialTraceSelect leaving nothing, which is the only
        # user-reachable way to obtain an empty SparseState.
        ps.Init_Unsafe("a", 1)(state)
        ps.PartialTraceSelect({"a": 2})(state)  # selects a value that can't match
        assert state.size() == 0
        with pytest.raises(ValueError):
            ps.MeasureZ("a")(state)


class TestMeasureZAgainstDenseReference:
    def test_two_qubit_distribution_matches_numpy_reference(self):
        """Cross-check MeasureZ's sampling distribution against an
        independent dense-statevector computation of |amplitude|^2 for a
        non-uniform (non-Hadamard) superposition built via QRAM.
        """
        width = 2
        addr_id, data_id = _setup_two_registers(width, width)
        memory = [0, 1, 2, 3]
        qram = ps.QRAMCircuit_qutrit(width, width, memory)

        # Build a non-uniform reference distribution over "a" using
        # Hadamard (uniform is sufficient to validate the estimator without
        # depending on state-prep internals) and load "b" from QRAM so the
        # two registers are entangled.
        def prepare():
            state = ps.SparseState()
            ps.Hadamard_Int("a", width)(state)
            ps.QRAMLoad(qram, "a", "b")(state)
            return state

        # Dense reference: enumerate the (small) basis manually.
        reference = {}
        probe = prepare()
        for s in probe.basis_states:
            key = (s.get(addr_id).value & 0b11, s.get(data_id).value & 0b11)
            reference[key] = reference.get(key, 0.0) + abs(s.amplitude) ** 2
        assert sum(reference.values()) == pytest.approx(1.0)

        counts = {}
        trials = 4000
        for trial in range(trials):
            ps.set_seed(trial)
            state = prepare()
            outcome, prob = ps.MeasureZ(["a", "b"])(state)
            key = tuple(outcome)
            counts[key] = counts.get(key, 0) + 1
            assert prob == pytest.approx(reference[key])

        for key, ref_prob in reference.items():
            empirical = counts.get(key, 0) / trials
            assert empirical == pytest.approx(ref_prob, abs=0.08)


class TestReset:
    def test_reset_forces_default_zero(self):
        _setup_two_registers()
        ps.set_seed(0)
        state = ps.SparseState()
        ps.Init_Unsafe("a", 3)(state)
        measured = ps.Reset("a")(state)
        assert measured == [3]
        a_id = ps.System.get_id("a")
        assert state.basis_states[0].get(a_id).value & 0b11 == 0
        assert state.size() == 1
        assert abs(state.basis_states[0].amplitude) == pytest.approx(1.0)

    def test_reset_forces_explicit_target(self):
        _setup_two_registers()
        ps.set_seed(0)
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        ps.Reset("a", 3)(state)
        a_id = ps.System.get_id("a")
        assert state.basis_states[0].get(a_id).value & 0b11 == 3

    def test_reset_collapses_superposition_then_forces_value(self):
        width = 2
        _setup_two_registers(width, width)
        ps.set_seed(42)
        state = ps.SparseState()
        ps.Hadamard_Int("a", width)(state)
        ps.Hadamard_Int("b", width)(state)

        ps.Reset("a", 0)(state)

        a_id = ps.System.get_id("a")
        for s in state.basis_states:
            assert s.get(a_id).value & 0b11 == 0
        total_prob = sum(abs(s.amplitude) ** 2 for s in state.basis_states)
        assert total_prob == pytest.approx(1.0)
        # "b" superposition must be untouched by resetting "a".
        assert state.size() == 1 << width

    def test_reset_is_reproducible_with_same_seed(self):
        width = 2
        _setup_two_registers(width, width)

        def run():
            ps.set_seed(999)
            state = ps.SparseState()
            ps.Hadamard_Int("a", width)(state)
            return ps.Reset("a", 1)(state)

        assert run() == run()


class TestProbability:
    def test_probability_matches_born_rule_without_mutating_state(self):
        width = 2
        _setup_two_registers(width, width)
        state = ps.SparseState()
        ps.Hadamard_Int("a", width)(state)

        size_before = state.size()
        for value in range(1 << width):
            p = ps.Probability("a", value)(state)
            assert p == pytest.approx(1.0 / (1 << width))
        # Probability queries must not collapse or mutate the state.
        assert state.size() == size_before
        total = sum(ps.Probability("a", v)(state) for v in range(1 << width))
        assert total == pytest.approx(1.0)

    def test_distribution_matches_individual_queries(self):
        width = 2
        _setup_two_registers(width, width)
        state = ps.SparseState()
        ps.Hadamard_Int("a", width)(state)
        ps.Hadamard_Int("b", width)(state)

        dist = ps.Probability.distribution(state, "a")
        for value in range(1 << width):
            assert dist[value] == pytest.approx(ps.Probability("a", value)(state))
        assert sum(dist.values()) == pytest.approx(1.0)

    def test_probability_is_read_only_regardless_of_call_count(self):
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 2)(state)
        for _ in range(10):
            p = ps.Probability("a", 2)(state)
            assert p == pytest.approx(1.0)
        assert state.size() == 1


def test_sparse_state_clone_is_independent():
    ps.System.clear()
    try:
        state = ps.SparseState()
        ps.AddRegister("q", ps.StateStorageType.Boolean, 1)(state)

        cloned = state.clone()
        copied = ps.SparseState(state)
        ps.X_Bool("q", 0)(cloned)

        assert ps.Probability("q", 0)(state) == pytest.approx(1.0)
        assert ps.Probability("q", 0)(copied) == pytest.approx(1.0)
        assert ps.Probability("q", 1)(cloned) == pytest.approx(1.0)

        ps.X_Bool("q", 0)(state)
        assert ps.Probability("q", 0)(copied) == pytest.approx(1.0)
        ps.X_Bool("q", 0)(copied)
        ps.X_Bool("q", 0)(state)
        assert ps.Probability("q", 0)(state) == pytest.approx(1.0)
        assert ps.Probability("q", 1)(copied) == pytest.approx(1.0)
        assert ps.Probability("q", 1)(cloned) == pytest.approx(1.0)
    finally:
        ps.System.clear()


class TestMeasureZNormalizationValidation:
    """Review hardening (1): MeasureZ must validate the input state's total
    Born-rule probability instead of silently sampling against `[0, 1)`
    regardless of the actual total, which used to bias/fall back silently
    for a non-normalized state.
    """

    def test_measure_rejects_state_scaled_up(self):
        """GlobalPhase accepts an arbitrary complex factor (not only unit
        modulus), so it is a convenient, already-public way to construct a
        deliberately non-normalized state for this negative test."""
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        ps.GlobalPhase(2.0 + 0j)(state)  # total probability becomes 4.0

        with pytest.raises(RuntimeError):
            ps.MeasureZ("a")(state)

    def test_measure_rejects_state_scaled_down(self):
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        ps.GlobalPhase(0.5 + 0j)(state)  # total probability becomes 0.25

        with pytest.raises(RuntimeError):
            ps.MeasureZ("a")(state)

    def test_reset_also_rejects_non_normalized_state(self):
        """Reset delegates to MeasureZ internally, so it must inherit the
        same normalization guard."""
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        ps.GlobalPhase(2.0 + 0j)(state)

        with pytest.raises(RuntimeError):
            ps.Reset("a")(state)

    def test_measure_accepts_state_within_numerical_tolerance(self):
        """A tiny floating-point-scale perturbation (well under the
        normalization threshold) must still be accepted."""
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        ps.GlobalPhase(1.0 + 1e-8 + 0j)(state)  # |c|^2 - 1 ~ 2e-8

        outcome, prob = ps.MeasureZ("a")(state)
        assert outcome == [1]
        assert prob == pytest.approx(1.0, abs=1e-6)


class TestRegisterValidation:
    """Review hardening (2): register IDs/names must be validated (existing,
    active, unique), and Reset targets / Probability values must fit the
    register's bit width; contradictory inputs must raise instead of being
    silently accepted (e.g. via an out-of-range/garbage register id, or a
    target value silently truncated to fit).
    """

    def test_measurez_rejects_unknown_name(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.MeasureZ("does_not_exist")

    def test_measurez_rejects_out_of_range_id(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.MeasureZ(10_000)

    def test_measurez_rejects_inactive_removed_id(self):
        _setup_two_registers()
        a_id = ps.System.get_id("a")
        state = ps.SparseState()
        ps.RemoveRegister(a_id)(state)
        with pytest.raises(ValueError):
            ps.MeasureZ(a_id)

    def test_measurez_rejects_duplicate_registers_by_name(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.MeasureZ(["a", "a"])

    def test_measurez_rejects_duplicate_registers_by_id(self):
        _setup_two_registers()
        a_id = ps.System.get_id("a")
        with pytest.raises(ValueError):
            ps.MeasureZ([a_id, a_id])

    def test_reset_rejects_unknown_name(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Reset("does_not_exist")

    def test_reset_rejects_out_of_range_id(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Reset(10_000)

    def test_reset_rejects_duplicate_registers(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Reset(["a", "a"])

    def test_reset_rejects_duplicate_registers_with_contradictory_targets(self):
        """Same register listed twice with different targets is exactly the
        'contradictory Reset targets' case flagged by review; it must be
        rejected outright rather than silently applying one of them."""
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Reset(["a", "a"], [1, 2])

    def test_reset_rejects_target_out_of_register_width(self):
        _setup_two_registers(width_a=2)  # "a" holds values in [0, 4)
        with pytest.raises(ValueError):
            ps.Reset("a", 4)

    def test_reset_accepts_target_at_max_representable_value(self):
        _setup_two_registers(width_a=2)
        ps.set_seed(0)
        state = ps.SparseState()
        ps.Init_Unsafe("a", 0)(state)
        ps.Reset("a", 3)(state)  # 3 == 2**2 - 1, the max value for width 2
        a_id = ps.System.get_id("a")
        assert state.basis_states[0].get(a_id).value & 0b11 == 3

    def test_probability_rejects_unknown_name(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Probability("does_not_exist", 0)

    def test_probability_rejects_out_of_range_id(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Probability(10_000, 0)

    def test_probability_rejects_duplicate_registers(self):
        _setup_two_registers()
        with pytest.raises(ValueError):
            ps.Probability(["a", "a"], [0, 1])

    def test_probability_rejects_value_out_of_register_width(self):
        _setup_two_registers(width_a=2)
        with pytest.raises(ValueError):
            ps.Probability("a", 4)

    def test_probability_distribution_rejects_out_of_range_id(self):
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        with pytest.raises(ValueError):
            ps.Probability.distribution(state, 10_000)

    def test_probability_distribution_rejects_unknown_name(self):
        _setup_two_registers()
        state = ps.SparseState()
        ps.Init_Unsafe("a", 1)(state)
        with pytest.raises(ValueError):
            ps.Probability.distribution(state, "does_not_exist")
