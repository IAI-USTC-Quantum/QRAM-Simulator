#pragma once
#include "quantum_interfere_basic.h"


namespace qram_simulator
{
	struct Hadamard_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		// std::string register_name;
		size_t id;
		size_t n_digits;
		uint64_t mask;
		ClassControllable
		Hadamard_Int(std::string_view reg_in, size_t n_digits_)
			: Hadamard_Int(System::get(reg_in), n_digits_)
		{
		}
		Hadamard_Int(size_t reg_in, size_t n_digits_)
		{
			id = reg_in;
			n_digits = n_digits_;
			mask = pow2(n_digits);
			mask--;
			mask = ~mask;
		}

		/*
		about hadamard matrix

		< x | H | y >
		if ( x[i] & y[i] ) H = -1
		else H = 1
		*/

		/* The utility function to compute conditional rotation. */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}
		void operate(size_t l, size_t r, std::vector<System>& state) const;
		void operator()(std::vector<System>& state) const; 
#ifdef USE_CUDA
		void operator()(CuSparseState& s) const;
#endif
	};

	struct Hadamard_Int_Full : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id;
		size_t n_digits;
		const size_t few_threshold = n_digits - 1;
		size_t full_size;
		ClassControllable

		Hadamard_Int_Full(std::string_view reg_in)
			: id(System::get(reg_in)), n_digits(System::size_of(reg_in)),
			full_size(pow2(n_digits)) {}

		Hadamard_Int_Full(size_t reg_in)
			: id(reg_in), n_digits(System::size_of(reg_in)),
			full_size(pow2(n_digits)) {}

		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}
		void operator()(std::vector<System>& state) const;
		void operate_bucket_sparse(const std::vector<size_t>& positions, std::vector<System>& state) const;
		void operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& s) const;
#endif
	};

	/* Only accept the case when input qubit is only one */
	struct Hadamard_Bool : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		// std::string register_name;
		size_t out_id;
		ClassControllable
		Hadamard_Bool(std::string_view reg_in)
			: out_id(System::get(reg_in))
		{
			// size must be 1
			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
		}
		Hadamard_Bool(size_t reg_in)
			: out_id(reg_in)
		{
			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
		}

		/* V2 */
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const;
		void operate_alone_zero(size_t zero, std::vector<System>& state) const;
		void operate_alone_one(size_t one, std::vector<System>& state) const;

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& s) const;
#endif
	};

	struct Hadamard_PartialQubit : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		// std::string register_name;
		size_t id;
		size_t mask;
		std::set<size_t> qubit_positions;
		ClassControllable
		Hadamard_PartialQubit(std::string_view reg_in, std::set<size_t>& qubit_positions_)
			: id(System::get(reg_in)), qubit_positions(qubit_positions_)
		{
			mask = make_mask(qubit_positions_);
		}
		Hadamard_PartialQubit(size_t reg_in, std::set<size_t>& qubit_positions_)
			: id(reg_in), qubit_positions(qubit_positions_)
		{
			mask = make_mask(qubit_positions_);
		}

		/*
		about hadamard matrix

		< x | H | y >
		if ( x[i] & y[i] ) H = -1
		else H = 1
		*/

		/* The utility function to compute conditional rotation. */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const;
		void operate_alone_zero(size_t zero, std::vector<System>& state) const;
		void operate_alone_one(size_t one, std::vector<System>& state) const;

		void operate(size_t l, size_t r, std::vector<System>& state) const;
		void operator()(std::vector<System>& state) const;

	};

}