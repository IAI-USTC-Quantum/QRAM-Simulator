#pragma once
#include "quantum_interfere_basic.h"

namespace qram_simulator
{
	HOST_DEVICE inline u22_t make_func(uint64_t value, size_t n_digit)
	{
		double theta = 0;
		if (n_digit == 64)
			theta = value * 1.0 / 2 / pow2(63);
		else
			theta = value * 1.0 / pow2(n_digit);

		theta *= (2 * pi);

		complex_t u00 = cos(theta),
			u01 = -sin(theta),
			u10 = sin(theta),
			u11 = cos(theta);

		return u22_t{ u00, u01, u10, u11 };
	}

	HOST_DEVICE inline u22_t make_func_inv(uint64_t value, size_t n_digit)
	{
		double theta = 0;
		if (n_digit == 64)
			theta = value * 1.0 / 2 / pow2(63);
		else
			theta = value * 1.0 / pow2(n_digit);

		theta *= (2 * pi);

		complex_t u00 = cos(theta),
			u01 = sin(theta),
			u10 = -sin(theta),
			u11 = cos(theta);

		return u22_t{ u00, u01, u10, u11 };
	}

	/* Conditional Rotation with a rational number */
	struct CondRot_Rational_Bool : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t register_in;
		size_t register_out;
		CondRot_Rational_Bool(std::string_view reg_in, std::string_view reg_out)
			:register_in(System::get(reg_in)), register_out(System::get(reg_out))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_in) != Rational ||
				System::type_of(register_out) != Boolean)
				throw_invalid_input();
#endif
		}
		CondRot_Rational_Bool(size_t reg_in, size_t reg_out)
			:register_in(reg_in), register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_in) != Rational ||
				System::type_of(register_out) != Boolean)
				throw_invalid_input();
#endif
		}

		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
	};

	template<typename Callable = std::function<u22_t(uint64_t)>>
	struct CondRot_General_Bool : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t in_id;
		size_t out_id;
		Callable func;

		CondRot_General_Bool(std::string_view reg_in, std::string_view reg_out, Callable angle_function)
			: in_id(System::get(reg_in)), out_id(System::get(reg_out)), func(angle_function)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(reg_out) != Boolean)
				throw_invalid_input();

			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
#endif
		}

		CondRot_General_Bool(size_t reg_in, size_t reg_out, Callable angle_function)
			: in_id(reg_in), out_id(reg_out), func(angle_function)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(reg_out) != Boolean)
				throw_invalid_input();

			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
#endif
		}
	
		//void operate(size_t l, size_t r, std::vector<System>& state) const
		//{
		//	size_t n = r - l;
		//	constexpr size_t full_size = 2;
		//	size_t original_size = state.size();
		//	if (n == 0) return;

		//	// 1. get the rotation matrix
		//	System& sys = state[l];
		//	// uint64_t v = sys.GetAs(in_id, uint64_t);
		//	StateStorage& storage = sys.get(in_id);
		//	uint64_t v = storage.as<uint64_t>(System::size_of(in_id));

		//	u22_t mat = func(v);
		//	// _operate_general(l, r, state, mat);
		//	// return;

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

		//static bool _is_diagonal(const u22_t& data)
		//{
		//	if (abs_sqr(data[1]) < epsilon &&
		//		abs_sqr(data[2]) < epsilon)
		//	{
		//		return true;
		//	}
		//	return false;
		//}

		//void _operate_diagonal(size_t l, size_t r,
		//	std::vector<System>& state, const u22_t& mat) const
		//{
		//	// diagonal means that no new elements will be created
		//	// any operation can be handled in-place

		//	std::complex<double> a0 = mat[0];
		//	std::complex<double> a1 = mat[3];

		//	for (size_t i = l; i < r; ++i)
		//	{
		//		auto& s = state[i];
		//		StateStorage storage = s.get(out_id);
		//		if (storage.as<bool>(1))
		//		{
		//			s.amplitude *= a1;
		//		}
		//		else
		//		{
		//			s.amplitude *= a0;
		//		}
		//	}
		//}

		//static bool _is_off_diagonal(const u22_t& data)
		//{
		//	if (abs_sqr(data[0]) < epsilon &&
		//		abs_sqr(data[3]) < epsilon)
		//	{
		//		return true;
		//	}
		//	return false;
		//}

		//void _operate_off_diagonal(size_t l, size_t r,
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
		//		StateStorage& reg = s.get(out_id);
		//		if (reg.as<bool>(1))
		//		{
		//			s.amplitude *= a1;
		//			reg.value = 0; // flip
		//		}
		//		else
		//		{
		//			s.amplitude *= a0;
		//			reg.value = 1; // flip
		//		}
		//	}
		//}

		//void _operate_general(size_t l, size_t r,
		//	std::vector<System>& state, const u22_t& mat) const
		//{
		//	size_t n = r - l;
		//	if (n == 1) // an extra entry should be added
		//	{
		//		size_t new_pos = state.size();
		//		state.push_back(state[l]);
		//		StateStorage& storage = state[l].get(out_id);
		//		bool v = storage.as<bool>(1);

		//		// if the original is 0
		//		if (!v)
		//		{
		//			state[new_pos].get(out_id).value = 1;

		//			state[l].amplitude *= mat[0];		// where |0>
		//			state[new_pos].amplitude *= mat[2]; // where |1>
		//		}
		//		// if the original is 1
		//		else
		//		{
		//			state[new_pos].get(out_id).value = 0;

		//			state[new_pos].amplitude *= mat[1]; // where |0>
		//			state[l].amplitude *= mat[3];		// where |1>
		//		}
		//	}
		//	else // everything can be computed in place
		//	{
		//		complex_t a = state[l + 0].amplitude;
		//		complex_t b = state[l + 1].amplitude;
		//		state[l + 0].amplitude = a * mat[0] + b * mat[1];
		//		state[l + 1].amplitude = a * mat[2] + b * mat[3];
		//	}
		//}

		/* V2 */
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const
		{
			//uint64_t v = state[zero].GetAs(in_id, uint64_t);
			StateStorage& storage = state[zero].get(in_id);
			uint64_t v = storage.as<uint64_t>(System::size_of(in_id));
			u22_t mat = func(v);

			complex_t a = state[zero].amplitude;
			complex_t b = state[one].amplitude;
			state[zero].amplitude = a * mat[0] + b * mat[1];
			state[one].amplitude = a * mat[2] + b * mat[3];
		}
		void operate_alone_zero(size_t zero, std::vector<System>& state) const
		{
			//uint64_t v = state[zero].GetAs(in_id, uint64_t);
			StateStorage& storage = state[zero].get(in_id);
			uint64_t v = storage.as<uint64_t>(System::size_of(in_id));
			u22_t mat = func(v);

			state.push_back(state[zero]);
			state.back().get(out_id).value = 1;

			state[zero].amplitude *= mat[0];
			state.back().amplitude *= mat[2];
		}
		void operate_alone_one(size_t one, std::vector<System>& state) const
		{
			//uint64_t v = state[one].GetAs(in_id, uint64_t);
			StateStorage& storage = state[one].get(in_id);
			uint64_t v = storage.as<uint64_t>(System::size_of(in_id));
			u22_t mat = func(v);

			state.push_back(state[one]);
			state.back().get(out_id).value = 0;

			state.back().amplitude *= mat[1];
			state[one].amplitude *= mat[3];
		}

		void operator()(std::vector<System>& state) const
		{
#define CONDROT_VERSION 2
#if CONDROT_VERSION == 1
			{
				profiler _("CondRot_General_Bool_v1");
				if (!state.size()) return;

				(SortExceptKey(out_id))(state);
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
					if (!compare_equal(state[iter_l], state[iter_r], out_id))
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
			}
#elif CONDROT_VERSION == 2
			{
				profiler _("CondRot_General_Bool_v2");

				if (!state.size()) return;

				//auto hash_func = StateHashExceptKey(out_id);
				//for (auto& s : state)
				//	s.cached_hash = hash_func(s);

				//std::unordered_map<System, size_t, StateHashExceptKey, StateEqualExceptKey> buckets(
				//	0, StateHashExceptKey(out_id), StateEqualExceptKey(out_id)
				//);

				/*StateHashExceptKey hash(out_id);
				StateEqualExceptKey pred(out_id);
				std::unordered_map<System, size_t, StateHashExceptKey, StateEqualExceptKey> buckets(0, hash, pred);*/

				// correct version
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
#ifdef SAFE_HASH
					const auto& s = state[i];
#else
					const auto& s = state[i].cached_hash;
#endif
					auto iter = buckets.find(s);
					if (iter == buckets.end())
					{
						buckets.insert({ s, i });
						// fmt::print("{}\n", buckets.size());
						continue;
					}
					else
					{
#ifdef CHECK_HASH
						auto pred = StateEqualExceptKey(out_id);
						if (!pred(state[iter->second], state[i]))
							throw_general_runtime_error();
#endif
						StateStorage& storage = state[iter->second].get(out_id);
						if (storage.as<bool>(1))
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
					StateStorage& storage = state[stored_key.second].get(out_id);
					if (storage.as<bool>(1))
					{
						operate_alone_one(stored_key.second, state);
					}
					else
					{
						operate_alone_zero(stored_key.second, state);
					}
				}
				ClearZero()(state);
			}
#endif
		}
#ifdef USE_CUDA
		void operator()(CuSparseState& s) const;
#endif
	};

}