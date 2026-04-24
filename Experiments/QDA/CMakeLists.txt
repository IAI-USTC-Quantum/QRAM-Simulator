project(Experiment_QDA)

add_subdirectory(QDA_Simple)
add_subdirectory(QDA_Poiseuille)
add_subdirectory(QDATest)

if (CUDA_FOUND)
	add_executable(${PROJECT_NAME}_cutest qda_cutest.cu)
	target_link_libraries(${PROJECT_NAME}_cutest SparQ_cuda)

	set_target_properties(${PROJECT_NAME}_cutest
	  PROPERTIES
	  CUDA_STANDARD 17
	  CUDA_STANDARD_REQUIRED ON
	  CUDA_ARCHITECTURES ${CMAKE_CUDA_ARCHITECTURES}  
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