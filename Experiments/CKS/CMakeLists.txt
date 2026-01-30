project(Experiment_CKS)


add_executable(${PROJECT_NAME}_Test HamiltonianSimulationTest.cpp)
target_link_libraries(${PROJECT_NAME}_Test SparQ)


add_executable(${PROJECT_NAME}_Noisy HamiltonianSimulationTest_Noisy.cpp)
target_link_libraries(${PROJECT_NAME}_Noisy SparQ)


if (CUDA_FOUND)
	add_executable(${PROJECT_NAME}_cutest cks_cutest.cu)
	target_link_libraries(${PROJECT_NAME}_cutest SparQ_cuda)

	set_target_properties(${PROJECT_NAME}_cutest
	  PROPERTIES
	  CUDA_STANDARD 17
	  CUDA_STANDARD_REQUIRED ON
	  CUDA_ARCHITECTURES "native"  
	  CUDA_SEPARABLE_COMPILATION ON  
	  CUDA_RESOLVE_DEVICE_SYMBOLS ON
	)

	target_compile_options(${PROJECT_NAME}_cutest PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
	--generate-line-info
	--use_fast_math
	--relocatable-device-code=true
	--extended-lambda
	--expt-relaxed-constexpr
	>)
endif()