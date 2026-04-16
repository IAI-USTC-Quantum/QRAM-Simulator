# Shared CUDA compile options for QRAM-Simulator
# Include this file after defining the CUDA target.

# Set standard CUDA target properties
function(sparq_setup_cuda_target target)
    set_target_properties(${target} PROPERTIES
        CUDA_STANDARD 17
        CUDA_STANDARD_REQUIRED ON
        CUDA_ARCHITECTURES "native"
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
    )

    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
        --generate-line-info
        --use_fast_math
        --relocatable-device-code=true
        --extended-lambda
        --expt-relaxed-constexpr
    >)

    target_compile_definitions(${target} PRIVATE USE_CUDA=1)

    target_link_libraries(${target} PRIVATE
        cudart cudadevrt
    )
endfunction()
