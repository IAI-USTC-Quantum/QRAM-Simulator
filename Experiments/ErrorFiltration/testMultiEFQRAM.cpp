#include <filesystem> // Add this include directive for C++17 and later
#include "argparse.h"
// #include "hamiltonian_simulation.h"
#include <Eigen/Eigen>
#include "state_preparation.h"

using namespace qram_simulator;

struct NConditionalSwap
{
	int controller;
	int reg1;
	int reg2;

	size_t control_select_N;

	NConditionalSwap(int controller_, int reg1_, int reg2_, size_t control_select_N_)
		: controller(controller_), reg1(reg1_), reg2(reg2_),
		control_select_N(control_select_N_)
	{}

	void operator()(std::vector<System>& state)
	{
		for (auto& s : state)
		{
			size_t controller_value = s.GetAs(controller, size_t);
			if (controller_value != control_select_N)
				continue;
			std::swap(s.get(reg1).value, s.get(reg2).value);
		}
	}

};

struct GlobalPhase
{
	complex_t extra_global_phase;

	GlobalPhase(complex_t extra_global_phase_)
		: extra_global_phase(extra_global_phase_)
	{}

	void operator()(std::vector<System>& state)
	{
		for (auto& s : state)
		{
			s.amplitude *= extra_global_phase;
		}
	}

};

struct PauliX
{
	int reg1;
	size_t n_digit;

	PauliX(int reg1_, size_t n_digit_)
		: reg1(reg1_), n_digit(n_digit_)
	{}
	void operator()(std::vector<System>& state)
	{
		for (auto& s : state)
		{
			size_t& val = s.get(reg1).value;
			val ^= pow2(n_digit);
		}
	}
};

struct PauliZ
{
	int reg1;
	size_t n_digit;

	PauliZ(int reg1_, size_t n_digit_)
		: reg1(reg1_), n_digit(n_digit_)
	{}
	void operator()(std::vector<System>& state)
	{
		for (auto& s : state)
		{
			size_t& val = s.get(reg1).value;
			if ((val >> n_digit) & 1)
			{
				s.amplitude *= -1;
			}
		}
	}
};

struct PauliY
{
	int reg1;
	size_t n_digit;

	PauliY(int reg1_, size_t n_digit_)
		: reg1(reg1_), n_digit(n_digit_)
	{}
	void operator()(std::vector<System>& state)
	{
		PauliX(reg1, n_digit)(state);
		PauliZ(reg1, n_digit)(state);
		GlobalPhase(complex_t(0, 1))(state);
	}
};

struct Depolarizing
{
	int reg1;
	double p;
	size_t n_digit;

	Depolarizing(int reg1_, size_t n_digit_, double p_)
		: reg1(reg1_), n_digit(n_digit_), p(p_)

	{}

	void operator()(std::vector<System>& state)
	{
		double r = random_engine::uniform01();
		//fmt::print("\nr: {}\n", r);
		if (r > p)
		{
			//fmt::print("I\n");
			return;
		}
		if (r < p / 3)
			// FlipBool
		{
			//fmt::print("X\n");
			PauliX(reg1, n_digit)(state);
		}
		else if (r < p / 3 * 2)
			// Phase + FlipBool with Phase
		{
			//fmt::print("Y\n");
			PauliY(reg1, n_digit)(state);
		}
		else
			// Phase
		{
			//fmt::print("Z\n");
			PauliZ(reg1, n_digit)(state);
		}
	}

};

qram_qutrit::QRAMCircuit configure_qram(size_t addr_sz, size_t data_sz, const noise_t& noise)
{
	qram_qutrit::QRAMCircuit qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);

	return qram;
}

double no_ef_single_shot(int address_size, int data_size, size_t reg_addr, size_t reg_data,
	const qram_qutrit::QRAMCircuit& qram, QRAMInputGenerator& gen, const std::string& mode) {

	std::vector<System> state;

	auto r_1 = reg_addr;
	auto s_1 = reg_data;

	gen.generate_input(state, r_1, s_1);

	QRAMLoad::version = mode;
	QRAMLoad(&qram, r_1, s_1)(state);

	QRAMLoad::version = "noisefree";
	QRAMLoad(&qram, r_1, s_1)(state);

	complex_t fidelity = 0;

	for (auto& s : state)
	{
		auto p = std::make_pair(s.get(r_1).value, s.get(s_1).value);
		if (std::find(gen.unique_set.begin(), gen.unique_set.end(), p)
			!= gen.unique_set.end())
		{
			fidelity += s.amplitude / std::sqrt(gen.input_sz);
		}
	}

	double prob_raw = abs_sqr(fidelity);

	if (std::isnan(prob_raw)) {
		prob_raw = 0;
	}

	return prob_raw;
}

std::tuple<StatisticDouble, std::vector<StatisticDouble>>
main_no_ef(
	int addr_sz,
	int data_sz,
	double depolarizing,
	double damping,
	int ancilla_sz,
	int input_size,
	int real_exp_reps,
	int shots,
	const std::string& mode)
{
	noise_t noise_model =
	{
		{ OperationType::Depolarizing, {depolarizing} },
		{ OperationType::Damping, {damping} },
	};
	System::clear();
	auto r_1 = System::add_register("addr_1", UnsignedInteger, addr_sz);
	auto s_1 = System::add_register("data_1", UnsignedInteger, data_sz);

	StatisticDouble global;
	std::vector<StatisticDouble> error_repetitions;
	for (int rep_exp = 0; rep_exp < real_exp_reps; ++rep_exp)
	{
		qram_qutrit::QRAMCircuit qram = configure_qram(addr_sz, data_sz, noise_model);
		QRAMInputGenerator gen(addr_sz, data_sz, input_size);

		StatisticDouble error_repetition;
		for (int rep = 0; rep < shots; ++rep)
		{
			double value3 = no_ef_single_shot(addr_sz, data_sz, r_1, s_1, qram, gen, mode);
			global.record(value3);
			error_repetition.record(value3);
		}
		error_repetitions.push_back(error_repetition);
	}
	return { global, error_repetitions };
}


std::pair<double, double> with_ef_single_shot(int ancilla_size, int address_size, int data_size, 
	size_t r_1, size_t s_1, size_t r_2, size_t s_2, size_t a_1,	
	const qram_qutrit::QRAMCircuit& qram, QRAMInputGenerator& gen, const std::string& mode){

	std::vector<System> state;

	gen.generate_input(state, r_1, s_1);

	Hadamard_Int(a_1, ancilla_size)(state);


	ClearZero()(state);
	for (int cond_num = 0; cond_num < std::pow(2, ancilla_size); ++cond_num)
	{
		NConditionalSwap(a_1, r_1, r_2, cond_num)(state);
		NConditionalSwap(a_1, s_1, s_2, cond_num)(state);

		QRAMLoad::version = mode;
		QRAMLoad(&qram, r_2, s_2)(state);

		NConditionalSwap(a_1, r_1, r_2, cond_num)(state);
		NConditionalSwap(a_1, s_1, s_2, cond_num)(state);
	}

	Hadamard_Int(a_1, ancilla_size)(state);
	ClearZero()(state);

	QRAMLoad::version = "noisefree";
	QRAMLoad(&qram, r_1, s_1)(state);

	std::map<size_t, size_t> ancilla_measurement_criteria = { {a_1, 0} };
	PartialTraceSelect select_ancilla_zero(ancilla_measurement_criteria);
	double probability_ancilla = select_ancilla_zero(state);

	std::vector<size_t> partial_reg = { r_2, s_2 };
	auto&& [_, __] = (PartialTrace(partial_reg))(state);
	complex_t fidelity = 0;
	for (auto& s : state)
	{
		auto p = std::make_pair(s.get(r_1).value, s.get(s_1).value);
		if (std::find(gen.unique_set.begin(), gen.unique_set.end(), p)
			!= gen.unique_set.end())
		{
			fidelity += s.amplitude / std::sqrt(gen.input_sz);
		}
	}
	double probability_target = abs_sqr(fidelity);

	double prob_anc;
	double prob_cond;

	if (probability_ancilla == 0) {
		prob_anc = 0;
	}
	else {
		prob_anc = 1 / (probability_ancilla * probability_ancilla);
		// Check for NaN result after computation and set to 0 if NaN
		if (std::isnan(prob_anc)) {
			prob_anc = 0;
		}
	}

	prob_cond = probability_target;
	// Check for NaN result after computation and set to 0 if NaN
	if (std::isnan(prob_cond)) {
		prob_cond = 0;
	}

	return { prob_anc, prob_cond };
}

std::tuple< StatisticDouble, StatisticDouble, std::vector<StatisticDouble>, std::vector<StatisticDouble>>
main_with_ef(int addr_sz,
	int data_sz,
	double depolarizing,
	double damping,
	int ancilla_sz,
	int input_size,
	int real_exp_reps,
	int shots, 
	const std::string& mode)
{
	System::clear();
	auto r_1 = System::add_register("addr_1", UnsignedInteger, addr_sz);
	auto s_1 = System::add_register("data_1", UnsignedInteger, data_sz);
	auto a_1 = System::add_register("ancilla", UnsignedInteger, ancilla_sz);
	auto r_2 = System::add_register("addr_2", UnsignedInteger, addr_sz);
	auto s_2 = System::add_register("data_2", UnsignedInteger, data_sz);
	noise_t noise_model =
	{
		{ OperationType::Depolarizing, {depolarizing} },
		{ OperationType::Damping, {damping} }
	};

	StatisticDouble stat_postselection;
	StatisticDouble stat_with_ef;
	std::vector<StatisticDouble> stat_postselection_repetitions;
	std::vector<StatisticDouble> stat_with_ef_repetitions;

	for (int rep_exp = 0; rep_exp < real_exp_reps; ++rep_exp)
	{
		qram_qutrit::QRAMCircuit qram = configure_qram(addr_sz, data_sz, noise_model);
		QRAMInputGenerator gen(addr_sz, data_sz, input_size);

		StatisticDouble stat_postselection_repetition;
		StatisticDouble stat_with_ef_repetition;
		for (int rep = 0; rep < shots; ++rep)
		{
			auto&& [prob_anc, fidelity_real] = with_ef_single_shot(ancilla_sz, addr_sz, data_sz, 
				r_1, s_1, r_2, s_2, a_1, qram, gen, mode);

			stat_postselection.record(prob_anc);
			stat_with_ef.record(fidelity_real * prob_anc);

			stat_postselection_repetition.record(prob_anc);
			stat_with_ef_repetition.record(fidelity_real * prob_anc);
		}
		stat_postselection_repetitions.push_back(stat_postselection_repetition);
		stat_with_ef_repetitions.push_back(stat_with_ef_repetition);
	}
	return { stat_with_ef, stat_postselection, stat_with_ef_repetitions, stat_postselection_repetitions };
}

int main(int argc, const char* argv[])
{
	int addr_sz = 5;
	int data_sz = 1;
	int real_exp_reps = 10;
	int shots = 1000;
	double depolarizing = 1e-5;
	double damping = 1e-5;
	seed_t seed = 12345;
	std::string mode = "normal";

	if (argc != 1)
	{
		if (argc != 9)
		{
			fmt::print("Must have correct number of args\n");
			exit(1);
		}
				
		addr_sz = std::stoi(argv[1]);
		data_sz = std::stoi(argv[2]);
		real_exp_reps = std::stoi(argv[3]);
		shots = std::stoi(argv[4]);
		depolarizing = std::stod(argv[5]);
		damping = std::stod(argv[6]);
		seed = std::stoi(argv[7]);

		mode = std::string(argv[8]);
	}
	int input_size = std::min(16, static_cast<int>(std::pow(2, addr_sz)));
	int ancilla_sz = 0;
	timer t1;
	// Create filename

	random_engine::set_seed(seed);

	std::string filename = fmt::format(
		"results_addr{}_data{}_dep{:.1e}_damp{:.1e}_seed{}_{}.txt",
		addr_sz, data_sz, depolarizing, damping, seed, mode
	);

	{
		FILE* fp = fopen(filename.c_str(), "w");
		if (fp == nullptr)
		{
			fmt::print("Failed to open file {}\n", filename);
			exit(0);
		}
		fclose(fp);
	}

	std::ofstream fp(filename, std::ios_base::app);
	fmt::print(fp, "addr_sz = {}\n", addr_sz);
	fmt::print(fp, "data_sz = {}\n", data_sz);
	fmt::print(fp, "real_exp_reps = {}\n", real_exp_reps);
	fmt::print(fp, "shots = {}\n", shots);
	fmt::print(fp, "depolarizing = {}\n", depolarizing);
	fmt::print(fp, "damping = {}\n", damping);
	fmt::print(fp, "input_size = {}\n", input_size);
	fmt::print(fp, "seed = {}\n", seed);
	fp.flush();

	auto&& [stat_no_ef, repetitions] = 
		main_no_ef(addr_sz, data_sz, depolarizing, damping, ancilla_sz, input_size, real_exp_reps, shots, mode);

	fmt::print(fp, "fidelity no ef = {}\n", (stat_no_ef.mean()));
	fmt::print(fp, "duration no ef (sec) = {} \n", t1.get(sec));
	fp.flush();

	for (auto& repetition : repetitions)
	{
		fmt::print(fp, "fidelity no ef (each_rep) = {}\n", (repetition.mean()));
	}
	fp.flush();

	double err_no_ef = 1 - stat_no_ef.mean();
		
	for (int ancilla_sz = 1; ancilla_sz <= 4; ++ancilla_sz)
	{
		timer t1;
		auto&& [stat_with_ef, stat_postselection, stat_with_ef_repetitions, stat_postselection_repetitions] = 
			main_with_ef(addr_sz, data_sz, depolarizing, damping, ancilla_sz, input_size, real_exp_reps, shots, mode);

		fmt::print(fp, "fidelity ef {} = {}\n", ancilla_sz, (stat_with_ef.mean()));
		fmt::print(fp, "postselection ef {} = {}\n", ancilla_sz, (stat_postselection.mean()));

		double err_with_ef = (1 - stat_with_ef.mean() / stat_postselection.mean());

		fmt::print(fp, "ratio ef {} = {}\n", ancilla_sz, err_no_ef / err_with_ef);

		for (size_t i = 0; i < stat_with_ef_repetitions.size(); ++i)
		{
			fmt::print(fp, "fidelity ef {} (each_rep) = {}\n", ancilla_sz, (stat_with_ef_repetitions[i].mean()));
			fmt::print(fp, "postselection ef {} (each_rep) = {}\n", ancilla_sz, (stat_postselection_repetitions[i].mean()));

			double err_with_ef = (1 - stat_with_ef_repetitions[i].mean() / stat_postselection_repetitions[i].mean());
			fmt::print(fp, "ratio ef {} (each_rep) = {}\n", ancilla_sz, err_no_ef / err_with_ef);
		}

		fmt::print(fp, "duration (sec) ef {} = {} \n", ancilla_sz, t1.get(sec));

		fp.flush();
	}	

	return 0;
}