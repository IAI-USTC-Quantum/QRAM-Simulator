/**
 * @file hello_qram.cpp
 * @brief Basic QRAM Hello World Example
 * 
 * This example demonstrates the basic usage of QRAM (Quantum Random Access Memory)
 * with the SparQ simulator. It shows how to:
 * 1. Create a QRAM circuit with address and data sizes
 * 2. Initialize memory with specific values
 * 3. Use QRAMLoad to load data from QRAM into a quantum register
 * 
 * @example
 *   ./hello_qram
 * 
 * Expected output: The data register should contain the value stored at address |0>
 */

#include <iostream>
#include <vector>
#include "sparse_state_simulator.h"
#include "qram.h"

using namespace qram_simulator;

int main() {
    // ============================================================
    // Step 1: Define QRAM parameters
    // ============================================================
    // Address size: 2 bits (can address 2^2 = 4 memory locations)
    // Data size: 3 bits (each memory location stores a 3-bit value)
    const size_t addr_size = 2;
    const size_t data_size = 3;
    
    std::cout << "=== QRAM Hello World Example ===" << std::endl;
    std::cout << "Address size: " << addr_size << " bits (" << (1 << addr_size) << " addresses)" << std::endl;
    std::cout << "Data size: " << data_size << " bits" << std::endl;
    std::cout << std::endl;

    // ============================================================
    // Step 2: Create and initialize QRAM circuit
    // ============================================================
    // Create a QRAM circuit with specified address and data sizes
    qram_qutrit::QRAMCircuit qram(addr_size, data_size);
    
    // Initialize memory with specific values:
    // Address 0 -> Value 5 (binary: 101)
    // Address 1 -> Value 3 (binary: 011)
    // Address 2 -> Value 7 (binary: 111)
    // Address 3 -> Value 1 (binary: 001)
    std::vector<size_t> memory = {5, 3, 7, 1};
    qram.set_memory(memory);
    
    std::cout << "Memory initialized:" << std::endl;
    for (size_t i = 0; i < memory.size(); ++i) {
        std::cout << "  Address " << i << " -> Value " << memory[i] << std::endl;
    }
    std::cout << std::endl;

    // ============================================================
    // Step 3: Create quantum state and registers
    // ============================================================
    // Create an empty sparse quantum state
    SparseState state;
    
    // Add address register (UnsignedInteger type for address)
    // The register ID is returned and used for subsequent operations
    size_t addr_reg = AddRegister("addr", UnsignedInteger, addr_size)(state);
    
    // Add data register (UnsignedInteger type for data)
    size_t data_reg = AddRegister("data", UnsignedInteger, data_size)(state);
    
    // Print initial state
    std::cout << "Initial quantum state:" << std::endl;
    (StatePrint(Detail))(state);
    std::cout << std::endl;

    // ============================================================
    // Step 4: Prepare address superposition (optional)
    // ============================================================
    // Apply Hadamard gate to address register to create superposition
    // This puts the address register in state: |0> + |1> + |2> + |3> (unnormalized)
    (Hadamard_Int_Full(addr_reg))(state);
    
    std::cout << "After Hadamard on address register (superposition):" << std::endl;
    (StatePrint(Detail))(state);
    std::cout << std::endl;

    // ============================================================
    // Step 5: Load data from QRAM
    // ============================================================
    // QRAMLoad performs: |a>|z> -> |a>|z XOR d[a]>
    // where a is address, z is data register, d[a] is memory value at address a
    // In this case, data register starts at |0>, so it becomes |d[a]>
    
    std::cout << "Loading data from QRAM..." << std::endl;
    QRAMLoad::version = "noisefree";  // Use noise-free version for clarity
    (QRAMLoad(&qram, addr_reg, data_reg))(state);
    
    std::cout << "After QRAM load:" << std::endl;
    (StatePrint(Detail))(state);
    std::cout << std::endl;

    // ============================================================
    // Step 6: Verify the result
    // ============================================================
    // The data register should now contain the memory values corresponding
    // to each address in the superposition
    std::cout << "Verification:" << std::endl;
    std::cout << "  Address |0> -> Data should be " << memory[0] << " (binary: ";
    for (int i = data_size - 1; i >= 0; --i) {
        std::cout << ((memory[0] >> i) & 1);
    }
    std::cout << ")" << std::endl;
    
    std::cout << "  Address |1> -> Data should be " << memory[1] << " (binary: ";
    for (int i = data_size - 1; i >= 0; --i) {
        std::cout << ((memory[1] >> i) & 1);
    }
    std::cout << ")" << std::endl;
    
    std::cout << "  Address |2> -> Data should be " << memory[2] << " (binary: ";
    for (int i = data_size - 1; i >= 0; --i) {
        std::cout << ((memory[2] >> i) & 1);
    }
    std::cout << ")" << std::endl;
    
    std::cout << "  Address |3> -> Data should be " << memory[3] << " (binary: ";
    for (int i = data_size - 1; i >= 0; --i) {
        std::cout << ((memory[3] >> i) & 1);
    }
    std::cout << ")" << std::endl;

    // ============================================================
    // Cleanup
    // ============================================================
    System::clear();  // Clear all registers
    
    std::cout << std::endl << "=== Example completed successfully ===" << std::endl;
    return 0;
}
