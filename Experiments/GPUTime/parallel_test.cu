#include "error_handler.h"
#include "cuda/sparse_state_simulator.cuh"
#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "DiscreteAdiabatic/cuda/qda_fundamental.cuh"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"

using namespace qram_simulator;

void parallel_time_test()
{
	int length = 19;
	/* GPU case */
	{
		System::clear();
		CuSparseState s;
		AddRegister("reg1", UnsignedInteger, length)(s);
		auto start_time = std::chrono::high_resolution_clock::now();

		for (int repeat = 0; repeat < 1; ++repeat)
		{
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				SplitRegister("reg1", reg_name, 1)(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				(Hadamard_Bool(reg_name))(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				CombineRegister("reg1", reg_name)(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				SplitRegister("reg1", reg_name, 1)(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				(Hadamard_Bool(reg_name))(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				CombineRegister("reg1", reg_name)(s);
			}
		}

		cudaDeviceSynchronize();

		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
		std::cout << "Parallel time test (GPU): " << duration << " microseconds" << std::endl;

		StatePrint(StatePrintDisplay::Detail | 0)(s);
	}

	/* CPU case */
	{
		System::clear();
		std::vector<System> s(1);
		AddRegister("reg1", UnsignedInteger, length)(s);

		auto start_time = std::chrono::high_resolution_clock::now();

		for (int repeat = 0; repeat < 1; ++repeat)
		{
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				SplitRegister("reg1", reg_name, 1)(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				(Hadamard_Bool(reg_name))(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				CombineRegister("reg1", reg_name)(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				SplitRegister("reg1", reg_name, 1)(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				(Hadamard_Bool(reg_name))(s);
			}
			for (int i = 0; i < length; ++i)
			{
				std::string reg_name = fmt::format("reg{}", i + 2);
				CombineRegister("reg1", reg_name)(s);
			}
		}
		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
		std::cout << "Parallel time test (CPU): " << duration << " microseconds" << std::endl;

		StatePrint(StatePrintDisplay::Detail | 0)(s);
	}

	fmt::print("{}", profiler::get_all_profiles_v2());
	profiler::init_profiler();
}

int main()
{
	parallel_time_test();

	return 0;
}