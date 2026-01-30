#pragma once

#include "typedefs.h"

#ifdef __INTELLISENSE__
#define __CUDACC__
#define __NVCC__
#endif

#if defined(__CUDACC__) && defined(USE_CUDA)
#include <cuda_runtime.h>
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

	template <typename T>
	void thrust_device_to_std(std::vector<T>& hv, const thrust::device_vector<T>& dv) {
		hv.resize(dv.size());

		//thrust::host_vector<T> tmp = dv;
		//for (size_t i = 0; i < tmp.size(); ++i) {
		//	hv[i] = tmp[i];
		//}

		thrust::copy(dv.begin(), dv.end(), hv.begin());
	}

	template <typename T>
	void std_to_thrust_device(thrust::device_vector<T>& dv, const std::vector<T>& hv) {
		thrust::device_vector<T> tmp(hv.begin(), hv.end());
		thrust::swap(dv, tmp);
	}

	//__device__ __host__
	//constexpr complex_t conj_dev(const complex_t& z)
	//{
	//	return complex_t(z.real(), -z.imag());
	//}

	using cu_complex_t = thrust::complex<double>;

	HOST_DEVICE inline cu_complex_t operator+=(cu_complex_t& a, const cu_complex_t& b) {
		a.real(a.real() + b.real());
		a.imag(a.imag() + b.imag());
		return a;
	}

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

