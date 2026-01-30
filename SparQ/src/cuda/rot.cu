#include "rot.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"
#include "cuda/quantum_interfere_basic.cuh"

namespace qram_simulator {

    namespace rot_general_unitary_gpu_detail {

        __global__ void Rot_GeneralUnitary_Full_operate_not_full(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, 
            int id, size_t n_digits, thrust::complex<double>* mat)
        {
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;

            if (tid < nsize) {
                size_t full_size = pow2(n_digits);
                size_t my_loc = unq_s[tid].sptr;
                size_t old_state_num = unq_s[tid].num;
                size_t new_pos = old_size + tid * full_size;

                /* init the new state */
                for (size_t i = 0; i < full_size; i++)
                {
                    state[new_pos + i] = state[my_loc];
                    CuSystemAmplitude(state[new_pos + i])[0] = 0.0;
                    CuSystemAmplitude(state[new_pos + i])[1] = 0.0;
                    CuGet(state[new_pos + i], id).value = i;
                }

                // place the state to the new location
                for (size_t i = 0; i < old_state_num; i++)
                {
                    size_t state_val = CuGet(state[my_loc + i], id).value;
                    CuSystemAmplitude(state[new_pos + state_val])[0] = CuSystemAmplitude(state[my_loc + i])[0];
                    CuSystemAmplitude(state[new_pos + state_val])[1] = CuSystemAmplitude(state[my_loc + i])[1];
                    CuSystemAmplitude(state[my_loc + i])[0] = 0;
                    CuSystemAmplitude(state[my_loc + i])[1] = 0;
                }

                thrust::complex<double> tmp_mem[16];
                for (size_t i = 0; i < full_size; ++i)
                {
                    tmp_mem[i] = 0.0;

                    for (size_t j = 0; j < full_size; ++j)
                    {
                        double a_real = CuSystemAmplitude(state[new_pos + j])[0];
                        double a_imag = CuSystemAmplitude(state[new_pos + j])[1];
                        cu_complex_t a(a_real, a_imag);
                        cu_complex_t mat_ij = mat[(i * full_size + j)];
                        tmp_mem[i] += a * mat_ij;
                    }
                }

                for (size_t i = 0; i < full_size; ++i)
                {
                    CuSystemAmplitude(state[new_pos + i])[0] = tmp_mem[i].real();
                    CuSystemAmplitude(state[new_pos + i])[1] = tmp_mem[i].imag();
                }
            }
        }

        __global__ void Rot_GeneralUnitary_Full_operate_full(System* state, size_t nsize, unq_ele* unq_s, size_t old_size,
            int id, size_t n_digits, thrust::complex<double>* mat)
        {
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            if (tid < nsize) {
                size_t full_size = pow2(n_digits);
                size_t my_loc = unq_s[tid].sptr;
                // size_t old_state_num = unq_s[tid].num;
                //size_t new_pos = old_size + tid * full_size;
                size_t new_pos = my_loc;

                thrust::complex<double> tmp_mem[16];
                for (size_t i = 0; i < full_size; ++i)
                {
                    tmp_mem[i] = 0;
                    for (size_t j = 0; j < full_size; ++j)
                    {
                        double a_real = CuSystemAmplitude(state[new_pos + j])[0];
                        double a_imag = CuSystemAmplitude(state[new_pos + j])[1];
                        cu_complex_t a(a_real, a_imag);
                        cu_complex_t mat_ij = mat[(i * full_size + j)];
                        tmp_mem[i] += a * mat_ij;
                    }
                }

                for (size_t i = 0; i < full_size; ++i)
                {
                    CuSystemAmplitude(state[new_pos + i])[0] = tmp_mem[i].real();
                    CuSystemAmplitude(state[new_pos + i])[1] = tmp_mem[i].imag();
                }
            }
        }
    }

	void Rot_GeneralUnitary::operator()(CuSparseState& state) const
	{
        using namespace rot_general_unitary_gpu_detail;
        profiler _("Rot_GeneralUnitary cuda");
        state.move_to_gpu();
        size_t n_digits = System::size_of(id);

        SPLIT_BY_CONDITIONS
        {
            state.move_to_gpu();
            SortExceptKey_devfunc(id, state.sparse_state_gpu);

            thrust::device_vector<unq_ele> unique_s;
            Unique_count_elem(state, unique_s, id);

            size_t unique_state_num = unique_s.size();
            //SortUniqueElements(unique_s);

            size_t num_not_full = EleNum_NotFull(unique_s, full_size);
            size_t num_full = unique_state_num - num_not_full;

            /* old size before operator() */
            size_t state_size_0 = state.sparse_state_gpu.size();
            size_t total_expand_size = full_size * num_not_full;

            /* expand state size */
            state.sparse_state_gpu.resize(state_size_0 + total_expand_size);

            /* new size after expanding */
            size_t state_size_1 = state.size();

            unq_ele* s_ptr;
            size_t blocksize = CUDA_BLOCK_SIZE;
            size_t nblock = 0;

            /* Transfer the matrix to gpu */
            thrust::device_vector<thrust::complex<double>> mat_gpu(mat.data.size());
            thrust::copy(mat.data.begin(), mat.data.end(), mat_gpu.begin());
            thrust::complex<double>* mat_ptr = mat_gpu.data().get();

            if (num_not_full > 0)
            {
                s_ptr = unique_s.data().get();
                nblock = (num_not_full - 1) / blocksize + 1;
                Rot_GeneralUnitary_Full_operate_not_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_full, s_ptr, 
                    state_size_0, id, n_digits, mat_ptr);
            }
            if (num_full > 0)
            {
                s_ptr = thrust::raw_pointer_cast(&(unique_s[num_not_full]));
                nblock = (num_full - 1) / blocksize + 1;
                Rot_GeneralUnitary_Full_operate_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_full, s_ptr, 
                    state_size_0, id, n_digits, mat_ptr);
            }
            ClearZero()(state);
        }
        MERGE_BY_CONDITIONS

        System::update_max_size(state.size());
	}

	void Rot_GeneralUnitary::dag(CuSparseState& state) const
	{
        using namespace rot_general_unitary_gpu_detail;
        profiler _("Rot_GeneralUnitary cuda");
        state.move_to_gpu();
        size_t n_digits = System::size_of(id);

        SPLIT_BY_CONDITIONS
        {
            state.move_to_gpu();
            SortExceptKey_devfunc(id, state.sparse_state_gpu);

            thrust::device_vector<unq_ele> unique_s;
            Unique_count_elem(state, unique_s, id);

            size_t unique_state_num = unique_s.size();
            //SortUniqueElements(unique_s);

            size_t num_not_full = EleNum_NotFull(unique_s, full_size);
            size_t num_full = unique_state_num - num_not_full;

            /* old size before operator() */
            size_t state_size_0 = state.sparse_state_gpu.size();
            size_t total_expand_size = full_size * num_not_full;

            /* expand state size */
            state.sparse_state_gpu.resize(state_size_0 + total_expand_size);

            /* new size after expanding */
            size_t state_size_1 = state.size();

            unq_ele* s_ptr;
            size_t blocksize = CUDA_BLOCK_SIZE;
            size_t nblock = 0;

            /* Transfer the matrix to gpu */
            thrust::device_vector<thrust::complex<double>> mat_gpu(mat.data.size());

            auto mat_dag = dagger(mat);

            thrust::copy(mat_dag.data.begin(), mat_dag.data.end(), mat_gpu.begin());
            thrust::complex<double>* mat_ptr = mat_gpu.data().get();

            if (num_not_full > 0)
            {
                s_ptr = unique_s.data().get();
                nblock = (num_not_full - 1) / blocksize + 1;
                Rot_GeneralUnitary_Full_operate_not_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_full, s_ptr,
                    state_size_0, id, n_digits, mat_ptr);
            }
            if (num_full > 0)
            {
                s_ptr = thrust::raw_pointer_cast(&(unique_s[num_not_full]));
                nblock = (num_full - 1) / blocksize + 1;
                Rot_GeneralUnitary_Full_operate_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_full, s_ptr,
                    state_size_0, id, n_digits, mat_ptr);
            }
            ClearZero()(state);
        }
            MERGE_BY_CONDITIONS

            System::update_max_size(state.size());
	}

    void Rot_GeneralStatePrep::operator()(CuSparseState& state) const
    {
        rot_general(state);
    }

    void Rot_GeneralStatePrep::dag(CuSparseState& state) const
    {
        rot_general.dag(state);
    }

} // namespace qram_simulator
