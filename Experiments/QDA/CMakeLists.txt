project(Experiment_QDA)

add_subdirectory(QDA_Simple)
add_subdirectory(QDA_Poiseuille)
add_subdirectory(QDATest)

if (CUDA_FOUND)
	include(${CMAKE_SOURCE_DIR}/cmake/CUDA.cmake)
	add_executable(${PROJECT_NAME}_cutest qda_cutest.cu)
	target_link_libraries(${PROJECT_NAME}_cutest SparQ_cuda)
	sparq_setup_cuda_target(${PROJECT_NAME}_cutest)
endif()
