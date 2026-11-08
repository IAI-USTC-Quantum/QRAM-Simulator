project(SparQ_Common)

file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(${PROJECT_NAME} STATIC ${src})
# fmt is defined by ThirdParty/CMakeLists.txt (added later from the root);
# PUBLIC because basic.h/logger.h expose fmt headers to all consumers.
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)

if (CUDA_FOUND)
    file(GLOB_RECURSE cuda_src ${CMAKE_CURRENT_SOURCE_DIR}/cuda/*.cu)

    # 以 nvcc 重编全部源码（USE_CUDA=1 切换数据结构布局），
    # 目标配置统一由 cmake/CUDA.cmake 的 sparq_setup_cuda_target 提供。
    include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CUDA.cmake)
    add_library(${PROJECT_NAME}_cuda STATIC ${cuda_src} ${src})
    target_link_libraries(${PROJECT_NAME}_cuda PUBLIC fmt)
    sparq_setup_cuda_target(${PROJECT_NAME}_cuda)
endif()