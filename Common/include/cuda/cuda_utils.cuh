#pragma once

#include "typedefs.h"

#ifdef __INTELLISENSE__
#define __CUDACC__
#define __NVCC__
#endif

#if defined(__CUDACC__) && defined(USE_CUDA)
#include <cuda_runtime.h>
#include <cuda/std/complex>
#include <thrust/binary_search.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/transform.h>
#include <thrust/transform_reduce.h>
#include <thrust/logical.h>
#include <thrust/complex.h>
#include <thrust/sort.h>
#include <thrust/pair.h>
#include <thrust/tuple.h>
#include <thrust/remove.h>
#include <thrust/partition.h>

#define CUDA_CHECK(err) do { \
    cudaError_t err_ = (err); \
    if (err_ != cudaSuccess) { \
        std::string errinfo = cudaGetErrorString(err_); \
        std::string errmsg = fmt::format("CUDA error {} at {}:{}\n", errinfo, __FILE__, __LINE__);\
        qram_simulator::throw_cuda_runtime_error(errmsg); \
    } \
} while (0)


namespace qram_simulator {

	void throw_cuda_runtime_error();
	void throw_cuda_runtime_error(const char* errinfo);
	void throw_cuda_runtime_error(const std::string& errinfo);
	void throw_cuda_runtime_error(std::string_view errinfo);
	void run_cuda_kernel();

	/// 将设备向量内容拷贝回宿主向量（宿主侧自动 resize 到设备长度）
	template <typename T>
	void thrust_device_to_std(std::vector<T>& hv, const thrust::device_vector<T>& dv) {
		hv.resize(dv.size());
		thrust::copy(dv.begin(), dv.end(), hv.begin());
	}

	/// 将宿主向量内容上传到设备向量（assign 复用既有设备缓冲，避免反复分配显存）
	template <typename T>
	void std_to_thrust_device(thrust::device_vector<T>& dv, const std::vector<T>& hv) {
		dv.assign(hv.begin(), hv.end());
	}

	/// 设备侧复数类型：cuda::std::complex 与 std::complex 布局兼容、可隐式互转，
	/// 全部运算符均可在 __host__ __device__ 代码中调用（libcu++）。
	/// 注：thrust::complex 在 CCCL 2.7/3.3 中仍是独立实现（非本类型别名），
	/// 与本类型布局兼容、值级可互转。
	using cu_complex_t = cuda::std::complex<double>;

	__device__ inline bool cuda_bit_parity(unsigned int v) {
		// 使用 CUDA 的 __popc 函数计算位数，然后检查奇偶性
		return (__popc(v) & 1) != 0;
	}

	__device__ inline bool cuda_bit_parity(unsigned long long v) {
		// 使用 CUDA 的 __popcll 函数计算64位整数中的位数
		return (__popcll(v) & 1) != 0;
	}
	
	constexpr size_t CUDA_BLOCK_SIZE = 512;
}

#else
#define CUDA_CHECK(err) (void)(err)
#endif

