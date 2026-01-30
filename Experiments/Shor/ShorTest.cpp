#include "shor.h"

using namespace qram_simulator;
using namespace shor;


int main()
{
	random_engine::time_seed();
	int shots = 100; 
	size_t N = 1049 * 1789;
	// size_t N = 7*11;
	while (shots -- > 0)
	{
		fmt::print("==== Shot {} ====\n\n", shots);
		semi_classical_shor(N);
		// common_shor(N);
		System::clear();
		fmt::print("\n\n", shots);
		profiler::print_profiler();
		profiler::init_profiler();
	}


	return 0;
}