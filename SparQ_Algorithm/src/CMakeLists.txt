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
    include(${CMAKE_SOURCE_DIR}/cmake/CUDA.cmake)
    sparq_setup_cuda_target(${PROJECT_NAME}_cuda)
endif()

# Only define EIGEN_NO_CUDA for CPU-only builds to avoid Eigen CUDA overhead
if(NOT CUDA_FOUND)
    target_compile_definitions(SparQ_Algorithm PRIVATE EIGEN_NO_CUDA)
endif()

# Aggregation target: links all component libraries together
add_library(SparQ INTERFACE)
target_link_libraries(SparQ INTERFACE
		SparQ_Algorithm
		SparQ_Simulator
		SparQ_QRAMSimulator
		SparQ_Common
		fmt
		)
if (TBB_FOUND)
    target_link_libraries(SparQ INTERFACE TBB::tbb)
endif()

if(CUDA_FOUND)
	add_library(SparQ_cuda INTERFACE)
	target_link_libraries(SparQ_cuda INTERFACE
			SparQ_Algorithm_cuda
			SparQ_Simulator_cuda
			SparQ_QRAMSimulator_cuda
			SparQ_Common_cuda
			fmt
			)
	if (TBB_FOUND)
	    target_link_libraries(SparQ_cuda INTERFACE TBB::tbb)
	endif()
endif()
