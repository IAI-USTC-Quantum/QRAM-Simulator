#include "sparse_state_simulator.h"

namespace qram_simulator {
	namespace shor {
		using ExpModFunc = std::function<size_t(size_t)>;

		/* compute a^x mod N for any large x */
		size_t general_expmod(size_t a, size_t x, size_t N);

		class ShorExecutionFailed : public std::runtime_error
		{
		public:
			ShorExecutionFailed(const std::string& message) : std::runtime_error(message) {}
		};

		inline void throw_bad_shor_result(const std::string& message)
		{
			throw ShorExecutionFailed(message);
		}
				
		/* compute |x>|z> -> |x>|z ^ (a^x mod N)> */
		struct ExpMod : SelfAdjointOperator
		{
			size_t reg_input;
			size_t reg_output;
			ExpModFunc anc_func;

			ExpMod(size_t reg_input_, size_t reg_output_, ExpModFunc func)
				:reg_input(reg_input_), reg_output(reg_output_), anc_func(func)
			{}

			void operator()(std::vector<System>& state) const;
		};

		/* Seems good */
		struct SemiClassicalShor
		{
			size_t a;
			size_t n;
			size_t N;
			size_t size;
			size_t meas_result = 0;
			size_t period = 0;
			size_t p = 0;
			size_t q = 0;

			SemiClassicalShor(size_t a_, size_t N_, size_t n_)
				: a(a_), N(N_), n(n_), size(n_ * 2)
			{
			}

			size_t run();
			void postprocess();
		};

		/* Seems good */
		struct Shor
		{
			size_t work_reg;
			size_t ancilla_reg;
			ExpModFunc anc_func;

			Shor(size_t work_register, size_t ancilla_register, size_t a_, size_t N_, ExpModFunc func)
				: work_reg(work_register), ancilla_reg(ancilla_register), anc_func(func)
			{}

			void operator()(std::vector<System>& state) const;
		};

		std::pair<size_t, size_t> find_best_fraction(size_t y, size_t Q, size_t N);
		uint64_t compute_period(uint64_t meas_result, size_t size, size_t N);
		void check_period(uint64_t period, uint64_t a, uint64_t N);
		std::tuple<uint64_t, uint64_t> shor_postprocess(uint64_t meas, size_t size, uint64_t a, uint64_t N);

		inline int common_shor(size_t N, std::optional<size_t> ainput = std::nullopt)
		{
			size_t n = log2(N) + 1;
			size_t size = n * 2;

			uint64_t a;
			if (ainput.has_value())
				a = ainput.value();
			else
				a = uint64_t(random_engine::rng() * (N - 1) + 1);

			// check whether a and N are coprime
			if (std::gcd(a, N) != 1)
			{
				fmt::print("a = {} and N = {} are not coprime\n", a, N);
				return 1;
			}

			auto work_reg = System::add_register("work_reg", UnsignedInteger, size);
			auto anc_reg = System::add_register("anc_reg", UnsignedInteger, n);

			std::vector<System> state;
			state.emplace_back();

			std::vector<uint64_t> axmodn;
			uint64_t value = 1;
			axmodn.push_back(value);
			for (size_t i = 1; i < N; ++i)
			{
				uint64_t next_val = axmodn.back() * a % N;
				if (next_val == 1)
					break;
				else
					axmodn.push_back(next_val);
			}

			size_t r = axmodn.size();
			fmt::print("r = {} (N = {}, n = {}, a = {})\n", r, N, n, a);

			ExpModFunc func = [r, &axmodn](size_t x) -> size_t
				{
					x = x % r;
					return axmodn[x];
				};

			Shor(work_reg, anc_reg, a, N, func)(state);

			//(RemoveRegister(anc_reg))(state);
			//SortByAmplitude()(state);
			//StatePrint(Detail | Prob)(state);

			auto&& [meas_res, _] = PartialTrace(std::vector{ work_reg })(state);
			auto&& [p, q] = shor_postprocess(meas_res[0], size, a, N);
		}

		inline auto semi_classical_shor(size_t N, std::optional<size_t> ainput = std::nullopt)
		{
			size_t n = log2(N) + 1;

			size_t a;
			if (ainput.has_value())
				a = ainput.value();
			else
				a = size_t(random_engine::rng() * (N - 1) + 1);

			// check whether a and N are coprime
			if (std::gcd(a, N) != 1)
			{
				fmt::print("a = {} and N = {} are not coprime\n", a, N);
				return 1;
			}

			std::vector<uint64_t> axmodn;
			uint64_t value = 1;
			axmodn.push_back(value);
			for (size_t i = 1; i < N; ++i)
			{
				uint64_t next_val = axmodn.back() * a % N;
				if (next_val == 1)
					break;
				else
					axmodn.push_back(next_val);
			}

			size_t r = axmodn.size();

			fmt::print("r = {} (N = {}, n = {}, a = {})\n", r, N, n, a);

			SemiClassicalShor obj(a, N, n);
			uint64_t result = obj.run();
			obj.postprocess();

			return 0;
		}
	}
} // namespace qram_simulator