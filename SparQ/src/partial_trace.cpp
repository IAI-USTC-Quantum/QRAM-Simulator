#include "partial_trace.h"

namespace qram_simulator
{
	PartialTrace::PartialTrace(const std::vector<std::string>& partial_trace_registers_)
	{
		if (partial_trace_registers_.size() == 0)
			return;

		partial_trace_registers.resize(partial_trace_registers_.size());

		for (size_t i = 0; i < partial_trace_registers_.size(); ++i)
		{
			partial_trace_registers[i] = System::get(partial_trace_registers_[i]);
		}
	}

	PartialTrace::PartialTrace(const std::vector<size_t>& partial_trace_registers_)
	{
		if (partial_trace_registers_.size() == 0)
			return;

		partial_trace_registers.resize(partial_trace_registers_.size());
		partial_trace_registers = partial_trace_registers_;
	}

	PartialTrace::PartialTrace(std::string_view partial_trace_registers_name)
	{
		partial_trace_registers.push_back(System::get(partial_trace_registers_name));
	}

	PartialTrace::PartialTrace(size_t partial_trace_registers_name)
	{
		partial_trace_registers.push_back(partial_trace_registers_name);
	}

	std::pair<std::vector<uint64_t>, double> PartialTrace::operator()(std::vector<System>& state) const
	{
		if (partial_trace_registers.size() == 0)
			return { {}, std::numeric_limits<double>::infinity() };
		if (state.size() == 0)
			return { {}, std::numeric_limits<double>::infinity() };
		double r = random_engine::uniform01();

		std::vector<uint64_t> select_values(partial_trace_registers.size(), 0);
		// 1. select the register values
		for (const auto& s : state)
		{
			double prob = abs_sqr(s.amplitude);

			if (r < prob) {
				for (size_t i = 0; i < partial_trace_registers.size(); ++i)
				{
					select_values[i] = s.get(partial_trace_registers[i]).value;
				}
				break;
			}
			else {
				r -= prob;
			}
		}
		//// 2. remove all unwanted values
		//double sum_prob = 0;

		//auto pred = [this, &sum_prob, &select_values](const System& s)
		//	{
		//		for (size_t i = 0; i < partial_trace_registers.size(); ++i)
		//		{
		//			if (select_values[i] != s.get(partial_trace_registers[i]).value)
		//				return true;
		//		}
		//		sum_prob += abs_sqr(s.amplitude);
		//		return false;
		//	};

		//state.erase(std::remove_if(state.begin(), state.end(), pred), state.end());

		//sum_prob = 1.0 / std::sqrt(sum_prob);

		//// 3. renormalize
		//std::for_each(state.begin(), state.end(),
		//	[sum_prob](System& s) { s.amplitude *= sum_prob; }
		//);
		//return { select_values, sum_prob };

		double prob = PartialTraceSelect(partial_trace_registers, select_values)(state);
		return { select_values, prob };
	}

	PartialTraceSelect::PartialTraceSelect(const std::map<std::string_view, uint64_t>& partial_traces)
	{
		if (partial_traces.size() == 0)
			return;

		partial_trace_registers.reserve(partial_traces.size());
		select_values.reserve(partial_traces.size());

		for (auto&& [k, v] : partial_traces)
		{
			partial_trace_registers.push_back(System::get(k));
			select_values.push_back(v);
		}
	}

	PartialTraceSelect::PartialTraceSelect(const std::map<size_t, uint64_t>& partial_traces)
	{
		if (partial_traces.size() == 0)
			return;

		partial_trace_registers.reserve(partial_traces.size());
		select_values.reserve(partial_traces.size());

		for (auto&& [k, v] : partial_traces)
		{
			partial_trace_registers.push_back(k);
			select_values.push_back(v);
		}
	}

	PartialTraceSelect::PartialTraceSelect(const std::vector<std::string>& partial_trace_regs_,
		const std::vector<uint64_t> &select_values_)
	{
		if (partial_trace_regs_.size() != select_values_.size())
			throw_invalid_input();

		for (size_t i = 0; i < partial_trace_regs_.size(); ++i)
		{
			partial_trace_registers.push_back(System::get(partial_trace_regs_[i]));
		}
		select_values = select_values_;
	}

	PartialTraceSelect::PartialTraceSelect(const std::vector<size_t>& partial_trace_regs_,
		const std::vector<uint64_t> &select_values_)
	{
		if (partial_trace_regs_.size() != select_values_.size())
			throw_invalid_input();

		partial_trace_registers = partial_trace_regs_;
		select_values = select_values_;
	}

	double PartialTraceSelect::operator()(std::vector<System>& state) const
	{
		profiler _("PartialTraceSelect");
		if (partial_trace_registers.size() == 0)
			return 0;
		if (state.size() == 0)
			return 0;

		// 1. remove all unwanted values
		double sum_prob = 0;

		auto pred = [this, &sum_prob](const System& s)
			{
				for (size_t i = 0; i < partial_trace_registers.size(); ++i)
				{
					if (select_values[i] != s.get(partial_trace_registers[i]).value)
						return true;
				}
				sum_prob += abs_sqr(s.amplitude);
				return false;
			};

		state.erase(std::remove_if(state.begin(), state.end(), pred), state.end());
		if (sum_prob != 0)
		{
			sum_prob = 1.0 / std::sqrt(sum_prob);

			// 2. renormalize
			std::for_each(state.begin(), state.end(),
				[sum_prob](System& s) { s.amplitude *= sum_prob; }
			);

			return sum_prob;
		}
		else {
			return std::numeric_limits<double>::infinity();
		}
	}

	double PartialTraceSelectRange::operator()(std::vector<System>& state) const
	{
		if (state.size() == 0)
			return 0;

		// 1. remove all unwanted values
		double sum_prob = 0;

		auto pred = [this, &sum_prob](const System& s)
			{
				uint64_t v = s.GetAs(partial_trace_register, uint64_t);

				if (v >= select_range.first && v <= select_range.second)
				{
					sum_prob += abs_sqr(s.amplitude);
					return false;
				}
				else {
					return true;
				}
			};

		state.erase(std::remove_if(state.begin(), state.end(), pred),
			state.end());
		if (sum_prob != 0)
		{
			sum_prob = 1.0 / std::sqrt(sum_prob);

			// 2. renormalize
			std::for_each(state.begin(), state.end(),
				[sum_prob](System& s) { s.amplitude *= sum_prob; }
			);

			return sum_prob;
		}
		else {
			return std::numeric_limits<double>::infinity();
		}
	}

}