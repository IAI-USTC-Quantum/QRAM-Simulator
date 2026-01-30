#pragma once

#include "qram_branch_qutrit.h"

namespace qram_simulator {
	namespace qram_qutrit {
		struct QRAMCircuit {
			using qram_state_t = typename SubBranch::element_type;

		public:
			size_t address_size;
			size_t data_size;
			TimeStep time_step;
			memory_t memory;

			/* Internal operations */
			// TimeSlices raw_operations;
			TimeSlices operations;

			/* noise parameters */
			noise_t noise_parameters;

			/* All branches (normalized, disentangled) */
			std::vector<Branch> branches;

			/* Probabilities of all branches */
			/* Used in sampling and damping noise evaluation */
			std::vector<double> branch_probs;

			/* Contains all branches need to be computed */
			/* version:
				"full"		: all branches
				"normal"	: 1st good branch and all bad branches
				"fast"		: no branches should be computed.
			*/
			std::vector<Branch*> valid_branch_view;

			/* Contain the indices of the 1st good branch */
			/* And computed as the reference */
			int64_t first_good_branch = -1;

			/* Contains all good branch indices except 1st good branch */
			std::vector<size_t> good_branch_ids;

		public:
			// double total_prob = 0;
			qram_state_t final_system_state;

			QRAMCircuit(size_t address_sz, size_t data_sz)
				: address_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(address_size));
			}
			QRAMCircuit(size_t address_sz, size_t data_sz, const memory_t& memory_)
				: address_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(address_size));
				set_memory(memory_);
			}
			QRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory_)
				: address_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(address_size));
				set_memory(std::move(memory_));
			}

			inline size_t get_qubit_num() const { return 2 * (pow2(address_size) - 1); }
			inline size_t memory_size() const { return memory.size(); }

			virtual void set_memory_random();
			virtual void set_memory(const memory_t& new_memory);
			virtual void set_memory(memory_t&& new_memory);
			inline auto& get_memory() { return memory; }
			inline auto& get_memory() const { return memory; }

			inline auto& get_branches() const { return branches; }
			inline auto& get_branches() { return branches; }
			inline auto& get_branch_probs() const { return branch_probs; }
			inline auto& get_branch_probs() { return branch_probs; }
			inline auto& get_operations() const { return operations; }
			inline auto& get_operations() { return operations; }

			inline void set_noise_models(const noise_t& noises) { noise_parameters = noises; }
			inline auto& get_noise_models() { return noise_parameters; }
			inline auto& get_noise_models() const { return noise_parameters; }
			inline bool is_noise_free() const { return noise_parameters.size() == 0; }
			inline bool has_damping() const { return noise_parameters.find(OperationType::Damping) != noise_parameters.end(); }

			/* generate noisy QRAM operation */
			void initialize_system();

			/* Randomly fill branch_probs and branches */
			void set_input_random(size_t n_inputs);

			/* Randomly fill branch_probs */
			void set_input_uniform(size_t n_inputs);

			double sample_and_get_fidelity();

			/*1. prepare valid_branch_view
			*
			* valid_branch_view will only include those branches
			* needed to evaluate
			*
			* 2. prepare first_good_branch
			* 3. prepare branches[i].good and set its bus output
			* 4. prepare good_branch_ids
			*/
			void pick_good_bad();

			/* prepare valid_branch_view
			*
			*  valid_branch_view will only include those branches
			*  needed to evaluate
			*/
			void pick_all();

			/*1. prepare valid_branch_view (good_only)
			*
			* valid_branch_view will only include those branches
			* needed to evaluate
			*
			* 2. prepare first_good_branch
			* 3. prepare branches[i].good and set its bus output
			* 4. prepare good_branch_ids
			*/
			// void prepare_good_only();

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

			double get_normalization_factor() const;
			double get_normalization_factor_without_damping() const;
			double get_normalization_factor_with_damping() const;
			void normalization();
			void normalization_without_damping();
			void normalization_with_damping();

			void sample_output_without_normalization();
			void sample_output_without_normalization_with_damping();
			void sample_output_without_normalization_without_damping();
			/* Sample the output according to the branches and branch_probs*/
			/* without damping */
			void sample_output();

			void run_good();
			void run_valid_branches();

			/* Normal algorithm */
			void run_normal();

			/* Run for all branches */
			/* Used to help debugging QRAMCircuit::run() */
			void run_full();

			void run_good_only();
			void run_good_only_without_damping();
			void run_good_only_with_damping();
			void run(const std::string& version);

			std::string to_string() const;
			std::string to_string_full_info() const;

			static constexpr auto FULL_VER = "full";
			static constexpr auto NORMAL_VER = "normal";

			virtual ~QRAMCircuit() {}
		};
	} // namespace qram_qutrit

	// using QRAMCircuit = qram_qutrit::QRAMCircuit;

} // namespace qram_simulator