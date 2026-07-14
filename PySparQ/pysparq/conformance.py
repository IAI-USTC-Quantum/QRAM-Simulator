"""Reusable semantic conformance harness for PySparQ built-in operators.

Stage 1 of the native QECC.Lang / QFVM plan requires every PySparQ primitive
accelerated for use by QECC.Lang / quantum-cfd-software to pass a shared
conformance matrix before it may be relied upon as a leaf semantics for a
reversible ``xor_into`` or ``inplace_bijective`` contract (see
``pysparq.dynamic_operator`` module docstring and README for the contract
vocabulary). This module provides that matrix as small, composable
assertions so individual test files stay short and declarative.

The matrix covers:

* **basis exhaustive / arbitrary nonzero targets** — :func:`build_forward_map`
  runs the operator on every (or a spot-checked sample of) classical basis
  input and records the resulting output tuple;
* **collisions** — :func:`build_forward_map` (and
  :func:`assert_no_collisions`) fail loudly if two distinct inputs map to the
  same output tuple over the *full* set of touched registers. This is
  exactly the bug class described for non-conformant ``compile_operator``
  callbacks that "overwrite outputs" instead of computing a genuine
  bijection / XOR-into map;
* **superposition linearity** — :func:`assert_superposition_linearity`
  builds a *real*, coherent equal-amplitude superposition (via
  ``Hadamard_Int``) over every swept input, applies the operator once, and
  checks that each surviving branch agrees with the basis-by-basis reference
  map with the correct, unchanged amplitude magnitude and branch count. This
  catches implementations that behave correctly in isolation but corrupt or
  cross-talk between branches when several coexist;
* **controls** — :func:`assert_controls` exercises
  ``conditioned_by_nonzeros`` / ``conditioned_by_all_ones`` in the positive
  (condition true), negative (condition false => identity), and
  multi-register conjunction configurations already provided by every
  ``ClassControllable`` C++ operator;
* **forward/dagger identity** — :func:`assert_forward_dagger_identity` and
  :func:`assert_dagger_forward_identity` check that applying the operator
  and its adjoint in either order returns the state to its starting values
  for arbitrary (not just zero) starting register contents.

Everything in this module operates on named registers pre-declared once via
:func:`setup_registers`, then exercised across many independent
``SparseState`` instances — mirroring the ``AddRegister(...)(state)`` /
``Init_Unsafe(...)(state)`` idiom already used throughout
``pysparq/algorithms``.

.. warning::
   ``pysparq.dynamic_operator.compile_operator()`` produces an arbitrary,
   runtime-compiled C++ operator. Compilation only checks that the supplied
   ``operator()``/``dag()`` pair type-checks; it can neither statically nor
   dynamically *prove* the pair is unitary or mutually inverse. This harness
   is therefore intentionally restricted to named, statically inspectable
   PySparQ built-ins (and Python composites built from them) — it cannot
   "bless" a dynamically compiled operator, and dynamically compiled
   operators must not be used on the supported QCFD path (see
   ``pysparq/dynamic_operator/README.md``).
"""

from __future__ import annotations

import itertools
import random
from dataclasses import dataclass
from typing import Callable, Iterable, Mapping, Sequence

import pysparq as ps

__all__ = [
    "RegisterSpec",
    "setup_registers",
    "make_basis_state",
    "read_registers",
    "build_forward_map",
    "assert_collision_free_for_output_starts",
    "assert_no_collisions",
    "assert_forward_dagger_identity",
    "assert_dagger_forward_identity",
    "assert_superposition_linearity",
    "assert_controls",
    "sample_values",
]


@dataclass(frozen=True)
class RegisterSpec:
    """Declares one register participating in a conformance check.

    Attributes:
        name: Register name, used both for ``AddRegister``/``Init_Unsafe``
            and for looking up the register id via ``System.get_id``.
        storage_type: One of ``ps.UnsignedInteger``, ``ps.SignedInteger``,
            ``ps.Boolean``.
        width: Register width in bits (or trits-worth of qubits, per the
            underlying storage type).
    """

    name: str
    storage_type: object
    width: int

    @property
    def mask(self) -> int:
        return (1 << self.width) - 1


def setup_registers(specs: Sequence[RegisterSpec]) -> None:
    """Clear the global ``System`` schema and (re)declare every register.

    Must be called once before creating any ``SparseState`` used by the
    checks in this module. Individual ``SparseState`` instances created
    afterwards do not need to repeat ``AddRegister`` — the fixed-size
    per-branch register array already reserves every declared slot at
    zero, so ``Init_Unsafe`` can be used directly on a fresh state.
    """
    ps.System.clear()
    scratch = ps.SparseState()
    for spec in specs:
        ps.AddRegister(spec.name, spec.storage_type, spec.width)(scratch)


def make_basis_state(values: Mapping[str, int]) -> "ps.SparseState":
    """Build a fresh, single-branch basis state with the given register values.

    Registers not present in ``values`` are left at their default (zero).
    """
    state = ps.SparseState()
    for name, value in values.items():
        ps.Init_Unsafe(name, int(value))(state)
    return state


def read_registers(
    state: "ps.SparseState",
    names: Sequence[str],
    masks: Mapping[str, int],
    branch: int = 0,
) -> tuple:
    """Read the (masked) values of ``names`` from one branch of ``state``."""
    branch_state = state.basis_states[branch]
    return tuple(branch_state.get(ps.System.get_id(name)).value & masks[name] for name in names)


def sample_values(
    width: int, *, max_exhaustive: int = 5, samples: int = 12, rng: random.Random | None = None
) -> list[int]:
    """Return the values to sweep for a register of the given width.

    Widths up to ``max_exhaustive`` bits are covered exhaustively (basis
    exhaustive coverage). Wider registers are spot-checked with
    ``samples`` pseudo-random values (arbitrary nonzero output targets),
    always including 0, 1, and the all-ones value so edge cases are never
    skipped.
    """
    size = 1 << width
    if width <= max_exhaustive:
        return list(range(size))

    rng = rng or random.Random(0)
    values = {0, 1, size - 1}
    while len(values) < min(samples, size):
        values.add(rng.randrange(size))
    return sorted(values)


def _iter_combos(sweep: Mapping[str, Iterable[int]]) -> Iterable[dict]:
    names = list(sweep.keys())
    for combo in itertools.product(*(sweep[n] for n in names)):
        yield dict(zip(names, combo))


def build_forward_map(
    make_op: Callable[[], "ps.BaseOperator"],
    touched: Sequence[str],
    masks: Mapping[str, int],
    sweep: Mapping[str, Iterable[int]],
    fixed: Mapping[str, int] | None = None,
) -> dict[tuple, tuple]:
    """Apply ``make_op()`` to every swept basis input and record the result.

    ``touched`` must list *every* register the operator reads or writes
    (inputs and outputs alike). ``sweep`` gives the register(s) whose value
    is varied; ``fixed`` gives the starting value for any other touched
    register (defaults to 0, i.e. a clean output/work register).

    .. important::
       For an ``xor_into`` leaf (``out_new = out_old XOR f(inputs)``),
       collision-freeness must hold for *every* starting value of the
       output register(s), not only the conventional clean/zero-ancilla
       start. Calling this function once with the output left at its
       default (0) is necessary but not sufficient: an implementation
       that discards/overwrites the incoming output value (rather than
       genuinely XORing into it) can still happen to look collision-free
       when the probe always starts from a clean register. Use
       :func:`assert_collision_free_for_output_starts` to additionally
       sweep arbitrary (including nonzero) starting output values.

    Returns a dict mapping the full input tuple (over ``touched``, in
    order) to the full output tuple. Raises ``AssertionError`` if:

    * the operator turns a basis state into a superposition (a classical
      leaf must map one basis state to exactly one basis state), or
    * two distinct inputs collide onto the same output tuple (the
      destructive-overwrite bug class this harness exists to catch).
    """
    fixed = dict(fixed or {})
    forward: dict[tuple, tuple] = {}
    seen_outputs: dict[tuple, tuple] = {}

    for combo in _iter_combos(sweep):
        values = {**fixed, **combo}
        state = make_basis_state(values)
        make_op()(state)

        assert state.size() == 1, (
            f"operator produced {state.size()} branches from a single basis "
            f"input {values}; a classical/basis-preserving leaf must return "
            "exactly one branch"
        )

        input_tuple = tuple(values.get(name, 0) & masks[name] for name in touched)
        output_tuple = read_registers(state, touched, masks)

        if output_tuple in seen_outputs:
            raise AssertionError(
                f"collision detected: inputs {seen_outputs[output_tuple]} and "
                f"{input_tuple} (registers {touched}) both map to output "
                f"{output_tuple}. This is the non-injective 'overwrites output' "
                "bug class forbidden by the reversibility contract."
            )
        seen_outputs[output_tuple] = input_tuple
        forward[input_tuple] = output_tuple

    return forward


def assert_collision_free_for_output_starts(
    make_op: Callable[[], "ps.BaseOperator"],
    touched: Sequence[str],
    masks: Mapping[str, int],
    sweep: Mapping[str, Iterable[int]],
    output_start_values: Mapping[str, Iterable[int]],
) -> None:
    """Re-check collision-freeness with the output register(s) seeded at
    several arbitrary (including nonzero) starting values.

    For a genuine ``xor_into`` leaf, ``out_new = out_old XOR f(inputs)`` is
    a bijection on ``inputs`` for *any* fixed ``out_old``. Only probing
    with a clean (zero) starting output — as a single
    :func:`build_forward_map` call conventionally does — cannot
    distinguish a real XOR-into implementation from one that discards the
    incoming output value and simply overwrites it with ``f(inputs)``: the
    two agree when ``out_old == 0`` but diverge for any other starting
    value. This function makes that distinction part of the conformance
    matrix by rebuilding the forward map (and therefore re-running
    :func:`build_forward_map`'s collision check) once per combination of
    ``output_start_values``.

    Args:
        output_start_values: one entry per xor_into output register,
            mapping its name to the (ideally including at least one
            nonzero) starting values to probe, e.g. ``{"c": [0, 1, 5, 7]}``.
    """
    names = list(output_start_values.keys())
    for combo in itertools.product(*(output_start_values[n] for n in names)):
        fixed = dict(zip(names, combo))
        build_forward_map(make_op, touched, masks, sweep, fixed=fixed)


def assert_no_collisions(forward_map: Mapping[tuple, tuple]) -> None:
    """Re-check an already-built forward map for output collisions.

    Useful when the map was built once and reused by several assertions.
    """
    seen: dict[tuple, tuple] = {}
    for inp, out in forward_map.items():
        if out in seen:
            raise AssertionError(
                f"collision detected: inputs {seen[out]} and {inp} both map to {out}"
            )
        seen[out] = inp


def assert_forward_dagger_identity(
    make_op: Callable[[], "ps.BaseOperator"],
    touched: Sequence[str],
    masks: Mapping[str, int],
    sweep: Mapping[str, Iterable[int]],
    fixed: Mapping[str, int] | None = None,
) -> None:
    """Check ``op.dag(op(state)) == state`` for every swept input.

    Uses arbitrary (not just zero) starting values for every touched
    register, per the Stage 1 conformance matrix.
    """
    fixed = dict(fixed or {})
    for combo in _iter_combos(sweep):
        values = {**fixed, **combo}
        state = make_basis_state(values)
        op = make_op()
        op(state)
        op.dag(state)
        got = read_registers(state, touched, masks)
        expected = tuple(values.get(name, 0) & masks[name] for name in touched)
        assert (
            got == expected
        ), f"forward+dag != identity for {values}: got {got}, expected {expected}"


def assert_dagger_forward_identity(
    make_op: Callable[[], "ps.BaseOperator"],
    touched: Sequence[str],
    masks: Mapping[str, int],
    sweep: Mapping[str, Iterable[int]],
    fixed: Mapping[str, int] | None = None,
) -> None:
    """Check ``op(op.dag(state)) == state`` for every swept input."""
    fixed = dict(fixed or {})
    for combo in _iter_combos(sweep):
        values = {**fixed, **combo}
        state = make_basis_state(values)
        op = make_op()
        op.dag(state)
        op(state)
        got = read_registers(state, touched, masks)
        expected = tuple(values.get(name, 0) & masks[name] for name in touched)
        assert (
            got == expected
        ), f"dag+forward != identity for {values}: got {got}, expected {expected}"


def assert_superposition_linearity(
    make_op: Callable[[], "ps.BaseOperator"],
    touched: Sequence[str],
    masks: Mapping[str, int],
    hadamard_regs: Sequence[tuple[str, int]],
    fixed: Mapping[str, int] | None = None,
    forward_map: Mapping[tuple, tuple] | None = None,
) -> None:
    """Verify linearity: one coherent call must equal per-branch reference.

    Builds a genuine equal-amplitude superposition over every combination
    of the registers in ``hadamard_regs`` (each entry is ``(name, width)``)
    using ``Hadamard_Int``, applies ``make_op()`` exactly once, and checks
    that the branch count, per-branch amplitude magnitude, and per-branch
    output all agree with ``forward_map`` (built with
    :func:`build_forward_map` over the same sweep/fixed values if not
    supplied).
    """
    fixed = dict(fixed or {})
    sweep = {name: range(1 << width) for name, width in hadamard_regs}

    if forward_map is None:
        forward_map = build_forward_map(make_op, touched, masks, sweep, fixed)

    state = ps.SparseState()
    for name, value in fixed.items():
        ps.Init_Unsafe(name, int(value))(state)
    for name, width in hadamard_regs:
        ps.Hadamard_Int(name, width)(state)

    n_branches_before = 1 << sum(width for _, width in hadamard_regs)
    assert state.size() == n_branches_before

    # Snapshot each branch's *pre*-operation touched-register values before
    # applying the operator. This is required (not merely convenient) when
    # an in-place bijective operator overwrites one of the very registers
    # that was put into superposition (e.g. Swap_General_General,
    # Xgate_Bool) — reading registers only works after the C++ call
    # returns, so the branch order (stable, since the operator must not
    # reorder/merge/split branches for a classical leaf) is used to align
    # each branch's pre-image with its post-image.
    pre_snapshots = [read_registers(state, touched, masks, branch=i) for i in range(state.size())]

    make_op()(state)

    assert state.size() == n_branches_before, (
        f"branch count changed under superposition ({state.size()} != "
        f"{n_branches_before}); expected coherent, non-collapsing evolution"
    )

    expected_amp = 1.0 / (n_branches_before**0.5)
    seen_outputs = set()
    for branch_idx in range(state.size()):
        s = state.basis_states[branch_idx]
        amp_mag = abs(s.amplitude)
        assert (
            abs(amp_mag - expected_amp) < 1e-9
        ), f"branch {branch_idx} amplitude magnitude {amp_mag} != expected {expected_amp}"

        input_tuple = pre_snapshots[branch_idx]
        output_tuple = read_registers(state, touched, masks, branch=branch_idx)

        expected_output = forward_map.get(input_tuple)
        assert (
            expected_output is not None
        ), f"branch input {input_tuple} not present in reference forward map"
        assert output_tuple == expected_output, (
            f"superposition branch mismatch for input {input_tuple}: "
            f"got {output_tuple}, expected {expected_output} (per-basis reference)"
        )
        assert output_tuple not in seen_outputs, (
            f"duplicate output {output_tuple} across branches under superposition "
            "(non-injective under coherent evolution)"
        )
        seen_outputs.add(output_tuple)


def assert_controls(
    make_op: Callable[[], "ps.BaseOperator"],
    touched: Sequence[str],
    masks: Mapping[str, int],
    control_specs: Sequence[RegisterSpec],
    active_values: Mapping[str, int],
    inactive_values: Mapping[str, int],
    fixed: Mapping[str, int] | None = None,
    conditioned_by: str = "nonzeros",
) -> None:
    """Check control-gated behavior: identity when off, action when on.

    Exercises single controls (one register at a time) as well as the
    full multi-register conjunction (all controls active together, and
    each one individually inactive while the others are active — the
    negative-control cases required by the matrix).

    ``conditioned_by`` selects ``conditioned_by_nonzeros`` or
    ``conditioned_by_all_ones``.
    """
    fixed = dict(fixed or {})
    method_name = f"conditioned_by_{conditioned_by}"
    control_names = [spec.name for spec in control_specs]

    def apply_conditioned(op, names):
        method = getattr(op, method_name)
        return method(names)

    # Reference: unconditioned forward map, seeded with the same payload
    # (non-control) register values used by the active-control case, so the
    # "action" output reflects what the operator actually does to `touched`.
    base_values = {**fixed, **{k: v for k, v in active_values.items() if k not in control_names}}
    base_state = make_basis_state(base_values)
    make_op()(base_state)
    action_output = read_registers(base_state, touched, masks)

    # 1. All controls active together => action should be applied.
    values = {**fixed, **active_values}
    state = make_basis_state(values)
    op = apply_conditioned(make_op(), control_names)
    op(state)
    got = read_registers(state, touched, masks)
    assert (
        got == action_output
    ), f"all-controls-active did not apply action: got {got}, expected {action_output}"

    # 2. All controls inactive together => identity (negative control).
    values = {**fixed, **inactive_values}
    identity_output = tuple(values.get(name, 0) & masks[name] for name in touched)
    state = make_basis_state(values)
    op = apply_conditioned(make_op(), control_names)
    op(state)
    got = read_registers(state, touched, masks)
    assert (
        got == identity_output
    ), f"all-controls-inactive did not preserve identity: got {got}, expected {identity_output}"

    # 3. Each control individually active (single-control case), the rest
    #    left at their inactive value => action should NOT apply (all
    #    controls must be satisfied for a conjunctive gate).
    if len(control_names) > 1:
        for name in control_names:
            values = {**fixed, **inactive_values, name: active_values[name]}
            state = make_basis_state(values)
            op = apply_conditioned(make_op(), control_names)
            op(state)
            got = read_registers(state, touched, masks)
            assert got == tuple(
                values.get(n, 0) & masks[n] for n in touched
            ), f"single active control {name} (others inactive) incorrectly applied action"

    # 4. Single-control positive case (redundant with (1) when there is
    #    exactly one control, but explicit for clarity/coverage).
    for name in control_names:
        op_single = getattr(make_op(), method_name)(name)
        values = {
            **fixed,
            **{k: v for k, v in active_values.items() if k not in control_names},
            name: active_values[name],
        }
        state = make_basis_state(values)
        op_single(state)
        got = read_registers(state, touched, masks)
        if len(control_names) == 1:
            assert (
                got == action_output
            ), f"single control {name} active did not apply action: got {got}"
