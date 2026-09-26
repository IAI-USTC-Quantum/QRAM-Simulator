# Quickstart

## Python: Noisy QRAM Loading Fidelity Simulation

```python
from qram_simulator import (
    QRAMCircuitQubit, OperationType, set_seed,
)

# 1. Fix the random seed (memory generation, input sampling, and output sampling all go through the global engine)
set_seed(42)

# 2. Build the QRAM circuit: addr_size=4 (16 addresses), data_size=2 (4 values per slot)
qram = QRAMCircuitQubit(4, 2)
qram.set_memory_random()          # random data tree

# 3. Inject noise (optional): probabilities must lie in [0,1]; Damping requires gamma < 1
qram.set_noise_models({
    OperationType.Depolarizing: 1e-3,
    OperationType.Damping: 1e-4,
})

# 4. Sample 100 input branches and run (pruning mode)
qram.set_input_uniform(100)
qram.run_normal()                 # or run_full(): unpruned ground truth

# 5. Sample the output and compute the loading fidelity
fidelity = qram.sample_and_get_fidelity()
print(f"loading fidelity = {fidelity:.4f}")
```

Sanity check for the noise-free case: the fidelity should return exactly 1.

```python
set_seed(7)
qram = QRAMCircuitQubit(3, 2)
qram.set_memory_random()
qram.set_input_uniform(20)
qram.run_normal()
assert abs(qram.sample_and_get_fidelity() - 1.0) < 1e-9
```

## Python: Qutrit Architecture

The interface has the same shape as the qubit architecture, so it can be swapped in directly:

```python
from qram_simulator import QRAMCircuitQutrit, OperationType, set_seed

set_seed(7)
qram = QRAMCircuitQutrit(3, 2)
qram.set_memory_random()
qram.set_noise_models({OperationType.Depolarizing: 1e-3})
qram.set_input_uniform(20)
qram.run_normal()
print(qram.sample_and_get_fidelity())
```

> Note: the `run(version)` dispatch entry of the qutrit architecture requires a **non-empty** noise model to be set first; in noise-free scenarios call `run_normal()` / `run_full()` directly.

## Python: Embedding into an External Full-Amplitude Simulator (QRAMFullAmp)

`QRAMFullAmp` composites a single QRAM loading onto a full-amplitude state vector — the external library only needs to provide the state vector and the qubit mapping:

```python
from qram_simulator import QRAMFullAmp, OperationType, set_seed

set_seed(5)
# address qubits {0,1}, data qubits {2,3}, data tree [0,1,2,3]
manipulator = QRAMFullAmp(2, 2, [0, 1, 2, 3])
manipulator.set_noise_models({OperationType.Depolarizing: 1e-3})

state = [0j] * 16
for addr in range(4):
    state[addr] = 0.5 + 0j      # uniform superposition over addresses, data bits are 0

out = manipulator.apply(
    state,
    address_qubits=[0, 1],
    data_qubits=[2, 3],
    other_qubits=[],
    version="normal",           # "normal" prunes / "full" does not prune
)
# after loading |a>|00> → |a>|mem[a]>: each of the four basis states has probability 1/4
```

## Python: Inspecting the Schedule (TimeStep)

```python
from qram_simulator import TimeStep, ARCH_QUBIT, OperationType

ts = TimeStep(2, 1)                                  # addr=2, data=1
noise_free = ts.generate({}, ARCH_QUBIT)             # noise-free schedule
noisy = ts.generate({OperationType.Depolarizing: 1e-3}, ARCH_QUBIT)

print(len(noise_free), "time slices")
print(noisy)                                          # human-readable schedule listing

lo, hi = ts.get_bad_range_qubit(0)                    # bad-branch address range
print(f"bad-branch range for an error at address 0 = [{lo}, {hi}]")
```

## C++: Minimal Example

```cpp
#include "qram_circuit_qubit.h"
#include <iostream>

using namespace qram_simulator;
using namespace qram_qubit;

int main() {
    random_engine::set_seed(42);

    QRAMCircuit qram(4, 2);
    qram.set_memory_random();

    qram.set_noise_models({
        { OperationType::Depolarizing, 1e-3 },
        { OperationType::Damping, 1e-4 },
    });

    qram.set_input_uniform(100);
    qram.run_normal();

    std::cout << "loading fidelity = "
              << qram.sample_and_get_fidelity() << std::endl;
    return 0;
}
```

To compile (assuming the repository root is the current directory):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Next Steps

- [Architecture document](architecture.md): module layout, data flow, and the pruning mechanism
- {doc}`C++ API reference <../api/cpp>`: complete Doxygen documentation for all core classes
- [Paper reproduction guide](../paper/reproduction.md): reproducing the figures of arXiv:2503.13832
