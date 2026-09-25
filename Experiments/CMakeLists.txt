add_subdirectory(QRAM)

# Noise simulation cross-check verification (qutrit/qubit QRAM ↔ built-in circuit-level noise simulator),
# registered as a ctest case, runs with the full CI regression
add_executable(verify_noisy_simulation verify_noisy_simulation.cpp)
target_link_libraries(verify_noisy_simulation SparQ_QRAMSimulator SparQ_Common)
add_test(NAME verify_noisy_simulation COMMAND verify_noisy_simulation)
