#pragma once

#if false

#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include <iostream>
#include <fstream>
#include <Eigen/Eigen>

namespace qram_simulator {

	namespace CNN {
		std::vector<double> convolve4x4(const std::vector<double>& image, const std::vector<double>& kernel);

		int findpow2(int input);

		std::vector<double> img2col(std::vector<double>& input, int kernel_size);
		std::vector<double> col2img(std::vector<double>& input, int kernel_size, int pic_size);

		//khan

		int reverseInt(int i);

		std::vector<int> read_mnist(std::string name);

		void print_vector(const std::vector<int>& v);

		void save_vector_to_file(const std::vector<int>& v, const std::string& filename);
		double khan_getProb(std::vector<System>& state, std::string anc, std::string anc_cr);

		struct CondRot_P {
			using angle_function_t = std::function<u22_t(size_t, double)>;

			int in_id;
			int out_id;
			angle_function_t func;
			double norm_c;

			ClassControllable

				CondRot_P(std::string reg_in, std::string reg_out, angle_function_t angle_function, double norm);
			CondRot_P(int reg_in, int reg_out, angle_function_t angle_function, double norm);

			void operate(size_t l, size_t r, std::vector<System>& state) const;

			static bool _is_diagonal(const u22_t& data);
			void _operate_diagonal(size_t l, size_t r,
				std::vector<System>& state, const u22_t& mat) const;

			static bool _is_off_diagonal(const u22_t& data);
			void _operate_off_diagonal(size_t l, size_t r,
				std::vector<System>& state, const u22_t& mat) const;

			void _operate_general(size_t l, size_t r,
				std::vector<System>& state, const u22_t& mat) const;

			void operator()(std::vector<System>& state) const;
		};

		u22_t conrotfunc_P(size_t x, double norm);

		u22_t conrotfunc_P_inv(size_t x, double norm);

		double getProb(std::vector<System>& state, int i, int j);

		memory_t padding(memory_t memory, size_t size);

		struct AllPhaseFlip
		{
			// std::vector<std::string> regs;

			ClassControllable

				AllPhaseFlip() {};
			void operator()(std::vector<System>& state) const;
		};

		struct isay
		{
			size_t low_bounder, high_bounder;
			std::string reg_to_change;
			isay(std::string reg_to_change, size_t low_bounder, size_t high_bounder);
			void operator()(std::vector<System>& state) const;
		};

		struct reindex
		{
			size_t div;
			std::string reg_to_change;
			reindex(std::string reg_to_change, size_t div);
			void operator()(std::vector<System>& state) const;
		};

		struct set_p
		{
			size_t col, row, start_index;
			std::string reg_to_change;
			set_p(std::string reg_to_change, size_t col, size_t row, size_t start_index);
			void operator()(std::vector<System>& state) const;
		};

		struct killreg
		{
			int addr_reg;
			killreg(int addr_reg)
				: addr_reg(addr_reg)
			{}
			killreg(std::string addr_reg)
				: addr_reg(System::get(addr_reg))
			{}
			void operator()(std::vector<System>& state);

		};
		struct indexCal
		{
			int addr_reg_i, addr_reg_j;
			int index;
			int N;
			indexCal(int addr_reg_i_, int addr_reg_j_, int data_reg_as, int index_, int N_);
			indexCal(std::string addr_reg_i_, std::string addr_reg_j_, std::string index_, int N_);
			void operator()(std::vector<System>& state);
		};

		struct 	AmplitudeLoad
		{
			int addr_reg_data;//0-p
			int index;//0-Api
			int data;//data
			int p;//为了保证归一化，在每一个分支之前要乘以sqrt(p)
			AmplitudeLoad(int addr_reg_data_, int index_, int data_, int p_);
			AmplitudeLoad(std::string addr_reg_data_, std::string index_, std::string data_, int p_);
			void operator()(std::vector<System>& state);
		};

		double get_martix_F_norm(const std::vector<std::vector<double>>& A);
		void test_get_martix_F_norm();
		Eigen::MatrixXd convertVecToEigen(const std::vector<std::vector<double>>& vec);
		void test_get_martix_Spectral_norm();
		double get_martix_Spectral_norm(std::vector<std::vector<double>> A);

		std::vector<double> vectorappend(std::vector<double> pic);
		std::vector<double> reversekernel(std::vector<double> kernel);
		std::vector<double> paddingforback(std::vector<double>& output, int kernel_size);
		double get_vector_F_form(std::vector<double> input);
	}
}

#endif