#pragma once

#include "basic_components.h"

namespace qram_simulator {

	inline __host__ __device__ uint64_t cu_as_uint64(const StateStorage& storage, size_t size) {
		uint64_t value = storage.value;
		uint64_t mask = (1ULL << size) - (size != 0);
		return value & mask;
	}
	inline __host__ __device__ double cu_as_double(const StateStorage& storage, size_t size) {
		uint64_t value = storage.value;
		uint64_t mask = (1ULL << size) - (size != 0);
		value &= mask;
		return value / 2.0 / (1ULL << (size - 1));
	}
	inline __host__ __device__ bool cu_as_bool(const StateStorage& storage, size_t size) {
		uint64_t value = storage.value;
		uint64_t mask = (1ULL << size) - (size != 0);
		value &= mask;
		return bool(value);
	}
	inline __host__ __device__ int64_t cu_as_int64(const StateStorage& storage, size_t size) {
		uint64_t value = storage.value;
		uint64_t mask = (1ULL << size) - (size != 0);
		value &= mask;
		return size ? (int64_t)(value << (64 - size)) >> (64 - size) : 0;
	}
	inline __host__ __device__ StateStorage& CuGet(System& system, size_t index)
	{
		return (reinterpret_cast<StateStorage*>(&system.registers))[index];
	}
	inline __host__ __device__ StateStorage CuGet(const System& system, size_t index)
	{
		return (reinterpret_cast<const StateStorage*>(&system.registers))[index];
	}
	inline __host__ __device__ uint64_t CuGetAsUint64(const System& system, size_t index, size_t size)
	{
		return cu_as_uint64(CuGet(system, index), size);
	}
	inline __host__ __device__ double CuGetAsDouble(const System& system, size_t index, size_t size)
	{
		return cu_as_double(CuGet(system, index), size);
	}
	inline __host__ __device__ bool CuGetAsBool(const System& system, size_t index, size_t size)
	{
		return cu_as_bool(CuGet(system, index), size);
	}
	inline __host__ __device__ int64_t CuGetAsInt64(const System& system, size_t index, size_t size)
	{
		return cu_as_int64(CuGet(system, index), size);
	}
	inline __host__ __device__ double* CuSystemAmplitude(System& system)
	{
		return reinterpret_cast<double*>(&system.amplitude);
	}
	inline __host__ __device__ const double* CuSystemAmplitude(const System& system)
	{
		return reinterpret_cast<const double*>(&system.amplitude);
	}
	inline __host__ __device__ double& cu_real(complex_t& c)
	{
		return reinterpret_cast<double*>(&c)[0];
	}
	inline __host__ __device__ double cu_real(const complex_t& c)
	{
		return reinterpret_cast<const double*>(&c)[0];
	}
	inline __host__ __device__ double& cu_imag(complex_t& c)
	{
		return reinterpret_cast<double*>(&c)[1];
	}
	inline __host__ __device__ double cu_imag(const complex_t& c)
	{
		return reinterpret_cast<const double*>(&c)[1];
	}
	inline __host__ __device__ double CuAbsSqr(const System& s)
	{
		const double* amplitude = CuSystemAmplitude(s);
		return amplitude[0] * amplitude[0] + amplitude[1] * amplitude[1];
	}


	struct CuSparseState
	{
		using vector_type = std::vector<System>;

		std::vector<System> sparse_state_cpu;
		thrust::device_vector<size_t> gpu_indices;
		thrust::device_vector<System> sparse_state_gpu;

		bool _on_gpu = false;

		CuSparseState();
		CuSparseState(size_t size);
		
		CuSparseState(const CuSparseState& other)
		{
			if (other._on_gpu)
			{
				sparse_state_gpu = other.sparse_state_gpu;
				_on_gpu = true;
			}
			else
			{
				sparse_state_cpu = other.sparse_state_cpu;
				_on_gpu = false;
			}
		}
		CuSparseState(CuSparseState&& other)
		{
			if (other._on_gpu)
			{
				sparse_state_gpu = std::move(other.sparse_state_gpu);
				_on_gpu = true;
			}
			else
			{
				sparse_state_cpu = std::move(other.sparse_state_cpu);
				_on_gpu = false;
			}
		}
		CuSparseState& operator=(const CuSparseState& other)
		{
			if (other._on_gpu)
			{
				sparse_state_gpu = other.sparse_state_gpu;
				_on_gpu = true;
			}
			else
			{
				sparse_state_cpu = other.sparse_state_cpu;
				_on_gpu = false;
			}
			return *this;
		}
		CuSparseState& operator=(CuSparseState&& other)
		{
			if (other._on_gpu)
			{
				sparse_state_gpu = std::move(other.sparse_state_gpu);
				_on_gpu = true;
			}
			else
			{
				sparse_state_cpu = std::move(other.sparse_state_cpu);
				_on_gpu = false;
			}
			return *this;
		}
		CuSparseState(const SparseState& other)
		{
			sparse_state_cpu = other.basis_states;
			_on_gpu = false;
		}
		CuSparseState(const std::vector<System>& other)
		{
			sparse_state_cpu = other;
			_on_gpu = false;
		}
		CuSparseState(const thrust::device_vector<System>& other)
		{
			sparse_state_gpu = other;
			_on_gpu = true;
		}
		CuSparseState(thrust::device_vector<System>::iterator begin, thrust::device_vector<System>::iterator end)
		{
			sparse_state_gpu = thrust::device_vector<System>(begin, end);
			_on_gpu = true;
		}
		void move_to_cpu();
		void copy_to_cpu();
		void move_to_gpu();
		std::vector<System> get_cpu_copy() const;

		bool on_gpu() const;
		bool on_cpu() const;
		bool empty() const;
		size_t size() const;
		
		System& back() { copy_to_cpu(); return sparse_state_cpu.back(); }
		vector_type::iterator begin() { copy_to_cpu(); return sparse_state_cpu.begin(); }
		vector_type::iterator end() { copy_to_cpu(); return sparse_state_cpu.end(); }
		vector_type::reverse_iterator rbegin() { copy_to_cpu(); return sparse_state_cpu.rbegin(); }
		vector_type::reverse_iterator rend() { copy_to_cpu(); return sparse_state_cpu.rend(); }
	};


	struct AbsSqrFunctor {
		__host__ __device__ double operator()(const System& s) const {
			return CuAbsSqr(s);
		}
	};

	struct Normalize_Functor {
		double factor;

		Normalize_Functor(double factor_) : factor(factor_) {}

		__host__ __device__ void operator()(System& s) const {
			double* amplitude = CuSystemAmplitude(s);
			amplitude[0] *= factor;
			amplitude[1] *= factor;
		}
	};

	struct SystemLess_Functor
	{
		size_t name_reg_map_size;
		size_t status_bitmap = 0;

		SystemLess_Functor()
			: name_reg_map_size(System::name_register_map.size()),
			status_bitmap(System::reg_status_bitmap)
		{
			//for (size_t i = 0; i < System::name_register_map.size(); ++i)
			//{
			//	if (System::status_of(i))
			//		status_bitmap |= pow2(i);
			//}
			
		}

		__host__ __device__ bool operator()(const System& a, const System& b) {
			for (size_t i = 0; i < name_reg_map_size; ++i)
			{
				auto regl = CuGet(a, i).value * ((status_bitmap >> i) & 1);
				auto regr = CuGet(b, i).value * ((status_bitmap >> i) & 1);
				if (regl < regr)
					return true;
				else if (regl > regr)
					return false;
			}
			return false;
		}
	};


	struct SystemEqual_Functor
	{
		size_t name_reg_map_size;
		size_t status_bitmap = 0;

		SystemEqual_Functor()
			: name_reg_map_size(System::name_register_map.size()),
			status_bitmap(System::reg_status_bitmap)
		{
			//for (size_t i = 0; i < System::name_register_map.size(); ++i)
			//{
			//	if (System::status_of(i))
			//		status_bitmap |= pow2(i);
			//}
		}

		__host__ __device__ bool operator()(const System& a, const System& b) {
			for (size_t i = 0; i < name_reg_map_size; ++i)
			{
				auto regl = CuGet(a, i).value * ((status_bitmap >> i) & 1);
				auto regr = CuGet(b, i).value * ((status_bitmap >> i) & 1);
				if (regl != regr)
					return false;
			}
			return true;
		}

		__host__ __device__ bool operator()(const thrust::tuple<System, System>& system_pair) {
			for (size_t i = 0; i < name_reg_map_size; ++i)
			{
				const System& a = thrust::get<0>(system_pair);
				const System& b = thrust::get<1>(system_pair);
				auto regl = CuGet(a, i).value * ((status_bitmap >> i) & 1);
				auto regr = CuGet(b, i).value * ((status_bitmap >> i) & 1);
				if (regl != regr)
					return false;
			}
			return true;
		}
	};

	struct AmplitudeZero_Functor
	{
		double eps;
		AmplitudeZero_Functor(double eps_) : eps(eps_) {}

		__host__ __device__ bool operator()(const System& s) const {
			const double* amplitude = CuSystemAmplitude(s);
			return (amplitude[0] * amplitude[0] + amplitude[1] * amplitude[1]) < eps;
		}
	};

}