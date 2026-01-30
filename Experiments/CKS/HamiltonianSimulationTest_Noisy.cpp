#include "hamiltonian_simulation.h"
#include "matrix.h"
#include "argparse.h"
using namespace qram_simulator;
using namespace CKS;
auto automatic_Chebyshev_noise_test(size_t step, const SparseMatrix &mat, 
	std::shared_ptr<qram_qutrit::QRAMCircuit> qram)
{
	auto densemat = sparse2dense<double>(mat);
	if (!densemat.is_symmetric())
	{
		fmt::print("Not symmetric!");
		throw_invalid_input();
	}
	densemat *= (1.0 / mat.nnz_col);
	if (mat.positive_only)
		densemat *= (1.0 / (pow2(mat.data_size) - 1));
	else
		densemat *= (1.0 / (pow2(mat.data_size - 1) - 1));

	auto vec = ones<double>(mat.n_row);

	QuantumWalkNSteps quantum_walk_obj(mat, qram.get());

	quantum_walk_obj.InitEnvironment();

	auto state = quantum_walk_obj.MakeNStepState(step);

	auto target_vec = chebyshev_n(step, densemat, vec);
	target_vec = target_vec / target_vec.norm2();

	double success_prob = 0;
	double fidelity = 0;

	auto ret = PartialTraceSelect( {	
			{quantum_walk_obj.b1, 0},
			{quantum_walk_obj.k, 0},
			{quantum_walk_obj.b2, 0},
			{quantum_walk_obj.j_comp, 0},
			{quantum_walk_obj.k_comp, 0},
			{quantum_walk_obj.sparse_offset, quantum_walk_obj.offset},
			{quantum_walk_obj.data_offset, 0}
		})(state);

	auto ret2 = PartialTraceSelectRange(
		quantum_walk_obj.j,
		{ 0, quantum_walk_obj.n_row - 1 })(state);

	if (std::isinf(ret) || std::isinf(ret2)) {
		success_prob = 0;
		fidelity = 0;
	}
	else {
		success_prob = ret * ret2;
		success_prob = 1.0 / success_prob / success_prob;

		std::vector<complex_t> m(mat.n_row, 0);
		auto id = System::get(quantum_walk_obj.j);
		for (auto& s : state)
		{
			m[s.GetAs(id, uint64_t)] = s.amplitude;
		}
		auto target_result = target_vec.to_vec<complex_t>();
		fidelity = get_fidelity(m, target_result);
	}
	if (std::isnan(fidelity) || std::isnan(success_prob))
		throw_bad_result();
	return std::pair<double, double>(fidelity, success_prob);
}

auto automatic_Chebyshev_noise_test_1shots(const SparseMatrix& mat,
	std::shared_ptr<qram_qutrit::QRAMCircuit> qram,
	size_t steps, noise_t noise_model, std::optional<seed_t> seed)
{
	if (!seed.has_value())
		seed = random_engine::get_instance().reseed();
	else
		random_engine::get_instance().set_seed(seed.value());

	qram->set_noise_models(noise_model);
	auto&& [fidelity, prob] = automatic_Chebyshev_noise_test(steps, mat, qram);
	return std::tuple<double, double, seed_t>(fidelity, prob, seed.value());
}

auto automatic_Chebyshev_noise_test(size_t row, size_t strip_size, size_t data_size, size_t shots, size_t steps, noise_t noise_model, std::optional<seed_t> seed)
{
	if (!seed.has_value())
		random_engine::time_seed();
	else
		random_engine::get_instance().set_seed(seed.value());

	auto save_seed = random_engine::get_instance().get_seed();
	SparseMatrix mat;
	auto densemat = generate_band_mat_unsigned(row, strip_size);
	mat = dense2sparse_band_unsigned(densemat, strip_size, data_size);

	std::string hash = get_random_hash_str();
	std::string metafile = "Metadata-" + hash + ".txt";
	std::string resultfile = "Result-" + hash + ".csv";

	FILE* fp_meta = fopen(metafile.c_str(), "w+");
	FILE* fp_result = fopen(resultfile.c_str(), "w+");

	fmt::print("seed = {}\n", save_seed);
	fmt::print("nrow = {}\n", mat.n_row);
	fmt::print("nnz  = {}\n", mat.nnz_col);
	fmt::print("step = {}\n", steps);
	fmt::print("shot = {}\n", shots);
	fmt::print("noise_model = {}\n", noise2str(noise_model));
	fmt::print("qram_version = {}\n", QRAMLoad::version);
	// fmt::print("kappa = {}\n", mat.get_kappa());
	// fmt::print("j0 (eps=1e-3) = {}\n", mat.get_j0(1e-3));
	// fmt::print("j0 (eps=1e-4) = {}\n", mat.get_j0(1e-4));

	fmt::print(fp_meta, "seed = {}\n", save_seed);
	fmt::print(fp_meta, "nrow = {}\n", mat.n_row);
	fmt::print(fp_meta, "nnz  = {}\n", mat.nnz_col);
	// fmt::print(fp_meta, "kappa  = {}\n", mat.get_kappa());
	fmt::print(fp_meta, "step = {}\n", steps);
	fmt::print(fp_meta, "shot = {}\n", shots);
	fmt::print(fp_meta, "noise_model = {}\n", noise2str(noise_model));
	fmt::print(fp_meta, "qram_version = {}\n", QRAMLoad::version);

	fmt::print("{:^19s}|{:^19s}|{:^19s}|{:^19s}|{:^19s}|{:^19s}|{:^12s}\n",
		"Fidelity",
		"Successful rate",
		"Max register count",
		"Max qubit count",
		"Max state size",
		"Save seed",
		"Progress");

	fmt::print(fp_meta, "{:^19s}|{:^19s}|{:^19s}|{:^19s}|{:^19s}\n",
		"Fidelity",
		"Successful rate",
		"Max register count",
		"Max qubit count",
		"Max state size"
	);
	fmt::print(fp_result, "{},{},{},{},{},{}\n",
		"Fidelity",
		"Successful rate",
		"Max register count",
		"Max qubit count",
		"Max state size",
		"Save seed");

	fclose(fp_meta);
	fclose(fp_result);

	StatisticDouble fid;
	StatisticDouble pro;
	StatisticSize max_reg;
	StatisticSize max_qub;
	StatisticSize sys_sz;

	for (size_t i = 0; i < shots; ++i) {

		auto&& data = mat.get_data();
		size_t addr_size = log2(data.size());
		auto qram = std::make_shared<qram_qutrit::QRAMCircuit>(addr_size, data_size, std::move(data));

		auto&& [fidelity, prob, save_seed] = automatic_Chebyshev_noise_test_1shots(
			mat, qram, steps, noise_model, std::nullopt);

		if (std::isnan(fidelity) || std::isnan(prob))
		{
			fmt::print("Error occurred at seed = {} , skip and retry.\n", save_seed);
			fp_meta = fopen(metafile.c_str(), "a+");
			fmt::print(fp_meta, "Error seed = {}. Please check.\n", save_seed);
			fclose(fp_meta);
			i -= 1;
		}
		else {
			std::string progress_str = fmt::format("{} / {}", i, shots);
			fmt::print("{:^19f}|{:^19f}|{:^19d}|{:^19d}|{:^19d}|{:^19d}|{:^12s}\n",
				fid.record(fidelity),
				pro.record(prob),
				max_reg.record(System::max_register_count),
				max_qub.record(System::max_qubit_count),
				sys_sz.record(System::max_system_size),
				save_seed,
				progress_str
			);

			fp_result = fopen(resultfile.c_str(), "a+");
			fmt::print(fp_result, "{:f},{:f},{:d},{:d},{:d},{:d}\n",
				fidelity,
				prob,
				System::max_register_count,
				System::max_qubit_count,
				System::max_system_size,
				save_seed
			);
			fclose(fp_result);
		}
		System::clear();
	}

	fp_meta = fopen(metafile.c_str(), "a+");
	{
		// print avg.
		std::string infostr = fmt::format("<-- Average on {} shots -->", shots);
		fmt::print("{:^100s}\n", infostr);
		fmt::print("{:^19f}|{:^19f}|{:^19f}|{:^19f}|{:^19f}\n",
			fid.mean(),
			pro.mean(),
			max_reg.mean(),
			max_qub.mean(),
			sys_sz.mean()
		);
		fmt::print(fp_meta, "{:^100s}\n", infostr);
		fmt::print(fp_meta, "{:^19f}|{:^19f}|{:^19f}|{:^19f}|{:^19f}\n",
			fid.mean(),
			pro.mean(),
			max_reg.mean(),
			max_qub.mean(),
			sys_sz.mean()
		);
	}
	{
		// print std.
		std::string infostr = fmt::format("<-- Std on {} shots -->", shots);
		fmt::print("{:^100s}\n", infostr);
		fmt::print("{:^19f}|{:^19f}|{:^19f}|{:^19f}|{:^19f}\n",
			fid.std(),
			pro.std(),
			max_reg.std(),
			max_qub.std(),
			sys_sz.std()
		);
		fmt::print(fp_meta, "{:^100s}\n", infostr);
		fmt::print(fp_meta, "{:^19f}|{:^19f}|{:^19f}|{:^19f}|{:^19f}\n",
			fid.std(),
			pro.std(),
			max_reg.std(),
			max_qub.std(),
			sys_sz.std()
		);
	}
	fmt::print("{}\n", profiler::get_all_profiles_v2());
	fmt::print(fp_meta, "{}\n", profiler::get_all_profiles_v2());
	fclose(fp_meta);
}

auto automatic_Chebyshev_noise_test_main()
{
	constexpr size_t row = 1024;
	constexpr size_t band_size = 16;
	constexpr size_t data_size = 16;
	constexpr size_t steps = 2;
	constexpr size_t shots = 1000;
	noise_t noise_model =
	{
		{ OperationType::Depolarizing, 1e-6 },
		{ OperationType::Damping, 1e-6 }
	};
	std::optional<seed_t> seed = std::nullopt;
	automatic_Chebyshev_noise_test(row, band_size, data_size, shots, steps, noise_model, seed);
}

struct NoisyHamiltonianSimulationTestArguments
{
	bool success = false;
	size_t rowsize = 128;
	size_t bandsize = 8;
	size_t datasize = 8;
	size_t steps = 1;
	size_t shots = 100;
	double depolarizing = 1e-6;
	double damping = 1e-6;
	std::optional<seed_t> seed = std::nullopt;
	std::string version = "normal";

	inline std::map<OperationType, double> generate_noise() const
	{
		std::map<OperationType, double> noise;
		if (depolarizing > 0.0)
			noise[OperationType::Depolarizing] = depolarizing;
		if (damping > 0.0)
			noise[OperationType::Damping] = damping;

		return noise;
	}
};

auto argument_parse(int argc, const char** argv)
{
	NoisyHamiltonianSimulationTestArguments args;
	argparse::ArgumentParser parser("HamiltonianSimulationTest_Noisy", "Noisy Hamiltonian Simulation Test");
	parser.add_argument()
		.name("--rowsize")
		.description("row size (default = 1024)")
		.required(false);

	parser.add_argument()
		.name("--bandsize")
		.description("band size (default = 16)")
		.required(false);

	parser.add_argument()
		.name("--datasize")
		.description("data size (default = 8)")
		.required(false);

	parser.add_argument()
		.name("--steps")
		.description("number of walk steps (default = 1)")
		.required(false);

	parser.add_argument()
		.name("--shots")
		.description("number of shots (default = 100)")
		.required(false);

	parser.add_argument()
		.name("--seed")
		.description("random engine's seed (default = time(0))")
		.required(false);

	parser.add_argument()
		.name("--depolarizing")
		.description("depolarizing noise (default = 1e-6)")
		.required(false);

	parser.add_argument()
		.name("--damping")
		.description("damping noise (default = 1e-6)")
		.required(false);

	parser.add_argument()
		.name("--version")
		.description("choose to use normal/fast version")
		.required(false);

	//parser.add_argument()
	//	.name("--damping")
	//	.description("damping noise")
	//	.required(false);

	//parser.add_argument()
	//	.name("--architecture")
	//	.description("architecture (qutrit or qubit) (default = qutrit)")
	//	.required(false);

	//parser.add_argument()
	//	.name("--experimentname")
	//	.description("name of the experiment, added as the postfix")
	//	.required(false);

	parser.enable_help();
	auto err = parser.parse(argc, argv);
	if (err) {
		std::cout << err << std::endl;
		return args;
	}

	if (parser.exists("help")) {
		parser.print_help();
		return args;
	}

	if (parser.exists("rowsize")) {
		size_t rowsize = parser.get<size_t>("rowsize");
		if (pow2(log2(rowsize))!= rowsize || 
			rowsize < 4 ||
			rowsize > 8192) {
			fmt::print("rowsize invalid."
				"(Expect: rowsize = pow2(n) && rowsize: [4, 8192] Input:{})\n", 
				rowsize);
			return args;
		}
		else {
			args.rowsize = rowsize;
		}
	}
	
	if (parser.exists("bandsize")) {
		size_t bandsize = parser.get<size_t>("bandsize");
		if (bandsize < 1 || bandsize > args.rowsize / 2) {
			fmt::print("bandsize invalid. (Expect: [1, rowsize/2] Input:{})\n",
				bandsize);
			return args;
		}
		else {
			args.bandsize = bandsize;
		}
	}

	if (parser.exists("datasize")) {
		size_t datasize = parser.get<size_t>("datasize");
		if (datasize < 1 || datasize > 32) {
			fmt::print("bandsize invalid. (Expect: [1, 32] Input:{})\n", datasize);
			return args;
		}
		else {
			args.datasize = datasize;
		}
	}

	if (parser.exists("steps"))
		args.steps = parser.get<size_t>("steps");

	if (parser.exists("shots"))
		args.shots = parser.get<size_t>("shots");

	if (parser.exists("seed"))
		args.seed = parser.get<seed_t>("seed");

	if (parser.exists("depolarizing"))
	{
		args.depolarizing = parser.get<double>("depolarizing");
		if (args.depolarizing <= 0)
		{
			fmt::print("depolarizing invalid. Expected: >= 0. Input:{}\n", args.depolarizing);
			return args;
		}
	}

	if (parser.exists("damping"))
	{
		args.damping = parser.get<double>("damping");
		if (args.damping <= 0)
		{
			fmt::print("depolarizing invalid. Expected: >= 0. Input:{}\n", args.damping);
			return args;
		}
	}

	if (parser.exists("version"))
		args.version = parser.get<std::string>("version");

	args.success = true;
	return args;
}

int main(int argc, const char** argv)
{
#if 0
	const char* test_argv[] = {
		"executable",
		"--rowsize", "1024",
		"--steps", "2",
		"--shots", "5",
		"--bandsize", "4",
		"--depolarizing", "1e-6",
		"--version", "fast",
		//"--seed", "1670073499",
	};
	argc = array_length<decltype(test_argv)>::value;
	argv = test_argv;
#endif
	auto args = argument_parse(argc, argv);
	if (!args.success)
	{
		fmt::print("Argument error.\n");
		return 1;
	}
		
	const size_t row = args.rowsize;
	const size_t band_size = args.bandsize;
	const size_t data_size = args.datasize;
	const size_t steps = args.steps;
	const size_t shots = args.shots;
	const noise_t noise_model = args.generate_noise();
	QRAMLoad::version = args.version;
	std::optional<seed_t> seed = args.seed;
	automatic_Chebyshev_noise_test(row, band_size, data_size, shots, steps, noise_model, seed);

	return 0;
}