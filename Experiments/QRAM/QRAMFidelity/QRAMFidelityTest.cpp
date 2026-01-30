#include <exception>
#include <typeinfo>

#include "argparse.h"
#include "logger.h"
#include "state_manipulator.h"
#include "sparse_state_simulator.h"
#include "grover.h"
#include "time_step.h"

using namespace qram_simulator;


struct QRAMFidelityTestArguments
{
	int addr_sz = 0;
	int data_sz = 3;
	size_t shots = 10;
	size_t input_size = 10;
	seed_t seed = 0;
	std::string version = "normal";
	double depolarizing = 0.0;
	double damping = 0.0;

	inline std::map<OperationType, double> generate_noise() const
	{
		std::map<OperationType, double> noise;
		if (depolarizing > 0.0)
			noise[OperationType::Depolarizing] = depolarizing;
		if (damping > 0.0)
			noise[OperationType::Damping] = damping;

		return noise;
	}

	inline std::string to_string() const
	{
		if (addr_sz > 0 && data_sz > 0) {
			return fmt::format(
				"address_size={}\n"
				"data_size={}\n"
				"shots={}\n"
				"inputsz={}\n"
				"seed={}\n"
				"depolarizing={}\n"
				"damping={}\n"
				"version={}\n",
				addr_sz, data_sz, shots, input_size, seed,
				depolarizing, damping, version
			);
		}
		else {
			return "Bad arguments.";
		}
	}
};

inline QRAMFidelityTestArguments qram_fidelity_argument_parse(int argc, const char** argv)
{
	QRAMFidelityTestArguments args;
	argparse::ArgumentParser parser("QRAM_Test", "Grover Algorithm with QRAM Test");
	parser.add_argument()
		.names({ "-a", "--addrsize" })
		.description("address size")
		.required(false);

	parser.add_argument()
		.names({ "-d", "--datasize" })
		.description("data size (default = 1)")
		.required(false);

	parser.add_argument()
		.names({ "-s", "--shots" })
		.description("number of shots (default = 10000)")
		.required(false);

	parser.add_argument()
		.name("--inputsize")
		.description("input size (default = pow2(addrsize + datasize)")
		.required(false);

	parser.add_argument()
		.name("--seed")
		.description("random engine's seed (default = time(0))")
		.required(false);

	parser.add_argument()
		.name("--depolarizing")
		.description("depolarizing noise")
		.required(false);

	parser.add_argument()
		.name("--damping")
		.description("damping noise")
		.required(false);

	parser.add_argument()
		.name("--version")
		.description("choose to use new/old/fast version")
		.required(false);

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

	if (parser.exists("addrsize")) {
		int addr_size = parser.get<int>("addrsize");
		if (addr_size < 1 || addr_size > 30) {
			fmt::print("Address size invalid. (Expect:[1,20] Input:{})\n", addr_size);
			throw_invalid_input();
		}
		else {
			args.addr_sz = addr_size;
		}
	}
	else {
		args.addr_sz = 3;
	}

	if (parser.exists("datasize")) {
		int data_size = parser.get<int>("datasize");
		if (data_size < 1 || data_size > 20) {
			fmt::print("Address size invalid. (Expect:[1,20] Input:{})\n", data_size);
			return args;
		}
		else {
			args.data_sz = data_size;
		}
	}
	else {
		args.data_sz = 1;
	}
	size_t N = pow2(args.addr_sz + args.data_sz);

	if (parser.exists("inputsize"))
		args.input_size = parser.get<size_t>("inputsize");
	else {
		args.input_size = N;
	}
	if (parser.exists("shots"))
		args.shots = parser.get<size_t>("shots");
	else
		args.shots = 10000;

	if (parser.exists("seed"))
		args.seed = parser.get<seed_t>("seed");
	else
		args.seed = time(0);

	if (parser.exists("depolarizing"))
		args.depolarizing = parser.get<double>("depolarizing");

	if (parser.exists("damping"))
		args.damping = parser.get<double>("damping");

	if (parser.exists("version")) {
		auto versionstr = parser.get<std::string>("version");

		static const char* possible_versionstr[] = {
			"new", "normal",
			"old", "full",
			"fast",
			"noisefree"
		};

		auto pred = [&](const char* i) {return versionstr == i; };

		if (std::any_of(
			std::begin(possible_versionstr),
			std::end(possible_versionstr),
			pred))
			args.version = versionstr;
		else
			throw_invalid_input();
	}
	return args;
}


qram_qutrit::QRAMCircuit configure_qram(size_t addr_sz, size_t data_sz, const noise_t& noise)
{
	qram_qutrit::QRAMCircuit qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);

	return qram;
}

auto QRAMFidelityTestImpl_1shot(size_t addr_sz, size_t data_sz, QRAMInputGenerator& gen,
	const noise_t& noise, size_t input_size, std::string version, size_t addr, size_t data,
	seed_t seed)
{
	random_engine::get_instance().set_seed(seed);
	std::vector<System> state;
	gen.generate_input(state);

	/****** QRAM Initialize ******/
	qram_qutrit::QRAMCircuit qram = qram_qutrit::QRAMCircuit(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);

	{
		profiler _("**RECORD**");
		QRAMLoad::version = version;
		QRAMLoad(&qram, addr, data)(state);
	}
	QRAMLoad::version = "noisefree";
	QRAMLoad(&qram, addr, data)(state);

	complex_t fidelity = 0;
	for (auto& s : state)
	{
		auto p = std::make_pair(s.get(addr).value, s.get(data).value);
		if (std::find(gen.unique_set.begin(), gen.unique_set.end(), p)
			!= gen.unique_set.end())
		{
			fidelity += std::conj(s.amplitude) / std::sqrt(gen.input_sz);
		}
	}
	return abs_sqr(fidelity);
}

auto QRAMFidelityTestImpl(size_t addr_sz, size_t data_sz,
	seed_t seed, const noise_t& noise, size_t shots, size_t input_size, std::string version)
{
	size_t max_input_size;

	if (addr_sz + data_sz >= 32)
		max_input_size = pow2(32);
	else
		max_input_size = pow2(addr_sz + data_sz);

	input_size = std::min(input_size, max_input_size);

	System::clear();
	auto addr = System::add_register("addr", UnsignedInteger, addr_sz);
	auto data = System::add_register("data", UnsignedInteger, data_sz);

	QRAMInputGenerator gen(addr_sz, data_sz, input_size, addr, data);

	random_engine::get_instance().set_seed(seed);
	std::string hash = get_random_hash_str();
	std::string metafile = fmt::format("Metadata-{}-{}.txt", hash, version);
	std::string resultfile = fmt::format("Result-{}-{}.csv", hash, version);

	FILE* fp_meta = fopen(metafile.c_str(), "w+");
	FILE* fp_result = fopen(resultfile.c_str(), "w+");

	fmt::print("seed = {}\n", seed);
	fmt::print("addr_sz = {}\n", addr_sz);
	fmt::print("data_sz = {}\n", data_sz);
	fmt::print("input_sz = {}\n", input_size);
	fmt::print("shot = {}\n", shots);
	fmt::print("noise_model = {}\n", noise2str(noise));
	fmt::print("qram_version = {}\n", QRAMLoad::version);

	fmt::print(fp_meta, "seed = {}\n", seed);
	fmt::print(fp_meta, "addr_sz = {}\n", addr_sz);
	fmt::print(fp_meta, "data_sz  = {}\n", data_sz);
	fmt::print(fp_meta, "input_sz = {}\n", input_size);
	fmt::print(fp_meta, "shot = {}\n", shots);
	fmt::print(fp_meta, "noise_model = {}\n", noise2str(noise));
	fmt::print(fp_meta, "qram_version = {}\n", QRAMLoad::version);

	fmt::print("{:^19s}|{:^12s}\n",
		"Fidelity",
		"Progress");
	fmt::print(fp_meta, "{:^19s}|{:^12s}\n",
		"Fidelity",
		"Progress");
	fmt::print(fp_result, "{:^19s}\n",
		"Fidelity");

	fclose(fp_meta);
	fclose(fp_result);

	StatisticDouble stat_fidelity;
	for (size_t i = 0; i < shots; ++i)
	{
		random_engine::get_instance().set_seed(seed + i);
		auto runseed = random_engine::get_instance().reseed();
		auto fidelity = QRAMFidelityTestImpl_1shot(addr_sz, data_sz, gen, noise, input_size, version, addr, data, runseed);
		if (std::isnan(fidelity))
		{
			fmt::print("Error occurred at seed = {}, shot = {} , skip and retry.\n", seed, shots);
			fp_meta = fopen(metafile.c_str(), "a+");
			fmt::print(fp_meta, "Error seed = {}, shot = {}. Please check.\n", seed, i);
			fclose(fp_meta);
			i -= 1;

		}
		else
		{
			fp_result = fopen(resultfile.c_str(), "a+");
			std::string progress_str = fmt::format("{} / {}", i, shots);
			fmt::print("{:^19.15f}|{:^12s}\n", stat_fidelity.record(fidelity), progress_str);
			fmt::print(fp_result, "{:^19.15f}\n", fidelity);
			fclose(fp_result);
		}
	}

	fp_meta = fopen(metafile.c_str(), "a+");
	{
		// print avg.
		std::string infostr = fmt::format("<-- Average on {} shots -->", shots);
		fmt::print("{:^40s}\n", infostr);
		fmt::print("{:^19f}|\n",
			stat_fidelity.mean()
		);
		fmt::print(fp_meta, "{:^19s}\n", infostr);
		fmt::print(fp_meta, "{:^19f}|\n",
			stat_fidelity.mean()
		);
	}
	{
		// print std.
		std::string infostr = fmt::format("<-- Std on {} shots -->", shots);
		fmt::print("{:^40s}\n", infostr);
		fmt::print("{:^19f}|\n",
			stat_fidelity.std()
		);
		fmt::print(fp_meta, "{:^19s}\n", infostr);
		fmt::print(fp_meta, "{:^19f}|\n",
			stat_fidelity.std()
		);
	}
	fmt::print("{}\n", profiler::get_all_profiles_v2());
	fmt::print(fp_meta, "{}\n", profiler::get_all_profiles_v2());
	fclose(fp_meta);
}

void QRAMFidelityTest(int argc, const char** argv)
{

	QRAMFidelityTestArguments args = qram_fidelity_argument_parse(argc, argv);

	if (args.addr_sz > 0)
	{
		QRAMFidelityTestImpl(
			args.addr_sz,
			args.data_sz,
			args.seed,
			args.generate_noise(),
			args.shots,
			args.input_size,
			args.version);
	}
	else
	{
		fmt::print("Bad input.\n");
		throw_invalid_input();
	}
}

int QRAMFidelityTest_main(int argc, const char** argv) {

	static const char* test_argv[] = {
			".",
			"--addrsize", "15",
			"--datasize", "3",
			"--shots", "100",
			"--inputsize", "10",
			"--depolarizing", "1e-4",
			"--damping", "1e-5",
			"--seed", "123456",
			"--version", "normal"
			// "--seed", "1515151"
	};

	const char** argv_ = argv;
	if (argc == 1) 
	{		
		argc = array_length<decltype(test_argv)>::value;
		argv_ = test_argv;
	}

	QRAMFidelityTest(argc, argv_);

	return 0;
}

int main(int argc, const char** argv)
{
	int ret = 0;

	ret = QRAMFidelityTest_main(argc, argv);	
	
	return ret;
}
