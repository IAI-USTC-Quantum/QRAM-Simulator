#pragma once

#include "quantum_interfere_basic.h"
#include "basic_components.h"
#include "cuda_utils.cuh"
#include "basic_components.cuh"

namespace qram_simulator {

    /* Used to denote the unique_element to partition the state */
    struct unq_ele {
        size_t sptr; /* The position of the first element of the partition */
        unsigned int num; /* The number of elements in the partition */
    };

    /* GPU-version: SortExceptKey() */
    void SortExceptKey_devfunc(int idi, thrust::device_vector<System>& state);
    const thrust::device_vector<size_t>& SortExceptKey_devfunc_logical(int idi, const thrust::device_vector<System>& state);

    // compare two states
    __device__ inline bool compare_equal_dev(const System& a, const System& b, int out_id,
        int mp_sz, uint64_t status_bitmap)
    {
        for (int i = mp_sz - 1; i >= 0; --i)
        {
            if (!((status_bitmap >> i) & 1))
                continue;
            if (i == out_id)
                continue;
            if ((CuGet(a, i).value != CuGet(b, i).value))
                return false;
        }
        return true;
    }

    // find first unique element of a sorted state series
    __global__ void unique_find_elem(System* dat, size_t* num, size_t nsize,
        int regbit, size_t reg_stat, int mpsz, int id);

    // count number of each unique element
    __global__ void unique_count_elem(size_t* dat, unq_ele* uele, size_t nsize, size_t nstate);

    void Unique_count_elem(CuSparseState& state, thrust::device_vector<unq_ele>& uele, int id);
    void Unique_count_elem(CuSparseState& state, const thrust::device_vector<size_t>& indices, thrust::device_vector<unq_ele>& uele, int id);

    HOST_DEVICE inline bool cu_compare_less(const System& lhs, const System& rhs, int mp_num, uint64_t status_bitmap, int id)
    {
        status_bitmap -= pow2(id);
        for (int i = 0; i < mp_num; ++i)
        {
            size_t flag = (status_bitmap >> i) & 1;
            uint64_t value_left = CuGet(lhs, i).value * flag;
            uint64_t value_right = CuGet(rhs, i).value * flag;
            if (value_left < value_right) {
                return true;
            }
            else if (value_left > value_right) {
                return false;
            }
        }
        return CuGet(lhs, id).value < CuGet(rhs, id).value;
    }

    struct CuSystemLessExceptKey {
        int mp_num;
        int id;
        uint64_t status_bitmap;

        CuSystemLessExceptKey(int id_)
            : id(id_), mp_num(System::name_register_map.size()), status_bitmap(System::reg_status_bitmap)
        {
        }

        HOST_DEVICE bool operator()(const System& lhs, const System& rhs)
        {
            return cu_compare_less(lhs, rhs, mp_num, status_bitmap, id);
        };
    };

    struct CuSystemLessExceptKey_Index {
        const System* objects;
        int id;
        int mp_num;
        uint64_t status_bitmap;

        CuSystemLessExceptKey_Index(const System* ptr, int id_)
            : objects(ptr), id(id_), mp_num(System::name_register_map.size()), status_bitmap(System::reg_status_bitmap)
        {
        }

        __host__ __device__ bool operator()(size_t left, size_t right) const {
            const System& lhs = objects[left];
            const System& rhs = objects[right];

            return cu_compare_less(lhs, rhs, mp_num, status_bitmap, id);
        }
    };


    HOST_DEVICE inline bool cu_compare_equal(const System& lhs, const System& rhs, int mp_num, uint64_t status_bitmap)
    {
        for (int i = 0; i < mp_num; ++i)
        {
            size_t flag = (status_bitmap >> i) & 1;
            size_t value_left = CuGet(lhs, i).value * flag;
            size_t value_right = CuGet(rhs, i).value * flag;
            if (value_left != value_right) {
                return false;
            }
        }
        return true;
    }

    struct CuSystemEqualExceptKey {
        int mp_num;
        uint64_t status_bitmap;

        CuSystemEqualExceptKey(int id)
            : mp_num(System::name_register_map.size()), status_bitmap(System::reg_status_bitmap)
        {
            status_bitmap -= pow2(id);
        }

        HOST_DEVICE bool operator()(const System& lhs, const System& rhs)
        {
            return cu_compare_equal(lhs, rhs, mp_num, status_bitmap);
        };
    };

    struct CuSystemEqualExceptKey_Index {
        const System* objects;
        int mp_num;
        uint64_t status_bitmap;

        CuSystemEqualExceptKey_Index(const System* ptr, int id)
            : objects(ptr), mp_num(System::name_register_map.size()), status_bitmap(System::reg_status_bitmap)
        {
            status_bitmap -= pow2(id);
        }

        __host__ __device__ bool operator()(size_t left, size_t right) const {
            const System& lhs = objects[left];
            const System& rhs = objects[right];

            return cu_compare_equal(lhs, rhs, mp_num, status_bitmap);
        }
    };

    HOST_DEVICE inline bool cu_compare_not_equal(const System& lhs, const System& rhs, int mp_num, uint64_t status_bitmap)
    {
        for (int i = 0; i < mp_num; ++i)
        {
            size_t flag = (status_bitmap >> i) & 1;
            size_t value_left = CuGet(lhs, i).value * flag;
            size_t value_right = CuGet(rhs, i).value * flag;
            if (value_left == value_right) {
                return false;
            }
        }
        return true;
    }

    struct CuSystemNotEqualExceptKey {
        int mp_num;
        uint64_t status_bitmap;

        CuSystemNotEqualExceptKey(int id)
            : mp_num(System::name_register_map.size()), status_bitmap(System::reg_status_bitmap)
        {
            status_bitmap -= pow2(id);
        }

        HOST_DEVICE bool operator()(const System& lhs, const System& rhs)
        {
            return cu_compare_not_equal(lhs, rhs, mp_num, status_bitmap);
        };
    };

    struct CuSystemNotEqualExceptKey_Index {
        const System* objects;
        int mp_num;
        uint64_t status_bitmap;

        CuSystemNotEqualExceptKey_Index(const System* ptr, int id)
            : objects(ptr), mp_num(System::name_register_map.size()), status_bitmap(System::reg_status_bitmap)
        {
            status_bitmap -= pow2(id);
        }

        __host__ __device__ bool operator()(size_t left, size_t right) const {
            const System& lhs = objects[left];
            const System& rhs = objects[right];

            return cu_compare_not_equal(lhs, rhs, mp_num, status_bitmap);
        }
    };

    struct CuSystemLessByKey {
        size_t id;

        CuSystemLessByKey(size_t id_) : id(id_) {}

        __host__ __device__ uint64_t operator()(const System& lhs, const System& rhs) const {
            return CuGet(lhs, id).value < CuGet(rhs, id).value;
        }
    };

    struct CuSystemLessByKey_Index {
        const System* objects; // 指向原始数据的指针
        size_t idi;
        CuSystemLessByKey_Index(const System* ptr, size_t id) : objects(ptr), idi(id)
        {
        }

        __host__ __device__ bool operator()(size_t left, size_t right) const {
            const System& lhs = objects[left];
            const System& rhs = objects[right];
            return CuGet(lhs, idi).value < CuGet(rhs, idi).value;
        }
    };

    /* For hash function */
    struct CuStateHashExceptKey {

        size_t name_reg_map_size;
        size_t count_bitmap;

        CuStateHashExceptKey(size_t bitmap)
            : count_bitmap(bitmap)
        {
            name_reg_map_size = System::name_register_map.size();
        }

        __host__ __device__ uint64_t operator()(const System& sys) const {
            const uint64_t prime = 0x9e3779b97f4a7c15;
            uint64_t hash = 0;
            for (size_t i = 0; i < name_reg_map_size; ++i)
            {
                uint64_t new_hash = hash ^ CuGet(sys, i).value;
                new_hash *= prime;

                hash = ((count_bitmap >> i) & 1) ? new_hash : hash;
            }
            return hash;
        }
    };

    /* Sort the unique elements */
    void SortUniqueElements(thrust::device_vector<unq_ele>& dat);

    /* Get the partition point of <1> and <2> */
    size_t EleNum_MoreThanOne(thrust::device_vector<unq_ele>& dat);

    /* Get the partition point of <N-1> */
    size_t EleNum_NotFull(thrust::device_vector<unq_ele>& dat, size_t full_size);
}