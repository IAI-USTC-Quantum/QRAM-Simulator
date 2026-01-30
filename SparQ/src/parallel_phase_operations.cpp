#include "parallel_phase_operations.h"
#include "matrix.h"

namespace qram_simulator
{
	void ZeroConditionalPhaseFlip::operator()(std::vector<System>& state) const
	{
		profiler _("ZeroConditionalPhaseFlip");
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionNotSatisfied(s))
				continue;
						
			if (std::all_of(ids.begin(), ids.end(), [&](auto id) {
				return s.get(id).value == 0;})) 
				
				s.amplitude *= -1;
		}
	}

	void RangeConditionalPhaseFlip::operator()(std::vector<System>& state) const
	{
		profiler _("RangeConditionalFlip");
		for (auto& s : state)
		{
			if (s.GetAs(id, uint64_t) > value_range)
			{
				s.amplitude *= -1;
				break;
			}
		}
	}

	void Reflection_Bool::operator()(std::vector<System>& state) const
	{
		profiler _("Reflection_Bool");		

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int64_t i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionNotSatisfied(s))
				continue;
			bool _iszero = std::all_of(regs.begin(), regs.end(), [s](size_t id) {
				return (s.GetAs(id, uint64_t) == 0);
				});

			// add global phase
			if (!(inverse ^ _iszero)) {
				s.amplitude *= -1.0;
			}
		}
	}

	void GlobalPhase_Int::operator()(std::vector<System>& state) const
	{
		profiler _("GlobalPhase_Int");
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int64_t i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionNotSatisfied(s))
				continue;

			s.amplitude *= c;
		}
	}

	void GlobalPhase_Int::dag(std::vector<System>& state) const
	{
		profiler _("GlobalPhase_Int");
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int64_t i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionNotSatisfied(s))
				continue;

			s.amplitude *= std::conj(c);
		}
	}
}