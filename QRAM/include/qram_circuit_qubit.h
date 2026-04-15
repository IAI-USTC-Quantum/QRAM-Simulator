#include "qram_branch_qubit.h"

namespace qram_simulator {
	namespace qram_qubit {
		struct QRAMCircuit
		{
			using SystemState = typename Branch::state_type;
			using qram_state_t = typename SystemState::element_type;

		public:
			size_t addr_size;
			size_t data_size;
			TimeStep time_step;
			memory_t memory;

			/* Internal operations */
			TimeSlices raw_operations;
			TimeSlices operations;
			noise_t noise_parameters;

			std::vector<BranchGroup> branch_groups;
			std::vector<BranchGroup*> valid_branch_group_view;
			ptrdiff_t first_good_branch_group = -1;
			std::vector<size_t> good_branch_group_ids;

		public:
			double total_prob = 0;
			qram_state_t final_system_state;

			QRAMCircuit(size_t address_sz, size_t data_sz) :
				addr_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(addr_size));
			}

			QRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory) :
				addr_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(addr_size));
				set_memory(std::move(memory));
			}

			void set_memory_random() { random_memory(memory, data_size); }
			void set_memory(const memory_t& new_memory)
			{
				if (new_memory.size() != pow2(addr_size))
					throw std::runtime_error("Bad Input.");

				memory = new_memory;
			}
			void set_memory(memory_t&& new_memory)
			{
				if (new_memory.size() != pow2(addr_size))
					throw std::runtime_error("Bad Input.");

				memory = std::move(new_memory);
			}
			auto& get_memory() { return memory; }
			auto& get_memory() const { return memory; }

			size_t get_qubit_num() const { return 2 * (pow2(addr_size) - 1); }
			size_t memory_size() const { return memory.size(); }

			// set branches for state_manipulator (see init_state for test usage)
			void set_branches(const std::vector<BranchGroup>& branch_groups_)
			{
				branch_groups = branch_groups_;
			}

			void set_branches(std::vector<BranchGroup>&& branch_groups_)
			{
				branch_groups = std::move(branch_groups_);
			}

			auto& get_branch_groups() const { return branch_groups; }
			auto& get_operations() const { return operations; }
			auto& get_branch_groups() { return branch_groups; }
			auto& get_operations() { return operations; }

			/* DEPRECATED */
			[[deprecated]] void clear_noise() { noise_parameters.clear(); }

			/* DEPRECATED */
			[[deprecated]] void add_noise_model(OperationType t, double v) {
				noise_parameters[t] = v;
			}
			void set_noise_models(const std::map<OperationType, double>& noises) {
				noise_parameters = noises;
			}

			/* generate noisy QRAM operation */
			void initialize_system() {

				profiler _("QRAMCircuit::initialize_system");
				operations = time_step.generate(noise_parameters, Branch::qunit_type);
				// good_branch_ids.clear();
				first_good_branch_group = -1;
				valid_branch_group_view.clear();
				std::for_each(branch_groups.begin(), branch_groups.end(),
					[](BranchGroup& branchgroup) { branchgroup.reset(); }
				);
			}

			double get_fidelity() {
				profiler _("get_fidelity");
				/*if (noise_parameters.find(OperationType::Damping) == noise_parameters.end())
					sample_output();
				else
					sample_output_with_damping();*/

				sample_output();

				if (valid_branch_group_view.size() == 0)
					return 0;

				complex_t ret = 0;
				for (size_t i = 0; i < branch_groups.size(); ++i)
				{
					ret += branch_groups[i].get_fidelity();
				}
				// if (ret > 1) { throw_bad_result(); }
				return abs_sqr(ret);
			}


			/*1. prepare valid_branch_view
			*
			* valid_branch_view will only include those branches
			* needed to evaluate
			*
			* 2. prepare first_good_branch
			* 3. prepare branches[i].good and set its bus output
			* 4. prepare good_branch_ids
			*/
			void prepare_bad()
			{
				profiler _("QRAMCircuit::prepare_bad");
				// int good_number = 0;
				initialize_system();
				good_branch_group_ids.clear();
				// initialize valid_branches

				auto head = branch_groups.data();

				for (size_t i = 0; i < branch_groups.size(); ++i)
				{
					if (!time_step.is_bad_branch(branch_groups[i].address))
					{
						if (first_good_branch_group < 0)
							first_good_branch_group = i;
						else {
							// this needs to gaurantee branches vector should
							// never change size, reallocate or move.
							branch_groups[i].set_good(std::shared_ptr<BranchGroup>(head + first_good_branch_group, [](BranchGroup*){}));

							// good_number++;
							good_branch_group_ids.push_back(i);

							/* TODO: wrong implementation */

							// branches[i].system_states.front().bus ^= memory[branches[i].address];
							continue;
						}
					}
					valid_branch_group_view.emplace_back(head + i);
				}
				// multipliers.resize(good_branch_ids.size());
			}

			/* prepare valid_branch_view
			*
			*  valid_branch_view will only include those branches
			*  needed to evaluate
			*/
			void prepare_all()
			{
				profiler _("QRAMCircuit::prepare_all");
				initialize_system();
				auto head = branch_groups.data();

				for (size_t i = 0; i < branch_groups.size(); ++i)
				{
					valid_branch_group_view.emplace_back(head + i);
					branch_groups[i].branches = branch_groups[i].branches_input;
				}
			}

			/*1. prepare valid_branch_view (good_only)
			*
			* valid_branch_view will only include those branches
			* needed to evaluate
			*
			* 2. prepare first_good_branch
			* 3. prepare branches[i].good and set its bus output
			* 4. prepare good_branch_ids
			*/
			void prepare_good_only()
			{
				profiler _("QRAMCircuit::prepare_good_only");
				// int good_number = 0;
				initialize_system();
				good_branch_group_ids.clear();
				// initialize valid_branches
				auto head = branch_groups.data();

				for (size_t i = 0; i < branch_groups.size(); ++i)
				{
					if (!time_step.is_bad_branch(branch_groups[i].address))
					{
						if (first_good_branch_group < 0) {
							first_good_branch_group = i;
							valid_branch_group_view.emplace_back(head + i);
						}
						else {
							// this needs to gaurantee branches vector should
							// never change size, reallocate or move.
							branch_groups[i].set_good(std::shared_ptr<BranchGroup>(head + first_good_branch_group, [](BranchGroup*){}));

							// good_number++;
							good_branch_group_ids.push_back(i);

							/* TODO: wrong implementation */

							// branches[i].system_states.front().bus ^= memory[branches[i].address];
							continue;
						}
					}
					else
					{
						branch_groups[i].set_empty_state();
					}
					// valid_branch_view.emplace_back(branches.data() + i);
				}
				if (first_good_branch_group < 0)
					// fail to find any of the good
					prepare_all();
			}

			void run_acopy(int layer)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					bool a = get_digit_reverse(branch_group_ptr->address, layer, addr_size);

					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[a](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[a](SystemState& state) {
									state.run_acopy(a);
								}
					);
						}
					);

					/*for (auto& state : branch_ptr->system_states)
						state.run_acopy(a);*/
				}
			}

			void run_swap(size_t layer_id)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[layer_id](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[layer_id](SystemState& state) {
									state.run_swap(layer_id);
								}
					);
						}
					);

					/*std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
						[layer_id](SystemState& state)
						{state.run_swap(layer_id); });*/
						/*for (auto& state : branch_ptr->system_states)
							state.run_swap(layer_id);*/
				}
			}

			void run_cswap(size_t layer_id)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[layer_id](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[layer_id](SystemState& state) {
									state.run_cswap(layer_id);
								}
					);
						}
					);
					/*std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
						[layer_id](SystemState& state)
						{state.run_cswap(layer_id); });*/
						/*for (auto& state : branch_ptr->system_states)
							state.run_cswap(layer_id);*/
				}
			}

			void run_bitflip(size_t qubit_id)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[qubit_id](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[qubit_id](SystemState& state) {
									state.run_bitflip(qubit_id);
								}
					);
						}
					);
					/*std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
						[qubit_id](SystemState& state)
						{state.run_bitflip(qubit_id); });*/
						/*for (auto& state : branch_ptr->system_states)
							state.run_bitflip(qubit_id);*/
				}
			}

			void run_phaseflip(size_t qubit_id, double depol_id)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[qubit_id](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[qubit_id](SystemState& state) {
									state.run_phaseflip(qubit_id, 0);
								}
					);
						}
					);
					/*std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
						[qubit_id, depol_id](SystemState& state)
						{state.run_phaseflip(qubit_id, depol_id); });*/
						/*for (auto& state : branch_ptr->system_states)
							state.run_phaseflip(qubit_id);*/
				}
			}

			void run_bitphaseflip(size_t qubit_id)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[qubit_id](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[qubit_id](SystemState& state) {
									state.run_bitphaseflip(qubit_id);
								}
					);
						}
					);
				}
			}

			void run_depolarizing(size_t qubit_id, double depol_id)
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[qubit_id, depol_id](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[qubit_id, depol_id](SystemState& state) {
									state.run_depolarizing(qubit_id, depol_id);
								} // lambda
					); // for_each
						} // lambda
					); // for_each

					/*std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
						[qubit_id, depol_id](SystemState& state)
						{state.run_depolarizing(qubit_id, depol_id); });*/
						/*for (auto& state : branch_ptr->system_states)
							state.run_depolarizing(qubit_id, depol_id);*/
				}
			}

			void run_hadamard() {
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[](Branch& branch) {
							branch.run_hadamard();
					branch.try_merge();
						}
					);
					/*branch_ptr->run_hadamard();
					branch_ptr->try_merge();*/
				}
			}

			void run_fetchdata(size_t digit) {
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[this, digit](Branch& branch) {
							branch.run_fetchdata(memory, digit);
						}
					);
					// branch_ptr->run_fetchdata(memory, digit);
				}
			}

			void run_busin(size_t digit) {
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[digit](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[digit](SystemState& state) {
									state.run_busin(digit);
								}
					);
						}
					);
					/*std::for_each(branch_ptr->iterbeg(), branch_ptr->iterend(),
						[digit](SystemState& state) {state.run_busin(digit); });*/

						/*for (auto& state : branch_ptr->system_states)
							state.run_busin(digit);*/
				}
			}

			void run_busout(size_t digit) {
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[digit](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[digit](SystemState& state) {
									state.run_busout(digit);
								}
					);
						}
					);
				}
			}

			void run_damp_full(size_t qubit_id, size_t step, double gamma) {
				// throw_not_implemented();

				// get_multipliers
				// time_step.get_multiplier_qubit()

				/* Forward to BranchType static method
				*  to allow type check
				*/

				/* check the state of first_good_branch
				* skip the case of no damping
				*/
				std::vector<double> prob_damp;
				int damp_op_num = 1;
				if constexpr (Branch::qunit_type == arch_qubit)
				{
					damp_op_num = 1;
				}
				else if constexpr (Branch::qunit_type == arch_qutrit)
				{
					damp_op_num = 2;
				}
				else {
					// static_assert(false, "Bad qunit_type");
					throw_invalid_input();
				}
				prob_damp.resize(damp_op_num);
				if (first_good_branch_group >= 0)
				{
					//Branch::get_multiplier(
					//	gamma,
					//	time_step,/*instance reference*/
					//	step,/*time step*/
					//	branch_groups, /*compute the branch_id*/
					//	first_good_branch_group,
					//	good_branch_group_ids,
					//	/*multipliers,*/
					//	memory);
					time_step.get_multiplier_qubit(gamma, step, branch_groups,
						first_good_branch_group, good_branch_group_ids);

					auto&& ref_prob = branch_groups[first_good_branch_group].get_prob_damp(qubit_id);

					for (size_t i = 0; i < good_branch_group_ids.size(); ++i) {
						size_t id = good_branch_group_ids[i];
						for (auto k = 0; k < damp_op_num; ++k) {
							prob_damp[k] += ref_prob[k] /** multipliers[i] */
								* branch_groups[id].relative_multiplier;
							// * branch_groups[id].get_damp_prob();
						}
					}
				}
				auto head = branch_groups.data();
				/* first: decide whether this qubit is non-zero*/
				for (auto branch_ptr : valid_branch_group_view)
				{
					// size_t branch_id = branch_ptr - head;
					auto&& prob = branch_ptr->get_prob_damp(qubit_id);
					for (auto k = 0; k < damp_op_num; ++k) {
						prob_damp[k] += prob[k];
					}
				}
				double global_coef = get_normalization_factor_with_damping();
				double r = random_engine::get_instance().uniform01() * global_coef;
				for (size_t k = 0; k < prob_damp.size(); ++k)
				{
					if (r < prob_damp[k])
					{
						//fmt::print("Damp={} Qubit={}\n", k, qubit_id);
						//fmt::print(to_string_full_info());
						for (auto branch_group_ptr : valid_branch_group_view)
						{
							std::for_each(
								branch_group_ptr->branches.begin(),
								branch_group_ptr->branches.end(),
								[qubit_id, k](Branch& branch) {
									branch.run_damp_full(qubit_id, k);
								}
							);
						}
						//fmt::print(to_string_full_info());
						//getchar();
						break;
					}
					r -= prob_damp[k];
				}
				// no need to run extra damp_common
				// because always include a damp_common at every time step
				// run_damp_common(gamma);	
			}

			void run_damp_common(double gamma) {
				/*profiler _("run_damp_common");
				for (size_t i = 0; i < branches.size(); ++i) {
					if (branches[i].good)
						continue;
					if (branch_probs[i] < epsilon)
						continue;
					branches[i].run_damp_common(coef);
				}*/

				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[gamma](Branch& branch) {
							branch.run_damp_common(gamma);
						}
					);
				}
			}

			void clear_zero_elements()
			{
				for (auto branch_group_ptr : valid_branch_group_view)
				{
					std::for_each(
						branch_group_ptr->branches.begin(),
						branch_group_ptr->branches.end(),
						[](Branch& branch) {
							std::for_each(branch.iterbeg(), branch.iterend(),
							[](SystemState& state)
								{
									state.state.clear_zero_elements();
								}
					);
						}
					);
				}
			}

			//OperationPack _noise_one_step() {
			//	OperationPack pack;
			//	pack.set_name("Noise");
			//	static std::uniform_real_distribution<double> ud(0, 1);
			//	for (auto noise : noise_parameters) {

			//		// TODO : open/close qubits
			//		for (size_t i = 0; i < get_qubit_num(); ++i) {
			//			// for (size_t i : get_open_qubit()) {
			//			double r = ud(random_engine::get_engine());
			//			switch (noise.first) {
			//			case OperationType::BitFlip:
			//			case OperationType::PhaseFlip:
			//				if (r < noise.second) {
			//					pack.append(Operation(noise.first, { i }));
			//				}
			//				break;
			//			case OperationType::BitPhaseFlip:
			//				if (r < noise.second) {
			//					pack.append(Operation(OperationType::BitFlip, { i }));
			//					pack.append(Operation(OperationType::PhaseFlip, { i }));
			//				}
			//				break;
			//			case OperationType::Depolarizing:
			//				if (r < noise.second / 3) {
			//					pack.append(Operation(OperationType::BitFlip, { i }));
			//				}
			//				else if (r < 2 * noise.second / 3) {
			//					pack.append(Operation(OperationType::PhaseFlip, { i }));
			//				}
			//				else if (r < noise.second) {
			//					pack.append(Operation(OperationType::BitFlip, { i }));
			//					pack.append(Operation(OperationType::PhaseFlip, { i }));
			//				}
			//				break;
			//			case OperationType::Damping:
			//				if (r < noise.second) {
			//					pack.append(Operation(OperationType::Damp_Full, { i }));
			//				}
			//				else {
			//					pack.append(Operation(OperationType::Damp_Common, { i }, { sqrt(1 - noise.second) }));
			//				}
			//			default:
			//				throw std::runtime_error("Bad enum type.");
			//			}
			//		}
			//	}

			//	return pack;
			//}

			///* DEPRECATED no longer use this function */
			//[[deprecated]] void append_noise() {
			//	operations = raw_operations;
			//	std::vector<OperationPack>& time_slices = operations.time_slices;
			//	for (auto& time_slice : time_slices) {
			//		OperationPack&& noise = _noise_one_step();
			//		if (!noise.empty()) {
			//			time_slice.append(noise);
			//		}
			//	}
			//}

			std::string to_string() const {
				profiler("qram::to_string");
				throw_not_implemented();
				return std::string();
				//std::vector<char> buf;

				//fmt::format_to(back_inserter(buf), "memory={}\n", memory);
				//for (size_t i = 0; i < branches.size(); ++i) {
				//	/*if (branch_probs_out.size() > 0) {
				//		fmt::format_to(back_inserter(buf), "prob(out) = {:.4f}\t", branch_probs_out[i]);
				//	}*/
				//	fmt::format_to(back_inserter(buf), "prob = {:.4f}\tstate = {}", branch_probs[i], branches[i].to_string());

				//	/*if (branches[i].is_good())
				//		fmt::format_to(back_inserter(buf), "good");*/
				//	fmt::format_to(back_inserter(buf), "\n");
				//}
				//return { buf.data(), buf.size() };
			}

			std::string to_string_full_info() const {
				throw_not_implemented();
				return std::string();
				//std::vector<char> buf;

				//fmt::format_to(std::back_inserter(buf), "memory={}\n", memory);
				//for (size_t i = 0; i < branches.size(); ++i) {
				//	/*if (branch_probs_out.size() > 0) {
				//		fmt::format_to(back_inserter(buf), "prob(out) = {:.4f}\t", branch_probs_out[i]);
				//	}*/
				//	fmt::format_to(std::back_inserter(buf), "prob = {:.4f}\tstate = {}", branch_probs[i], branches[i].to_string());

				//	/*if (branches[i].is_good())
				//		fmt::format_to(back_inserter(buf), "good");*/
				//	fmt::format_to(std::back_inserter(buf), "\n");
				//}
				//// fmt::format_to(std::back_inserter(buf), "Bad = {}\n", time_step.bad_branches);
				//fmt::format_to(std::back_inserter(buf), "Operations = \n{}\n", operations.to_string());
				//fmt::format_to(std::back_inserter(buf), "FinalSystem = {}\n", final_system_state);
				////if (good_system_cache.has_value())
				////	fmt::format_to(std::back_inserter(buf), "GoodSystem = {}\n", good_system_cache.value());
				////else
				////	fmt::format_to(std::back_inserter(buf), "GoodSystem = (N/A)\n");
				//fmt::format_to(std::back_inserter(buf), "first_good_branch = {}\n", first_good_branch);
				//fmt::format_to(std::back_inserter(buf), "prob = {}\n", get_normalization_factor());
				//return { buf.data(), buf.size() };
			}

			double get_normalization_factor() const
			{
				if (noise_parameters.find(OperationType::Damping) != noise_parameters.end())
				{
					return get_normalization_factor_with_damping();
				}
				double new_prob = 0;
				for (size_t i = 0; i < branch_groups.size(); ++i) {
					new_prob += branch_groups[i].get_prob();
					check_nan(new_prob);
				}

				return new_prob;
			}

			double get_normalization_factor_with_damping() const
			{
				double new_prob = 0;
				size_t j = 0;
				for (auto branch_group_ptr : valid_branch_group_view) {
					new_prob += branch_group_ptr->get_prob();
					check_nan(new_prob);
				}
				if (first_good_branch_group >= 0)
				{
					double first_good_branch_prob = branch_groups[first_good_branch_group].get_prob();
					for (size_t i = 0; i < good_branch_group_ids.size(); ++i)
					{
						new_prob += first_good_branch_prob /* *multipliers[i]*/
							* branch_groups[good_branch_group_ids[i]].relative_multiplier
							* branch_groups[good_branch_group_ids[i]].get_prob();
						// * branch_probs[good_branch_ids[i]];
					}
				}
				return new_prob;
			}

			void normalization()
			{
				double multiplier = get_normalization_factor_with_damping();

				for (auto branch_group : valid_branch_group_view) {
					// double p_state = branches[i].get_prob();
					// if (p_state > epsilon) {
					for (auto& branch : branch_group->branches)
					{
						for (auto iter = branch.iterbeg(); iter != branch.iterend(); ++iter)
							iter->amplitude /= sqrt(multiplier);
					}

					// branch_probs_out[i] = branch_probs[i] / multiplier * p_state;
					// }
					/*else {
						branch_probs_out[i] = 0;
					}*/
				}
			}

			//void normalization_with_damping()
			//{
			//	// branch_probs_out.resize(branch_probs.size(), 0);
			//	double multiplier = get_normalization_factor_with_damping();

			//	/*if (multiplier < epsilon * epsilon)
			//	{
			//		fmt::print("[Full Error Info]\n{}\n", to_string_full_info());
			//		throw_bad_result();
			//	}*/

			//	for (auto branch_ptr : valid_branch_view) {
			//		int i = branch_ptr - branches.data();

			//		//double p_state = branch_ptr->get_prob();
			//		//if (p_state > epsilon) {
			//		for (auto iter = branch_ptr->iterbeg(); iter != branch_ptr->iterend(); ++iter)
			//		{
			//			iter->amplitude /= sqrt(multiplier);
			//		}
			//		//branch_probs_out[i] = branch_probs[i] / multiplier * p_state;
			//	//}
			//	//else {
			//	//	branch_probs_out[i] = 0;
			//	//}
			//	}
			//	size_t j = 0;
			//	//if (first_good_branch >= 0)
			//	//{
			//	//	// double relative = branch_probs_out[first_good_branch] / branch_probs[first_good_branch];
			//	//	for (size_t i = 0; i < good_branch_ids.size(); ++i)
			//	//	{
			//	//		size_t id = good_branch_ids[i];
			//	//		branches[id].relative_multiplier = multipliers[i];
			//	//		// branch_probs_out[id] = relative * multipliers[i] * branch_probs[id];
			//	//	}
			//	//}
			//}

			/* Sample the output according to the branches and branch_probs*/
			/* without damping */
			void sample_output()
			{
				bool has_damping = noise_parameters.find(OperationType::Damping) != noise_parameters.end();
				if (has_damping)
				{
					/* preprocess the multipliers */
					double gamma = noise_parameters[OperationType::Damping];
					if (first_good_branch_group >= 0)
					{
						/* Good branch gaurantees the identical system_states
						*  (never splitted)
						*/
						// good_system_cache = branches[first_good_branch].get_good_branch_system();

						time_step.get_multiplier_qubit(
							gamma,
							time_step.last_step(),
							branch_groups,
							first_good_branch_group,
							good_branch_group_ids
						);
					}
					//sample_output_with_damping();
					//return;
				}
				profiler _("sample_output");

				if (valid_branch_group_view.size() == 0)
				{
					// no branch is calculated
					return;
				}

				/*	Sample from only 1 branch when:
					Case 1: only 1 branch input (regardless good/bad)
					Case 2: good only
					Case 3: shortcircuit calculation (good only)
				*/
				else if (valid_branch_group_view.size() == 1)
				{
					double factor = valid_branch_group_view[0]->get_prob();
					std::uniform_real_distribution<double> urd(0, factor);
					double r = urd(random_engine::get_engine());

					if (has_damping)
						valid_branch_group_view[0]->sample_output_with_damping(final_system_state, r);
					else
						valid_branch_group_view[0]->sample_output_no_damping(final_system_state, r);

					valid_branch_group_view[0]->remove_mismatch_state(final_system_state);
					normalization();
					return;
				}
				else // valid_branch_group_view.size() > 1
				{
					double factor = get_normalization_factor();
					std::uniform_real_distribution<double> urd(0, factor);
					double r = urd(random_engine::get_engine());

					// TODO:
					// use binary search to optimize to O(nlog n)
					bool set = false;
					if (has_damping) {
						for (auto branch_group : valid_branch_group_view) {
							set = branch_group->sample_output_with_damping(final_system_state, r);
							if (set) break;
						}
					}
					else {
						for (auto branch_group : valid_branch_group_view) {
							set = branch_group->sample_output_no_damping(final_system_state, r);
							if (set) break;
						}
					}

					if (!set) // in the good branch
					{
						if (first_good_branch_group < 0)
						{
							fmt::print(to_string());
							throw_bad_result();
						}
						else {
							double factor = branch_groups[first_good_branch_group].get_prob();
							std::uniform_real_distribution<double> urd(0, factor);
							double r = urd(random_engine::get_engine());

							if (has_damping)
								set = branch_groups[first_good_branch_group].sample_output_with_damping
								(final_system_state, r);
							else
								set = branch_groups[first_good_branch_group].sample_output_no_damping
								(final_system_state, r);
						}
					}

					if (!set) // still not get sampled?
					{
						fmt::print(to_string());
						throw_bad_result();
					}

					for (auto branch_ptr : valid_branch_group_view) {
						/*if (branch_probs[branch_ptr-branches.data()] < epsilon)
							continue;*/
						branch_ptr->remove_mismatch_state(final_system_state);
					}

					normalization();
				}
			}

			/* Sample the output according to the branches and branch_probs*/
			/* with damping */
			//void sample_output_with_damping()
			//{
			//	profiler _("sample_output_damping");

			//	/* preprocess the multipliers */
			//	double gamma = noise_parameters[OperationType::Damping];
			//	if (first_good_branch >= 0)
			//	{
			//		/* Good branch gaurantees the identical system_states
			//		*  (never splitted)
			//		*/
			//		// good_system_cache = branches[first_good_branch].get_good_branch_system();
			//	
			//		Branch::get_multiplier(
			//			gamma,
			//			time_step,
			//			time_step.last_step(),
			//			branches,
			//			first_good_branch, good_branch_ids, /*multipliers, */memory
			//		);
			//	}

			//	double factor = get_normalization_factor_with_damping();
			//	std::uniform_real_distribution<double> urd(0, factor);
			//	double r = urd(random_engine::get_engine());
			//	size_t id = 0;

			//	fmt::print("Factor={}, r={}\n", factor, r);

			//	// use binary search to optimize to O(nlog n)
			//	bool set = false;
			//	for (size_t i = 0; i < branches.size();++i) {
			//		
			//		/*auto& branch = *branch_ptr;
			//		auto i = branch_ptr - branches.data();*/

			//		auto& branch = branches[i];
			//		
			//		if (branch.is_good()) {
			//			for (auto iter = branch.good_ref->iterbeg(); iter != branch.good_ref->iterend(); ++iter) {
			//				double thisprob = branch.relative_multiplier * branch_probs[i] * abs_sqr(iter->amplitude);
			//				// fmt::print("thisprob={}, r={}\n", branch.relative_multiplier * branch_probs[i], r);
			//				if (r < thisprob) {
			//					final_system_state = iter->state.nz_elements;	
			//					// fmt::print("SetFinal = {}\n", final_system_state);
			//					set = true;
			//					break;
			//				}
			//				r -= thisprob;
			//			}
			//		}
			//		else {
			//			for (auto iter = branch.iterbeg(); iter != branch.iterend(); ++iter) {
			//				double thisprob = branch_probs[i] * abs_sqr(iter->amplitude);
			//				fmt::print("thisprob={}, r={}\n", thisprob, r);
			//				if (r < thisprob) {
			//					final_system_state = iter->state.nz_elements;
			//					fmt::print("SetFinal = {}\n", final_system_state);
			//					set = true;
			//					break;
			//				}
			//				r -= thisprob;
			//			}
			//		}
			//	}
			//	if (!set)
			//	{
			//		fmt::print(to_string());
			//		throw_bad_result();
			//	}
			//	for (auto branch_ptr : valid_branch_view) {
			//		/*if (branch_probs[branch_ptr-branches.data()] < epsilon)
			//			continue;*/
			//		branch_ptr->remove_mismatch_state(final_system_state);
			//	}

			//	normalization_with_damping();
			//}

			/* DEPRECATED will be removed in future */
			[[deprecated]] std::string mem_to_string() const {
				return fmt::format("{}", memory);
			}

			void run_good()
			{
				profiler _("run_good");
				auto head = branch_groups.data();
				for (size_t i = 0; i < branch_groups.size(); ++i)
				{
					if (!time_step.is_bad_branch(branch_groups[i].address))
					{
						if (first_good_branch_group < 0)
							first_good_branch_group = i;
						else {
							/*branches[i].good = true;
							branches[i].system_states.front().bus = memory[branches[i].address];*/
							branch_groups[i].set_good(std::shared_ptr<BranchGroup>(head + first_good_branch_group, [](BranchGroup*){}));
						}
					}
				}
			}

			void run_bad()
			{
				profiler _("run_bad");
				int step = 0;
				for (OperationPack& ops : operations.time_slices) {
					step++;
					for (Operation& op : ops.operations) {
						switch (op.type) {
						case OperationType::ControlSwap:
							run_cswap(op.targets[0]); break;
							// case OperationType::HadamardData:							
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
							run_swap(op.targets[0]); break;
						case OperationType::FirstCopy:
							run_acopy(op.targets[0]); break;
						case OperationType::FetchData:
							run_fetchdata(op.targets[0]); break;
						case OperationType::BitFlip:
							run_bitflip(op.targets[0]); break;
						case OperationType::PhaseFlip:
							run_phaseflip(op.targets[0], op.coefficients[0]); break;
						case OperationType::BitPhaseFlip:
							run_bitphaseflip(op.targets[0]); break;
						case OperationType::Depolarizing:
							run_depolarizing(op.targets[0], op.coefficients[0]); break;
						case OperationType::Damp_Full:
							run_damp_full(op.targets[0], step, op.coefficients[0]);
							clear_zero_elements();
							break;
						case OperationType::Damp_Common:
							run_damp_common(op.coefficients[0]); break;
						default:
							throw std::runtime_error("Bad type.");
						}
						// fmt::print("{}", op.to_string());
						// INFO(op.to_string());
						// getchar();
						// fmt::print("{}\n", to_string()); 
						// INFO(to_string());
					}
				}
				clear_zero_elements();
			}

			/* Generate the operations and run */
			void run() {
				profiler _("QRAMCircuit::run");
				// generate_QRAM_operations();

				/*run_good();
				run_bad();*/
				prepare_bad();
				run_bad();
			}

			/* A plain version for circuit running */
			/* Used to help debugging QRAMCircuit::run() */
			void run_all()
			{
				profiler _("QRAMCircuit::run_all");
				// generate_QRAM_operations();
				prepare_all();
				run_bad();
			}

			/* A plain version for circuit running */
			/* Used to help debugging QRAMCircuit::run() */
			void run_good_only()
			{
				profiler _("QRAMCircuit::run_good_only");
				// generate_QRAM_operations();
				prepare_good_only();
				run_bad();
			}

			void run(std::string version)
			{
				if (version == "old")
				{
					run_all();
					return;
				}
				else if (version == "new")
				{
					run();
					return;
				}
				else if (version == "fast") {
					run_good_only();
					return;
				}
				throw_invalid_input();
			}
		};


	}
}