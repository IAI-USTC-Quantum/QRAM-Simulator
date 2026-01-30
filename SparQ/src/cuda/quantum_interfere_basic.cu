#include "quantum_interfere_basic.h"
#include "cuda_utils.cuh"
#include "cuda/quantum_interfere_basic.cuh"

#ifdef USE_CUDA

namespace qram_simulator {

    void SortExceptKey_devfunc(int idi, thrust::device_vector<System>& state)
    {
        profiler _("SortExceptKey_devfunc");

#define DEV_SORT_VERSION 1
#if DEV_SORT_VERSION == 1
        thrust::sort(thrust::device, state.begin(), state.end(), CuSystemLessExceptKey(idi));
#elif DEV_SORT_VERSION == 2
        
        thrust::stable_sort(thrust::device, state.begin(), state.end(), CuSystemLessByKey(idi));
        for (size_t i = 0; i < System::name_register_map.size(); ++i)
        {
            if (System::status_of(i) && i != idi)
                thrust::stable_sort(thrust::device, state.begin(), state.end(), CuSystemLessByKey(i));

        }
#elif DEV_SORT_VERSION == 3
        
        static thrust::device_vector<uint64_t> hash_vec(204800);

        hash_vec.resize(state.size());

        size_t mp_num = System::name_register_map.size();
        size_t status_bitmap = 0;
        for (size_t i = 0; i < System::name_register_map.size(); ++i)
        {
            if (System::status_of(i) && i != idi)
                status_bitmap |= pow2(i);
        }
        thrust::transform(state.begin(), state.end(), hash_vec.begin(), CuStateHashExceptKey(status_bitmap));

        auto pred = [idi, mp_num, status_bitmap] HOST_DEVICE(const thrust::tuple<uint64_t, System>&left, const thrust::tuple<uint64_t, System>&right)
        {
            if (thrust::get<0>(left) < thrust::get<0>(right))
                return true;
            else if (thrust::get<0>(left) > thrust::get<0>(right))
                return false;
            else
            {
                const System& lhs = thrust::get<1>(left);
                const System& rhs = thrust::get<1>(right);
                for (int i = 0; i < mp_num; ++i)
                {
                    size_t flag = (status_bitmap >> i) & 1;
                    size_t value_left = CuGet(lhs, i).value * flag;
                    size_t value_right = CuGet(rhs, i).value * flag;
                    if (value_left < value_right) {
                        return true;
                    }
                    else if (value_left > value_right) {
                        return false;
                    }
                }
                return CuGet(lhs, idi).value < CuGet(rhs, idi).value;
            }
        };

        thrust::sort_by_key(thrust::device, 
            thrust::make_zip_iterator(hash_vec.begin(), state.begin()),
            thrust::make_zip_iterator(hash_vec.end(), state.end()),
            state.begin(), pred);

#endif
#undef DEV_SORT_VERSION

        cudaDeviceSynchronize();        
    }

    const thrust::device_vector<size_t>& SortExceptKey_devfunc_logical(int idi, const thrust::device_vector<System>& state)
    {
        profiler _("SortExceptKey_devfunc logical");
        static thrust::device_vector<size_t> indices(320000);
        indices.resize(state.size());

        thrust::sequence(indices.begin(), indices.end());
        const System* raw_ptr = thrust::raw_pointer_cast(state.data());
#define DEV_SORT_VERSION 1
#if DEV_SORT_VERSION == 1
        thrust::sort(indices.begin(), indices.end(), CuSystemLessExceptKey_Index(raw_ptr, idi));
#elif DEV_SORT_VERSION == 2

        thrust::stable_sort(thrust::device, indices.begin(), indices.end(), CuSystemLessByKey_Index(raw_ptr, idi));
        for (size_t i = 0; i < System::name_register_map.size(); ++i)
        {
            if (System::status_of(i) && i != idi)
                thrust::stable_sort(thrust::device, indices.begin(), indices.end(), CuSystemLessByKey_Index(raw_ptr, i));
        }
#endif
#undef DEV_SORT_VERSION

        cudaDeviceSynchronize();
        return indices;
    }

    __global__ void unique_find_elem(System* dat, size_t* num, size_t nsize, uint64_t reg_stat, int mpsz, int id)
    {
        int tIdx = threadIdx.x;
        int bIdx = blockIdx.x;
        int bSz = blockDim.x;
        int tid = bIdx * bSz + tIdx;

        extern __shared__ size_t shmem_dat[];

        //size_t * shmem = (size_t*) (&shmem_dat[0]);
        size_t* ids = (size_t*)(&shmem_dat[bSz]);

        ids[tIdx] = ULONG_MAX;

        /* check difference pair */
        bool iseq;
        if (tIdx > 0 && tid > 0 && tid < nsize) {
            iseq = compare_equal_dev(dat[tid], dat[tid - 1], id, mpsz, reg_stat);
            if (!(iseq)) {
                ids[tIdx] = tid;
            }
        }

        /* check head */
        if (tIdx == 0) {
            if (bIdx > 0) {
                iseq = compare_equal_dev(dat[tid], dat[tid - 1], id, mpsz, reg_stat);
                if (!(iseq)) {
                    ids[0] = tid;
                }
            }
            else {
                ids[0] = tid;
            }
        }
        __syncthreads();

        if (tid < nsize) {
            num[tid] = ids[tIdx];
        }
    }

    __global__ void unique_find_elem(System* dat, const size_t* indices, size_t* num, size_t nsize, uint64_t reg_stat, int mpsz, int id)
    {
        int tIdx = threadIdx.x;
        int bIdx = blockIdx.x;
        int bSz = blockDim.x;
        int tid = bIdx * bSz + tIdx;
        size_t idx0 = indices[tid];

        extern __shared__ size_t shmem_dat[];

        //size_t * shmem = (size_t*) (&shmem_dat[0]);
        size_t* ids = (size_t*)(&shmem_dat[bSz]);

        ids[tIdx] = ULONG_MAX;

        /* check difference pair */
        bool iseq;
        if (tIdx > 0 && tid > 0 && tid < nsize) {
            size_t idx1 = indices[tid - 1];
            iseq = compare_equal_dev(dat[idx0], dat[idx1], id, mpsz, reg_stat);
            if (!(iseq)) {
                ids[tIdx] = tid;
            }
        }

        /* check head */
        if (tIdx == 0) {
            if (bIdx > 0) {
                size_t idx1 = indices[tid - 1];
                iseq = compare_equal_dev(dat[idx0], dat[idx1], id, mpsz, reg_stat);
                if (!(iseq)) {
                    ids[0] = tid;
                }
            }
            else {
                ids[0] = tid;
            }
        }
        __syncthreads();

        if (tid < nsize) {
            num[tid] = ids[tIdx];
        }
    }


    __global__ void unique_count_elem(size_t* dat,
        unq_ele* uele,
        size_t     nsize,
        size_t     nstate
    )
    {
        int tIdx = threadIdx.x;
        int bIdx = blockIdx.x;
        int bSz = blockDim.x;
        int tid = bIdx * bSz + tIdx;

        extern __shared__ size_t shmem[];

        size_t te;
        if (tid < nsize) {
            shmem[tIdx] = dat[tid];
        }

        if (tid < (nsize - 1)) {
            te = dat[tid + 1];
        }
        else {
            te = nstate;
        }
        __syncthreads();

        // write start pointer
        if (tid < nsize) {
            uele[tid].sptr = shmem[tIdx];
        }

        // compute element number
        if (tid < (nsize - 1)) {
            if (tIdx < (bSz - 1)) {
                uele[tid].num = shmem[tIdx + 1] - shmem[tIdx];
            }
            else {
                uele[tid].num = te - shmem[tIdx];
            }
        }

        if (tid == (nsize - 1)) {
            uele[tid].num = te - shmem[tIdx];
        }

    }

    void Unique_count_elem(CuSparseState& state, thrust::device_vector<unq_ele>& uele, int id)
    {
        profiler _("Unique_count_elem");
        state.move_to_gpu();
        size_t blocksize = CUDA_BLOCK_SIZE;
        size_t nblock;
        size_t state_size = state.sparse_state_gpu.size();

        if (state_size == 0)
            return;

        if (state_size == 1) {
            uele.resize(1);
            unq_ele tmp;
            tmp.num = 1;
            tmp.sptr = 0;
            uele[0] = tmp;            
            return;
        }
        
#define UNIQUE_COUNT_ELEM_VERSION 1
#if UNIQUE_COUNT_ELEM_VERSION == 1
        thrust::device_vector<size_t> dev_sizet_pub(state_size);
        size_t* pub_sizet_ptr = dev_sizet_pub.data().get();

        int mp_num = System::name_register_map.size();
        size_t status_bitmap = System::reg_status_bitmap;

        System* SystemDevPtr = state.sparse_state_gpu.data().get();

        nblock = (state_size - 1) / blocksize + 1;
        unique_find_elem <<< nblock, blocksize, 2 * blocksize * sizeof(size_t) >>>
            (SystemDevPtr, pub_sizet_ptr, state_size, status_bitmap, mp_num, id);

        // remove useless eles
        auto pred = []__host__ __device__(const size_t & a) {
            return a == ULONG_MAX;
        };
        auto iter = thrust::remove_if(thrust::device, dev_sizet_pub.begin(), dev_sizet_pub.end(), pred);
        dev_sizet_pub.erase(iter, dev_sizet_pub.end());
        size_t unique_num = dev_sizet_pub.size();

        // count number
        uele.resize(unique_num);
        unq_ele* s_ptr = uele.data().get();
        nblock = (unique_num - 1) / blocksize + 1;
        unique_count_elem <<< nblock, blocksize, blocksize * sizeof(size_t) >>>
            (pub_sizet_ptr, s_ptr, unique_num, state_size);
#elif UNIQUE_COUNT_ELEM_VERSION == 2

        // create an alias
        auto& v = state.sparse_state_gpu;

        // Create a vector to mark the start of each group (1 = start of group, 0 = not start)
        thrust::device_vector<size_t> group_flags(state_size);
        thrust::sequence(group_flags.begin(), group_flags.end());

        uint64_t status_bitmap = System::reg_status_bitmap - pow2(id);
        int mp_num = System::name_register_map.size();

        thrust::for_each(thrust::device,
            thrust::make_zip_iterator(v.begin(), v.begin() + 1, group_flags.begin() + 1),
            thrust::make_zip_iterator(v.end() - 1, v.end(), group_flags.end()),
            [mp_num, status_bitmap] HOST_DEVICE (thrust::tuple<const System&, const System&, size_t&> t)
        {
            const System& lhs = thrust::get<0>(t);
            const System& rhs = thrust::get<1>(t);
            size_t& val = thrust::get<2>(t);
            if (cu_compare_equal(lhs, rhs, mp_num, status_bitmap)) {
                val = ULONG_MAX;
            }
        });

        // Count the number of groups
        auto iter = thrust::remove_if(group_flags.begin(), group_flags.end(),
            [] __device__(size_t flag) { return flag == ULONG_MAX; }
        );
        size_t unique_num = thrust::distance(group_flags.begin(), iter);

        // count number
        uele.resize(unique_num - 1);
        thrust::for_each(thrust::device,
            thrust::make_zip_iterator(group_flags.begin(), group_flags.begin() + 1, uele.begin()),
            thrust::make_zip_iterator(group_flags.begin() + unique_num - 1, group_flags.begin() + unique_num, uele.end() - 1),
            [] HOST_DEVICE (thrust::tuple<size_t, size_t, unq_ele&> t)
        {
            size_t pos1 = thrust::get<0>(t);
            size_t pos2 = thrust::get<1>(t);
            unq_ele& ue = thrust::get<2>(t);
            ue.num = pos2 - pos1;
            ue.sptr = pos1;
        });

        unq_ele last;
        iter--;
        last.sptr = *iter;
        last.num = state_size - *iter;
        uele.push_back(last);

#endif
#undef UNIQUE_COUNT_ELEM_VERSION
        cudaDeviceSynchronize();
    }

    /* From logical indices */
    void Unique_count_elem(CuSparseState& state, const thrust::device_vector<size_t> &indices, thrust::device_vector<unq_ele>& uele, int id)
    {
        profiler _("Unique_count_elem");
        state.move_to_gpu();
        size_t blocksize = CUDA_BLOCK_SIZE;
        size_t nblock;
        size_t state_size = state.sparse_state_gpu.size();

        if (state_size == 0)
            return;

        if (state_size == 1) {
            uele.resize(1);
            unq_ele tmp;
            tmp.num = 1;
            tmp.sptr = 0;
            uele[0] = tmp;
            return;
        }

        thrust::device_vector<size_t> dev_sizet_pub(state_size);
        size_t* pub_sizet_ptr = dev_sizet_pub.data().get();

        int mp_num = System::name_register_map.size();
        //size_t status_bitmap = 0;
        //for (size_t i = 0; i < System::name_register_map.size(); ++i)
        //{
        //    if (System::status_of(i))
        //        status_bitmap |= pow2(i);
        //}
        size_t status_bitmap = System::reg_status_bitmap;

        System* SystemDevPtr = state.sparse_state_gpu.data().get();
        const size_t* indices_ptr = indices.data().get();

        nblock = (state_size - 1) / blocksize + 1;
        unique_find_elem << < nblock, blocksize, 2 * blocksize * sizeof(size_t) >> >
            (SystemDevPtr, indices_ptr, pub_sizet_ptr, state_size, status_bitmap, mp_num, id);

        // remove useless eles
        auto pred = []__host__ __device__(const size_t & a) {
            return a == ULONG_MAX;
        };
        auto iter = thrust::remove_if(thrust::device, dev_sizet_pub.begin(), dev_sizet_pub.end(), pred);
        dev_sizet_pub.erase(iter, dev_sizet_pub.end());
        size_t unique_num = dev_sizet_pub.size();

        // count number
        uele.resize(unique_num);
        unq_ele* s_ptr = uele.data().get();
        nblock = (unique_num - 1) / blocksize + 1;
        unique_count_elem << < nblock, blocksize, blocksize * sizeof(size_t) >> >
            (pub_sizet_ptr, s_ptr, unique_num, state_size);

        cudaDeviceSynchronize();
    }

    void SortUniqueElements(thrust::device_vector<unq_ele>& dat)
    {
        profiler _("SortUniqueElements");
        auto pred = []__host__ __device__(const unq_ele & lhs, const unq_ele & rhs)
        {
            return lhs.num < rhs.num;
        };

        thrust::sort(thrust::device, dat.begin(), dat.end(), pred);
        cudaDeviceSynchronize();
    }

    /* Get the partition point of <1> and <2> */
    size_t EleNum_MoreThanOne(thrust::device_vector<unq_ele>& dat) {
        profiler _("EleNum_MoreThanOne");

        auto pred = []__host__ __device__(const unq_ele & ele)
        {
            //if (ele.num == 1)
            //    return true;
            //else
            //    return false;
            return ele.num == 1;
        };
        auto iter = thrust::partition(thrust::device, dat.begin(), dat.end(), pred);
        size_t num_not_one = thrust::distance(dat.begin(), iter);

        cudaDeviceSynchronize();
        return num_not_one;
    }

    /* Get the partition point of <1> and <2> */
    size_t EleNum_NotFull(thrust::device_vector<unq_ele>& dat, size_t full_size) {
        profiler _("EleNum_NotFull");

        auto pred = [full_size]__host__ __device__(const unq_ele & ele)
        {
            //if (ele.num < full_size)
            //    return true;
            //else
            //    return false;
            return ele.num < full_size;
        };
        auto iter = thrust::partition(thrust::device, dat.begin(), dat.end(), pred);
        size_t num_not_full = thrust::distance(dat.begin(), iter);

        cudaDeviceSynchronize();
        return num_not_full;
    }
} // namespace qram_simulator

#endif // USE_CUDA