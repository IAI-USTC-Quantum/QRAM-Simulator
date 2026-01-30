#include "partial_trace.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"

namespace qram_simulator {

	std::pair<std::vector<uint64_t>, double> PartialTrace::operator()(CuSparseState& state) const
	{
		profiler _("PartialTrace");
		state.move_to_gpu();

		/* Compute the prefix sum of probs */
		thrust::device_vector<double> prefix_sum(state.sparse_state_gpu.size());
		
		auto beg = thrust::make_transform_iterator(state.sparse_state_gpu.begin(), AbsSqrFunctor());
		auto end = thrust::make_transform_iterator(state.sparse_state_gpu.end(), AbsSqrFunctor());
		thrust::inclusive_scan(thrust::device, beg, end, prefix_sum.begin());

		double r = random_engine::uniform01();
		auto iter = thrust::lower_bound(thrust::device, prefix_sum.begin(), prefix_sum.end(), r);

		if (iter == prefix_sum.end())
			throw std::runtime_error("Error: Sum prob is not 1.0");

		auto index = iter - prefix_sum.begin();
		System select;
		thrust::copy(state.sparse_state_gpu.begin() + index, state.sparse_state_gpu.begin() + index + 1, &select);

		/* Get select values */
		std::vector<size_t> select_values(partial_trace_registers.size());

		for (auto i = 0; i < partial_trace_registers.size(); i++)
		{
			select_values[i] = select.GetAs(partial_trace_registers[i], uint64_t);
		}

		double p = PartialTraceSelect(partial_trace_registers, select_values)(state);

		return { select_values, p };
	}
	
	struct PartialTraceSelectFunctor
	{
		int* partial_trace_registers;
		size_t* select_values;
		size_t n_registers;

		PartialTraceSelectFunctor(int* partial_trace_registers, size_t* select_values, size_t n_registers)
			: partial_trace_registers(partial_trace_registers), select_values(select_values), n_registers(n_registers)
		{
		}

		__host__ __device__ bool operator()(const System& s)
		{
			for (size_t i = 0; i < n_registers; i++)
			{
				if (select_values[i] != CuGet(s, partial_trace_registers[i]).value)
					return true;
			}
			return false;
		}
	};

	double PartialTraceSelect::operator()(CuSparseState& state) const
	{
		profiler _("PartialTraceSelect");
		state.move_to_gpu();

		thrust::device_vector<int> partial_trace_registers_dev(partial_trace_registers.begin(), partial_trace_registers.end());
		thrust::device_vector<size_t> select_values_dev(select_values.begin(), select_values.end());
				
		auto iter = thrust::remove_if(thrust::device, 
			state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			PartialTraceSelectFunctor(
				partial_trace_registers_dev.data().get(),
				select_values_dev.data().get(),
				partial_trace_registers.size())
		);

		state.sparse_state_gpu.erase(iter, state.sparse_state_gpu.end());

		double sum_prob = thrust::transform_reduce(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			AbsSqrFunctor(),
			0.0,
			thrust::plus<double>()
		);

		if (sum_prob != 0)
		{
			sum_prob = 1.0 / std::sqrt(sum_prob);

			// 2. renormalize
			thrust::for_each(state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Normalize_Functor(sum_prob));

			return sum_prob;
		}
		else {
			return std::numeric_limits<double>::infinity();
		}

	}

	struct PartialTraceSelectRangeFunctor
	{
		int partial_trace_register;
		size_t register_size;
		size_t select_range_start;
		size_t select_range_end;

		PartialTraceSelectRangeFunctor(int partial_trace_register_, size_t register_size_, std::pair<size_t, size_t> select_range_)
			: partial_trace_register(partial_trace_register_), register_size(register_size_), 
			select_range_start(select_range_.first), select_range_end(select_range_.second)
		{
		}

		__host__ __device__ bool operator()(const System& s)
		{
			uint64_t v = CuGetAsUint64(s, partial_trace_register, register_size);
			if (v >= select_range_start && v <= select_range_end)
				return false;
			else
				return true;
		}
	};

	double PartialTraceSelectRange::operator()(CuSparseState& state)
	{
		state.move_to_gpu();

		if (state.size() == 0)
			return 0;

		size_t register_size = System::size_of(partial_trace_register);

		auto iter = thrust::remove_if(thrust::device,
			state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			PartialTraceSelectRangeFunctor(partial_trace_register, register_size, select_range)
		);

		state.sparse_state_gpu.erase(iter, state.sparse_state_gpu.end());

		double sum_prob = thrust::transform_reduce(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			AbsSqrFunctor(),
			0.0,
			thrust::plus<double>()
		);

		if (sum_prob != 0)
		{
			sum_prob = 1.0 / std::sqrt(sum_prob);

			// 2. renormalize
			thrust::for_each(state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Normalize_Functor(sum_prob));

			return sum_prob;
		}
		else {
			return std::numeric_limits<double>::infinity();
		}
	}

} // namespace qram_simulator
