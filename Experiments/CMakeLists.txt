add_subdirectory(QRAM)

# 噪声模拟对拍验证（qutrit/qubit QRAM ↔ 内置电路级噪声模拟器），
# 注册为 ctest 用例，随 CI 全量回归
add_executable(verify_noisy_simulation verify_noisy_simulation.cpp)
target_link_libraries(verify_noisy_simulation SparQ_QRAMSimulator SparQ_Common)
add_test(NAME verify_noisy_simulation COMMAND verify_noisy_simulation)
