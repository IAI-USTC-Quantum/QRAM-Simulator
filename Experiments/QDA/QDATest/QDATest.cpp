#include "argparse.h"
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
#include "DiscreteAdiabatic/qda_via_QRAM.h"
#include "BlockEncoding/make_qram.h"

using namespace qram_simulator;
using namespace QDA;
using namespace QDA::QDA_via_QRAM;
using namespace block_encoding;


inline DenseMatrix<double> generate_test_mat(size_t row_size)
{
	std::vector<double> matrix_A = { 1.0,  2.0,  3.0,  4.0,
										2.0,  1.0,  4.0,  5.0,
										3.0,  4.0,  1.0,  6.0,
										4.0,  5.0,  6.0,  1.0 };
	DenseMatrix<double> m(row_size, matrix_A);
	return m;
}

inline DenseVector<double> generate_test_vec(size_t row_size)
{
	std::vector<double> data_b = { 3.0, 4.5, 11.8, 0.2 };
	DenseVector<double> b(row_size, data_b);
	return b;
}

struct QDATestArguments
{
	double step_rate;
	double filter_eps;
	double p;
	size_t nqubit;
	size_t seed;
	bool use_filter;
	bool only_filter;
	double specified_kappa;
	int sparsity_number;
	bool is_Hermitian;
	std::string filename;
	std::string fprefix;
};

inline QDATestArguments qda_argument_parse(int argc, const char** argv)
{
	QDATestArguments args;
	argparse::ArgumentParser parser("QDA_Test", "QDA Fidelity Test");

	parser.add_argument()
		.names({ "-s", "--steprate" })
		.description("walk sequence length")
		.required(false);

	parser.add_argument()
		.name("--filterEps")
		.description("filterEps")
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
		.name("--seed")
		.description("random seed")
		.required(false);

	parser.add_argument()
		.name("--onlyFilter")
		.description("If only do filtering part.")
		.required(false);

	parser.add_argument()
		.name("--isHermitian")
		.description("Hermitian check.")
		.required(false);

	parser.add_argument()
		.name("--kappa")
		.description("Specify condition number.")
		.required(false);

	parser.add_argument()
		.name("--sparsity")
		.description("Sparsity number / Sparsity constant.")
		.required(false);

	parser.add_argument()
		.name("--filename")
		.description("saved data name")
		.required(false);

	parser.add_argument()
		.name("--fprefix")
		.description("prefix of saving name")
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

	if (parser.exists("onlyFilter"))
	{
		args.only_filter = parser.get<bool>("onlyFilter");
	}
	else {
		args.only_filter = false;
	}

	args.use_filter = false;

	if (args.only_filter) {
		if (parser.exists("filterEps"))
		{
			args.filter_eps = parser.get<double>("filterEps");
			if (args.filter_eps <= 0 && args.filter_eps > 0.5)
			{
				fmt::print("\nThe `filterEps` argument is invalid!\n");
				std::exit(2);
			}
		}
		else {
			args.filter_eps = 1e-2;
		}
	}
	else {
		args.filter_eps = -1.0;
	}

	if (parser.exists("steprate"))
		args.step_rate = parser.get<double>("steprate");
	else {
		args.step_rate = 0.001;
	}

	if (parser.exists("p"))
		args.p = parser.get<double>("p");
	else {
		args.p = 1.3;
	}

	if (parser.exists("seed")) {
		args.seed = parser.get<size_t>("seed");
	}
	else {
		args.seed = 1001;
	}

	if (parser.exists("nqubit"))
		args.nqubit = parser.get<size_t>("nqubit");
	else {
		args.nqubit = 3;
	}

	if (parser.exists("isHermitian"))
		args.is_Hermitian = parser.get<bool>("isHermitian");
	else {
		args.is_Hermitian = true;
	}

	if (parser.exists("kappa"))
		args.specified_kappa = parser.get<double>("kappa");
	else {
		args.specified_kappa = -1.0;
	}

	if (parser.exists("sparsity"))
		args.sparsity_number = parser.get<int>("sparsity");
	else {
		args.sparsity_number = -1;
	}

	if (args.only_filter)
	{
		if (parser.exists("filename"))
		{
			args.filename = parser.get<std::string>("filename");
			std::string savepath = std::filesystem::current_path().string() + "\\" + args.filename;
			if (!std::filesystem::exists(savepath))
			{
				fmt::print("File path for loading quantum state `{}` does not exist. "
					"Please check working directory!\n", savepath);
				std::exit(5);
			}
		}
		else
		{
			fmt::print("Missing `filename` argument input!\n");
			std::exit(2);
		}
		args.fprefix = "";
	}
	else {
		if (parser.exists("fprefix"))
		{
			args.fprefix = parser.get<std::string>("fprefix");
		}
		else {
			args.fprefix = "";
		}
		args.filename = ".";
	}

	return args;
}

QDATestArguments read_arguments_from_file(std::string filename)
{
	QDATestArguments args;
	std::regex sr_pattern(R"(_sr([0-9]*\.?[0-9]+))");
	std::regex p_pattern(R"(_p([0-9]*\.?[0-9]+))");
	std::regex nq_pattern(R"(_nq([0-9]+))");

	std::smatch sr_matches;
	std::smatch p_matches;
	std::smatch nq_matches;

	if (std::regex_search(filename, sr_matches, sr_pattern)) {
		args.step_rate = std::stod(sr_matches[1].str());
	}

	if (std::regex_search(filename, p_matches, p_pattern)) {
		args.p = std::stod(p_matches[1].str());
	}

	if (std::regex_search(filename, nq_matches, nq_pattern)) {
		args.nqubit = std::stoi(nq_matches[1].str());
	}
	args.use_filter = true;
	return args;
}

void QDAWalkTest(double step_rate,
	double p, size_t nqubit, 
	double specified_kappa, 
	int sparsity_number, 
	bool is_Hermitian,
	std::string fprefix,
	std::optional<size_t> seed = std::nullopt)
{
	// step 0: data initialization.
	auto start = std::chrono::high_resolution_clock::now();
	std::filesystem::path current_dir = std::filesystem::current_path();

	if (!seed.has_value())
	{
		random_engine::time_seed();
		random_engine::set_seed(1001);
	}
	else
	{
		random_engine::get_instance().set_seed(seed.value());
	}
	auto save_seed = random_engine::get_instance().get_seed();

	int exponent = 15;
	size_t log_column_size = nqubit;
	int m_size = pow2(log_column_size);
	DenseMatrix<double> mat(m_size);

	if (specified_kappa > 0 && is_Hermitian)
	{
		mat = generate_specified_kappa_mat_symmetric<double>(m_size, specified_kappa);
	}
	else if (specified_kappa > 0 && !is_Hermitian) {
		mat = generate_specified_kappa_mat_asymmetric<double>(m_size, specified_kappa);
	}
	else if (specified_kappa <= 0 && is_Hermitian) {
		//mat = generate_band_mat_signed<double>(m_size, sparsity_number);
		double _alpha = 2.0;
		double _beta =  1.0;
		//std::exit(3);
		mat = generate_Poiseuille_mat<double>(m_size, _alpha, _beta);
	}
	else if (specified_kappa <= 0 && !is_Hermitian) {
		mat = generate_band_mat_asymmetric<double>(m_size, sparsity_number);
	}
	else {
		fmt::print("The specified_kappa or is_Hermitian is invalid!\n");
		throw_invalid_input();
	}
	
	mat = mat / mat.normF();
	//DenseVector<double> b = randvec<double>(m_size);
	DenseVector<double> b = ones<double>(m_size);

	fmt::print("mat: {}", mat.to_string());
	double kappa;
	// kappa = 2300;
	if (nqubit > 7)
	{
		kappa = specified_kappa;
	}
	else {
		kappa = get_kappa_general(mat);
	}
	
	fmt::print("kappa: {}\n", kappa);
	//getchar();
	//std::exit(11);
	if (specified_kappa > 0 && abs(kappa - specified_kappa) > 1e-10 ) {
		fmt::print("Invalid specified condition number!\n");
		throw_invalid_input();
	}
	
	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) {
		steps += 1;
	}
	DenseVector<double> res = my_linear_solver(mat, b);
	res = res / (res.norm2());
	std::vector<double> theoretical_res = get_output(res.data, b.size);


	size_t data_size = 50;
	size_t rational_size = 51;

	// set filename for data saving:
	std::string _savename = fmt::format("_sr{:.5f}_p{:.2f}_nq{}_seed{}.txt", 
				step_rate, p, log_column_size, save_seed);
	// fmt::print("\n========= _savename: {}==========\n", _savename);
	std::string stdout_savename = current_dir.string() + "\\stdout" + fprefix + _savename;
	std::string state_savename = current_dir.string() + "\\state" + fprefix + _savename;

	// step 2: implement QRAM for data loading.
	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);

	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);

	// print:
	{
		std::ofstream file_stdout(stdout_savename);
		if (!file_stdout.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}
		file_stdout << fmt::format("\nDiscrete Adiabatic Test\n");
		file_stdout << fmt::format("\nseed:{}\n", save_seed);
		file_stdout << fmt::format("\nnqubit:{}\n", nqubit);
		file_stdout << fmt::format("\nkappa:{}\n", kappa);
		file_stdout << fmt::format("\nstep_rate = {}, steps = {}\n", step_rate, steps);
		file_stdout << fmt::format("\np:{}\n", p);
		file_stdout << fmt::format("\nspecified_kappa:{}\n", specified_kappa);
		file_stdout << fmt::format("\nsparsity_number:{}\n", sparsity_number);
		file_stdout << fmt::format("\nis_Hermitian:{}\n", is_Hermitian);
		file_stdout << fmt::format("\nrandom matrix:\n{}\n", mat.to_string());
		file_stdout << fmt::format("\nrandom vector:\n{}\n", b.to_string());
		file_stdout << fmt::format("\nideal result: {}\n", theoretical_res);
		file_stdout << fmt::format("\ndata_tree_A:{}\n", data_tree_A);
		file_stdout << fmt::format("\ndata_tree_b:{}\n", data_tree_b);
		file_stdout.close();
	}

	size_t addr_size = log_column_size * 2 + 1;

	qram_qutrit::QRAMCircuit qram_A(addr_size, data_size);
	qram_A.set_memory(data_tree_A);
	qram_qutrit::QRAMCircuit qram_b(log_column_size + 1, data_size);
	qram_b.set_memory(data_tree_b);

	// step 3: add registers.
	std::vector<System> state;
	state.emplace_back();

	auto main_reg = AddRegister("main_reg", UnsignedInteger, log_column_size)(state);
	auto anc_UA = AddRegister("anc_UA", UnsignedInteger, log_column_size)(state);
	auto anc_4 = AddRegister("anc_4", Boolean, 1)(state);
	auto anc_3 = AddRegister("anc_3", Boolean, 1)(state);
	auto anc_2 = AddRegister("anc_2", Boolean, 1)(state);
	auto anc_1 = AddRegister("anc_1", Boolean, 1)(state);

	// step 4: do walk sequence or directly prepare the eigenstate of H_1.
	// To set initial state as |1>_{anc_1}|b>
	//State_Prep(&qram_b, "main_reg", data_size, rational_size)(state);
	(Hadamard_Int_Full(main_reg))(state);
	//Xgate_Bool(anc_1, 0)(state);

	WalkSequence_via_QRAM_Debug(&qram_A, &qram_b, mat, b, 
		"main_reg", "anc_UA",
		"anc_1", "anc_2", "anc_3", "anc_4",
		steps, kappa, p, data_size, rational_size, stdout_savename)(state);

	double prob_inv0 = PartialTraceSelect({ anc_UA, anc_2, anc_3 }, { 0, 0, 0 })(state);
	double prob0 = (1.0 / prob_inv0) * (1.0 / prob_inv0);

	auto output = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2",
		"anc_1");
	auto output_walked = output(state);
	
	double fidelity_before = get_fidelity(theoretical_res, output_walked.first);

	auto end_w = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration_w = end_w - start;


	// wirte to file:
	{
		std::ofstream file_stdout(stdout_savename, std::ios_base::app);
		if (!file_stdout.is_open()) {
			throw std::runtime_error("Failed to open file for appending.");
		}
		file_stdout << fmt::format("\nfidelity after walk sequence: {}\n", fidelity_before);
		file_stdout << fmt::format("\nSuccess probability after walk sequence: {}\n", prob0);
		file_stdout << fmt::format("\n===================== Walk Time =====================\n");
		file_stdout << fmt::format("Walk duration: {} mins\n", duration_w.count() / 60);
		file_stdout << fmt::format("===================== Walk Time =====================\n");
		file_stdout << fmt::format("{}\n", profiler::get_all_profiles_v2());
		file_stdout.close();
	}

	print_state_to_file(state, state_savename, 16);
}

void QDAFilteringTest(double filter_eps, std::string filename, 
	double specified_kappa, int sparsity_number, bool is_Hermitian,
	std::optional<size_t> seed = std::nullopt)
{
	auto start = std::chrono::high_resolution_clock::now();
	std::filesystem::path current_dir = std::filesystem::current_path();

	if (!seed.has_value())
	{
		random_engine::time_seed();
		random_engine::set_seed(1001);
	}
	else
	{
		random_engine::get_instance().set_seed(seed.value());
	}
	auto save_seed = random_engine::get_instance().get_seed();

	QDATestArguments args = read_arguments_from_file(filename);
	
	int exponent = 15;
	size_t log_column_size = args.nqubit;
	int m_size = pow2(log_column_size);
	DenseMatrix<double> mat(m_size);

	if (specified_kappa > 0 && is_Hermitian)
	{
		mat = generate_specified_kappa_mat_symmetric<double>(m_size, specified_kappa);
	}
	else if (specified_kappa > 0 && !is_Hermitian) {
		//mat = generate_band_mat_unsigned<double>(m_size, sparsity_number);
		mat = generate_specified_kappa_mat_asymmetric<double>(m_size, specified_kappa);
	}
	else if (specified_kappa <= 0 && is_Hermitian) {
		mat = generate_band_mat_signed<double>(m_size, sparsity_number);
	}
	else if (specified_kappa <= 0 && !is_Hermitian) {
		mat = generate_band_mat_asymmetric<double>(m_size, sparsity_number);
	}
	else {
		fmt::print("The specified_kappa or is_Hermitian is invalid!\n");
		throw_invalid_input();
	}

	mat = mat / mat.normF();
	DenseVector<double> b = randvec<double>(m_size);
	double kappa;
	kappa = get_kappa_general(mat);
	if (specified_kappa > 0 && kappa != specified_kappa) {
		fmt::print("Invalid specified condition number!\n");
		throw_invalid_input();
	}
	
	DenseVector<double> res = my_linear_solver(mat, b);
	res = res / (res.norm2());
	std::vector<double> theoretical_res = get_output(res.data, b.size);

	size_t data_size = 50;
	size_t rational_size = 51;

	// set filename for data saving:
	std::string _savename_eps = fmt::format("_sr{:.5f}_p{:.2f}_nq{}_seed{}_eps{}.txt", 
				args.step_rate, args.p, args.nqubit, save_seed, filter_eps);
	std::string _savename_ = fmt::format("_sr{:.5f}_p{:.2f}_nq{}_seed{}.txt",
				args.step_rate, args.p, args.nqubit, save_seed);
	std::string stdout_savename = current_dir.string() + "\\stdout" + _savename_eps;
	std::string filtering_savename = current_dir.string() + "\\filtering" + _savename_;
		
	// step 2: implement QRAM for data loading.
	std::vector<size_t> conv_A = scaleAndConvertVector(mat.data, exponent, data_size);
	std::vector<size_t> conv_b = scaleAndConvertVector(b.data, exponent, data_size, false);

	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);


	size_t addr_size = log_column_size * 2 + 1;

	qram_qutrit::QRAMCircuit qram_A(addr_size, data_size);
	qram_A.set_memory(data_tree_A);
	qram_qutrit::QRAMCircuit qram_b(log_column_size + 1, data_size);
	qram_b.set_memory(data_tree_b);

	// step 3: add registers.
	std::vector<System> state;
	state.emplace_back();
	
	int main_reg = AddRegister("main_reg", UnsignedInteger, log_column_size)(state);
	int anc_UA = AddRegister("anc_UA", UnsignedInteger, log_column_size)(state);
	int anc_4 = AddRegister("anc_4", Boolean, 1)(state);
	int anc_3 = AddRegister("anc_3", Boolean, 1)(state);
	int anc_2 = AddRegister("anc_2", Boolean, 1)(state);
	int anc_1 = AddRegister("anc_1", Boolean, 1)(state);
	
	auto output = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2",
		"anc_1");

	// step 4: state load.
	std::string state_savename = current_dir.string() + "\\" + filename;
	state = StateLoad("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2",
		"anc_1", data_size, rational_size)(state_savename);
	
	// step 5: LCU implementation for filtering.

	double epsilon_ = filter_eps;
	int l_ = static_cast<int>(std::floor(kappa * log(2.0 / epsilon_)));
	l_ = l_ % 2 == 0 ? l_ : l_ - 1;

	fmt::print("\nseed: {}, nqubit: {}, kappa: {}, l_: {}\n", save_seed, log_column_size, kappa, l_);
	// step 5.1: determine the index register size and the weights.
	std::vector<double> weights = ComputeFourierCoeffs(epsilon_, l_);
	std::vector<double> weights_sqrt(weights.size());
	for (int i = 0; i < weights.size(); i++) weights_sqrt[i] = sqrt(weights[i]);
	int num = weights.size();
	int index_size = static_cast<int>(std::ceil(log2(double(num))));
	weights_sqrt.resize(pow2(index_size), 0.0);

	std::vector<size_t> conv_w = scaleAndConvertVector(weights_sqrt, exponent, data_size, false);
	memory_t data_tree_w = make_vector_tree(conv_w, data_size);
	qram_qutrit::QRAMCircuit qram_w(index_size + 1, data_size);
	qram_w.set_memory(data_tree_w);

	int index = AddRegister("index", UnsignedInteger, index_size)(state);
	int anc_h = AddRegister("anc_h", UnsignedInteger, 1)(state);

	Walk_s_via_QRAM_Debug Walk_1 = Walk_s_via_QRAM_Debug(&qram_A, &qram_b, mat, b, 
		"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
		1.0, kappa, args.p, false, data_size, rational_size);
		
	Walk_1.conditioned_by_all_ones(anc_h);

	auto filter = Filtering<Walk_s_via_QRAM_Debug>(&qram_w, Walk_1, 
		"main_reg", "anc_UA", "anc_4", "anc_3", "anc_2",
		"anc_1", "index", "anc_h", data_size, rational_size, stdout_savename);
	double prob1 = filter(state);

	auto output_filtered = GetOutput("main_reg", "anc_UA", "anc_4", "anc_3", "anc_2",
		"anc_1", "index", "anc_h")(state);

	double fidelity_after = get_fidelity(theoretical_res, output_filtered.first);
	fmt::print("\neps: {}, fidelity: {}, p_success: {}\n", filter_eps, fidelity_after, prob1);


	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	{
		try {
			std::ofstream file_stdout(stdout_savename, std::ios_base::app);
			if (!file_stdout.is_open()) {
				throw std::runtime_error("Failed to open file for appending.");
			}
			file_stdout << fmt::format("\nStart Filtering with eps: {}\n", filter_eps);
			file_stdout << fmt::format("\nseed: {}, nqubit: {}, kappa: {}, l_: {}\n", save_seed, log_column_size, kappa, l_);
			file_stdout << fmt::format("\nfidelity after filtering: {}\n", fidelity_after);
			file_stdout << fmt::format("\nSuccess probability after filtering: {}\n", prob1);
			file_stdout << fmt::format("\n===================== Filtering Time =====================\n");
			file_stdout << fmt::format("Filtering duration: {} mins\n", duration.count() / 60);
			file_stdout << fmt::format("===================== Filtering Time =====================\n");
			file_stdout << fmt::format("{}\n", profiler::get_all_profiles_v2());
			file_stdout.close();
		} catch (const std::runtime_error& e) {
			fmt::print("Error: {}\n", e.what());
			return;
		}
	}
	{
		try {
			std::ofstream file_filtering(filtering_savename, std::ios_base::app);
			if (!file_filtering.is_open()) {
				throw std::runtime_error("Failed to open file for appending.");
			}
			file_filtering << fmt::format("eps: {}, fidelity: {}, p_success: {}\n", filter_eps, fidelity_after, prob1);
			file_filtering.close();

		} catch (const std::runtime_error& e) {
			fmt::print("Error: {}\n", e.what());
			return;
		}
	}
}


int main(int argc, const char** argv) {

	omp_set_num_threads(8);
	//QModule::set_dumpfile("QDAdump.txt");
	QDATestArguments args;
	if (argc == 1)
	{
		const char* test_argv[] = {
			".",
			"--steprate", "0.01",
			"--filterEps", "1e-1",
			"--filename", "state_sr0.00100_p1.30_nq4_seed1756823046.txt",
			"--nqubit", "4",
			"--sparsity", "4",
			"--kappa", "20",
			"--isHermitian", "0",
			"--onlyFilter", "0",
			"--seed","1756823046",
		};
		argc = array_length<decltype(test_argv)>::value;
		argv = test_argv;
	}

	args = qda_argument_parse(argc, argv);
	
	
	if (args.only_filter)
	{
		fmt::print("==================== Filtering mode ====================\n\n");
		QDAFilteringTest(args.filter_eps, args.filename, args.specified_kappa, 
							args.sparsity_number, args.is_Hermitian, args.seed);
		fmt::print("filter_eps = {}, filename = {}, only_filter = {}, specified_kappa = {}, "
			"sparsity_number = {}, is_Hermitian = {}, seed = {}\n", 
			args.filter_eps, args.filename, args.only_filter, args.specified_kappa, 
			args.sparsity_number, args.is_Hermitian, args.seed);
	}
	else {
		fmt::print("==================== Walk mode ====================\n\n");
		QDAWalkTest(args.step_rate, args.p, args.nqubit, args.specified_kappa,
			args.sparsity_number, args.is_Hermitian, args.fprefix, args.seed);
		fmt::print("step_rate = {}, p = {}, nqubit = {}, "
			"only_filter = {}, "
			"specified_kappa = {}, "
			"sparsity_number = {}, "
			"is_Hermitian = {}, "
			"fprefix = {}, "
			"seed = {}\n", 
			args.step_rate, args.p, args.nqubit, 
			args.only_filter, 
			args.specified_kappa,
			args.sparsity_number, 
			args.is_Hermitian, 
			args.fprefix,
			args.seed);
	}

	fmt::print("{}\n", profiler::get_all_profiles_v2());

	return 0;
}


