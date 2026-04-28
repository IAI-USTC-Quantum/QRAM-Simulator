#include "quantum_arithmetic.h"

namespace qram_simulator
{

	void FlipBools::operator()(std::vector<System>& state) const
	{
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

			auto& reg = s.get(id);
			reg.value = ~reg.value;
		}
	}

	void Swap_Bool_Bool::operator()(std::vector<System>& state) const
	{
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

			auto& reg1 = s.get(lhs);
			auto& reg2 = s.get(rhs);
			bool v1 = get_digit(reg1.value, digit1);
			bool v2 = get_digit(reg2.value, digit2);
			if (v1 && (!v2))
			{
				reg1.value -= pow2(digit1);
				reg2.value += pow2(digit2);
			}
			if (v2 && (!v1))
			{
				reg1.value += pow2(digit1);
				reg2.value -= pow2(digit2);
			}
		}
	}

	void ShiftLeft_InPlace::operator()(std::vector<System>& state) const
	{
		size_t size = System::size_of(register_1);
		//change >= to >
		if (digit > size)
			throw_invalid_input();

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

			uint64_t value = s.GetAs(register_1, uint64_t);
			/* [digit, size-digit]*/

			uint64_t high = value >> (size - digit);
			uint64_t low = value - (high << (size - digit));
			s.get(register_1).value = (low << digit) + high;
			//Debug_CheckOverflow(register_1);
		}
	}

	// high(size-d)-low(d)
	// low-high

	void ShiftRight_InPlace::operator()(std::vector<System>& state) const
	{
		size_t size = System::size_of(register_1);
		//change >= to >
		if (digit > size)
			throw_invalid_input();

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

			uint64_t value = s.GetAs(register_1, uint64_t);
			/* [digit, size-digit]*/

			uint64_t high = value >> digit; // high
			uint64_t low = value - (high << (digit)); // low
			s.get(register_1).value = (low << (size - digit)) + high;
		}
	}

	void ShiftLeft_InPlace::dag(std::vector<System>& state) const
	{
		ShiftRight_InPlace{register_1, digit}(state);
	}

	void ShiftRight_InPlace::dag(std::vector<System>& state) const
	{
		ShiftLeft_InPlace{register_1, digit}(state);
	}

	void Mult_UInt_ConstUInt::operator()(std::vector<System>& state) const
	{
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

			auto& reg_out = s.get(res);
			reg_out.value ^= (s.GetAs(lhs, uint64_t) * mult_int);
		}
	}

	void Add_Mult_UInt_ConstUInt_InPlace::operator()(std::vector<System>& state) const
	{
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
			auto dim = System::size_of(res);
			auto dim_val = 1ULL << dim;
			s.get(res).value += (mult_int * s.GetAs(lhs, uint64_t));
			s.get(res).value %= dim_val;
		}
	}


	void Add_Mult_UInt_ConstUInt_InPlace::dag(std::vector<System>& state) const
	{
		auto dim = System::size_of(res);
		auto dim_val = 1ULL << dim;

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
			// Inverse: res -= lhs * mult (mod 2^dim).  lhs is NOT modified.
			auto lhs_val = s.GetAs(lhs, uint64_t);
			s.get(res).value += (dim_val - lhs_val * mult_int % dim_val);
			s.get(res).value %= dim_val;
		}
	}



	Mod_Mult_UInt_ConstUInt_InPlace::Mod_Mult_UInt_ConstUInt_InPlace(std::string_view reg_name, uint64_t a_, uint64_t x_, uint64_t N_)
		: reg(System::get(reg_name)), a(a_), x(x_), N(N_)
	{
		if (std::gcd(a, N) > 1)
			throw std::invalid_argument("a and N must be coprime");

		opnum = a % N;
		for (size_t i = 0; i < x; ++i)
			opnum = (opnum * opnum) % N;
	}

	Mod_Mult_UInt_ConstUInt_InPlace::Mod_Mult_UInt_ConstUInt_InPlace(size_t reg_id, uint64_t a_, uint64_t x_, uint64_t N_)
		: reg(reg_id), a(a_), x(x_), N(N_)
	{
		if (std::gcd(a, N) > 1)
			throw std::invalid_argument("a and N must be coprime");

		opnum = a % N;
		for (size_t i = 0; i < x; ++i)
			opnum = (opnum * opnum) % N;
	}

	void Mod_Mult_UInt_ConstUInt_InPlace::operator()(std::vector<System>& state) const
	{
		profiler _("Mod_Mult_UInt_ConstUInt_InPlace");

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

			uint64_t val = s.GetAs(reg, uint64_t);
			val = (val * opnum) % N;
			s.get(reg).value = val;
		}
	}

	void Mod_Mult_UInt_ConstUInt_InPlace::dag(std::vector<System>& state) const
	{
		profiler _("Mod_Mult_UInt_ConstUInt_dag");
		// Compute modular inverse using extended Euclidean algorithm
		int64_t t = 0, new_t = 1;
		int64_t r_gcd = (int64_t)N, new_r_gcd = (int64_t)opnum;
		while (new_r_gcd != 0) {
			int64_t quotient = r_gcd / new_r_gcd;
			int64_t temp_t = t - quotient * new_t;
			int64_t temp_r = r_gcd - quotient * new_r_gcd;
			t = new_t;
			r_gcd = new_r_gcd;
			new_t = temp_t;
			new_r_gcd = temp_r;
		}
		uint64_t inverse_opnum;
		if (r_gcd == 1) {
			int64_t t_normalized = t % (int64_t)N;
			if (t_normalized < 0) t_normalized += (int64_t)N;
			inverse_opnum = (uint64_t)t_normalized;
		} else {
			inverse_opnum = 1;
		}

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

			uint64_t val = s.GetAs(reg, uint64_t);
			val = (val * inverse_opnum) % N;
			s.get(reg).value = val;
		}
	}


	void Add_UInt_UInt::operator()(std::vector<System>& state) const
	{
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

			s.get(res).value ^= (s.GetAs(lhs, uint64_t) + s.GetAs(rhs, uint64_t));
		}
	}

	void Add_UInt_UInt_InPlace::operator()(std::vector<System>& state) const
	{
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

			s.get(rhs).value += s.GetAs(lhs, uint64_t);
		}
	}

	void Add_UInt_UInt_InPlace::dag(std::vector<System>& state) const
	{
		auto dim = System::size_of(rhs);

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

			s.get(rhs).value += (pow2(dim) - s.GetAs(lhs, uint64_t));
			s.get(rhs).value = s.get(rhs).value % pow2(dim);
		}
	}

	void Add_UInt_ConstUInt::operator()(std::vector<System>& state) const
	{
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

			auto& reg_out = s.get(res);
			reg_out.value ^= (s.GetAs(lhs, uint64_t) + add_int);
			//Debug_CheckOverflow(res);
		}
	}

	void Add_ConstUInt_InPlace::operator()(std::vector<System>& state) const
	{
		auto dim = System::size_of(reg_in);

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

			auto& reg_ = s.get(reg_in);
			reg_.value += add_int;
			reg_.value = reg_.value % pow2(dim);
		}
	}

	void Add_ConstUInt_InPlace::dag(std::vector<System>& state) const
	{
		auto dim = System::size_of(reg_in);

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

			auto& reg_ = s.get(reg_in);
			reg_.value += (pow2(dim) - add_int);
			reg_.value = reg_.value % pow2(dim);
		}
	}

	void Div_Sqrt_Arccos_Int_Int::operator()(std::vector<System>& state) const
	{
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

			uint64_t regl_v = s.GetAs(register_lhs, uint64_t);
			uint64_t regr_v = s.GetAs(register_rhs, uint64_t);
			double out = std::acos(std::sqrt(1.0 * regl_v / regr_v)) / pi / 2;
			auto& regout = s.get(register_out);
			regout.value ^= get_rational(out, System::size_of(register_out));
		}
	}

	void Sqrt_Div_Arccos_Int_Int::operator()(std::vector<System>& state) const
	{
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

			int64_t lvalue = s.GetAs(register_lhs, int64_t);
			uint64_t rvalue = s.GetAs(register_rhs, uint64_t);

			double out = std::acos(lvalue / std::sqrt(rvalue)) / pi / 2;
			auto& regout = s.get(register_out);
			regout.value ^= get_rational(out, System::size_of(register_out));
		}
	}

	void GetRotateAngle_Int_Int::operator()(std::vector<System>& state) const
	{
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

			auto& regl = s.get(register_lhs);
			auto& regr = s.get(register_rhs);
			auto l_complement = s.GetAs(register_lhs, uint64_t);
			auto r_complement = s.GetAs(register_rhs, uint64_t);
			auto l = get_complement(l_complement, System::size_of(register_lhs));
			auto r = get_complement(r_complement, System::size_of(register_rhs));
			double out;
			if (l == 0 && r >= 0) { out = 0.25; }
			else if (l == 0 && r < 0) { out = 0.75; }
			else {
				out = std::atan2(r, l) / pi / 2;
				if (out < 0) out += 1;
			}
			auto& regout = s.get(register_out);
			regout.value ^= get_rational(out, System::size_of(register_out));
		}
	}

	void AddAssign_AnyInt_AnyInt_InPlace::_operate_uint_uint(uint64_t& lhs, size_t l_size, uint64_t rhs, size_t r_size)
	{
		lhs += rhs;
		lhs %= pow2(l_size);
	}

	void AddAssign_AnyInt_AnyInt_InPlace::_operate_uint_uint_dag(uint64_t& lhs, size_t l_size, uint64_t rhs, size_t r_size)
	{
		lhs -= rhs;
		lhs %= pow2(l_size);
	}

	void AddAssign_AnyInt_AnyInt_InPlace::operator()(std::vector<System>& state) const
	{
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

			_operate_uint_uint(
				s.get(lhs_id).value, lhs_size,
				s.get(rhs_id).as<uint64_t>(rhs_size), rhs_size);
		}
	}

	void AddAssign_AnyInt_AnyInt_InPlace::dag(std::vector<System>& state) const
	{
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

			_operate_uint_uint_dag(
				s.get(lhs_id).value, lhs_size,
				s.get(rhs_id).as<uint64_t>(rhs_size), rhs_size);
		}
	}

	void Assign::operator()(std::vector<System>& state) const
	{
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

			auto& reg1 = s.get(register_1);
			auto& reg2 = s.get(register_2);
			reg2.value ^= reg1.value;

		}
	}

	void Compare_UInt_UInt::operator()(std::vector<System>& state) const
	{
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

			uint64_t l = s.GetAs(left_id, uint64_t);
			uint64_t r = s.GetAs(right_id, uint64_t);

			if (l == r)
			{
				s.get(compare_equal_id).value ^= 1;
			}
			else
			{
				s.get(compare_less_id).value ^= (l < r);
			}
		}
	}


	void Less_UInt_UInt::operator()(std::vector<System>& state) const
	{
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

			uint64_t l = s.GetAs(left_id, uint64_t);
			uint64_t r = s.GetAs(right_id, uint64_t);

			s.get(compare_less_id).value ^= (l < r);
		}
	}

	void Swap_General_General::operator()(std::vector<System>& state) const
	{
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
			//Debug_CheckOverflow(id1);
			//Debug_CheckOverflow(id2);
			std::swap(s.get(id1).value, s.get(id2).value);
		}
	}

	void GetMid_UInt_UInt::operator()(std::vector<System>& state) const
	{
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

			uint64_t l = s.GetAs(left_id, uint64_t);
			uint64_t r = s.GetAs(right_id, uint64_t);

			s.get(mid_id).value ^= (l + r) / 2;
		}
	}
}