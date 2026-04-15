project(Experiment_CKS)


add_executable(${PROJECT_NAME}_Test HamiltonianSimulationTest.cpp)
target_link_libraries(${PROJECT_NAME}_Test SparQ)


add_executable(${PROJECT_NAME}_Noisy HamiltonianSimulationTest_Noisy.cpp)
target_link_libraries(${PROJECT_NAME}_Noisy SparQ)


if (CUDA_FOUND)
	include(${CMAKE_SOURCE_DIR}/cmake/CUDA.cmake)
	add_executable(${PROJECT_NAME}_cutest cks_cutest.cu)
	target_link_libraries(${PROJECT_NAME}_cutest SparQ_cuda)
	sparq_setup_cuda_target(${PROJECT_NAME}_cutest)
endif()
