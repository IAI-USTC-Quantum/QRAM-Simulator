#pragma once
#include "global_macros.h"
#include "block_encoding.h"
#include "matrix.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <algorithm>
#include <string>
#include <Eigen/Eigen>
#include <regex>
#include "BlockEncoding/make_qram.h"
#include "BlockEncoding/block_encoding_via_QRAM.h"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"

#include "argparse.h"


using namespace qram_simulator;
using namespace QDA;
using namespace QDA::QDA_tridiagonal;
using namespace QDA::QDA_via_QRAM;
using namespace block_encoding;
using namespace block_encoding::block_encoding_via_QRAM;

inline EigenMat<double> GetA(double alpha, double beta, size_t sz) {
	EigenMat<double> mat = EigenMat<double>::Zero(sz, sz);

	// set diagonal elements
	for (size_t i = 0; i < sz; ++i) {
		mat(i, i) = alpha; // set sub-diagonal elements
		if (i < sz - 1) {
			mat(i, i + 1) = beta; // upper sub-diagonal elements
			mat(i + 1, i) = beta; // lower sub-diagonal elements
		}
	}

	return mat;
}

inline EigenMat<double> Getb(size_t sz)
{
	EigenMat<double> mat = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);
	return mat;
}

void testBlockEncodingA_via_Tridiagonal(size_t nqubit, double alpha, double beta)
{
	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("anc_UA", UnsignedInteger, 4);
	
	fmt::print("A (Extraction of block encoding of A):\n");
	{
		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_Tridiagonal block_enc("main_reg", "anc_UA", alpha, beta);
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
			is_full, is_dag);
		fmt::print("{}\n", ret.to_string());
	}

	fmt::print("Test Block Encoding U_A:\n");
	{
		bool is_full = true;
		bool is_dag = false;
		Block_Encoding_Tridiagonal block_enc("main_reg", "anc_UA", alpha, beta);
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
			is_full, is_dag);
		//fmt::print("{}\n", ret.to_string());

		std::filesystem::path current_dir = std::filesystem::current_path();
		std::string sign = beta > 0 ? "pos" : "neg";
		std::string _savename = current_dir.string() + 
			fmt::format("\\U_A_via_Tridiagonal_{}_ssc.txt", sign);
		{
			std::ofstream f_file(_savename);
			if (!f_file.is_open()) {
				throw std::runtime_error("Failed to open file.");
			}
			f_file << ret.to_string();
			f_file.close();
		}
	}
}

void testBlockEncodingA_via_QRAM(size_t nqubit, double alpha, double beta)
{
	size_t sz = pow2(nqubit);
	DenseMatrix<double> mat = generate_Poiseuille_mat<double>(sz, alpha, beta);
	int exponent = 20;
	size_t data_size = 50;
	size_t rational_size = 51;

	//DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(qubit_num), alpha, beta);
	mat = mat / mat.normF();
	fmt::print("mat_A:\n{}", mat.to_string());

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("anc_UA", UnsignedInteger, nqubit);

	fmt::print("Test Block Encoding U_A:\n");
	{
		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_via_QRAM block_enc(&qram_A, "main_reg", "anc_UA", data_size, rational_size);

		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA", 
															is_full, is_dag);

		fmt::print("{}\n", ret.to_string());
	}

	fmt::print("Test Block Encoding U_A (full):\n");
	{
		bool is_full = true;
		bool is_dag = false;
		Block_Encoding_via_QRAM block_enc(&qram_A, "main_reg", "anc_UA", data_size, rational_size);

		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA", 
															is_full, is_dag);

		//fmt::print("{}\n", ret.to_string());

		std::filesystem::path current_dir = std::filesystem::current_path();
		std::string sign = beta > 0 ? "pos" : "neg";
		std::string _savename = current_dir.string() +
			fmt::format("\\U_A_via_QRAM_{}_ssc.txt", sign);
		{
			std::ofstream f_file(_savename);
			if (!f_file.is_open()) {
				throw std::runtime_error("Failed to open file.");
			}
			f_file << ret.to_string();
			f_file.close();
		}
	}
}

DenseVector<complex_t> extract_full_state(std::vector<System>& state,
	std::vector<std::string>& regs)
{
	size_t reg_num = regs.size();
	std::vector<int> pos_vec(reg_num);
	std::vector<size_t> size_vec(reg_num);
	size_t qubit_num = 0;

	for (size_t i = 0; i < reg_num; ++i)
	{
		pos_vec[i] = System::get(regs[i]);
		size_vec[i] = System::size_of(regs[i]);
		qubit_num += size_vec[i];
	}
	
	DenseVector<complex_t> ret_vec(pow2(qubit_num));

	for (auto& s : state)
	{
		std::vector<std::pair<size_t, size_t>> value_vec(0);
		for (size_t i = 0; i < reg_num; ++i)
		{
			value_vec.emplace_back(s.get(pos_vec[i]).value, size_vec[i]);
		}
		size_t index = concat_value(value_vec);
		ret_vec[index] = s.amplitude;
	}
	return ret_vec;
}

void testBlockEncodingHs_via_Tridiagonal(size_t nqubit, double alpha, double beta, double fs)
{
	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("anc_UA", UnsignedInteger, 4);
	System::add_register("anc_1", Boolean, 1);
	System::add_register("anc_2", Boolean, 1);
	System::add_register("anc_3", Boolean, 1);
	System::add_register("anc_4", Boolean, 1);

	Block_Encoding_Tridiagonal encA("main_reg", "anc_UA", alpha, beta);
	Hadamard_Int_Full encb("main_reg");

	Block_Encoding_Hs<Block_Encoding_Tridiagonal, Hadamard_Int_Full> encHs(encA, encb,
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

	DenseMatrix<complex_t> ret = extract_block_encoding_Hs(encHs,
		"main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4", nqubit);
	fmt::print("Is Hermitian?\n{}\n", ret.is_Hermitian());
	fmt::print("{}\n", ret.to_string());
}

void testBlockEncodingUs_via_Tridiagonal(size_t nqubit, double alpha, double beta, double fs)
{
	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("anc_UA", UnsignedInteger, 4);
	System::add_register("anc_1", Boolean, 1);
	System::add_register("anc_2", Boolean, 1);
	System::add_register("anc_3", Boolean, 1);
	System::add_register("anc_4", Boolean, 1);

	Block_Encoding_Tridiagonal encA("main_reg", "anc_UA", alpha, beta);
	Hadamard_Int_Full encb("main_reg");

	Block_Encoding_Hs<Block_Encoding_Tridiagonal, Hadamard_Int_Full> encHs(encA, encb,
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

	DenseMatrix<complex_t> ret = extract_full_unitary(encHs,
		"main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4");
	fmt::print("\nIs Hermitian?\n{}\n", ret.is_Hermitian());
	//fmt::print("{}\n", ret.to_string());

	std::filesystem::path current_dir = std::filesystem::current_path();
	std::string sign = beta > 0 ? "pos" : "neg";
	std::string _savename = current_dir.string() +
							fmt::format("\\U_Hs_via_Tridiagonal_{}_ssc.txt", sign);
	{
		std::ofstream f_file(_savename);
		if (!f_file.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_file << ret.to_string();
		f_file.close();
	}

}

void testBlockEncodingHs_via_QRAM(size_t nqubit, double alpha, double beta, double fs)
{
	size_t sz = pow2(nqubit);
	DenseMatrix<double> mat = generate_Poiseuille_mat<double>(sz, alpha, beta);
	int exponent = 20;
	size_t data_size = 50;
	size_t rational_size = 51;

	//DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(qubit_num), alpha, beta);
	mat = mat / mat.normF();
	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);

	Block_Encoding_via_QRAM encA(&qram_A, "main_reg", "anc_UA", data_size, rational_size);
	
	Hadamard_Int_Full encb("main_reg");

	Block_Encoding_Hs<Block_Encoding_via_QRAM, Hadamard_Int_Full> encHs(encA, encb,
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

	DenseMatrix<complex_t> ret = extract_block_encoding_Hs(encHs,
		"main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4", nqubit);
	fmt::print("Hs is Hermitian? {}\n", ret.is_Hermitian());
	fmt::print("{}\n", ret.to_string());
}

void testBlockEncodingUs_via_QRAM(size_t nqubit, double alpha, double beta, double fs)
{
	size_t sz = pow2(nqubit);
	DenseMatrix<double> mat = generate_Poiseuille_mat<double>(sz, alpha, beta);
	int exponent = 20;
	size_t data_size = 50;
	size_t rational_size = 51;

	
	mat = mat / mat.normF();
	fmt::print("mat_A:\n{}", mat.to_string());

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("anc_UA", UnsignedInteger, nqubit);
	System::add_register("anc_1", Boolean, 1);
	System::add_register("anc_2", Boolean, 1);
	System::add_register("anc_3", Boolean, 1);
	System::add_register("anc_4", Boolean, 1);

	Block_Encoding_via_QRAM encA(&qram_A, "main_reg", "anc_UA", data_size, rational_size);

	Hadamard_Int_Full encb("main_reg");

	Block_Encoding_Hs<Block_Encoding_via_QRAM, Hadamard_Int_Full> encHs(encA, encb,
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

	DenseMatrix<complex_t> ret = extract_full_unitary(encHs,
		"main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4");
	fmt::print("Is Hermitian?\n{}\n", ret.is_Hermitian());
	//fmt::print("{}\n", ret.to_string());

	std::filesystem::path current_dir = std::filesystem::current_path();
	std::string sign = beta > 0 ? "pos" : "neg";
	std::string _savename = current_dir.string() +
		fmt::format("\\U_Hs_via_QRAM_{}_ssc.txt", sign);
	{
		std::ofstream f_file(_savename);
		if (!f_file.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_file << ret.to_string();
		f_file.close();
	}
}

void testBlockEncodingHs_via_EigenMat(size_t nqubit, DenseMatrix<double>& mat, double fs)
{	
	size_t sz = pow2(nqubit);
	auto matA = mat.to_eigen();
	double normA = matA.norm();
	matA = matA / normA;

	auto matb = Getb(sz);
	auto matHs = GetHs(matA, fs, matb);
	fmt::print("A={}\n", eigenmat2str(matA));
	fmt::print("b={}\n", eigenmat2str(matb));
	DenseMatrix<complex_t> matHs_dense(matHs);

	fmt::print("Hs=\n{}\n", matHs_dense.to_string());
}

void testBlockEncodingHs_via_EigenMat(size_t nqubit, double alpha, double beta, double fs)
{
	size_t sz = pow2(nqubit);
	auto matA = GetA(alpha, beta, sz);
	double normA = matA.norm();
	matA = matA / normA;

	auto matb = Getb(sz);
	auto matHs = GetHs(matA, fs, matb);
	fmt::print("A={}\n", eigenmat2str(matA));
	fmt::print("b={}\n", eigenmat2str(matb));
	DenseMatrix<complex_t> matHs_dense(matHs);

	fmt::print("Hs=\n{}\n", matHs_dense.to_string());
}


int PoiseuilleTest_via_Tridiagonal_nonPD(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	auto mat = get_tridiagonal_matrix(alpha, beta, pow2(nqubit));
	mat = mat / mat.normF();

	fmt::print("matrix = \n{}\n", mat.to_string());
	auto vec = ones<double>(pow2(nqubit));
	vec = vec / vec.norm2();
	
	// set filename for data saving:
	std::filesystem::path current_dir = std::filesystem::current_path();
	std::string sign;
	if (beta > 0)
		sign = "pos";
	else
		sign = "neg";
	std::string _savename = fmt::format("_sr{:.5f}_p{:.2f}_nq{}_Tridiagonal_{}.txt",
		step_rate, p, nqubit, sign);
	std::string stdout_savename = current_dir.string() + "\\stdout" + _savename;
	std::string state_savename = current_dir.string() + "\\state" + _savename;
	std::string fidelity_savename = current_dir.string() + "\\fidelity" + _savename;
	//std::string dump_savename = current_dir.string() + "\\dump" + _savename;
	fmt::print("\nfilename for fidelity saving: {}\n", fidelity_savename);
	fmt::print("\nfilename for std output: {}\n", stdout_savename);
	fmt::print("\nfilename for state saving: {}\n", state_savename);
	//fmt::print("\nfilename for dump saving: {}\n", dump_savename);
	//QModule::set_dumpfile(dump_savename);
	// System initialization.
	std::vector<System> state;
	state.emplace_back();
	auto main_reg = AddRegister("main_reg", UnsignedInteger, nqubit)(state);
	auto anc_UA = AddRegister("anc_UA", UnsignedInteger, 4)(state);
	auto anc_4 = AddRegister("anc_4", Boolean, 1)(state);
	auto anc_3 = AddRegister("anc_3", Boolean, 1)(state);
	auto anc_2 = AddRegister("anc_2", Boolean, 1)(state);
	auto anc_1 = AddRegister("anc_1", Boolean, 1)(state);
	
	// Do walk sequence or directly prepare the eigenstate of H_1.
	// To set initial state as |0>_{anc_1}|b>
	(Hadamard_Int_Full(main_reg))(state);
	
	double kappa = get_kappa_Tridiagonal(alpha, beta, pow2(nqubit));
	fmt::print("kappa = {}\n", kappa);
	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) {
		steps += 1;
	}
	//getchar();
	{
		std::ofstream f_stdout(stdout_savename);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_stdout << fmt::format("\nDiscrete Adiabatic Test for Poiseuille Flow\n");
		f_stdout << fmt::format("\nnqubit:{}\n", nqubit);
		f_stdout << fmt::format("\nkappa:{}\n", kappa);
		f_stdout << fmt::format("\nstep_rate = {}, steps = {}, p = {}\n", step_rate, steps, p);
		f_stdout << fmt::format("\nPoiseuille matrix:\n{}\n", mat.to_string());
		f_stdout << fmt::format("\nvector b:\n{}\n", vec.to_string());
		f_stdout.close();
	}
	{// fidelity writing
		std::ofstream f_fidelity(fidelity_savename);
		if (!f_fidelity.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_fidelity.close();
	}

	auto start = std::chrono::high_resolution_clock::now();
	for (size_t n = 0; n < steps; n++)
	{
		double s = double(n) / steps;
		auto walk = Walk_s_Tridiagonal_Debug(mat, vec, "main_reg", "anc_UA",
			"anc_1", "anc_2", "anc_3", "anc_4",
			s, kappa, p, alpha, beta);
		walk(state);
		ClearZero()(state);

		CheckNormalization(1e-10)(state);
		if ((n + 1) % 2 == 0)
		{
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
			std::vector<double> ideal_state = walk.get_mid_eigenstate();

			double fidelity = get_fidelity(ideal_state, mid_state.first);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);
			
			{// stdout writing
				std::ofstream f_stdout(stdout_savename, std::ios::app);
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
				std::ofstream f_fidelity(fidelity_savename, std::ios::app);
				if (!f_fidelity.is_open()) {
					throw std::runtime_error("Failed to open file.");
				}
				f_fidelity << fmt::format("step: {} / {}, fidelity: {}, p_success: {}, max_system_size: {}\n",
					n, steps, fidelity, mid_state.second, System::max_system_size);
				f_fidelity.close();
			}
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);

		}
		else {
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
			//fmt::print("state after walk = {}\n", mid_state.first);
		}
	}

	auto walk = Walk_s_Tridiagonal_Debug(mat, vec, "main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4",
		1.0, kappa, p, alpha, beta);
	std::vector<double> ideal_state = walk.get_mid_eigenstate();
	fmt::print("{}\n", ideal_state);

	double prob_inv0 = PartialTraceSelect({ anc_UA, anc_2, anc_3 }, { 0, 0, 0 })(state);
	StatePrint(0, 10)(state);

	auto final_result = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
	double fidelity = get_fidelity(ideal_state, final_result.first);
	auto end_w = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration_w = end_w - start;

	// wirte final result to file:
	{
		std::ofstream f_stdout(stdout_savename, std::ios_base::app);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file for appending.");
		}
		f_stdout << fmt::format("\nfidelity after walk sequence: {}\n", fidelity);
		f_stdout << fmt::format("\n===================== Walk Time =====================\n");
		f_stdout << fmt::format("Walk duration: {} mins\n", duration_w.count() / 60);
		f_stdout << fmt::format("===================== Walk Time =====================\n");
		f_stdout << fmt::format("{}\n", profiler::get_all_profiles_v2());

		f_stdout.close();
	}
	

	print_state_to_file(state, state_savename, 16);

	return 0;
}

int PoiseuilleTest_via_Tridiagonal_PD(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	auto mat = get_tridiagonal_matrix(alpha, beta, pow2(nqubit));
	mat = mat / mat.normF();

	fmt::print("matrix = \n{}\n", mat.to_string());
	auto vec = ones<double>(pow2(nqubit));
	vec = vec / vec.norm2();

	// set filename for data saving:
	std::filesystem::path current_dir = std::filesystem::current_path();
	std::string sign;
	if (beta > 0)
		sign = "pos";
	else
		sign = "neg";
	std::string _savename = fmt::format("_sr{:.5f}_p{:.2f}_nq{}_Tridiagonal_{}.txt",
		step_rate, p, nqubit, sign);
	std::string stdout_savename = current_dir.string() + "\\stdout" + _savename;
	std::string state_savename = current_dir.string() + "\\state" + _savename;
	std::string fidelity_savename = current_dir.string() + "\\fidelity" + _savename;
	fmt::print("\nfilename for fidelity saving: {}\n", fidelity_savename);
	fmt::print("\nfilename for std output: {}\n", stdout_savename);
	fmt::print("\nfilename for state saving: {}\n", state_savename);

	// System initialization.
	std::vector<System> state;
	state.emplace_back();
	auto main_reg = AddRegister("main_reg", UnsignedInteger, nqubit)(state);
	auto anc_UA = AddRegister("anc_UA", UnsignedInteger, 4)(state);

	auto anc_3 = AddRegister("anc_3", Boolean, 1)(state);
	auto anc_2 = AddRegister("anc_2", Boolean, 1)(state);
	auto anc_1 = AddRegister("anc_1", Boolean, 1)(state);

	// Do walk sequence or directly prepare the eigenstate of H_1.
	// To set initial state as |0>_{anc_1}|b>
	(Hadamard_Int_Full(main_reg))(state);

	double kappa = get_kappa_Tridiagonal(alpha, beta, pow2(nqubit));
	fmt::print("kappa = {}\n", kappa);
	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) {
		steps += 1;
	}
	//getchar();
	{
		std::ofstream f_stdout(stdout_savename);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_stdout << fmt::format("\nDiscrete Adiabatic Test for Poiseuille Flow\n");
		f_stdout << fmt::format("\nnqubit:{}\n", nqubit);
		f_stdout << fmt::format("\nkappa:{}\n", kappa);
		f_stdout << fmt::format("\nstep_rate = {}, steps = {}, p = {}\n", step_rate, steps, p);
		f_stdout << fmt::format("\nPoiseuille matrix:\n{}\n", mat.to_string());
		f_stdout << fmt::format("\nvector b:\n{}\n", vec.to_string());
		f_stdout.close();
	}
	{// fidelity writing
		std::ofstream f_fidelity(fidelity_savename);
		if (!f_fidelity.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_fidelity.close();
	}

	auto start = std::chrono::high_resolution_clock::now();
	for (size_t n = 0; n < steps; n++)
	{
		double s = double(n) / steps;
		auto walk = Walk_s_Tridiagonal_Debug(mat, vec, "main_reg", "anc_UA",
			"anc_1", "anc_2", "anc_3", "anc_4",
			s, kappa, p, alpha, beta);
		walk(state);
		ClearZero()(state);

		CheckNormalization(1e-10)(state);
		if ((n + 1) % 2 == 0)
		{
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
			std::vector<double> ideal_state = walk.get_mid_eigenstate();

			double fidelity = get_fidelity(ideal_state, mid_state.first);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);

			{// stdout writing
				std::ofstream f_stdout(stdout_savename, std::ios::app);
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
				std::ofstream f_fidelity(fidelity_savename, std::ios::app);
				if (!f_fidelity.is_open()) {
					throw std::runtime_error("Failed to open file.");
				}
				f_fidelity << fmt::format("step: {} / {}, fidelity: {}, p_success: {}, max_system_size: {}\n",
					n, steps, fidelity, mid_state.second, System::max_system_size);
				f_fidelity.close();
			}
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);

		}
		else {
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
			//fmt::print("state after walk = {}\n", mid_state.first);
		}
	}

	auto walk = Walk_s_Tridiagonal_Debug(mat, vec, "main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4",
		1.0, kappa, p, alpha, beta);
	std::vector<double> ideal_state = walk.get_mid_eigenstate();
	fmt::print("{}\n", ideal_state);

	double prob_inv0 = PartialTraceSelect({ anc_UA, anc_2, anc_3 }, { 0, 0, 0 })(state);
	StatePrint(0, 10)(state);

	auto final_result = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
	double fidelity = get_fidelity(ideal_state, final_result.first);
	auto end_w = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration_w = end_w - start;

	// wirte final result to file:
	{
		std::ofstream f_stdout(stdout_savename, std::ios_base::app);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file for appending.");
		}
		f_stdout << fmt::format("\nfidelity after walk sequence: {}\n", fidelity);
		f_stdout << fmt::format("\n===================== Walk Time =====================\n");
		f_stdout << fmt::format("Walk duration: {} mins\n", duration_w.count() / 60);
		f_stdout << fmt::format("===================== Walk Time =====================\n");
		f_stdout << fmt::format("{}\n", profiler::get_all_profiles_v2());
		f_stdout.close();
	}


	print_state_to_file(state, state_savename, 16);

	return 0;
}

int PoiseuilleTest_via_QRAM_nonPD(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	int exponent = 15;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();
	
	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	DenseVector<double> b = ones<double>(pow2(nqubit));
	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	qram_qutrit::QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	// set filename for data saving:
	std::filesystem::path current_dir = std::filesystem::current_path();
	std::string sign;
	if (beta > 0)
		sign = "pos";
	else
		sign = "neg";
	std::string _savename = fmt::format("_sr{:.5f}_p{:.2f}_nq{}_QRAM_{}.txt",
		step_rate, p, nqubit, sign);
	std::string stdout_savename = current_dir.string() + "\\stdout" + _savename;
	std::string state_savename = current_dir.string() + "\\state" + _savename;
	std::string fidelity_savename = current_dir.string() + "\\fidelity" + _savename;

	fmt::print("\nfilename for fidelity saving: {}\n", fidelity_savename);
	fmt::print("\nfilename for std output: {}\n", stdout_savename);
	fmt::print("\nfilename for state saving: {}\n", state_savename);


	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);
	// do walk sequence or directly prepare the eigenstate of H_1.
	// To set initial state as |1>_{anc_1}|b>
	std::vector<System> state;
	state.emplace_back();

	(Hadamard_Int_Full(main_reg))(state);
	//Xgate_Bool(anc_1, 0)(state);

	double kappa = get_kappa_Tridiagonal(alpha, beta, pow2(nqubit));
	fmt::print("kappa = {}\n", kappa);

	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) {
		steps += 1;
	}
	
	{
		std::ofstream f_stdout(stdout_savename);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_stdout << fmt::format("\nDiscrete Adiabatic Test for Poiseuille Flow\n");
		f_stdout << fmt::format("\nnqubit:{}\n", nqubit);
		f_stdout << fmt::format("\nkappa:{}\n", kappa);
		f_stdout << fmt::format("\nstep_rate = {}, steps = {}, p = {}\n", step_rate, steps, p);
		f_stdout << fmt::format("\nPoiseuille matrix:\n{}\n", mat.to_string());
		f_stdout << fmt::format("\nvector b:\n{}\n", b.to_string());
		f_stdout.close();
	}

	{// fidelity writing
		std::ofstream f_fidelity(fidelity_savename);
		if (!f_fidelity.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_fidelity.close();
	}

	//getchar();
	auto start = std::chrono::high_resolution_clock::now();
	for (size_t n = 0; n < steps; n++)
	{
		double s = double(n) / steps;
		auto walk = Walk_s_via_QRAM_Debug(&qram_A, &qram_b, mat, b, 
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
			s, kappa, p, false, data_size, rational_size);
		walk(state);
		ClearZero()(state);

		CheckNormalization(1e-10)(state);
		if ((n + 1) % 2 == 0)
		{
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
			std::vector<double> ideal_state = walk.get_mid_eigenstate();

			double fidelity = get_fidelity(ideal_state, mid_state.first);
			

			{// stdout writing
				std::ofstream f_stdout(stdout_savename, std::ios::app);
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
				std::ofstream f_fidelity(fidelity_savename, std::ios::app);
				if (!f_fidelity.is_open()) {
					throw std::runtime_error("Failed to open file.");
				}
				f_fidelity << fmt::format("step: {} / {}, fidelity: {}, p_success: {}, max_system_size: {}\n",
					n, steps, fidelity, mid_state.second, System::max_system_size);
				f_fidelity.close();
			}

			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);
		}
		else {
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}\n", now, n, steps, System::max_system_size);
			//fmt::print("state after walk = {}\n", mid_state.first);
		}
		
	}

	auto walk = Walk_s_via_QRAM_Debug(&qram_A, &qram_b, mat, b,
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
		1.0, kappa, p, false, data_size, rational_size);
	std::vector<double> ideal_state = walk.get_mid_eigenstate();
	fmt::print("{}\n", ideal_state);

	double prob_inv0 = PartialTraceSelect({ anc_UA, anc_2, anc_3 }, { 0, 0, 0 })(state);
	StatePrint(0, 10)(state);
	auto final_result = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1")(state);
	double fidelity = get_fidelity(ideal_state, final_result.first);
	auto end_w = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration_w = end_w - start;


	// wirte final result to file:
	{
		std::ofstream f_stdout(stdout_savename, std::ios_base::app);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file for appending.");
		}
		f_stdout << fmt::format("\nfidelity after walk sequence: {}\n", fidelity);
		f_stdout << fmt::format("\n===================== Walk Time =====================\n");
		f_stdout << fmt::format("Walk duration: {} mins\n", duration_w.count() / 60);
		f_stdout << fmt::format("===================== Walk Time =====================\n");
		f_stdout << fmt::format("{}\n", profiler::get_all_profiles_v2());

		f_stdout.close();
	}

	print_state_to_file(state, state_savename, 16);

	return 0;
}