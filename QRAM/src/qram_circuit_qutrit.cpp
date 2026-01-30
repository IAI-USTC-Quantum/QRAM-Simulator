#include "qram_circuit_qutrit.h"

namespace qram_simulator {
	namespace qram_qutrit {
		void QRAMCircuit::set_memory_random()
		{
			random_memory(memory, data_size);
		}

		void QRAMCircuit::set_memory(const memory_t& new_memory)
		{
			if (new_memory.size() != pow2(address_size))
				throw_invalid_input();

			memory = new_memory;
		}

		void QRAMCircuit::set_memory(memory_t&& new_memory)
		{
			if (new_memory.size() != pow2(address_size))
				throw_invalid_input();

			memory = std::move(new_memory);
		}

		void QRAMCircuit::initialize_system()
		{
			profiler _("QRAMCircuit::initialize_system");
			operations = time_step.generate(noise_parameters, Branch::arch_type);
			first_good_branch = -1;
			valid_branch_view.clear();
			good_branch_ids.clear();
			final_system_state.clear();
			std::for_each(branches.begin(), branches.end(),
				[](Branch& branch) { branch.reset(); }
			);
		}

		void QRAMCircuit::set_input_random(size_t n_inputs)
		{
			// use rho, theta to initialize random inputs
			static std::uniform_real_distribution<double> urd(0, 1);

			if (address_size + data_size > 64)
				throw_invalid_input();

			size_t max_branch_size = pow2(address_size + data_size);
			auto inputsz = std::min(n_inputs, max_branch_size);
			branches.reserve(inputsz);
			branches.clear();
			branch_probs.resize(inputsz, 0);

			if (max_branch_size < pow2(30) &&
				inputsz * 1.0 / max_branch_size > 0.3)
			{
				static std::vector<size_t> indices;

				if (indices.size() != max_branch_size)
					indices.resize(max_branch_size);

				std::iota(indices.begin(), indices.end(), 0);

				double total_prob = 0;
				// 0123456 from 0123456 select 3, swap 3&0
				// 3120456 from  120456 select 4, swap 4&1
				// 3420156 from   20156 select 0, swap 0&2
				// 3402156 from    2156 select ...  
				for (size_t i = 0; i < inputsz; ++i)
				{
					std::uniform_int_distribution<size_t> ud(i, indices.size() - 1);
					// generate random new
					size_t a = ud(random_engine::get_engine());

					// new input, generate the probability
					double r = urd(random_engine::get_engine());

					// use _indices[a] to mark the selected number
					branches.emplace_back(indices[a], data_size);
					branch_probs[i] = r;
					total_prob += r;

					// remove the selected
					std::swap(indices[i], indices[a]);
				}
				for (size_t i = 0; i < inputsz; ++i)
				{
					branch_probs[i] /= total_prob;
				}
			}
			else
			{
				double total_prob = 0;
				std::set<size_t> unique_address;

				for (size_t i = 0; i < inputsz; ++i)
				{
					std::uniform_int_distribution<size_t> ud(0, max_branch_size - 1);

					size_t a = ud(random_engine::get_engine());
					if (unique_address.find(a) != unique_address.end())
						continue;

					// new input, generate the probability
					double r = urd(random_engine::get_engine());

					// use _indices[a] to mark the selected number
					branches.emplace_back(a, data_size);
					branch_probs[i] = r;
					total_prob += r;
				}
				for (size_t i = 0; i < inputsz; ++i)
				{
					branch_probs[i] /= total_prob;
				}
			}
		}

		void QRAMCircuit::set_input_uniform(size_t n_inputs)
		{
			// use rho, theta to initialize random inputs

			size_t max_branch_size = pow2(address_size + data_size);
			if (address_size + data_size > 64)
				throw_invalid_input();

			auto inputsz = std::min(n_inputs, max_branch_size);
			branches.clear();
			branches.reserve(inputsz);
			branch_probs.resize(inputsz, 0);
			static std::vector<size_t> indices;

			if (max_branch_size < pow2(30) &&
				inputsz * 1.0 / max_branch_size > 0.3)
			{
				if (indices.size() != max_branch_size)
					indices.resize(max_branch_size);

				std::iota(indices.begin(), indices.end(), 0);

				double total_prob = 0;
				// 0123456 from 0123456 select 3, swap 3&0
				// 3120456 from  120456 select 4, swap 4&1
				// 3420156 from   20156 select 0, swap 0&2
				// 3402156 from    2156 select ...  
				for (size_t i = 0; i < inputsz; ++i)
				{
					std::uniform_int_distribution<size_t> ud(i, indices.size() - 1);
					// generate random new
					size_t a = ud(random_engine::get_engine());

					// new input, generate the probability
					double r = 1.0;

					// use _indices[a] to mark the selected number
					branches.emplace_back(indices[a], data_size);
					branch_probs[i] = r;
					total_prob += r;

					// remove the selected
					std::swap(indices[i], indices[a]);
				}
				for (size_t i = 0; i < inputsz; ++i)
				{
					branch_probs[i] /= total_prob;
				}
			}
			else
			{
				double total_prob = 0;
				std::set<size_t> unique_id;

				for (size_t i = 0; i < inputsz; ++i)
				{
					std::uniform_int_distribution<size_t> ud(0, max_branch_size - 1);

					size_t a = ud(random_engine::get_engine());
					if (unique_id.find(a) != unique_id.end())
						continue;

					unique_id.insert(a);

					// new input, generate the probability
					double r = 1.0;

					// use _indices[a] to mark the selected number
					branches.emplace_back(a, data_size);
					branch_probs[i] = r;
					total_prob += r;

				}
				for (size_t i = 0; i < inputsz; ++i)
				{
					branch_probs[i] /= total_prob;
				}
			}
		}

		double QRAMCircuit::sample_and_get_fidelity()
		{
			profiler _("sample_and_get_fidelity");

			sample_output();

			if (valid_branch_view.size() == 0)
				return 0;

			complex_t ret = 0;
			for (size_t i = 0; i < branches.size(); ++i)
			{
				ret += branch_probs[i] * branches[i].get_fidelity(memory)
					* std::sqrt(branches[i].relative_multiplier);
			}
			return abs_sqr(ret);
		}

		void QRAMCircuit::pick_good_bad()
		{
			profiler _("QRAMCircuit::pick_good_bad");
			initialize_system();

			for (size_t i = 0; i < branches.size(); ++i)
			{
				if (!time_step.is_bad_branch(branches[i].address))
				{
					if (first_good_branch < 0)
						first_good_branch = i;
					else {
						// this needs to gaurantee branches vector should
						// never change size, reallocate or move.
						branches[i].set_good(branches.data() + first_good_branch);
						good_branch_ids.push_back(i);

						/* TODO: wrong implementation */

						// branches[i].system_states.front().bus ^= memory[branches[i].address];
						continue;
					}
				}
				valid_branch_view.emplace_back(branches.data() + i);
			}
		}

		void QRAMCircuit::pick_all()
		{
			profiler _("QRAMCircuit::pick_all");
			initialize_system();

			for (size_t i = 0; i < branches.size(); ++i)
			{
				valid_branch_view.emplace_back(branches.data() + i);
			}
		}

		//void QRAMCircuit::prepare_good_only()
		//{
		//	profiler _("QRAMCircuit::prepare_good_only");
		//	initialize_system();

		//	for (size_t i = 0; i < branches.size(); ++i)
		//	{
		//		if (!time_step.is_bad_branch(branches[i].address))
		//		{
		//			if (first_good_branch < 0)
		//			{
		//				first_good_branch = i;
		//				valid_branch_view.emplace_back(branches.data() + i);
		//			}
		//			else {
		//				// this needs to gaurantee branches vector should
		//				// never change size, reallocate or move.
		//				branches[i].set_good(branches.data() + first_good_branch);
		//				good_branch_ids.push_back(i);
		//				continue;
		//			}
		//		}
		//		else
		//		{
		//			branches[i].remove_all_state();
		//		}
		//	}

		//	/* No good branches exists, which shouldn't happen exactly */
		//	if (valid_branch_view.size() == 0)
		//	{
		//		pick_all();
		//		fmt::print("Warning: run pick_all() while there is no good branch.\n");
		//	}
		//}

		void QRAMCircuit::run_acopy(int layer)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				bool a = get_digit_reverse(branch_ptr->address, layer, address_size);

				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[a](SubBranch& state)
					{state.run_acopy(a); });
			}
		}

		void QRAMCircuit::run_swap(size_t layer_id)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[layer_id](SubBranch& state)
					{state.run_swap(layer_id); });
			}
		}

		void QRAMCircuit::run_cswap(size_t layer_id)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[layer_id](SubBranch& state)
					{state.run_cswap(layer_id); });
			}
		}

		void QRAMCircuit::run_bitflip(size_t qubit_id)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[qubit_id](SubBranch& state)
					{state.run_bitflip(qubit_id); });
			}
		}

		void QRAMCircuit::run_phaseflip(size_t qubit_id, double depol_id)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[qubit_id, depol_id](SubBranch& state)
					{state.run_phaseflip(qubit_id, depol_id); });
			}
		}

		void QRAMCircuit::run_bitphaseflip(size_t qubit_id)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[qubit_id](SubBranch& state)
					{state.run_bitphaseflip(qubit_id); });
			}
		}

		void QRAMCircuit::run_depolarizing(size_t qubit_id, double depol_id)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[qubit_id, depol_id](SubBranch& state)
					{state.run_depolarizing(qubit_id, depol_id); });
			}
		}

		void QRAMCircuit::run_hadamard()
		{
			for (auto branch_ptr : valid_branch_view)
			{
				branch_ptr->run_hadamard();
				branch_ptr->try_merge();
			}
		}

		void QRAMCircuit::run_fetchdata(size_t digit)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				branch_ptr->run_fetchdata(memory, digit);
			}
		}

		void QRAMCircuit::run_busin(size_t digit)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[digit](SubBranch& state) {state.run_busin(digit); });
			}
		}

		void QRAMCircuit::run_busout(size_t digit)
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[digit](SubBranch& state) {state.run_busout(digit); });
			}
		}

		void QRAMCircuit::run_damp_full(size_t qubit_id, size_t step, double gamma) {
			// If entering this function, there must be damping error

			// get_multipliers
			// time_step.get_multiplier_qubit()

			/* Forward to BranchType static method
			*  to allow type check
			*/

			/* check the state of first_good_branch
			* skip the case of no damping
			*/

			/* Probability of damping */
			std::array<double, Branch::damp_op_num> prob_damp;
			prob_damp.fill(0);

			/* if there is good branch */
			if (first_good_branch >= 0)
			{
				Branch::get_multiplier(
					gamma,
					time_step,			/*instance reference*/
					step,				/*time step*/
					branches,			/*compute the branch_id*/
					first_good_branch,	/*the first good branch*/
					good_branch_ids,	/*indices of good branches*/
					/*multipliers,*/
					memory);

				/* Compute the probability of the damping */
				auto&& ref_prob = branches[first_good_branch].get_prob_damp(qubit_id);

				/* From good branches, add to the prob_damp */
				for (size_t i = 0; i < good_branch_ids.size(); ++i)
				{
					size_t id = good_branch_ids[i];
					for (auto k = 0; k < prob_damp.size(); ++k)
					{
						prob_damp[k] += ref_prob[k] /** multipliers[i] */
							* branches[id].relative_multiplier
							* branch_probs[id];
					}
				}
			}

			/* From valid (bad) branches, add to the prob_damp */
			for (auto branch_ptr : valid_branch_view)
			{
				size_t branch_id = branch_ptr - &branches[0];
				auto&& prob = branch_ptr->get_prob_damp(qubit_id);
				for (auto k = 0; k < prob_damp.size(); ++k) {
					prob_damp[k] += prob[k] * branch_probs[branch_id];
				}
			}

			double global_coef = get_normalization_factor_with_damping();
			double r = random_engine::get_instance().uniform01() * global_coef;
			for (size_t k = 0; k < prob_damp.size(); ++k)
			{
				if (r < prob_damp[k])
				{
					for (auto branch_ptr : valid_branch_view)
					{
						branch_ptr->run_damp_full(qubit_id, k);
					}
					break;
				}
				r -= prob_damp[k];
			}
		}

		void QRAMCircuit::run_damp_common(double gamma) {
			/*profiler _("run_damp_common");
			for (size_t i = 0; i < branches.size(); ++i) {
				if (branches[i].good)
					continue;
				if (branch_probs[i] < epsilon)
					continue;
				branches[i].run_damp_common(coef);
			}*/

			for (auto branch_ptr : valid_branch_view)
			{
				branch_ptr->run_damp_common(gamma);
			}
		}

		void QRAMCircuit::clear_zero_elements()
		{
			for (auto branch_ptr : valid_branch_view)
			{
				std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
					[](SubBranch& state) {
						state.state.clear_zero_elements();
					});
			}
		}

		double QRAMCircuit::get_normalization_factor() const
		{
			if (has_damping())
			{
				return get_normalization_factor_with_damping();
			}
			else
			{
				return get_normalization_factor_without_damping();
			}
		}

		double QRAMCircuit::get_normalization_factor_without_damping() const
		{
			double new_prob = 0;
			for (size_t i = 0; i < branches.size(); ++i) {
				new_prob += branches[i].get_prob() * branch_probs[i];
			}
			return new_prob;
		}

		double QRAMCircuit::get_normalization_factor_with_damping() const
		{
			double new_prob = 0;
			size_t j = 0;
			for (auto branch_ptr : valid_branch_view) {
				new_prob += branch_ptr->get_prob() * branch_probs[branch_ptr - branches.data()];
				check_nan(new_prob);
			}
			if (first_good_branch >= 0)
			{
				double first_good_branch_prob = branches[first_good_branch].get_prob();
				for (size_t i = 0; i < good_branch_ids.size(); ++i)
				{
					new_prob +=
						first_good_branch_prob /* *multipliers[i]*/
						* branches[good_branch_ids[i]].relative_multiplier
						* branch_probs[good_branch_ids[i]];
				}
			}
			return new_prob;
		}

		void QRAMCircuit::normalization()
		{
			if (has_damping())
			{
				normalization_with_damping();
			}
			else
			{
				normalization_without_damping();
			}
		}

		void QRAMCircuit::normalization_without_damping()
		{
			double multiplier = get_normalization_factor();

			for (size_t i = 0; i < branches.size(); ++i) {
				for (auto iter = branches[i].iterbeg(); iter != branches[i].iterend(); ++iter)
				{
					iter->amplitude /= sqrt(multiplier);
				}
			}
		}

		void QRAMCircuit::normalization_with_damping()
		{
			// branch_probs_out.resize(branch_probs.size(), 0);
			double multiplier = get_normalization_factor_with_damping();

			/*if (multiplier < epsilon * epsilon)
			{
				fmt::print("[Full Error Info]\n{}\n", to_string_full_info());
				throw_bad_result();
			}*/

			for (auto branch_ptr : valid_branch_view) {
				int i = branch_ptr - branches.data();

				//double p_state = branch_ptr->get_prob();
				//if (p_state > epsilon) {
				for (auto iter = branch_ptr->iterbeg(); iter != branch_ptr->iterend(); ++iter)
				{
					iter->amplitude /= sqrt(multiplier);
				}
				//branch_probs_out[i] = branch_probs[i] / multiplier * p_state;
			//}
			//else {
			//	branch_probs_out[i] = 0;
			//}
			}
			size_t j = 0;
			//if (first_good_branch >= 0)
			//{
			//	// double relative = branch_probs_out[first_good_branch] / branch_probs[first_good_branch];
			//	for (size_t i = 0; i < good_branch_ids.size(); ++i)
			//	{
			//		size_t id = good_branch_ids[i];
			//		branches[id].relative_multiplier = multipliers[i];
			//		// branch_probs_out[id] = relative * multipliers[i] * branch_probs[id];
			//	}
			//}
		}

		void QRAMCircuit::sample_output_without_normalization()
		{
			profiler _("sample_output_without_normalization");
			if (branches.size() == 0)
				return;
			if (valid_branch_view.size() == 0)
			{
				// no branches are included, there should be some error.
				throw_invalid_input();
			}
			else if (valid_branch_view.size() == 1)
			{
				/*	Sample from only 1 branch when:
					Case 1: only 1 branch input (regardless good/bad)
					Case 2: normal, when there is almost no error
					Case 3: shortcircuit calculation (good only)
				*/

				if (has_damping())
				{
					if (first_good_branch >= 0)
					{
						Branch::get_multiplier(
							noise_parameters[OperationType::Damping],
							time_step,
							time_step.last_step(),
							branches,
							first_good_branch,
							good_branch_ids,
							memory
						);
					}

				}

				if (valid_branch_view[0]->system_size() == 1)
				{
					// no branches would be handled.
				}
				else
				{
					double factor = valid_branch_view[0]->get_prob();
					std::uniform_real_distribution<double> urd(0, factor);
					double r = urd(random_engine::get_engine());
					for (auto iter = valid_branch_view[0]->iterbeg(); iter != valid_branch_view[0]->iterend(); ++iter)
					{
						double thisprob = abs_sqr(iter->amplitude);
						if (r < thisprob)
						{
							final_system_state = iter->state.nz_elements;
							break;
						}
						r -= thisprob;
					}
					valid_branch_view[0]->remove_mismatch_state(final_system_state);
					return;
				}
			}
			else
			{
				if (has_damping())
					sample_output_without_normalization_with_damping();
				else
					sample_output_without_normalization_without_damping();
			}
		}

		void QRAMCircuit::sample_output_without_normalization_without_damping()
		{
			std::uniform_real_distribution<double> urd(0, get_normalization_factor());
			double r = urd(random_engine::get_engine());
			size_t i = 0;

			// use binary search to optimize to O(nlog n)
			bool set = false;
			for (size_t i = 0; i < branches.size(); ++i)
			{
				if (r > branch_probs[i])
				{
					r -= branch_probs[i];
					continue;
				}
				else
				{
					auto& branch = branches[i];
					using itertype = decltype(branch.iterbeg());
					itertype iter, end;

					if (branch.is_good())
					{
						iter = branch.good_ref->iterbeg();
						end = branch.good_ref->iterend();
					}
					else
					{
						iter = branch.iterbeg();
						end = branch.iterend();
					}
					for (; iter != end; ++iter)
					{
						double thisprob = branch_probs[i] * abs_sqr(iter->amplitude);
						if (r < thisprob) {
							final_system_state = iter->state.nz_elements;
							set = true;
							goto StateSet;
						}
						r -= thisprob;
					}
				}
			}
		StateSet:
			// assert if not set
			if (!set)
			{
				// fmt::print(to_string());
				// fmt::print("Normalization factor = {}\n", get_normalization_factor());
				throw_bad_result();
			}
			for (auto branch_ptr : valid_branch_view) {
				branch_ptr->remove_mismatch_state(final_system_state);
			}
		}

		void QRAMCircuit::sample_output_without_normalization_with_damping()
		{
			/* preprocess the multipliers */
			double gamma = noise_parameters[OperationType::Damping];
			if (first_good_branch >= 0)
			{
				Branch::get_multiplier(
					gamma,
					time_step,
					time_step.last_step(),
					branches,
					first_good_branch, good_branch_ids, /*multipliers, */memory
				);
			}
			double factor = get_normalization_factor_with_damping();
			std::uniform_real_distribution<double> urd(0, factor);
			double r = urd(random_engine::get_engine());
			size_t i = 0;

			// use binary search to optimize to O(nlog n)
			bool set = false;
			for (size_t i = 0; i < branches.size(); ++i)
			{
				auto& branch = branches[i];
				if (branch.is_good()) {
					for (auto iter = branch.good_ref->iterbeg(); iter != branch.good_ref->iterend(); ++iter) {
						double thisprob = branch.relative_multiplier * branch_probs[i] * abs_sqr(iter->amplitude);
						if (r < thisprob) {
							final_system_state = iter->state.nz_elements;
							set = true;
							goto StateSet;
						}
						r -= thisprob;
					}
				}
				else {
					for (auto iter = branch.iterbeg(); iter != branch.iterend(); ++iter) {
						double thisprob = branch_probs[i] * abs_sqr(iter->amplitude);
						if (r < thisprob) {
							final_system_state = iter->state.nz_elements;
							set = true;
							goto StateSet;
						}
						r -= thisprob;
					}
				}
			}
		StateSet:
			// assert if not set
			if (!set)
			{
				fmt::print(to_string());
				throw_bad_result();
			}
			for (auto branch_ptr : valid_branch_view) {
				branch_ptr->remove_mismatch_state(final_system_state);
			}
		}

		void QRAMCircuit::sample_output()
		{
			sample_output_without_normalization();
			normalization();
		}

		/* Decide first good branch and set remaining. */
		void QRAMCircuit::run_good()
		{
			profiler _("run_good");
			for (size_t i = 0; i < branches.size(); ++i)
			{
				if (!time_step.is_bad_branch(branches[i].address))
				{
					if (first_good_branch < 0)
						first_good_branch = i;
					else {
						branches[i].set_good(branches.data() + first_good_branch);
					}
				}
			}
		}

		void QRAMCircuit::run_valid_branches()
		{
			profiler _("run_valid_branches");
			int step = 0;

			for (const OperationPack& ops : operations.time_slices) {
				step++;
				for (const Operation& op : ops.operations) {
					switch (op.type) {
					case OperationType::ControlSwap:
						run_cswap(op.targets[0]); break;
					case OperationType::CopyIn:
						if (op.targets[0] == 0)
							run_hadamard();
						run_busin(op.targets[0]);
						break;
					case OperationType::CopyOut:
						run_busout(op.targets[0]);
						if (op.targets[0] == data_size - 1)
							run_hadamard();
						break;
					case OperationType::SwapInternal:
						run_swap(op.targets[0]);
						break;
					case OperationType::FirstCopy:
						run_acopy(op.targets[0]);
						break;
					case OperationType::FetchData:
						run_fetchdata(op.targets[0]);
						break;
					case OperationType::BitFlip:
						run_bitflip(op.targets[0]);
						break;
					case OperationType::PhaseFlip:
						run_phaseflip(op.targets[0], op.coefficients[0]);
						break;
					case OperationType::BitPhaseFlip:
						run_bitphaseflip(op.targets[0]);
						break;
					case OperationType::Depolarizing:
						run_depolarizing(op.targets[0], op.coefficients[0]);
						break;
					case OperationType::Damp_Full:
						run_damp_full(op.targets[0], step, op.coefficients[0]);
						clear_zero_elements();
						break;
					case OperationType::Damp_Common:
						run_damp_common(op.coefficients[0]); break;
					default:
						throw std::runtime_error("Bad type.");
					}
				}
			}
			clear_zero_elements();
		}

		void QRAMCircuit::run_normal()
		{
			profiler _("QRAMCircuit::run_normal");
			pick_good_bad();
			run_valid_branches();
		}

		void QRAMCircuit::run_full()
		{
			profiler _("QRAMCircuit::run_full");
			pick_all();
			run_valid_branches();
		}

		void QRAMCircuit::run_good_only()
		{
			// the good-only mode does not require input
			// also does not perform any actual operation.
			throw_general_runtime_error();
			profiler _("QRAMCircuit::run_good_only");
			//prepare_good_only();
			//run_valid_branches();
			// time_step.append_noise_range_only(noise_parameters, Branch::arch_type);
		}

		void QRAMCircuit::run_good_only_without_damping()
		{
			throw_general_runtime_error();
		}

		void QRAMCircuit::run_good_only_with_damping()
		{
			throw_general_runtime_error();
		}

		void QRAMCircuit::run(const std::string &version)
		{
			if (branches.size() == 0)
				return;
			/* if no noise, reject immediately.
			*  Because it should be handled outside of QRAMCircuit,
			   therefore no overhead exists.
			*/
			if (noise_parameters.size() == 0)
				throw_invalid_input();

			/* A full run, all branches are calculated */
			if (version == "old" || version == "full")
			{
				run_full();
				return;
			}
			/* Full run of bad branches (and first good branch),
			   and other good branches are predicted. */
			else if (version == "new" || version == "normal")
			{
				run_normal();
				return;
			}
			/* Discard all bad branches,
			   and all good branches are predicted. */
			   //else if (version == "fast") {
			   //	run_good_only();
			   //	return;
			   //}
			throw_invalid_input();
		}

		std::string QRAMCircuit::to_string() const
		{
			profiler _("qram::to_string");
			std::vector<char> buf;

			fmt::format_to(back_inserter(buf), "memory={}\n", memory);
			for (size_t i = 0; i < branches.size(); ++i) {
				fmt::format_to(back_inserter(buf), "prob = {:.4f}\tstate = {}", branch_probs[i], branches[i].to_string());
				fmt::format_to(back_inserter(buf), "\n");
			}
			return { buf.data(), buf.size() };
		}

		std::string QRAMCircuit::to_string_full_info() const
		{
			std::vector<char> buf;

			fmt::format_to(std::back_inserter(buf), "memory={}\n", memory);
			for (size_t i = 0; i < branches.size(); ++i) {
				fmt::format_to(std::back_inserter(buf), "prob = {:.4f}\tstate = {}", branch_probs[i], branches[i].to_string());
				fmt::format_to(std::back_inserter(buf), "\n");
			}
			fmt::format_to(std::back_inserter(buf), "Operations = \n{}\n", operations.to_string());
			fmt::format_to(std::back_inserter(buf), "FinalSystem = {}\n", final_system_state);
			fmt::format_to(std::back_inserter(buf), "first_good_branch = {}\n", first_good_branch);
			fmt::format_to(std::back_inserter(buf), "prob = {}\n", get_normalization_factor());
			return { buf.data(), buf.size() };
		}

	} // namesapce qram_qutrit
} // namespace qram_simulator