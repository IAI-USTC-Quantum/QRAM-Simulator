project(SparQ_QRAMSimulator)

file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(${PROJECT_NAME} STATIC ${src})


if (CUDA_FOUND)
    file(GLOB_RECURSE cuda_src ${CMAKE_CURRENT_SOURCE_DIR}/cuda/*.cu)

    add_library(${PROJECT_NAME}_cuda STATIC ${cuda_src} ${src})
    target_sources(${PROJECT_NAME}_cuda PRIVATE ${cuda_src})
    include(${CMAKE_SOURCE_DIR}/cmake/CUDA.cmake)
    sparq_setup_cuda_target(${PROJECT_NAME}_cuda)
endif()
