#include "cuda_utils.cuh"

#ifdef USE_CUDA

namespace qram_simulator {
    void throw_cuda_runtime_error()
    {
#ifndef QRAM_Release
        throw std::runtime_error("[CUDA Fatal] Cuda runtime error. Please check the code.");
#endif
    }

    void throw_cuda_runtime_error(const char* errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_cuda_runtime_error(const std::string& errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_cuda_runtime_error(std::string_view errinfo)
    {
        return throw_cuda_runtime_error(errinfo.data());
    }

    __global__ void hello_cuda() {
        printf("Hello from GPU thread %d!\n", threadIdx.x);
    }

    void run_cuda_kernel() {
        hello_cuda << <1, 5 >> > ();
        CUDA_CHECK(cudaDeviceSynchronize());
    }
} // namespace qram_simulator

#endif