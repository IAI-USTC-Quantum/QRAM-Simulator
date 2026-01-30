#pragma once
#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include "matrix.h"
#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "qda_fundamental.h"
#include <Eigen/Eigen>

namespace qram_simulator {
	using namespace block_encoding::block_encoding_tridiagonal;

	namespace QDA {
		namespace QDA_tridiagonal {

			struct Walk_s_Tridiagonal : Walk_s<Block_Encoding_Tridiagonal, Hadamard_Int_Full>
			{
				using EncA = Block_Encoding_Tridiagonal;
				using Encb = Hadamard_Int_Full;

				Walk_s_Tridiagonal(
					std::string_view main_reg_,
					std::string_view anc_UA_,
					std::string_view anc_1_,
					std::string_view anc_2_,
					std::string_view anc_3_,
					std::string_view anc_4_,
					double s_,
					double kappa_,
					double p_,
					double alpha_,
					double beta_
				) :
					Walk_s(
						Block_Encoding_Tridiagonal(main_reg_, anc_UA_, alpha_, beta_),
						Hadamard_Int_Full(main_reg_),
						main_reg_, anc_UA_, anc_1_, anc_2_, anc_3_, anc_4_,
						s_, kappa_, p_)
				{}
			};

			struct Walk_s_Tridiagonal_Debug : public Walk_s_Tridiagonal, QDADebugger
			{
				// size_t row_size;

				Walk_s_Tridiagonal_Debug(
					const DenseMatrix<double>& matrix,
					const DenseVector<double>& vec,
					std::string_view main_reg_,
					std::string_view anc_UA_,
					std::string_view anc_1_,
					std::string_view anc_2_,
					std::string_view anc_3_,
					std::string_view anc_4_,
					double s_,
					double kappa_,
					double p_,
					double alpha_,
					double beta_) :
					Walk_s_Tridiagonal(main_reg_, anc_UA_,
						anc_1_, anc_2_, anc_3_, anc_4_,
						s_, kappa_, p_, alpha_, beta_),
					QDADebugger(
						matrix, vec,
						s_, kappa_, p_)
				{
				};
			};
		}

	}
}