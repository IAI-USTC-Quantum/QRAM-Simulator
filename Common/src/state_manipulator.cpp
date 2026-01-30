#include "state_manipulator.h"

namespace qram_simulator {
	void QRAMFullAmp::_set_branches_full(
		const std::vector<std::complex<double>>& state,
		const std::vector<size_t>& address_qubits,
		const std::vector<size_t>& data_qubits
	) 
	{
		profiler _("_set_branches_full");
		size_t branchsz = pow2(address_qubits.size() + data_qubits.size());

		auto& branches = qram->get_branches();
		auto& branch_probs = qram->get_branch_probs();

		if (branches.size() != branchsz)
		{
			branches.clear();
			branch_probs.clear();

			branches.reserve(branchsz);
			branch_probs.resize(branchsz, 0);

			for (size_t i = 0; i < branchsz; ++i) {
				branches.emplace_back(i, data_qubits.size());
			}
		}
		else
		{
			for (size_t i = 0; i < branchsz; ++i)
			{
				branches[i].reset();
				branch_probs[i] = 0;
			}
		}

		std::vector<size_t> total_qubit = data_qubits;
		total_qubit.insert(total_qubit.end(), address_qubits.begin(), address_qubits.end());

		for (size_t i = 0; i < state.size(); ++i) {
			size_t branchid = quantum_simulator::extract_binary(i, total_qubit);
			branch_probs[branchid] += abs_sqr(state[i]);
			check_nan(branch_probs[branchid]);
		}
	}

	void QRAMFullAmp::_set_branches(const std::vector<std::complex<double>>& state,
		const std::vector<size_t>& address_qubits,
		const std::vector<size_t>& data_qubits)
	{

		if (address_qubits.size() + data_qubits.size() <= 25)
		{
			_set_branches_full(state, address_qubits, data_qubits);
			return;
		}

		profiler _("_set_branches");
		auto& branches = qram->get_branches();
		auto& branch_probs = qram->get_branch_probs();
		branches.clear();
		branch_probs.clear();

		std::map<size_t, double> map_branchid_prob;

		size_t data_size = data_qubits.size();
		std::vector<size_t> total_qubit = data_qubits;
		total_qubit.insert(total_qubit.end(), address_qubits.begin(), address_qubits.end());

		for (size_t i = 0; i < state.size(); ++i)
		{
			double prob = abs_sqr(state[i]);
			// if (prob < epsilon) continue;
			size_t branchid = quantum_simulator::extract_binary
			(i, total_qubit);
			auto&& [iter, flag] = map_branchid_prob.insert({ branchid, prob });
			if (!flag) { iter->second += prob; }
		}

		branches.reserve(map_branchid_prob.size());
		branch_probs.reserve(map_branchid_prob.size());
		for (auto&& [branchid, prob] : map_branchid_prob)
		{
			branches.emplace_back(branchid, data_size);
			branch_probs.push_back(prob);
		}
	}

	void QRAMFullAmp::_reconstruct(
		std::vector<std::complex<double>>& ret,
		const std::vector<std::complex<double>>& state,
		const std::vector<size_t>& address_qubits,
		const std::vector<size_t>& data_qubits,
		const std::vector<size_t>& other_qubits)
	{
		profiler _("_reconstruct");

		//fmt::print("-------------\n");
		for (size_t i = 0; i < qram->branches.size(); ++i)
		{
			auto& branch = qram->branches[i];
			size_t branchid = branch.get_branchid();

			size_t address = branch.address;
			size_t address_index = quantum_simulator::reconstruct_binary
			(address, address_qubits);

			size_t bus_input = branch.bus_input;
			size_t bus_input_index = quantum_simulator::reconstruct_binary
			(bus_input, data_qubits);
			if (!branch.is_good()) {
				for (auto iter = branch.iterbeg(); iter != branch.iterend(); ++iter) {
					size_t bus_output = iter->data_bus;
					size_t bus_output_index = quantum_simulator::reconstruct_binary
					(bus_output, data_qubits);

					for (size_t other = 0; other < pow2(other_qubits.size()); ++other)
					{
						size_t other_index = quantum_simulator::reconstruct_binary
						(other, other_qubits);
						//fmt::print("{} ({},{},{}) -> {} ({},{},{})\n", 
						//	address_index + bus_input_index + other_index, address, bus_input, other,
						//	address_index + bus_output_index + other_index, address, bus_output, other
						//);
						ret[address_index + bus_output_index + other_index] +=
							state[address_index + bus_input_index + other_index] * iter->amplitude;
						/*fmt::print("out: {} {} {} {}\nin: {} {} {} {}\namp:{}\n",
							address_index, bus_output_index, other_index, address_index + bus_output_index + other_index,
							address_index, bus_input_index, other_index, address_index + bus_input_index + other_index,
							iter->amplitude
						);*/
						check_nan(ret[address_index + bus_output_index + other_index]);
					}
				}
			}
			else {
				const auto& memory = qram->get_memory();
				const auto good_ref = branch.good_ref;
				size_t good_ref_address = good_ref->address;
				size_t good_ref_bus_input = good_ref->bus_input;
				auto multiplier = sqrt(branch.relative_multiplier);

				for (auto iter = good_ref->iterbeg(); iter != good_ref->iterend(); ++iter) {
					size_t good_ref_bus_output = iter->data_bus;
					size_t bus_output = bus_input ^ memory[address] ^
						(good_ref_bus_input ^ memory[good_ref_address] ^ good_ref_bus_output);
					size_t bus_output_index = quantum_simulator::reconstruct_binary
					(bus_output, data_qubits);

					for (size_t other = 0; other < pow2(other_qubits.size()); ++other)
					{
						size_t other_index = quantum_simulator::reconstruct_binary
						(other, other_qubits);

						ret[address_index + bus_output_index + other_index] +=
							state[address_index + bus_input_index + other_index] * multiplier * iter->amplitude;

						/*fmt::print("out: {} {} {} {}\nin: {} {} {} {}\nmul: {} amp:{}\n",
							address_index, bus_output_index, other_index, address_index + bus_output_index + other_index,
							address_index, bus_input_index, other_index, address_index + bus_input_index + other_index,
							multiplier, iter->amplitude
						);*/

						check_nan(ret[address_index + bus_output_index + other_index]);
					}
				}
			}
		}
		check_normalization(ret);
	}

	std::vector<std::complex<double>> QRAMFullAmp::apply(
		const std::vector<std::complex<double>>& state,
		const std::vector<size_t>& address_qubits,
		const std::vector<size_t>& data_qubits,
		const std::vector<size_t>& other_qubits,
		std::string version)
	{
		FunctionProfiler;
		if (address_qubits.size() != address_size ||
			data_qubits.size() != data_size)
			throw std::runtime_error("Size does not match.");

		_set_branches(state, address_qubits, data_qubits);
		qram->run(version);
		qram->sample_output();
		// fmt::print(qram->to_string());
		std::vector<std::complex<double>> ret(state.size(), 0);
		_reconstruct(ret, state, address_qubits, data_qubits, other_qubits);
		/*_reconstruct_v2(ret, state, address_qubits, data_qubits);*/
		return ret;
	}

} // namespace qram_simulator