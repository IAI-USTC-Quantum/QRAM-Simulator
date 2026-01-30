#include "basic_gates.h"
#include "cuda_utils.cuh"
#include "cuda/quantum_interfere_basic.cuh"

#ifdef USE_CUDA

namespace qram_simulator {

	struct RotBool_Diagonal_Functor_Control {
		int id;
		double u00_real;
		double u00_imag;
		double u11_real;
		double u11_imag;

		CuCondition_Functor

		RotBool_Diagonal_Functor_Control(int id_, complex_t u00, complex_t u11, CuCondition_Params)
			: id(id_), u00_real(u00.real()), u00_imag(u00.imag()), u11_real(u11.real()), u11_imag(u11.imag()), 
			CuCondition_Init
		{}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				double a_real = CuSystemAmplitude(s)[0];
				double a_imag = CuSystemAmplitude(s)[1];

				if (CuGetAsBool(s, id, 1) == 0)
				{
					CuSystemAmplitude(s)[0] = a_real * u00_real - a_imag * u00_imag;
					CuSystemAmplitude(s)[1] = a_real * u00_imag + a_imag * u00_real;
				}
				else
				{
					CuSystemAmplitude(s)[0] = a_real * u11_real - a_imag * u11_imag;
					CuSystemAmplitude(s)[1] = a_real * u11_imag + a_imag * u11_real;
				}
			}
		}
	};

	struct RotBool_Diagonal_Functor {
		int id;
		double u00_real;
		double u00_imag;
		double u11_real;
		double u11_imag;

		RotBool_Diagonal_Functor(int id_, complex_t u00, complex_t u11)
			: id(id_), u00_real(u00.real()), u00_imag(u00.imag()), u11_real(u11.real()), u11_imag(u11.imag())
		{}

		__host__ __device__ void operator()(System& s) const {
			double a_real = CuSystemAmplitude(s)[0];
			double a_imag = CuSystemAmplitude(s)[1];

			if (CuGetAsBool(s, id, 1) == 0)
			{
				CuSystemAmplitude(s)[0] = a_real * u00_real - a_imag * u00_imag;
				CuSystemAmplitude(s)[1] = a_real * u00_imag + a_imag * u00_real;
			}
			else
			{
				CuSystemAmplitude(s)[0] = a_real * u11_real - a_imag * u11_imag;
				CuSystemAmplitude(s)[1] = a_real * u11_imag + a_imag * u11_real;
			}
		}
	};


	void Rot_Bool::cu_operate_diagonal(CuSparseState& state, complex_t u00, complex_t u11) const {
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				RotBool_Diagonal_Functor(id, u00, u11)
			);
		}
		else
		{
			CuCondition_Host_Prepare
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				RotBool_Diagonal_Functor_Control(id, u00, u11, CuCondition_Args)
			);
		}
	}


	struct RotBool_Off_Diagonal_Functor_Control {
		int id;
		double u01_real;
		double u01_imag;
		double u10_real;
		double u10_imag;

		CuCondition_Functor

		RotBool_Off_Diagonal_Functor_Control(int id_, complex_t u01, complex_t u10, CuCondition_Params)
			: id(id_), u01_real(u01.real()), u01_imag(u01.imag()), u10_real(u10.real()), u10_imag(u10.imag()), 
			CuCondition_Init
		{}

			__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				double a_real = CuSystemAmplitude(s)[0];
				double a_imag = CuSystemAmplitude(s)[1];

				if (CuGetAsBool(s, id, 1) == 0)
				{
					CuGet(s, id).value = 1;
					CuSystemAmplitude(s)[0] = a_real * u10_real - a_imag * u10_imag;
					CuSystemAmplitude(s)[1] = a_real * u10_imag + a_imag * u10_real;
				}
				else
				{
					CuGet(s, id).value = 0;
					CuSystemAmplitude(s)[0] = a_real * u01_real - a_imag * u01_imag;
					CuSystemAmplitude(s)[1] = a_real * u01_imag + a_imag * u01_real;
				}
			}
		}
	};

	struct RotBool_Off_Diagonal_Functor {
		int id;
		double u01_real;
		double u01_imag;
		double u10_real;
		double u10_imag;

		RotBool_Off_Diagonal_Functor(int id_, complex_t u01, complex_t u10)
			: id(id_), u01_real(u01.real()), u01_imag(u01.imag()), u10_real(u10.real()), u10_imag(u10.imag()) {
		}

		__host__ __device__ void operator()(System& s) const {
			double a_real = CuSystemAmplitude(s)[0];
			double a_imag = CuSystemAmplitude(s)[1];

			if (CuGetAsBool(s, id, 1) == 0)
			{
				CuGet(s, id).value = 1;
				CuSystemAmplitude(s)[0] = a_real * u10_real - a_imag * u10_imag;
				CuSystemAmplitude(s)[1] = a_real * u10_imag + a_imag * u10_real;
			}
			else
			{
				CuGet(s, id).value = 0;
				CuSystemAmplitude(s)[0] = a_real * u01_real - a_imag * u01_imag;
				CuSystemAmplitude(s)[1] = a_real * u01_imag + a_imag * u01_real;
			}
		}
	};


	void Rot_Bool::cu_operate_off_diagonal(CuSparseState& state, complex_t u01, complex_t u10) const {
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				RotBool_Diagonal_Functor(id, u01, u10)
			);
		}
		else
		{
			CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					RotBool_Diagonal_Functor_Control(id, u01, u10, CuCondition_Args)
				);
		}
	}
	namespace rot_bool_gpu_detail {

		__global__ void Rot_Bool_operate_alone(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, int id,
			double u00_real, double u00_imag, double u01_real, double u01_imag, 
			double u10_real, double u10_imag, double u11_real, double u11_imag) 
		{
			size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
			size_t my_loc;

			if (tid < nsize) {
				my_loc = unq_s[tid].sptr;
				state[old_size + tid] = state[my_loc];

				if (CuGetAsBool(state[my_loc], id, 1) == 0) {
					CuGet(state[old_size + tid], id).value = 1;
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
					CuGet(state[old_size + tid], id).value = 0;
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

		__global__ void Rot_Bool_operate_pair(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, int id,
			double u00_real, double u00_imag, double u01_real, double u01_imag,
			double u10_real, double u10_imag, double u11_real, double u11_imag) 
		{
			size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
			size_t my_loc;
			if (tid < nsize) {
				my_loc = unq_s[tid].sptr;

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
	void Rot_Bool::cu_operate_general(CuSparseState& state, complex_t u00, complex_t u01, complex_t u10, complex_t u11) const
	{
		using namespace rot_bool_gpu_detail;
		SPLIT_BY_CONDITIONS
		{
			state.move_to_gpu();
			SortExceptKey_devfunc(id, state.sparse_state_gpu);

			thrust::device_vector<unq_ele> unique_s;
			Unique_count_elem(state, unique_s, id);
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
			if (num_one > 0)
			{
				s_ptr = unique_s.data().get();
				nblock = (num_one - 1) / blocksize + 1;
				Rot_Bool_operate_alone << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_one, s_ptr, state_size_0, id,
					u00.real(), u00.imag(), u01.real(), u01.imag(), u10.real(), u10.imag(), u11.real(), u11.imag());
			}
			if (num_not_one > 0)
			{
				s_ptr = thrust::raw_pointer_cast(&(unique_s[num_one]));
				nblock = (num_not_one - 1) / blocksize + 1;
				Rot_Bool_operate_pair << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_one, s_ptr, state_size_0, id,
					u00.real(), u00.imag(), u01.real(), u01.imag(), u10.real(), u10.imag(), u11.real(), u11.imag());
			}

			ClearZero()(state);
		}
		MERGE_BY_CONDITIONS
		System::update_max_size(state.size());
	}

	void Rot_Bool::operator()(CuSparseState& state) const {
		profiler _("Rot_Bool cuda");
		if (mask != 1)
			throw_invalid_input("GPU implementation of Rot_Bool only supports Bool register");

		state.move_to_gpu();

		if (_is_diagonal(mat))
		{
			cu_operate_diagonal(state, mat[0], mat[3]);
		}
		else if (_is_off_diagonal(mat))
		{
			cu_operate_off_diagonal(state, mat[1], mat[2]);
		}
		else
		{
			cu_operate_general(state, mat[0], mat[1], mat[2], mat[3]);
		}

	}


	struct FlipBool_Functor_Control {
		size_t id;
		size_t digit;

		CuCondition_Functor

			FlipBool_Functor_Control(size_t id_, size_t digit_, CuCondition_Params)
			: id(id_), digit(digit_), CuCondition_Init
		{}

			__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				CuGet(s, id).value = flip_digit(CuGet(s, id).value, digit);
			}
		}
	};

	struct FlipBool_Functor {
		size_t id;
		size_t digit;

		FlipBool_Functor(size_t id_, size_t digit_)
			: id(id_), digit(digit_) {
		}

		__host__ __device__ void operator()(System& s) const {
			CuGet(s, id).value = flip_digit(CuGet(s, id).value, digit);
		}
	};

	void Xgate_Bool::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				FlipBool_Functor(id, digit)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					FlipBool_Functor_Control(id, digit, CuCondition_Args)
				);
		}
	}

} // namespace qram_simulator

#endif // USE_CUDA