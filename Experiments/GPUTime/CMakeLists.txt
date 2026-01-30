project(Experiment_GPUTime)

add_executable(${PROJECT_NAME}_parallel_test parallel_test.cu)
target_link_libraries(${PROJECT_NAME}_parallel_test SparQ_cuda)

set_target_properties(${PROJECT_NAME}_parallel_test
  PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
  CUDA_ARCHITECTURES "native"  
  CUDA_SEPARABLE_COMPILATION ON  
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_compile_options(${PROJECT_NAME}_parallel_test PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --generate-line-info
    --use_fast_math
    --relocatable-device-code=true
    --extended-lambda
    --expt-relaxed-constexpr
    >)


add_executable(${PROJECT_NAME}_block_encoding_test block_encoding_test.cu)
target_link_libraries(${PROJECT_NAME}_block_encoding_test SparQ_cuda)

set_target_properties(${PROJECT_NAME}_block_encoding_test
  PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
  CUDA_ARCHITECTURES "native"  
  CUDA_SEPARABLE_COMPILATION ON  
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_compile_options(${PROJECT_NAME}_block_encoding_test PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --generate-line-info
    --use_fast_math
    --relocatable-device-code=true
    --extended-lambda
    --expt-relaxed-constexpr
    >)


add_executable(${PROJECT_NAME}_cks_test cks_test.cu)
target_link_libraries(${PROJECT_NAME}_cks_test SparQ_cuda)

set_target_properties(${PROJECT_NAME}_cks_test
  PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
  CUDA_ARCHITECTURES "native"  
  CUDA_SEPARABLE_COMPILATION ON  
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_compile_options(${PROJECT_NAME}_cks_test PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --generate-line-info
    --use_fast_math
    --relocatable-device-code=true
    --extended-lambda
    --expt-relaxed-constexpr
    >)
    

add_executable(${PROJECT_NAME}_qda_test qda_test.cu)
target_link_libraries(${PROJECT_NAME}_qda_test SparQ_cuda)

set_target_properties(${PROJECT_NAME}_qda_test
  PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
  CUDA_ARCHITECTURES "native"  
  CUDA_SEPARABLE_COMPILATION ON  
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_compile_options(${PROJECT_NAME}_qda_test PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --generate-line-info
    --use_fast_math
    --relocatable-device-code=true
    --extended-lambda
    --expt-relaxed-constexpr
    >)