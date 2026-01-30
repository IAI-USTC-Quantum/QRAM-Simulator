#pragma once
#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include "matrix.h"
#include "BlockEncoding/block_encoding_via_QRAM.h"
#include "state_preparation.h"
#include "qda_fundamental.h"
#include <Eigen/Eigen>

namespace qram_simulator {
	namespace QDA {
		namespace QDA_via_QRAM {
			using namespace block_encoding::block_encoding_via_QRAM;
			using namespace state_prep;

			template<typename Encb_type>
			struct Walk_s_via_QRAM_A : Walk_s<Block_Encoding_via_QRAM, Encb_type>
			{
				qram_qutrit::QRAMCircuit* qram_A;
				size_t data_size;
				size_t rational_size;
				using EncA = Block_Encoding_via_QRAM;
				using Encb = Encb_type;

				Walk_s_via_QRAM_A(
					qram_qutrit::QRAMCircuit* qram_A_,
					Encb_type encb_,
					std::string_view main_reg_,
					std::string_view anc_UA_,
					std::string_view anc_1_,
					std::string_view anc_2_,
					std::string_view anc_3_,
					std::string_view anc_4_,
					double s_,
					double kappa_,
					double p_,
					size_t dsz,
					size_t rsz
				) :
					qram_A(qram_A_), data_size(dsz), rational_size(rsz),
					Walk_s<Block_Encoding_via_QRAM, Encb_type>(
						Block_Encoding_via_QRAM(qram_A_, main_reg_, anc_UA_, dsz, rsz),
						encb_,
						main_reg_, anc_UA_, anc_1_, anc_2_, anc_3_, anc_4_,
						s_, kappa_, p_)
				{
				}
			
			};
			
			struct Walk_s_via_QRAM : Walk_s<Block_Encoding_via_QRAM, State_Prep_via_QRAM>
			{
				qram_qutrit::QRAMCircuit* qram_A;
				qram_qutrit::QRAMCircuit* qram_b;
				size_t data_size;
				size_t rational_size;

				using EncA = Block_Encoding_via_QRAM;
				using Encb = State_Prep_via_QRAM;

				Walk_s_via_QRAM(
					qram_qutrit::QRAMCircuit* qram_A_,
					qram_qutrit::QRAMCircuit* qram_b_,
					std::string_view main_reg_,
					std::string_view anc_UA_,
					std::string_view anc_1_,
					std::string_view anc_2_,
					std::string_view anc_3_,
					std::string_view anc_4_,
					double s_,
					double kappa_,
					double p_,
					size_t dsz,
					size_t rsz
				) :
					qram_A(qram_A_), qram_b(qram_b_), data_size(dsz), rational_size(rsz),
					Walk_s(
						Block_Encoding_via_QRAM(qram_A_, main_reg_, anc_UA_, dsz, rsz),
						State_Prep_via_QRAM(qram_b_, main_reg_, dsz, rsz),
						main_reg_, anc_UA_, anc_1_, anc_2_, anc_3_, anc_4_,
						s_, kappa_, p_)
				{}
			};


			struct Walk_s_via_QRAM_Debug : public Walk_s_via_QRAM, QDADebugger
			{
				Walk_s_via_QRAM_Debug(qram_qutrit::QRAMCircuit* qram_A_,
					qram_qutrit::QRAMCircuit* qram_b_,
					const DenseMatrix<double>& matrix_A_,
					const DenseVector<double>& vector_b_,
					std::string main_reg_,
					std::string anc_UA_,
					std::string anc_1_,
					std::string anc_2_,
					std::string anc_3_,
					std::string anc_4_,
					double s_,
					double kappa_,
					double p_,
					bool is_PD,
					size_t dsz,
					size_t rsz) :
					Walk_s_via_QRAM(qram_A_, qram_b_, main_reg_, anc_UA_,
						anc_1_, anc_2_, anc_3_, anc_4_,
						s_, kappa_, p_, dsz, rsz),
					QDADebugger(matrix_A_, vector_b_, s_, kappa_, p_)
				{
				};
			};

			struct WalkSequence_via_QRAM_Debug
			{
				size_t steps;
				double kappa;
				double p;
				std::string main_reg;
				std::string anc_UA;
				std::string anc_1;
				std::string anc_2;
				std::string anc_3;
				std::string anc_4;
				qram_qutrit::QRAMCircuit* qram_A;
				qram_qutrit::QRAMCircuit* qram_b;
				DenseMatrix<double> matrix_A;
				DenseVector<double> vector_b;
				size_t data_size;
				size_t rational_size;
				std::string stdout_filename;

				WalkSequence_via_QRAM_Debug(qram_qutrit::QRAMCircuit* qram_A_,
					qram_qutrit::QRAMCircuit* qram_b_,
					const DenseMatrix<double>& matrix_A,
					const DenseVector<double>& vector_b,
					std::string main_reg_,
					std::string anc_UA_,
					std::string anc_1_,
					std::string anc_2_,
					std::string anc_3_,
					std::string anc_4_,
					size_t steps_,
					double kappa_,
					double p_,
					size_t dsz,
					size_t rsz,
					std::string stdout_filename_) : qram_A(qram_A_), qram_b(qram_b_), matrix_A(matrix_A), vector_b(vector_b),
					main_reg(main_reg_), anc_UA(anc_UA_), anc_1(anc_1_), anc_2(anc_2_), anc_3(anc_3_), anc_4(anc_4_),
					steps(steps_), kappa(kappa_), p(p_), data_size(dsz), rational_size(rsz), stdout_filename(stdout_filename_)
				{
				};

				template<typename Ty>
				void operator()(Ty& state)
				{
					std::regex _pattern("stdout");
					std::string fidelity_filename = std::regex_replace(stdout_filename, _pattern, "fidelity");
					if (stdout_filename == fidelity_filename)
					{
						fmt::print("\nstdout filename `{}` is invalid!\n", stdout_filename);
						std::exit(5);
					}

					fmt::print("\nfilename for fidelity saving: {}\n", fidelity_filename);
					fmt::print("\nfilename for std output: {}\n", stdout_filename);

					{
						std::ofstream f_fidelity(fidelity_filename);
						if (!f_fidelity.is_open()) {
							throw std::runtime_error("Failed to open file.");
						}
					}

					for (size_t n = 0; n < steps; n++)
					{
						double s = double(n) / steps;
						auto walk = Walk_s_via_QRAM_Debug(qram_A, qram_b, matrix_A, vector_b,
							main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4,
							s, kappa, p, false, data_size, rational_size);
						walk(state);
						ClearZero()(state);

						if ((n + 1) % 2 == 0)
						{
							auto mid_state = GetOutput(main_reg, anc_UA, anc_4, anc_3, anc_2, anc_1)(state);
							std::vector<double> ideal_state = walk.get_mid_eigenstate();

							double fidelity = get_fidelity(ideal_state, mid_state.first);


							{// stdout writing
								std::ofstream f_stdout(stdout_filename, std::ios::app);
								if (!f_stdout.is_open()) {
									throw std::runtime_error("Failed to open file.");
								}
								f_stdout << fmt::format("step: {} / {}, fidelity: {}, p_success: {}\n", n, steps, fidelity, mid_state.second);
								f_stdout << fmt::format("Maximum Qubit Count = {}\n", System::max_qubit_count);
								f_stdout << fmt::format("Maximum Register Count = {}\n", System::max_register_count);
								f_stdout << fmt::format("Maximum System Size = {}\n\n", System::max_system_size);
								f_stdout.close();
							}

							{// fidelity writing
								std::ofstream f_fidelity(fidelity_filename, std::ios::app);
								if (!f_fidelity.is_open()) {
									throw std::runtime_error("Failed to open file.");
								}
								f_fidelity << fmt::format("step: {} / {}, fidelity: {}, p_success: {}, max_system_size: {}\n",
									n, steps, fidelity, mid_state.second, System::max_system_size);
								f_fidelity.close();
							}
						}
						auto now = std::chrono::system_clock::now();
						fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
					}
				}

				template<typename Ty>
				void dag(Ty& state)
				{
					for (size_t n = 0; n < steps; n++) {
						if ((n + 1) % 10 == 0) fmt::print("n: {:>5}\n", n);
						double s = double(steps - n - 1) / steps;
						auto walk = Walk_s_via_QRAM_Debug(qram_A, qram_b, matrix_A, vector_b,
							main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4,
							s, kappa, p, data_size, rational_size);
						walk.dag(state);

						ClearZero()(state);
					}
				}
			};

		} // namespace QDA_via_QRAM
	} // namespace QDA
}