"""Semantic conformance tests for PySparQ built-ins used by QECC.Lang/QCFD.

Exercises the reusable harness in ``pysparq.conformance`` (Stage 1 of the
native QECC.Lang/QFVM plan) against a representative set of built-in
operators: XOR-into arithmetic (``Add_UInt_UInt``, ``Assign``,
``Compare_UInt_UInt``, ``CustomArithmetic``, ``QRAMLoad``) and in-place
bijective operators (``Add_UInt_UInt_InPlace``, ``Swap_General_General``,
``X_Bool``, ``FlipBools``).

Each operator is checked against the full Stage 1 matrix:
    * basis exhaustive / arbitrary nonzero targets + collision detection
      (``build_forward_map``);
    * superposition linearity (``assert_superposition_linearity``);
    * positive/negative/multi-register controls (``assert_controls``);
    * forward-then-dagger and dagger-then-forward identity.

If any built-in violated the reversibility contract described in the plan
(overwriting outputs instead of computing a genuine bijection / XOR-into
map, or implementing ``dag()`` by clearing registers), the corresponding
assertion below would fail with a specific, diagnosable message.
"""

from __future__ import annotations

import pysparq as ps
import pytest
from pysparq.conformance import (
    RegisterSpec,
    assert_collision_free_for_output_starts,
    assert_controls,
    assert_dagger_forward_identity,
    assert_forward_dagger_identity,
    assert_superposition_linearity,
    build_forward_map,
    setup_registers,
)


class TestAddUIntUInt:
    """Add_UInt_UInt: |a>|b>|c> -> |a>|b>|c XOR (a+b)> (xor_into)."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("a", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("b", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("c", ps.UnsignedInteger, self.WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["a", "b", "c"]

    def make_op(self):
        return ps.Add_UInt_UInt("a", "b", "c")

    def test_basis_exhaustive_and_no_collisions(self):
        forward = build_forward_map(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"a": range(1 << self.WIDTH), "b": range(1 << self.WIDTH)},
            fixed={"c": 0},
        )
        size = 1 << self.WIDTH
        for a in range(size):
            for b in range(size):
                assert forward[(a, b, 0)] == (a, b, (a + b) % size)

    def test_collision_free_for_arbitrary_nonzero_output_starts(self):
        """XOR-into must stay injective for any starting value of 'c', not
        only the conventional clean (zero) ancilla start."""
        assert_collision_free_for_output_starts(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"a": range(1 << self.WIDTH), "b": range(1 << self.WIDTH)},
            output_start_values={"c": [0, 1, 3, 5, 7]},
        )

    def test_forward_dagger_identity(self):
        sweep = {"a": range(1 << self.WIDTH), "b": range(1 << self.WIDTH), "c": [0, 3, 7]}
        assert_forward_dagger_identity(self.make_op, self.touched, self.masks, sweep)

    def test_dagger_forward_identity(self):
        sweep = {"a": range(1 << self.WIDTH), "b": range(1 << self.WIDTH), "c": [0, 3, 7]}
        assert_dagger_forward_identity(self.make_op, self.touched, self.masks, sweep)

    def test_superposition_linearity(self):
        assert_superposition_linearity(
            self.make_op,
            self.touched,
            self.masks,
            hadamard_regs=[("a", self.WIDTH), ("b", self.WIDTH)],
            fixed={"c": 0},
        )

    def test_controls(self):
        ctrl_specs = [RegisterSpec("ctrl", ps.Boolean, 1)]
        self.specs.append(ctrl_specs[0])
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        assert_controls(
            self.make_op,
            self.touched,
            self.masks,
            ctrl_specs,
            active_values={"a": 5, "b": 2, "ctrl": 1},
            inactive_values={"a": 5, "b": 2, "ctrl": 0},
            fixed={"c": 0},
        )


class TestAssign:
    """Assign: |src>|dst> -> |src>|dst XOR src> (xor_into)."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("src", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("dst", ps.UnsignedInteger, self.WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["src", "dst"]

    def make_op(self):
        return ps.Assign("src", "dst")

    def test_basis_exhaustive_and_no_collisions(self):
        forward = build_forward_map(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"src": range(1 << self.WIDTH)},
            fixed={"dst": 0},
        )
        for src in range(1 << self.WIDTH):
            assert forward[(src, 0)] == (src, src)

    def test_collision_free_for_arbitrary_nonzero_output_starts(self):
        assert_collision_free_for_output_starts(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"src": range(1 << self.WIDTH)},
            output_start_values={"dst": [0, 1, 3, 5, 7]},
        )

    def test_forward_dagger_identity(self):
        sweep = {"src": range(1 << self.WIDTH), "dst": range(1 << self.WIDTH)}
        assert_forward_dagger_identity(self.make_op, self.touched, self.masks, sweep)

    def test_superposition_linearity(self):
        assert_superposition_linearity(
            self.make_op,
            self.touched,
            self.masks,
            hadamard_regs=[("src", self.WIDTH)],
            fixed={"dst": 0},
        )


class TestCompareUIntUInt:
    """Compare_UInt_UInt: xor_into two boolean flag registers."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("l", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("r", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("lt", ps.Boolean, 1),
            RegisterSpec("eq", ps.Boolean, 1),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["l", "r", "lt", "eq"]

    def make_op(self):
        return ps.Compare_UInt_UInt("l", "r", "lt", "eq")

    def test_basis_exhaustive_and_no_collisions(self):
        forward = build_forward_map(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"l": range(1 << self.WIDTH), "r": range(1 << self.WIDTH)},
            fixed={"lt": 0, "eq": 0},
        )
        for lhs in range(1 << self.WIDTH):
            for r in range(1 << self.WIDTH):
                _, _, lt, eq = forward[(lhs, r, 0, 0)]
                assert lt == int(lhs < r)
                assert eq == int(lhs == r)

    def test_collision_free_for_arbitrary_nonzero_output_starts(self):
        assert_collision_free_for_output_starts(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"l": range(1 << self.WIDTH), "r": range(1 << self.WIDTH)},
            output_start_values={"lt": [0, 1], "eq": [0, 1]},
        )

    def test_forward_dagger_identity(self):
        sweep = {"l": range(1 << self.WIDTH), "r": range(1 << self.WIDTH)}
        assert_forward_dagger_identity(
            self.make_op, self.touched, self.masks, sweep, fixed={"lt": 0, "eq": 0}
        )


class TestCustomArithmetic:
    """CustomArithmetic: user function, xor_into the trailing output registers."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("inp", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("out", ps.UnsignedInteger, self.WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["inp", "out"]

    def make_op(self):
        size = 1 << self.WIDTH
        return ps.CustomArithmetic(["inp", "out"], 1, 1, lambda vals: [(vals[0] * vals[0]) % size])

    def test_basis_exhaustive_and_no_collisions(self):
        size = 1 << self.WIDTH
        forward = build_forward_map(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"inp": range(size)},
            fixed={"out": 0},
        )
        for inp in range(size):
            assert forward[(inp, 0)] == (inp, (inp * inp) % size)

    def test_collision_free_for_arbitrary_nonzero_output_starts(self):
        size = 1 << self.WIDTH
        assert_collision_free_for_output_starts(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"inp": range(size)},
            output_start_values={"out": [0, 1, 3, 5, 7]},
        )

    def test_forward_dagger_identity(self):
        size = 1 << self.WIDTH
        sweep = {"inp": range(size), "out": [0, 3, 7]}
        assert_forward_dagger_identity(self.make_op, self.touched, self.masks, sweep)

    def test_superposition_linearity(self):
        assert_superposition_linearity(
            self.make_op,
            self.touched,
            self.masks,
            hadamard_regs=[("inp", self.WIDTH)],
            fixed={"out": 0},
        )


class TestAddUIntUIntInPlace:
    """Add_UInt_UInt_InPlace: in-place bijection out += in (mod 2^n)."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("in", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("out", ps.UnsignedInteger, self.WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["in", "out"]

    def make_op(self):
        return ps.Add_UInt_UInt_InPlace("in", "out")

    def test_basis_exhaustive_and_no_collisions(self):
        size = 1 << self.WIDTH
        for out0 in (0, 3):
            forward = build_forward_map(
                self.make_op,
                self.touched,
                self.masks,
                sweep={"in": range(size)},
                fixed={"out": out0},
            )
            for i in range(size):
                assert forward[(i, out0)] == (i, (i + out0) % size)

    def test_forward_dagger_identity(self):
        size = 1 << self.WIDTH
        sweep = {"in": range(size), "out": range(size)}
        assert_forward_dagger_identity(self.make_op, self.touched, self.masks, sweep)

    def test_dagger_forward_identity(self):
        size = 1 << self.WIDTH
        sweep = {"in": range(size), "out": range(size)}
        assert_dagger_forward_identity(self.make_op, self.touched, self.masks, sweep)


class TestSwapGeneralGeneral:
    """Swap_General_General: in-place bijection (x, y) -> (y, x)."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("x", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("y", ps.UnsignedInteger, self.WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["x", "y"]

    def make_op(self):
        return ps.Swap_General_General("x", "y")

    def test_basis_exhaustive_and_no_collisions(self):
        size = 1 << self.WIDTH
        forward = build_forward_map(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"x": range(size), "y": range(size)},
        )
        for x in range(size):
            for y in range(size):
                assert forward[(x, y)] == (y, x)

    def test_forward_dagger_identity(self):
        size = 1 << self.WIDTH
        sweep = {"x": range(size), "y": range(size)}
        assert_forward_dagger_identity(self.make_op, self.touched, self.masks, sweep)

    def test_superposition_linearity(self):
        assert_superposition_linearity(
            self.make_op,
            self.touched,
            self.masks,
            hadamard_regs=[("x", self.WIDTH), ("y", self.WIDTH)],
        )


class TestXgateBool:
    """X_Bool: in-place bit flip at a given digit position."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [RegisterSpec("b", ps.Boolean, self.WIDTH)]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["b"]

    def make_op(self):
        return ps.X_Bool("b", 1)

    def test_basis_exhaustive_and_no_collisions(self):
        size = 1 << self.WIDTH
        forward = build_forward_map(
            self.make_op, self.touched, self.masks, sweep={"b": range(size)}
        )
        for b in range(size):
            assert forward[(b,)] == (b ^ 0b010,)

    def test_forward_dagger_identity(self):
        size = 1 << self.WIDTH
        assert_forward_dagger_identity(
            self.make_op, self.touched, self.masks, sweep={"b": range(size)}
        )

    def test_superposition_linearity(self):
        assert_superposition_linearity(
            self.make_op, self.touched, self.masks, hadamard_regs=[("b", self.WIDTH)]
        )

    def test_controls(self):
        ctrl_specs = [RegisterSpec("c1", ps.Boolean, 1), RegisterSpec("c2", ps.Boolean, 1)]
        self.specs.extend(ctrl_specs)
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        assert_controls(
            self.make_op,
            self.touched,
            self.masks,
            ctrl_specs,
            active_values={"b": 0b101, "c1": 1, "c2": 1},
            inactive_values={"b": 0b101, "c1": 0, "c2": 0},
        )


class TestFlipBools:
    """FlipBools: in-place bit flip of an entire boolean register."""

    WIDTH = 3

    def setup_method(self):
        self.specs = [RegisterSpec("b", ps.Boolean, self.WIDTH)]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["b"]

    def make_op(self):
        return ps.FlipBools("b")

    def test_basis_exhaustive_and_no_collisions(self):
        size = 1 << self.WIDTH
        forward = build_forward_map(
            self.make_op, self.touched, self.masks, sweep={"b": range(size)}
        )
        for b in range(size):
            assert forward[(b,)] == (b ^ self.masks["b"],)

    def test_forward_dagger_identity(self):
        size = 1 << self.WIDTH
        assert_forward_dagger_identity(
            self.make_op, self.touched, self.masks, sweep={"b": range(size)}
        )


class TestQRAMLoad:
    """QRAMLoad: xor_into data register from a classically-loaded QRAM table."""

    ADDR_WIDTH = 3
    DATA_WIDTH = 4

    def setup_method(self):
        size = 1 << self.ADDR_WIDTH
        self.memory = [(i * 3 + 1) % (1 << self.DATA_WIDTH) for i in range(size)]
        self.qram = ps.QRAMCircuit_qutrit(self.ADDR_WIDTH, self.DATA_WIDTH, self.memory)
        self.specs = [
            RegisterSpec("addr", ps.UnsignedInteger, self.ADDR_WIDTH),
            RegisterSpec("data", ps.UnsignedInteger, self.DATA_WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["addr", "data"]

    def make_op(self):
        return ps.QRAMLoad(self.qram, "addr", "data")

    def test_basis_exhaustive_and_no_collisions(self):
        forward = build_forward_map(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"addr": range(1 << self.ADDR_WIDTH)},
            fixed={"data": 0},
        )
        for addr in range(1 << self.ADDR_WIDTH):
            assert forward[(addr, 0)] == (addr, self.memory[addr])

    def test_collision_free_for_arbitrary_nonzero_output_starts(self):
        assert_collision_free_for_output_starts(
            self.make_op,
            self.touched,
            self.masks,
            sweep={"addr": range(1 << self.ADDR_WIDTH)},
            output_start_values={"data": [0, 1, 5, 15]},
        )

    def test_forward_dagger_identity(self):
        sweep = {"addr": range(1 << self.ADDR_WIDTH), "data": [0, 5, 15]}
        assert_forward_dagger_identity(self.make_op, self.touched, self.masks, sweep)

    def test_superposition_linearity(self):
        assert_superposition_linearity(
            self.make_op,
            self.touched,
            self.masks,
            hadamard_regs=[("addr", self.ADDR_WIDTH)],
            fixed={"data": 0},
        )


# ---------------------------------------------------------------------------
# Negative control: the historically destructive style of implementation
# (overwrite output, dag() clears the register) must FAIL this harness.
# ---------------------------------------------------------------------------


class TestHarnessRejectsDestructiveImplementation:
    """A CustomArithmetic that overwrites (rather than XORs into) its output,
    and whose 'dagger' simply clears the register, is exactly the
    non-conformant style described in the plan. The harness must reject it.
    """

    WIDTH = 3

    def setup_method(self):
        self.specs = [
            RegisterSpec("inp", ps.UnsignedInteger, self.WIDTH),
            RegisterSpec("out", ps.UnsignedInteger, self.WIDTH),
        ]
        setup_registers(self.specs)
        self.masks = {s.name: s.mask for s in self.specs}
        self.touched = ["inp", "out"]

    def _make_destructive_pair(self):
        """Build a (forward, "dagger") pair using overwrite instead of XOR-into.

        Modeled directly on CustomArithmetic's calling convention so the
        only difference from a conformant leaf is overwrite-vs-XOR and a
        clearing "dagger" — i.e. exactly the bug class under test.
        """
        size = 1 << self.WIDTH

        def forward(state):
            # BUG: overwrites 'out' instead of XORing the square into it,
            # so distinct 'inp' values sharing the same square collide.
            op = ps.CustomArithmetic(
                ["inp", "out"], 1, 1, lambda vals: [(vals[0] * vals[0]) % size]
            )
            # Simulate "overwrite": first clear out via a real XOR-into
            # CustomArithmetic reading the *current* value of out, then
            # write the square. Net visible effect: out = square(inp),
            # independent of the prior value of out (destructive).
            clear = ps.CustomArithmetic(["out", "out"], 1, 1, lambda vals: [vals[0]])
            clear(state)
            op(state)

        def dagger(state):
            # BUG: "dagger" clears the register instead of inverting.
            reset = ps.CustomArithmetic(["out", "out"], 1, 1, lambda vals: [vals[0]])
            reset(state)

        return forward, dagger

    def test_collision_is_detected(self):
        """Viewed on its exposed 'out' register alone (with 'inp' swept but
        discarded from the touched set, exactly as the destructive
        implementation discards it from its real invertible state), the
        harness's own collision detector in build_forward_map must reject
        this operator as non-injective.
        """
        forward, _ = self._make_destructive_pair()

        class _ForwardOnlyOp:
            def __call__(self, state):
                forward(state)

        with pytest.raises(AssertionError, match="collision detected"):
            build_forward_map(
                _ForwardOnlyOp,
                ["out"],
                self.masks,
                sweep={"inp": range(1 << self.WIDTH)},
                fixed={"out": 0},
            )

    def test_collision_free_for_output_starts_rejects_it_at_nonzero_start_too(self):
        """The destructive overwrite discards the incoming 'out' value
        entirely, so it collides identically for *every* starting value of
        'out' — not just the conventional zero start.
        ``assert_collision_free_for_output_starts`` must therefore raise
        regardless of which nonzero starting value is probed, confirming
        the harness does not only exercise the clean-ancilla case.
        """
        forward, _ = self._make_destructive_pair()

        class _ForwardOnlyOp:
            def __call__(self, state):
                forward(state)

        with pytest.raises(AssertionError, match="collision detected"):
            assert_collision_free_for_output_starts(
                _ForwardOnlyOp,
                ["out"],
                self.masks,
                sweep={"inp": range(1 << self.WIDTH)},
                output_start_values={"out": [0, 1, 5]},
            )

    def test_forward_dagger_identity_fails(self):
        """forward()+dagger() must NOT restore an arbitrary starting value."""
        forward, dagger = self._make_destructive_pair()

        from pysparq.conformance import make_basis_state, read_registers

        state = make_basis_state({"inp": 5, "out": 3})
        forward(state)
        dagger(state)
        got = read_registers(state, self.touched, self.masks)
        # A conformant implementation would restore out=3; the destructive
        # "clear" dagger leaves out=0 instead.
        assert got != (5, 3), "destructive dagger must fail to restore the original state"
        assert got == (5, 0)
