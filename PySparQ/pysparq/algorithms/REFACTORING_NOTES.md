# PySparQ Algorithm Refactoring Notes

## What changed

### 0. QDA/CKS primitive-level parity

The Python QDA and CKS paths are intended to mirror the C++ algorithms by
composing low-level PySparQ primitives.  They should not bypass the Python
implementation by binding an entire C++ solver.

For QDA, `WalkS` combines Python-side block encoding, state preparation,
reflections, controlled rotations, and global phase operations in the same
register layout used by the C++ `Walk_s_Tridiagonal` and `Walk_s_via_QRAM`
families.  The active integration tests now run both primitive compositions
and verify that `walk` followed by `walk.dag` restores the full sparse state.

For CKS, the Python implementation uses the C++ `SparseMatrix` QRAM layout and
binds only the missing primitive operations needed to reproduce the walk:
`CondRot_General_Bool_QW`, `QuantumBinarySearchFast`, `GetRowAddr`, and
`GetDataAddr`.  `TOperator`, `CKS_apply_walk_step`, and the LCU loop are still
assembled in Python.

Solver-level result extraction is not complete.  `qda_solve()` raises after
the quantum walk sequence because measurement/post-selection readout is not
implemented.  `cks_solve_v2()` runs the quantum walk/LCU path, emits a warning,
and currently returns `np.linalg.solve` only as an explicit placeholder for the
unimplemented measurement readout.

### 1. `ControllableOperatorMixin` (`pysparq/operators/condition_mixin.py`)

Sixteen classes across five algorithm files each duplicated the same four-method
condition interface verbatim:

```python
self._condition_regs: list[str | int] = []
self._condition_bits: list[tuple[str | int, int]] = []

def conditioned_by_nonzeros(self, cond) → Self: ...
def conditioned_by_all_ones(self, cond) → Self: ...
def conditioned_by_bit(self, reg, pos) → Self: ...
def clear_conditions(self) → Self: ...
```

These were extracted into `ControllableOperatorMixin` (a mixin class, not an ABC),
placed in `pysparq/operators/condition_mixin.py`, and all 16 classes now inherit
from it.  Each subclass only needs to implement `__call__(state)` and optionally
`dag(state)`.

The mixin uses Python's double-underscore name mangling (`__condition_regs` becomes
`_ControllableOperatorMixin__condition_regs`) so that subclass instances do not
accidentally clash with any future private names.  Public read-only accessors are
provided: `condition_regs` and `condition_bits` properties.

### 2. Functional v2 APIs

Two new entry points were added that avoid global mutable state and do not call
`ps.System.clear()` internally — the caller manages the quantum system lifecycle.

#### `cks_solve_v2(A, b, kappa=None, eps=1e-3, data_size=32) -> np.ndarray`

Replaces `LCUContainer` and `QuantumWalkNSteps`, which held mutable
`current_state`/`step_state` attributes.  The new function passes state as a
local variable through three helpers:

- `CKS_build_walk_environment(mat)` → `(qram, addr_size, nnz_col, n_row)`
- `CKS_init_walk_state(qram, addr_size, data_size, b_normalized)` → `SparseState`
- `CKS_apply_walk_step(...)` → `SparseState` (mutates in-place, returns same object)
- `CKS_run_lcu_loop(...)` → `SparseState`

Currently runs the primitive walk/LCU path, then warns and returns
`np.linalg.solve` as a placeholder because measurement-based solution
extraction is not yet implemented.  The NOTE below explains why.

#### `make_tree_and_qram(distribution, *, data_size=8, rational_size=None, work_reg=None) -> (StatePrepViaQRAM, QRAMCircuit_qutrit, str)`

Replaces `StatePreparation.run()`, which called `ps.System.clear()` as a global
side effect.  The new function:

- Accepts a float distribution (converts to integers internally with proper scaling)
- Always registers `work_reg` in the quantum system before constructing the operator
- Returns `(operator, qram, work_reg_name)` — caller manages the state lifecycle

`prepare_state_v2` is a public alias for discoverability.

### 3. Deprecations

| Old API | Replacement | Notes |
|---|---|---|
| `StatePreparation.run()` | `make_tree_and_qram()` | Emits `DeprecationWarning` |
| `cks_solve(A, b, ...)` | `cks_solve_v2(A, b, ...)` | Old API unchanged for now; v2 is stable |

---

## NOTE: Why the classical-simulation LCU fallback

The quantum CKS circuit uses LCU to apply `H = Σ_j c_j W^j` where `W` is one
quantum-walk step and `c_j` are Chebyshev coefficients.  The pure quantum circuit
applies each `W^j` as a separate LCU branch.  Re-running `W^j` from scratch for
every `j` would be exponentially expensive in classical simulation — which is why
the original C++ implementation uses caching via `LCUContainer.current_state` and
`QuantumWalkNSteps.step_state`.

`cks_solve_v2` follows the same indirect approach (state as a local variable,
classical accumulation) for consistency with the C++ simulation contract.
In the future, a full CKS circuit should be implemented that applies each `W^j`
via a separate LCU branch — at that point the classical fallback can be replaced
with proper quantum post-processing.

---

## pybind11 quirks discovered during refactoring

### `ps.AddRegister()` returns an integer ID, not a string name

```python
# WRONG — id_ is an integer, not a register name
id_ = ps.AddRegister("my_reg", ps.UnsignedInteger, 4)
ps.Hadamard_Int(id_, state)  # TypeError

# CORRECT — use the string name directly
ps.AddRegister("my_reg", ps.UnsignedInteger, 4)
ps.Hadamard_Int("my_reg", state)
```

### `QRAMCircuit_qutrit` does not expose `address_size` / `data_size` as Python attributes

These must be passed as explicit constructor parameters to any class that needs them:

```python
# WRONG — AttributeError
class Foo:
    def __init__(self, qram):
        self.addr_size = qram.address_size  # not accessible

# CORRECT
class Foo:
    def __init__(self, qram, addr_size, data_size):
        self.addr_size = addr_size
        self.data_size = data_size
```

### `ps.System.size_of(reg_name)` crashes if the register does not exist

Always call `ps.System.add_register(...)` before constructing an operator that
calls `ps.System.size_of()` internally.  If a custom `work_reg` is passed to
`make_tree_and_qram`, it is still registered (this was a bug fixed after the
initial v2 release).

### `SparseState` cannot be deep-copied (pybind11 limitation)

Functions that appear to return a new state (e.g. `CKS_apply_walk_step`) mutate
the input state in-place and return the same object.  This matches the C++
contract.  Tests that need to inspect intermediate state must take a snapshot
before the next mutation.

### Pre-existing `RemoveRegister` bug in `QuantumBinarySearch._find_column_position`

`InverseConditionalPhaseFlip` on `row_addr` is not fully uncomputed during the
binary search loop, causing `RemoveRegister` to raise `RuntimeError` with the
message "RemoveRegister failed (row_addr not zero)".  This is a pre-existing C++
bug, not introduced by the Python refactoring.  Tests wrap affected calls in
`try/except RuntimeError` blocks and assert that the state was mutated before
the error fires.
