#include "condrot.h"
#include "system_operations.h"

namespace qram_simulator
{

	void CondRot_Rational_Bool::operator()(std::vector<System>& state) const
	{
		profiler _("CondRot");
		auto pred = [this](const System& lhs, const System& rhs) {
			if (lhs.get(register_in).value < rhs.get(register_in).value)
				return true;
			if (lhs.get(register_in).value > rhs.get(register_in).value)
				return false;
			if (lhs.get(register_out).value < rhs.get(register_out).value)
				return true;

			return false;
			};

		std::sort(exec_policy, state.begin(), state.end(), pred);

		size_t sz = state.size();
		for (size_t i = 0; i < sz; ++i)
		{
			double rot = state[i].GetAs(register_in, double);

			rot *= (2 * pi);

			double ang1 = std::cos(rot);
			double ang2 = std::sin(rot);

			auto mapping = [](System& s1, System& s2, double cosine, double sine)
				{
					auto amp0 = s1.amplitude;
					auto amp1 = s2.amplitude;
					s1.amplitude = amp0 * cosine - amp1 * sine;
					s2.amplitude = amp0 * sine + amp1 * cosine;
				};

			if (i + 1 == state.size() || state[i].get(register_in).value != state[i + 1].get(register_in).value)
			{
				auto case1 = state[i].GetAs(register_out, bool);
				state.push_back(state[i]);
				state.back().get(register_out).flip(0);
				state.back().amplitude = 0;

				if (case1) {
					mapping(state.back(), state[i], ang1, ang2);
				}
				else {
					mapping(state[i], state.back(), ang1, ang2);
				}
			}
			else
			{
				auto case1 = state[i].GetAs(register_out, bool);

				if (case1) {
					mapping(state[i + 1], state[i], ang1, ang2);
				}
				else {
					mapping(state[i], state[i + 1], ang1, ang2);
				}

				i++;
			}
		}

		ClearZero()(state);
		// sort, merge and unique

		/*auto iter = std::remove_if(state.begin(), state.end(),
			[](System &s) { return abs_sqr(s.amplitude) < epsilon; }
		);

		state.erase(iter, state.end());*/
	}


	void CondRot_Rational_Bool::dag(std::vector<System>& state) const
	{
		profiler _("CondRot_dag");
		auto pred = [this](const System& lhs, const System& rhs) {
			if (lhs.get(register_in).value < rhs.get(register_in).value)
				return true;
			if (lhs.get(register_in).value > rhs.get(register_in).value)
				return false;
			if (lhs.get(register_out).value < rhs.get(register_out).value)
				return true;

			return false;
			};

		std::sort(state.begin(), state.end(), pred);
		size_t sz = state.size();
		for (size_t i = 0; i < sz; ++i)
		{
			double rot = state[i].GetAs(register_in, double);

			rot *= (-2 * pi);

			double ang1 = std::cos(rot);
			double ang2 = std::sin(rot);

			auto mapping = [](System& s1, System& s2, double cosine, double sine)
				{
					auto amp0 = s1.amplitude;
					auto amp1 = s2.amplitude;
					s1.amplitude = amp0 * cosine - amp1 * sine;
					s2.amplitude = amp0 * sine + amp1 * cosine;
				};

			if (i + 1 == state.size() ||
				state[i].get(register_in).value != state[i + 1].get(register_in).value)
			{
				auto case1 = state[i].GetAs(register_out, bool);
				state.push_back(state[i]);
				state.back().get(register_out).flip(0);
				state.back().amplitude = 0;

				if (case1) {
					mapping(state.back(), state[i], ang1, ang2);
				}
				else {
					mapping(state[i], state.back(), ang1, ang2);
				}
			}
			else
			{
				auto case1 = state[i].GetAs(register_out, bool);

				if (case1) {
					mapping(state[i + 1], state[i], ang1, ang2);
				}
				else {
					mapping(state[i], state[i + 1], ang1, ang2);
				}

				i++;
			}
		}
		ClearZero()(state);
	}


}