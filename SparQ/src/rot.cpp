#include "rot.h"
#include "system_operations.h"
#include "quantum_interfere_basic.h"

namespace qram_simulator
{

	//Rot_General_Bool::Rot_General_Bool(std::string_view reg, size_t digit_, u22_t mat)
	//	: id(System::get(reg)), digit(digit_), mat(mat)
	//{
	//	if (std::get<2>(System::name_register_map[id]) <= digit) {
	//		throw_invalid_input();
	//	}

	//	mask = pow2(digit);
	//}

	//Rot_General_Bool::Rot_General_Bool(int reg, size_t digit_, u22_t mat)
	//	: id(reg), digit(digit_), mat(mat)
	//{
	//	if (std::get<2>(System::name_register_map[id]) <= digit) {
	//		throw_invalid_input();
	//	}

	//	mask = pow2(digit);
	//}

	//void Rot_General_Bool::operate(size_t l, size_t r, std::vector<System>& state) const
	//{
	//	size_t n = r - l;
	//	constexpr size_t full_size = 2;
	//	size_t original_size = state.size();

	//	if (n == 0) return;

	//	if (_is_diagonal(mat))
	//	{
	//		_operate_diagonal(l, r, state, mat);
	//	}
	//	else if (_is_off_diagonal(mat))
	//	{
	//		_operate_off_diagonal(l, r, state, mat);
	//	}
	//	else
	//	{
	//		_operate_general(l, r, state, mat);
	//	}

	//}

	//bool Rot_General_Bool::_is_diagonal(const u22_t& data)
	//{
	//	if (abs_sqr(data[1]) < epsilon &&
	//		abs_sqr(data[2]) < epsilon)
	//	{
	//		return true;
	//	}
	//	return false;
	//}

	//void Rot_General_Bool::_operate_diagonal(size_t l, size_t r,
	//	std::vector<System>& state, const u22_t& mat) const
	//{
	//	// diagonal means that no new elements will be created
	//	// any operation can be handled in-place

	//	std::complex<double> a0 = mat[0];
	//	std::complex<double> a1 = mat[3];

	//	for (size_t i = l; i < r; ++i)
	//	{
	//		auto& s = state[i];

	//		if (s.get(id).value & mask)
	//		{
	//			s.amplitude *= a1;
	//		}
	//		else
	//		{
	//			s.amplitude *= a0;
	//		}
	//	}
	//}

	//bool Rot_General_Bool::_is_off_diagonal(const u22_t& data)
	//{
	//	if (abs_sqr(data[0]) < epsilon &&
	//		abs_sqr(data[3]) < epsilon)
	//	{
	//		return true;
	//	}
	//	return false;
	//}

	//void Rot_General_Bool::_operate_off_diagonal(size_t l, size_t r,
	//	std::vector<System>& state, const u22_t& mat) const
	//{
	//	// diagonal means that no new elements will be created
	//	// any operation can be handled in-place
	//	// with changing of storage (flipping)

	//	std::complex<double> a0 = mat[2];
	//	std::complex<double> a1 = mat[1];

	//	for (size_t i = l; i < r; ++i)
	//	{
	//		auto& s = state[i];
	//		auto& reg = s.get(id);

	//		if (reg.value & mask)
	//		{
	//			s.amplitude *= a1;
	//			reg.value ^= mask; // flip
	//		}
	//		else
	//		{
	//			s.amplitude *= a0;
	//			reg.value ^= mask; // flip
	//		}
	//	}
	//}

	//void Rot_General_Bool::_operate_general(size_t l, size_t r,
	//	std::vector<System>& state, const u22_t& mat) const
	//{
	//	size_t n = r - l;
	//	if (n == 1) // an extra entry should be added
	//	{
	//		size_t new_pos = state.size();
	//		state.push_back(state[l]);

	//		bool v = (state[l].get(id).value & mask);

	//		// if the original is 0
	//		if (!v)
	//		{
	//			state[new_pos].get(id).value ^= mask;

	//			state[l].amplitude *= mat[0];		// where |0>
	//			state[new_pos].amplitude *= mat[2]; // where |1>
	//		}
	//		// if the original is 1
	//		else
	//		{
	//			state[new_pos].get(id).value ^= mask;

	//			state[new_pos].amplitude *= mat[1]; // where |0>
	//			state[l].amplitude *= mat[3];		// where |1>
	//		}
	//	}
	//	else // everything can be computed in place
	//	{
	//		bool is_zero = state[l].GetAs(id, size_t) & pow2(digit);
	//		if (!is_zero) {
	//			complex_t a = state[l + 0].amplitude;
	//			complex_t b = state[l + 1].amplitude;
	//			state[l + 0].amplitude = a * mat[0] + b * mat[1];
	//			state[l + 1].amplitude = a * mat[2] + b * mat[3];
	//		}
	//		else {
	//			complex_t a = state[l + 0].amplitude;
	//			complex_t b = state[l + 1].amplitude;
	//			state[l + 1].amplitude = b * mat[0] + a * mat[1];
	//			state[l + 0].amplitude = b * mat[2] + a * mat[3];
	//		}
	//	}
	//}

	//void Rot_General_Bool::operator()(std::vector<System>& state) const
	//{
	//	profiler _("Rot_General_Bool");

	//	SPLIT_BY_CONDITIONS
	//	{
	//		(SortExceptBit(id, digit))(state);
	//		size_t current_size = state.size();
	//		auto iter_l = 0;
	//		auto iter_r = 1;

	//		while (true)
	//		{
	//			if (iter_r == current_size)
	//			{
	//				operate(iter_l, iter_r, state);
	//				break;
	//			}
	//			if (!compare_equal_rot(state[iter_l], state[iter_r], id, ~mask))
	//			{
	//				operate(iter_l, iter_r, state);
	//				iter_l = iter_r;
	//				iter_r = iter_l + 1;
	//			}
	//			else
	//			{
	//				iter_r++;
	//			}
	//		}
	//	}
	//	MERGE_BY_CONDITIONS

	//	ClearZero()(state);
	//	System::update_max_size(state.size());
	//}


//	Rot_Bool::Rot_Bool(std::string_view reg_in, u22_t mat_)
//		: out_id(System::get(reg_in)), mat(mat_)
//	{ }
//
//	Rot_Bool::Rot_Bool(int reg_in, u22_t mat_)
//		: out_id(reg_in), mat(mat_)
//	{ }
//
//	void Rot_Bool::operate_pair(size_t zero, size_t one, std::vector<System>& state, bool dagger) const
//	{
//		complex_t a = state[zero].amplitude;
//		complex_t b = state[one].amplitude;
//		if (!dagger)
//		{
//			state[zero].amplitude = a * mat[0] + b * mat[1];
//			state[one].amplitude = a * mat[2] + b * mat[3];
//		}
//		else
//		{
//			state[zero].amplitude = a * std::conj(mat[0]) + b * std::conj(mat[2]);
//			state[one].amplitude = a * std::conj(mat[1]) + b * std::conj(mat[3]);
//		}
//	}
//
//	void Rot_Bool::operate_alone_zero(size_t zero, std::vector<System>& state, bool dagger) const
//	{
//		state.push_back(state[zero]);
//		state.back().get(out_id).value = 1;
//
//		if (!dagger)
//		{
//			state[zero].amplitude *= mat[0];
//			state.back().amplitude *= mat[2];
//		}
//		else
//		{
//			state[zero].amplitude *= std::conj(mat[0]);
//			state.back().amplitude *= std::conj(mat[1]);
//		}
//	}
//
//	void Rot_Bool::operate_alone_one(size_t one, std::vector<System>& state, bool dagger) const
//	{
//		state.push_back(state[one]);
//		state.back().get(out_id).value = 0;
//
//		if (!dagger)
//		{
//			state.back().amplitude *= mat[1];
//			state[one].amplitude *= mat[3];
//		}
//		else
//		{
//			state.back().amplitude *= std::conj(mat[2]);
//			state[one].amplitude *= std::conj(mat[3]);
//		}
//	}
//
//	void Rot_Bool::operate(std::vector<System>& state, bool dagger) const
//	{
//		if (!state.size()) return;
//
//#ifdef SAFE_HASH
//		StateLessExceptKey pred(out_id);
//		std::map<System, size_t, StateLessExceptKey> buckets(pred);
//#else
//		auto hash_func = StateHashExceptKey(out_id);
//		for (auto& s : state)
//			s.cached_hash = hash_func(s);
//
//		std::unordered_map<size_t, size_t> buckets;
//#endif
//		size_t current_size = state.size();
//
//		for (size_t i = 0; i < current_size; ++i)
//		{
//			if (!ConditionSatisfied(state[i]))
//				return;
//#ifdef SAFE_HASH	
//			const auto& s = state[i];
//#else
//			const auto& s = state[i].cached_hash;
//#endif
//			auto iter = buckets.find(s);
//
//			if (iter == buckets.end())
//			{
//				buckets.insert({ s, i });
//				continue;
//			}
//			else
//			{
//#ifdef CHECK_HASH
//				auto pred = StateEqualExceptKey(out_id);
//				if (!pred(state[iter->second], state[i]))
//					throw_general_runtime_error();
//#endif
//
//				if (state[iter->second].get(out_id).as_bool())
//				{
//					// stored 1 and now 0
//					operate_pair(i, iter->second, state, dagger);
//				}
//				else
//				{
//					// stored 0 and now 1
//					operate_pair(iter->second, i, state, dagger);
//				}
//				buckets.erase(iter);
//			}
//		}
//
//		for (auto& stored_key : buckets)
//		{
//			// alone
//			if (state[stored_key.second].get(out_id).as_bool())
//			{
//				operate_alone_one(stored_key.second, state, dagger);
//			}
//			else
//			{
//				operate_alone_zero(stored_key.second, state, dagger);
//			}
//		}
//
//		ClearZero()(state);
//		System::update_max_size(state.size());
//	}
//
//	void Rot_Bool::operator()(std::vector<System>& state) const
//	{
//		profiler _("Rot_Bool_v2");
//		operate(state, false);
//	}
//
//	void Rot_Bool::dag(std::vector<System>& state) const
//	{
//		profiler _("Rot_Bool_v2::dag");
//		operate(state, true);
//	}


	void Rot_GeneralUnitary::operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state, bool dagger) const
	{
		/* Assign amplitudes to a temporary array */
		std::vector<complex_t> amps(positions.size());
		for (size_t i = 0; i < positions.size(); ++i)
		{
			amps[i] = state[positions[i]].amplitude;
		}

		std::vector<complex_t> new_amps(positions.size(), 0);

		/* Use matrix-vector multiplication */
		if (!dagger) {
			for (size_t i = 0; i < full_size; ++i)
			{
				for (size_t j = 0; j < full_size; ++j)
				{
					new_amps[i] += mat(i, j) * amps[j];
				}
			}
		}
		else {
			for (size_t i = 0; i < full_size; ++i)
			{
				for (size_t j = 0; j < full_size; ++j)
				{
					new_amps[i] += std::conj(mat(j, i)) * amps[j];
				}
			}
		}

		/* Return their amplitudes */
		for (size_t i = 0; i < positions.size(); ++i)
		{
			state[positions[i]].amplitude = new_amps[i];
		}
	}

	void Rot_GeneralUnitary::operate(std::vector<System>& state, bool dagger) const
	{
		profiler _("Rot_GeneralUnitary");

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
		std::unordered_map<size_t, size_t> buckets;
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
				operate_bucket_inplace(iter->second.second, state, dagger);
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
			operate_bucket_inplace(all_pos, state, dagger);
		}
		ClearZero()(state);
		System::update_max_size(state.size());
	}

	void Rot_GeneralUnitary::operator()(std::vector<System>& state) const
	{
		profiler _("Rot_GeneralUnitary::dag");
		operate(state, false);
	}

	void Rot_GeneralUnitary::dag(std::vector<System>& state) const
	{
		profiler _("Rot_GeneralUnitary");
		operate(state, true);
	}

	static void normalize(std::vector<complex_t>& psi) {
		double norm = 0.0;
		for (const auto& element : psi) {
			norm += abs_sqr(element);
		}
		norm = std::sqrt(norm);
		for (auto& element : psi) {
			element /= norm;
		}
	}

	DenseMatrix<complex_t> stateprep_unitary_build_schmidt(const std::vector<complex_t>& psi) {
		auto N = psi.size(); // Dimension of the state vector
		DenseMatrix<complex_t> ret(N);

		// Fill U such that U|0> = |psi>
		for (size_t j = 0; j < N; ++j){
			for (size_t i = 0; i < N; ++i) {
				if (j == 0)
					// U[i][0] should be psi[i]
					ret(i, j) = psi[i];

				else
					ret(i, j) = (i == j ? 1 : 0);
			}
			normalize_column(ret, j);
		}		
		gram_schmidt_process(ret);

		return ret;
	}

	Rot_GeneralStatePrep::Rot_GeneralStatePrep(std::string_view reg_in, const std::vector<complex_t>& vec_)
		: id(System::get(reg_in)), 
		vec(vec_), 
		rot_general(System::get(reg_in), stateprep_unitary_build_schmidt(vec_))
	{
		n_digits = System::size_of(id);
		full_size = pow2(n_digits);

		if (full_size != vec.size())
			throw_invalid_input("Matrix size does not match the register's size.");
	}

	Rot_GeneralStatePrep::Rot_GeneralStatePrep(size_t reg_in, const std::vector<complex_t>& vec_)
		: id(reg_in),
		vec(vec_),
		rot_general(reg_in, stateprep_unitary_build_schmidt(vec_))
	{
		n_digits = System::size_of(id);
		full_size = pow2(n_digits);

		if (full_size != vec.size())
			throw_invalid_input("Matrix size does not match the register's size.");
	}

	void Rot_GeneralStatePrep::operator()(std::vector<System>& state) const
	{
		rot_general(state);
	}

	void Rot_GeneralStatePrep::dag(std::vector<System>& state) const
	{
		rot_general.dag(state);
	}
}