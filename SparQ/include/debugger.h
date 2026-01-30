#pragma once
#include "basic_components.h"
#include "matrix.h"
#include "partial_trace.h"
#include "sort_state.h"

namespace qram_simulator
{
	struct ModuleInheritance_Test : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		inline void operator()(std::vector<System>& state) const
		{
			fmt::print("ModuleInheritance_Test::operator()\n");
		}
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct ModuleInheritance_Test_SelfAdjoint : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		inline void operator()(std::vector<System>& state) const
		{
			fmt::print("ModuleInheritance_Test_SelfAdjoint::operator()\n");
		}
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct CheckNormalization : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		double threshold = 1e-5;
		CheckNormalization();
		CheckNormalization(double threshold_) : threshold(threshold_) {}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct CheckNormalization_Renormalize : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		double threshold = 1e-5;
		CheckNormalization_Renormalize() {}
		CheckNormalization_Renormalize(double threshold_) : threshold(threshold_) {}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct CheckNan : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		CheckNan();
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct ViewNormalization : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		ViewNormalization();
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	enum StatePrintDisplay : int32_t
	{
		Default = 0,
		Detail = 1,
		Binary = 2,
		Prob = 4,
	};


	struct StatePrint : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		static bool on;
		int32_t display;
		int precision;
		StatePrint(int32_t disp = 0) : display(disp), precision(0) {}
		StatePrint(int32_t disp, int precision) : display(disp), precision(precision) {}
		StatePrint(StatePrintDisplay disp) : display(static_cast<int32_t>(disp)), precision(0) {}
		std::string disp2str() const;
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct TestRemovable : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t register_id;
		TestRemovable(std::string_view register_name);
		TestRemovable(size_t register_name);
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct CheckDuplicateKey : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		CheckDuplicateKey() {}
		bool has_duplicate(std::vector<System>& system_states) const;
		void operator()(std::vector<System>& system_states) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	template<typename OperationNq1q>
	std::vector<size_t> check_inplace_unitarity(bool dagger)
	{
		/* An inplace operation with two registers (N, 1) */
		/* Check whether the operation is unitary */
		/* Return the truth table of the operation */
		System::clear();

		size_t N = 2;
		size_t reg1 = System::add_register("reg1", UnsignedInteger, N);
		size_t reg2 = System::add_register("reg2", UnsignedInteger, 1);

		OperationNq1q op("reg1", "reg2");

		std::vector<bool> expected_output(pow2(N + 1), false);
		std::vector<size_t> truth_table(pow2(N + 1), 0);

		for (size_t i = 0; i < pow2(N); ++i)
		{
			for (size_t j = 0; j < pow2(1); ++j)
			{
				std::vector<System> state;
				state.emplace_back();
				state.back().get(reg1).value = i;
				state.back().get(reg2).value = j;

				if (dagger)
					op.dag(state);
				else
					op(state);

				/* Check whether state is still of size 1*/
				if (state.size() != 1)
				{
					throw_bad_result("Bad check inplace unitarity: state size is not 1");
				}

				/* Get output state */
				size_t out_i = state[0].get(reg1).value;
				size_t out_j = state[0].get(reg2).value;
				size_t out = (out_j << N) + out_i;
				size_t in = (j << N) + i;
				/* Check whether out exists in expected_output */
				if (expected_output[out])
				{
					fmt::print("truth_table: {}", truth_table);
					throw_bad_result("Bad check inplace unitarity: output state already exists");
				}
				expected_output[out] = true;
				truth_table[in] = out;
			}
		}
		System::clear();
		return truth_table;
	}

	template<typename BlockEncoding, typename StateType = SparseState>
	DenseMatrix<complex_t> _extract_block_encoding(BlockEncoding encA, std::string_view main_reg, std::string_view anc_UA,
		bool is_full = false, bool is_dag = false)
	{
		size_t main_reg_num = System::size_of(main_reg);
		size_t anc_UA_num = System::size_of(anc_UA);
		size_t main_reg_pos = System::get(main_reg);
		size_t anc_UA_pos = System::get(anc_UA);
		if (is_full == true)
		{
			DenseMatrix<complex_t> ret(pow2(main_reg_num + anc_UA_num));
			auto range_a = range(pow2(anc_UA_num));
			auto range_m = range(pow2(main_reg_num));

			for (auto [a, m] : product(range_a, range_m))
			{
				StateType state(1);

				state.back().get(main_reg_pos).value = m;
				state.back().get(anc_UA_pos).value = a;
				if (is_dag)
					encA.dag(state);
				else
					encA(state);
				std::vector<complex_t> vec(pow2(main_reg_num + anc_UA_num), 0);
				for (auto& s : state)
				{
					size_t _index = concat_value(
						{
							{s.get(main_reg_pos).value, main_reg_num},
							{s.get(anc_UA_pos).value, anc_UA_num},
						}
						);
					vec[_index] = s.amplitude;
				}
				size_t index = concat_value(
					{
						{m, main_reg_num},
						{a, anc_UA_num},
					}
					);
				for (size_t j = 0; j < pow2(main_reg_num + anc_UA_num); ++j)
				{
					ret(j, index) = vec[j];
				}
			}
			return ret;
		}
		else
		{
			DenseMatrix<complex_t> ret(pow2(main_reg_num));

			for (auto i : range(pow2(main_reg_num)))
			{
				StateType state(1);

				state.back().get(main_reg_pos).value = i;
				state.back().get(anc_UA_pos).value = 0;

				if (is_dag)
					encA.dag(state);
				else
					encA(state);

				double prob = PartialTraceSelect({ {anc_UA, 0} })(state);

				std::vector<complex_t> vec(pow2(main_reg_num), 0);
				for (auto& s : state)
				{
					vec[s.get(main_reg_pos).value] = s.amplitude;
				}
				for (size_t j = 0; j < pow2(main_reg_num); ++j)
				{
					ret(j, i) = vec[j] / prob;
				}
			}
			return ret;
		}
	}


	template<typename BlockEncoding>
	DenseMatrix<complex_t> extract_block_encoding(BlockEncoding encA, std::string_view main_reg, std::string_view anc_UA,
		bool is_full = false, bool is_dag = false)
	{
		return _extract_block_encoding<BlockEncoding, SparseState>(encA, main_reg, anc_UA, is_full, is_dag);
	}

	inline void state_equal_check(std::vector<System> state1, std::vector<System> state2)
	{	
		SortUnconditional()(state1);
		SortUnconditional()(state2);
		if (state1.size() != state2.size())
		{
			fmt::print("Size not equal: {} vs {}", state1.size(), state2.size());
			goto CHECK_FAILED;
		}
		for (size_t i = 0; i < state1.size(); ++i)
		{
			if (state1[i] != state2[i])
			{
				fmt::print("State not equal at index {}:\n", i);
				goto CHECK_FAILED;
			}
		}
		return;
	CHECK_FAILED:
		fmt::print("State 1:\n");
		StatePrint(0 | Detail)(state1);
		fmt::print("State 2:\n");
		StatePrint(0 | Detail)(state2);
		throw_general_runtime_error("Hadamard_Bool: GPU-version failed!");
	}
}

#ifdef USE_CUDA

namespace qram_simulator {

	struct ParallelOperationTest : BaseOperator {
		double real, imag;
		uint64_t value;

		ParallelOperationTest(double real_, double imag_, uint64_t value_) :
			real(real_), imag(imag_), value(value_) {
		}

		void operator()(std::vector<System>& state) const {
			throw_not_implemented();
		}
		void operator()(CuSparseState& state) const;
	};


}

#endif