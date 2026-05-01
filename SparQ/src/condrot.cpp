#include "condrot.h"
#include "system_operations.h"

namespace qram_simulator
{

	void CondRot_Rational_Bool::operator()(std::vector<System>& state) const
	{
		profiler _("CondRot");
		if (!state.size()) return;

#ifdef SAFE_HASH
		StateLessExceptKey pred(register_out);
		std::map<System, size_t, StateLessExceptKey> buckets(pred);
#else
		auto hash_func = StateHashExceptKey(register_out);
		for (auto& s : state)
			s.cached_hash = hash_func(s);

		std::unordered_map<size_t, size_t> buckets;
#endif
		auto mapping = [](System& s0, System& s1, double cosine, double sine)
			{
				auto amp0 = s0.amplitude;
				auto amp1 = s1.amplitude;
				s0.amplitude = amp0 * cosine - amp1 * sine;
				s1.amplitude = amp0 * sine + amp1 * cosine;
			};

		size_t current_size = state.size();
		for (size_t i = 0; i < current_size; ++i)
		{
#ifdef SAFE_HASH
			const auto& key = state[i];
#else
			const auto& key = state[i].cached_hash;
#endif
			auto iter = buckets.find(key);
			if (iter == buckets.end())
			{
				buckets.insert({ key, i });
				continue;
			}

#ifdef CHECK_HASH
			auto pred = StateEqualExceptKey(register_out);
			if (!pred(state[iter->second], state[i]))
				throw_general_runtime_error();
#endif
			size_t first = iter->second;
			size_t second = i;
			size_t zero = state[first].GetAs(register_out, bool) ? second : first;
			size_t one = state[first].GetAs(register_out, bool) ? first : second;

			double rot = state[zero].GetAs(register_in, double) * (2 * pi);
			mapping(state[zero], state[one], std::cos(rot), std::sin(rot));
			buckets.erase(iter);
		}

		for (auto& stored_key : buckets)
		{
			size_t original = stored_key.second;
			state.push_back(state[original]);
			state.back().get(register_out).flip(0);
			state.back().amplitude = 0;

			bool original_is_one = state[original].GetAs(register_out, bool);
			size_t zero = original_is_one ? state.size() - 1 : original;
			size_t one = original_is_one ? original : state.size() - 1;

			double rot = state[zero].GetAs(register_in, double) * (2 * pi);
			mapping(state[zero], state[one], std::cos(rot), std::sin(rot));
		}
		ClearZero()(state);
	}


	void CondRot_Rational_Bool::dag(std::vector<System>& state) const
	{
		profiler _("CondRot_dag");
		if (!state.size()) return;

#ifdef SAFE_HASH
		StateLessExceptKey pred(register_out);
		std::map<System, size_t, StateLessExceptKey> buckets(pred);
#else
		auto hash_func = StateHashExceptKey(register_out);
		for (auto& s : state)
			s.cached_hash = hash_func(s);

		std::unordered_map<size_t, size_t> buckets;
#endif
		auto mapping = [](System& s0, System& s1, double cosine, double sine)
			{
				auto amp0 = s0.amplitude;
				auto amp1 = s1.amplitude;
				s0.amplitude = amp0 * cosine - amp1 * sine;
				s1.amplitude = amp0 * sine + amp1 * cosine;
			};

		size_t current_size = state.size();
		for (size_t i = 0; i < current_size; ++i)
		{
#ifdef SAFE_HASH
			const auto& key = state[i];
#else
			const auto& key = state[i].cached_hash;
#endif
			auto iter = buckets.find(key);
			if (iter == buckets.end())
			{
				buckets.insert({ key, i });
				continue;
			}

#ifdef CHECK_HASH
			auto pred = StateEqualExceptKey(register_out);
			if (!pred(state[iter->second], state[i]))
				throw_general_runtime_error();
#endif
			size_t first = iter->second;
			size_t second = i;
			size_t zero = state[first].GetAs(register_out, bool) ? second : first;
			size_t one = state[first].GetAs(register_out, bool) ? first : second;

			double rot = state[zero].GetAs(register_in, double) * (-2 * pi);
			mapping(state[zero], state[one], std::cos(rot), std::sin(rot));
			buckets.erase(iter);
		}

		for (auto& stored_key : buckets)
		{
			size_t original = stored_key.second;
			state.push_back(state[original]);
			state.back().get(register_out).flip(0);
			state.back().amplitude = 0;

			bool original_is_one = state[original].GetAs(register_out, bool);
			size_t zero = original_is_one ? state.size() - 1 : original;
			size_t one = original_is_one ? original : state.size() - 1;

			double rot = state[zero].GetAs(register_in, double) * (-2 * pi);
			mapping(state[zero], state[one], std::cos(rot), std::sin(rot));
		}
		ClearZero()(state);
	}


}
