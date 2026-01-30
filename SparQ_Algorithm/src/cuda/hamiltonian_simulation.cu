#include "hamiltonian_simulation.h"
#include "cuda_utils.cuh"
#include "cuda/sparse_state_simulator.cuh"

namespace qram_simulator {

	namespace CKS {

		namespace cks_condrot_general_gpu_detail
		{
			struct CuWalkAngleFunction {
				size_t mat_data_size;
				bool positive_only;
				CuWalkAngleFunction(size_t mat_data_size, bool positive_only) 
					: mat_data_size(mat_data_size), positive_only(positive_only) {}

				__host__ __device__ void operator()(size_t v, size_t row, size_t col, double* mat) const
				{
					if (positive_only)
						_get_coef_positive_only(mat_data_size, v, row, col, mat);
					else
						_get_coef_common(mat_data_size, v, row, col, mat);
				}
			};

			struct CuWalkAngleFunction_Dag {
				size_t mat_data_size;
				bool positive_only;
				CuWalkAngleFunction_Dag(size_t mat_data_size, bool positive_only) 
					: mat_data_size(mat_data_size), positive_only(positive_only) {}

				__host__ __device__ void operator()(size_t v, size_t row, size_t col, double* mat) const
				{
					if (positive_only)
					{
						_get_coef_positive_only_inv(mat_data_size, v, row, col, mat);
					}
					else
					{
						_get_coef_common_inv(mat_data_size, v, row, col, mat);
					}
				}
			};

			// CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
			template<typename CuWalkAngleFunction>
			__global__ void CondRot_General_Bool_QW_operate_alone(System* state, size_t nsize, unq_ele* unq_s, size_t old_size,
				int j_id, int k_id, int in_id, int out_id, size_t j_size, size_t k_size, size_t in_size,
				CuWalkAngleFunction func)
			{
				size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
				size_t my_loc;

				if (tid < nsize) {
					my_loc = unq_s[tid].sptr;
					state[old_size + tid] = state[my_loc];

					size_t v = CuGetAsUint64(state[my_loc], in_id, in_size);
					size_t row_id = CuGetAsUint64(state[my_loc], j_id, j_size);
					size_t col_id = CuGetAsUint64(state[my_loc], k_id, k_size);

					double mat[8];
					func(v, row_id, col_id, mat);
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

			// CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
			template<typename CuWalkAngleFunction>
			__global__ void CondRot_General_Bool_QW_operate_pair(System* state, size_t nsize, unq_ele* unq_s, size_t old_size,
				int j_id, int k_id, int in_id, int out_id, size_t j_size, size_t k_size, size_t in_size,
				CuWalkAngleFunction func)
			{
				size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
				size_t my_loc;

				if (tid < nsize) {
					my_loc = unq_s[tid].sptr;
					size_t v = CuGetAsUint64(state[my_loc], in_id, in_size);
					size_t row_id = CuGetAsUint64(state[my_loc], j_id, j_size);
					size_t col_id = CuGetAsUint64(state[my_loc], k_id, k_size);

					double mat[8];
					func(v, row_id, col_id, mat);
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

		void CondRot_General_Bool_QW::operator()(CuSparseState& state) const
		{
			profiler _("CondRot_General_Bool_QW cuda");
			//state.move_to_cpu();
			//(*this)(state.sparse_state_cpu);
			//state.move_to_gpu();
			//return;

			using namespace cks_condrot_general_gpu_detail;

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

			size_t j_size = System::size_of(j_id);
			size_t k_size = System::size_of(k_id);
			size_t in_size = System::size_of(in_id);

			CuWalkAngleFunction cu_func(mat->data_size, mat->positive_only);

			if (num_one > 0)
			{
				s_ptr = unique_s.data().get();
				nblock = (num_one - 1) / blocksize + 1;
				CondRot_General_Bool_QW_operate_alone << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_one, s_ptr, state_size_0,
					j_id, k_id, in_id, out_id, j_size, k_size, in_size, cu_func);
			}
			if (num_not_one > 0)
			{
				s_ptr = thrust::raw_pointer_cast(&(unique_s[num_one]));
				nblock = (num_not_one - 1) / blocksize + 1;
				CondRot_General_Bool_QW_operate_pair << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_one, s_ptr, state_size_0,
					j_id, k_id, in_id, out_id, j_size, k_size, in_size, cu_func);
			}

			ClearZero()(state);
			System::update_max_size(state.size());
		}


		void CondRot_General_Bool_QW::dag(CuSparseState& state) const
		{
			profiler _("CondRot_General_Bool_QW cuda");
			using namespace cks_condrot_general_gpu_detail;

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

			size_t j_size = System::size_of(j_id);
			size_t k_size = System::size_of(k_id);
			size_t in_size = System::size_of(in_id);

			CuWalkAngleFunction_Dag cu_func(mat->data_size, mat->positive_only);

			if (num_one > 0)
			{
				s_ptr = unique_s.data().get();
				nblock = (num_one - 1) / blocksize + 1;
				CondRot_General_Bool_QW_operate_alone << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_one, s_ptr, state_size_0,
					j_id, k_id, in_id, out_id, j_size, k_size, in_size, cu_func);
			}
			if (num_not_one > 0)
			{
				s_ptr = thrust::raw_pointer_cast(&(unique_s[num_one]));
				nblock = (num_not_one - 1) / blocksize + 1;
				CondRot_General_Bool_QW_operate_pair << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_one, s_ptr, state_size_0,
					j_id, k_id, in_id, out_id, j_size, k_size, in_size, cu_func);
			}

			ClearZero()(state);
			System::update_max_size(state.size());
		}

		struct GetRowAddrFunctor {
			int offset_id;
			int row_id;
			size_t row_sz;
			int row_offset_id;

			size_t offset_size;
			size_t row_size;

			GetRowAddrFunctor(int offset_id, int row_id, size_t row_sz, int row_offset_id,
				size_t offset_size, size_t row_size)
				: offset_id(offset_id), row_id(row_id), row_sz(row_sz), row_offset_id(row_offset_id),
				offset_size(offset_size), row_size(row_size)
			{
			}

			__host__ __device__ void operator()(System& s) const {

				CuGet(s, row_offset_id).value ^=
					CuGetAsUint64(s, offset_id, offset_size) +
					row_sz * CuGetAsUint64(s, row_id, row_size);
			}
		};

		void GetRowAddr::operator()(CuSparseState& state) const
		{
			state.move_to_gpu();
			size_t offset_size = System::size_of(offset_id);
			size_t row_size = System::size_of(row_id);

			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				GetRowAddrFunctor(offset_id, row_id, row_sz, row_offset_id,
					offset_size, row_size)
			);
		}

		struct GetDataAddrFunctor {

			int offset_id;
			int row_id;
			size_t row_sz;
			int col_sparse_id;
			int row_data_id;

			size_t offset_size;
			size_t row_size;
			size_t col_sparse_size;

			GetDataAddrFunctor(int offset_id, int row_id, size_t row_sz, int col_sparse_id, int row_data_id,
				size_t offset_size, size_t row_size, size_t col_sparse_size)
				: offset_id(offset_id), row_id(row_id), row_sz(row_sz), col_sparse_id(col_sparse_id), row_data_id(row_data_id),
				offset_size(offset_size), row_size(row_size), col_sparse_size(col_sparse_size)
			{}

			__host__ __device__ void operator()(System& s) const {

				CuGet(s, row_data_id).value ^=
					CuGetAsUint64(s, offset_id, offset_size) +
					row_sz * CuGetAsUint64(s, row_id, row_size) +
					CuGetAsUint64(s, col_sparse_id, col_sparse_size);
			}
		};

		void GetDataAddr::operator()(CuSparseState& state) const
		{
			state.move_to_gpu();
			size_t offset_size = System::size_of(offset_id);
			size_t row_size = System::size_of(row_id);
			size_t col_sparse_size = System::size_of(col_sparse_id);

			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				GetDataAddrFunctor(offset_id, row_id, row_sz, col_sparse_id, row_data_id,
					offset_size, row_size, col_sparse_size)
			);
		}

		struct QuantumBinarySearchFastFunctor {
			size_t total_length;
			size_t max_step;
			size_t address_offset_id;
			size_t target_id;
			size_t result_id;

			size_t address_offset_size;
			size_t target_size;

			size_t* mem;

			QuantumBinarySearchFastFunctor(size_t total_length, size_t max_step, size_t address_offset_id, size_t target_id, size_t result_id,
				size_t address_offset_size, size_t target_size, size_t* mem)
				: total_length(total_length), max_step(max_step), address_offset_id(address_offset_id), target_id(target_id), result_id(result_id),
				address_offset_size(address_offset_size), target_size(target_size), mem(mem)
			{}

			__host__ __device__ size_t binary_search(uint64_t offset, uint64_t target) const
			{
				size_t l = offset;
				size_t r = l + total_length;
				size_t mid;
				for (size_t i = 0; i < max_step; ++i)
				{
					mid = (l + r) / 2;
					if (mem[mid] == target)
						return mid;
					else if (mem[mid] < target)
						l = mid;
					else
						r = mid;
				}
				return 0;
			}


			__host__ __device__ void operator()(System& s) const {
				uint64_t offset = CuGetAsUint64(s, address_offset_id, address_offset_size);
				uint64_t target = CuGetAsUint64(s, target_id, target_size);

				size_t result = binary_search(offset, target);
				CuGet(s, result_id).value ^= result;
			}
		};

		void QuantumBinarySearchFast::operator()(CuSparseState& state) const
		{
			profiler _("QBS_Fast");
			state.move_to_gpu();
			size_t address_offset_size = System::size_of(address_offset_id);
			size_t target_size = System::size_of(target_id);

			auto cu_qram = dynamic_cast<qram_qutrit::CuQRAMCircuit*>(qram);
			if (cu_qram == nullptr) {
				throw std::runtime_error("QRAM is not a CuQRAMCircuit.");
			}

			size_t* mem = cu_qram->memory_dev.data().get();

			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				QuantumBinarySearchFastFunctor(total_length, max_step, address_offset_id, target_id, result_id,
					address_offset_size, target_size, mem)
			);
		}

		CU_IMPL(QuantumBinarySearch)
		CU_IMPL(SparseMatrixOracle1)
		CU_IMPL(SparseMatrixOracle2)
		CU_IMPL(T)
		CU_IMPL(QuantumWalk)
	}
}