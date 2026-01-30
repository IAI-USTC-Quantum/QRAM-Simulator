#include <exception>
#include <typeinfo>

#include "logger.h"
#include "state_manipulator.h"
#include "sparse_state_simulator.h"
#include "grover.h"
#include "time_step.h"
#include "command_line_config.h"

using namespace qram_simulator;

qram_qutrit::QRAMCircuit configure_qram(size_t addr_sz, size_t data_sz, const noise_t& noise)
{
	qram_qutrit::QRAMCircuit qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);

	return qram;
}

auto QRAMFidelityTestImpl_1shot(size_t addr_sz, size_t data_sz, QRAMInputGenerator& gen,
	const noise_t& noise, size_t input_size, std::string version, size_t addr, size_t data)
{
	std::vector<System> state;
	gen.generate_input(state);

	/****** QRAM Initialize ******/
	qram_qutrit::QRAMCircuit qram = qram_qutrit::QRAMCircuit(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);

	QRAMLoad::version = version;
	QRAMLoad(&qram, addr, data)(state);
	QRAMLoad::version = "noisefree";
	QRAMLoad(&qram, addr, data)(state);
	complex_t fidelity = 0;
	for (auto& s : state)
	{
		auto p = std::make_pair(s.get(addr).value, s.get(data).value);
		if (std::find(gen.unique_set.begin(), gen.unique_set.end(), p)
			!= gen.unique_set.end())
		{
			fidelity += std::conj(s.amplitude) / std::sqrt(input_size);
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

	random_engine::get_instance().set_seed(seed);
	auto addr = System::add_register("addr", UnsignedInteger, addr_sz);
	auto data = System::add_register("data", UnsignedInteger, data_sz);

	QRAMInputGenerator gen(addr_sz, data_sz, input_size, addr, data);

	std::string hash = get_random_hash_str();
	std::string metafile = "Metadata-" + hash + ".txt";
	std::string resultfile = "Result-" + hash + ".csv";

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
		auto fidelity = QRAMFidelityTestImpl_1shot(addr_sz, data_sz, gen, noise, input_size, version, addr, data);
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

#if 1
	const char* test_argv[] = {
		".",
		"--addrsize", "15",
		"--datasize", "3",
		"--shots", "10000",
		"--inputsize", "64",
		"--depolarizing", "1e-5",
		"--damping", "1e-5",
		"--version", "normal"
		// "--seed", "1515151"
	};
	argc = array_length<decltype(test_argv)>::value;
	argv = test_argv;
#endif

	QRAMFidelityTest(argc, argv);
	return 0;
}

int main(int argc, const char** argv)
{
	int ret = 0;
	ret = QRAMFidelityTest_main(argc, argv);
	return ret;
}
