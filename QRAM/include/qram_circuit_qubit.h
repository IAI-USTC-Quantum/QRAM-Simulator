#pragma once

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

			QRAMCircuit(size_t address_sz, size_t data_sz);
			QRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory);

			inline void set_memory_random() { random_memory(memory, data_size); }
			void set_memory(const memory_t& new_memory);
			void set_memory(memory_t&& new_memory);
			inline auto& get_memory() { return memory; }
			inline auto& get_memory() const { return memory; }

			inline size_t get_qubit_num() const { return 2 * (pow2(addr_size) - 1); }
			inline size_t memory_size() const { return memory.size(); }

			// set branches for state_manipulator (see init_state for test usage)
			inline void set_branches(const std::vector<BranchGroup>& branch_groups_) { branch_groups = branch_groups_; }
			inline void set_branches(std::vector<BranchGroup>&& branch_groups_) { branch_groups = std::move(branch_groups_); }

			/* Fill branch_groups with n_inputs sampled (addr, bus) branches.
			*  uniform = true  : equal input probability for each branch
			*  uniform = false : input probability ~ U(0,1), normalized
			*/
			void _set_input_impl(size_t n_inputs, bool uniform);
			inline void set_input_random(size_t n_inputs) { _set_input_impl(n_inputs, false); }
			inline void set_input_uniform(size_t n_inputs) { _set_input_impl(n_inputs, true); }

			inline auto& get_branch_groups() const { return branch_groups; }
			inline auto& get_operations() const { return operations; }
			inline auto& get_branch_groups() { return branch_groups; }
			inline auto& get_operations() { return operations; }

			/* DEPRECATED */
			[[deprecated]] inline void clear_noise() { noise_parameters.clear(); }

			/* DEPRECATED */
			[[deprecated]] inline void add_noise_model(OperationType t, double v) {
				noise_parameters[t] = v;
			}
			inline void set_noise_models(const std::map<OperationType, double>& noises) {
				/* 概率参数范围校验；Damping 要求 γ < 1（γ=1 会在一步内
				   衰灭全部激发，使轨迹范数为 0，破坏 sample_output/normalization） */
				for (auto& [type, p] : noises)
				{
					if (!(p >= 0.0 && p <= 1.0) ||
						(type == OperationType::Damping && p >= 1.0))
						throw std::runtime_error(
							"set_noise_models: noise probability out of range "
							"(must be in [0,1]; Damping requires gamma < 1)");
				}
				noise_parameters = noises;
			}

			inline bool is_noise_free() const { return noise_parameters.size() == 0; }
			inline bool has_damping() const { return noise_parameters.find(OperationType::Damping) != noise_parameters.end(); }

			/* generate noisy QRAM operation */
			void initialize_system();

			double sample_and_get_fidelity();
			double get_fidelity() { return sample_and_get_fidelity(); }

			/*1. prepare valid_branch_view
			*
			* valid_branch_view will only include those branches
			* needed to evaluate
			*
			* 2. prepare first_good_branch
			* 3. prepare branches[i].good and set its bus output
			* 4. prepare good_branch_ids
			*/
			void prepare_bad();

			/* prepare valid_branch_view
			*
			*  valid_branch_view will only include those branches
			*  needed to evaluate
			*/
			void prepare_all();

			/*1. prepare valid_branch_view (good_only)
			*
			* valid_branch_view will only include those branches
			* needed to evaluate
			*
			* 2. prepare first_good_branch
			* 3. prepare branches[i].good and set its bus output
			* 4. prepare good_branch_ids
			*/
			void prepare_good_only();

			/* Execution of operations */
			void run_acopy(int layer);
			void run_swap(size_t layer_id);
			void run_cswap(size_t layer_id);
			void run_bitflip(size_t qubit_id);
			void run_phaseflip(size_t qubit_id, double depol_id);
			void run_bitphaseflip(size_t qubit_id);
			void run_depolarizing(size_t qubit_id, double depol_id);
			void run_hadamard();
			void run_fetchdata(size_t digit);
			void run_busin(size_t digit);
			void run_busout(size_t digit);
			void run_damp_full(size_t qubit_id, size_t step, double gamma);
			void run_damp_common(double gamma);

			void clear_zero_elements();

			std::string to_string() const;
			std::string to_string_full_info() const;

			double get_normalization_factor() const;
			double get_normalization_factor_with_damping() const;
			void normalization();

			void materialize_good_branches();

			/* Sample the output according to the branches and branch_probs*/
			void sample_output();

			/* DEPRECATED will be removed in future */
			[[deprecated]] inline std::string mem_to_string() const {
				return fmt::format("{}", memory);
			}

			void run_good();
			void run_bad();

			/* Generate the operations and run */
			void run();

			/* A plain version for circuit running */
			/* Used to help debugging QRAMCircuit::run() */
			void run_all();

			/* A plain version for circuit running */
			/* Used to help debugging QRAMCircuit::run() */
			void run_good_only();

			/* Full run of all branches: no pruning, used as ground truth */
			void run_full();

			/* Pruned run: only bad branches + the reference good branch */
			void run_normal();

			static constexpr auto FULL_VER = "full";
			static constexpr auto NORMAL_VER = "normal";

			void run(std::string version);
		};
	} // namespace qram_qubit
} // namespace qram_simulator
