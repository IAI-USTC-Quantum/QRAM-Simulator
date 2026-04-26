"""Tests for ControllableOperatorMixin."""

import pytest
from pysparq.operators import ControllableOperatorMixin


class _DummyOp(ControllableOperatorMixin):
    """Minimal concrete subclass for testing the mixin."""

    def __init__(self, *, condition_regs=None, condition_bits=None):
        super().__init__(condition_regs=condition_regs, condition_bits=condition_bits)

    def __call__(self, state):
        pass

    def dag(self, state):
        pass


class TestControllableOperatorMixin:
    """Test the condition-setting mixin."""

    def test_returns_self_for_chaining(self):
        """conditioned_by_nonzeros, conditioned_by_bit return self for chaining."""
        op = _DummyOp()
        r = op.conditioned_by_nonzeros("flag").conditioned_by_bit("ctrl", 0)
        assert r is op

    def test_conditioned_by_nonzeros_single(self):
        """Single non-condition converted to list."""
        op = _DummyOp()
        op.conditioned_by_nonzeros("reg")
        assert op.condition_regs == ["reg"]

    def test_conditioned_by_nonzeros_list(self):
        """List condition stored as-is."""
        op = _DummyOp()
        op.conditioned_by_nonzeros(["a", "b"])
        assert op.condition_regs == ["a", "b"]

    def test_conditioned_by_all_ones_single(self):
        """Single all-ones condition converted to list."""
        op = _DummyOp()
        op.conditioned_by_all_ones("reg")
        assert op.condition_regs == ["reg"]

    def test_conditioned_by_all_ones_list(self):
        """List all-ones condition stored as list."""
        op = _DummyOp()
        op.conditioned_by_all_ones(["a", "b"])
        assert op.condition_regs == ["a", "b"]

    def test_conditioned_by_bit_replaces(self):
        """conditioned_by_bit replaces (not appends) the condition_bits list.

        This matches the original behavior in BlockEncodingTridiagonal and WalkS,
        where calling conditioned_by_bit twice replaces rather than accumulates.
        """
        op = _DummyOp()
        op.conditioned_by_bit("ctrl", 0)
        assert op.condition_bits == [("ctrl", 0)]
        op.conditioned_by_bit("ctrl", 1)
        assert op.condition_bits == [("ctrl", 1)]

    def test_clear_conditions(self):
        """clear_conditions resets both lists."""
        op = _DummyOp()
        op.conditioned_by_nonzeros(["a", "b"])
        op.conditioned_by_bit("c", 3)
        op.clear_conditions()
        assert op.condition_regs == []
        assert op.condition_bits == []

    def test_init_with_kwargs(self):
        """Init-time kwargs set the lists."""
        op = _DummyOp(condition_regs=["x"], condition_bits=[("y", 2)])
        assert op.condition_regs == ["x"]
        assert op.condition_bits == [("y", 2)]

    def test_condition_bits_replace_on_conditioned_by_bit(self):
        """conditioned_by_bit replaces the entire list."""
        op = _DummyOp()
        op.conditioned_by_bit("ctrl", 0)
        assert op.condition_bits == [("ctrl", 0)]
        op.conditioned_by_bit("ctrl", 1)
        assert op.condition_bits == [("ctrl", 1)]

    def test_conditioned_by_nonzeros_overwrites(self):
        """conditioned_by_nonzeros replaces existing list."""
        op = _DummyOp()
        op.conditioned_by_nonzeros(["a"])
        op.conditioned_by_nonzeros(["b", "c"])
        assert op.condition_regs == ["b", "c"]
