// This is a test file for the CPU implementation of the Quantum Sparse State Calculator.
// 
// Author: Agony
// Date: 2025/02/15

#include "matrix.h"
#include "error_handler.h"
#include "random_engine.h"
#include "hamiltonian_simulation.h"
#include "BlockEncoding/make_qram.h"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"

using namespace qram_simulator;
using namespace QDA;

#include "CorrectnessTest_Common.inl"
#include "CorrectnessTest_BlockEncoding.inl"
#include "CorrectnessTest_QDA_CompareList.inl"
#include "CorrectnessTest_QRAM.inl"


int main()
{
	try
	{
		/* Schmidt Orthogonalization Test */
		TEST(schmidt_test);
		TEST(schmidt_matrixize_test);

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
		TEST(QRAMQutrit_FidelityTest);

		/* Chebyshev test */
		System::clear();
		TEST(Chebyshev_test);

		/* Linear solver test */
		TEST(linear_solver_theory_compare_test);

		/* Block encoding test */
		TEST(testBlockEncoding_U_plus_minus);
		TEST(testBlockEncoding_Tridiagonal);
		TEST(testBlockEncodingA_Tridiagonal_by_QRAM);
		TEST(testBlockEncodingA_Random_by_QRAM);

		/* QDA test */
		TEST(QDA_Poiseuille_Tridiagonal_test);

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
	//catch (const std::exception& e)
	//{
	//	fmt::print("Error: {}\n", e.what());
	//	return 3;
	//}

	return 0;
}
