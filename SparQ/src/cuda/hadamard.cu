#include "hadamard.h"
#include "system_operations.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"
#include "cuda/quantum_interfere_basic.cuh"

namespace qram_simulator {

    /* ***********************************************************
           Hadamard by Ye C.-C.
     *********************************************************** */   

    namespace hadamard_int_gpu_detail {
        __global__ void Hadamard_Int_Full_operate_not_full(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, int id, size_t n_digits)
        {
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            double amp_real, amp_imag;

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

                for (size_t qn = 0; qn < n_digits; ++qn)
                {
                    size_t step = pow2(qn);
                    for (size_t i = 0; i < full_size; i += 2 * step)
                    {
                        for (size_t j = i; j < i + step; ++j)
                        {
                            double amp0_real = CuSystemAmplitude(state[new_pos + j])[0];
                            double amp0_imag = CuSystemAmplitude(state[new_pos + j])[1];
                            double amp1_real = CuSystemAmplitude(state[new_pos + j + step])[0];
                            double amp1_imag = CuSystemAmplitude(state[new_pos + j + step])[1];

                            CuSystemAmplitude(state[new_pos + j])[0] = (amp0_real + amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j])[1] = (amp0_imag + amp1_imag) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j + step])[0] = (amp0_real - amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j + step])[1] = (amp0_imag - amp1_imag) * sqrt2inv;
                        }
                    }
                }

            }
        }

        __global__ void Hadamard_Int_Full_operate_full(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, int id, size_t n_digits)
        {
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            if (tid < nsize) {
                size_t full_size = pow2(n_digits);
                size_t new_pos = unq_s[tid].sptr;                

                for (size_t qn = 0; qn < n_digits; ++qn)
                {
                    size_t step = pow2(qn);
                    for (size_t i = 0; i < full_size; i += 2 * step)
                    {
                        for (size_t j = i; j < i + step; ++j)
                        {
                            double amp0_real = CuSystemAmplitude(state[new_pos + j])[0];
                            double amp0_imag = CuSystemAmplitude(state[new_pos + j])[1];
                            double amp1_real = CuSystemAmplitude(state[new_pos + j + step])[0];
                            double amp1_imag = CuSystemAmplitude(state[new_pos + j + step])[1];

                            CuSystemAmplitude(state[new_pos + j])[0] = (amp0_real + amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j])[1] = (amp0_imag + amp1_imag) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j + step])[0] = (amp0_real - amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j + step])[1] = (amp0_imag - amp1_imag) * sqrt2inv;
                        }
                    }
                }

            }
        }

        __global__ void Hadamard_Int_Full_operate_not_full(System* state, const size_t *indices, size_t nsize, unq_ele* unq_s, size_t old_size, int id, size_t n_digits)
        {
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            double amp_real, amp_imag;

            if (tid < nsize) {
                size_t full_size = pow2(n_digits);
                size_t my_loc = unq_s[tid].sptr;
                size_t old_state_num = unq_s[tid].num;
                size_t new_pos = old_size + tid * full_size;

                /* init the new state */
                for (size_t i = 0; i < full_size; i++)
                {
                    state[new_pos + i] = state[indices[my_loc]];
                    CuSystemAmplitude(state[new_pos + i])[0] = 0.0;
                    CuSystemAmplitude(state[new_pos + i])[1] = 0.0;
                    CuGet(state[new_pos + i], id).value = i;
                }

                // place the state to the new location
                for (size_t i = 0; i < old_state_num; i++)
                {
                    size_t state_val = CuGet(state[indices[my_loc + i]], id).value;
                    CuSystemAmplitude(state[new_pos + state_val])[0] = CuSystemAmplitude(state[indices[my_loc + i]])[0];
                    CuSystemAmplitude(state[new_pos + state_val])[1] = CuSystemAmplitude(state[indices[my_loc + i]])[1];
                    CuSystemAmplitude(state[indices[my_loc + i]])[0] = 0;
                    CuSystemAmplitude(state[indices[my_loc + i]])[1] = 0;
                }

                for (size_t qn = 0; qn < n_digits; ++qn)
                {
                    size_t step = pow2(qn);
                    for (size_t i = 0; i < full_size; i += 2 * step)
                    {
                        for (size_t j = i; j < i + step; ++j)
                        {
                            double amp0_real = CuSystemAmplitude(state[new_pos + j])[0];
                            double amp0_imag = CuSystemAmplitude(state[new_pos + j])[1];
                            double amp1_real = CuSystemAmplitude(state[new_pos + j + step])[0];
                            double amp1_imag = CuSystemAmplitude(state[new_pos + j + step])[1];

                            CuSystemAmplitude(state[new_pos + j])[0] = (amp0_real + amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j])[1] = (amp0_imag + amp1_imag) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j + step])[0] = (amp0_real - amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[new_pos + j + step])[1] = (amp0_imag - amp1_imag) * sqrt2inv;
                        }
                    }
                }

            }
        }

        __global__ void Hadamard_Int_Full_operate_full(System* state, const size_t* indices, size_t nsize, unq_ele* unq_s, size_t old_size, int id, size_t n_digits)
        {
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            if (tid < nsize) {
                size_t full_size = pow2(n_digits);
                size_t new_pos = unq_s[tid].sptr;

                for (size_t qn = 0; qn < n_digits; ++qn)
                {
                    size_t step = pow2(qn);
                    for (size_t i = 0; i < full_size; i += 2 * step)
                    {
                        for (size_t j = i; j < i + step; ++j)
                        {
                            double amp0_real = CuSystemAmplitude(state[indices[new_pos + j]])[0];
                            double amp0_imag = CuSystemAmplitude(state[indices[new_pos + j]])[1];
                            double amp1_real = CuSystemAmplitude(state[indices[new_pos + j + step]])[0];
                            double amp1_imag = CuSystemAmplitude(state[indices[new_pos + j + step]])[1];

                            CuSystemAmplitude(state[indices[new_pos + j]])[0] = (amp0_real + amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[indices[new_pos + j]])[1] = (amp0_imag + amp1_imag) * sqrt2inv;
                            CuSystemAmplitude(state[indices[new_pos + j + step]])[0] = (amp0_real - amp1_real) * sqrt2inv;
                            CuSystemAmplitude(state[indices[new_pos + j + step]])[1] = (amp0_imag - amp1_imag) * sqrt2inv;
                        }
                    }
                }

            }
        }

    }

    /* ----------------------- Host functions --------------------------*/

    /* GPU-version: Hadamard_Int::operator() */
    void Hadamard_Int::operator()(CuSparseState& state) const
    {
        profiler _("Hadamard_Int cuda");

        state.move_to_gpu();

        if (n_digits == System::size_of(id))
        {
            (Hadamard_Int_Full(id))(state);
            return;
        }

        auto temp_id = SplitRegister(id, "_Hadamard_Int_Temp", n_digits)(state);
        (Hadamard_Int_Full(temp_id))(state);
        CombineRegister(id, temp_id)(state);
    }

    void Hadamard_Int_Full::operator()(CuSparseState& state) const
    {
        profiler _("Hadamard_Int_Full cuda");

        //state.move_to_cpu();
        //(*this)(state.sparse_state_cpu);
        //state.move_to_gpu();
        //return;

        using namespace hadamard_int_gpu_detail;

        size_t n_digits = System::size_of(id);

        SPLIT_BY_CONDITIONS
        {
            state.move_to_gpu();
#define HADAMARD_CUDA_VERSION 2
#if HADAMARD_CUDA_VERSION == 1
            SortExceptKey_devfunc(id, state.sparse_state_gpu);

            thrust::device_vector<unq_ele> unique_s;
            Unique_count_elem(state, unique_s, id);

            size_t unique_state_num = unique_s.size();
            //SortUniqueElements(unique_s);
            size_t full_size = pow2(n_digits);

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
            if (num_not_full > 0)
            {
                s_ptr = unique_s.data().get();
                nblock = (num_not_full - 1) / blocksize + 1;
                Hadamard_Int_Full_operate_not_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_full, s_ptr, state_size_0, id, n_digits);

            }
            if (num_full > 0)
            {
                s_ptr = thrust::raw_pointer_cast(&(unique_s[num_not_full]));
                nblock = (num_full - 1) / blocksize + 1;
                Hadamard_Int_Full_operate_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_full, s_ptr, state_size_0, id, n_digits);
            }
#elif HADAMARD_CUDA_VERSION == 2
            const thrust::device_vector<size_t> &indices = SortExceptKey_devfunc_logical(id, state.sparse_state_gpu);

            thrust::device_vector<unq_ele> unique_s;
            Unique_count_elem(state, indices, unique_s, id);

            size_t unique_state_num = unique_s.size();
            //SortUniqueElements(unique_s);
            size_t full_size = pow2(n_digits);

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
            if (num_not_full > 0)
            {
                s_ptr = unique_s.data().get();
                nblock = (num_not_full - 1) / blocksize + 1;
                Hadamard_Int_Full_operate_not_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), indices.data().get(),
                    num_not_full, s_ptr, state_size_0, id, n_digits);

            }
            if (num_full > 0)
            {
                s_ptr = thrust::raw_pointer_cast(&(unique_s[num_not_full]));
                nblock = (num_full - 1) / blocksize + 1;
                Hadamard_Int_Full_operate_full << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), indices.data().get(),
                    num_full, s_ptr, state_size_0, id, n_digits);
            }
#endif
#undef HADAMARD_CUDA_VERSION

            ClearZero()(state);
        }
        MERGE_BY_CONDITIONS

        System::update_max_size(state.size());
    }
    
    /**************************************************************** 
                       Hadamard_Bool
     *****************************************************************/
    /* GPU-version: SortExceptKey() */
    namespace hadamard_bool_gpu_detail {

        // CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
        __global__ void Hadamard_Bool_operate_alone(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, int id) {
            const double extra_amp = sqrt2inv;
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            double amp_real, amp_imag;
            size_t my_loc;

            if (tid < nsize) {
                my_loc = unq_s[tid].sptr;
                state[old_size + tid] = state[my_loc];

                if (CuGetAsBool(state[my_loc], id, 1) == 0) {
                    CuGet(state[old_size + tid], id).value = 1;
                    amp_real = CuSystemAmplitude(state[my_loc])[0];
                    amp_imag = CuSystemAmplitude(state[my_loc])[1];
                    CuSystemAmplitude(state[my_loc])[0] = amp_real * extra_amp;
                    CuSystemAmplitude(state[my_loc])[1] = amp_imag * extra_amp;
                    CuSystemAmplitude(state[old_size + tid])[0] = amp_real * extra_amp;
                    CuSystemAmplitude(state[old_size + tid])[1] = amp_imag * extra_amp;
                }
                else {
                    CuGet(state[old_size + tid], id).value = 0;
                    amp_real = CuSystemAmplitude(state[my_loc])[0];
                    amp_imag = CuSystemAmplitude(state[my_loc])[1];
                    CuSystemAmplitude(state[my_loc])[0] = amp_real * (-1.0 * extra_amp);
                    CuSystemAmplitude(state[my_loc])[1] = amp_imag * (-1.0 * extra_amp);
                    CuSystemAmplitude(state[old_size + tid])[0] = amp_real * extra_amp;
                    CuSystemAmplitude(state[old_size + tid])[1] = amp_imag * extra_amp;
                }
            }
        }

        // CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
        __global__ void Hadamard_Bool_operate_alone(System* state, const size_t *indices, size_t nsize, unq_ele* unq_s, size_t old_size, int id) {
            const double extra_amp = sqrt2inv;
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            double amp_real, amp_imag;
            size_t my_loc;

            if (tid < nsize) {
                my_loc = unq_s[tid].sptr;
                my_loc = indices[my_loc];
                state[old_size + tid] = state[my_loc];

                if (CuGetAsBool(state[my_loc], id, 1) == 0) {
                    CuGet(state[old_size + tid], id).value = 1;
                    amp_real = CuSystemAmplitude(state[my_loc])[0];
                    amp_imag = CuSystemAmplitude(state[my_loc])[1];
                    CuSystemAmplitude(state[my_loc])[0] = amp_real * extra_amp;
                    CuSystemAmplitude(state[my_loc])[1] = amp_imag * extra_amp;
                    CuSystemAmplitude(state[old_size + tid])[0] = amp_real * extra_amp;
                    CuSystemAmplitude(state[old_size + tid])[1] = amp_imag * extra_amp;
                }
                else {
                    CuGet(state[old_size + tid], id).value = 0;
                    amp_real = CuSystemAmplitude(state[my_loc])[0];
                    amp_imag = CuSystemAmplitude(state[my_loc])[1];
                    CuSystemAmplitude(state[my_loc])[0] = amp_real * (-1.0 * extra_amp);
                    CuSystemAmplitude(state[my_loc])[1] = amp_imag * (-1.0 * extra_amp);
                    CuSystemAmplitude(state[old_size + tid])[0] = amp_real * extra_amp;
                    CuSystemAmplitude(state[old_size + tid])[1] = amp_imag * extra_amp;
                }
            }
        }

        // CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
        __global__ void Hadamard_Bool_operate_pair(System* state, size_t nsize, unq_ele* unq_s, size_t old_size, int id) {
            const double extra_amp = sqrt2inv;
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            double a_amp_real, a_amp_imag;
            double b_amp_real, b_amp_imag;
            size_t my_loc;
            if (tid < nsize) {
                my_loc = unq_s[tid].sptr;

                a_amp_real = CuSystemAmplitude(state[my_loc])[0];
                a_amp_imag = CuSystemAmplitude(state[my_loc])[1];
                b_amp_real = CuSystemAmplitude(state[my_loc + 1])[0];
                b_amp_imag = CuSystemAmplitude(state[my_loc + 1])[1];

                CuSystemAmplitude(state[my_loc])[0] = (a_amp_real + b_amp_real) * extra_amp;
                CuSystemAmplitude(state[my_loc])[1] = (a_amp_imag + b_amp_imag) * extra_amp;
                CuSystemAmplitude(state[my_loc + 1])[0] = (a_amp_real - b_amp_real) * extra_amp;
                CuSystemAmplitude(state[my_loc + 1])[1] = (a_amp_imag - b_amp_imag) * extra_amp;
            }
        }

        // CUDA kernel: Hadamard_Bool::operate_alone_zero + Hadamard_Bool::operate_alone_one
        __global__ void Hadamard_Bool_operate_pair(System* state, const size_t *indices, size_t nsize, unq_ele* unq_s, size_t old_size, int id) {
            const double extra_amp = sqrt2inv;
            size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
            double a_amp_real, a_amp_imag;
            double b_amp_real, b_amp_imag;
            size_t my_loc;
            if (tid < nsize) {
                my_loc = unq_s[tid].sptr;
                auto& state0 = state[indices[my_loc]];
                auto& state1 = state[indices[my_loc + 1]];

                a_amp_real = CuSystemAmplitude(state0)[0];
                a_amp_imag = CuSystemAmplitude(state0)[1];
                b_amp_real = CuSystemAmplitude(state1)[0];
                b_amp_imag = CuSystemAmplitude(state1)[1];

                CuSystemAmplitude(state0)[0] = (a_amp_real + b_amp_real) * extra_amp;
                CuSystemAmplitude(state0)[1] = (a_amp_imag + b_amp_imag) * extra_amp;
                CuSystemAmplitude(state1)[0] = (a_amp_real - b_amp_real) * extra_amp;
                CuSystemAmplitude(state1)[1] = (a_amp_imag - b_amp_imag) * extra_amp;
            }
        }
    }

    // Hadamard_Bool::operator()
    void Hadamard_Bool::operator()(CuSparseState& state) const
    {
        profiler _("Hadamard_Bool cuda");
        state.move_to_cpu();
        (*this)(state.sparse_state_cpu);
        state.move_to_gpu();
        return;
        
        using namespace hadamard_bool_gpu_detail;

        state.move_to_gpu();

        SPLIT_BY_CONDITIONS
        {
            state.move_to_gpu();
#define HADAMARD_BOOL_CUDA_VERSION 2
#if HADAMARD_BOOL_CUDA_VERSION == 1
            {
                profiler _("Hadamard_bool sort_except_key cuda");
                SortExceptKey_devfunc(out_id, state.sparse_state_gpu);
            }

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
            if (num_one > 0)
            {
                s_ptr = unique_s.data().get();
                nblock = (num_one - 1) / blocksize + 1;
                Hadamard_Bool_operate_alone << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_one, s_ptr, state_size_0, out_id);
            }
            if (num_not_one > 0)
            {
                s_ptr = thrust::raw_pointer_cast(&(unique_s[num_one]));
                nblock = (num_not_one - 1) / blocksize + 1;
                Hadamard_Bool_operate_pair << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), num_not_one, s_ptr, state_size_0, out_id);
            }
#elif HADAMARD_BOOL_CUDA_VERSION == 2
            
            profiler *_1 = new profiler("Hadamard_bool sort_except_key cuda");
            const thrust::device_vector<size_t>& indices = SortExceptKey_devfunc_logical(out_id, state.sparse_state_gpu);
            delete _1;

            thrust::device_vector<unq_ele> unique_s;

            {
                profiler _("Hadamard_bool Unique_count_elem cuda");
                Unique_count_elem(state, indices, unique_s, out_id);
            }
            //{
            //    profiler _("Hadamard_bool SortUniqueElements cuda");
            //    //SortUniqueElements(unique_s);
            //}
            size_t num_one;
            {
                profiler _("Hadamard_bool EleNum_MoreThanOne cuda");
                num_one = EleNum_MoreThanOne(unique_s);
            }
            size_t unique_state_num = unique_s.size();
            size_t num_not_one = unique_state_num - num_one;
            /* expand state size */
            size_t state_size_0 = state.sparse_state_gpu.size();

            {
                profiler _("Hadamard_bool resize cuda");
                state.sparse_state_gpu.resize(state_size_0 + num_one);
            }
            unq_ele* s_ptr;
            size_t blocksize = CUDA_BLOCK_SIZE;
            size_t nblock = 0;
            if (num_one > 0)
            {
                profiler _("Hadamard_bool operate_one cuda");
                s_ptr = unique_s.data().get();
                nblock = (num_one - 1) / blocksize + 1;
                Hadamard_Bool_operate_alone << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), indices.data().get(), num_one, s_ptr, state_size_0, out_id);
            }
            if (num_not_one > 0)
            {
                profiler _("Hadamard_bool operate_zero cuda");
                s_ptr = thrust::raw_pointer_cast(&(unique_s[num_one]));
                nblock = (num_not_one - 1) / blocksize + 1;
                Hadamard_Bool_operate_pair << <nblock, blocksize >> > (state.sparse_state_gpu.data().get(), indices.data().get(), num_not_one, s_ptr, state_size_0, out_id);
            }

#endif
#undef HADAMARD_BOOL_CUDA_VERSION

            ClearZero()(state);
        }
        MERGE_BY_CONDITIONS
        System::update_max_size(state.size());
    }

} // namespace qram_simulator
