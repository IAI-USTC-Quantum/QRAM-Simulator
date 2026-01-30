project(SparQ_Algorithm)

file(GLOB_RECURSE src "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
					  "${CMAKE_CURRENT_SOURCE_DIR}/BlockEncoding/*.cpp"
					  "${CMAKE_CURRENT_SOURCE_DIR}/DiscreteAdiabatic/*.cpp")
add_library(${PROJECT_NAME} STATIC ${src})

if (CUDA_FOUND)
    file(GLOB_RECURSE cuda_src ${CMAKE_CURRENT_SOURCE_DIR}/cuda/*.cu
                               ${CMAKE_CURRENT_SOURCE_DIR}/BlockEncoding/cuda/*.cu
                               ${CMAKE_CURRENT_SOURCE_DIR}/DiscreteAdiabatic/cuda/*.cu)
    
    add_library(${PROJECT_NAME}_cuda STATIC ${cuda_src} ${src})
    target_sources(${PROJECT_NAME}_cuda PRIVATE ${cuda_src})
    set_target_properties(${PROJECT_NAME}_cuda PROPERTIES
            CUDA_STANDARD 17
            CUDA_STANDARD_REQUIRED ON
            CUDA_ARCHITECTURES "native" 
            CUDA_SEPARABLE_COMPILATION ON 
            CUDA_RESOLVE_DEVICE_SYMBOLS ON
        )

    target_compile_definitions(${PROJECT_NAME}_cuda PUBLIC USE_CUDA=1 EIGEN_NO_CUDA)

    target_link_libraries(${PROJECT_NAME}_cuda PRIVATE 
        cudart cudadevrt
    )

    target_compile_options(${PROJECT_NAME}_cuda PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
        --generate-line-info
        --use_fast_math
        --relocatable-device-code=true
        --extended-lambda
        --expt-relaxed-constexpr
        >)

endif()

add_library(SparQ STATIC ${src})

target_link_libraries(SparQ 
	SparQ_Algorithm
	SparQ_Simulator
	SparQ_QRAMSimulator
	SparQ_Common 
	fmt
)
if (TBB_FOUND)
    target_link_libraries(SparQ TBB::tbb)
endif()

target_compile_definitions(SparQ_Algorithm PRIVATE EIGEN_NO_CUDA)

if(CUDA_FOUND)
	add_library(SparQ_cuda STATIC ${src})
	target_link_libraries(SparQ_cuda
		SparQ_Algorithm_cuda
		SparQ_Simulator_cuda
		SparQ_QRAMSimulator_cuda 
		SparQ_Common_cuda 
		fmt
	)
if (TBB_FOUND)
    target_link_libraries(SparQ_cuda TBB::tbb)
endif()	

endif()