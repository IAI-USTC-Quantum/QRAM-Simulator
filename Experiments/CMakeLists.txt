add_subdirectory(QDA)
add_subdirectory(QRAM)
add_subdirectory(Grover)
add_subdirectory(StatePreparation)
add_subdirectory(QCNN)
add_subdirectory(QFT)
add_subdirectory(CKS)
add_subdirectory(Shor)
add_subdirectory(GHZ)
add_subdirectory(ErrorFiltration)

if (CUDA_FOUND)
add_subdirectory(GPUTime)
endif()

# 噪声模拟对拍验证（qutrit/qubit QRAM ↔ 内置电路级噪声模拟器），
# 注册为 ctest 用例，随 CI 全量回归
add_executable(verify_noisy_simulation verify_noisy_simulation.cpp)
target_link_libraries(verify_noisy_simulation SparQ)
add_test(NAME verify_noisy_simulation COMMAND verify_noisy_simulation)