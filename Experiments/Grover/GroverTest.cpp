#include <exception>
#include <typeinfo>

#include "argparse.h"
#include "logger.h"
#include "state_manipulator.h"
#include "grover.h"
#include "time_step.h"
#include "simple_quantum_simulator.h"

using namespace std;
using namespace qram_simulator;
using namespace qram_simulator::quantum_simulator;
using namespace grover;
using namespace grover_dense;


struct GroverTestArguments
{
	static constexpr const char* name = "GroverTest";
	int qubit = -1;
	size_t position = 0;
	size_t shots = 0;
	size_t repeats = 0;
	seed_t seed = 1010101;
	double depolarizing = 0.0;
	double damping = 0.0;
	std::string experimentname;
	std::string version = "new";

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
		if (qubit != -1) {
			std::vector<char> buf;
			fmt::format_to(back_inserter(buf),
				"qubit={} position={} shots={} grover_repeats={}", qubit, position, shots, repeats);

			if (architecture == arch_qutrit)
				fmt::format_to(back_inserter(buf), " arch=qutrit");
			else
				throw_invalid_input();
			//else if (architecture == arch_qubit)
			//	fmt::format_to(back_inserter(buf), " arch=qubit");

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
			"\"qubit\":{}, "
			"\"position\":{}, "
			"\"shots\":{}, "
			"\"grover_repeats\":{}, "
			"\"depolarizing\":{}, "
			"\"damping\":{}, "
			"\"architecture\":\"{}\", "
			"\"version\":\"{}\""
			"}}", qubit, position, shots, repeats, depolarizing, damping,
			arch2str(architecture), version
		);
	}

};

inline GroverTestArguments grover_argument_parse(int argc, const char** argv)
{
	GroverTestArguments args;
	argparse::ArgumentParser parser("QRAM_Test", "Grover Algorithm with QRAM Test");
	parser.add_argument()
		.names({ "-q", "--qubit" })
		.description("qubit number")
		.required(false);

	parser.add_argument()
		.names({ "-p", "--position" })
		.description("target position (default = random)")
		.required(false);

	parser.add_argument()
		.names({ "-s", "--shots" })
		.description("number of shots (default = 10000)")
		.required(false);

	parser.add_argument()
		.name("--seed")
		.description("random engine's seed (default = time(0))")
		.required(false);

	parser.add_argument()
		.names({ "-r", "--repeats" })
		.description("number of grover repeats (default = sqrt(N)*pi/4 )")
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
		.name("--architecture")
		.description("architecture (qutrit or qubit) (default = qutrit)")
		.required(false);

	parser.add_argument()
		.name("--experimentname")
		.description("name of the experiment, added as the postfix")
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

	if (parser.exists("qubit")) {
		int qubit = parser.get<int>("qubit");
		if (qubit < 1 || qubit > 20) {
			fmt::print("Qubit number invalid. (Expect:[1,20] Input:{})\n", qubit);
			return args;
		}
		else {
			args.qubit = qubit;
		}
	}
	else {
		args.qubit = 3;
	}

	size_t N = pow2(args.qubit);

	if (parser.exists("position"))
		args.position = parser.get<size_t>("position");
	else {
		static std::uniform_int_distribution<size_t> ud(0, N - 1);
		args.position = ud(random_engine::get_engine());
	}
	if (parser.exists("shots"))
		args.shots = parser.get<size_t>("shots");
	else
		args.shots = 100;

	if (parser.exists("seed"))
		args.seed = parser.get<seed_t>("seed");
	else
		args.seed = time(0);

	if (parser.exists("repeats"))
		args.repeats = parser.get<size_t>("repeats");
	else
		args.repeats = (size_t)(std::round(sqrt(N) / 4 * pi));

	if (parser.exists("depolarizing"))
		args.depolarizing = parser.get<double>("depolarizing");

	if (parser.exists("damping"))
		args.damping = parser.get<double>("damping");

	if (parser.exists("architecture"))
	{
		std::string arch = parser.get<std::string>("architecture");
		if (arch == "qutrit") {
			args.architecture = arch_qutrit;
		}
		else
			throw_invalid_input();
	}

	if (parser.exists("experimentname"))
		args.experimentname = parser.get<std::string>("experimentname");

	if (parser.exists("version")) {
		auto versionstr = parser.get<std::string>("version");

		if (versionstr == "new" ||
			versionstr == "old" ||
			versionstr == "fast")
			args.version = versionstr;
	}

	fmt::print("Configurations:\n{}\n", args.to_string());
	return args;
}



// main_grover_test
int grover_test_legacy(int argc, const char **argv)
{
#if 1
	const char *test_argv[] = { 
		"GroverTest.exe", 
		"--qubit", "3", 
		"--shots", "10000"
		"--depolarizing", "1e-4",
		"--damping", "1e-4",
		"--version", "fast",
		"--architecture", "qutrit",
	};

	argc = array_length<decltype(test_argv)>::value;
	GroverTestArguments args = grover_argument_parse(argc, test_argv);
#else	
	GroverTestArguments args = grover_argument_parse(argc, argv);
#endif

	if (args.qubit == -1)
		return 1;

	std::vector<size_t> ret;
	if (args.architecture == arch_qutrit) {
		ret = grover_shots<QRAMFullAmp>(
			args.qubit, args.position, args.shots, args.repeats, args.generate_noise(), args.version);
	}
	else {
		return 1;
	}

	size_t max_n = 10;
	auto&& maxret = get_max_elements(ret, max_n);

	size_t correct = ret[args.position];
	double correct_rate = 1.0 * correct / args.shots;
	Outputter outputter;
	outputter.make_output(args, std::make_tuple(
		std::make_tuple("max10amplitudes", maxret),
		std::make_tuple("correct_rate", correct_rate)
	));

	return 0;
}

auto grover_test_sparse(size_t addr_size, size_t data_size)
{
	random_engine::time_seed();
	qram_qutrit::QRAMCircuit qram(addr_size, data_size);
	qram.set_memory_random();
	size_t full_size = pow2(addr_size);
	
	// search target set to a random position
	auto& mem = qram.get_memory();
	size_t randpos = random_engine::rng() * full_size;
	size_t search_target = mem[randpos];

	// get all positions, and count
	size_t count = 0;
	std::vector<size_t> target_positions;
	for (size_t i = 0; i < full_size; ++i)
	{
		if (mem[i] == search_target)
		{
			++count;
			target_positions.push_back(i);
		}
	}

	if (count != 4)
		return;

	// calculate repeats
	size_t n_repeats = std::sqrt(full_size * 1.0 / count) * pi / 4;

	// begin to initialize the quantum system
	auto qram_addr_reg = System::add_register("addr", UnsignedInteger, addr_size);
	auto search_data_reg = System::add_register("target", UnsignedInteger, data_size);

	std::vector<System> state;
	state.emplace_back();
	Init_Unsafe("target", search_target)(state);
	
	// start to amplify
	// 	
	// GroverAmplify(&qram, qram_addr_reg, data_size, n_repeats)(state);

	(Hadamard_Int_Full(qram_addr_reg))(state); 

	fmt::print("N(count) = {}\n", count);
	fmt::print("N(repeats) = {}\n", n_repeats);

	for (size_t i = 0; i < n_repeats * 4; ++i)
	{
		auto qram_data_reg = AddRegister("data", UnsignedInteger, data_size)(state);
		GroverOperator(&qram, qram_addr_reg, qram_data_reg, search_data_reg)(state);
		(RemoveRegister(qram_data_reg))(state);

		if (1) {
			double success_prob = 0;
			std::vector<complex_t> full_amplitudes(full_size);
			for (auto& s : state)
			{
				full_amplitudes[s.get(qram_addr_reg).value] = s.amplitude;
			}

			for (auto target_position : target_positions)
			{
				success_prob += abs_sqr(full_amplitudes[target_position]);
			}

			fmt::print("i = {:^3d} | Successful rate = {}\n", i, success_prob);
		}
	}

	double success_prob = 0;
	std::vector<complex_t> full_amplitudes(full_size);
	for (auto& s : state)
	{
		full_amplitudes[s.get(qram_addr_reg).value] = s.amplitude;
	}

	for (auto target_position : target_positions)
	{
		success_prob += abs_sqr(full_amplitudes[target_position]);
	}

	fmt::print("Successful rate = {}\n", success_prob);
}

auto grover_test_main()
{
	size_t addr_size = 10;
	size_t data_size = 8;
	size_t shots = 1000;
	for (size_t i = 0; i < shots; ++i)
	{
		grover_test_sparse(addr_size, data_size);
	}
}

auto grover_count_test(size_t addr_size, size_t data_size, size_t count_precision)
{
	qram_qutrit::QRAMCircuit qram(addr_size, data_size);
	qram.set_memory_random();
	size_t full_size = pow2(addr_size);

	// search target set to a random position
	auto& mem = qram.get_memory();
	size_t randpos = random_engine::rng() * full_size;
	size_t search_target = mem[randpos];

	// get all positions, and count
	size_t count = 0;
	std::vector<size_t> target_positions;
	for (size_t i = 0; i < full_size; ++i)
	{
		if (mem[i] == search_target)
		{
			++count;
			target_positions.push_back(i);
		}
	}
	/*if (count != 4)
		return;*/

	// begin to initialize the quantum system
	auto count_reg = System::add_register("count", UnsignedInteger, count_precision);
	auto qram_addr_reg = System::add_register("addr", UnsignedInteger, addr_size);
	auto qram_data_reg = System::add_register("data", UnsignedInteger, data_size);
	auto search_data_reg = System::add_register("target", UnsignedInteger, data_size);

	std::vector<System> state;
	state.emplace_back();
	Init_Unsafe("target", search_target)(state);

	GroverCount(&qram, count_reg, qram_addr_reg, qram_data_reg, search_data_reg)(state);
	
	fmt::print("Target = {}  |  Count = {}  |  ", search_target, count);

	auto pred = [](const System& lhs, const System& rhs)
	{
		return abs_sqr(lhs.amplitude) > abs_sqr(rhs.amplitude);
	};

	std::sort(state.begin(), state.end(), pred);

	size_t value = state[0].get(count_reg).value;
	if (value > pow2(count_precision - 1))
		value = pow2(count_precision) - value;

	fmt::print("Value = {}  |  ", value);
	double theta = value * 1.0 / pow2(count_precision) * pi * 2;
	double prob = std::sin(theta / 2) * std::sin(theta / 2);
	double count_estimate = prob * pow2(addr_size);

	fmt::print("Estimated Count = {}\n", count_estimate);

	if (1 || std::abs(count_estimate - 4) > 1)
	{
		StatePrint(StatePrintDisplay::Detail | StatePrintDisplay::Prob)(state);
		getchar();
	}

}

auto grover_count_test_main()
{
	random_engine::time_seed();
	size_t addr_size = 4;
	size_t data_size = 3;
	size_t count_precision = 8;

	for (size_t i = 0; i < 100; ++i)
	{
		grover_count_test(addr_size, data_size, count_precision);
		System::clear();
	}
	fmt::print("{}\n", profiler::get_all_profiles_v2());
	return 0;
}

auto grover_count_success_test(size_t addr_size, size_t data_size, size_t count_precision)
{
	qram_qutrit::QRAMCircuit qram(addr_size, data_size);
	qram.set_memory_random();
		
	size_t full_size = pow2(addr_size);

	// search target set to a random position
	auto& mem = qram.get_memory();
	size_t randpos = random_engine::rng() * full_size;
	size_t search_target = mem[randpos];

	// get all positions, and count
	size_t count = 0;
	std::vector<size_t> target_positions;
	for (size_t i = 0; i < full_size; ++i)
	{
		if (mem[i] == search_target)
		{
			++count;
			target_positions.push_back(i);
		}
	}
	/*if (count != 4)
		return std::pair{ std::numeric_limits<size_t>::max(), false };*/

	// begin to initialize the quantum system
	auto count_reg = System::add_register("count", UnsignedInteger, count_precision);
	auto qram_addr_reg = System::add_register("addr", UnsignedInteger, addr_size);
	auto qram_data_reg = System::add_register("data", UnsignedInteger, data_size);
	auto search_data_reg = System::add_register("target", UnsignedInteger, data_size);

	std::vector<System> state;
	state.emplace_back();
	Init_Unsafe("target", search_target)(state);

	GroverCount(&qram, count_reg, qram_addr_reg, qram_data_reg, search_data_reg)(state);

	fmt::print("Target = {}  |  Count = {}  |  ", search_target, count);

	auto&& [measured_results, test_prob] = PartialTrace(std::vector{ count_reg })(state);
	if (std::isinf(test_prob))
		fmt::print("(inf?)");
	fmt::print("Value = {}  |  ", measured_results[0]);
	double theta = measured_results[0] * 1.0 / pow2(count_precision) * pi * 2;
	double prob = std::sin(theta / 2) * std::sin(theta / 2);
	double count_estimate = prob * pow2(addr_size);

	fmt::print("Estimated Count = {}", count_estimate);

	if (std::abs(count_estimate - count) > 1)
		return std::pair{ count, false };
	else
		return std::pair{ count, true };
}

auto grover_count_success_test_main()
{
	size_t addr_size = 5;
	size_t data_size = 3;
	size_t count_precision = 8;
	std::vector<size_t> n_total(pow2(addr_size - 1));
	std::vector<size_t> n_success(pow2(addr_size - 1));
	size_t shots = 1000;
	for (size_t i = 0; i < shots; ++i)
	{
		fmt::print("i = {} | ", i);
		auto&& [count, success] = grover_count_success_test(addr_size, data_size, count_precision);
		if (count >= n_total.size())
			continue;
		
		n_total[count]++;
		if (success)
		{
			n_success[count]++;
			fmt::print(" (success)\n");
		}
		else {
			fmt::print(" (fail)\n");
		}
		System::clear();
	}
	fmt::print("ALL = {}\nSUC = {}\n", n_total, n_success);
}

int main()
{
	// grover_test_main();
	grover_count_success_test_main();
	return 0;
}