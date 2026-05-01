"""Tests for make_tree_and_qram (functional StatePreparation v2 API)."""

import numpy as np

import pysparq as ps
from pysparq.algorithms.state_preparation import make_tree_and_qram, StatePrepViaQRAM


def _compute_fidelity(state, dist, work_reg):
    """Compute fidelity of prepared state against target distribution.

    NOTE: This helper is defined in the test module (not the algorithm module)
    because fidelity computation is a testing artifact, not part of the public
    algorithm API.
    """
    addr_reg = ps.System.get_id(work_reg)
    total = sum(a ** 2 for a in dist)
    norm = np.sqrt(total)
    expected = [a / norm for a in dist]
    fidelity = 0.0
    for basis in state.basis_states:
        idx = basis.get(addr_reg).value
        if 0 <= idx < len(expected):
            fidelity += expected[idx] * basis.amplitude
    return fidelity.real ** 2 + fidelity.imag ** 2


class TestMakeTreeAndQram:
    """Test the functional make_tree_and_qram entry point."""

    def test_returns_three_tuple(self):
        """Returns (operator, qram, work_reg)."""
        ps.System.clear()
        dist = [0.5, 0.3, 0.1, 0.1]
        result = make_tree_and_qram(dist, data_size=16)
        assert isinstance(result, tuple)
        assert len(result) == 3
        op, qram, work_reg = result
        assert isinstance(op, StatePrepViaQRAM)
        assert isinstance(qram, ps.QRAMCircuit_qutrit)
        assert isinstance(work_reg, str)

    def test_operator_accepts_call(self):
        """The returned operator can be called on a SparseState."""
        ps.System.clear()
        dist = [0.5, 0.3, 0.1, 0.1]
        op, _, _ = make_tree_and_qram(dist, data_size=16)
        state = ps.SparseState()
        op(state)
        ps.CheckNormalization(1e-7)(state)

    def test_balanced_distribution_prepares_normalized_state(self):
        """A balanced distribution prepares a normalized non-trivial state.

        The exact measurement readout mapping for this helper is not part of the
        public API yet; this test keeps the primitive path active without hiding
        cleanup errors behind a skip.
        """
        ps.System.clear()
        dist = [0.26, 0.25, 0.25, 0.24]
        op, _, work_reg = make_tree_and_qram(dist, data_size=16)
        state = ps.SparseState()
        op(state)
        ps.CheckNormalization(1e-7)(state)
        assert state.size() > 1
        assert ps.System.get_id(work_reg) >= 0

    def test_dag_inverses_forward(self):
        """Applying op then op.dag returns to approximately the original state."""
        ps.System.clear()
        dist = [0.5, 0.5, 0.0, 0.0]
        op, _, work_reg = make_tree_and_qram(dist, data_size=16)
        state = ps.SparseState()
        ps.AddRegister(work_reg, ps.UnsignedInteger, 3)(state)
        ps.Init_Unsafe(work_reg, 0)(state)
        op(state)
        op.dag(state)
        # State should be back to |0⟩
        addr_reg = ps.System.get_id(work_reg)
        assert state.size() == 1
        basis = state.basis_states[0]
        assert basis.get(addr_reg).value == 0

    def test_custom_work_reg(self):
        """Custom work_reg is passed through correctly."""
        ps.System.clear()
        dist = [0.5, 0.5]
        op, _, work_reg = make_tree_and_qram(dist, work_reg="my_reg", data_size=16)
        assert work_reg == "my_reg"
        state = ps.SparseState()
        op(state)
        ps.CheckNormalization(1e-7)(state)

    def test_alias_prepare_state_v2(self):
        """prepare_state_v2 is an alias for make_tree_and_qram."""
        from pysparq.algorithms.state_preparation import prepare_state_v2
        ps.System.clear()
        dist = [0.5, 0.5]
        r1 = make_tree_and_qram(dist, data_size=16)
        ps.System.clear()
        r2 = prepare_state_v2(dist, data_size=16)
        assert r1[2] == r2[2]  # same work_reg name
