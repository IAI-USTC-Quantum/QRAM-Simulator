#pragma once
#include "quantum_interfere_basic.h"
#include "matrix.h"

namespace qram_simulator
{
	struct Rot_GeneralUnitary : BaseOperator
	{
		using BaseOperator::operator();
		using BaseOperator::dag;

		using unitary_t = DenseMatrix<complex_t>;

		unitary_t mat;
		size_t id;
		size_t n_digits;
		size_t full_size;
		ClassControllable
		Rot_GeneralUnitary(std::string_view reg_in, const unitary_t &mat_)
			: id(System::get(reg_in)), mat(mat_)
		{
			n_digits = System::size_of(id);
			full_size = pow2(n_digits);

			if (full_size != mat_.size)
				throw_invalid_input("Matrix size does not match the register's size.");
		}
		Rot_GeneralUnitary(size_t reg_in, const unitary_t &mat_)
			: id(reg_in), mat(mat_)
		{
			n_digits = System::size_of(id);
			full_size = pow2(n_digits);

			if (full_size != mat_.size)
				throw_invalid_input("Matrix size does not match the register's size.");
		}

		void operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state, bool dagger) const;
		void operate(std::vector<System>& state, bool dagger) const;
		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		virtual void operator()(CuSparseState& state) const;
		virtual void dag(CuSparseState& state) const;
#endif
	};

	DenseMatrix<complex_t> stateprep_unitary_build_schmidt(const std::vector<complex_t>& vec);

	struct Rot_GeneralStatePrep : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		using unitary_t = DenseMatrix<complex_t>;
		std::vector<complex_t> vec;
		size_t id;
		size_t n_digits;
		size_t full_size;
		Rot_GeneralUnitary rot_general;

		ClassControllable

		Rot_GeneralStatePrep(std::string_view reg_in, const std::vector<complex_t> &vec);
		Rot_GeneralStatePrep(size_t reg_in, const std::vector<complex_t> &vec);

		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		virtual void operator()(CuSparseState& state) const;
		virtual void dag(CuSparseState& state) const;
#endif
	};
}

