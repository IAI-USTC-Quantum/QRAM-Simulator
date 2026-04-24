cmake_minimum_required(VERSION 3.20)
project(CudaExecutabilityTest CUDA)
add_executable(CudaExecutabilityTest CudaExecutabilityTest.cu)
target_link_libraries(CudaExecutabilityTest SparQ_cuda)

if (DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CUDA_ARCHS ${CMAKE_CUDA_ARCHITECTURES})
else()
  set(CUDA_ARCHS 75)
endif()

set_target_properties(CudaExecutabilityTest
  PROPERTIES
  CUDA_STANDARD 17
  CUDA_STANDARD_REQUIRED ON
  CUDA_ARCHITECTURES ${CUDA_ARCHS}
  CUDA_SEPARABLE_COMPILATION ON
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_compile_options(CudaExecutabilityTest PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --generate-line-info
    --use_fast_math
    --relocatable-device-code=true
    --extended-lambda
    --expt-relaxed-constexpr
    >)