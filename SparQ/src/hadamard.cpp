#include "hadamard.h"
#include "system_operations.h"

namespace qram_simulator
{
	/*
	about hadamard matrix

	< x | H | y >
	if ( x[i] & y[i] ) H = -1
	else H = 1
	*/
	void Hadamard_Int::operate(size_t l, size_t r, std::vector<System>& state) const
	{
		profiler _("Hadamard::operate");
		size_t n = r - l;
		size_t full_size = pow2(n_digits);
		size_t original_size = state.size();
		double extra_amplitude = 1.0 / std::sqrt(full_size);

		if (n == 0) return;
		else if (n == 1)
		{
			if (!ConditionSatisfied(state[l])) 
				return;
			size_t v = val(l, state);
			state[l].amplitude *= extra_amplitude;
			state.insert(state.end(), full_size, state[l]);
			for (size_t i = 0; i < full_size; ++i)
			{
				size_t real_position = original_size + i;
				val(real_position, state) = (val(real_position, state) & mask) + i;
				if (bitcount(i & v) & 1) state[real_position].amplitude *= -1;
			}
			state[l].amplitude = 0;
		}
		else if (n == full_size)
		{
			if (!ConditionSatisfied(state[l])) 
				return;
			// Version 1 : use Fast Hadamard
			for (size_t qn = 0; qn < n_digits; ++qn)
			{
				for (size_t i = 0; i < full_size; ++i)
				{
					if ((i >> qn) & 1)
					{
						complex_t amps0 = state[i + l - pow2(qn)].amplitude;
						complex_t amps1 = state[i + l].amplitude;
						state[i + l - pow2(qn)].amplitude = (amps0 + amps1) * sqrt2inv;
						state[i + l].amplitude = (amps0 - amps1) * sqrt2inv;
					}
				}
			}
			// 
			// Version 2 : naive
			//std::vector<std::complex<double>> new_amps(full_size, 0);
			//for (size_t i = 0; i < full_size; ++i)
			//{
			//	size_t real_position = l + i;
			//	for (size_t j = 0; j < full_size; ++j)
			//	{
			//		if (bitcount(i & j) & 1)
			//			new_amps[j] -= state[real_position].amplitude;
			//		else
			//			new_amps[j] += state[real_position].amplitude;
			//	}
			//}
			//for (size_t i = 0; i < full_size; ++i)
			//{
			//	state[i + l].amplitude = new_amps[i] * extra_amplitude;
			//}
		}
		else // the general case
		{
			if (!ConditionSatisfied(state[l])) 
				return;
			state.insert(state.end(), full_size, state[l]);

			// Version 1 : naive
			//std::vector<std::complex<double>> new_amps(full_size, 0);
			//for (size_t i = l; i < r; ++i)
			//{
			//	for (size_t j = 0; j < full_size; ++j)
			//	{
			//		if (bitcount(val(i, state) & j) & 1)
			//			new_amps[j] -= state[i].amplitude;
			//		else
			//			new_amps[j] += state[i].amplitude;
			//	}
			//}
			//for (size_t i = 0; i < full_size; ++i)
			//{
			//	state[i + original_size].amplitude = new_amps[i] * extra_amplitude;
			//	val(i + original_size, state) = (val(l, state) & mask) + i;				
			//}
			//for (size_t i = l; i < r; ++i)
			//	state[i].amplitude = 0;

			// 
			// Version 2 : use Fast Hadamard
			size_t value_mask = (val(l, state) & mask);
			for (size_t i = 0; i < full_size; ++i)
			{
				state[i + original_size].amplitude = 0;
				val(i + original_size, state) = value_mask + i;
			}
			for (size_t i = l; i < r; ++i)
			{
				state[val(i, state) - value_mask + original_size].amplitude = state[i].amplitude;
				state[i].amplitude = 0;
			}
			for (size_t qn = 0; qn < n_digits; ++qn)
			{
				for (size_t i = 0; i < full_size; ++i)
				{
					if ((i >> qn) & 1)
					{
						complex_t amps0 = state[i + original_size - pow2(qn)].amplitude;
						complex_t amps1 = state[i + original_size].amplitude;
						state[i + original_size - pow2(qn)].amplitude = (amps0 + amps1) * sqrt2inv;
						state[i + original_size].amplitude = (amps0 - amps1) * sqrt2inv;
					}
				}
			}
		}
	}

	void Hadamard_Int::operator()(std::vector<System>& state) const
	{
		profiler _("Hadamard_Int");
		(SortExceptKey(id))(state);
		size_t current_size = state.size();
		auto iter_l = 0;
		auto iter_r = 1;

		while (true)
		{
			if (iter_r == current_size)
			{
				operate(iter_l, iter_r, state);
				break;
			}
			if (!compare_equal(state[iter_l], state[iter_r], id))
			{
				operate(iter_l, iter_r, state);
				iter_l = iter_r;
				iter_r = iter_l + 1;
			}
			else
			{
				iter_r++;
			}
		}
		ClearZero()(state);
		System::update_max_size(state.size());
	}

	struct HadamardBucket
	{
		int count;
#ifndef SAFE_HASH
		size_t first_position;
#endif
		std::vector<int> all_positions;

		HadamardBucket(size_t first_pos, size_t full_size)
			: count(0),
#ifndef SAFE_HASH
			first_position(first_pos),
#endif
			all_positions(full_size, -1)
		{}
	};

	void Hadamard_Int_Full::operator()(std::vector<System>& state) const
	{
		CheckDuplicateKey()(state);
		profiler _("Hadamard_Int_Full");

		if (!state.size()) return;

		size_t current_size = state.size();

#ifdef SAFE_HASH
		StateLessExceptKey pred(id);
		//std::map<System, HadamardBucket, StateLessExceptKey> buckets(pred);
		std::map<System, std::pair<size_t, std::vector<size_t>>, StateLessExceptKey> buckets(pred);
#else
		auto hash_func = StateHashExceptKey(id);
		for (auto& s : state)
			s.cached_hash = hash_func(s);

		std::unordered_map<size_t, HadamardBucket> buckets;
#endif

		for (size_t i = 0; i < current_size; ++i)
		{
			if (!ConditionSatisfied(state[i]))
				continue;
#ifdef SAFE_HASH
			const auto& s = state[i];
			size_t val = s.GetAs(id, uint64_t);
			auto&& [iter, flag] = buckets.insert({ s, {0, std::vector<size_t>(full_size, -1)} });
			iter->second.second[val] = i;
			iter->second.first += 1;
			if (iter->second.first == full_size)
			{
				operate_bucket_inplace(iter->second.second, state);
				buckets.erase(iter);
			}
#else
			size_t s = state[i].cached_hash;
			size_t val = state[i].GetAs(id, uint64_t);
			auto&& [iter, flag] = buckets.insert({ s, HadamardBucket(i, bucket_full_size) });
			iter->second.all_positions[val] = i;
			iter->second.count += 1;
			if (iter->second.count == bucket_full_size)
			{
				operate_bucket_inplace(iter->second.all_positions, state);
				buckets.erase(iter);
			}
#endif

		}

		/* Remaining buckets */
		for (auto& [s, bucket] : buckets)
		{
			/* Fill the remaining states */
#ifdef SAFE_HASH
			auto& all_pos = bucket.second;
#else
			auto& all_pos = bucket.all_positions;
#endif
			/* Case 1: When there are only few states in the bucket */
			if (bucket.first <= few_threshold)
			{
				operate_bucket_sparse(all_pos, state);
				continue;
			}

			/* Case 2: For many states in the bucket */
			for (size_t i = 0; i < all_pos.size(); ++i)
			{
				if (all_pos[i] == -1)
				{
#ifdef SAFE_HASH
					state.push_back(s);
#else
					state.push_back(state[bucket.first_position]);
#endif
					state.back().get(id).value = i;
					state.back().amplitude = 0;
					all_pos[i] = state.size() - 1;
				}
			}
			operate_bucket_inplace(all_pos, state);
		}
		ClearZero()(state);
		System::update_max_size(state.size());
	}
	
	void Hadamard_Int_Full::operate_bucket_sparse(const std::vector<size_t>& positions, std::vector<System>& state) const
	{
		double extra_amplitude = 1.0 / std::sqrt(full_size);
		size_t original_size = state.size(); // before insert
		bool inserted = false; // for lazy insertion

		for (size_t i = 0; i < positions.size(); ++i)
		{
			if (positions[i] == -1) continue;

			size_t v = val(positions[i], state);
			if (!inserted)
			{
				// insert at the first valid position
				state.insert(state.end(), full_size, state[positions[i]]);

				for (size_t j = 0; j < full_size; ++j)
				{
					size_t new_position = original_size + j;
					val(new_position, state) = j;
					state[new_position].amplitude = 0;
				}
				inserted = true;
			}

			// do matrix multiplication
			for (size_t j = 0; j < full_size; ++j)
			{
				size_t new_position = original_size + j;
				state[new_position].amplitude += (state[positions[i]].amplitude * (bit_parity(j & v) ? -1. : 1.) * extra_amplitude);
			}

			// clear amplitude
			state[positions[i]].amplitude = 0;
		}
	}

	void Hadamard_Int_Full::operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state) const
	{
		size_t n_digits = System::size_of(id);
		size_t full_size = pow2(n_digits);

		/* Assign amplitudes to a temporary array */
		std::vector<complex_t> amps(positions.size(), 0);
		for (size_t i = 0; i < positions.size(); ++i)
		{
			amps[i] = state[positions[i]].amplitude;
		}

		/* Use fast Hadamard transform */
		for (size_t qn = 0; qn < n_digits; ++qn)
		{
			for (size_t i = 0; i < full_size; ++i)
			{
				if ((i >> qn) & 1)
				{
					complex_t amps0 = amps[i - pow2(qn)];
					complex_t amps1 = amps[i];
					amps[i - pow2(qn)] = (amps0 + amps1) * sqrt2inv;
					amps[i] = (amps0 - amps1) * sqrt2inv;
				}
			}
		}

		/* Return their amplitudes */
		for (size_t i = 0; i < positions.size(); ++i)
		{
			state[positions[i]].amplitude = amps[i];
		}
	}

	void Hadamard_Bool::operate_pair(size_t zero, size_t one, std::vector<System>& state) const
	{
		complex_t a = state[zero].amplitude;
		complex_t b = state[one].amplitude;
		state[zero].amplitude = a * sqrt2inv + b * sqrt2inv;
		state[one].amplitude = a * sqrt2inv - b * sqrt2inv;
	}

	void Hadamard_Bool::operate_alone_zero(size_t zero, std::vector<System>& state) const
	{
		state.push_back(state[zero]);
		state.back().get(out_id).value = 1;

		state[zero].amplitude *= sqrt2inv;
		state.back().amplitude *= sqrt2inv;
	}

	void Hadamard_Bool::operate_alone_one(size_t one, std::vector<System>& state) const
	{
		state.push_back(state[one]);
		state.back().get(out_id).value = 0;

		state.back().amplitude *= sqrt2inv;
		state[one].amplitude *= (-sqrt2inv);
	}

	void Hadamard_Bool::operator()(std::vector<System>& state) const
	{
		profiler _("Hadamard_Bool_v2");

		if (!state.size()) return;
#ifdef SAFE_HASH
		StateLessExceptKey pred(out_id);
		std::map<System, size_t, StateLessExceptKey> buckets(pred);
#else
		auto hash_func = StateHashExceptKey(out_id);
		for (auto& s : state)
			s.cached_hash = hash_func(s);

		std::unordered_map<size_t, size_t> buckets;
#endif
		size_t current_size = state.size();

		for (size_t i = 0; i < current_size; ++i)
		{
			if (!ConditionSatisfied(state[i]))
				continue;
#ifdef SAFE_HASH
			const auto& s = state[i];
#else
			const auto& s = state[i].cached_hash;
#endif
			auto iter = buckets.find(s);
			if (iter == buckets.end())
			{
				buckets.insert({ s, i });
				continue;
			}
			else
			{
				//auto pred = StateEqualExceptKey(out_id);
				//if (!pred(state[iter->second], state[i]))
				//{
				//	fmt::print("{}\n", state[i].to_string());
				//	fmt::print("{}\n", state[iter->second].to_string());
				//	fmt::print("Collide with the same hash {}\n", state[i].cached_hash);
				//	fmt::print("Collide with the same hash {}\n", state[iter->second].cached_hash);
				//	throw_general_runtime_error();
				//}

				if (state[iter->second].get(out_id).as<bool>(1))
				{
					// stored 1 and now 0
					operate_pair(i, iter->second, state);
				}
				else
				{
					// stored 0 and now 1
					operate_pair(iter->second, i, state);
				}
				buckets.erase(iter);
			}
		}


		for (auto& stored_key : buckets)
		{
			// alone
			if (state[stored_key.second].get(out_id).as<bool>(1))
			{
				operate_alone_one(stored_key.second, state);
			}
			else
			{
				operate_alone_zero(stored_key.second, state);
			}
		}
		ClearZero()(state);
		System::update_max_size(state.size());
	}


	/*
	Transform from 0~(2^n-1) to the position in qubit_position
	*/
	static size_t map_position(size_t i, const std::set<size_t>& qubit_position)
	{
		size_t position = 0, temp = 0;
		for (auto qn : qubit_position)
		{
			position += ((i >> temp) % 2) * pow2(qn);
			temp++;
		}
		return position;
	}

	/*
	Inverse of map_position
	*/
	static size_t from_map_position(size_t i, const std::set<size_t>& qubit_position)
	{
		size_t position = 0, temp = 0;
		for (auto qn : qubit_position)
		{
			position += ((i >> qn) % 2) * pow2(temp);
			temp++;
		}
		return position;
	}

	void Hadamard_PartialQubit::operate(size_t l, size_t r, std::vector<System>& state) const
	{
		profiler _("Hadamard_PartialQubit::operate");
		size_t n = r - l; // number of Feynman paths in same interference pattern
		size_t full_size = pow2(qubit_positions.size());
		size_t original_size = state.size();
		double extra_amplitude = 1.0 / std::sqrt(full_size);
		if (n == 0) return;
		else if (n == 1)
		{
			/* There is only one Feynman path in this interference pattern.
			e.g. qubit_positions = {0, 1}, state[l] = 0.123612|000 01>, state[l+1] = 0.337461|111 01>.

			Step 1. Create new states;
				A = {state[l]}\cup B = {} should be enlarged to A' \cup B', 
				where B' = {state[N], state[N+1], state[N+2], state[N+3]|amplitude = 1.0/sqrt(2^qubit_positions.size())},
				N is the number of total Feynman paths, A' = {state[l]|amplitude = 0.0}.
			Step 2. Assign the register values of each state in B'.
			Step 3. Adjust the phase of each state in B'.
				Then the phase (-1) is added to each state in B' if it satisfies the condition.
			Complexity: O(M), M = 2^qubit_positions.size()
			*/
			if (!ConditionSatisfied(state[l]))
				return;
			size_t v = val(l, state);
			state[l].amplitude *= extra_amplitude;
			state.insert(state.end(), full_size, state[l]);
			for (size_t i = 0; i < full_size; ++i)
			{
				size_t relative_pos = 0;
				size_t real_position = original_size + i;
				//val(real_position, state) = (val(real_position, state) & mask) + i;
				val(real_position, state) = (val(real_position, state) & mask) + map_position(i, qubit_positions);
				if (bitcount(i & (from_map_position(v, qubit_positions)) & 1))
					state[real_position].amplitude *= -1;
			}
			state[l].amplitude = 0;
		}
		else if (n == full_size)
		{
			/* There are full Feynman paths in this interference pattern.
			e.g. qubit_positions = {0, 1}, state[l] = 0.123612|000 00>, state[l+1] = 0.337461|000 01>, 
			state[l+2] = 0.123612|000 10>, state[l+3] = 0.337461|000 11>.

			Step 1. Create new states;
				A = {state[l]}\cup B = {} should be enlarged to A' \cup B',
				where B' = {state[N], state[N+1], state[N+2], state[N+3]|amplitude = 1.0/sqrt(2^qubit_positions.size())},
				N is the number of total Feynman paths, A' = {state[l]|amplitude = 0.0}.
			Step 2. Assign the register values of each state in B'.
			Step 3. Adjust the phase of each state in B'.
				Then the phase (-1) is added to each state in B' if it satisfies the condition.
			Complexity: O(M log M), M = 2^qubit_positions.size()
			*/

			// Version 1 : use Fast Hadamard (Walsh-Hadamard Transform)
			// size_t temp = 0;
			if (!ConditionSatisfied(state[l]))
				return;
			for (size_t qn = 0; qn < qubit_positions.size(); ++qn)
			{
#ifdef SINGLE_THREAD
				for (size_t i = 0; i < full_size; ++i)
				{
#else
#pragma omp parallel for
				for (int64_t i = 0; i < full_size; ++i)
				{
#endif
					if ((i >> qn) & 1)
					{
						complex_t amps0 = state[i + l - pow2(qn)].amplitude;
						complex_t amps1 = state[i + l].amplitude;
						state[i + l - pow2(qn)].amplitude = (amps0 + amps1) * sqrt2inv;
						state[i + l].amplitude = (amps0 - amps1) * sqrt2inv;
					}
				}
				// temp++;
			}
		}
		else // the general case
		{
			/* There are several Feynman paths between 1~2^qubit_positions.size() in this interference pattern.
			e.g. qubit_positions = {0, 1}, state[l] = 0.123612|000 00>, state[l+1] = 0.337461|000 01>,
			state[l+2] = 0.123612|000 10>, state[l+3] = 0.337461|000 11>.

			It is similar to the case of one Feynman path, but we need to erase n states.
			*/
			if (!ConditionSatisfied(state[l]))
				return;
			state.insert(state.end(), full_size, state[l]);
			size_t value_mask = (val(l, state) & mask);
			for (size_t i = 0; i < full_size; ++i)
			{
				state[i + original_size].amplitude = 0;
				val(i + original_size, state) = value_mask + map_position(i, qubit_positions);
			}
			for (size_t i = l; i < r; ++i)
			{
				state[from_map_position(val(i, state), qubit_positions) + original_size].amplitude = state[i].amplitude;
				state[i].amplitude = 0;
			}
			for (size_t qn = 0; qn < qubit_positions.size(); ++qn)
			{
#ifdef SINGLE_THREAD
				for (size_t i = 0; i < full_size; ++i)
				{
#else
#pragma omp parallel for
				for (int64_t i = 0; i < full_size; ++i)
				{
#endif
					if ((i >> qn) & 1)
					{
						complex_t amps0 = state[i + original_size - pow2(qn)].amplitude;
						complex_t amps1 = state[i + original_size].amplitude;
						state[i + original_size - pow2(qn)].amplitude = (amps0 + amps1) * sqrt2inv;
						state[i + original_size].amplitude = (amps0 - amps1) * sqrt2inv;
					}
				}
			}
		}
	}

	void Hadamard_PartialQubit::operate_pair(size_t zero, size_t one, std::vector<System>& state) const
	{	// only for single qubit
		complex_t a = state[zero].amplitude;
		complex_t b = state[one].amplitude;
		state[zero].amplitude = a * sqrt2inv + b * sqrt2inv;
		state[one].amplitude = a * sqrt2inv - b * sqrt2inv;
	}

	void Hadamard_PartialQubit::operate_alone_zero(size_t zero, std::vector<System>& state) const
	{	// only for single qubit
		state.push_back(state[zero]);
		state.back().get(id).value |= (~mask);

		state[zero].amplitude *= sqrt2inv;
		state.back().amplitude *= sqrt2inv;
	}

	void Hadamard_PartialQubit::operate_alone_one(size_t one, std::vector<System>& state) const
	{	// only for single qubit
		state.push_back(state[one]);
		state.back().get(id).value &= mask;

		state.back().amplitude *= sqrt2inv;
		state[one].amplitude *= (-sqrt2inv);
	}

	void Hadamard_PartialQubit::operator()(std::vector<System>& state) const
	{
		bool use_hash = true;
		if (!use_hash)
		{
			profiler _("Hadamard_PartialQubit_v1");
			(SortExceptKeyHadamard(id, qubit_positions))(state);
			size_t current_size = state.size();
			auto iter_l = 0;
			auto iter_r = 1;

			while (true)
			{
				if (iter_r == current_size)
				{
					operate(iter_l, iter_r, state);
					break;
				}
				if (!compare_equal_hadamard(state[iter_l], state[iter_r], id, mask))
				{
					operate(iter_l, iter_r, state);
					iter_l = iter_r;
					iter_r = iter_l + 1;
				}
				else
				{
					iter_r++;
				}
			}
			ClearZero()(state);
			System::update_max_size(state.size());
		}
		else {
			profiler _("Hadamard_PartialQubit_v2");
			if (!state.size()) return;
#ifdef SAFE_HASH
			StateLessExceptQubits pred(id, qubit_positions);
			std::map<System, size_t, StateLessExceptQubits> buckets(pred);
#else
			auto hash_func = StateHashExceptQubits(id, qubit_positions);
			for (auto& s : state)
				s.cached_hash = hash_func(s);

			std::unordered_map<size_t, size_t> buckets;
#endif
			size_t current_size = state.size();

			for (size_t i = 0; i < current_size; ++i)
			{
#ifdef SAFE_HASH
				const auto& s = state[i];
#else
				const auto& s = state[i].cached_hash;
#endif
				auto iter = buckets.find(s);
				if (iter == buckets.end())
				{
					buckets.insert({ s, i });
					continue;
				}
				else
				{
					if (state[iter->second].get(id).value & (~mask))
					{
						// stored 1 and now 0
						operate_pair(i, iter->second, state);
					}
					else
					{
						// stored 0 and now 1
						operate_pair(iter->second, i, state);
					}
					buckets.erase(iter);
				}
			}


			for (auto& stored_key : buckets)
			{
				// alone
				if (state[stored_key.second].get(id).value & (~mask))
				{
					operate_alone_one(stored_key.second, state);
				}
				else
				{
					operate_alone_zero(stored_key.second, state);
				}
			}
			ClearZero()(state);
			System::update_max_size(state.size());
		}
		
	}
}