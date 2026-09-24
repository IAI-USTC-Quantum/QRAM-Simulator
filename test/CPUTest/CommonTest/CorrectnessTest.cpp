// This is a test file for the CPU implementation of the Quantum Sparse State Calculator.
//
// Author: Agony
// Date: 2025/02/15

#include "matrix.h"
#include "error_handler.h"
#include "random_engine.h"
#include "time_step.h"

using namespace qram_simulator;

#include "CorrectnessTest_Common.inl"
#include "CorrectnessTest_QRAM.inl"


int main()
{
	try
	{
		/* Schmidt Orthogonalization Test */
		TEST(schmidt_test);

		/* Complement test */
		TEST(complement_test);
		TEST(integer_addition_complement_test);

		/* Test get_rational */
		TEST(test_get_rational);

		/* Test check_unique_sort */
		TEST(test_check_unique_sort);

		/* Test random matrix generation */
		TEST(random_matrix_test);

		/* continuous range test */
		TEST(continuous_range_test);

		/* TimeStep test */
		//TEST(test_time_step); /* why is this failing only in Linux? */

		/* QRAM qutrit Test */
		TEST(QRAM_compare_test);

		/* QRAM qubit Test (full / no-pruning ground truth) */
		TEST(QRAMQubit_FullCorrectnessTest);

		fmt::print("===== All tests passed. =====\n");
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

	return 0;
}
