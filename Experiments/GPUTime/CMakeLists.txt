project(Experiment_GPUTime)

include(${CMAKE_SOURCE_DIR}/cmake/CUDA.cmake)

add_executable(${PROJECT_NAME}_parallel_test parallel_test.cu)
target_link_libraries(${PROJECT_NAME}_parallel_test SparQ_cuda)
sparq_setup_cuda_target(${PROJECT_NAME}_parallel_test)

add_executable(${PROJECT_NAME}_block_encoding_test block_encoding_test.cu)
target_link_libraries(${PROJECT_NAME}_block_encoding_test SparQ_cuda)
sparq_setup_cuda_target(${PROJECT_NAME}_block_encoding_test)

add_executable(${PROJECT_NAME}_cks_test cks_test.cu)
target_link_libraries(${PROJECT_NAME}_cks_test SparQ_cuda)
sparq_setup_cuda_target(${PROJECT_NAME}_cks_test)

add_executable(${PROJECT_NAME}_qda_test qda_test.cu)
target_link_libraries(${PROJECT_NAME}_qda_test SparQ_cuda)
sparq_setup_cuda_target(${PROJECT_NAME}_qda_test)