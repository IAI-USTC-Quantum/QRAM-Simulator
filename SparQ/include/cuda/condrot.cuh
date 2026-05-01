#pragma once

#include "basic_components.cuh"
#include "quantum_interfere_basic.cuh"
#include "condrot.h"

namespace qram_simulator {

	namespace condrot_general_gpu_detail {
		template<typename Callable>
		struct CuAngleFunction
		{
			Callable func;

			CuAngleFunction(Callable f) : func(f) {}

			__device__ void operator()(size_t v, double* mat) const {
				u22_t mat_u22 = func(v);
				mat[0] = mat_u22[0].real();
				mat[1] = mat_u22[0].imag();
				mat[2] = mat_u22[1].real();
				mat[3] = mat_u22[1].imag();
				mat[4] = mat_u22[2].real();
				mat[5] = mat_u22[2].imag();
				mat[6] = mat_u22[3].real();
				mat[7] = mat_u22[3].imag();
			}
		};

		// CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
		template<typename CuAngleFunction>
		__global__ void CondRot_General_Bool_operate_alone(System* state, size_t nsize, unq_ele* unq_s, size_t old_size,
			int in_id, int out_id, size_t in_size, CuAngleFunction func)
		{
			size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
			size_t my_loc;

			if (tid < nsize) {
				my_loc = unq_s[tid].sptr;
				state[old_size + tid] = state[my_loc];

				size_t v = CuGetAsUint64(state[my_loc], in_id, in_size);

				double mat[8];
				func(v, mat);
				double u00_real = mat[0];
				double u00_imag = mat[1];
				double u01_real = mat[2];
				double u01_imag = mat[3];
				double u10_real = mat[4];
				double u10_imag = mat[5];
				double u11_real = mat[6];
				double u11_imag = mat[7];
				//printf("CondRot mat: %lf %lf %lf %lf %lf %lf %lf %lf\n", mat[0], mat[1], mat[2], mat[3], mat[4], mat[5], mat[6], mat[7]);

				if (CuGetAsBool(state[my_loc], out_id, 1) == 0) {
					CuGet(state[old_size + tid], out_id).value = 1;

					double a_real = CuSystemAmplitude(state[my_loc])[0];
					double a_imag = CuSystemAmplitude(state[my_loc])[1];
					double b_real = 0;
					double b_imag = 0;

					CuSystemAmplitude(state[my_loc])[0] = (u00_real * a_real - u00_imag * a_imag) + (u01_real * b_real - u01_imag * b_imag);
					CuSystemAmplitude(state[my_loc])[1] = (u00_real * a_imag + u00_imag * a_real) + (u01_real * b_imag + u01_imag * b_real);
					CuSystemAmplitude(state[old_size + tid])[0] = (u10_real * a_real - u10_imag * a_imag) + (u11_real * b_real - u11_imag * b_imag);
					CuSystemAmplitude(state[old_size + tid])[1] = (u10_real * a_imag + u10_imag * a_real) + (u11_real * b_imag + u11_imag * b_real);
				}
				else {
					CuGet(state[old_size + tid], out_id).value = 0;
					double a_real = 0;
					double a_imag = 0;
					double b_real = CuSystemAmplitude(state[my_loc])[0];
					double b_imag = CuSystemAmplitude(state[my_loc])[1];

					CuSystemAmplitude(state[old_size + tid])[0] = (u00_real * a_real - u00_imag * a_imag) + (u01_real * b_real - u01_imag * b_imag);
					CuSystemAmplitude(state[old_size + tid])[1] = (u00_real * a_imag + u00_imag * a_real) + (u01_real * b_imag + u01_imag * b_real);
					CuSystemAmplitude(state[my_loc])[0] = (u10_real * a_real - u10_imag * a_imag) + (u11_real * b_real - u11_imag * b_imag);
					CuSystemAmplitude(state[my_loc])[1] = (u10_real * a_imag + u10_imag * a_real) + (u11_real * b_imag + u11_imag * b_real);
				}
			}
		}

		template<typename CuAngleFunction>
		__global__ void CondRot_General_Bool_operate_pair(System* state, size_t nsize, unq_ele* unq_s, size_t old_size,
			int in_id, int out_id, size_t in_size, CuAngleFunction func)
		{
			size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
			size_t my_loc;

			if (tid < nsize) {
				my_loc = unq_s[tid].sptr;
				size_t v = CuGetAsUint64(state[my_loc], in_id, in_size);

				double mat[8];
				func(v, mat);
				double u00_real = mat[0];
				double u00_imag = mat[1];
				double u01_real = mat[2];
				double u01_imag = mat[3];
				double u10_real = mat[4];
				double u10_imag = mat[5];
				double u11_real = mat[6];
				double u11_imag = mat[7];
				//printf("CondRot mat: %lf %lf %lf %lf %lf %lf %lf %lf\n", mat[0], mat[1], mat[2], mat[3], mat[4], mat[5], mat[6], mat[7]);

				double a_real = CuSystemAmplitude(state[my_loc])[0];
				double a_imag = CuSystemAmplitude(state[my_loc])[1];
				double b_real = CuSystemAmplitude(state[my_loc + 1])[0];
				double b_imag = CuSystemAmplitude(state[my_loc + 1])[1];

				CuSystemAmplitude(state[my_loc])[0] = (u00_real * a_real - u00_imag * a_imag) + (u01_real * b_real - u01_imag * b_imag);
				CuSystemAmplitude(state[my_loc])[1] = (u00_real * a_imag + u00_imag * a_real) + (u01_real * b_imag + u01_imag * b_real);
				CuSystemAmplitude(state[my_loc + 1])[0] = (u10_real * a_real - u10_imag * a_imag) + (u11_real * b_real - u11_imag * b_imag);
				CuSystemAmplitude(state[my_loc + 1])[1] = (u10_real * a_imag + u10_imag * a_real) + (u11_real * b_imag + u11_imag * b_real);
			}
		}
	}

	template<typename Callable>
	void CondRot_General_Bool_fast<Callable>::operator()(CuSparseState& state) const
	{
		profiler _("CondRot_General_Bool cuda");
		using namespace condrot_general_gpu_detail;

		state.move_to_gpu();
		SortExceptKey_devfunc(out_id, state.sparse_state_gpu);

		thrust::device_vector<unq_ele> unique_s;
		Unique_count_elem(state, unique_s, out_id);
		//SortUniqueElements(unique_s);

		size_t num_one = EleNum_MoreThanOne(unique_s);
		size_t unique_state_num = unique_s.size();
		size_t num_not_one = unique_state_num - num_one;
		/* expand state size */
		size_t state_size_0 = state.sparse_state_gpu.size();

		state.sparse_state_gpu.resize(state_size_0 + num_one);
		unq_ele* s_ptr;
		size_t blocksize = CUDA_BLOCK_SIZE;
		size_t nblock = 0;

		size_t in_size = System::size_of(in_id);

		CuAngleFunction<Callable> cu_func(func);

		if (num_one > 0)
		{
			s_ptr = unique_s.data().get();
			nblock = (num_one - 1) / blocksize + 1;
			CondRot_General_Bool_operate_alone << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_one, s_ptr, state_size_0,
				in_id, out_id, in_size, cu_func);
		}
		if (num_not_one > 0)
		{
			s_ptr = thrust::raw_pointer_cast(&(unique_s[num_one]));
			nblock = (num_not_one - 1) / blocksize + 1;
			CondRot_General_Bool_operate_pair << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_one, s_ptr, state_size_0,
				in_id, out_id, in_size, cu_func);
		}

		ClearZero()(state);
		System::update_max_size(state.size());
	}

} // namespace qram_simulator
