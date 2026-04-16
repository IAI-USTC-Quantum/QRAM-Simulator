# QRAM-Simulator Examples

This directory contains example programs demonstrating the usage of QRAM-Simulator and the SparQ sparse-state quantum simulator.

## Directory Structure

```
examples/
├── README.md              # This file
├── hello_qram.cpp         # Basic QRAM usage example
├── basic_gates.cpp        # Quantum gates demonstration
└── grover_search.cpp      # Grover's search algorithm with QRAM
```

## Building the Examples

The examples are regular C++ programs that can be built using the project's CMake configuration.

### Option 1: Add to CMake (Recommended)

Add the following to the main `CMakeLists.txt` to build examples:

```cmake
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

Create `examples/CMakeLists.txt`:

```cmake
# Hello QRAM example
add_executable(hello_qram hello_qram.cpp)
target_link_libraries(hello_qram SparQ QRAM)

# Basic gates example
add_executable(basic_gates basic_gates.cpp)
target_link_libraries(basic_gates SparQ QRAM SparQ_Algorithm)

# Grover search example
add_executable(grover_search grover_search.cpp)
target_link_libraries(grover_search SparQ QRAM SparQ_Algorithm)
```

### Option 2: Manual Compilation

If you have already built the project, you can compile examples manually:

```bash
cd /path/to/QRAM-Simulator

# Compile hello_qram
g++ -std=c++17 -I./SparQ/include -I./QRAM/include -I./Common/include \
    -I./SparQ_Algorithm/include -I./ThirdParty/fmt/include \
    -I./ThirdParty/eigen-3.4.0 -I./ThirdParty/argparse \
    examples/hello_qram.cpp \
    -L./build/lib -lSparQ -lQRAM -lCommon -lfmt \
    -fopenmp -o hello_qram

# Run
./hello_qram
```

## Example Descriptions

### 1. hello_qram.cpp

**Purpose**: Introduction to basic QRAM operations

**Key Concepts**:
- Creating QRAM circuits with address and data sizes
- Initializing QRAM memory
- Using `QRAMLoad` to load data into quantum registers
- Creating address superpositions with Hadamard gates

**Run**:
```bash
./hello_qram
```

**Expected Output**:
- Shows initial quantum state
- Shows state after Hadamard (superposition)
- Shows state after QRAM load
- Verification that data matches memory contents

---

### 2. basic_gates.cpp

**Purpose**: Demonstration of quantum gates and operations

**Key Concepts**:
- Single-qubit gates: X, Y, Z, H, S, T, RX, RY, RZ
- Multi-qubit gates: CNOT, SWAP
- Creating Bell states
- Integer register operations
- Controlled operations (by value, by bit, all-ones)
- Integration of gates with QRAM

**Functions Demonstrated**:
- `Xgate_Bool`, `Ygate_Bool`, `Zgate_Bool`
- `Hadamard_Bool`, `Hadamard_Int_Full`
- `RXgate_Bool`, `RYgate_Bool`, `RZgate_Bool`
- `conditioned_by_all_ones()`, `conditioned_by_value()`, `conditioned_by_bit()`

**Run**:
```bash
./basic_gates
```

---

### 3. grover_search.cpp

**Purpose**: Implementation of Grover's search algorithm using QRAM

**Key Concepts**:
- Unstructured search with quadratic speedup
- Oracle construction for marking solutions
- Diffusion operator (amplitude amplification)
- Optimal iteration count: π/4 × √N

**Algorithm**:
1. Initialize address register in uniform superposition
2. For k iterations:
   - Apply Oracle (mark solution with phase flip)
   - Apply Diffusion (amplify marked state's amplitude)
3. Measure to obtain solution with high probability

**Usage**:
```bash
# Use default parameters
./grover_search

# Specify target value and iterations
./grover_search 7 3
```

**Parameters**:
- `target_value`: The value to search for (default: 7)
- `num_iterations`: Number of Grover iterations (default: optimal)

---

## Python Examples

Python bindings (PySparQ) provide similar functionality. Basic usage:

```python
from pysparq import *

# Create quantum state
state = SparseState()

# Add registers
addr_reg = System.add_register("addr", StateStorageType.UnsignedInteger, 2)
data_reg = System.add_register("data", StateStorageType.UnsignedInteger, 3)

# Create QRAM
qram = QRAMCircuit_qutrit(2, 3, [1, 2, 3, 4])

# Apply Hadamard to create superposition
Hadamard_Int_Full("addr").apply(state)

# Load from QRAM
QRAMLoad(qram, "addr", "data").apply(state)

# Print state
StatePrint(StatePrintDisplay.Detail).apply(state)
```

See `test/PythonTest/` for more Python examples.

## Further Reading

- **Paper**: https://arxiv.org/abs/2503.13832
- **Main README**: `../README.md`
- **API Documentation**: See header files in `SparQ/include/` and `QRAM/include/`

## Troubleshooting

### Compilation Errors

**Error**: `fatal error: sparse_state_simulator.h: No such file`
- **Solution**: Ensure include paths are correctly set. The examples need access to:
  - `SparQ/include/`
  - `QRAM/include/`
  - `Common/include/`
  - `SparQ_Algorithm/include/`
  - Third-party headers

### Runtime Errors

**Error**: `Register type mismatch`
- **Solution**: Ensure registers are created with the correct type (`UnsignedInteger` for QRAM operations)

**Error**: `Invalid input`
- **Solution**: Check that address/data sizes match between QRAM circuit and registers

## Contributing

Feel free to add more examples! Good additions would be:
- Shor's algorithm example
- Quantum Fourier Transform (QFT) demonstration
- Error correction codes
- Variational quantum algorithms
