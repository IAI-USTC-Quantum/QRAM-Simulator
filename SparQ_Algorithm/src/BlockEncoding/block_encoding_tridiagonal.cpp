#include "BlockEncoding/block_encoding_tridiagonal.h"

namespace qram_simulator {
	namespace block_encoding {
		namespace block_encoding_tridiagonal {
			void PlusOneAndOverflow::operator()(std::vector<System>& state) const {
				profiler _("Quantum_Modulo_Adder");
				auto main_reg_idx = System::get(main_reg);
				auto overflow_reg_idx = System::get(overflow);
				auto main_reg_num = System::size_of(main_reg);
#ifdef SINGLE_THREAD
				for (auto& s : state)
				{
#else
#pragma omp parallel for
				for (int i = 0; i < state.size(); ++i)
				{
					auto& s = state[i];
#endif
					if (!ConditionSatisfied(s))
						continue;
					auto& val = s.get(main_reg_idx).value;
					auto& overflow_val = s.get(overflow_reg_idx).value;
					if (val == pow2(main_reg_num) - 1)
					{
						overflow_val = (overflow_val + 1) % 2;
						val = 0;
					}
					else {
						val += 1;
					}
				}
			}

			void PlusOneAndOverflow::dag(std::vector<System>& state) const {
				profiler _("Quantum_Modulo_Adder");
				auto main_reg_idx = System::get(main_reg);
				auto overflow_reg_idx = System::get(overflow);
				auto main_reg_num = System::size_of(main_reg);
#ifdef SINGLE_THREAD
				for (auto& s : state)
				{
#else
#pragma omp parallel for
				for (int i = 0; i < state.size(); ++i)
				{
					auto& s = state[i];
#endif
					if (!ConditionSatisfied(s))
						continue;
					
					auto& val = s.get(main_reg_idx).value;
					auto& overflow_val = s.get(overflow_reg_idx).value;
					if (val == 0)
					{
						overflow_val = (overflow_val + 1) % 2;
						val = pow2(main_reg_num) - 1;
					}
					else {
						val -= 1;
					}

				}
			}

			Block_Encoding_Tridiagonal::Block_Encoding_Tridiagonal(
				std::string_view main_reg_,
				std::string_view anc_UA_,
				double alpha_,
				double beta_) :				
				main_reg(main_reg_), anc_UA(anc_UA_), alpha(alpha_), beta(beta_)
			{
				auto n = pow2(System::size_of(main_reg));
				auto sum = n * std::abs(alpha_) * std::abs(alpha_) + 2 * (n - 1) * std::abs(beta_) * std::abs(beta_);
				auto norm_F = std::sqrt(sum);

				//mat = DenseMatrix<complex_t>(4);
				//mat(0, 0) = std::sqrt(std::abs(alpha_)) / std::sqrt(norm_F);
				//mat(1, 0) = std::sqrt(std::abs(beta_)) / std::sqrt(norm_F);
				//mat(2, 0) = std::sqrt(std::abs(beta_)) / std::sqrt(norm_F);
				//mat(3, 0) = std::sqrt(1 - (std::abs(alpha_) + 2 * std::abs(beta_)) / norm_F);
				//mat(1, 1) = 1;
				//mat(2, 2) = 1;
				//mat(3, 3) = 1;
				//gram_schmidt_process(mat);

				prep_state = std::vector<complex_t>{
					std::sqrt(std::abs(alpha_)) / std::sqrt(norm_F),
					std::sqrt(std::abs(beta_)) / std::sqrt(norm_F),
					std::sqrt(std::abs(beta_)) / std::sqrt(norm_F),
					std::sqrt(1 - (std::abs(alpha_) + 2 * std::abs(beta_)) / norm_F)
				};
			};

			


		} // namespace block_encoding_tridiagonal
	} // namespace block_encoding
}