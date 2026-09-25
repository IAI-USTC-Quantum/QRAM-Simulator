#pragma once

/**
 * @file simple_quantum_simulator.h
 * @brief Simple full-amplitude quantum circuit simulator (state vector method) with bit-order utilities.
 *
 * Provides primitives acting on the state vector, such as single-qubit gates
 * (unitary1q), Hadamard, measurement, and bit-order extraction /
 * reconstruction (extract_binary / reconstruct_binary); mainly serves the
 * full-amplitude bridging of QRAMFullAmp and small-scale verification experiments.
 */

#include "basic.h"

namespace qram_simulator {
	namespace quantum_simulator
	{
		/**
		 * @brief Extract bits from index according to the digits list.
		 *
		 * Example: index=10101 (binary), digits=[1,3] -> output 111
		 * (digits[0] corresponds to the least significant output bit).
		 * @param index Input integer
		 * @param digits List of bit indices to extract
		 * @return Integer assembled from the extracted bits
		 */
		size_t extract_binary(size_t index, const std::vector<size_t>& digits);

		/**
		 * @brief Mask of a digits list.
		 *
		 * Example: digits=[1,3] -> output 1010 (binary).
		 */
		size_t mask(const std::vector<size_t>& digits);

		/**
		 * @brief Inverse of extract_binary: spreads the bits of value according to digits.
		 *
		 * Example: index=111, digits=[1,3] -> output 10101.
		 */
		size_t reconstruct_binary(size_t index, const std::vector<size_t>& digits);

		/**
		 * @brief Clear the bits of index corresponding to mask_digits.
		 *
		 * Example: index=11111, mask=[1,3] -> output 10101.
		 */
		size_t mask_remain(size_t index, const std::vector<size_t>& mask);

		/**
		 * @brief Replace the bits of index corresponding to mask with new_value.
		 *
		 * Equivalent to (index & ~mask) + reconstruct_binary(new_value, mask).
		 */
		size_t mask_replace(size_t index, const std::vector<size_t>& mask, size_t new_value);

		/// Write the bits of number into remain according to digits
		void insert_binary(size_t& remain, const std::vector<size_t>& digits, size_t number);
		/// Get the list of spectator qubits other than the address / data qubits when the total qubit count is sz
		std::vector<size_t> _get_remain_qubits(
			size_t sz,
			std::vector<size_t> address_qubits,
			const std::vector<size_t>& data_qubits);

		/// Discard the bits of index corresponding to digits (compacting the remaining bits)
		size_t discard(size_t n, std::vector<size_t> digits);
		/// Discard the specified bits according to mask_target on the state vector (dimension reduction)
		void discard(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n, size_t mask_target);

		/// Return a human-readable string of the state vector
		std::string get_state(const std::vector<complex_t>& state);
		/// Return a human-readable string of the state vector around position pos
		std::string get_state(const std::vector<complex_t>& state, size_t pos);

		/// Initialize state to the n-qubit |0...0> (length 2^n, first element 1)
		void init_n_state(std::vector<complex_t>& state, size_t n);

		/// Sample one basis vector from the state vector according to the Born rule (global random engine)
		size_t measure(const std::vector<complex_t>& state);

		/**
		 * @brief Measurement result over a specified set of bits (collapse + normalization).
		 */
		struct MeasureResult
		{
			/// Measured classical value
			size_t measure_result;
			/// Mask of the measured bits
			size_t measure_mask;
			/// Born probability of this outcome
			double probability;
		};

		/**
		 * @brief Measure the n bits listed in qn on the state vector.
		 *
		 * Samples one outcome, collapses the state to that branch, and renormalizes.
		 * @param state State vector (modified in place)
		 * @param qn List of indices of the measured bits
		 * @param n Number of bits (length of qn)
		 * @return Measurement result (classical value / mask / probability)
		 */
		MeasureResult measure(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n);

		/**
		 * @brief Apply the single-qubit unitary gate func to bit digit.
		 *
		 * func receives a pair of references to the |0> / |1> amplitudes of that bit and updates them in place.
		 * @param state State vector
		 * @param digit Index of the target bit
		 * @param func Binary callable (gate application function)
		 */
		template<typename Fn>
		void unitary1q(std::vector<complex_t>& state, size_t digit, Fn func)
		{
			for (size_t i = 0; i < state.size(); ++i)
			{
				if (digit1(i, digit)) continue;
				func(state[i], state[i + pow2(digit)]);
			}
		}

		/// Hadamard gate: (a,b) <- ((a+b)/sqrt(2), (a-b)/sqrt(2))
		void hadamard(complex_t& a, complex_t& b);
		/// NOT gate (X gate): (a,b) <- (b,a)
		void not_gate(complex_t& a, complex_t& b);

		/// Apply Hadamard to each of the first n bits
		inline void hadamard_all(std::vector<complex_t>& state, size_t n)
		{
			for (size_t i = 0; i < n; ++i) {
				unitary1q(state, i, hadamard);
			}
		}

		/**
		 * @brief Take the largest n_max elements of the state vector (map kept ordered).
		 * @param state Input sequence
		 * @param n_max Number of elements to keep
		 * @return Map of {index: element}
		 */
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

		/// Take the n_max basis vectors with the largest amplitudes in the state vector (compared by squared modulus)
		std::map<size_t, complex_t> get_max_state(const std::vector<complex_t>& state, size_t n_max);
		/// Human-readable string of the largest n_max basis vectors
		std::string max_state2str(const std::vector<complex_t>& state, size_t n_max);

		/// Print the state vector (also performs a normalization check when check is true)
		inline void print_state(const std::vector<complex_t>& state, bool check = true)
		{
			fmt::print(get_state(state));
			if (check) {
				if (std::abs(amp_sum(state) - 1.0) > epsilon)
					throw_bad_result();
			}
		}

		/// Print the content of the state vector around position pos
		inline void print_state(const std::vector<complex_t>& state, size_t pos)
		{
			fmt::print(get_state(state, pos));
		}

		/// Print the largest n_max basis vectors
		inline void print_max_state(const std::vector<complex_t>& state, size_t n_max)
		{
			fmt::print(max_state2str(state, n_max));
		}
	} // namespace quantum_simulator
} // namespace qram_simulator
