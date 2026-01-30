#pragma once
#include "basic_components.h"
#ifdef USE_CUDA
#include "cuda/cuda_utils.cuh"
#endif

namespace qram_simulator
{
	struct PartialTrace {
		std::vector<size_t> partial_trace_registers;
		PartialTrace(const std::vector<std::string>& partial_trace_register_names);
		PartialTrace(const std::vector<size_t>& partial_trace_register_names);
		PartialTrace(std::string_view partial_trace_register_name);
		PartialTrace(size_t partial_trace_register_name);
		std::pair<std::vector<uint64_t>, double> operator()(std::vector<System>& state) const;
		std::pair<std::vector<uint64_t>, double> operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}
#ifdef USE_CUDA
		std::pair<std::vector<uint64_t>, double> operator()(CuSparseState& state) const;
#endif
	};

	struct PartialTraceSelect {
		std::vector<size_t> partial_trace_registers;
		std::vector<uint64_t> select_values;
		PartialTraceSelect(const std::map<std::string_view, uint64_t>& partial_traces);
		// PartialTraceSelect(const std::map<std::string, size_t>& partial_traces);
		PartialTraceSelect(const std::map<size_t, uint64_t>& partial_traces);
		PartialTraceSelect(const std::vector<std::string>& partial_trace_regs_,
			const std::vector<uint64_t> &select_values_);
		PartialTraceSelect(const std::vector<size_t>& partial_trace_regs_,
			const std::vector<uint64_t> &select_values_);
		double operator()(std::vector<System>& state) const;
		double operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}
#ifdef USE_CUDA
		double operator()(CuSparseState& state) const;
#endif

	};

	struct PartialTraceSelectRange {
		size_t partial_trace_register;
		std::pair<size_t, size_t> select_range;
		double r = 0.0;
		PartialTraceSelectRange(std::string_view partial_trace_register_,
			std::pair<size_t, size_t> select_range_) :
			partial_trace_register(System::get(partial_trace_register_)),
			select_range(select_range_)
		{
		}
		PartialTraceSelectRange(size_t partial_trace_register_,
			std::pair<size_t, size_t> select_range_) :
			partial_trace_register(partial_trace_register_),
			select_range(select_range_)
		{
		}
		double operator()(std::vector<System>& state) const;
		double operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}
#ifdef USE_CUDA
		double operator()(CuSparseState& state);
#endif
	};


}
