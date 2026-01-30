#include "grover.h"
namespace qram_simulator {	
	namespace grover_dense {
		using namespace quantum_simulator;
		void diffusion(std::vector<complex_t>& state, size_t n)
		{
			hadamard_all(state, n);
			for (size_t i = 0; i < state.size(); i += pow2(n))
			{
				state[i] *= -1;
			}
			hadamard_all(state, n);
			// fmt::print("AfterDiffusion\n");
			// print_state(state);
		}

		void grover(std::vector<complex_t>& state, size_t n, size_t pos, size_t repeat,
			std::function<void(decltype(state))> oracle)
		{
			// fmt::print("GroverBegin\n");
			size_t N = pow2(n);
			unitary1q(state, n, not_gate);
			unitary1q(state, n, hadamard);
			hadamard_all(state, n);
			// print_state(state);
			for (size_t i = 0; i < repeat; ++i)
			{
				oracle(state);
				// fmt::print("AfterOracle\n");
				// print_state(state);
				diffusion(state, n);
			}
			unitary1q(state, n, hadamard);
			unitary1q(state, n, not_gate);
			// print_state(state);
		}
	}

	namespace grover {
		void GroverOracle::operator()(std::vector<System>& state) const
		{
			QRAMLoad(qram, qram_address_id, qram_data_id)(state);
			auto compare_less = AddRegister("compare_less", Boolean, 1)(state);
			auto compare_eql = AddRegister("compare_eql", Boolean, 1)(state);
			Compare_UInt_UInt(qram_data_id, search_data_id, compare_less, compare_eql)(state);

			ZeroConditionalPhaseFlip(std::vector{ compare_eql })
				.conditioned_by_nonzeros(condition_variable_nonzeros)
				(state);

			Compare_UInt_UInt(qram_data_id, search_data_id, compare_less, compare_eql)(state);
			(RemoveRegister(compare_eql))(state);
			(RemoveRegister(compare_less))(state);
			QRAMLoad(qram, qram_address_id, qram_data_id)(state);
		}

		void HPH::operator()(std::vector<System>& state) const
		{
			(Hadamard_Int_Full(qram_address_id))(state);
			ZeroConditionalPhaseFlip(std::vector{ qram_address_id })
				.conditioned_by_nonzeros(condition_variable_nonzeros)(state);
			(Hadamard_Int_Full(qram_address_id))(state);
		}

		void GroverOperator::operator()(std::vector<System>& state) const
		{
			GroverOracle(qram, qram_address_id, qram_data_id, search_data_id)
				.conditioned_by_nonzeros(condition_variable_nonzeros)(state);

			HPH(qram_address_id).conditioned_by_nonzeros(condition_variable_nonzeros)(state);
		}

		void GroverAmplify::operator()(std::vector<System>& state)
		{
			(Hadamard_Int_Full(qram_address_id))(state);
			AddRegister add_data("data", UnsignedInteger, data_size);
			RemoveRegister remove_data("data");

			for (size_t i = 0; i < n_repeats; ++i)
			{
				qram_data_id = add_data(state);
				GroverOperator(qram, qram_address_id,
					qram_data_id, search_data_id)(state);
				remove_data(state);
			}
		}

		void GroverCount::operator()(std::vector<System>& state)
		{
			size_t precision_sz = System::size_of(count_reg);
			(Hadamard_Int_Full(count_reg))(state);
			(Hadamard_Int_Full(addr_reg))(state);

			GroverOperator op(qram, addr_reg, data_reg, search_data_reg);

			for (size_t i = 0; i < precision_sz; ++i)
			{
				for (size_t j = 0; j < pow2(i); ++j)
				{
					op.conditioned_by_bit(count_reg, j)(state);
				}
			}

			// auto&& [measured_results, prob] = PartialTrace(std::vector{ addr_reg, data_reg, search_data_reg })(state);

			(inverseQFT(count_reg))(state);
		}
	}
}//namespace qram_simulator