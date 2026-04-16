# PySparQ Documentation

PySparQ is the Python binding for the QRAM-Simulator's sparse-state quantum circuit simulator. It provides access to the high-performance C++ core through a Pythonic interface.

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Core Concepts](#core-concepts)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Performance Tips](#performance-tips)

## Installation

### From Source

```bash
# Clone the repository
git clone https://github.com/IAI-USTC-Quantum/QRAM-Simulator.git
cd QRAM-Simulator

# Install dependencies
pip install pybind11 numpy

# Build and install
pip install .
```

### Requirements

- Python 3.9+
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
ps.StatePrint()(state)
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

#### `Add_ConstUInt(reg, constant)`
Add constant to register (in-place).

```python
ps.Add_ConstUInt("counter", 1)(state)  # counter += 1
```

#### `Mult_UInt_ConstUInt(input_reg, multiplier, output_reg)`
Multiply by constant (out-of-place): `output ^= input * multiplier`

```python
ps.Mult_UInt_ConstUInt("a", 3, "triple_a")(state)
```

#### `ShiftLeft(reg, shift_bits)` / `ShiftRight(reg, shift_bits)`
Cyclic shift operations.

```python
ps.ShiftLeft("data", 2)(state)   # Left shift by 2 bits
ps.ShiftRight("data", 2)(state)  # Right shift by 2 bits (inverse of left)
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
ps.Xgate_Bool("qubit")(state)           # X gate (bit flip)
ps.Ygate_Bool("qubit")(state)           # Y gate
ps.Zgate_Bool("qubit")(state)           # Z gate (phase flip)

# Phase gates
ps.Sgate_Bool("qubit")(state)           # S gate (phase by i)
ps.Tgate_Bool("qubit")(state)           # T gate (phase by e^(iπ/4))

# Rotation gates
ps.RXgate_Bool("qubit", theta)(state)   # X rotation
ps.RYgate_Bool("qubit", theta)(state)   # Y rotation
ps.RZgate_Bool("qubit", theta)(state)   # Z rotation

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
ps.inverseQFT("reg")(state)

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
# Create QRAM circuit
qram = ps.QRAMCircuit_qutrit(addr_size=4, data_size=8)
qram.set_memory(memory_tree)

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

#### `StatePrint(display_mode=0, precision=0)`
Print the quantum state.

```python
ps.StatePrint()(state)                    # Default
ps.StatePrint(ps.Detail)(state)           # Detailed view
ps.StatePrint(ps.Binary)(state)           # Binary representation
ps.StatePrint(ps.Prob)(state)             # Probability view
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
ps.StatePrint()(state)
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

# Result: y = 13 (if flag was 1), otherwise y = 10
ps.StatePrint()(state)
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
ps.StatePrint(ps.Prob)(state)
```

### Example 4: QRAM Data Loading

```python
import pysparq as ps

ps.System.clear()

# Create QRAM with 4-bit addresses, 8-bit data
qram = ps.QRAMCircuit_qutrit(4, 8)

# Set memory content (simplified)
memory_data = [0] * 16  # 16 addresses
memory_data[5] = 42     # Address 5 contains 42
# ... load memory into qram ...

# Registers
addr = ps.System.add_register("addr", ps.UnsignedInteger, 4)
data = ps.System.add_register("data", ps.UnsignedInteger, 8)

state = ps.SparseState()

# Set address to 5
ps.Init_Unsafe("addr", 5)(state)

# Load data from QRAM
ps.QRAMLoad(qram, "addr", "data")(state)

# Result: data register now contains 42
ps.StatePrint()(state)
```

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
