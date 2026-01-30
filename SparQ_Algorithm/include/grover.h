#pragma once

#include "state_manipulator.h"
#include "simple_quantum_simulator.h"
#include "sparse_state_simulator.h"
#include "hamiltonian_simulation.h"

namespace qram_simulator {
	namespace grover_dense {
		template<typename QRAM>
		void oracle(std::vector<complex_t>& state, size_t n, QRAM* qram, std::string version)
		{
			profiler _("Grover_shots: oracle");

			std::vector<size_t> addrqubit(n);
			iota(addrqubit.begin(), addrqubit.end(), 0);
			std::vector<size_t> dataqubit(1, n);
			std::vector<size_t> otherqubit;

			state = qram->apply(state, addrqubit, dataqubit, otherqubit, version);
			if (std::abs(amp_sum(state) - 1.0) > epsilon)
			{
				double ampsum = amp_sum(state);
				quantum_simulator::print_state(state, false);
				fmt::print("amp_sum={}\n", ampsum);
				fmt::print("{}\n", (*qram)->to_string_full_info());

				throw_bad_result();
			}
		}

		void diffusion(std::vector<complex_t>& state, size_t n);

		void grover(std::vector<complex_t>& state, size_t n, size_t pos, size_t repeat,
			std::function<void(decltype(state))> oracle);

		template<typename QRAM>
		std::vector<size_t> grover_shots(size_t n, size_t pos, size_t shots, size_t repeat,
			const std::map<OperationType, double>& noise, std::string version)
		{
			std::vector<size_t> measurements(pow2(n), 0);
			std::vector<complex_t> state;
			memory_t memory(pow2(n), 0);
			memory[pos] = 1;
			QRAM qram(n, 1, memory);
			qram->set_noise_models(noise);

			for (size_t i = 0; i < shots; ++i) {
				profiler _("Grover_shots: Mainloop");
				quantum_simulator::init_n_state(state, n + 1);
				auto oracle_ = std::bind(oracle<QRAM>, std::placeholders::_1, n, &qram, version);
				grover(state, n, pos, repeat, oracle_);
				size_t m = quantum_simulator::measure(state);
				if (m == state.size())
					throw_bad_result();
				// only extract the lowest n
				m -= ((m >> n) << n);
				++measurements[m];
			}
			return measurements;
		}
	}

	namespace grover
	{
		struct GroverOracle
		{
			size_t qram_address_id;
			size_t qram_data_id;
			size_t search_data_id;
			qram_qutrit::QRAMCircuit* qram;

			ClassControllable

			GroverOracle(qram_qutrit::QRAMCircuit* qram_, size_t qram_address_id_,
				size_t qram_data_id_, size_t search_data_id_)
				:
				qram(qram_),
				qram_address_id(qram_address_id_),
				qram_data_id(qram_data_id_),
				search_data_id(search_data_id_)
			{
			}

			GroverOracle(qram_qutrit::QRAMCircuit* qram_, std::string_view qram_address_,
				std::string_view qram_data_, std::string_view search_data_)
				:
				qram(qram_),
				qram_address_id(System::get(qram_address_)),
				qram_data_id(System::get(qram_data_)),
				search_data_id(System::get(search_data_))
			{
			}

			void operator()(std::vector<System>& state) const;

		};

		struct HPH
		{
			size_t qram_address_id;
			size_t size;

			ClassControllable

				HPH(size_t qram_address_id_)
				: qram_address_id(qram_address_id_),
				size(System::size_of(qram_address_id_))
			{ }

			HPH(std::string qram_address_name, size_t size_)
				: qram_address_id(System::get(qram_address_name)), size(size_)
			{ }

			void operator()(std::vector<System>& state) const;

		};

		struct GroverOperator
		{
			size_t qram_address_id;
			size_t qram_data_id;
			size_t search_data_id;
			qram_qutrit::QRAMCircuit* qram;

			ClassControllable

				GroverOperator(qram_qutrit::QRAMCircuit* qram_, size_t qram_address_id_,
					size_t qram_data_id_, size_t search_data_id_)
				: qram(qram_), qram_address_id(qram_address_id_),
				qram_data_id(qram_data_id_),
				search_data_id(search_data_id_)
			{
			}

			void operator()(std::vector<System>& state) const;
		};

		struct GroverAmplify
		{
			size_t n_repeats;

			size_t qram_address_id;
			size_t qram_data_id;
			size_t search_data_id;
			size_t data_size;
			qram_qutrit::QRAMCircuit* qram;

			GroverAmplify(qram_qutrit::QRAMCircuit* qram_, size_t qram_address_id_, size_t search_data_id_,
				size_t data_size_, size_t n_repeats_)
				: qram(qram_), qram_address_id(qram_address_id_), n_repeats(n_repeats_),
				search_data_id(search_data_id_), data_size(data_size_)
			{}

			void operator()(std::vector<System>& state);

		};

		struct GroverCount
		{
			/*
			-- H -- (c-U) - iQFT
			*/
			size_t count_reg;
			size_t addr_reg;
			size_t data_reg;
			size_t search_data_reg;
			qram_qutrit::QRAMCircuit* qram;

			GroverCount(qram_qutrit::QRAMCircuit* qram_, size_t count_reg_, size_t addr_reg_,
				size_t data_reg_, size_t search_data_reg_)
				: qram(qram_), count_reg(count_reg_), addr_reg(addr_reg_),
				data_reg(data_reg_), search_data_reg(search_data_reg_)
			{}

			void operator()(std::vector<System>& state);
		};
	}

} // namespace qram_simulator