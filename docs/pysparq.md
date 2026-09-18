# PySparQ Documentation

PySparQ is the Python binding for the QRAM-Simulator's sparse-state quantum circuit simulator. It provides access to the high-performance C++ core through a Pythonic interface.

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Core Concepts](#core-concepts)
- [API Reference](#api-reference)
- [Examples](#examples)
- [RIR Execution](#rir-execution)
- [Performance Tips](#performance-tips)

## Installation

### From Source

```bash
# Clone the repository
git clone git@git.chenzhaoyun.com:agony/QRAM-Simulator.git
cd QRAM-Simulator

# Install dependencies
pip install pybind11 numpy

# Build and install
pip install .
```

### Requirements

- Python 3.10+
- C++17 compatible compiler
- CMake 3.20+
- pybind11

## Quick Start

```python
import pysparq as ps

# Clear the system (important before starting)
ps.System.clear()

# Create a sparse quantum state
state = ps.SparseState()

# Add registers
reg_a = ps.System.add_register("a", ps.UnsignedInteger, 4)
reg_b = ps.System.add_register("b", ps.UnsignedInteger, 4)
reg_result = ps.System.add_register("result", ps.UnsignedInteger, 4)

# Initialize register values
ps.Init_Unsafe("a", 5)(state)
ps.Init_Unsafe("b", 3)(state)

# Perform arithmetic: result = a + b
ps.Add_UInt_UInt("a", "b", "result")(state)

# Print state
ps.pprint(state)
# 输出：
# StatePrint (mode=Detail)
# |(0)a : UInt4 | |(1)b : UInt4 | |(2)result : UInt4 |
# 1.000000+0.000000i  a=|5> b=|3> result=|8>
```

## Core Concepts

### Register Level Programming

Unlike traditional quantum simulators that operate on individual qubits, PySparQ uses a "Register Level Programming" paradigm:

- **Registers** are named collections of qubits with a specific type
- **Types**: `UnsignedInteger`, `SignedInteger`, `Boolean`, `Rational`, `General`
- Operations are performed on entire registers
- The C++ core automatically decomposes register operations into gates

### Register Types

```python
# Unsigned Integer: for non-negative integers
reg = ps.System.add_register("counter", ps.UnsignedInteger, 8)  # 0 to 255

# Signed Integer: for signed integers (two's complement)
reg = ps.System.add_register("offset", ps.SignedInteger, 8)  # -128 to 127

# Boolean: single qubit
reg = ps.System.add_register("flag", ps.Boolean, 1)

# Rational: for floating-point values (encoded as fixed-point)
reg = ps.System.add_register("angle", ps.Rational, 16)
```

### SparseState

`SparseState` represents a quantum state using sparse storage:

```python
# Create an empty state (|0...0⟩)
state = ps.SparseState()

# Access basis states
for system in state.basis_states:
    print(f"Amplitude: {system.amplitude}")
    print(f"Register value: {system.get(reg_id).value}")

# Check state size (number of non-zero basis states)
print(state.size())
```

### Operators

All quantum operations are implemented as operator objects:

```python
# Create operator
add_op = ps.Add_UInt_UInt("a", "b", "result")

# Apply to state
add_op(state)

# For non-self-adjoint operators, use dagger (inverse)
add_op.dag(state)  # Undo the addition
```

## API Reference

### System Management

#### `System.clear()`
Clear all register definitions and system state. **Always call this before starting a new computation.**

```python
ps.System.clear()
```

#### `System.add_register(name, type, size)`
Add a new register to the system.

**Parameters:**
- `name` (str): Register name
- `type` (StateStorageType): Register type
- `size` (int): Number of bits

**Returns:** Register ID (int)

```python
reg_id = ps.System.add_register("my_reg", ps.UnsignedInteger, 8)
```

#### `System.remove_register(name_or_id)`
Remove a register from the system.

```python
ps.System.remove_register("my_reg")
# or
ps.System.remove_register(reg_id)
```

#### `System.get_qubit_count()`
Get total number of qubits in all active registers.

```python
n_qubits = ps.System.get_qubit_count()
```

### Arithmetic Operators

#### `Add_UInt_UInt(lhs, rhs, result)`
Out-of-place unsigned integer addition: `result ^= lhs + rhs`

```python
ps.Add_UInt_UInt("a", "b", "sum")(state)
```

#### `Add_UInt_UInt_InPlace(lhs, rhs)`
In-place unsigned integer addition: `rhs += lhs (mod 2^N)`

```python
ps.Add_UInt_UInt_InPlace("a", "b")(state)
# b now contains a + b

# To undo:
ps.Add_UInt_UInt_InPlace("a", "b").dag(state)
```

#### `Add_ConstUInt_InPlace(reg, constant)`
Add constant to register (in-place).

```python
ps.Add_ConstUInt_InPlace("counter", 1)(state)  # counter += 1
```

#### `Mult_UInt_ConstUInt(input_reg, multiplier, output_reg)`
Multiply by constant (out-of-place): `output ^= input * multiplier`

```python
ps.Mult_UInt_ConstUInt("a", 3, "triple_a")(state)
```

#### `ShiftLeft_InPlace(reg, shift_bits)` / `ShiftRight_InPlace(reg, shift_bits)`
Cyclic shift operations. They are each other's dagger.

```python
ps.ShiftLeft_InPlace("data", 2)(state)   # Left shift by 2 bits
ps.ShiftRight_InPlace("data", 2)(state)  # Right shift by 2 bits (inverse of left)
ps.ShiftLeft_InPlace("data", 2).dag(state)  # Use .dag() to undo
```

#### `FlipBools(reg)`
Flip all bits in a register.

```python
ps.FlipBools("flags")(state)
```

#### `Assign(src, dst)`
Copy register value (via XOR): `dst ^= src`

```python
# dst = src (if dst was initially 0)
ps.Assign("src", "dst")(state)
```

### Comparison Operators

#### `Compare_UInt_UInt(left, right, less_flag, equal_flag)`
Compare two unsigned integers.

```python
ps.Compare_UInt_UInt("a", "b", "a_less_than_b", "a_equals_b")(state)
```

#### `Less_UInt_UInt(left, right, less_flag)`
Check if left < right.

```python
ps.Less_UInt_UInt("a", "b", "a_is_less")(state)
```

#### `GetMid_UInt_UInt(left, right, mid)`
Compute midpoint: `mid ^= (left + right) / 2`

```python
ps.GetMid_UInt_UInt("low", "high", "mid")(state)
```

### Quantum Gates

#### Single-Qubit Gates

```python
# Pauli gates
ps.X_Bool("qubit", 0)(state)        # X gate (bit flip) - digit parameter required
ps.Y_Bool("qubit")(state)           # Y gate - digit defaults to 0
ps.Z_Bool("qubit")(state)           # Z gate (phase flip) - digit defaults to 0

# Phase gates
ps.S_Bool("qubit")(state)           # S gate (phase by i)
ps.T_Bool("qubit")(state)           # T gate (phase by e^(iπ/4))

# Rotation gates
ps.RX_Bool("qubit", theta)(state)   # X rotation
ps.RY_Bool("qubit", theta)(state)   # Y rotation
ps.RZ_Bool("qubit", theta)(state)   # Z rotation

# General unitary
matrix = [[a, b], [c, d]]  # 2x2 unitary matrix
ps.Rot_GeneralUnitary("qubit", matrix)(state)
```

#### Multi-Qubit Gates

```python
# Hadamard on all qubits of a register
ps.Hadamard_Int_Full("reg")(state)

# Hadamard on partial register
ps.Hadamard_Int("reg", n_digits)(state)

# QFT
ps.QFT("reg")(state)
ps.InverseQFT("reg")(state)

# Reflection (diffusion) operator
ps.Reflection_Bool("reg")(state)
```

### Conditional Operations

Many operators support conditional execution:

```python
# Create a controlled operation
add_op = ps.Add_UInt_UInt("a", "b", "result")

# Apply only when condition register is non-zero
add_op.conditioned_by_nonzeros("control_reg")(state)

# Apply when register has specific bit set
add_op.conditioned_by_bit("flag_reg", 0)(state)

# Apply when register equals specific value
add_op.conditioned_by_value("mode_reg", 1)(state)
```

### QRAM Operations

```python
# Create QRAM circuit with memory data
# Note: memory must be passed at construction time
memory_tree = [0, 1, 2, 3, ...]  # your memory data
qram = ps.QRAMCircuit_qutrit(addr_size=4, data_size=8, memory=memory_tree)

# Load data from QRAM
ps.QRAMLoad(qram, "addr_reg", "data_reg")(state)

# Fast QRAM load (for certain memory patterns)
ps.QRAMLoadFast(qram, "addr_reg", "data_reg")(state)
```

### State Manipulation

#### `Init_Unsafe(reg, value)`
Initialize a register to a specific value (not unitary, for testing only).

```python
ps.Init_Unsafe("counter", 42)(state)
```

#### `ClearZero(epsilon=1e-10)`
Remove basis states with amplitude smaller than epsilon.

```python
ps.ClearZero()(state)  # Remove near-zero amplitudes
```

#### `Normalize()`
Renormalize the state.

```python
ps.Normalize()(state)
```

#### Partial Trace

```python
# Trace out a register
result = ps.PartialTrace(["reg_to_trace"])(state)

# Select specific values
prob = ps.PartialTraceSelect({"reg_a": 0, "reg_b": 1})(state)
```

### State Inspection

#### `StatePrint(state, mode=1, precision=0)` / `ps.pprint(state, mode=1)`
Print or return a formatted string representation of the quantum state.

- ``ps.StatePrint(state)`` returns a string (Detail mode by default)
- ``ps.pprint(state)`` prints to stdout (Detail mode by default)

```python
ps.pprint(state)                                              # Detail → stdout
ps.StatePrint(state)                                         # Detail → string
ps.StatePrint(state, mode=ps.StatePrintDisplay.Default)       # Default → string
ps.StatePrint(state, mode=ps.StatePrintDisplay.Binary)       # Binary → string
ps.pprint(state, mode=ps.StatePrintDisplay.Prob)               # Prob → stdout
```

#### `CheckNormalization(threshold=1e-5)`
Verify state is normalized.

```python
ps.CheckNormalization()(state)
```

#### `CheckNan()`
Check for NaN values in amplitudes.

```python
ps.CheckNan()(state)
```

## Examples

### Example 1: Simple Arithmetic

```python
import pysparq as ps

ps.System.clear()

# Create registers
a = ps.System.add_register("a", ps.UnsignedInteger, 4)
b = ps.System.add_register("b", ps.UnsignedInteger, 4)
sum_reg = ps.System.add_register("sum", ps.UnsignedInteger, 4)

# Create state
state = ps.SparseState()

# Initialize values
ps.Init_Unsafe("a", 7)(state)
ps.Init_Unsafe("b", 5)(state)

# Compute sum
ps.Add_UInt_UInt("a", "b", "sum")(state)

# Result: sum = 7 + 5 = 12
ps.pprint(state)  # Detail mode
# 输出：
# StatePrint (mode=Detail)
# |(0)a : UInt4 | |(1)b : UInt4 | |(2)sum : UInt4 |
# 1.000000+0.000000i  a=|7> b=|5> sum=|12>
```

### Example 2: Conditional Operation

```python
import pysparq as ps

ps.System.clear()

# Registers
x = ps.System.add_register("x", ps.UnsignedInteger, 4)
y = ps.System.add_register("y", ps.UnsignedInteger, 4)
flag = ps.System.add_register("flag", ps.Boolean, 1)

state = ps.SparseState()
ps.Init_Unsafe("x", 3)(state)
ps.Init_Unsafe("y", 10)(state)

# Set flag (for demonstration)
ps.Init_Unsafe("flag", 1)(state)

# Add x to y only when flag is set
ps.Add_UInt_UInt_InPlace("x", "y").conditioned_by_nonzeros("flag")(state)

# Result: y = 13 (flag=1 时触发条件加法：y = y + x = 10 + 3)
ps.pprint(state)
# 输出：
# StatePrint (mode=Detail)
# |(0)x : UInt4 | |(1)y : UInt4 | |(2)flag : Bool1 |
# 1.000000+0.000000i  x=|3> y=|13> flag=|true>
```

### Example 3: Grover-like Search

```python
import pysparq as ps
import math

ps.System.clear()

# Create search register
n_qubits = 4
search_reg = ps.System.add_register("search", ps.UnsignedInteger, n_qubits)

# Initialize uniform superposition
state = ps.SparseState()
ps.AddRegisterWithHadamard("search", ps.UnsignedInteger, n_qubits)(state)

# Apply Grover iterations
n_iterations = int(math.pi / 4 * math.sqrt(2**n_qubits))

for _ in range(n_iterations):
    # Oracle: mark target state (e.g., |5⟩)
    # This is a simplified example
    ps.Reflection_Bool("search")(state)

# Measure
ps.pprint(state, mode=ps.StatePrintDisplay.Prob)
# 输出：
# StatePrint (mode=Prob)
# 1.000000+0.000000i (p = 1) |0>
# （简化示例，Reflection_Bool 仅作演示用；实际 Grover Oracle 需要 QRAMLoad）
```

### Example 4: QRAM Data Loading

```python
import pysparq as ps

ps.System.clear()

# Create QRAM with 4-bit addresses, 8-bit data
# Note: memory must be passed at construction time
memory_data = [0] * 16  # 16 addresses
memory_data[5] = 42     # Address 5 contains 42
qram = ps.QRAMCircuit_qutrit(4, 8, memory_data)

# Registers
addr = ps.System.add_register("addr", ps.UnsignedInteger, 4)
data = ps.System.add_register("data", ps.UnsignedInteger, 8)

state = ps.SparseState()

# Set address to 5
ps.Init_Unsafe("addr", 5)(state)

# Load data from QRAM
ps.QRAMLoad(qram, "addr", "data")(state)

# Result: data register now contains 42
ps.pprint(state)
# 输出：
# StatePrint (mode=Detail)
# |(0)addr : UInt4 | |(1)data : UInt8 |
# 1.000000+0.000000i  addr=|5> data=|42>
```

## RIR Execution

PySparQ is the **natural interpreter for RIR** — the register-level intermediate
representation of the *QECC.Lang* quantum language (the `pyqecclang` package).
RIR is itself register-level: its JSON documents declare typed named registers
(`bits` / `uint` / `sint` / `rational` with widths), QRAM resources, and a module
graph mixing gate-level primitives with structured control nodes (`Call` /
`Repeat` / `Control` / `Adjoint`). That is exactly the abstraction level PySparQ
natively operates on, so RIR needs no separate lowering pass:
`pysparq.run_rir` interprets the document directly on a `SparseState`.

### Why It Is a Natural Fit

| RIR concept | PySparQ native counterpart |
|---|---|
| kinds `bits`/`uint`/`sint`/`rational` | `StateStorageType.General`/`UnsignedInteger`/`SignedInteger`/`Rational` |
| `add_const` on a whole register | `Add_ConstUInt_InPlace` (one native register operation) |
| uncontrolled `gphase` | `GlobalPhase` |
| controlled `gphase` | `Phase_Bool` with remaining control bits |
| gate broadcast on a view | `Rot_Bool` per bit |
| `xor` / `swap` views | controlled `X_Bool` chains |
| QRAM resource + `Load` | `QRAMCircuit_qutrit` + `QRAMLoad` |
| `Control` | multi-bit `conditioned_by_bit` conditions |
| `Call` / `Repeat` / `Adjoint` | inlined / replayed / inverted at interpretation time |

The module graph is expanded **at interpretation time**: calls are inlined
through register renaming, `Repeat` bodies are replayed, `Adjoint` walks the body
backwards with inverted operations, and nested `Control` conditions accumulate
into multi-bit coherent conditions. Whenever an RIR operand aligns with a whole
register, the interpreter dispatches the native register-level operator instead
of decomposing into gate chains; register *slices* fall back to bit-level
decomposition with wraparound at the view width, as the RIR spec requires.

### API

```python
import pysparq as ps

result = ps.run_rir(document, memory)  # default budgets
result = ps.run_rir(document, memory, max_steps=500_000, max_states=4_096)
result = ps.run_rir_file("program.rir.json", memory={"rom": [1, 2, 4, 7]})
document = ps.load_rir("program.rir.json")  # dict, JSON string, or file path
```

- `document` — decoded mapping, JSON string, or path; schema versions `0.1`–`0.3`.
- `memory` — QRAM contents keyed by resource name; each value is a word sequence
  or a sparse `{address: word}` mapping.
- `max_steps` / `max_states` — expansion and sparse-state budgets; exceeding
  either raises `RIRError` instead of truncating silently.

The returned `RIRResult` provides `registers` (entry-module `(name, width)`
pairs in declaration order), `amplitudes` (mapping from tuples of per-register
integer values to complex amplitudes) and `statevector()` (dense little-endian
vector, `index = value(r0) + (value(r1) << width(r0)) + ...`).

### Example: Bell State

With small helpers mirroring the RIR JSON encoding (see
`PySparQ/test/test_rir.py`):

```python
import pysparq as ps

def ref(register, start, width, kind="uint"):
    return {"tag": "Ref",
            "parts": [{"tag": "Span", "register": register, "start": start, "width": width}],
            "type": {"tag": "RegType", "kind": kind, "width": width}}

def prim(op, operands, angle=None, value=None):
    return {"tag": "Primitive", "op": op, "operands": operands, "angle": angle, "value": value}

def module(name, registers, body, locals_=None, resources=None):
    return {"tag": "Module", "name": name, "registers": registers,
            "locals": locals_ or [], "resources": resources or [],
            "body": body, "attributes": []}

def program(entry, modules):
    return {"tag": "Program", "entry": entry, "modules": modules, "version": "0.3"}

bell = program("main", [module(
    "main",
    [{"tag": "Register", "name": "a", "type": {"tag": "RegType", "kind": "bits", "width": 1}},
     {"tag": "Register", "name": "b", "type": {"tag": "RegType", "kind": "bits", "width": 1}}],
    [
        prim("h", [ref("a", 0, 1, "bits")]),
        {"tag": "Control", "register": ref("a", 0, 1, "bits"), "value": 1,
         "body": [prim("x", [ref("b", 0, 1, "bits")])]},
    ],
)])

result = ps.run_rir(bell)
print(result.amplitudes)
# {(0, 0): (0.7071067811865476+0j), (1, 1): (0.7071067811865476+0j)}
```

### Example: Module Graph with Call / Repeat / Adjoint

```python
def register(name, width, kind="uint"):
    return {"tag": "Register", "name": name, "type": {"tag": "RegType", "kind": kind, "width": width}}

def call(module_name, arguments, resources=()):
    return {"tag": "Call", "module": module_name, "arguments": arguments, "resources": list(resources)}

inc = module("inc", [register("x", 4)], [prim("add_const", [ref("x", 0, 4)], value=1)])
main = module(
    "main",
    [register("x", 4)],
    [
        {"tag": "Repeat", "count": 3, "body": [call("inc", [ref("x", 0, 4)])]},
        {"tag": "Adjoint", "body": [call("inc", [ref("x", 0, 4)])]},
    ],
)

result = ps.run_rir(program("main", [main, inc]))
print(result.amplitudes)
# {(2,): (1+0j)}   — three increments, then one adjoint decrement
```

The whole-register `add_const` is dispatched as a single native
`Add_ConstUInt_InPlace`; the adjoint negates the constant modulo `2^width`.

### Example: QRAM Loading

```python
values = {"tag": "Resource", "name": "values",
          "type": {"tag": "QRAM", "address_width": 2, "data_width": 3}}

main = module(
    "main",
    [register("address", 2), register("data", 3)],
    [
        prim("h", [ref("address", 0, 2)]),
        {"tag": "Load", "resource": "values",
         "address": ref("address", 0, 2), "data": ref("data", 0, 3)},
    ],
    resources=[values],
)

result = ps.run_rir(program("main", [main]), memory={"values": [1, 2, 4, 7]})
print(result.amplitudes)
# each branch ≈ 0.5:
# {(0, 1): (0.5+0j), (1, 2): (0.5+0j), (2, 4): (0.5+0j), (3, 7): (0.5+0j)}
```

### Safety Properties

- `max_steps` / `max_states` budgets raise `RIRError` instead of truncating.
- QRAMs wider than `2^20` cells are refused; `memory` must match the declared
  resources exactly, with range-checked addresses and words.
- `Module.locals` workspaces must be uncomputed at exit (runtime check).
- `run_rir` requires an empty global register table and always finishes with
  `System.clear()`.

### Cross-Validation with pyqecclang

The interpreter consumes only the JSON document — it never imports
`pyqecclang`. Together with `pyqecclang`'s own event-based adapter
(`run_pysparq`) and its JSON path (`run_pysparq_rir`, which calls
`pysparq.run_rir`), the two implementations serve as independent cross-checks
for each other. Regression tests live in `PySparQ/test/test_rir.py`.

## Performance Tips

1. **Always call `System.clear()`** before starting a new computation to avoid register conflicts.

2. **Use appropriate register sizes**: Don't use 32-bit registers when 8 bits suffice. Smaller registers mean faster simulation.

3. **Clear zero amplitudes**: Periodically call `ClearZero()` to remove negligible amplitudes and reduce memory usage.

4. **Use in-place operations when possible**: `Add_UInt_UInt_InPlace` is more memory-efficient than `Add_UInt_UInt`.

5. **Batch operations**: Apply multiple operators before checking state to reduce overhead.

6. **Use QRAM for large classical data**: Instead of encoding classical data in quantum gates, use QRAM operations.

## Differences from C++ API

While PySparQ closely mirrors the C++ API, there are some Python-specific adaptations:

- **Strings vs IDs**: Python API accepts both register names (str) and IDs (int) where applicable
- **Lists**: Python lists can be used where C++ uses `std::vector`
- **Dicts**: Python dicts can be used where C++ uses `std::map`
- **Lambda functions**: Python lambdas can be passed for custom operations

## See Also

- [C++ Operators Documentation](operators.md)
- [SparQ Paper](https://arxiv.org/abs/2503.15118)
- [QRAM Paper](https://arxiv.org/abs/2503.13832)
