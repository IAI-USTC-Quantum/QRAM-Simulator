add_subdirectory(CPUTest)
if (CUDA_FOUND)
add_subdirectory(GPUTest)
endif()