#include "qram_circuit_qubit.h"

namespace qram_simulator {
	namespace qram_qubit {

		QRAMCircuit::QRAMCircuit(size_t address_sz, size_t data_sz) :
			addr_size(address_sz), data_size(data_sz),
			time_step(address_sz, data_sz)
		{
			memory.resize(pow2(addr_size));
		}

		QRAMCircuit::QRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory_) :
			addr_size(address_sz), data_size(data_sz),
			time_step(address_sz, data_sz)
		{
			memory_.resize(pow2(addr_size));
			set_memory(std::move(memory_));
		}

		void QRAMCircuit::set_memory(const memory_t& new_memory)
		{
			if (new_memory.size() != pow2(addr_size))
				throw std::runtime_error("Bad Input.");

			memory = new_memory;
		}

		void QRAMCircuit::set_memory(memory_t&& new_memory)
		{
			if (new_memory.size() != pow2(addr_size))
				throw std::runtime_error("Bad Input.");

			memory = std::move(new_memory);
		}

		void QRAMCircuit::_set_input_impl(size_t n_inputs, bool uniform)
		{
			static std::uniform_real_distribution<double> urd(0, 1);

			if (addr_size + data_size > 64)
				throw_invalid_input();

			size_t max_branch_size = pow2(addr_size + data_size);
			auto inputsz = std::min(n_inputs, max_branch_size);

			std::vector<size_t> sampled_ids;
			sampled_ids.reserve(inputsz);
			std::vector<double> probs;
			probs.reserve(inputsz);

			double total_prob = 0;
			if (max_branch_size < pow2(30) &&
				inputsz * 1.0 / max_branch_size > 0.3)
			{
				std::vector<size_t> indices(max_branch_size);
				std::iota(indices.begin(), indices.end(), 0);
				for (size_t i = 0; i < inputsz; ++i)
				{
					std::uniform_int_distribution<size_t> ud(i, indices.size() - 1);
					size_t a = ud(random_engine::get_engine());

					double r = uniform ? 1.0 : urd(random_engine::get_engine());
					sampled_ids.push_back(indices[a]);
					probs.push_back(r);
					total_prob += r;

					std::swap(indices[i], indices[a]);
				}
			}
			else
			{
				std::set<size_t> unique_id;
				std::uniform_int_distribution<size_t> ud(0, max_branch_size - 1);
				while (sampled_ids.size() < inputsz)
				{
					size_t a = ud(random_engine::get_engine());
					if (!unique_id.insert(a).second)
						continue;

					double r = uniform ? 1.0 : urd(random_engine::get_engine());
					sampled_ids.push_back(a);
					probs.push_back(r);
					total_prob += r;
				}
			}

			branch_groups.clear();
			std::map<size_t, size_t> group_of_addr;
			for (size_t i = 0; i < inputsz; ++i)
			{
				size_t addr = sampled_ids[i] >> data_size;
				bus_t bus = sampled_ids[i] - (addr << data_size);
				auto [it, flag] = group_of_addr.try_emplace(addr, branch_groups.size());
				if (flag)
					branch_groups.emplace_back(addr);
				auto& group = branch_groups[it->second];
				group.branches_input.emplace_back(addr, data_size, bus);
				group.branch_probs.push_back(probs[i] / total_prob);
				group.state_probs.push_back(probs[i] / total_prob);
			}
		}

		/* generate noisy QRAM operation */
		void QRAMCircuit::initialize_system() {

			profiler _("QRAMCircuit::initialize_system");
			operations = time_step.generate(noise_parameters, Branch::qunit_type);
			good_branch_group_ids.clear();
			first_good_branch_group = -1;
			valid_branch_group_view.clear();
			fired_jump_count = 0;
			std::for_each(branch_groups.begin(), branch_groups.end(),
				[](BranchGroup& branchgroup) { branchgroup.reset(); }
			);
		}

		double QRAMCircuit::sample_and_get_fidelity() {
			profiler _("sample_and_get_fidelity");

			sample_output();

			if (branch_groups.size() == 0)
				return 0;

			/* the good groups' relative_multiplier is already embedded in
			their materialized amplitudes */
			complex_t ret = 0;
			for (size_t i = 0; i < branch_groups.size(); ++i)
			{
				ret += branch_groups[i].get_fidelity(memory);
			}
			return abs_sqr(ret);
		}

		void QRAMCircuit::prepare_bad()
		{
			profiler _("QRAMCircuit::prepare_bad");
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
						branch_groups[i].set_good(head + first_good_branch_group);

						good_branch_group_ids.push_back(i);
						continue;
					}
				}
				valid_branch_group_view.emplace_back(head + i);
			}
		}

		void QRAMCircuit::prepare_all()
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

		void QRAMCircuit::prepare_good_only()
		{
			profiler _("QRAMCircuit::prepare_good_only");
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
						branch_groups[i].set_good(head + first_good_branch_group);

						good_branch_group_ids.push_back(i);
						continue;
					}
				}
				else
				{
					branch_groups[i].set_empty_state();
				}
			}
			if (first_good_branch_group < 0)
				// fail to find any of the good
				prepare_all();
		}

		void QRAMCircuit::run_acopy(int layer)
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
			}
		}

		void QRAMCircuit::run_swap(size_t layer_id)
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
			}
		}

		void QRAMCircuit::run_cswap(size_t layer_id)
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
			}
		}

		void QRAMCircuit::run_bitflip(size_t qubit_id)
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
			}
		}

		void QRAMCircuit::run_phaseflip(size_t qubit_id, double depol_id)
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
			}
		}

		void QRAMCircuit::run_bitphaseflip(size_t qubit_id)
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

		void QRAMCircuit::run_depolarizing(size_t qubit_id, double depol_id)
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
			}
		}

		void QRAMCircuit::run_hadamard() {
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
			}
		}

		void QRAMCircuit::run_fetchdata(size_t digit) {
			for (auto branch_group_ptr : valid_branch_group_view)
			{
				std::for_each(
					branch_group_ptr->branches.begin(),
					branch_group_ptr->branches.end(),
					[this, digit](Branch& branch) {
						branch.run_fetchdata(memory, digit);
					}
				);
			}
		}

		void QRAMCircuit::run_busin(size_t digit) {
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
			}
		}

		void QRAMCircuit::run_busout(size_t digit) {
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

		void QRAMCircuit::run_damp_full(size_t qubit_id, size_t step, double gamma) {

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
				throw_invalid_input();
			}
			prob_damp.resize(damp_op_num);
			if (first_good_branch_group >= 0)
			{
				time_step.get_multiplier_qubit(gamma, step, branch_groups,
					first_good_branch_group, good_branch_group_ids);

				auto&& ref_prob_full = branch_groups[first_good_branch_group].get_prob_damp(qubit_id);
				const auto& refg = branch_groups[first_good_branch_group];
				double ref_input = 0;
				for (double bp : refg.branch_probs) ref_input += bp;

				for (size_t i = 0; i < good_branch_group_ids.size(); ++i) {
					size_t id = good_branch_group_ids[i];
					if (branch_groups[id].annihilated) continue;
					double g_input = 0;
					for (double bp : branch_groups[id].branch_probs) g_input += bp;
					for (auto k = 0; k < damp_op_num; ++k) {
						prob_damp[k] += (ref_prob_full[k] / ref_input)
							* branch_groups[id].relative_multiplier
							* g_input;
					}
				}
			}
			/* first: decide whether this qubit is non-zero*/
			for (auto branch_ptr : valid_branch_group_view)
			{
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
					/* No mid-run action is taken on the un-evolved good
					   groups. Their predictable trajectories share the
					   reference branch's component-wise tree configuration
					   (XOR mirror; docs/sphinx/source/en/paper/
					   qubit_qram_pruning.md, Theorem 3), so the fired K1
					   projection acts on them exactly as on the reference:
					   a good branch excited at the fired node decays in
					   lockstep with the reference (the mirror amplitudes
					   already carry that), and an idle one is annihilated
					   together with it. Every mid-run nominal estimate
					   (ref_unit_norm * relative_multiplier * g_input here
					   and in get_normalization_factor_with_damping) tracks
					   the reference's surviving norm on its own, so the
					   `annihilated` skips at those sites are defensive only
					   (nothing sets the flag before materialization).
					   KNOWN TRAP (do not reintroduce): a blanket "kill every
					   good group on fire" mid-run marker was tried and is
					   WRONG -- it breaks trajectories in which the reference
					   survives the fire while some good groups are excited
					   at the fired node (perf_scan eps=1e-3 seeds
					   7744644691974779904 at n=10/12, 1183008555922881536,
					   3227135189013113856, 3287329390135220224,
					   4366419402265368576 at n=12; deviations up to 6.6e-1).
					   Reference survival alone decides everything at
					   materialization; see materialize_good_branches. */
					++fired_jump_count;
					break;
				}
				r -= prob_damp[k];
			}
			// no need to run extra damp_common
			// because always include a damp_common at every time step
		}

		void QRAMCircuit::run_damp_common(double gamma) {
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

		void QRAMCircuit::clear_zero_elements()
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

		namespace {
			/* Dump every branch group: address, aggregate probability and
			the per-branch input probabilities and states. Mirrors the
			qutrit per-branch "prob = ... state = ..." line, grouped by
			address, plus the good/predicted markers of the pruning. */
			void format_branch_groups(const std::vector<BranchGroup>& groups, std::vector<char>& buf)
			{
				for (size_t g = 0; g < groups.size(); ++g) {
					const auto& group = groups[g];
					fmt::format_to(std::back_inserter(buf),
						"[{}] addr={} prob={:.4f}", g, group.address, group.get_prob());
					if (group.is_good) {
						fmt::format_to(std::back_inserter(buf),
							" (good, mult={:.4f}{})", group.relative_multiplier,
							group.predicted ? ", predicted" : "");
					}
					fmt::format_to(std::back_inserter(buf), "\n");
					for (size_t b = 0; b < group.branches.size(); ++b) {
						fmt::format_to(std::back_inserter(buf),
							"\tprob = {:.4f}\tstate = {}\n",
							group.branch_probs[b], group.branches[b].to_string());
					}
				}
			}
		}

		std::string QRAMCircuit::to_string() const {
			profiler _("qram::to_string");
			std::vector<char> buf;

			fmt::format_to(std::back_inserter(buf), "memory={}\n", memory);
			format_branch_groups(branch_groups, buf);
			return { buf.data(), buf.size() };
		}

		std::string QRAMCircuit::to_string_full_info() const {
			std::vector<char> buf;

			fmt::format_to(std::back_inserter(buf), "memory={}\n", memory);
			format_branch_groups(branch_groups, buf);
			fmt::format_to(std::back_inserter(buf), "Operations = \n{}\n", operations.to_string());
			fmt::format_to(std::back_inserter(buf), "FinalSystem = {}\n", final_system_state);
			fmt::format_to(std::back_inserter(buf), "first_good_branch_group = {}\n", first_good_branch_group);
			fmt::format_to(std::back_inserter(buf), "prob = {}\n", get_normalization_factor());
			return { buf.data(), buf.size() };
		}

		double QRAMCircuit::get_normalization_factor() const
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

		double QRAMCircuit::get_normalization_factor_with_damping() const
		{
			double new_prob = 0;
			for (auto branch_group_ptr : valid_branch_group_view) {
				new_prob += branch_group_ptr->get_prob();
				check_nan(new_prob);
			}
			if (first_good_branch_group >= 0)
			{
				/* a group's get_prob() already carries its own input
				weight; fold with the reference's per-unit-weight norm */
				const auto& ref = branch_groups[first_good_branch_group];
				double ref_input = 0;
				for (double bp : ref.branch_probs) ref_input += bp;
				double ref_unit_norm = ref.get_prob() / ref_input;
				for (size_t i = 0; i < good_branch_group_ids.size(); ++i)
				{
					const auto& g = branch_groups[good_branch_group_ids[i]];
					if (g.annihilated) continue;
					double g_input = 0;
					for (double bp : g.branch_probs) g_input += bp;
					new_prob += ref_unit_norm * g.relative_multiplier * g_input;
				}
			}
			return new_prob;
		}

		void QRAMCircuit::normalization()
		{
			/* after materialization the good groups carry real states and
			must be normalized exactly like the evolved ones */
			double multiplier = 0;
			for (auto& group : branch_groups) {
				multiplier += group.get_prob();
				check_nan(multiplier);
			}

			for (auto& group : branch_groups) {
				for (auto& branch : group.branches)
				{
					for (auto iter = branch.iterbeg(); iter != branch.iterend(); ++iter)
						iter->amplitude /= sqrt(multiplier);
				}
			}
		}

		/* Materialize the predicted final states of every good branch
		*  group from the reference good group (the XOR mirror):
		*
		*    data_bus(c) := data_bus_ref(c') with c' = c XOR delta,
		*    delta = (j_ref XOR d[addr_ref]) XOR (j XOR d[addr]),
		*    amplitude := amplitude_ref * sqrt(relative_multiplier).
		*
		*  This is exact: the H-K0-H data-bus amplitudes depend only on
		*  the Hamming weight of the error pattern relative to the ideal
		*  output (window 2n is address- and input-independent), and the
		*  address part is carried by relative_multiplier = (1-gamma)^dc.
		*  After materialization the generic prob/sampling/fidelity paths
		*  apply to good groups unchanged.
		*/
		void QRAMCircuit::materialize_good_branches()
		{
			if (first_good_branch_group < 0) return;

			auto& ref = branch_groups[first_good_branch_group];

			/* any reference branch carries the full 2^k data-bus
			   structure; pick the first with a surviving state */
			Branch* ref_branch = nullptr;
			for (auto& rb : ref.branches)
				if (rb.system_states_sz > 0) { ref_branch = &rb; break; }

			if (ref_branch == nullptr)
			{
				/* The reference good branch was fully annihilated (a fired
				   K1 jump whose node was |0> along the whole reference
				   trajectory). Every other good branch shares the
				   reference's component-wise tree configuration, so the
				   same projection annihilated it too; the full mode leaves
				   all of them at exactly zero weight. Flag them dead so
				   the un-materialized nominal paths contribute nothing.
				   (After reference death the mid-run estimates vanish on
				   their own: ref.get_prob() == 0 makes ref_unit_norm == 0,
				   so no mid-run gating is required.)
				   FIX STATUS: this early-return is the load-bearing piece
				   of the pruned-vs-full exactness repair. At the time of
				   writing it is compile-verified only -- run the
				   QubitPaperExactness ctest battery, then the perf_scan
				   re-check (1000/1000 pairs at |dF| <= 1e-9), before
				   trusting it. ASSUMPTION: reference death is total and
				   shared, i.e. no good branch survives a fire that killed
				   the reference. A battery failure showing nonzero
				   full-mode weight on unmarked addresses is the symptom of
				   that assumption failing. */
				for (size_t id : good_branch_group_ids)
					branch_groups[id].mark_annihilated();
				return;
			}

			for (size_t idx = 0; idx < good_branch_group_ids.size(); ++idx)
			{
				size_t id = good_branch_group_ids[idx];
				auto& group = branch_groups[id];
				if (group.annihilated) continue;
				double amp_factor = std::sqrt(group.relative_multiplier);

				for (size_t b = 0; b < group.branches.size(); ++b)
				{
					auto& br = group.branches[b];

					size_t delta = (ref_branch->bus_input ^ memory[ref.address])
						^ (br.bus_input ^ memory[group.address]);

					br.system_states_sz = 0;
					for (auto it = ref_branch->iterbeg(); it != ref_branch->iterend(); ++it)
					{
						if (br.system_states_sz >= br.system_states.size())
							br.system_states.resize(br.system_states_sz + 1);
						br.system_states[br.system_states_sz] = *it;
						br.system_states[br.system_states_sz].data_bus = it->data_bus ^ delta;
						br.system_states[br.system_states_sz].amplitude = it->amplitude * amp_factor;
						++br.system_states_sz;
					}
				}
				group.predicted = true;
			}
		}

		void QRAMCircuit::sample_output()
		{
			bool has_damping = noise_parameters.find(OperationType::Damping) != noise_parameters.end();
			if (has_damping)
			{
				/* preprocess the multipliers */
				double gamma = noise_parameters[OperationType::Damping];
				if (first_good_branch_group >= 0)
				{
					time_step.get_multiplier_qubit(
						gamma,
						time_step.last_step(),
						branch_groups,
						first_good_branch_group,
						good_branch_group_ids
					);
				}
			}
			materialize_good_branches();

			profiler _("sample_output");

			if (branch_groups.size() == 0)
			{
				// no branch at all
				return;
			}

			double factor = 0;
			for (auto& group : branch_groups) {
				factor += group.get_prob();
				check_nan(factor);
			}

			std::uniform_real_distribution<double> urd(0, factor);
			double r = urd(random_engine::get_engine());

			bool set = false;
			for (auto& group : branch_groups) {
				set = group.sample_output_with_damping(final_system_state, r);
				if (set) break;
			}

			if (!set)
			{
				// r may exceed the accumulated weights by floating-point
				// residue; fall back to the last available state
				for (auto& group : branch_groups) {
					for (auto& branch : group.branches) {
						for (auto iter = branch.iterbeg(); iter != branch.iterend(); ++iter) {
							if (!ignorable(std::abs(iter->amplitude))) {
								final_system_state = iter->state.nz_elements;
								set = true;
							}
						}
					}
				}
			}

			if (!set) // no state available at all
			{
				throw_bad_result();
			}

			for (auto& group : branch_groups) {
				group.remove_mismatch_state(final_system_state);
			}

			normalization();
		}

		void QRAMCircuit::run_good()
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
						branch_groups[i].set_good(head + first_good_branch_group);
					}
				}
			}
		}

		void QRAMCircuit::run_bad()
		{
			profiler _("run_bad");
			int step = 0;
			for (OperationPack& ops : operations.time_slices) {
				step++;
				for (Operation& op : ops.operations) {
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
				}
			}
			clear_zero_elements();
		}

		/* Generate the operations and run */
		void QRAMCircuit::run() {
			profiler _("QRAMCircuit::run");
			prepare_bad();
			run_bad();
		}

		/* A plain version for circuit running */
		/* Used to help debugging QRAMCircuit::run() */
		void QRAMCircuit::run_all()
		{
			profiler _("QRAMCircuit::run_all");
			prepare_all();
			run_bad();
		}

		/* A plain version for circuit running */
		/* Used to help debugging QRAMCircuit::run() */
		void QRAMCircuit::run_good_only()
		{
			profiler _("QRAMCircuit::run_good_only");
			prepare_good_only();
			run_bad();
		}

		/* Full run of all branches: no pruning, used as ground truth */
		void QRAMCircuit::run_full()
		{
			profiler _("QRAMCircuit::run_full");
			run_all();
		}

		/* Pruned run: only bad branches + the reference good branch */
		void QRAMCircuit::run_normal()
		{
			profiler _("QRAMCircuit::run_normal");
			run();
		}

		void QRAMCircuit::run(std::string version)
		{
			if (version == "old" || version == FULL_VER)
			{
				run_all();
				return;
			}
			else if (version == "new" || version == NORMAL_VER)
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

	} // namespace qram_qubit
} // namespace qram_simulator
