#pragma once

#include "time_step.h"
#include <memory>

namespace qram_simulator {
	namespace qram_qubit {

		constexpr size_t Addr = 0;
		constexpr size_t Data = 1;
		constexpr bool ZeroState = false;
		constexpr bool OneState = true;
		constexpr size_t MAX_BUS_SIZE = 30; // Maximum bus size to prevent exponential memory explosion

		struct State
		{
			std::set<size_t> nz_elements;

			using element_type = decltype(nz_elements);
			using data_type = element_type::key_type;
			using iterator_type = decltype(nz_elements.begin());

			bool operator<(const State& rhs) const;
			bool operator==(const State& rhs) const;
			std::pair<iterator_type, iterator_type> get_layer_iter(size_t layerid);
			void get_layer_nonzero_nodes(size_t layerid, std::set<size_t>& nodes);
			void get_layer_nonzero_parent_nodes(size_t layerid, std::set<size_t>& nodes);
			inline void clear_zero_elements() {}

			constexpr static size_t get_nz_location(size_t layer, size_t pos, size_t lr);
			constexpr static size_t get_nz_location(size_t node_location, size_t lr);
			constexpr static size_t left_of(size_t node_location);
			constexpr static size_t right_of(size_t node_location);
			constexpr static size_t parent_of(size_t node_location);
			bool state_of(size_t nz_location) const;
			void busin(bus_t& bus, size_t digit);
			void busout(bus_t& bus, size_t digit);
			void flip(size_t nz_location);
			void cnot(size_t nz_location, bool condition);
			void general_swap(size_t nz_l, size_t nz_r);
			void internal_swap(size_t node_location);
			void internal_swap_layer(size_t layerid);
			void acopy(bool a);
			void cswap(size_t node_location);
			void cswap_layer(size_t layerid);
			void set_zero(size_t qubit_id);
			inline void clear() { nz_elements.clear(); }
		};

		struct SystemState
		{
			State state;
			bus_t data_bus;
			size_t bus_size;
			std::complex<double> amplitude = 1.0;

			using element_type = typename State::element_type;

			SystemState() = default;
			SystemState(bus_t bus_, size_t bus_sz) : data_bus(bus_), bus_size(bus_sz)
			{
			}
			SystemState(const SystemState& other) = default;

			inline bool operator<(const SystemState& rhs) const noexcept
			{
				return std::tie(data_bus, state) < std::tie(rhs.data_bus, rhs.state);
			}

			inline bool operator==(const SystemState& rhs) const noexcept
			{
				return std::tie(data_bus, state) == std::tie(rhs.data_bus, rhs.state);
			}

			void run_acopy(bool a);
			void run_swap(size_t node);
			void run_cswap(size_t node);
			void run_bitflip(size_t qubit_id);
			void run_phaseflip(size_t qubit_id, double depol_id);
			void run_bitphaseflip(size_t qubit_id);
			void run_depolarizing(size_t qubit_id, double depol_id);
			void set_zero(size_t qubit_id);
			void run_busin(size_t digit);
			void run_busout(size_t digit);

			inline void run_damp_common(double gamma)
			{
				amplitude *= std::pow(std::sqrt(1 - gamma), state.nz_elements.size());
			}
			inline void clear()
			{
				state.clear();
				amplitude = 1.0;
			}
		};

		struct Branch {

			constexpr static int qunit_type = arch_qubit;

			using state_type = SystemState;
			using element_type = typename state_type::element_type;
			using damp_prob_type = std::array<double, 1>;

			size_t address = 0;
			size_t bus_size = 1;
			bus_t bus_input = 0;
			double relative_multiplier = 1;
			std::shared_ptr<Branch> good_ref;
		private:
			bool good = false;
		public:

			std::vector<SystemState> system_states;
			size_t system_states_sz = 0;
			inline size_t get_branchid() const { return (address << bus_size) + bus_input; }

			inline void set_good(const std::shared_ptr<Branch>& first_good_ptr)
			{
				good = true;
				good_ref = first_good_ptr;
			}

			inline bool is_good() const
			{
				return good;
			}

			inline auto iterbeg()
			{
				return system_states.begin();
			}

			inline auto iterbeg() const
			{
				return system_states.begin();
			}

			inline auto iterend()
			{
				return system_states.begin() + system_states_sz;
			}

			inline auto iterend() const
			{
				return system_states.begin() + system_states_sz;
			}

			template<typename Pred>
			inline void remove_if(Pred pred)
			{
				auto iter = std::remove_if(iterbeg(), iterend(), pred);
				system_states_sz = iter - iterbeg();
			}

			inline void reset() {
				system_states_sz = 1;
				system_states[0] = std::move(SystemState(bus_input, bus_size));
				good = false;
				relative_multiplier = 1;
			}

			Branch() = default;
			Branch(size_t addr, size_t bus_sz, bus_t bus) {
				bus_size = bus_sz;
				address = addr;
				bus_input = bus;
				if (bus_sz >= MAX_BUS_SIZE)
					throw_invalid_input();
				system_states.resize(pow2(bus_sz + 1));
				reset();
			}
			Branch(size_t branchid, size_t bus_sz)
			{
				bus_size = bus_sz;
				address = branchid >> bus_sz;
				bus_input = branchid - (address << bus_sz);
				if (bus_sz >= MAX_BUS_SIZE)
					throw_invalid_input();
				system_states.resize(pow2(bus_sz + 1));
				reset();
			}
			Branch(const Branch& old_branch) = default;

			inline bool operator<(const Branch& other) const {
				return std::tie(address, bus_input, system_states) <
					std::tie(other.address, other.bus_input, other.system_states);
			}

			inline bool operator==(const Branch& other) const {
				return std::tie(address, bus_input, system_states) ==
					std::tie(other.address, other.bus_input, other.system_states);
			}

			double get_prob() const;
			complex_t get_fidelity(const memory_t& memory) const;
			std::string to_string() const;
			void run_fetchdata(memory_t& memory, size_t digit);
			void run_hadamard();
			void try_merge();

			inline std::optional<element_type> get_good_branch_system() const
			{
				if (system_states.size() == 0) return std::nullopt;

				return system_states.begin()->state.nz_elements;
			}

			void run_damp_common(double gamma);

			damp_prob_type get_prob_damp(size_t qubit_id) const;

			void run_damp_full(size_t qubit_id, size_t k);

			//inline static void get_multiplier(
			//	double gamma,
			//	const TimeStep& time_step,
			//	size_t step,
			//	const std::vector<BranchGroup<Branch>>& branches,
			//	size_t first_good_branch,
			//	const std::vector<size_t>& good_branch_ids,
			//	// std::vector<double>& multipliers,
			//	const memory_t &memory
			//)
			//{
			//	time_step.get_multiplier_qubit(gamma, step, branches, first_good_branch, good_branch_ids/*, multipliers*/);
			//}

			void remove_mismatch_state(const State::element_type& target_state);
			void remove_all_state();

			inline void clear_all_state() { system_states_sz = 0; }

		};

		struct BranchGroup
		{
			size_t address;
			std::vector<Branch> branches_input; // |ui(input)>
			std::vector<Branch> branches;       // |ui>
			std::vector<double> branch_probs;   // <ui|ui>
			std::vector<double> state_probs;    // <ψi|ψi>

			bool is_good = false;
			std::shared_ptr<BranchGroup> good_ref;
			double relative_multiplier = 1.0;

			BranchGroup(size_t addr) : address(addr)
			{}

			void reset()
			{
				branches = branches_input;
				is_good = false;
			}

			void set_good(const std::shared_ptr<BranchGroup>& good_ref_)
			{
				is_good = true;
				good_ref = good_ref_;
			}

			void set_empty_state()
			{
				branches.clear();
			}

			complex_t get_fidelity()
			{
				throw_not_implemented();
				return 0;
			}

			// 1. construct from
			// 2. reconstruct to
			// 3. calculate probability
			Branch::damp_prob_type get_prob_damp(size_t qubit_id) const
			{
				size_t branch_sz = branches.size();
				Branch::damp_prob_type ret;
				if (branch_sz == 0) return ret;
				ret.fill(0);
				for (size_t bid = 0; bid < branch_sz; ++bid)
				{
					double prob = state_probs[bid];
					auto&& damp_prob = branches[bid].get_prob_damp(qubit_id);
					for (size_t i = 0; i < ret.size(); ++i)
					{
						ret[i] += damp_prob[i] * branch_probs[i];
					}
				}
				return ret;
			}

			double get_prob() const
			{
				if (!is_good)
				{
					double ret = 0;
					for (size_t i = 0; i < branches.size(); ++i)
					{
						ret += branches[i].get_prob() * branch_probs[i];
					}
					return ret;
				}
				else
				{
					double ret = 0;
					for (size_t i = 0; i < branches.size(); ++i)
					{
						ret += branch_probs[i];
					}
					return ret;
				}
			}

			bool sample_output_no_damping(Branch::element_type& output, double& r)
			{
				for (size_t i = 0; i < state_probs.size(); ++i)
				{
					if (r > state_probs[i]) {
						r -= state_probs[i];
						continue;
					}
					else {
						for (auto iter = branches[i].iterbeg();
							iter != branches[i].iterend(); ++iter)
						{
							if (r > abs_sqr(iter->amplitude))
								r -= abs_sqr(iter->amplitude);
							else {
								output = iter->state.nz_elements;
								return true;
							}
						}
					}
				}
				return false;
			}

			bool sample_output_with_damping(Branch::element_type& output, double& r)
			{
				for (size_t i = 0; i < state_probs.size(); ++i)
				{
					for (auto iter = branches[i].iterbeg();
						iter != branches[i].iterend(); ++iter)
					{
						if (r > abs_sqr(iter->amplitude))
							r -= abs_sqr(iter->amplitude);
						else {
							output = iter->state.nz_elements;
							return true;
						}
					}
				}
				return false;
			}

			void remove_mismatch_state(const Branch::element_type& target)
			{
				for (auto& branch : branches) {
					branch.remove_mismatch_state(target);
				}
			}
		};
	} // namespace qram_qubit
} // namespace qram_simulator 
