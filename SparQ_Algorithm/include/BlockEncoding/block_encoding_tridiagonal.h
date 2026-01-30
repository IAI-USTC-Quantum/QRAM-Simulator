#pragma once
#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include <iostream>
#include <fstream>
#include "matrix.h"
#include <Eigen/Eigen>
#include <filesystem>


namespace qram_simulator {
	namespace block_encoding {
		namespace block_encoding_tridiagonal
		{
			struct PlusOneAndOverflow : BaseOperator
			{
				using BaseOperator::operator();
				using BaseOperator::dag;

				ClassControllable
				std::string main_reg;
				std::string overflow;
				PlusOneAndOverflow(std::string_view main_reg_, std::string_view overflow_) :
					main_reg(main_reg_), overflow(overflow_) {}
				void operator()(std::vector<System>& state) const;
				void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
				void operator()(CuSparseState& s) const;
				void dag(CuSparseState& s) const;
#endif
			};

			struct Block_Encoding_Tridiagonal : BaseOperator
			{
				double alpha;
				double beta;
				std::string main_reg;
				std::string anc_UA;
				// std::vector<complex_t> matrix_elements;
				//DenseMatrix<complex_t> mat;
				std::vector<complex_t> prep_state;
				ClassControllable

				Block_Encoding_Tridiagonal(
					std::string_view main_reg_,
					std::string_view anc_UA_,
					double alpha_,
					double beta_);

				template<typename Ty>
				void impl(Ty& state) const
				{
					profiler _("Block_Encoding_Tridiagonal");
					SPLIT_BY_CONDITIONS
					{
						auto overflow = SplitRegister(anc_UA, "overflow", 1)(state);
						auto other = SplitRegister(anc_UA, "other", 1)(state);
						Rot_GeneralStatePrep stateprep(anc_UA, prep_state);
						stateprep(state);

						(PlusOneAndOverflow(main_reg, "overflow")).conditioned_by_value(anc_UA, 1)(state);
						if (beta < 0)
						{
							Reflection_Bool({ main_reg, "overflow" }, false).conditioned_by_value(anc_UA, 1)(state);
							Reflection_Bool({ main_reg, "overflow" }, false).conditioned_by_value(anc_UA, 2)(state);
						}
						(PlusOneAndOverflow(main_reg, "overflow")).conditioned_by_value(anc_UA, 2).dag(state);

						Xgate_Bool("other", 0).conditioned_by_all_ones(anc_UA)(state);

						stateprep.dag(state);
						CombineRegister(anc_UA, "other")(state);
						CombineRegister(anc_UA, "overflow")(state);
					}
					MERGE_BY_CONDITIONS
				}

				template<typename Ty>
				void impl_dag(Ty& state) const
				{
					profiler _("Block_Encoding_Tridiagonal");

					SPLIT_BY_CONDITIONS
					{
						auto overflow = SplitRegister(anc_UA, "overflow", 1)(state);
						auto other = SplitRegister(anc_UA, "other", 1)(state);
						Rot_GeneralStatePrep stateprep(anc_UA, prep_state);
						stateprep(state);

						(PlusOneAndOverflow(main_reg, "overflow")).conditioned_by_value(anc_UA, 2)(state);
						if (beta < 0)
						{
							Reflection_Bool({ main_reg, "overflow" }, false).conditioned_by_value(anc_UA, 2)(state);
							Reflection_Bool({ main_reg, "overflow" }, false).conditioned_by_value(anc_UA, 1)(state);
						}
						(PlusOneAndOverflow(main_reg, "overflow")).conditioned_by_value(anc_UA, 1).dag(state);

						Xgate_Bool("other", 0).conditioned_by_all_ones(anc_UA)(state);

						stateprep.dag(state);
						CombineRegister(anc_UA, "other")(state);
						CombineRegister(anc_UA, "overflow")(state);
					}
					MERGE_BY_CONDITIONS
				}

				COMPOSITE_OPERATION
			};

			inline DenseMatrix<double> get_block_encoding_tridiagonal(size_t qubit_num, double alpha, double beta)
			{
				System::add_register("main_reg", UnsignedInteger, qubit_num);
				System::add_register("anc_UA", UnsignedInteger, 4);
				Block_Encoding_Tridiagonal block_enc("main_reg", "anc_UA", alpha, beta);

				DenseMatrix<complex_t> mat = extract_block_encoding(block_enc, "main_reg", "anc_UA", qubit_num);
				DenseMatrix<double> ret(mat.size);
				for (int i = 0; i < pow2(qubit_num); ++i)
				{
					for (int j = 0; j < pow2(qubit_num); ++j)
					{
						ret(i, j) = mat(i, j).real();
					}
				}

				System::clear();
				return ret;
			}

			inline DenseMatrix<double> get_tridiagonal_matrix(double alpha, double beta, size_t dim)
			{
				DenseMatrix<double> mat(dim);
				for (size_t i = 0; i < dim; ++i)
				{
					mat(i, i) = alpha;
					if (i > 0)
						mat(i - 1, i) = beta;
					if (i < (dim - 1))
						mat(i + 1, i) = beta;
				}
				/*fmt::print("{}", mat.to_string());*/
				return mat;
			}

			template<typename Ty>
			DenseMatrix<Ty> Get_U_plus(size_t size)
			{
				DenseMatrix<Ty> U_plus(size);
				for (size_t i = 1; i < size; ++i)
				{
					U_plus(i, i - 1) = 1;
				}
				return U_plus;
			}

			template<typename Ty>
			DenseMatrix<Ty> Get_U_minus(size_t size)
			{
				DenseMatrix<Ty> U_minus(size);
				for (size_t i = 0; i < size - 1; ++i)
				{
					U_minus(i, i + 1) = 1;
				}
				return U_minus;
			}

		}
	}
}