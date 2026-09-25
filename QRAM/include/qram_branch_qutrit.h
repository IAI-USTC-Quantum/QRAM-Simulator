#pragma once

#include "time_step.h"

namespace qram_simulator {
	namespace qram_qutrit {
		/**
		 * @brief A single qutrit tree node: address level + data level.
		 *
		 * The address level addr ∈ {W=-1, L=0, R=1} (W is the bus/waiting level),
		 * the data level data ∈ {0, 1}. Inline gate operations are implemented as level transitions: rotation
		 * gates A1/A2 (three-level unitaries carrying the w phase), internal swap, data flip, etc.
		 */
		struct QRAMNode
		{
			/// Address level: W / L / R
			int addr = 0;
			/// Data level: 0 / 1
			int data = 0;

			/// Converts to the readable string "(A,D)", e.g. "(W,0)"
			operator std::string() const {
				return fmt::format("({},{})", addr2str(addr), data);
			}

			QRAMNode() = default;
			/// @brief Constructs from levels. @param a Address level @param d Data level
			QRAMNode(int a, int d) : addr(a), data(d) {}

			/// Flips the data level 0↔1
			inline void data_flip() { data ^= 1; }
			/// Flips the address level L↔R (W unchanged)
			inline void addr_flip()
			{
				switch (addr) {
				case L:
					addr = R;
					return;
				case R:
					addr = L;
					return;
				default:
					return;
				}
			}
			/// Sets the data level to 0
			inline void data_setzero() { data = 0; }
			/// Sets the data level to 1
			inline void data_setone() { data = 1; }
			/// Sets the data level to the given value
			inline void data_set(bool value) { data = bool2int(value); }
			/// Classically conditioned data flip: flips when classical_data is true
			inline void data_control_flip(bool classical_data)
			{
				if (classical_data) data_flip();
			}
			/// Address-data level swap within a node (transfer between W and data)
			inline void internal_swap()
			{
				if (addr == W)
				{
					addr = data;
					data = 0;
				}
				else {
					if (data == 0) {
						data = addr;
						addr = W;
					}
				}
			}

			/*
			A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
				 [ 0 0 1 ]	     [ 0 w 0  ]
				 [ 1 0 0 ]	     [ 0 0 w^2]
			A1   = L->W, W->R, R->L
			*/
			/// A1 rotation gate (L→W→R→L). Returns whether a distinguishable state with data==0 is produced
			inline bool rotate_A1()
			{
				if (addr == L) {
					addr = W;
					return data == 0;
				}
				if (addr == W) {
					addr = R;
					return false;
				}
				if (addr == R) {
					addr = L;
				}
				return false;
			}

			/// A2 rotation gate (R→W, L→R, W→L). Returns whether a distinguishable state with data==0 is produced
			inline bool rotate_A2()
			{
				if (addr == R) {
					addr = W;
					return data == 0;
				}
				if (addr == L) addr = R;
				if (addr == W) addr = L;
				return false;
			}

			/// Whether in the (W, 0) state
			inline bool check_0() const
			{
				if (addr == W && data == 0) return true;
				else return false;
			}
			/// Lexicographic comparison by (addr, data)
			inline bool operator<(const QRAMNode& rhs) const
			{
				return std::tie(addr, data) < std::tie(rhs.addr, rhs.data);
			}

			/// Level equality test
			inline bool operator==(const QRAMNode& rhs) const
			{
				return std::tie(addr, data) == std::tie(rhs.addr, rhs.data);
			}
		};

		/**
		 * @brief A sparse qutrit tree state represented by a map of non-zero nodes.
		 *
		 * Only nodes not in the (W,0) ground state are recorded (nz_elements, ordered by location);
		 * all other nodes are implicitly (W,0). Gate operations are implemented as insertions, deletions, and updates on the map.
		 */
		struct QRAMState
		{
			using value_type = QRAMNode;

			// sparse storage and keep sorted
			/// Map of non-ground-state nodes: node location → node levels (kept sorted)
			using element_type = std::map<size_t, QRAMNode>;
			using iterator_t = typename element_type::iterator;
			using const_iterator_t = typename element_type::const_iterator;

			element_type nz_elements;

			QRAMState() = default;

			/// Compares two sparse tree states (map lexicographic order)
			bool operator<(const QRAMState& rhs) const;
			/// Sparse tree state equality test
			bool operator==(const QRAMState& rhs) const;

			/// Cleans up zero-amplitude elements (a compatibility interface for qutrit tree states)
			void clear_zero_elements();

			/// @brief Encodes a node location from a layer number and an in-layer position (complete binary tree level-order numbering).
			constexpr static size_t get_nnz_location(size_t layer, size_t pos)
			{
				size_t begin = pow2(layer) - 1;
				return (begin + pos);
			}

			/// Location of the left child of node node_location
			constexpr static size_t left_of(size_t node_location)
			{
				return 2 * node_location + 1;
			}
			/// Location of the right child of node node_location
			constexpr static size_t right_of(size_t node_location)
			{
				return 2 * node_location + 2;
			}
			/// Location of the parent of node node_location
			constexpr static size_t parent_of(size_t node_location)
			{
				return (node_location - 1) / 2;
			}

			/* Time: O(log n) */
			/// Gets the iterator at location nnz_location (O(log n))
			iterator_t iterof(size_t nnz_location);
			/// Gets the constant iterator at location nnz_location (O(log n))
			const_iterator_t iterof(size_t nnz_location) const;

			/* Time: O(1) */
			/// End iterator (O(1))
			iterator_t iterend();
			/// Constant end iterator (O(1))
			const_iterator_t iterend() const;

			/// Bus input: writes bus bit digit into the node's data level
			void busin(bus_t& bus, size_t digit);
			/// Bus output: reads the node's data level out to bus bit digit
			void busout(bus_t& bus, size_t digit);
			/// Address copy: writes the address level into the data level in direction a (FirstCopy)
			void acopy(bool a);

			/// Unconditional within-node swap (underlying implementation of internal_swap)
			void _unconditional_internal_swap(size_t node_location);
			/// Conditional within-node swap (executed when the data level is 0)
			void _impl_conditional_internal_swap(size_t node_location);
			/// Internal swap of all nodes at layer layerid
			void internal_swap_layer(size_t layerid);
			/// Moves the node at iterator iter to location target
			void _impl_iter_swap(iterator_t iter, size_t target);

			/// Controlled swap (CSWAP) at node node_location
			void cswap(size_t node_location);
			/// Controlled swap of all nodes at layer layerid
			void cswap_layer(size_t layerid);
			/// Flips the data level of the node at location qubit_id (noise)
			void flip(size_t qubit_id);

			/*
			A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
				 [ 0 0 1 ]	     [ 0 w 0  ]
				 [ 1 0 0 ]	     [ 0 0 w^2]
			A1   = L->W, W->R, R->L
			A1^2 = L->R, W->L, R->W
			*/
			/// Applies the A1 rotation gate at location qubit_id (phase w)
			void rotate_A1(size_t qubit_id);
			/// Applies the A2 rotation gate at location qubit_id (phase w^2)
			void rotate_A2(size_t qubit_id);

			/// Checks whether the node at location qubit_id is not in the ground state (returns a level tag)
			int state_of(size_t qubit_id) const;
			/// Resets the node at location qubit_id back to the ground state (W,0) (noise SetZero)
			void set_zero(size_t qubit_id);

			/// Returns a human-readable string representation
			std::string to_string() const;

		};

		/**
		 * @brief Qutrit system state with a data bus and an amplitude (a trajectory unit).
		 *
		 * Composed of the sparse tree state QRAMState, the data bus data_bus,
		 * and a complex amplitude; the run_* series executes gate / noise operations, evolving one trajectory.
		 */
		struct SubBranch
		{
			/// Sparse qutrit tree state
			QRAMState state;
			/// Data bus
			bus_t data_bus;
			/// Bus width
			size_t data_size;
			/// Trajectory amplitude (noise damping acts on this coefficient)
			std::complex<double> amplitude = 1.0;

			using element_type = typename QRAMState::element_type;

			SubBranch() = default;
			/// @brief Constructs from an initial bus value. @param bus_ Bus value @param bus_sz Bus width
			SubBranch(memory_entry_t bus_, size_t bus_sz) : data_size(bus_sz), data_bus(bus_)
			{ }

			/// Lexicographic comparison by (bus value, tree state)
			inline bool operator<(const SubBranch& rhs) const noexcept
			{
				return std::tie(data_bus, state) < std::tie(rhs.data_bus, rhs.state);
			}

			/// Equality test
			inline bool operator==(const SubBranch& rhs) const noexcept
			{
				return std::tie(data_bus, state) == std::tie(rhs.data_bus, rhs.state);
			}

			/// Address copy: writes into the data level in direction a
			void run_acopy(bool a);
			/// Plain swap at node node
			void run_swap(size_t node);
			/// Controlled swap at node node
			void run_cswap(size_t node);
			/// Data level flip at location qubit_id (noise)
			void run_bitflip(size_t qubit_id);
			/// A1 rotation gate at location qubit_id
			void run_A1(size_t qubit_id);
			/// A1^2 rotation gate at location qubit_id
			void run_A1_2(size_t qubit_id);
			/// A2 rotation gate at location qubit_id
			void run_A2(size_t qubit_id);
			/// A2^2 rotation gate at location qubit_id
			void run_A2_2(size_t qubit_id);
			/// Phase flip at location qubit_id (noise, amplitude negated)
			void run_phaseflip(size_t qubit_id, double depol_id);
			/// Combined bit-phase flip at location qubit_id (noise)
			void run_bitphaseflip(size_t qubit_id);
			/// Depolarizing at location qubit_id (noise)
			void run_depolarizing(size_t qubit_id, double depol_id);
			/// Reset to the ground state at location qubit_id (noise)
			void set_zero(size_t qubit_id);
			/// Bus input of bit digit
			void run_busin(size_t digit);
			/// Bus output of bit digit
			void run_busout(size_t digit);
		};

		/**
		 * @brief Trajectory container of a single (address, bus_input) input branch (qutrit architecture).
		 *
		 * Holds a set of SubBranch trajectories, providing fidelity against the data tree, output
		 * sampling, and Damping multiplier computation; get_multiplier forwards to the
		 * qutrit implementation of TimeStep.
		 */
		struct Branch {
			/// Architecture identifier: qutrit
			constexpr static int arch_type = arch_qutrit;
			/// Number of Damping-related operations (2 in the qutrit architecture)
			constexpr static int damp_op_num = 2;

			using state_type = SubBranch;
			using element_type = typename state_type::element_type;
			/// Damping probability array type (two elements in the qutrit architecture)
			using damp_prob_type = std::array<double, 2>;

			/// Branch address
			size_t address = 0;
			/// Bus width
			size_t bus_size = 1;
			/// Input bus value
			memory_entry_t bus_input = 0;
			/// Relative Damping multiplier (relative to the reference good branch)
			double relative_multiplier = 1;
			/// Prediction reference for good branches (pointer to the first good branch)
			Branch* good_ref = nullptr;
			/// Input probability of this branch
			double branch_prob = 0.0;
		private:
			bool good = false;
			std::vector<SubBranch> system_states;
		public:
			Branch() = default;
			/// @brief Constructs with an explicit (address, bus value).
			Branch(size_t address_, size_t bus_sz_, memory_entry_t bus_input_)
				: address(address_), bus_size(bus_sz_), bus_input(bus_input_)
			{ }
			Branch(const Branch& old_branch) = default;
			/// @brief Constructs from a branch ID (address and bus value extracted automatically).
			/// @param branchid (address << bus_sz) | bus_input
			/// @param bus_sz Bus width
			Branch(size_t branchid, size_t bus_sz)
			{
				bus_size = bus_sz;
				address = branchid >> bus_sz;
				bus_input = branchid - (address << bus_sz);
			}

			/// Branch ID: (address << bus_size) + bus_input
			inline size_t get_branchid() const
			{
				return (address << bus_size) + bus_input;
			}

			/// Iterator to the beginning of trajectories (const)
			auto iterbeg() const { return system_states.begin(); }
			/// Iterator to the end of trajectories (const)
			auto iterend() const { return system_states.end(); }
			/// Iterator to the beginning of trajectories
			auto iterbeg() { return system_states.begin(); }
			/// Iterator to the end of trajectories
			auto iterend() { return system_states.end(); }
			/// Number of trajectories
			auto system_size() const { return system_states.size(); }

			/// Removes trajectories not satisfying predicate pred (physical deletion)
			template<typename Pred>
			inline void remove_if(Pred pred)
			{
				auto iter = std::remove_if(system_states.begin(), system_states.end(), pred);
				system_states.erase(iter, system_states.end());
			}

			/// Resets to a single initial trajectory (initial bus value bus_input), clearing the good-branch flag
			inline void reset()
			{
				system_states.clear();
				system_states.push_back(SubBranch(bus_input, bus_size));
				good = false;
				relative_multiplier = 1;
			}

			/// Marks this branch as a good branch and binds the prediction reference
			void set_good(Branch* first_good_ptr);
			/// Whether this branch is a good branch
			inline bool is_good() const { return good; }

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

			/// Total probability of this branch's trajectories
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

			/// Common amplitude damping
			void run_damp_common(double gamma);

			/// Trajectory probability under Damping
			damp_prob_type get_prob_damp(size_t qubit_id) const;

			/// Full amplitude damping: splits trajectories at qubit_id by damping step count k
			void run_damp_full(size_t qubit_id, size_t k);

			/// @brief Computes the relative Damping multiplier for good branches (forwards to TimeStep::get_multiplier_qutrit).
			/// @param gamma Damping decay rate
			/// @param time_step The scheduler
			/// @param step Time slice
			/// @param branches Branch collection
			/// @param first_good_branch Index of the reference good branch
			/// @param good_branch_ids Indices of the remaining good branches
			/// @param memory Data tree
			inline static void get_multiplier(
				double gamma,
				const TimeStep& time_step,
				size_t step,
				std::vector<Branch>& branches,
				size_t first_good_branch,
				const std::vector<size_t>& good_branch_ids,
				/*std::vector<double>& multipliers,*/
				const memory_t& memory
			)
			{
				time_step.get_multiplier_qutrit(gamma, step, branches, first_good_branch, good_branch_ids,/* multipliers,*/ memory);
			}

			/// Removes trajectories not matching the target tree state
			void remove_mismatch_state(const QRAMState::element_type& target_state);
			/// Removes all trajectories
			void remove_all_state();

			/// Clears all trajectories
			inline void clear_all_state() { system_states.clear(); }
		};
	} // namespace qram_qutrit
} // namespace qram_simulator

/// fmt formatter specialization for QRAMNode: outputs the "(A,D)" form
template <> struct fmt::formatter<qram_simulator::qram_qutrit::QRAMNode> {
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}
	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(const qram_simulator::qram_qutrit::QRAMNode& p,	FormatContext& ctx) const -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "{}", std::string(p));
	}
};
