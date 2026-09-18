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
			const auto size = System::size_of(id);
			const auto mask = width_mask(size);
			reg.value = (~reg.value) & mask;
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
		ShiftRight_InPlace inverse{register_1, digit};
		copy_control_conditions_to(inverse);
		inverse(state);
	}

	void ShiftRight_InPlace::dag(std::vector<System>& state) const
	{
		ShiftLeft_InPlace inverse{register_1, digit};
		copy_control_conditions_to(inverse);
		inverse(state);
	}

	void Mult_UInt_ConstUInt::operator()(std::vector<System>& state) const
	{
		const auto mask = width_mask(System::size_of(res));
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
			reg_out.value = (reg_out.value ^ (s.GetAs(lhs, uint64_t) * mult_int)) & mask;
		}
	}

	void Add_Mult_UInt_ConstUInt_InPlace::operator()(std::vector<System>& state) const
	{
		const auto mask = width_mask(System::size_of(res));
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
			auto& result = s.get(res).value;
			result = (result + mult_int * s.GetAs(lhs, uint64_t)) & mask;
		}
	}


	void Add_Mult_UInt_ConstUInt_InPlace::dag(std::vector<System>& state) const
	{
		const auto mask = width_mask(System::size_of(res));

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
			auto& result = s.get(res).value;
			result = (result - s.GetAs(lhs, uint64_t) * mult_int) & mask;
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (s.GetAs(lhs, uint64_t) + s.GetAs(rhs, uint64_t))) & mask;
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

			auto& result = s.get(rhs).value;
			const auto size = System::size_of(rhs);
			const auto mask = width_mask(size);
			result = (result + s.GetAs(lhs, uint64_t)) & mask;
		}
	}

	void Add_UInt_UInt_InPlace::dag(std::vector<System>& state) const
	{
		const auto size = System::size_of(rhs);
		const auto mask = size == 64 ? ~uint64_t{0} : pow2(size) - 1;

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

			auto& result = s.get(rhs).value;
			result = (result - s.GetAs(lhs, uint64_t)) & mask;
		}
	}

	void Add_UInt_ConstUInt::operator()(std::vector<System>& state) const
	{
		const auto mask = width_mask(System::size_of(res));
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
			reg_out.value = (reg_out.value ^ (s.GetAs(lhs, uint64_t) + add_int)) & mask;
		}
	}

	void Add_ConstUInt_InPlace::operator()(std::vector<System>& state) const
	{
		const auto mask = width_mask(System::size_of(reg_in));

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
			reg_.value = (reg_.value + add_int) & mask;
		}
	}

	void Add_ConstUInt_InPlace::dag(std::vector<System>& state) const
	{
		const auto mask = width_mask(System::size_of(reg_in));

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
			reg_.value = (reg_.value - add_int) & mask;
		}
	}

	void Div_Sqrt_Arccos_UInt_UInt::operator()(std::vector<System>& state) const
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

	void Sqrt_Div_Arccos_Int_UInt::operator()(std::vector<System>& state) const
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

	void Sub_UInt_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (s.GetAs(lhs, uint64_t) - s.GetAs(rhs, uint64_t))) & mask;
		}
	}

	void Neg_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (uint64_t{0} - s.GetAs(reg, uint64_t))) & mask;
		}
	}

	void Abs_SInt::operator()(std::vector<System>& state) const
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

			const int64_t v = get_complement(s.GetAs(reg, uint64_t), System::size_of(reg));
			/* 最小负数（w = 64）回绕为自身 */
			const uint64_t magnitude = v < 0 ? (uint64_t)(-v) : (uint64_t)v;
			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ magnitude) & mask;
		}
	}

	void Mul_UInt_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (s.GetAs(lhs, uint64_t) * s.GetAs(rhs, uint64_t))) & mask;
		}
	}

	void Div_UInt_UInt::operator()(std::vector<System>& state) const
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

			/* 全域化：除数为零时商取 0（宽度与截断约定） */
			const uint64_t a = s.GetAs(lhs, uint64_t);
			const uint64_t b = s.GetAs(rhs, uint64_t);
			const uint64_t quotient = b == 0 ? uint64_t{0} : a / b;
			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ quotient) & mask;
		}
	}

	void Sqrt_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ isqrt_u64(s.GetAs(reg, uint64_t))) & mask;
		}
	}

	void Select_Bool_UInt_UInt::operator()(std::vector<System>& state) const
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

			const uint64_t cond_bit = s.GetAs(cond, uint64_t) & 1ull;
			const uint64_t selected = cond_bit != 0 ? s.GetAs(lhs, uint64_t) : s.GetAs(rhs, uint64_t);
			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ selected) & mask;
		}
	}

	void And_UInt_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (s.GetAs(lhs, uint64_t) & s.GetAs(rhs, uint64_t))) & mask;
		}
	}

	void Or_UInt_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (s.GetAs(lhs, uint64_t) | s.GetAs(rhs, uint64_t))) & mask;
		}
	}

	void Xor_UInt_UInt::operator()(std::vector<System>& state) const
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

			auto& result = s.get(res).value;
			const auto size = System::size_of(res);
			const auto mask = width_mask(size);
			result = (result ^ (s.GetAs(lhs, uint64_t) ^ s.GetAs(rhs, uint64_t))) & mask;
		}
	}

	void Less_SInt_SInt::operator()(std::vector<System>& state) const
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

			const int64_t l = get_complement(s.GetAs(lhs, uint64_t), System::size_of(lhs));
			const int64_t r = get_complement(s.GetAs(rhs, uint64_t), System::size_of(rhs));
			const bool pred = l < r;
			auto& flag = s.get(flag_id);
			flag.value = flag.value ^ (pred ? 1ull : 0ull);
		}
	}

	void Carry_UInt_UInt::operator()(std::vector<System>& state) const
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

			/* res 仅提供宽度 w，不读其值（宽度与截断约定） */
			const uint64_t a = s.GetAs(lhs, uint64_t);
			const uint64_t b = s.GetAs(rhs, uint64_t);
			const size_t w = System::size_of(res);
			const bool pred = w >= 64 ? (a + b < a) :
				(a >= (1ull << w)) || (b >= (1ull << w) - a);
			auto& flag = s.get(flag_id);
			flag.value = flag.value ^ (pred ? 1ull : 0ull);
		}
	}

	void Overflow_SInt_SInt::operator()(std::vector<System>& state) const
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

			/* res 仅提供宽度 w，不读其值（宽度与截断约定） */
			const size_t w = System::size_of(res);
			const uint64_t mask = width_mask(w);
			const uint64_t A = static_cast<uint64_t>(
				get_complement(s.GetAs(lhs, uint64_t), System::size_of(lhs))) & mask;
			const uint64_t B = static_cast<uint64_t>(
				get_complement(s.GetAs(rhs, uint64_t), System::size_of(rhs))) & mask;
			const uint64_t S = (A + B) & mask;
			const uint64_t signA = (A >> (w - 1)) & 1ull;
			const uint64_t signB = (B >> (w - 1)) & 1ull;
			const uint64_t signS = (S >> (w - 1)) & 1ull;
			const bool pred = (signA == signB) && (signS != signA);
			auto& flag = s.get(flag_id);
			flag.value = flag.value ^ (pred ? 1ull : 0ull);
		}
	}

	void MulOverflow_UInt_UInt::operator()(std::vector<System>& state) const
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

			/* res 仅提供宽度 w，不读其值（宽度与截断约定） */
			const uint64_t a = s.GetAs(lhs, uint64_t);
			const uint64_t b = s.GetAs(rhs, uint64_t);
			const uint64_t lo = a * b;
			/* 128 位乘积高 64 位：32 位分块（与 CUDA 侧 mul_hi_u64 位等价） */
			const uint64_t a_lo = uint32_t(a), a_hi = a >> 32;
			const uint64_t b_lo = uint32_t(b), b_hi = b >> 32;
			const uint64_t p0 = a_lo * b_lo, p1 = a_lo * b_hi;
			const uint64_t p2 = a_hi * b_lo, p3 = a_hi * b_hi;
			const uint64_t hi = p3 + (p1 >> 32) + (p2 >> 32)
				+ (((p0 >> 32) + uint32_t(p1) + uint32_t(p2)) >> 32);
			const size_t w = System::size_of(res);
			const bool pred = hi != 0 || (w < 64 && lo >= (1ull << w));
			auto& flag = s.get(flag_id);
			flag.value = flag.value ^ (pred ? 1ull : 0ull);
		}
	}

	void IsZero_UInt::operator()(std::vector<System>& state) const
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

			const bool pred = s.GetAs(reg, uint64_t) == 0;
			auto& flag = s.get(flag_id);
			flag.value = flag.value ^ (pred ? 1ull : 0ull);
		}
	}

	void Negative_SInt::operator()(std::vector<System>& state) const
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

			const bool pred = get_complement(s.GetAs(reg, uint64_t), System::size_of(reg)) < 0;
			auto& flag = s.get(flag_id);
			flag.value = flag.value ^ (pred ? 1ull : 0ull);
		}
	}

	uint64_t Add_AnyInt_AnyInt_InPlace::_extended_rhs(const System& s, size_t id)
	{
		const size_t size = System::size_of(id);
		const uint64_t raw = s.get(id).as<uint64_t>(size);
		// AnyInt 槽：按寄存器声明类型扩展（宽度与截断约定，见 docs/operators.md）
		if (System::type_of(id) == SignedInteger)
			return static_cast<uint64_t>(get_complement(raw, size));
		return raw;
	}

	void Add_AnyInt_AnyInt_InPlace::operator()(std::vector<System>& state) const
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

			const size_t lhs_size = System::size_of(lhs_id);
			s.get(lhs_id).value =
				(s.get(lhs_id).value + _extended_rhs(s, rhs_id)) & width_mask(lhs_size);
		}
	}

	void Add_AnyInt_AnyInt_InPlace::dag(std::vector<System>& state) const
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

			const size_t lhs_size = System::size_of(lhs_id);
			s.get(lhs_id).value =
				(s.get(lhs_id).value - _extended_rhs(s, rhs_id)) & width_mask(lhs_size);
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
			const auto size = System::size_of(register_2);
			const auto mask = width_mask(size);
			reg2.value = (reg2.value ^ reg1.value) & mask;

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