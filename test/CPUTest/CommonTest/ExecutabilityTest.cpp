#include "state_manipulator.h"
#include "simple_quantum_simulator.h"

using namespace qram_simulator;

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

int main()
{
	try
	{
		/* Common test */
		TEST(random_hash_str_test);
		TEST(QRAM_state_manipulator_test);

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
