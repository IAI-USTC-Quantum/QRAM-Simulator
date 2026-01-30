#include "simple_quantum_simulator.h"

namespace qram_simulator
{
	namespace quantum_simulator
	{
		size_t extract_binary(size_t index, const std::vector<size_t>& digits)
		{
			size_t ret = 0;
			for (size_t i = 0; i < digits.size(); ++i) {
				ret += (((index >> digits[i]) & 1) << i);
			}
			return ret;
		}

		size_t mask(const std::vector<size_t>& mask_digits)
		{
			size_t mask = 0;
			for (auto digit : mask_digits)
			{
				mask += (size_t(1) << digit);
			}
			return mask;
		}

		size_t reconstruct_binary(size_t index, const std::vector<size_t>& digits)
		{
			size_t ret = 0;
			if (index == 0) return 0;
			for (size_t i = 0; i < digits.size(); ++i) {
				ret += (((index >> i) & 1) << digits[i]);
			}
			return ret;
		}

		size_t mask_remain(size_t index, const std::vector<size_t>& mask_digits)
		{
			return index & ~mask(mask_digits);
		}

		size_t mask_replace(size_t index, const std::vector<size_t>& mask, size_t new_value)
		{
			size_t original = index;
			size_t i = 0;
			for (auto digit : mask)
			{
				if ((new_value >> i) & 1)
					index |= (size_t(1) << digit);
				else
					index &= ~(size_t(1) << digit);
				i++;
			}
			return index;
		}

		void insert_binary(size_t& remain, const std::vector<size_t>& digits, size_t number)
		{
			for (size_t i = 0; i < digits.size(); ++i) {
				remain += (((number >> i) & 1) << digits[i]);
			}
		}

		void discard(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n, size_t mask_target)
		{
			std::vector<complex_t> temp_state(pow2(n - qn.size()));
			size_t m = mask(qn);
			for (size_t i = 0; i < state.size(); ++i)
			{
				if ((i & m) == mask_target)
				{
					temp_state[discard(i, qn)] = state[i];
				}
			}
			state = temp_state;
		}

		inline std::vector<size_t> _get_remain_qubits(
			size_t sz,
			std::vector<size_t> address_qubits,
			const std::vector<size_t>& data_qubits) {

			address_qubits.insert(address_qubits.end(), data_qubits.begin(), data_qubits.end());
			std::sort(address_qubits.begin(), address_qubits.end());

			size_t ptr = 0;
			std::vector<size_t> remain_qubits;
			bool overflow = false;
			for (size_t i = 0; (sz >> i) != 1; ++i) {
				if (ptr >= address_qubits.size())
					overflow = true;

				if (!overflow) {
					if (i == address_qubits[ptr]) {
						ptr++;
					}
					else if (i < address_qubits[ptr]) {
						remain_qubits.push_back(i);
					}
				}
				else {
					remain_qubits.push_back(i);
				}
			}
			return remain_qubits;
		}

		size_t discard(size_t n, std::vector<size_t> digits)
		{
			std::sort(digits.begin(), digits.end());
			for (auto iter = digits.rbegin(); iter != digits.rend(); ++iter)
			{
				auto digit = *iter;
				auto mask = pow2(digit) - 1;
				auto lower = mask & n;
				n = ((n - lower) >> (digit + 1) << digit) + lower;
			}
			return n;
		}

		std::string get_state(const std::vector<complex_t>& state)
		{
			size_t i = 0;
			std::vector<char> buf;
			double norm = 0.0;

			fmt::format_to(back_inserter(buf), "-----------------------\n");
			for (auto& term : state)
			{
				fmt::format_to(back_inserter(buf), "{:<5d}{}\n", i, state[i]);
				norm += abs_sqr(state[i]);
				++i;
			}
			fmt::format_to(back_inserter(buf), "Factor = {}\n", norm);
			fmt::format_to(back_inserter(buf), "-----------------------\n");
			return { buf.data(), buf.size() };
		}

		std::string get_state(const std::vector<complex_t>& state, size_t pos)
		{
			std::vector<char> buf;
			fmt::format_to(back_inserter(buf), "-----------------------\n");
			fmt::format_to(back_inserter(buf), "{:<5d}{}\n", pos, state[pos]);
			fmt::format_to(back_inserter(buf), "-----------------------\n");
			return { buf.data(), buf.size() };
		}

		void init_n_state(std::vector<complex_t>& state, size_t n)
		{
			state.resize(pow2(n), 0);
			std::fill(state.begin() + 1, state.end(), 0);
			state[0] = 1;
		}

		void hadamard(complex_t& a, complex_t& b)
		{
			complex_t m0 = (a + b) / sqrt(2);
			complex_t m1 = (a - b) / sqrt(2);
			a = m0;
			b = m1;
		}

		void not_gate(complex_t& a, complex_t& b)
		{
			std::swap(a, b);
		}

		size_t measure(const std::vector<complex_t>& state)
		{
			double accum_p = 0;
			static std::uniform_real_distribution<double> ud;
			double r = ud(random_engine::get_engine());
			for (size_t i = 0; i < state.size(); ++i)
			{
				accum_p += abs_sqr(state[i]);
				if (accum_p > r) { return i; }
			}
			return state.size();
		}

		MeasureResult measure(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n)
		{
			MeasureResult mr;
			double accum_p = 0;
			static std::uniform_real_distribution<double> ud(0, 1);
			double r = ud(random_engine::get_engine());
			size_t i = 0;
			size_t meas_res = 0, meas_mask = 0;
			for (; i < state.size(); ++i)
			{
				accum_p += abs_sqr(state[i]);
				if (accum_p > r) {
					meas_mask = mask(qn) & i;
					meas_res = extract_binary(i, qn);
				}
			}
			mr.measure_mask = meas_mask;
			mr.measure_result = meas_res;
			accum_p = 0;
			for (size_t i = 0; i < state.size(); ++i)
			{
				if ((mask(qn) & i) == meas_mask)
				{
					accum_p += abs_sqr(state[i]);
				}
				else {
					state[i] = 0;
				}
			}
			mr.probability = accum_p;
			accum_p = 1.0 / sqrt(accum_p);
			for (size_t i = 0; i < state.size(); ++i)
			{
				state[i] *= accum_p;
			}
			return mr;
		}

		std::map<size_t, complex_t> get_max_state(const std::vector<complex_t>& state, size_t n_max)
		{
			return get_max_elements<complex_t>(state, n_max);
		}

		std::string max_state2str(const std::vector<complex_t>& state, size_t n_max)
		{
			std::vector<char> buf;
			auto&& maximums = get_max_state(state, n_max);
			for (auto&& [idx, state] : maximums)
			{
				fmt::format_to(back_inserter(buf), "{:<5d}{}\n", idx, state);
			}
			return { buf.data(), buf.size() };
		}
	}
}