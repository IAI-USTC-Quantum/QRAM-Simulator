//#include "./QDA_Poiseuille/PoiseuilleTestUtils.h"
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
#include "DiscreteAdiabatic/cuda/qda_fundamental.cuh"

using namespace qram_simulator;
using namespace QDA;
using namespace QDA::QDA_via_QRAM;
using namespace block_encoding;

int test_error_bound(size_t nqubit, double steps, double p, double alpha, double beta)
{
	int exponent = 15;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix<double> mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);

	using QRAMCircuit = qram_qutrit::CuQRAMCircuit;
	QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	DenseVector<double> b = ones<double>(pow2(nqubit));
	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	// set filename for data saving:
	//std::filesystem::path current_dir = std::filesystem::current_path();
	std::filesystem::path current_dir = ".\\error_bound_test";
	std::string sign;
	if (beta > 0)
		sign = "pos";
	else
		sign = "neg";
	std::string _savename = fmt::format("_steps{}_p{:.2f}_nq{}_QRAM_{}.txt",
		steps, p, nqubit, sign);

	std::filesystem::path stdout_savepath = current_dir / ("stdout" + _savename);
	std::filesystem::path state_savepath = current_dir / ("state" + _savename);
	std::filesystem::path fidelity_savepath = current_dir / ("fidelity" + _savename);
	std::filesystem::path dump_savepath = current_dir / ("dump" + _savename);
	std::string stdout_savename = stdout_savepath.string();
	std::string state_savename = state_savepath.string();
	std::string fidelity_savename = fidelity_savepath.string();
	std::string dump_savename = dump_savepath.string();
	//QModule::set_dumpfile(dump_savename);

	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);
	// do walk sequence or directly prepare the eigenstate of H_1.
	// To set initial state as |1>_{anc_1}|b>
	/*std::vector<System> state;
	state.emplace_back();*/
	SparseState state;

	Hadamard_Int(main_reg, System::size_of(main_reg))(state);
	//Xgate_Bool(anc_1, 0)(state);

	double kappa = get_kappa_Tridiagonal(alpha, beta, pow2(nqubit));
	fmt::print("kappa = {}\n", kappa);

	constexpr int StepConstant = 2305;
	/*size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) {
		steps += 1;
	}*/

	{
		std::ofstream f_stdout(stdout_savename);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_stdout << fmt::format("\nDiscrete Adiabatic Test for Poiseuille Flow\n");
		f_stdout << fmt::format("\nnqubit:{}\n", nqubit);
		f_stdout << fmt::format("\nkappa:{}\n", kappa);
		f_stdout << fmt::format("\nsteps = {}, p = {}\n", steps, p);
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

		//CheckNormalization(1e-10)(state);
		if ((n + 1) % 2 == 0)
		{
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1", false)(state);
			std::vector<double> ideal_state = walk.get_mid_eigenstate();

			double fidelity = get_fidelity(ideal_state, mid_state.first);


			//{// stdout writing
			//	std::ofstream f_stdout(stdout_savename, std::ios::app);
			//	if (!f_stdout.is_open()) {
			//		throw std::runtime_error("Failed to open file.");
			//	}
			//	f_stdout << fmt::format("step: {} / {}, fidelity: {}, p_success: {}\n", n, steps, fidelity, mid_state.second);
			//	f_stdout << fmt::format("Maximum Qubit Count = {}\n", System::max_qubit_count);
			//	f_stdout << fmt::format("Maximum Register Count = {}\n", System::max_register_count);
			//	f_stdout << fmt::format("Maximum System Size = {}\n\n", System::max_system_size);
			//	f_stdout.close();
			//}

			{// fidelity writing
				std::ofstream f_fidelity(fidelity_savename, std::ios::app);
				if (!f_fidelity.is_open()) {
					throw std::runtime_error("Failed to open file.");
				}
				f_fidelity << fmt::format("step: {} / {}, fidelity: {}, p_success: {}, max_system_size: {}, max_qubit_count: {}\n",
					n, steps, fidelity, mid_state.second, System::max_system_size, System::max_qubit_count);
				f_fidelity.close();
			}

			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}, max_qubit_count: {}\n",
				now, n, steps, System::max_system_size, System::max_qubit_count);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);
		}
		else {
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1", false)(state);
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}, max_qubit_count: {}\n",
				now, n, steps, System::max_system_size, System::max_qubit_count);
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
	auto final_result = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1", false)(state);
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

	return 0;
}

int random_matrix_error_bound(size_t nqubit, double steps, double p, double kappa, size_t seed, bool is_PD)
{
	int exponent = 15;
	size_t data_size = 50;
	size_t rational_size = 51;
	DenseMatrix<double> mat(pow2(nqubit));
	random_engine::set_seed(seed);
	if (is_PD)
		mat = generate_specified_kappa_mat_symmetric(pow2(nqubit), kappa);
	else
		mat = generate_specified_kappa_mat_asymmetric(pow2(nqubit), kappa);
	mat = mat / mat.normF();

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);

	using QRAMCircuit = qram_qutrit::CuQRAMCircuit;
	QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	DenseVector<double> b = ones<double>(pow2(nqubit));
	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	// set filename for data saving:
	std::filesystem::path current_dir = std::filesystem::current_path();
	fmt::print("current_dir = {}\n", current_dir.string());
	//std::filesystem::path current_dir = "E:\\SparQ-DA-QLSS\\results\\error_bound1";
	std::string is_sym;
	if (is_PD)
		is_sym = "PD";
	else
		is_sym = "nonHerm";
	std::string _savename = fmt::format("_steps{}_p{:.2f}_nq{}_kappa{}_QRAM_{}_seed{}.txt",
		steps, p, nqubit, kappa, is_sym, seed);
	std::filesystem::path stdout_savepath = current_dir / ("stdout" + _savename);
	std::filesystem::path state_savepath = current_dir / ("state" + _savename);
	std::filesystem::path fidelity_savepath = current_dir / ("fidelity" + _savename);
	std::filesystem::path dump_savepath = current_dir / ("dump" + _savename);
	std::string stdout_savename = stdout_savepath.string();
	std::string state_savename = state_savepath.string();
	std::string fidelity_savename = fidelity_savepath.string();
	std::string dump_savename = dump_savepath.string();
	fmt::print("\nfilename for fidelity saving: {}\n", fidelity_savename);
	fmt::print("\nfilename for std output: {}\n", stdout_savename);
	fmt::print("\nfilename for state saving: {}\n", state_savename);
	//fmt::print("\nfilename for dump saving: {}\n", dump_savename);
	//QModule::set_dumpfile(dump_savename);

	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);
	// do walk sequence or directly prepare the eigenstate of H_1.
	// To set initial state as |0>_{anc_1}|b>
	CuSparseState state;

	Hadamard_Int(main_reg, System::size_of(main_reg))(state);

	double _kappa = get_kappa_general(mat);
	fmt::print("kappa = {}\n", _kappa);

	{
		std::ofstream f_stdout(stdout_savename);
		if (!f_stdout.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		f_stdout << fmt::format("\nDiscrete Adiabatic Test for Random Matrixix\n");
		f_stdout << fmt::format("\nnqubit:{}\n", nqubit);
		f_stdout << fmt::format("\nkappa:{}\n", _kappa);
		f_stdout << fmt::format("\nsteps = {}, p = {}\n", steps, p);
		f_stdout << fmt::format("\nMatrix:\n{}\n", mat.to_string());
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
			s, kappa, p, is_PD, data_size, rational_size);
		walk(state);
		ClearZero()(state);

		//CheckNormalization(1e-10)(state);
		if ((n + 1) % 2 == 0)
		{
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1", is_PD)(state);
			std::vector<double> ideal_state = walk.get_mid_eigenstate(is_PD);

			double fidelity = get_fidelity(ideal_state, mid_state.first);


			//{// stdout writing
			//	std::ofstream f_stdout(stdout_savename, std::ios::app);
			//	if (!f_stdout.is_open()) {
			//		throw std::runtime_error("Failed to open file.");
			//	}
			//	f_stdout << fmt::format("step: {} / {}, fidelity: {}, p_success: {}\n", n, steps, fidelity, mid_state.second);
			//	f_stdout << fmt::format("Maximum Qubit Count = {}\n", System::max_qubit_count);
			//	f_stdout << fmt::format("Maximum Register Count = {}\n", System::max_register_count);
			//	f_stdout << fmt::format("Maximum System Size = {}\n\n", System::max_system_size);
			//	f_stdout.close();
			//}

			{// fidelity writing
				std::ofstream f_fidelity(fidelity_savename, std::ios::app);
				if (!f_fidelity.is_open()) {
					throw std::runtime_error("Failed to open file.");
				}
				f_fidelity << fmt::format("step: {} / {}, fidelity: {}, p_success: {}, max_system_size: {}, max_qubit_count: {}\n",
					n, steps, fidelity, mid_state.second, System::max_system_size, System::max_qubit_count);
				f_fidelity.close();
			}

			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}, max_qubit_count: {}\n",
				now, n, steps, System::max_system_size, System::max_qubit_count);
			//fmt::print("state after walk = {}\n", mid_state.first);
			fmt::print("fidelity = {} \n", fidelity);
		}
		else {
			auto mid_state = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1", is_PD)(state);
			auto now = std::chrono::system_clock::now();
			fmt::print("{:%Y-%m-%d %H:%M:%S} | step: {:>7} / {:>7}, max_system_size: {}, max_qubit_count: {}\n",
				now, n, steps, System::max_system_size, System::max_qubit_count);
			//fmt::print("state after walk = {}\n", mid_state.first);
		}

	}

	auto walk = Walk_s_via_QRAM_Debug(&qram_A, &qram_b, mat, b,
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
		1.0, kappa, p, is_PD, data_size, rational_size);
	std::vector<double> ideal_state = walk.get_mid_eigenstate(is_PD);
	fmt::print("{}\n", ideal_state);

	double prob_inv0 = PartialTraceSelect({ anc_UA, anc_2, anc_3 }, { 0, 0, 0 })(state);
	StatePrint(0, 10)(state);
	auto final_result = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2", "anc_1", is_PD)(state);
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

	return 0;
}

struct TestArguments
{
	size_t steps;
	double p;
	size_t nqubit;
	double kappa;
	double alpha;
	double beta;
	size_t seed;
	bool use_qram;
	bool is_PD;
};

inline TestArguments qda_argument_parse(int argc, const char** argv)
{
	TestArguments args;
	argparse::ArgumentParser parser("QDA_Test", "QDA Fidelity Test");

	parser.add_argument()
		.names({ "-s", "--steps" })
		.description("walk sequence length")
		.required(false);

	parser.add_argument()
		.name("--p")
		.description("p")
		.required(false);

	parser.add_argument()
		.name("--nqubit")
		.description("num of qubit")
		.required(false);

	parser.add_argument()
		.name("--kappa")
		.description("kappa")
		.required(false);

	parser.add_argument()
		.name("--alpha")
		.description("alpha")
		.required(false);

	parser.add_argument()
		.name("--beta")
		.description("beta")
		.required(false);

	parser.add_argument()
		.name("--seed")
		.description("seed")
		.required(false);

	parser.add_argument()
		.name("--useqram")
		.description("use qram or not")
		.required(false);

	parser.add_argument()
		.name("--ispd")
		.description("is symmetric or not")
		.required(false);

	parser.enable_help();
	auto err = parser.parse(argc, argv);

	if (err) {
		std::cout << err << std::endl;
		std::exit(2);
		return args;
	}

	if (parser.exists("help")) {
		parser.print_help();
		// return args;
		exit(0);
	}

	if (parser.exists("steps"))
	{
		args.steps = parser.get<size_t>("steps");
	}
	else {
		args.steps = 1;
	}

	if (parser.exists("p"))
	{
		args.p = parser.get<double>("p");
	}
	else {
		args.p = 1.3;
	}

	if (parser.exists("nqubit"))
	{
		args.nqubit = parser.get<size_t>("nqubit");
	}
	else {
		args.nqubit = 3;
	}

	if (parser.exists("kappa"))
	{
		args.kappa = parser.get<double>("kappa");
	}
	else {
		args.kappa = 10;
	}

	if (parser.exists("alpha"))
	{
		args.alpha = parser.get<double>("alpha");
	}
	else {
		args.alpha = 2.0;
	}

	if (parser.exists("beta"))
	{
		args.beta = parser.get<double>("beta");
	}
	else {
		args.beta = 1.0;
	}

	if (parser.exists("seed"))
	{
		args.seed = parser.get<size_t>("seed");
	}
	else {
		args.seed = 256;
	}

	if (parser.exists("useqram"))
	{
		args.use_qram = parser.get<bool>("useqram");
	}
	else {
		args.use_qram = false;
	}

	if (parser.exists("ispd"))
	{
		args.is_PD = parser.get<bool>("ispd");
	}
	else {
		args.is_PD = false;
	}

	return args;
}


int main(int argc, const char** argv)
{
	TestArguments args;

	if (argc == 1)
	{
		const char* test_argv[] = {
			".",
			"--steps", "1000",
			"--p", "1.3",
			"--nqubit", "5",
			"--kappa", "50",
			"--alpha", "2.0",
			"--beta", "-1.0",
			"--seed", "256",
			"--useqram", "1",
			//"--ispd", "1"
		};
		argc = array_length<decltype(test_argv)>::value;
		argv = test_argv;
	}

	args = qda_argument_parse(argc, argv);

	//test_error_bound(args.nqubit, args.steps, args.p, args.alpha, args.beta);
	random_matrix_error_bound(args.nqubit, args.steps, args.p, args.kappa, args.seed, args.is_PD);

}