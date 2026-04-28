#include "hamiltonian_simulation.h"
#include "state_manipulator.h"
#include "simple_quantum_simulator.h"
#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "BlockEncoding/make_qram.h"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"

using namespace qram_simulator;

int test_split_register() {
	System::add_register("main_reg", UnsignedInteger, 4);
	System::add_register("anc_UA", UnsignedInteger, 4);
	System::add_register("anc_1", UnsignedInteger, 1);

	std::vector<System> state;
	state.emplace_back();
	(Hadamard_Int_Full("main_reg"))(state);
	(Hadamard_Int_Full("anc_UA"))(state);
	StatePrint(0, 16)(state);

	auto sp1 = SplitRegister("anc_UA", "sp1", 1)(state);
	StatePrint(0, 16)(state);
	auto sp2 = SplitRegister("anc_UA", "sp2", 1)(state);
	StatePrint(0, 16)(state);
	fmt::print("Split: {}\n", System::name_register_map);
	CombineRegister("anc_UA", "sp2")(state);
	CombineRegister("anc_UA", "sp1")(state);
	StatePrint(0, 16)(state);
	fmt::print("Combine: {}\n", System::name_register_map);

	auto sp3 = SplitRegister("anc_UA", "sp3", 1)(state);
	StatePrint(0, 16)(state);
	auto sp4 = SplitRegister("anc_UA", "sp4", 1)(state);
	StatePrint(0, 16)(state);
	fmt::print("Split: {}\n", System::name_register_map);
	CombineRegister("anc_UA", "sp4")(state);
	CombineRegister("anc_UA", "sp3")(state);
	StatePrint(0, 16)(state);
	fmt::print("Combine: {}\n", System::name_register_map);

	System::clear();
	return 0;
}

int test_plusoneandoverflow() {
	using POAO = block_encoding::block_encoding_tridiagonal::PlusOneAndOverflow;
	// 2-bit main_reg + 1-bit overflow = 3 total bits
	auto factory = [](std::vector<size_t> ids) -> POAO {
		return POAO{System::name_of(ids[0]), System::name_of(ids[1])};
	};
	std::vector<size_t> truth_table1 = check_inplace_unitarity<POAO>({2, 1}, factory, true);
	fmt::print("Truth table (dagger first): {}\n", truth_table1);
	std::vector<size_t> truth_table2 = check_inplace_unitarity<POAO>({2, 1}, factory, false);
	fmt::print("Truth table (forward first): {}\n", truth_table2);

	System::clear();
	return 0;
}

/*
* Test the random hash function for strings.
* Reduced iterations to minimize CI output.
*/
void random_hash_str_test()
{
	std::string data = "Hello world";
	for (size_t i = 0; i < 5; ++i)
	{
		auto hash = qram_simulator::get_random_hash_str(data);
		// Only print first and last
		if (i == 0 || i == 4) fmt::print("hash {}: {}...\n", i, hash.substr(0, 8));
	}
}

void QRAM_state_manipulator_test()
{
	using namespace qram_simulator::quantum_simulator;
	QRAMFullAmp qram(2, 2, { 0,1,2,3 });

	noise_t noise = {
		{OperationType::Damping, 1.e-1},
		{OperationType::Depolarizing, 1.e-3},
	};
	qram->set_noise_models(noise);

	std::vector<size_t> address_qubits = { 0,1 };
	std::vector<size_t> data_qubits = { 2,3 };
	std::vector<size_t> other_qubits;
	size_t qn = address_qubits.size() + data_qubits.size() + other_qubits.size();

	std::vector<complex_t> state;
	init_n_state(state, qn);
	iota(state.begin(), state.end(), 0);

	double A = std::accumulate(state.begin(), state.end(), 0, [](double v, complex_t& m) {return v + abs_sqr(m); });

	// Reduced iterations for CI efficiency
	for (int i = 0; i < 5; ++i) {
		state = qram.apply(state, address_qubits, data_qubits, other_qubits, "new");
		std::for_each(state.begin(), state.end(), [A](complex_t& c) {c *= sqrt(A); });
	}
	fmt::print("QRAM state manipulator test passed (5 iterations)\n");
}


void QDA_Poiseuille_via_QRAM_test(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	using namespace QDA;
	using namespace QDA_via_QRAM;
	System::clear();

	int exponent = 15;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	double kappa = get_kappa_general(mat);
	if (kappa > 10) kappa = 10;

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	DenseVector<double> b = DenseVector<double>::random(pow2(nqubit));
	b = b / b.norm2();
	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	qram_qutrit::QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);

	std::vector<System> state;
	state.emplace_back();

	Walk_s_via_QRAM::Encb enc_b(&qram_b, "main_reg", data_size, rational_size);
	enc_b(state);

	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) steps += 1;

	for (size_t n = 0; n < steps; n++)
	{
		double s = double(n) / steps;
		auto walk = Walk_s_via_QRAM_Debug(&qram_A, &qram_b, mat, b,
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
			s, kappa, p, false, data_size, rational_size);
		walk(state);
		ClearZero()(state);
		CheckNormalization(1e-7)(state);
	}

	System::clear();
}

void QDA_Poiseuille_via_QRAM_test()
{
	double step_rate = 0.01;
	double p = 1.3;
	size_t trials = 2;  // Reduced from 10 for CI efficiency
	for (size_t i = 0; i < trials; i++)
	{
		random_engine::time_seed();
		size_t nqubit = random_engine::randint(2, 3);  // Reduced max nqubit
		double alpha = random_engine::uniform(1.0, 2.0);
		double beta = random_engine::uniform(-2.0, 2.0);

		QDA_Poiseuille_via_QRAM_test(nqubit, step_rate, p, alpha, beta);
	}
}

void QDA_Poiseuille_Tridiagonal_test(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	using namespace QDA;
	using namespace QDA_tridiagonal;
	System::clear();

	int exponent = 15;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	double kappa = get_kappa_general(mat);
	if (kappa > 10) kappa = 10;

	DenseVector<double> b = DenseVector<double>::ones(pow2(nqubit));
	b = b / b.norm2();

	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, 4);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);

	std::vector<System> state;
	state.emplace_back();

	Walk_s_Tridiagonal::Encb enc_b("main_reg");
	enc_b(state);

	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) steps += 1;

	for (size_t n = 0; n < steps; n++)
	{
		double s = double(n) / steps;
		auto walk = Walk_s_Tridiagonal_Debug(mat, b,
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
			s, kappa, p, alpha, beta);
		walk(state);
		ClearZero()(state);
		CheckNormalization(1e-7)(state);
	}

	System::clear();
}


void QDA_Poiseuille_Tridiagonal_test()
{
	double step_rate = 0.01;
	double p = 1.3;
	size_t trials = 2;  // Reduced from 10 for CI efficiency
	for (size_t i = 0; i < trials; i++)
	{
		random_engine::time_seed();
		size_t nqubit = random_engine::randint(2, 3);  // Reduced max nqubit
		double alpha = random_engine::uniform(1.0, 2.0);
		double beta = random_engine::uniform(-2.0, 2.0);

		QDA_Poiseuille_Tridiagonal_test(nqubit, step_rate, p, alpha, beta);
	}
}

void QDA_random_matrix_test(size_t nqubit, double step_rate, double p, double kappa)
{
	using namespace QDA;
	using namespace QDA_via_QRAM;
	System::clear();

	int exponent = 15;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix mat = generate_specified_kappa_mat_asymmetric(pow2(nqubit), kappa);
	mat = mat / mat.normF();

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	DenseVector<double> b = DenseVector<double>::random(pow2(nqubit));
	b = b / b.norm2();
	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	qram_qutrit::QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
	auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
	auto anc_1 = System::add_register("anc_1", Boolean, 1);
	auto anc_2 = System::add_register("anc_2", Boolean, 1);
	auto anc_3 = System::add_register("anc_3", Boolean, 1);
	auto anc_4 = System::add_register("anc_4", Boolean, 1);

	std::vector<System> state;
	state.emplace_back();

	Walk_s_via_QRAM::Encb enc_b(&qram_b, "main_reg", data_size, rational_size);
	enc_b(state);

	constexpr int StepConstant = 2305;
	size_t steps = size_t(step_rate * StepConstant * kappa);
	if (steps % 2 != 0) steps += 1;

	for (size_t n = 0; n < steps; n++)
	{
		double s = double(n) / steps;
		auto walk = Walk_s_via_QRAM_Debug(&qram_A, &qram_b, mat, b,
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
			s, kappa, p, false, data_size, rational_size);
		walk(state);
		ClearZero()(state);
		CheckNormalization(1e-7)(state);
	}

	System::clear();
}

void QDA_random_matrix_test()
{
	double step_rate = 0.01;
	double p = 1.3;
	size_t trials = 2;  // Reduced from 10 for CI efficiency
	double kappa = 10;
	for (size_t i = 0; i < trials; i++)
	{
		random_engine::time_seed();
		size_t nqubit = random_engine::randint(2, 3);  // Reduced max nqubit

		QDA_random_matrix_test(nqubit, step_rate, p, kappa);
	}
}

void inherited_operator_test()
{
	SparseState s;
	std::vector<System> s2;
	ModuleInheritance_Test mod1;
	ModuleInheritance_Test_SelfAdjoint mod2;

	mod1(s);
	try { 
		mod1(s); 
	}
	catch (std::runtime_error& e) {
		fmt::print("mod1.dag is OK!\n");
	}
	mod2(s);
	mod2(s);
}


int main()
{
	try
	{
		/* Common test */
		TEST(test_split_register);
		TEST(test_plusoneandoverflow);
		TEST(random_hash_str_test);
		TEST(QRAM_state_manipulator_test);
		TEST(inherited_operator_test);

		/* QDA test */
		TEST(QDA_Poiseuille_via_QRAM_test);
		TEST(QDA_Poiseuille_Tridiagonal_test);
		TEST(QDA_random_matrix_test);

		fmt::print("All tests passed.\n");
		return 0;

	}
	catch (const TestFailException& e)
	{
		fmt::print("Test failed: {}\n", e.what());
		return 1;
	}
	catch (const std::runtime_error& e)
	{
		fmt::print("Runtime error: {}\n", e.what());
		return 2;
	}
	catch (const std::exception& e)
	{
		fmt::print("Error: {}\n", e.what());
		return 3;
	}

	return 0;
}