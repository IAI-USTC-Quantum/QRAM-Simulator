#pragma once
#include "quantum_interfere_basic.h"

namespace qram_simulator {

	struct QFT : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		// std::string register_name;
		size_t id;
		size_t n_digits;
		complex_t omega;
		ClassControllable
		QFT(std::string_view reg_ins);
		QFT(size_t reg_in);

		/*
		about hadamard matrix

		< x | H | y >
		 H = exp(2 * pi * i * x * y / 2^n) / 2^n
		else H = 1
		*/

		/* The utility function to compute conditional rotation. */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}
		void operate(size_t l, size_t r, std::vector<System>& state) const;
		void operator()(std::vector<System>& state) const;
	};

	struct inverseQFT : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		// std::string register_name;
		size_t id;
		size_t n_digits;
		complex_t omega;
		ClassControllable
		inverseQFT(std::string_view reg_ins);
		inverseQFT(size_t reg_in);

		/*
		about hadamard matrix

		< x | H | y >
		 H = exp(2 * pi * i * x * y / 2^n) / 2^n
		else H = 1
		*/

		/* The utility function to compute conditional rotation. */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}
		void operate(size_t l, size_t r, std::vector<System>& state) const;
		void operator()(std::vector<System>& state) const;
	};

	struct QFT_Full : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t id;
		size_t n_digits;
		complex_t omega;
		const size_t few_threshold = n_digits - 1;
		size_t full_size;
		double extra_amplitude;
		std::vector<uint64_t> bitrev;

		ClassControllable

		inline void precompute_bitrev() {
			profiler _("prefcompute_bitrev");
			if (bitrev.empty())
			{
				bitrev.resize(full_size);
#ifndef SINGLE_THREAD
#pragma omp parallel for schedule(static)
#endif
				for (int64_t i = 0; i < full_size; ++i) {
					uint64_t x = i;
					uint64_t rev = 0;
					for (size_t j = 0; j < n_digits; ++j) {
						rev = (rev << 1) | (x & 1);
						x >>= 1;
					}
					bitrev[i] = rev;
				}
			}
		}

		QFT_Full(size_t reg_in)
			: id(reg_in), n_digits(System::size_of(reg_in)), 
			full_size(pow2(n_digits)), extra_amplitude(1.0 / std::sqrt(full_size))
		{
			double theta = 2 * pi / pow2(n_digits);
			omega = complex_t{ cos(theta), sin(theta) };
			precompute_bitrev();
		}
		QFT_Full(std::string_view reg_in)
			: id(System::get(reg_in)), n_digits(System::size_of(id)),
			full_size(pow2(n_digits)), extra_amplitude(1.0 / std::sqrt(full_size))
		{
			double theta = 2 * pi / pow2(n_digits);
			omega = complex_t{ cos(theta), sin(theta) };
			precompute_bitrev();
		}

		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}
		void operate_bucket_sparse(const std::vector<size_t>& positions, std::vector<System>& state) const;
		void fft(const std::vector<size_t>& positions, std::vector<System>& state, bool inverse) const;
		void operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state) const;
		void operate_bucket_inplace_inv(const std::vector<size_t>& positions, std::vector<System>& state) const;
		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;

	};
}