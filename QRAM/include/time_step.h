#pragma once

#include "basic.h"

namespace qram_simulator {

	/// qutrit architecture identifier constant (value of the arch_type parameter of TimeStep::generate)
	constexpr int arch_qutrit = 0x1A1A;
	/// qubit architecture identifier constant (value of the arch_type parameter of TimeStep::generate)
	constexpr int arch_qubit = 0x1B1B;

	/**
	 * @brief Enumeration of QRAM operation types.
	 *
	 * Serves both as the key type of the noise model (the key of noise_t) and as the type tag of Operation.
	 * Noise members are occurrence probabilities that must lie in [0, 1]; Damping additionally
	 * requires gamma < 1 (gamma = 1 would extinguish all excitations in a single step, breaking sampling and normalization).
	 */
	enum class OperationType {
		Begin,             ///< Enum sentinel (unused)
		ControlSwap,       ///< Controlled swap (routing)
		HadamardData,      ///< Hadamard on the data bus
		CopyIn,            ///< Copy in (busin)
		CopyOut,           ///< Copy out (busout)
		SwapInternal,      ///< Internal swap within a node
		FirstCopy,         ///< First-layer copy (acopy)
		FetchData,         ///< Fetch data from the data tree (fetchdata)
		// Noise Op
		SetZero,           ///< Noise: set to zero
		Damping,           ///< Noise: amplitude damping (gamma < 1)
		Damp_Common,       ///< Noise: common amplitude damping
		Damp_Full,         ///< Noise: full amplitude damping
		BitFlip,           ///< Noise: bit flip
		PhaseFlip,         ///< Noise: phase flip
		BitPhaseFlip,      ///< Noise: combined bit-phase flip
		Depolarizing,      ///< Noise: depolarizing
		// Identity
		Identity_Active,   ///< Identity operation (active qubit)
		Identity_Inactive, ///< Identity operation (inactive qubit)
		End                ///< Enum sentinel (unused)
	};

	/// Noise model: {operation type: occurrence probability}
	using noise_t = std::map<OperationType, double>;

	/**
	 * @brief A single quantum operation.
	 *
	 * Describes one gate / noise event: the type, the target qubit (or node)
	 * indices, and optional noise coefficients (e.g. gamma of Damping).
	 */
	struct Operation {
		/// Operation type
		OperationType type;
		/// Target qubit / node indices
		std::vector<size_t> targets;
		/// Noise coefficients (optional)
		std::vector<double> coefficients;
		/// Whether this is the conjugate-transpose (inverse) operation
		bool dagger = false;

		Operation(OperationType type_, std::vector<size_t> targets_)
			:type(type_), targets(targets_)
		{ }

		Operation(OperationType type_, std::vector<size_t> targets_, std::vector<double> coefs)
			:type(type_), targets(targets_), coefficients(coefs)
		{ }

		Operation(const Operation& oldop) = default;

		/// Returns the list of target indices
		inline std::vector<size_t> get_targets() const { return targets; }
		/// Returns the inverse of this operation
		Operation reverse();
		/// Returns a human-readable string representation
		std::string to_string() const;
	};

	/**
	 * @brief A set of operations executed concurrently within one time slice.
	 *
	 * Mutually non-adjacent nodes of the QRAM tree can be operated on in parallel
	 * within the same time slice; OperationPack bundles them into one scheduling unit identified by name.
	 */
	struct OperationPack {
		/// List of operations within this time slice
		std::list<Operation> operations;
		/// Operation pack name (for schedule debugging)
		std::string name;

		/// Returns the inverse operation pack: reverses the order and inverts each operation
		inline OperationPack reverse() {
			OperationPack ret;
			for (auto iter = operations.rbegin(); iter != operations.rend(); ++iter) {
				ret.operations.push_back(iter->reverse());
			}
			ret.name = name + "^";
			return ret;
		}
		/// Whether this pack is empty
		inline bool empty() const { return operations.size() == 0; }
		/// Sets the pack name
		inline void set_name(std::string s) { name = s; }
		/// Appends a single operation
		inline void append(Operation op) { operations.push_back(op); }
		/// @brief Appends all operations of another pack and concatenates the names.
		/// @param ops The pack being merged in
		inline void append(OperationPack ops) {
			for (auto& op : ops.operations) {
				append(op);
			}
			name += "->";
			name += ops.name;
		}
		/// Returns a human-readable string representation
		std::string to_string() const;
	};

	/**
	 * @brief Complete QRAM loading schedule: a sequence of OperationPacks ordered by time slice.
	 */
	struct TimeSlices {
		/// List of time slices (the i-th element is the operation pack of the i-th time slice)
		std::vector<OperationPack> time_slices;

		/// Clears the schedule
		inline void clear() {
			time_slices.clear();
		}
		/// Appends a single time slice
		inline void append(const OperationPack &op) {
			time_slices.emplace_back(op);
		}
		/// Appends all time slices of another schedule
		inline void append(const TimeSlices &ts) {
			for (auto& tslice : ts.time_slices) {
				append(tslice);
			}
		}
		/// Returns the inverse schedule: reversed as a whole with each pack inverted
		inline TimeSlices reverse() {
			TimeSlices ret;
			for (auto iter = time_slices.rbegin(); iter != time_slices.rend(); ++iter) {
				ret.time_slices.push_back(iter->reverse());
			}
			return ret;
		}

		/// Returns a human-readable string representation
		std::string to_string() const;
	};

	/**
	 * @brief A set of continuous intervals (merged representation of bad branch ranges).
	 *
	 * Represents an address set as an ordered collection of disjoint intervals [l1,r1],[l2,r2],...;
	 * merge incorporates a new interval [l,r] according to 4 intersection cases, and accept checks whether
	 * address t falls inside the set. Used for TimeStep's bad range computation.
	 */
	struct ContinuousRange
	{
		/// Sequence of interval endpoints: [l1,r1,l2,r2,...]
		std::vector<int> range_bound;

		// [l1,r1],[l2,r2],[l3,r3],[l4,r4] new=[l5,r5]
		// (in x, out y)
		// (in x, in y)
		// (out x, out y)
		// (out x, in y)
		ContinuousRange() = default;
		/// Constructs a single interval [l, r]
		ContinuousRange(int l, int r);

		/// Clears all intervals
		void clear();
		/// Merges in a new interval [l, r] (staying ordered and disjoint)
		void merge(int l, int r);
		/// Checks whether t falls within any interval
		bool accept(int t) const;
		/// Returns a human-readable string representation
		operator std::string();
	};

	/**
	 * @brief Timing and noise scheduler for the QRAM loading circuit.
	 *
	 * Derives the routing, copy, and swap operations of each time slice from the address / data width
	 * (generate_step), and can insert noise operations after each time slice according to a noise model,
	 * producing the complete TimeSlices schedule (generate). QRAMCircuit holds
	 * a TimeStep instance internally. Also provides bad branch range (bad range) derivation: given some
	 * faulty qubit / node, it derives the address range that would load wrong data.
	 */
	struct TimeStep
	{
		/// Address width
		size_t addr_size;
		/// Data width
		size_t data_size;

		/// Bad branch interval set (filled by fill_bad_range)
		ContinuousRange cr;
		/// Cached noise-free schedule (generated by init_noise_free)
		TimeSlices time_slices_noise_free;

		/**
		 * @brief Constructs the scheduler.
		 * @param addr_sz Address width
		 * @param data_sz Data width
		 */
		TimeStep(size_t addr_sz, size_t data_sz);
		/// Total number of time slices of the full loading procedure
		size_t full_step() const;
		/// Returns the output (copy-out) time slice index corresponding to step in_step
		size_t out(size_t in_step) const;
		/// Index of the last time slice
		size_t last_step() const;

		/// Target layer (direction) of the address copy at step step; returns -1 if there is no operation
		int acopy(size_t step) const;
		/// Output time slice of the address copy at step step; returns -1 if none
		int acopy_out(size_t step) const;
		/// Target bit of the data copy at step step; returns -1 if there is no operation
		int dcopy(size_t step) const;
		/// Output time slice of the data copy at step step; returns -1 if none
		int dcopy_out(size_t step) const;
		/// Starting time slice of the routing phase of layer layer
		size_t route_begin(size_t layer) const;
		/// Starting time slice of the routing-wait phase of layer layer
		size_t route_wait_begin(size_t layer) const;
		/// Whether step step routes through the odd (right) subtree at layer layer
		bool route_odd(size_t step, size_t layer) const;
		/// Whether step step routes through the even (left) subtree at layer layer
		bool route_even(size_t step, size_t layer) const;
		/// Routing decision of step step at layer layer (odd/even dispatch)
		bool route(size_t step, size_t layer) const;
		/// Memory copy bit of step step; returns -1 if there is no operation
		int memory_copy(size_t step) const;
		/// Internal swap target of a node at step time_step; returns -1 if there is no operation
		int internal_swap(size_t time_step) const;
		/// Output time slice of the internal swap at step time_step; returns -1 if none
		int internal_swap_out(size_t time_step) const;

		/// Generates the operation pack of time slice step (noise excluded)
		OperationPack generate_step(size_t step) const;
		/// Maximum number of layers that can be entangled at step time_step (basis for the noise insertion range)
		size_t layer_entangle_max(size_t time_step) const;

		/// Checks whether address addr falls within the bad branch interval
		bool is_bad_branch(size_t addr) const;

		/// qubit architecture: derives the bad branch address range [lo, hi] from faulty qubit bad_qubit
		std::pair<size_t, size_t> get_bad_range_qubit(size_t bad_qubit) const;
		/// qutrit architecture: derives the bad branch address range [lo, hi] from faulty qubit bad_qubit
		std::pair<size_t, size_t> get_bad_range_qutrit(size_t bad_qubit) const;

		/// @brief Fills the bad branch interval set cr.
		/// @param qubit The faulty qubit / node
		/// @param arch_type Architecture constant (arch_qubit / arch_qutrit)
		void fill_bad_range(size_t qubit, int arch_type);
		/// @brief Appends the noise operations following one time slice to pack.
		/// @param pack Target operation pack
		/// @param max_entangle_layer Maximum number of layers that can be entangled in that time slice
		/// @param noises Noise model
		/// @param arch_type Architecture constant
		void noise_one_step(OperationPack& pack, size_t max_entangle_layer,
			const std::map<OperationType, double>& noises, int arch_type);

		/// @brief Generates the noise operations of one time slice (without inserting them into any OperationPack).
		/// @param max_entangle_layer Maximum number of layers that can be entangled
		/// @param noises Noise model
		/// @param arch_type Architecture constant
		void noise_one_step(size_t max_entangle_layer,
			const std::map<OperationType, double>& noises, int arch_type);

		/// Generates the noise-free schedule and caches it in time_slices_noise_free
		void init_noise_free();
		/// Appends noise intervals only (does not generate the noisy operation sequence)
		void append_noise_range_only(const std::map<OperationType, double>& noises, int arch_type);
		/// Inserts noise into the cached noise-free schedule and returns the noisy schedule
		TimeSlices append_noise(const std::map<OperationType, double>& noises, int arch_type);
		/**
		 * @brief Generates the complete noisy schedule.
		 * @param noises Noise model (may be empty, i.e. a noise-free schedule)
		 * @param arch_type Architecture constant (arch_qubit / arch_qutrit)
		 * @return The noisy TimeSlices schedule
		 */
		TimeSlices generate(const std::map<OperationType, double>& noises, int arch_type);

		/// qubit architecture: implementation of the damping multiplier exponent for address addr at step step
		ptrdiff_t _get_multiplier_impl_qubit(
			size_t step, size_t addr) const;

		/// qutrit architecture: implementation computing the damping multiplier exponent from branchid
		ptrdiff_t _get_multiplier_impl_qutrit(
			size_t step, size_t branch_id, const memory_t& mem) const;

		/// qutrit architecture: implementation computing the damping multiplier exponent from (addr, data)
		ptrdiff_t _get_multiplier_impl_qutrit(
			size_t step, size_t addr, size_t data, const memory_t& mem) const;

		/**
		 * @brief qubit architecture: computes the relative Damping multiplier for good branches.
		 *
		 * Using the first good branch as the reference, sets
		 * branch.relative_multiplier = (1-gamma)^(exponent difference) according to each good branch's address damping exponent difference.
		 * @param gamma Damping decay rate
		 * @param step Time slice
		 * @param branches Branch collection
		 * @param first_good_branch Index of the reference good branch
		 * @param good_branch_ids Indices of the remaining good branches
		 */
		template<typename BranchType>
		inline void get_multiplier_qubit(
			double gamma,
			size_t step,
			std::vector<BranchType>& branches,
			size_t first_good_branch,
			const std::vector<size_t>& good_branch_ids
		) const
		{
			ptrdiff_t addr_ref_count, addr_count;
			addr_ref_count = _get_multiplier_impl_qubit(step,
				branches[first_good_branch].address);

			for (size_t i = 0; i < good_branch_ids.size(); ++i)
			{
				size_t id = good_branch_ids[i];
				auto& branch = branches[id];

				addr_count = _get_multiplier_impl_qubit(step,
					branches[id].address);
				branch.relative_multiplier = std::pow(1 - gamma, addr_count - addr_ref_count);
			}
		}

		/**
		 * @brief qutrit architecture: computes the relative Damping multiplier for good branches.
		 *
		 * Semantics identical to get_multiplier_qubit, with the damping multiplier derived
		 * from branchid and the contents of the data tree.
		 * @param gamma Damping decay rate
		 * @param step Time slice
		 * @param branches Branch collection
		 * @param first_good_branch Index of the reference good branch
		 * @param good_branch_ids Indices of the remaining good branches
		 * @param memory Data tree
		 */
		template<typename BranchType>
		void get_multiplier_qutrit(
			double gamma,
			size_t step,
			std::vector<BranchType>& branches,
			size_t first_good_branch,
			const std::vector<size_t>& good_branch_ids,
			const memory_t& memory
		) const
		{
			ptrdiff_t addr_ref_count, addr_count;
			addr_ref_count = _get_multiplier_impl_qutrit(step,
				branches[first_good_branch].get_branchid(),
				memory);

			//double l = std::sqrt(1 - gamma);
			double l = 1 - gamma;
			for (size_t i = 0; i < good_branch_ids.size(); ++i)
			{
				size_t id = good_branch_ids[i];
				auto& branch = branches[id];

				addr_count = _get_multiplier_impl_qutrit(step,
					branches[good_branch_ids[i]].get_branchid(), memory);
				branch.relative_multiplier = std::pow(l, addr_count - addr_ref_count);
			}
		}

		/// Prints the schedule details of all time slices to stdout (fmt output)
		inline void print() const
		{
			for (size_t step = 1; step < full_step(); ++step)
			{
				auto ops = generate_step(step);
				fmt::print("step={}\nentangle_max={}\n{}\n", step, layer_entangle_max(step), ops.to_string());
			}
		}

	};

	/// Returns the left/right routing sequence of the pos-th position at layer layer (left/right subtree choices)
	std::vector<bool> calc_pos(int pos, int layer);
	/// Returns all node indices at layer layer
	std::vector<size_t> get_nodes_in_layer(int layer);
	/// Returns the binary routing string of the pos-th position at layer layer
	std::string pos2str(int pos, int layer);
	/// Returns the abbreviated name of an operation type
	std::string type2str(OperationType type);

	/// Converts a bool to "1"/"0"
	constexpr const char* bool2char(bool x)
	{
		return x ? "1" : "0";
	}

	/// Converts a bool to "True"/"False"
	constexpr const char* bool2str(bool x)
	{
		return x ? "True" : "False";
	}

	/// Converts a bool to 0/1
	constexpr int bool2int(bool x)
	{
		return x ? 1 : 0;
	}

	/// Converts a bool to the Pauli basis symbol "+"/"-"
	constexpr const char* bool2char_pmbasis(bool x)
	{
		return x ? "+" : "-";
	}

	/// Converts a qutrit address level to a character: W→'W', L→'L', R→'R', otherwise→'?'
	constexpr char addr2str(int addr)
	{
		if (addr == W) { return 'W'; }
		if (addr == L) { return 'L'; }
		if (addr == R) { return 'R'; }
		else { return '?'; }
	}

	/// Converts an architecture constant to a name: "qutrit" / "qubit" / "unknown_arch"
	constexpr const char* arch2str(int arch)
	{
		if (arch == arch_qutrit) return "qutrit";
		if (arch == arch_qubit) return "qubit";
		return "unknown_arch";
	}

	/// Converts a noise model to a readable string, e.g. "[BitFlip=0.01,Damping=0.001]"
	inline std::string noise2str(const noise_t& noises)
	{
		std::string ret = "[";
		for (auto& noise : noises)
		{
			fmt::format_to(std::back_inserter(ret),
				"{}={},", type2str(noise.first), noise.second
			);
		}
		if (noises.size() > 0)
			ret.back() = ']';
		else
			ret += "]";
		return ret;
	}

}
