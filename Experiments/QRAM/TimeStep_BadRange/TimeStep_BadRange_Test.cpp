#include "time_step.h"
using namespace qram_simulator;

auto test_time_step() {
	size_t n_trials = 10000;
	static std::uniform_real_distribution<double> ud(0, 1);
	double p = 1.0e-3;
	//double p = 1.0;
	double node_p = 2 * p - p * p;
	for (size_t n = 3; n < 20; ++n) {

		size_t sum_count1 = 0;
		size_t sum_count2 = 0;

		for (size_t i = 0; i < n_trials; ++i) {

			size_t k = 1;
			TimeStep step(n, k);
			double sum = 0;
			size_t n_qubit = 2 * (pow2(n) - 1);
			std::vector<int> bad1(pow2(n), 0);
			std::vector<int> bad2(pow2(n), 0);
			for (size_t i = 0; i < n_qubit; ++i)
			{
				if (ud(random_engine::get_engine()) < p) {
					{
						auto&& [l, r] = step.get_bad_range_qubit(i);
						//fmt::print("{},{},{}\n", i, l, r);
						std::fill(bad1.begin() + l, bad1.begin() + r + 1, 1);
					}
					{
						auto&& [l, r] = step.get_bad_range_qutrit(i);
						//fmt::print("{},{},{}\n", i, l, r);
						std::fill(bad2.begin() + l, bad2.begin() + r + 1, 1);
					}
				}
			}
			auto ct1 = std::count(bad1.begin(), bad1.end(), 1);
			auto ct2 = std::count(bad2.begin(), bad2.end(), 1);
			sum_count1 += ct1;
			sum_count2 += ct2;
		}

		fmt::print("n={}, avg(qubit)={}, avg(qutrit)={}, guess={}\n", n,
			sum_count1 * 1.0 / n_trials / pow2(n),
			sum_count2 * 1.0 / n_trials / pow2(n),
			1 - std::pow(1 - node_p, n)
		);
	}
	return 0;
}

int main() {
	test_time_step();
	return 0;
}