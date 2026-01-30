#include "logger.h"
#include "state_manipulator.h"
#include "grover.h"
#include "time_step.h"
#include "state_preparation.h"
#include "argparse.h"

using namespace qram_simulator;
using namespace state_preparation_demo;

struct StatePreparationTestArguments
{
	static constexpr const char* name = "StatePreparationTest";
	int qubit = -1;
	size_t data_range = 3;
	size_t data_sz = 16;
	size_t shots = 100;
	seed_t seed = 1010101;
	double depolarizing = 1e-5;
	double damping = 1e-5;
	std::string experimentname;
	std::string version = "new";

	int architecture = arch_qutrit;

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
		std::vector<char> buf;
		fmt::format_to(back_inserter(buf),
			"qubit={} data_range={} data_size={} shots={}", qubit, data_range, data_sz, shots);

		if (depolarizing > 0.0)
			fmt::format_to(back_inserter(buf),
				" depolarizing={}", depolarizing);
		if (damping > 0.0)
			fmt::format_to(back_inserter(buf),
				" damping={}", damping);
		return { buf.data(), buf.size() };
	}

	//inline std::string to_vecstring() const
	//{
	//	std::map<std::string, std::string> ret;
	//	ret["qubit"] = std::to_string(qubit);
	//	ret["data_range"] = std::to_string(data_range);
	//	ret["data_size"] = std::to_string(data_sz);
	//	ret["shots"] = std::to_string(shots);
	//	ret["seed"] = std::to_string(seed);
	//	ret["depolarizing"] = std::to_string(depolarizing);
	//	ret["damping"] = std::to_string(damping);

	//	return fmt::format("{}", ret);
	//}

	inline std::string to_mapstring() const
	{
		return fmt::format("{{"
			"\"qubit\":{}, "
			"\"data_range\":{}, "
			"\"data_size\":{}, "
			"\"shots\":{}, "
			"\"depolarizing\":{}, "
			"\"damping\":{}, "
			"\"architecture\":\"{}\", "
			"\"version\":\"{}\""
			"}}", qubit, data_range, data_sz, shots, depolarizing, damping,
			arch2str(architecture), version
		);
	}
};

inline StatePreparationTestArguments state_preparation_argument_parse(int argc, const char** argv)
{
	StatePreparationTestArguments args;
	argparse::ArgumentParser parser("QRAM_Test", "Grover Algorithm with QRAM Test");
	parser.add_argument()
		.names({ "-q", "--qubit" })
		.description("qubit number (default = 3)")
		.required(false);

	parser.add_argument()
		.names({ "--datarange" })
		.description("random data range (default = 3)")
		.required(false);

	parser.add_argument()
		.names({ "--datasize" })
		.description("data register size (default = 16)")
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
		.name("--depolarizing")
		.description("depolarizing noise")
		.required(false);

	parser.add_argument()
		.name("--damping")
		.description("damping noise")
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

	if (parser.exists("shots"))
		args.shots = parser.get<size_t>("shots");

	if (parser.exists("seed"))
		args.seed = parser.get<seed_t>("seed");
	else
		args.seed = time(0);

	if (parser.exists("datarange"))
		args.data_range = parser.get<size_t>("datarange");

	if (parser.exists("datasize"))
		args.data_sz = parser.get<size_t>("datasize");

	if (parser.exists("depolarizing"))
		args.depolarizing = parser.get<double>("depolarizing");

	if (parser.exists("damping"))
		args.damping = parser.get<double>("damping");

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


auto state_preparation_test(const StatePreparationTestArguments &args)
{
	if (args.qubit == -1)
		throw_general_runtime_error("No invalid input.");

	size_t qn = args.qubit;
	size_t data_range = args.data_range;
	size_t data_sz = args.data_sz;
	size_t shots = args.shots;

	StatePreparation prep(qn, data_sz, data_range, args.version);
	prep.make_qram();
	random_engine::get_instance().set_seed(args.seed);

	// detail::state_preparation::StatePrint::on = false;
	std::vector<double> fidelity_list;
	double avg_fid = 0;
	for (size_t i = 0; i < shots; ++i)
	{
		profiler _("MainLoop");
		fmt::print("{} / {}\n", i, shots);
		random_engine::get_instance().reseed();
		prep.clear_state();
		prep.random_distribution();
		prep.make_tree();
		prep.set_qram();
		prep.set_noise(args.generate_noise());
		prep.run();
		// prep.print_state();
		// fmt::print("Fidelity={}\n", prep.get_fidelity());
		double fid = prep.get_fidelity();
		fidelity_list.push_back(fid);
		avg_fid += fid;
	}
	// fmt::print("Fidelity={}", prep.get_fidelity_show());
	/*INFO(args.to_string());
	fmt::print("Avg. Fidelity = {}\n", avg_fid / shots);
	INFO(fmt::format("Avg. Fidelity = {}\n", avg_fid / shots));
	fmt::print(profiler::get_all_profiles_v2());
	INFO(profiler::get_all_profiles_v2());*/
	avg_fid /= shots;

	Outputter outputter;
	outputter.make_output(args, std::make_tuple(
		std::make_tuple("average_fidelity", avg_fid),
		std::make_tuple("fidelity_list", fidelity_list)
	));
}

int main(int argc, const char** argv)
{
	if (argc == 1)
	{
		const char* test_argv[] = { "StatePreparationTest",
			"-q", "7",
			"--datasize", "32",
			"--shots", "100",
			"--depolarizing", "1e-8",
			"--damping", "1e-8"
		};
		argc = array_length<decltype(test_argv)>::value;
		auto args = state_preparation_argument_parse(argc, test_argv);
		state_preparation_test(args);
	}
	else
	{
		auto args = state_preparation_argument_parse(argc, argv);
		state_preparation_test(args);
	}

	return 0;
}