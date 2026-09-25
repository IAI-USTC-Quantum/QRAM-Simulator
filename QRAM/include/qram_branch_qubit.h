#pragma once

#include "time_step.h"

namespace qram_simulator {
	namespace qram_qubit {

		/// State internal layout constant: address slot index (relative to nz_location)
		constexpr size_t Addr = 0;
		/// State internal layout constant: data slot index (relative to nz_location)
		constexpr size_t Data = 1;
		/// Qubit basis state constant: |0>
		constexpr bool ZeroState = false;
		/// Qubit basis state constant: |1>
		constexpr bool OneState = true;

		/**
		 * @brief A sparse computational basis state represented by the set of non-zero basis nodes.
		 *
		 * The state of a complete binary QRAM tree only needs to record the node positions
		 * in |1> (nz_elements); all other nodes are implicitly |0>. The level-order numbering of the tree
		 * satisfies the bitwise left/right/parent relations. All gate operations are implemented
		 * as insertions/deletions on the sparse set, with complexity on the order of the number of non-zero elements.
		 */
		struct State
		{
			/// Set of node positions in |1> (non-zero elements)
			std::set<size_t> nz_elements;

			using element_type = decltype(nz_elements);
			using data_type = element_type::key_type;
			using iterator_type = decltype(nz_elements.begin());

			/// Lexicographic comparison (natural order of the set)
			bool operator<(const State& rhs) const;
			/// Set equality test
			bool operator==(const State& rhs) const;
			/// Returns the element iterator range [begin, end) of layer layerid
			std::pair<iterator_type, iterator_type> get_layer_iter(size_t layerid);
			/// Collects the non-zero node indices of layer layerid
			void get_layer_nonzero_nodes(size_t layerid, std::set<size_t>& nodes);
			/// Collects the non-zero parent node indices of layer layerid
			void get_layer_nonzero_parent_nodes(size_t layerid, std::set<size_t>& nodes);
			/// Compatibility interface: a qubit basis state has no zero-amplitude elements to clean up, so this is always a no-op
			inline void clear_zero_elements() {}

			/// @brief Encodes a non-zero element location from a layer number and an in-layer position.
			/// @param layer Layer number @param pos In-layer position @param lr Left (0) / right (1) child
			/// @return The location key of the node in nz_elements
			constexpr static size_t get_nz_location(size_t layer, size_t pos, size_t lr);
			/// @brief Encodes a non-zero element location from a node location and a left/right child flag.
			/// @param node_location Node location @param lr Left (0) / right (1) child
			constexpr static size_t get_nz_location(size_t node_location, size_t lr);
			/// Location of the left child of node node_location
			constexpr static size_t left_of(size_t node_location);
			/// Location of the right child of node node_location
			constexpr static size_t right_of(size_t node_location);
			/// Location of the parent of node node_location
			constexpr static size_t parent_of(size_t node_location);
			/// Checks whether the qubit at location nz_location is |1>
			bool state_of(size_t nz_location) const;
			/// Bus input: writes bus bit digit into the address slot
			void busin(bus_t& bus, size_t digit);
			/// Bus output: reads the address slot out to bus bit digit
			void busout(bus_t& bus, size_t digit);
			/// Flips the qubit at location nz_location (X gate)
			void flip(size_t nz_location);
			/// Conditional NOT: flips nz_location when condition is true
			void cnot(size_t nz_location, bool condition);
			/// General swap between two arbitrary locations nz_l / nz_r
			void general_swap(size_t nz_l, size_t nz_r);
			/// Internal (address slot - data slot) swap within node node_location
			void internal_swap(size_t node_location);
			/// Internal swap of all nodes at layer layerid
			void internal_swap_layer(size_t layerid);
			/// Address copy: writes the address slot into the data slot in direction a (FirstCopy)
			void acopy(bool a);
			/// Controlled swap (CSWAP) at node node_location
			void cswap(size_t node_location);
			/// Controlled swap of all nodes at layer layerid
			void cswap_layer(size_t layerid);
			/// Sets the qubit at location qubit_id to zero (noise SetZero)
			void set_zero(size_t qubit_id);
			/// Clears all non-zero elements (back to all |0>)
			inline void clear() { nz_elements.clear(); }
		};

		/**
		 * @brief System state with a data bus and an amplitude (a single element of a branch trajectory).
		 *
		 * Composed of the sparse tree state State, the data_size-bit data bus data_bus,
		 * and the complex amplitude; the run_* series applies gate / noise operations
		 * to the tree state and the bus simultaneously, evolving one trajectory.
		 */
		struct SystemState
		{
			/// Sparse tree state
			State state;
			/// Data bus (a data_size-bit integer)
			bus_t data_bus;
			/// Bus width
			size_t bus_size;
			/// Trajectory amplitude (noise damping acts on this coefficient)
			std::complex<double> amplitude = 1.0;

			using element_type = typename State::element_type;

			SystemState() = default;
			/// @brief Constructs with the given initial bus value.
			/// @param bus_ Bus value @param bus_sz Bus width
			SystemState(bus_t bus_, size_t bus_sz) : data_bus(bus_), bus_size(bus_sz)
			{
			}
			SystemState(const SystemState& other) = default;

			/// Lexicographic comparison by (bus value, tree state)
			inline bool operator<(const SystemState& rhs) const noexcept
			{
				return std::tie(data_bus, state) < std::tie(rhs.data_bus, rhs.state);
			}
			/// Equality test (same bus value and tree state)
			inline bool operator==(const SystemState& rhs) const noexcept
			{
				return std::tie(data_bus, state) == std::tie(rhs.data_bus, rhs.state);
			}

			/// Address copy: writes into the data slot in direction a
			void run_acopy(bool a);
			/// Plain swap at node node
			void run_swap(size_t node);
			/// Controlled swap at node node
			void run_cswap(size_t node);
			/// Bit flip at location qubit_id (noise)
			void run_bitflip(size_t qubit_id);
			/// Phase flip at location qubit_id (noise, amplitude negated)
			void run_phaseflip(size_t qubit_id, double depol_id);
			/// Combined bit-phase flip at location qubit_id (noise)
			void run_bitphaseflip(size_t qubit_id);
			/// Depolarizing at location qubit_id (noise)
			void run_depolarizing(size_t qubit_id, double depol_id);
			/// Set to zero at location qubit_id (noise)
			void set_zero(size_t qubit_id);
			/// Bus input of bit digit
			void run_busin(size_t digit);
			/// Bus output of bit digit
			void run_busout(size_t digit);

			/// Common amplitude damping: multiplies by sqrt(1-gamma)^|nz| per the number of non-zero elements
			inline void run_damp_common(double gamma)
			{
				amplitude *= std::pow(std::sqrt(1 - gamma), state.nz_elements.size());
			}
			/// Clears the tree state and resets the amplitude
			inline void clear()
			{
				state.clear();
				amplitude = 1.0;
			}
		};

		/**
		 * @brief Trajectory container of a single (address, bus_input) input branch.
		 *
		 * Holds a set of SystemState (multiple trajectories split from the same input
		 * under noise / merging), and supports fidelity computation against the data tree and output sampling.
		 */
		struct Branch {

			/// Architecture identifier: qubit
			constexpr static int qunit_type = arch_qubit;

			using state_type = SystemState;
			using element_type = typename state_type::element_type;
			/// Damping probability array type (single element in the qubit architecture)
			using damp_prob_type = std::array<double, 1>;

			/// Branch address
			size_t address = 0;
			/// Bus width
			size_t bus_size = 1;
			/// Input bus value
			bus_t bus_input = 0;
			/// Relative Damping multiplier (relative to the reference good branch, used for amplitude prediction)
			double relative_multiplier = 1;
			/// Prediction reference for good branches (pointer to the first good branch)
			Branch* good_ref = nullptr;
		private:
			bool good = false;
		public:

			/// All trajectories of this branch (including split ones)
			std::vector<SystemState> system_states;
			/// Number of valid trajectories (the first system_states_sz entries of system_states are valid)
			size_t system_states_sz = 0;
			/// Branch ID: (address << bus_size) + bus_input
			inline size_t get_branchid() const { return (address << bus_size) + bus_input; }

			/// Marks this branch as a good branch and binds the prediction reference
			void set_good(Branch* first_good_ptr);

			/// Whether this branch is a good branch (loading result consistent with the data tree)
			inline bool is_good() const
			{
				return good;
			}

			/// Iterator to the beginning of valid trajectories
			inline auto iterbeg()
			{
				return system_states.begin();
			}

			/// Iterator to the beginning of valid trajectories (const)
			inline auto iterbeg() const
			{
				return system_states.begin();
			}

			/// Iterator to the end of valid trajectories
			inline auto iterend()
			{
				return system_states.begin() + system_states_sz;
			}

			/// Iterator to the end of valid trajectories (const)
			inline auto iterend() const
			{
				return system_states.begin() + system_states_sz;
			}

			/// Removes trajectories not satisfying predicate pred and shrinks the valid length
			template<typename Pred>
			inline void remove_if(Pred pred)
			{
				auto iter = std::remove_if(iterbeg(), iterend(), pred);
				system_states_sz = iter - iterbeg();
			}

			/// Resets to a single initial trajectory (initial bus value bus_input), clearing the good-branch flag
			inline void reset() {
				system_states_sz = 1;
				system_states[0] = std::move(SystemState(bus_input, bus_size));
				good = false;
				relative_multiplier = 1;
			}

			Branch() = default;
			/// @brief Constructs with an explicit (address, bus value).
			/// @param addr Address @param bus_sz Bus width @param bus Bus value
			Branch(size_t addr, size_t bus_sz, bus_t bus);
			/// @brief Constructs from a branch ID (address and bus value extracted automatically).
			/// @param branchid (address << bus_sz) | bus_input
			/// @param bus_sz Bus width
			Branch(size_t branchid, size_t bus_sz);
			Branch(const Branch& old_branch) = default;

			/// Lexicographic comparison by (address, input bus, trajectory set)
			inline bool operator<(const Branch& other) const {
				return std::tie(address, bus_input, system_states) <
					std::tie(other.address, other.bus_input, other.system_states);
			}

			/// Equality test
			inline bool operator==(const Branch& other) const {
				return std::tie(address, bus_input, system_states) ==
					std::tie(other.address, other.bus_input, other.system_states);
			}

			/// Total probability of this branch's trajectories (sum of squared amplitude moduli)
			double get_prob() const;
			/// Fidelity amplitude against the data tree (modulus not squared)
			complex_t get_fidelity(const memory_t& memory) const;
			/// Returns a human-readable string representation
			std::string to_string() const;
			/// Fetches bit digit from the data tree memory to the bus (FetchData)
			void run_fetchdata(memory_t& memory, size_t digit);
			/// Hadamard transform on the bus
			void run_hadamard();
			/// Merges identical trajectories (amplitudes added up)
			void try_merge();

			/// Tree state of the reference good branch trajectory (valid when this branch is a good branch)
			std::optional<element_type> get_good_branch_system() const;

			/// Common amplitude damping (same as SystemState::run_damp_common)
			void run_damp_common(double gamma);

			/// Returns the trajectory probability under Damping (corrected by the excitation count at qubit_id)
			damp_prob_type get_prob_damp(size_t qubit_id) const;

			/// Full amplitude damping: splits trajectories at qubit_id by damping step count k
			void run_damp_full(size_t qubit_id, size_t k);

			/// Removes trajectories not matching the target tree state (cleanup after output sampling)
			void remove_mismatch_state(const State::element_type& target_state);
			/// Removes all trajectories
			void remove_all_state();

			/// Clears valid trajectories (sets system_states_sz = 0)
			inline void clear_all_state() { system_states_sz = 0; }

		};

		/**
		 * @brief Aggregation of all input branches at the same address (the pruning unit of the qubit architecture).
		 *
		 * BranchGroup aggregates Branch per address: groups at good addresses only need to materialize
		 * the reference branch (XOR mirror prediction), while groups at bad addresses are evolved one by one;
		 * the group level provides probability, fidelity, and output sampling interfaces, and QRAMCircuit
		 * manages good/bad pruning at BranchGroup granularity.
		 */
		struct BranchGroup
		{
			/// Group address
			size_t address;
			/// Input-side branches |ui(input)>
			std::vector<Branch> branches_input; // |ui(input)>
			/// Evolved branches |ui>
			std::vector<Branch> branches;       // |ui>
			/// Per-branch probabilities <ui|ui>
			std::vector<double> branch_probs;   // <ui|ui>
			/// Per-state probabilities <ψi|ψi>
			std::vector<double> state_probs;    // <ψi|ψi>

			/// Whether this group is a good group (address outside the bad branch interval)
			bool is_good = false;
			/// Prediction reference for good groups (pointer to the first good group)
			BranchGroup* good_ref = nullptr;
			/// Relative Damping multiplier (relative to the reference good group)
			double relative_multiplier = 1.0;
			/* set once the good group's final states have been materialized
			from the reference branch (XOR mirror), after which the generic
			prob/fidelity paths apply */
			/// Set once the reference good branch's system states have been materialized (XOR mirror), after which the generic probability/fidelity paths apply
			bool predicted = false;

			/// @brief Constructs an empty group at address addr.
			BranchGroup(size_t addr) : address(addr)
			{}

			/// Resets all branches and flags
			void reset();
			/// Marks as a good group and binds the prediction reference
			void set_good(BranchGroup* good_ref_);
			/// Clears all branch states (branch skeleton kept)
			void set_empty_state();

			/// Fidelity amplitude against the data tree (modulus not squared)
			complex_t get_fidelity(const memory_t& memory) const;

			/// Trajectory probability under Damping
			Branch::damp_prob_type get_prob_damp(size_t qubit_id) const;

			/// Total probability of all branches in this group
			double get_prob() const;

			/// Output sampling without Damping: r is a uniform random number; returns whether sampling succeeded
			bool sample_output_no_damping(Branch::element_type& output, double& r);
			/// Output sampling with Damping: r is a uniform random number; returns whether sampling succeeded
			bool sample_output_with_damping(Branch::element_type& output, double& r);

			/// Removes branch states not matching the target tree state
			void remove_mismatch_state(const Branch::element_type& target);
		};
	} // namespace qram_qubit
} // namespace qram_simulator
