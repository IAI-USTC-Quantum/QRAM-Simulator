#pragma once

/* A simple quantum circuit simulator (full-amplitude) 
* 
*/

#include "basic.h"

namespace qram_simulator {
	namespace quantum_simulator
	{
		/* index = 10101 digit = [1,3] */
		/* out = 111 */
		size_t extract_binary(size_t index, const std::vector<size_t>& digits);

		/* digit = [1,3] */
		/* out = 1010 */
		size_t mask(const std::vector<size_t>& digits);

		/* reverse computation of extract_binary */
		/* index = 111 digit = [1,3] */
		/* out = 10101 */
		size_t reconstruct_binary(size_t index, const std::vector<size_t>& digits);

		/* index = 11111 mask = [1,3] */
		/* out = 10101 */
		/* mask_remain = index & ~mask */
		size_t mask_remain(size_t index, const std::vector<size_t>& mask);

		/* use new_value and mask to reconstruct binary */
		/* then replace the digits in index with such binary */
		/* mask_replace = index & ~mask + reconstruct_binary(new_value, index)*/
		size_t mask_replace(size_t index, const std::vector<size_t>& mask, size_t new_value);

		void insert_binary(size_t& remain, const std::vector<size_t>& digits, size_t number);
		std::vector<size_t> _get_remain_qubits(
			size_t sz,
			std::vector<size_t> address_qubits,
			const std::vector<size_t>& data_qubits);

		size_t discard(size_t n, std::vector<size_t> digits);
		void discard(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n, size_t mask_target);

		std::string get_state(const std::vector<complex_t>& state);
		std::string get_state(const std::vector<complex_t>& state, size_t pos);

		void init_n_state(std::vector<complex_t>& state, size_t n);

		size_t measure(const std::vector<complex_t>& state);

		struct MeasureResult
		{
			size_t measure_result;
			size_t measure_mask;
			double probability;
		};

		MeasureResult measure(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n);

		template<typename Fn>
		void unitary1q(std::vector<complex_t>& state, size_t digit, Fn func)
		{
			for (size_t i = 0; i < state.size(); ++i)
			{
				if (digit1(i, digit)) continue;
				func(state[i], state[i + pow2(digit)]);
			}
		}

		void hadamard(complex_t& a, complex_t& b);
		void not_gate(complex_t& a, complex_t& b);

		inline void hadamard_all(std::vector<complex_t>& state, size_t n)
		{
			for (size_t i = 0; i < n; ++i) {
				unitary1q(state, i, hadamard);
			}
		}

		template<typename Ty>
		std::map<size_t, Ty> get_max_elements(const std::vector<Ty>& state, size_t n_max)
		{
			std::map<size_t, Ty> maximums;
			for (size_t i = 0; i < std::size(state); ++i)
			{
				if (maximums.size() < n_max) {
					maximums[i] = state[i];
					continue;
				}
				auto&& mini = maximums.begin()->second;
				if (mini < state[i])
				{
					maximums.erase(maximums.begin());
					maximums[i] = state[i];
				}
			}
			return maximums;
		}

		std::map<size_t, complex_t> get_max_state(const std::vector<complex_t>& state, size_t n_max);
		std::string max_state2str(const std::vector<complex_t>& state, size_t n_max);

		inline void print_state(const std::vector<complex_t>& state, bool check = true)
		{
			fmt::print(get_state(state));
			if (check) {
				if (std::abs(amp_sum(state) - 1.0) > epsilon)
					throw_bad_result();
			}
		}

		inline void print_state(const std::vector<complex_t>& state, size_t pos)
		{
			fmt::print(get_state(state, pos));
		}

		inline void print_max_state(const std::vector<complex_t>& state, size_t n_max)
		{
			fmt::print(max_state2str(state, n_max));
		}
	} // namespace quantum_simulator
} // namespace qram_simulator