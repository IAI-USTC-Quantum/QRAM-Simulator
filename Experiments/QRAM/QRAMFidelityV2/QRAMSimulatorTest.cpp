#include <exception>
#include <typeinfo>

#include "argparse.h"
#include "logger.h"
#include "state_manipulator.h"
#include "grover.h"
#include "time_step.h"

using namespace std;
using namespace qram_simulator;


struct QRAMSimulatorTestArguments
{
	static constexpr const char* name = "QRAMSimulatorTest";
	int addr_sz = 3;
	int data_sz = 1;
	size_t shots = 0;
	size_t input_size = 0;
	seed_t seed = 1010101;
	std::string version = "new";
	double depolarizing = 0.0;
	double damping = 0.0;
	std::string experimentname;

	int architecture = arch_qutrit; // 0 for qutrit, 1 for qubit

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
			std::vector<char> buf;
			fmt::format_to(back_inserter(buf),
				"asz={} dsz={} shots={} inputsz={} seed={}", addr_sz, data_sz, shots, input_size, seed);

			if (architecture == arch_qutrit)
				fmt::format_to(back_inserter(buf), " arch=qutrit");

			/* In this version, qutrit architecture is supported only. */
			//else if (architecture == arch_qubit)
			//	// fmt::format_to(back_inserter(buf), " arch=qubit");
			//	throw_bad_switch_case();
			else
				throw_bad_switch_case();

			if (depolarizing > 0.0)
				fmt::format_to(back_inserter(buf),
					" depolarizing={}", depolarizing);
			if (damping > 0.0)
				fmt::format_to(back_inserter(buf),
					" damping={}", damping);
			return { buf.data(), buf.size() };
		}
		else {
			return "Bad arguments.";
		}
	}

	inline std::string to_mapstring() const
	{
		return fmt::format("{{"
			"\"addr_sz\":{}, "
			"\"data_sz\":{}, "
			"\"input_sz\":{}, "
			"\"shots\":{}, "
			"\"depolarizing\":{}, "
			"\"damping\":{}, "
			"\"version\":\"{}\", "
			"\"architecture\":\"{}\""
			"}}", addr_sz, data_sz, input_size, shots,
			depolarizing, damping, version, arch2str(architecture)
		);
	}

};

inline QRAMSimulatorTestArguments qram_simulator_argument_parse(int argc, const char** argv)
{
	QRAMSimulatorTestArguments args;
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

	parser.add_argument()
		.name("--architecture")
		.description("architecture (qutrit or qubit) (default = qutrit)")
		.required(false);

	parser.add_argument()
		.name("--experimentname")
		.description("name of the experiment, added as the postfix")
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
		if (addr_size < 1 || addr_size > 20) {
			fmt::print("Address size invalid. (Expect:[1,20] Input:{})\n", addr_size);
			return args;
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

		if (versionstr == "new" ||
			versionstr == "old" ||
			versionstr == "fast")
			args.version = versionstr;
	}

	if (parser.exists("architecture"))
	{
		std::string arch = parser.get<std::string>("architecture");
		if (arch == "qutrit") {
			args.architecture = arch_qutrit;
		}
		/*else if (arch == "qubit") {
			args.architecture = arch_qubit;
		}*/
		else {
			throw_invalid_input();
		}
	}

	if (parser.exists("experimentname"))
		args.experimentname = parser.get<std::string>("experimentname");

	fmt::print("Configurations:\n{}\n", args.to_string());
	return args;
}



template<typename QRAM_type = qram_qutrit::QRAMCircuit>
QRAM_type configure_qram(size_t addr_sz, size_t data_sz, const noise_t& noise, seed_t seed, size_t input_sz)
{
	random_engine::get_instance().set_seed(seed);

	QRAM_type qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	qram.set_input_uniform(input_sz);
	// qram.set_input_random(input_sz);

	return qram;
}

auto QRAM_compare_algorithm(qram_qutrit::QRAMCircuit& qram, size_t addr, size_t data, std::string version, bool verbose, seed_t seed)
{
	profiler *prof = new profiler(fmt::format("Qutrit({}) asz={} dsz={}", version, addr, data));
		
	random_engine::get_instance().set_seed(seed);

	if (version == qram_qutrit::QRAMCircuit::FULL_VER) {
		qram.run_full();
	}
	else if (version == qram_qutrit::QRAMCircuit::NORMAL_VER) {
		qram.run_normal();
	}
	else if (version == "fast") {
		// not yet support in-place fast simulation
		throw_invalid_input();
	}
	else
		throw_invalid_input();
	
	if (verbose) {
		log_info(fmt::format("Seed = {}\n", seed));
		log_info(fmt::format("{}\n", qram.to_string()));
		// log_info(fmt::format("Bad = {}", qram.time_step.bad_branches));
		log_info(fmt::format("Operations = \n{}\n", qram.operations.to_string()));
		log_info(fmt::format("Fidelity = {}\n", qram.sample_and_get_fidelity()));
		log_info(fmt::format("FinalSystem = {}\n", qram.final_system_state));
		log_info(fmt::format("first_good_branch = {}", qram.first_good_branch));
		log_info(fmt::format("{}\n", qram.to_string()));
	}
	delete prof;
	return qram.sample_and_get_fidelity();
}

auto test1(size_t addr, size_t data, seed_t seed, const noise_t &noise, int trials = 100, size_t input_sz = -1)
{
	double old_avg_fid = 0;
	double new_avg_fid = 0;
	double fast_avg_fid = 0;
	std::vector<double> old_fidelity_list(trials, 0);
	std::vector<double> new_fidelity_list(trials, 0);
	std::vector<double> fast_fidelity_list(trials, 0);
	size_t fail = 0;
	for (int it = 0; it < trials; ++it)
	{
		profiler _("MainLoop");
		random_engine::get_instance().set_seed(seed + it); 
		auto runseed = random_engine::get_instance().reseed();
		fmt::print("{} / {} (seed={})\n", it, trials, runseed);

		auto qram_old = configure_qram(addr, data, noise, seed, input_sz);
		auto old_fid = QRAM_compare_algorithm(qram_old, addr, data, "full", false, runseed);
		// fmt::print(qram_old.to_string());
		old_avg_fid += old_fid;
		old_fidelity_list[it] = old_fid;

		auto qram_new = configure_qram(addr, data, noise, seed, input_sz);
		auto new_fid = QRAM_compare_algorithm(qram_new, addr, data, "normal", false, runseed);
		// fmt::print(qram_new.to_string());
		new_avg_fid += new_fid;
		new_fidelity_list[it] = new_fid;

		if (!ignorable(new_fid - old_fid))
			fail++;
		// fmt::print("asz={} dsz={}, ", addr, data);
		if (abs(old_fid - new_fid) > epsilon) {
			fmt::print("fail, fid_full={:.5f}, fid_normal={:.5f}, seed={}\n", old_fid, new_fid, runseed);
		}
		else
			fmt::print("pass, fid_full={:.5f}, fid_normal={:.5f}\n", old_fid, new_fid);
	}
	old_avg_fid /= trials;
	new_avg_fid /= trials;
	fast_avg_fid /= trials;

	return std::make_tuple(
		std::make_tuple("old_average_fidelity", old_avg_fid),
		std::make_tuple("new_average_fidelity", new_avg_fid),
		std::make_tuple("fast_average_fidelity", fast_avg_fid),
		std::make_tuple("old_fidelity_list", old_fidelity_list),
		std::make_tuple("new_fidelity_list", new_fidelity_list),
		std::make_tuple("fast_fidelity_list", fast_fidelity_list),
		std::make_tuple("failed", fail)
	);
}


void QRAMSimulatorTest(int argc, const char** argv)
{
	QRAMSimulatorTestArguments args = qram_simulator_argument_parse(argc, argv);
	if (args.architecture == arch_qutrit)
	{
		auto result = test1(
			args.addr_sz, 
			args.data_sz, 
			args.seed,
			args.generate_noise(), 
			args.shots, 
			args.input_size);
		Outputter outputter;
		outputter.make_output(args, result);
	}
	return;
}


int main(int argc, const char** argv) {

#if 1
	const char* test_argv[] = {
		".",
		"--addrsize", "10",
		"--datasize", "3",
		"--shots", "100",
		"--inputsize", "500",
		"--depolarizing", "1e-4",
		"--damping", "1e-4",
		"--seed", "123456789",
		"--architecture", "qutrit",
		"--experimentname", "qutrit_scheme"
	};
	argc = array_length<decltype(test_argv)>::value;
	argv = test_argv;
#endif

	QRAMSimulatorTest(argc, argv);
	return 0;
}
