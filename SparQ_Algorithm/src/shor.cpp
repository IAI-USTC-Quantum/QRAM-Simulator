#include "shor.h"

namespace qram_simulator
{
	namespace shor
	{
		size_t general_expmod(size_t a, size_t x, size_t N)
		{
			if (x == 0)
				return 1;
			if (x == 1)
				return a % N;
			if (x & 1)
			{
				return (general_expmod(a, x - 1, N) * a) % N;
			}
			else
			{
				return (general_expmod(a, x / 2, N) * general_expmod(a, x / 2, N)) % N;
			}
		}

		void ExpMod::operator()(std::vector<System>& state) const
		{
#ifdef SINGLE_THREAD
			for (auto& s : state)
			{
#else
#pragma omp parallel for
			for (int i = 0; i < state.size(); ++i)
			{
				auto& s = state[i];
#endif
				auto val = s.GetAs(reg_input, size_t);
				s.get(reg_output).value ^= anc_func(val);
			}
		}

		std::pair<size_t, size_t> find_best_fraction(size_t y, size_t Q, size_t N) {
			double target = (double)y / Q;
			double low = 0.0;
			double high = 1.0;
			size_t low_num = 0;
			size_t low_den = 1;
			size_t high_num = 1;
			size_t high_den = 1;

			size_t best_num = 0;
			size_t best_den = 1;
			double best_diff = 1.0; // ��ʼ��ֵ��Ϊ 1

			while (true) {
				size_t mediant_num = low_num + high_num;
				size_t mediant_den = low_den + high_den;

				if (mediant_den > N) {
					break; // ��ĸ���� N��ֹͣ����
				}

				double mediant_value = (double)mediant_num / mediant_den;
				double diff = std::abs(mediant_value - target);

				if (diff < best_diff) {
					best_diff = diff;
					best_num = mediant_num;
					best_den = mediant_den;
				}

				if (mediant_value < target) {
					low_num = mediant_num;
					low_den = mediant_den;
				}
				else {
					high_num = mediant_num;
					high_den = mediant_den;
				}
			}

			return { best_den, best_num }; // ���� (��ĸ, ����)
		}


		uint64_t compute_period(uint64_t meas_result, size_t size, size_t N)
		{
			if (meas_result == 0)
			{
				throw_bad_shor_result("Fail for y = 0.\n");
			}
			uint64_t Q = pow2(size);
			uint64_t y = meas_result;
			uint64_t period = 0;

			std::pair<uint64_t, uint64_t> best_fraction = find_best_fraction(y, Q, N);
			uint64_t r = best_fraction.first;
			uint64_t c = best_fraction.second;

			if (r > 0 && r < N) {
				period = r;
				std::cout << "Found period: " << period << ", c: " << c << std::endl;
			}
			else {
				throw_bad_shor_result("Failed to find a suitable period.");
			}

			return period;
		}

		void check_period(uint64_t period, uint64_t a, uint64_t N)
		{
			if (period > N)
			{
				throw_bad_shor_result("Fail for r > N.\n");
			}

			if (period % 2 == 1)
			{
				throw_bad_shor_result(fmt::format("Fail for odd period (r = {}).\n", period));
			}

			size_t out = 1;
			for (size_t i = 0; i < period / 2; ++i)
			{
				out *= a;
				out %= N;
			}

			if (out == N - 1)
			{
				throw_bad_shor_result("Fail for a^(r/2)=-1 mod N.\n");
			}
		}

		std::tuple<uint64_t, uint64_t> shor_postprocess(uint64_t meas, size_t size, uint64_t a, uint64_t N)
		{
			try {
				uint64_t period = compute_period(meas, size, N);
				check_period(period, a, N);
				uint64_t a_exp_r_modN = general_expmod(a, period / 2, N);
				fmt::print("a^(r/2) mod N = {}\n", a_exp_r_modN);
				uint64_t p = std::gcd(a_exp_r_modN + 1, N);
				uint64_t q = std::gcd(a_exp_r_modN - 1, N);
				fmt::print("p={}, q={}, p*q = {}\n", p, q, p * q);

				return { p, q };
			}
			catch (const ShorExecutionFailed& e)
			{
				fmt::print("Shor failed: {}\n", e.what());
				return std::make_tuple(1, 1);
			}
		}

		size_t SemiClassicalShor::run()
		{
			profiler _("Semiclassical-Shor");
			auto ancilla_reg = System::add_register("anc_reg", UnsignedInteger, n);

			std::vector<System> state;
			state.emplace_back();
			std::vector<uint64_t> res;
			Xgate_Bool(ancilla_reg, 0)(state);
			for (size_t x = 0; x < size; ++x)
			{
				size_t work_reg = AddRegisterWithHadamard("work_reg", UnsignedInteger, 1)(state);
				Mod_Mult_UInt_ConstUInt(ancilla_reg, a, size - 1 - x, N).conditioned_by_all_ones(work_reg)(state);
				for (size_t i = 0; i < res.size(); ++i)
				{
					if (res[res.size() - 1 - i] == 1)
						Phase_Bool(work_reg, -2 * pi / pow2(i + 2))(state);
				}
				(Hadamard_Bool(work_reg))(state);
				auto&& [measured_results, prob] = PartialTrace(std::vector{ work_reg })(state);
				res.push_back(measured_results[0]);
				(RemoveRegister(work_reg))(state);
				//fmt::print("State size = {}\n", state.size());
			}

			uint64_t ret = 0;
			for (size_t i = 0; i < res.size(); ++i)
			{
				ret += res[i] * pow2(i);
			}
			// fmt::print("res = {} | ret = {}\n", res, ret);
			return meas_result = ret;
		}

		void SemiClassicalShor::postprocess()
		{
			std::tie(p, q) = shor_postprocess(meas_result, size, a, N);
		}


		void Shor::operator()(std::vector<System>& state) const
		{
			size_t precision_sz = System::size_of(work_reg);
			(Hadamard_Int_Full(work_reg))(state);
			fmt::print("Hadamard finished.\n");
			ExpMod(work_reg, ancilla_reg, anc_func)(state);
			fmt::print("ExpMod finished.\n");
			auto&& [measured_results, prob] = PartialTrace(std::vector{ ancilla_reg })(state);
			fmt::print("PartialTrace finished.\n");
			QFT_Full(work_reg).dag(state);
			fmt::print("inverseQFT finished.\n");
		}
	}
}