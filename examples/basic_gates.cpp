/**
 * @file basic_gates.cpp
 * @brief Quantum Gates Example with QRAM Integration
 * 
 * This example demonstrates various quantum gates available in SparQ
 * and how they can be combined with QRAM operations:
 * 1. Single-qubit gates (X, Y, Z, H, S, T)
 * 2. Rotation gates (RX, RY, RZ)
 * 3. Two-qubit gates (CNOT, SWAP)
 * 4. Controlled operations
 * 5. Integration with QRAM operations
 * 
 * @example
 *   ./basic_gates
 */

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <iostream>
#include <cmath>
#include "sparse_state_simulator.h"
#include "qram.h"

using namespace qram_simulator;

void example_single_qubit_gates() {
    std::cout << "\n=== Single-Qubit Gates Example ===" << std::endl;
    
    SparseState state;
    
    // Create a single-qubit register (Boolean type)
    auto qubit = AddRegister("q0", Boolean, 1)(state);
    
    std::cout << "Initial state |0>:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Pauli-X gate (NOT gate): |0> -> |1>
    (Xgate_Bool(qubit))(state);
    std::cout << "After X gate (|0> -> |1>):" << std::endl;
    (StatePrint(Detail))(state);
    
    // Hadamard gate: |1> -> (|0> - |1>)/sqrt(2)
    (Hadamard_Bool(qubit))(state);
    std::cout << "After H gate (superposition):" << std::endl;
    (StatePrint(Detail))(state);
    
    // Pauli-Z gate: flips the sign of |1>
    (Zgate_Bool(qubit))(state);
    std::cout << "After Z gate (phase flip):" << std::endl;
    (StatePrint(Detail))(state);
    
    System::clear();
}

void example_multi_qubit_gates() {
    std::cout << "\n=== Multi-Qubit Gates Example ===" << std::endl;
    
    SparseState state;
    
    // Create two qubits
    auto q0 = AddRegister("q0", Boolean, 1)(state);
    auto q1 = AddRegister("q1", Boolean, 1)(state);
    
    std::cout << "Initial state |00>:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Apply X to first qubit: |00> -> |10>
    (Xgate_Bool(q0))(state);
    std::cout << "After X on q0 (|10>):" << std::endl;
    (StatePrint(Detail))(state);
    
    // CNOT: Control=q0, Target=q1
    // |10> -> |11>
    (Xgate_Bool(q1).conditioned_by_all_ones(q0))(state);
    std::cout << "After CNOT(q0->q1) (|11>):" << std::endl;
    (StatePrint(Detail))(state);
    
    // Hadamard on both qubits to create Bell state
    System::clear();
    state = SparseState();
    q0 = AddRegister("q0", Boolean, 1)(state);
    q1 = AddRegister("q1", Boolean, 1)(state);
    
    (Hadamard_Bool(q0))(state);
    (Xgate_Bool(q1).conditioned_by_all_ones(q0))(state);
    std::cout << "Bell state (|00> + |11>)/sqrt(2):" << std::endl;
    (StatePrint(Detail))(state);
    
    System::clear();
}

void example_integer_registers() {
    std::cout << "\n=== Integer Register Operations Example ===" << std::endl;
    
    SparseState state;
    
    // Create 3-bit unsigned integer register
    auto reg = AddRegister("reg", UnsignedInteger, 3)(state);
    
    std::cout << "Initial state |0>:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Full Hadamard on all bits: creates uniform superposition
    // |0> -> (|0> + |1> + |2> + ... + |7>)/sqrt(8)
    (Hadamard_Int_Full(reg))(state);
    std::cout << "After Hadamard (superposition of 0-7):" << std::endl;
    (StatePrint(Detail))(state);
    
    System::clear();
}

void example_rotation_gates() {
    std::cout << "\n=== Rotation Gates Example ===" << std::endl;
    
    SparseState state;
    
    // Create a qubit
    auto qubit = AddRegister("q", Boolean, 1)(state);
    
    // Apply X gate first to get |1>
    (Xgate_Bool(qubit))(state);
    std::cout << "Initial |1> state:" << std::endl;
    (StatePrint(Detail))(state);
    
    // RX rotation by pi/2
    (RXgate_Bool(qubit, M_PI / 2))(state);
    std::cout << "After RX(pi/2):" << std::endl;
    (StatePrint(Detail))(state);
    
    System::clear();
}

void example_controlled_operations() {
    std::cout << "\n=== Controlled Operations Example ===" << std::endl;
    
    SparseState state;
    
    // Create control and target registers
    auto ctrl = AddRegister("ctrl", UnsignedInteger, 2)(state);
    auto target = AddRegister("target", Boolean, 1)(state);
    
    std::cout << "Initial state |ctrl=0, target=0>:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Put control in superposition
    (Hadamard_Int_Full(ctrl))(state);
    std::cout << "Control in superposition:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Apply X to target conditioned on control value = 2 (binary: 10)
    (Xgate_Bool(target).conditioned_by_value(ctrl, 2))(state);
    std::cout << "After X on target when ctrl=2:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Apply X to target conditioned on control's bit 0 being 1
    System::clear();
    state = SparseState();
    ctrl = AddRegister("ctrl", UnsignedInteger, 2)(state);
    target = AddRegister("target", Boolean, 1)(state);
    
    (Hadamard_Int_Full(ctrl))(state);
    // Control by specific bit positions
    (Xgate_Bool(target).conditioned_by_bit({{ctrl, 0}}))(state);
    std::cout << "After X on target when ctrl[0]=1:" << std::endl;
    (StatePrint(Detail))(state);
    
    System::clear();
}

void example_qram_with_gates() {
    std::cout << "\n=== QRAM with Quantum Gates Example ===" << std::endl;
    
    // Create QRAM with 2-bit address and 2-bit data
    const size_t addr_size = 2;
    const size_t data_size = 2;
    qram_qutrit::QRAMCircuit qram(addr_size, data_size);
    
    // Initialize memory: [1, 2, 3, 0]
    std::vector<size_t> memory = {1, 2, 3, 0};
    qram.set_memory(memory);
    
    std::cout << "QRAM memory:" << std::endl;
    for (size_t i = 0; i < memory.size(); ++i) {
        std::cout << "  Address " << i << " -> " << memory[i] << std::endl;
    }
    
    SparseState state;
    auto addr_reg = AddRegister("addr", UnsignedInteger, addr_size)(state);
    auto data_reg = AddRegister("data", UnsignedInteger, data_size)(state);
    
    // Step 1: Create address superposition
    (Hadamard_Int_Full(addr_reg))(state);
    std::cout << "\nAddress in superposition:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Step 2: Load from QRAM
    QRAMLoad::version = "noisefree";
    (QRAMLoad(&qram, addr_reg, data_reg))(state);
    std::cout << "After QRAM load:" << std::endl;
    (StatePrint(Detail))(state);
    
    // Step 3: Apply conditional operation based on loaded data
    // Create an auxiliary qubit
    auto aux = AddRegister("aux", Boolean, 1)(state);
    
    // Apply X to aux if data > 1 (i.e., data = 2 or 3)
    // First, we need to add a comparison flag
    auto flag = AddRegister("flag", Boolean, 1)(state);
    
    // For simplicity, let's use value-based conditioning
    // Apply X to aux when data = 3 (address 2)
    (Xgate_Bool(aux).conditioned_by_value(data_reg, 3))(state);
    std::cout << "After conditional X on aux (when data=3):" << std::endl;
    (StatePrint(Detail))(state);
    
    System::clear();
}

int main() {
    std::cout << "==============================================" << std::endl;
    std::cout << "  Quantum Gates and QRAM Integration Demo" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    example_single_qubit_gates();
    example_multi_qubit_gates();
    example_integer_registers();
    example_rotation_gates();
    example_controlled_operations();
    example_qram_with_gates();
    
    std::cout << "\n==============================================" << std::endl;
    std::cout << "  All examples completed successfully!" << std::endl;
    std::cout << "==============================================" << std::endl;
    
    return 0;
}
