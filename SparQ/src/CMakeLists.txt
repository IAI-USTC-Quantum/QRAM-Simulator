project(SparQ_Simulator)

file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(${PROJECT_NAME} STATIC ${src})

if (CUDA_FOUND)
    file(GLOB_RECURSE cuda_src ${CMAKE_CURRENT_SOURCE_DIR}/cuda/*.cu)
    
    add_library(${PROJECT_NAME}_cuda STATIC ${cuda_src} ${src})
    target_sources(${PROJECT_NAME}_cuda PRIVATE ${cuda_src})
    set_target_properties(${PROJECT_NAME}_cuda PROPERTIES
            CUDA_STANDARD 17
            CUDA_STANDARD_REQUIRED ON
            CUDA_ARCHITECTURES ${CMAKE_CUDA_ARCHITECTURES}  
            CUDA_SEPARABLE_COMPILATION ON  
            CUDA_RESOLVE_DEVICE_SYMBOLS ON
        )

    target_link_libraries(${PROJECT_NAME}_cuda PRIVATE 
        cudart cudadevrt
    )

    target_compile_definitions(${PROJECT_NAME}_cuda PUBLIC USE_CUDA=1 EIGEN_NO_CUDA)

    target_compile_options(${PROJECT_NAME}_cuda PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --generate-line-info
    --use_fast_math
    --relocatable-device-code=true
    --extended-lambda
    --expt-relaxed-constexpr
    >)

endif()

