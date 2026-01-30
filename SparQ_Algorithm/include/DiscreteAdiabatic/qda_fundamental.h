#pragma once
#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include "matrix.h"
#include "block_encoding.h"
#include "state_preparation.h"
#ifdef USE_CUDA
#include "cuda/cuda_utils.cuh"
#endif

/*****************************************************
				Quantum Walk Process
/*****************************************************/
namespace qram_simulator {
	namespace QDA {
		/*
		block-encoding of H(s)
		H(s)=(1-f(s))H_0 + f(s)H_1
		reference: Optimal scaling quantum linear-systems solver via discrete adiabatic theorem[J]. PRX quantum, 2022, 3(4): 040303.
		doi: https://journals.aps.org/prxquantum/abstract/10.1103/PRXQuantum.3.040303
		Construction details: Appendix F
		*/
		template<typename Block_Encoding, typename State_Prep>
		struct Block_Encoding_Hs
		{
			double fs;
			u22_t R_s;
			std::string main_reg;
			std::string anc_UA;
			std::string anc_1;
			std::string anc_2;
			std::string anc_3;
			std::string anc_4;
			// size_t data_size;
			// size_t rational_size;
			Block_Encoding enc_A;
			State_Prep enc_b;
			ClassControllable

			Block_Encoding_Hs(
				Block_Encoding enc_A_,
				State_Prep enc_b_,
				std::string_view main_reg_,
				std::string_view anc_UA_,
				std::string_view anc_1_,
				std::string_view anc_2_,
				std::string_view anc_3_,
				std::string_view anc_4_,
				double fs_) : enc_A(enc_A_), enc_b(enc_b_),
			main_reg(main_reg_), anc_UA(anc_UA_),
			anc_1(anc_1_), anc_2(anc_2_), anc_3(anc_3_), anc_4(anc_4_), fs(fs_)
			// data_size(dsz), rational_size(rsz)
			{
				double sqrt_N = 1.0 / sqrt((1 - fs) * (1 - fs) + fs * fs);
				double u00 = sqrt_N * (1 - fs),
					u01 = sqrt_N * fs,
					u10 = sqrt_N * fs,
					u11 = sqrt_N * (fs - 1);
				R_s = u22_t{ u00, u01, u10, u11 };
			};

			template<typename Ty>
			void impl(Ty& state) const
			{
				profiler _("Block_Encoding_Hs");
				SPLIT_BY_CONDITIONS {
					Block_Encoding enc_A_copy = enc_A;

					(Hadamard_Bool(anc_3))(state);
					enc_b.dag(state);
					Xgate_Bool(anc_1, 0)(state);
					Reflection_Bool(main_reg, true).conditioned_by_all_ones({ anc_1, anc_3, anc_4 })(state);
					Xgate_Bool(anc_1, 0)(state);
					enc_b(state);
					Xgate_Bool(anc_4, 0)(state);
					Rot_Bool(anc_2, R_s).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					Hadamard_Bool(anc_2).conditioned_by_all_ones(anc_4)(state);
					enc_A_copy.conditioned_by_all_ones({ anc_1, anc_2 })(state);
					Xgate_Bool(anc_1, 0).conditioned_by_all_ones(anc_2)(state);
					Reflection_Bool(anc_2, true).conditioned_by_all_ones(anc_1)(state);
					enc_A_copy.conditioned_by_all_ones({ anc_1, anc_2 }).dag(state);
					Xgate_Bool(anc_4, 0)(state);
					Hadamard_Bool(anc_2).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					Rot_Bool(anc_2, R_s).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					enc_b.dag(state);
					Xgate_Bool(anc_1, 0)(state);
					Reflection_Bool(main_reg, true).conditioned_by_all_ones({ anc_1, anc_3, anc_4 })(state);
					Xgate_Bool(anc_1, 0)(state);
					enc_b(state);
					(Hadamard_Bool(anc_3))(state);
				}
				MERGE_BY_CONDITIONS
			}

			template<typename Ty>
			void impl_dag(Ty& state) const
			{
				throw_general_runtime_error();
				profiler _("Block_Encoding_Hs::dag");
				
				SPLIT_BY_CONDITIONS {
					Block_Encoding enc_A_copy = enc_A;

					(Hadamard_Bool(anc_3))(state);
					enc_b.dag(state);
					Reflection_Bool(main_reg, true).conditioned_by_all_ones({ anc_1, anc_3, anc_4 })(state);
					enc_b(state);
					Xgate_Bool(anc_4, 0)(state);
					Rot_Bool(anc_2, R_s).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					Hadamard_Bool(anc_2).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					enc_A_copy.conditioned_by_all_ones({ anc_1, anc_2 })(state);
					Reflection_Bool(anc_2, true).conditioned_by_all_ones(anc_1)(state);
					Xgate_Bool(anc_1, 0).conditioned_by_all_ones(anc_2)(state);
					enc_A_copy.conditioned_by_all_ones({ anc_1, anc_2 }).dag(state);
					Hadamard_Bool(anc_2).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					Rot_Bool(anc_2, R_s).conditioned_by_all_ones(anc_4)(state);
					Xgate_Bool(anc_4, 0)(state);
					enc_b.dag(state);
					Reflection_Bool(main_reg, true).conditioned_by_all_ones({ anc_1, anc_3, anc_4 })(state);
					enc_b(state);
					(Hadamard_Bool(anc_3))(state);
				}
				MERGE_BY_CONDITIONS
			}

			COMPOSITE_OPERATION
		};

		/*
		block-encoding of H(s): positive-definite version
		H(s)=(1-f(s))H_0 + f(s)H_1
		reference: Optimal scaling quantum linear-systems solver via discrete adiabatic theorem[J]. PRX quantum, 2022, 3(4): 040303.
		doi: https://journals.aps.org/prxquantum/abstract/10.1103/PRXQuantum.3.040303
		Construction details: Appendix F
		*/
		template<typename Block_Encoding, typename State_Prep>
		struct Block_Encoding_Hs_PD
		{
			double fs;
			u22_t R_s;
			std::string main_reg;
			std::string anc_UA;
			std::string anc_1;
			std::string anc_2;
			std::string anc_3;
			std::string anc_4;
			// size_t data_size;
			// size_t rational_size;
			Block_Encoding enc_A;
			State_Prep enc_b;
			ClassControllable

			Block_Encoding_Hs_PD(
				Block_Encoding enc_A_,
				State_Prep enc_b_,
				std::string_view main_reg_,
				std::string_view anc_UA_,
				std::string_view anc_1_,
				std::string_view anc_2_,
				std::string_view anc_3_,//a_h
				std::string_view anc_4_,//none
				double fs_) : enc_A(enc_A_), enc_b(enc_b_),
			main_reg(main_reg_), anc_UA(anc_UA_),
			anc_1(anc_1_), anc_2(anc_2_), anc_3(anc_3_), anc_4(anc_4_), fs(fs_)
				// data_size(dsz), rational_size(rsz)
			{
				double sqrt_N = 1.0 / sqrt((1 - fs) * (1 - fs) + fs * fs);
				double u00 = sqrt_N * (1 - fs),
					u01 = sqrt_N * fs,
					u10 = sqrt_N * fs,
					u11 = sqrt_N * (fs - 1);
				R_s = u22_t{ u00, u01, u10, u11 };
			};

			template<typename Ty>
			void operator()(Ty& state) const
			{
				profiler _("Block_Encoding_Hs_PD");
					
				SPLIT_BY_CONDITIONS
				{
					Block_Encoding enc_A_copy = enc_A;
					(Hadamard_Bool(anc_2))(state);
					enc_b.dag(state);					
					Reflection_Bool(main_reg, true).conditioned_by_all_ones({ anc_2, anc_3 })(state);
					enc_b(state);
					Xgate_Bool(anc_3, 0)(state);
					Rot_Bool(anc_1, R_s).conditioned_by_all_ones(anc_3)(state);
					Xgate_Bool(anc_3, 0)(state);
					Hadamard_Bool(anc_1).conditioned_by_all_ones(anc_3)(state);
					enc_A_copy.conditioned_by_all_ones(std::vector<std::string>{ anc_1, anc_3 })(state);
					Xgate_Bool(anc_3, 0)(state);
					enc_A_copy.conditioned_by_all_ones(std::vector<std::string>{ anc_1, anc_3 }).dag(state);
					Hadamard_Bool(anc_1).conditioned_by_all_ones(anc_3)(state);
					Xgate_Bool(anc_3, 0)(state);
					Rot_Bool(anc_1, R_s).conditioned_by_all_ones(anc_3)(state);
					Xgate_Bool(anc_3, 0)(state);
					enc_b.dag(state);
					Reflection_Bool(main_reg, true).conditioned_by_all_ones({ anc_2, anc_3 })(state);
					enc_b(state);
					(Hadamard_Bool(anc_2))(state);
				}
				MERGE_BY_CONDITIONS
			}

		};
		//bool PD = false;
		// template<typename Block_Encoding, typename State_Prep>
		template<typename Block_Encoding, typename State_Prep, bool PD = false>
		struct Walk_s
		{
			double s;
			double kappa;
			double p;
			double fs;
			// double temp;
			complex_t phase = complex_t(0, 1.0);
			std::string main_reg;
			std::string anc_UA;
			std::string anc_1;
			std::string anc_2;
			std::string anc_3;
			std::string anc_4;
			Block_Encoding enc_A;
			State_Prep enc_b;
			constexpr static bool is_positive_definite = PD;
			using EncHs = Block_Encoding_Hs<Block_Encoding, State_Prep>;
			//using EncHs = std::conditional<PD, Block_Encoding_Hs_PD<Block_Encoding, State_Prep>, Block_Encoding_Hs<Block_Encoding, State_Prep>>;
			EncHs enc_Hs;

			ClassControllable

			Walk_s(Block_Encoding enc_A_,
				State_Prep enc_b_,
				std::string_view main_reg_,
				std::string_view anc_UA_,
				std::string_view anc_1_,
				std::string_view anc_2_,
				std::string_view anc_3_,
				std::string_view anc_4_,
				double s_,
				double kappa_,
				double p_) :
			main_reg(main_reg_), anc_UA(anc_UA_),
			anc_1(anc_1_), anc_2(anc_2_), anc_3(anc_3_), anc_4(anc_4_),
			s(s_), kappa(kappa_), p(p_), 
			enc_A(enc_A_), enc_b(enc_b_),
			/* fs (Eq. (69)) page 11 */
			fs(kappa_ / (kappa_ - 1) * (1 - pow(1 + s_ * (pow(kappa_, p_ - 1) - 1), 1 / (1 - p_)))),
			enc_Hs(enc_A_, enc_b_, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4, fs)
			{
			};

			template<typename Ty>
			void impl(Ty& state) const
			{
				profiler _("Walk_s");

				SPLIT_BY_CONDITIONS
				{
					(EncHs(enc_Hs))(state);

					if constexpr (!is_positive_definite)
						Reflection_Bool({ anc_UA, anc_2, anc_3 }, false)(state);
					else {
						Reflection_Bool({ anc_UA, anc_1, anc_2 }, false)(state);
					}
				}
				MERGE_BY_CONDITIONS

				(GlobalPhase_Int(phase))(state);
			}

			template<typename Ty>
			void impl_dag(Ty& state) const
			{
				profiler _("Walk_s::dag");

				(GlobalPhase_Int(-phase))(state);

				SPLIT_BY_CONDITIONS
				{
					if constexpr (!is_positive_definite) {
						Reflection_Bool({ anc_UA, anc_2, anc_3 }, false)(state);
					}
					else {
						Reflection_Bool({ anc_UA, anc_1, anc_2 }, false)(state);
					}

					(EncHs(enc_Hs)).dag(state);
				}
				MERGE_BY_CONDITIONS
			}

			COMPOSITE_OPERATION
		};


		struct QDADebugger
		{
			DenseMatrix<double> matrix_A;
			DenseVector<double> vector_b;
			double fs;
			size_t row_size;

			QDADebugger(
				const DenseMatrix<double>& matrix_A_,
				const DenseVector<double>& vector_b_,
				double s_,
				double kappa_,
				double p_
			) :
				matrix_A(matrix_A_),
				vector_b(vector_b_),
				fs(kappa_ / (kappa_ - 1) * (1 - pow(1 + s_ * (pow(kappa_, p_ - 1) - 1), 1 / (1 - p_)))),
				row_size(vector_b_.size)
			{}

			//void init_eigenstate(std::vector<System>& state);

			DenseMatrix<double> get_matrix_Af();
			DenseVector<double> get_vector_0b();
			DenseVector<double> get_vector_1b();
			std::vector<double> get_mid_eigenstate(bool is_PD=false);
			//std::vector<double> get_matrix_dag();
		};

		struct GetOutput {
			size_t main_reg;
			size_t anc_UA;
			size_t anc_4;
			size_t anc_3;
			size_t anc_2;
			size_t anc_1;
			size_t index;
			size_t anc_h;
			std::vector<size_t> anc_registers;
			//std::vector<double> weights;
			GetOutput(
				std::string main_reg,
				std::string anc_UA,
				std::string anc_4,
				std::string anc_3,
				std::string anc_2,
				std::string anc_1,
				bool is_PD=false) :
				main_reg(System::get(main_reg)), anc_UA(System::get(anc_UA)),
				anc_4(System::get(anc_4)), anc_3(System::get(anc_3)), anc_2(System::get(anc_2)), anc_1(System::get(anc_1))
				//,index(index), anc_h(anc_h)
			{
				/* Mode 1: 5 ancillas are included to post-select */
				//if (!is_PD)
				//{
				//	anc_registers = { anc_UA, anc_3, anc_2, index, anc_h };
				//}
				//else {
				//	anc_registers = { anc_UA, anc_3, anc_2 };
				//}
				anc_registers = { System::get(anc_UA), System::get(anc_3), System::get(anc_2) };
			};
			GetOutput(
				std::string main_reg,
				std::string anc_UA,
				std::string anc_4,
				std::string anc_3,
				std::string anc_2,
				std::string anc_1,
				std::string index,
				std::string anc_h) :
				main_reg(System::get(main_reg)), anc_UA(System::get(anc_UA)),
				anc_4(System::get(anc_4)), anc_3(System::get(anc_3)), anc_2(System::get(anc_2)), anc_1(System::get(anc_1)),
				index(System::get(index)), anc_h(System::get(anc_h))
			{
				anc_registers = { System::get(anc_UA), System::get(anc_3), System::get(anc_2), 
					System::get(index), System::get(anc_h) };
			}

			std::pair<std::vector<complex_t>, double> operator()(const std::vector<System>& state) const;
			
			std::pair<std::vector<complex_t>, double> operator()(const SparseState& state) const
			{
				return (*this)(state.basis_states);
			}
#ifdef USE_CUDA
			std::pair<std::vector<complex_t>, double> operator()(const CuSparseState& state) const;
#endif

			template<typename Ty>
			bool check_removable(Ty& state)
			{
				std::vector<size_t> anc_registers = { anc_UA, anc_3, anc_2, index, anc_h };
				for (size_t i = 0; i < state.size(); ++i)
				{
					System& s = state[i];
					for (auto &reg_id : anc_registers)
					{
						if (s.GetAs(reg_id, size_t) != 0)
							return false;
					}
				}
				return true;
			}

			template<typename Ty>
			std::pair<Ty, double> get_subspace(Ty& state)
			{
				size_t size_mreg = System::size_of(main_reg);
				// The size of anc_4/anc_1 is 1. The length of state_ps is pow2(size_mreg + 1 + 1).
				std::vector<System> state_ps;
				double sum = 0.0;
				for (int i = 0; i < state.size(); ++i)
				{
					System& s = state[i];

					bool is_zero = std::all_of(anc_registers.begin(), anc_registers.end(),
						[&](const size_t& reg)
						{
							size_t v = s.GetAs(reg, size_t);
							return v == 0;
						});
					if (!is_zero)
						continue;
					else {
						state_ps.push_back(s);
						sum += abs_sqr(s.amplitude);
					}
				}
				if (std::abs(sum - 0.0) < epsilon) {
					return { state_ps, sum };
				}
				else {
					double _sqr = sum != 0 ? 1.0 / std::sqrt(sum) : 1.0;
					for (int i = 0; i < state_ps.size(); i++) {
						state_ps[i].amplitude *= _sqr;
					}
					return { state_ps, sum };
				}
			}
		};

		template<typename Ty>
		auto GetQb(const Eigen::MatrixBase<Ty>& b) -> EigenMat<complex_t>
		{
			//auto Vec0 = GetVec0();
			auto Vec = GetVec0<complex_t>();
			//fmt::print("Vec0={}\n", eigenmat2str(Vec0));
			auto Vec0b = kroneckerProduct(Vec, b);
			//fmt::print("Vec0b={}\n", eigenmat2str(Vec0b));
			auto Mat0b = Vec0b * Vec0b.adjoint();
			//fmt::print("Mat0b={}\n", eigenmat2str(Mat0b));
			auto MatI = eyes_like(Mat0b);
			//fmt::print("MatI={}\n", eigenmat2str(MatI));
			auto MatQb = MatI - Mat0b;
			//fmt::print("MatQb={}\n", eigenmat2str(MatQb));
			return MatQb;
		}

		template<typename Ty>
		EigenMat<complex_t> GetAf(const Eigen::MatrixBase<Ty>& A, double fs)
		{
			return (1 - fs) * kroneckerProduct(GetSigmaZ(), eyes_like(A))
				+ fs * HermitianA(A);
		}

		template<typename Ty1, typename Ty2>
		EigenMat<complex_t> GetHs(const Eigen::MatrixBase<Ty1>& A, double fs, const Eigen::MatrixBase<Ty2>& b)
		{
			double scalar = 1.0 / std::sqrt(2.0 * fs * fs + 2.0 * (1 - fs) * (1 - fs));
			auto Af = GetAf(A, fs);
			auto Qb = GetQb(b);
			auto AfQb = scalar * Af * Qb;
			auto QbAf = scalar * Qb * Af;
			auto AfQb01 = kroneckerProduct(GetMat01<complex_t>(), AfQb);
			auto QbAf10 = kroneckerProduct(GetMat10<complex_t>(), QbAf);

			return AfQb01 + QbAf10;
		}
	}
}

/*****************************************************
				Filtering Process
/*****************************************************/
namespace qram_simulator{
	namespace QDA
	{
		template<typename Walk_s>
		struct LCU
		{
			size_t index;
			Walk_s Walk;
			size_t index_size;
			std::string filename;
			ClassControllable

			LCU(Walk_s Walk, size_t index, std::string filename_) :
				Walk(Walk), index(index), filename(filename_)
			{
				index_size = System::size_of(index);
			};

			LCU(Walk_s Walk, std::string index, std::string filename_) :
				Walk(Walk), index(System::get(index)), filename(filename_)
			{
				index_size = System::size_of(index);

			}

			template<typename Ty>
			void operator()(Ty& state)
			{
				for (size_t i = 0; i < index_size; i++)
				{
					fmt::print("LCU step {} / {}\n", i, index_size);
					{
						std::ofstream f_stdout(filename, std::ios::app);
						if (!f_stdout.is_open()) {
							throw std::runtime_error("Failed to open file.");
						}
						f_stdout << fmt::format("LCU step {} / {}\n", i, index_size);
						f_stdout.close();
					}
					Walk.clear_control_by_bit();
					auto _walk = Walk.conditioned_by_bit(index, i).conditioned_by_all_ones(condition_variable_all_ones);
					for (int j = 0; j < pow2(i + 1); j++)
					{
						_walk(state);
					}
				}
			}

			template<typename Ty>
			void dag(Ty& state)
			{
				for (size_t i = 0; i < index_size; i++)
				{
					fmt::print("LCUdag step {} / {}\n", i, index_size);
					{
						std::ofstream f_stdout(filename, std::ios::app);
						if (!f_stdout.is_open()) {
							throw std::runtime_error("Failed to open file.");
						}
						f_stdout << fmt::format("LCUdag step {} / {}\n", i, index_size);
						f_stdout.close();
					}
					Walk.clear_control_by_bit();
					auto _walk = Walk.conditioned_by_bit(index, i).conditioned_by_all_ones(condition_variable_all_ones);
					for (int j = 0; j < pow2(i + 1); j++)
					{
						_walk.dag(state);
					}
				}
			}
		};


		inline std::vector<double> CalculateAngles(std::vector<double>& coeffs) {
			auto l = coeffs.size();
			std::vector<double> angles(l);
			double sum = std::accumulate(coeffs.begin(), coeffs.end(), 0.0);
			for (size_t i = 0; i < l; ++i)
			{
				double s = coeffs[i];
				if (s < 0) { throw_invalid_input(); }
				else {
					double cos_theta_2 = sqrt(s / std::accumulate(coeffs.begin() + i, coeffs.end(), 0.0));
					angles[i] = 2 * acos(cos_theta_2);
				}
			}
			return angles;
		};

		inline double chebyshevT(size_t n, double x) {
			// Base cases
			if (n == 0) return 1;
			if (n == 1) return x;
			double T0 = 1;
			double T1 = x;
			double Tn = 0;
			for (size_t k = 2; k <= n; ++k) {
				Tn = 2 * x * T1 - T0;
				T0 = T1;
				T1 = Tn;
			}
			return Tn;
		}

		inline double DolphChebyshev(double epsilon_, int l_, double phi_) {
			double beta = cosh(acosh(1.0 / epsilon_) / l_);
			double x = epsilon_ * chebyshevT(l_, beta * cos(phi_));
			return x;
		}

		inline double FourierSeries(std::vector<double> weights, double x)
		{
			auto l = weights.size();
			double sum = weights[0];
			for (int i = 1; i < l; i++)
			{
				sum += weights[i] * 2 * cos(i * x);
			}
			return sum;
		}
		// Function to compute the coefficients of the Fourier series
		inline std::vector<double> ComputeFourierCoeffs(double epsilon_, int l_) {
			std::vector<double> coeffs; // Initialize coefficients array
			double P_ = 2 * pi;

			// Calculate each coefficient from w_0 to w_l
			for (int j = 0; j <= l_; ++j) {
				double coeff = 0.0;
				double integral = 0.0;
				double delta_phi = P_ / 10000.0; // Set a small delta_phi for numerical integration

				// Perform numerical integration (e.g., using the trapezoidal rule)
				for (double phi = 0; phi <= P_ / 2; phi += delta_phi) {
					double cos_term = cos(2 * pi * j * phi / P_);
					double func_value = DolphChebyshev(epsilon_, l_, phi);
					double term = func_value * cos_term;

					// Use trapezoidal rule for integration
					integral += term;
				}
				coeff = integral * delta_phi / P_; // Finalize the coefficient
				// Since the function is even, double the coefficient
				//std::cout << "Coefficient a" << j << ": " << 2*coeff << std::endl;
				if ((j % 2) == 0)
					coeffs.push_back(2 * coeff);
			}
			return coeffs;
		}

		template<typename Walk_type>
		struct Filtering
		{
			qram_qutrit::QRAMCircuit* qram_w;
			std::string main_reg;
			std::string anc_UA;
			std::string anc_4;
			std::string anc_3;
			std::string anc_2;
			std::string anc_1;
			std::string index;
			std::string anc_h;
			size_t data_size;
			size_t rational_size;
			Walk_type Walk;
			int index_size;
			std::string stdout_filename;
			Filtering(qram_qutrit::QRAMCircuit* qram_w,
				Walk_type Walk,
				std::string main_reg,
				std::string anc_UA,
				std::string anc_4,
				std::string anc_3,
				std::string anc_2,
				std::string anc_1,
				std::string index,
				std::string anc_h,
				size_t ds,
				size_t rs,
				std::string stdout_filename_) :
				qram_w(qram_w), Walk(Walk), main_reg(main_reg), anc_UA(anc_UA),
				anc_4(anc_4), anc_3(anc_3), anc_2(anc_2), anc_1(anc_1),
				index(index), anc_h(anc_h), data_size(ds), rational_size(rs), stdout_filename(stdout_filename_)
			{
				index_size = System::size_of(index);
			};

			template<typename Ty>
			void random_state_generate(Ty& state)
			{
				//Hadamard_Int(anc_4, 1)(state);
				Hadamard_Int(anc_1, 1)(state);
				Hadamard_Int(main_reg, System::size_of(main_reg))(state);

				for (auto& s : state)
				{
					s.amplitude = random_engine::rng() * 2 - 1;
				}
				Normalize()(state);
			}
			//std::string dump_format() const
			//{
			//	return fmt::format("Filtering");
			//}

			template<typename Ty>
			double operator()(Ty& state)
			{
				using namespace state_prep;
				State_Prep_via_QRAM(qram_w, index, data_size, rational_size)(state);
				// StatePrint(0, 16)(state);
				(Hadamard_Int_Full(anc_h))(state);
				LCU<Walk_type>(Walk, index, stdout_filename).conditioned_by_all_ones(anc_h)(state);

				Xgate_Bool(anc_h, 0)(state);
				LCU<Walk_type>(Walk, index, stdout_filename).conditioned_by_all_ones(anc_h).dag(state);
				Xgate_Bool(anc_h, 0)(state);

				(Hadamard_Int_Full(anc_h))(state);
				State_Prep_via_QRAM(qram_w, index, data_size, rational_size).dag(state);

				{// stdout writing
					std::ofstream f_stdout(stdout_filename, std::ios::app);
					if (!f_stdout.is_open()) {
						throw std::runtime_error("Failed to open file.");
					}
					f_stdout << fmt::format("\nAfter filtering: \n");
					f_stdout << fmt::format("Maximum Qubit Count = {}\n", System::max_qubit_count);
					f_stdout << fmt::format("Maximum Register Count = {}\n", System::max_register_count);
					f_stdout << fmt::format("Maximum System Size = {}\n\n", System::max_system_size);
					f_stdout.close();
				}

				double prob_inv = PartialTraceSelect({ System::get(anc_h), System::get(index) }, { 0, 0 })(state);
				double prob = (1.0 / prob_inv) * (1.0 / prob_inv);
				return prob;
			}
		};
	} // namespace QDA

	namespace QDA
	{
		/* Extract full unitary of BlockEncoding_Hs */
		template <typename Block_Encoding, typename State_Prep, typename StateType = SparseState>
		DenseMatrix<complex_t> _extract_full_unitary(
			Block_Encoding_Hs<Block_Encoding, State_Prep> encHs,
			std::string main_reg, std::string anc_UA,
			std::string anc_1, std::string anc_2, std::string anc_3, std::string anc_4)
		{
			size_t main_reg_num = System::size_of(main_reg);
			size_t anc_UA_num = System::size_of(anc_UA);
			size_t qubit_num = main_reg_num + anc_UA_num + 4;
			fmt::print("main_reg_num = {}, anc_UA_num = {}", main_reg_num, anc_UA_num);

			DenseMatrix<complex_t> ret(pow2(qubit_num));
			int main_reg_pos = System::get(main_reg);
			int anc_UA_pos = System::get(anc_UA);
			int anc_1_pos = System::get(anc_1);
			int anc_2_pos = System::get(anc_2);
			int anc_3_pos = System::get(anc_3);
			int anc_4_pos = System::get(anc_4);

			auto i_range = range(pow2(main_reg_num));
			auto a_A_range = range(pow2(anc_UA_num));
			auto b1_range = range(2);
			auto b2_range = range(2);
			auto b3_range = range(2);
			auto b4_range = range(2);

			for (auto [i, a_A, b1, b2, b3, b4] : product(i_range, a_A_range, b1_range, b2_range, b3_range, b4_range)) {
				/*std::vector<System> state;
				state.emplace_back();*/
				StateType state(1);
				state.back().get(main_reg_pos).value = i;
				state.back().get(anc_UA_pos).value = a_A;
				state.back().get(anc_1_pos).value = b1;
				state.back().get(anc_2_pos).value = b2;
				state.back().get(anc_3_pos).value = b3;
				state.back().get(anc_4_pos).value = b4;

				encHs(state);

				std::vector<complex_t> vec(pow2(qubit_num), 0);

				for (auto& s : state)
				{
					size_t index = concat_value(
						{
							{s.get(main_reg_pos).value, main_reg_num},
							{s.get(anc_UA_pos).value, anc_UA_num},
							{s.get(anc_1_pos).value, 1},
							{s.get(anc_2_pos).value, 1},
							{s.get(anc_3_pos).value, 1},
							{s.get(anc_4_pos).value, 1}
						}
					);
					vec[index] = s.amplitude;
				}
				for (size_t j = 0; j < pow2(qubit_num); ++j)
				{
					size_t index = concat_value(
						{
							{i, main_reg_num},
							{a_A, anc_UA_num},
							{b1, 1},
							{b2, 1},
							{b3, 1},
							{b4, 1}
						}
					);

					ret(j, index) = vec[j];
				}
			}
			return ret;
		}

		template <typename Block_Encoding, typename State_Prep>
		DenseMatrix<complex_t> extract_full_unitary(
			Block_Encoding_Hs<Block_Encoding, State_Prep> encHs,
			std::string main_reg, std::string anc_UA,
			std::string anc_1, std::string anc_2, std::string anc_3, std::string anc_4)
		{
			return _extract_full_unitary<Block_Encoding, State_Prep, SparseState>(encHs, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4);
		}
		

		/* Extract the block encoding part of the Hs */
		template <typename Block_Encoding, typename State_Prep, typename StateType = SparseState>
		DenseMatrix<complex_t> _extract_block_encoding_Hs(
			Block_Encoding_Hs<Block_Encoding, State_Prep> encHs,
			std::string main_reg, std::string anc_UA,
			std::string anc_1, std::string anc_2, std::string anc_3, std::string anc_4,
			size_t qubit_num)
		{
			DenseMatrix<complex_t> ret(pow2(qubit_num + 2));
			auto main_reg_pos = System::get(main_reg);
			auto anc_UA_pos = System::get(anc_UA);
			auto anc_1_pos = System::get(anc_1);
			auto anc_2_pos = System::get(anc_2);
			auto anc_3_pos = System::get(anc_3);
			auto anc_4_pos = System::get(anc_4);

			auto i_range = range(pow2(qubit_num));
			auto b1_range = range(2);
			auto b4_range = range(2);

			for (auto [i, b1, b4] : product(i_range, b1_range, b4_range))
			{
				//std::vector<System> state;
				//state.emplace_back();
				StateType state(1);
				state.back().get(main_reg_pos).value = i;
				state.back().get(anc_UA_pos).value = 0;
				state.back().get(anc_1_pos).value = b1;
				state.back().get(anc_2_pos).value = 0;
				state.back().get(anc_3_pos).value = 0;
				state.back().get(anc_4_pos).value = b4;

				encHs(state);
				//StatePrint()(state);
				double prob = PartialTraceSelect({ { anc_UA, 0 },{ anc_2, 0 },{ anc_3, 0 } })(state);

				std::vector<complex_t> vec(pow2(qubit_num + 2), 0);
				for (auto& s : state)
				{
					vec[s.get(main_reg_pos).value
						+ (s.get(anc_1_pos).value << qubit_num)
						+ (s.get(anc_4_pos).value << (qubit_num + 1))
					] = s.amplitude;
				}
				for (size_t j = 0; j < pow2(qubit_num + 2); ++j)
				{
					ret(j, (b4 << (qubit_num + 1)) + (b1 << qubit_num) + i) = vec[j] / prob;
				}
			}
			return ret;
		}


		template <typename Block_Encoding, typename State_Prep>
		DenseMatrix<complex_t> extract_block_encoding_Hs(
			Block_Encoding_Hs<Block_Encoding, State_Prep> encHs,
			std::string main_reg, std::string anc_UA,
			std::string anc_1, std::string anc_2, std::string anc_3, std::string anc_4,
			size_t qubit_num)
		{
			return _extract_block_encoding_Hs<Block_Encoding, State_Prep, SparseState>(encHs, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4, qubit_num);
		}
	}
}