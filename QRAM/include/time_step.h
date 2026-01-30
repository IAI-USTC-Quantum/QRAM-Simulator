#pragma once

#include "basic.h"

namespace qram_simulator {

	constexpr int arch_qutrit = 0x1A1A;
	constexpr int arch_qubit = 0x1B1B;

	enum class OperationType {
		Begin,

		ControlSwap,
		HadamardData,
		CopyIn,
		CopyOut,
		SwapInternal,
		FirstCopy,
		FetchData,

		// Noise Op
		SetZero,
		Damping,
		Damp_Common,
		Damp_Full,
		BitFlip,
		PhaseFlip,
		BitPhaseFlip,
		Depolarizing,

		// Identity
		Identity_Active,
		Identity_Inactive,

		End
	};
	
	using noise_t = std::map<OperationType, double>;

	struct Operation {
		OperationType type;
		std::vector<size_t> targets;
		std::vector<double> coefficients;
		bool dagger = false;

		Operation(OperationType type_, std::vector<size_t> targets_)
			:type(type_), targets(targets_)
		{ }

		Operation(OperationType type_, std::vector<size_t> targets_, std::vector<double> coefs)
			:type(type_), targets(targets_), coefficients(coefs)
		{ }

		Operation(const Operation& oldop) = default;

		inline std::vector<size_t> get_targets() const { return targets; }
		Operation reverse();
		std::string to_string() const;
	};

	struct OperationPack {
		std::list<Operation> operations;
		std::string name;

		inline OperationPack reverse() {
			OperationPack ret;
			for (auto iter = operations.rbegin(); iter != operations.rend(); ++iter) {
				ret.operations.push_back(iter->reverse());
			}
			ret.name = name + "^";
			return ret;
		}
		inline bool empty() const { return operations.size() == 0; }
		inline void set_name(std::string s) { name = s; }
		inline void append(Operation op) { operations.push_back(op); }
		inline void append(OperationPack ops) {
			for (auto& op : ops.operations) {
				append(op);
			}
			name += "->";
			name += ops.name;
		}
		std::string to_string() const;
	};

	struct TimeSlices {
		std::vector<OperationPack> time_slices;

		inline void clear() {
			time_slices.clear();
		}
		inline void append(const OperationPack &op) {
			time_slices.emplace_back(op);
		}
		inline void append(const TimeSlices &ts) {
			for (auto& tslice : ts.time_slices) {
				append(tslice);
			}
		}
		inline TimeSlices reverse() {
			TimeSlices ret;
			for (auto iter = time_slices.rbegin(); iter != time_slices.rend(); ++iter) {
				ret.time_slices.push_back(iter->reverse());
			}
			return ret;
		}

		std::string to_string() const;
	};

	struct ContinuousRange
	{
		std::vector<int> range_bound;

		// [l1,r1],[l2,r2],[l3,r3],[l4,r4] new=[l5,r5]
		// (in x, out y)
		// (in x, in y)
		// (out x, out y)
		// (out x, in y)
		ContinuousRange() = default;
		ContinuousRange(int l, int r);

		void clear();
		void merge(int l, int r);		
		bool accept(int t) const;
		operator std::string();
	};

	struct TimeStep
	{
		size_t addr_size;
		size_t data_size;

		ContinuousRange cr;
		TimeSlices time_slices_noise_free;

		TimeStep(size_t addr_sz, size_t data_sz);
		size_t full_step() const;
		size_t out(size_t in_step) const;
		size_t last_step() const;

		int acopy(size_t step) const;
		int acopy_out(size_t step) const;
		int dcopy(size_t step) const;
		int dcopy_out(size_t step) const;
		size_t route_begin(size_t layer) const;
		size_t route_wait_begin(size_t layer) const;
		bool route_odd(size_t step, size_t layer) const;
		bool route_even(size_t step, size_t layer) const;
		bool route(size_t step, size_t layer) const;
		int memory_copy(size_t step) const;
		int internal_swap(size_t time_step) const;
		int internal_swap_out(size_t time_step) const;

		OperationPack generate_step(size_t step) const;
		size_t layer_entangle_max(size_t time_step) const;

		bool is_bad_branch(size_t addr) const;

		std::pair<size_t, size_t> get_bad_range_qubit(size_t bad_qubit) const;
		std::pair<size_t, size_t> get_bad_range_qutrit(size_t bad_qubit) const;

		void fill_bad_range(size_t qubit, int arch_type);
		void noise_one_step(OperationPack& pack, size_t max_entangle_layer,
			const std::map<OperationType, double>& noises, int arch_type);

		// without inserting to the OperationPack
		void noise_one_step(size_t max_entangle_layer,
			const std::map<OperationType, double>& noises, int arch_type);

		void init_noise_free();
		void append_noise_range_only(const std::map<OperationType, double>& noises, int arch_type);
		TimeSlices append_noise(const std::map<OperationType, double>& noises, int arch_type);
		TimeSlices generate(const std::map<OperationType, double>& noises, int arch_type);

		ptrdiff_t _get_multiplier_impl_qubit(
			size_t step, size_t addr) const;

		ptrdiff_t _get_multiplier_impl_qutrit(
			size_t step, size_t branch_id, const memory_t& mem) const;

		ptrdiff_t _get_multiplier_impl_qutrit(
			size_t step, size_t addr, size_t data, const memory_t& mem) const;
		
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

		template<typename BranchType>
		void get_multiplier_qutrit(
			double gamma,
			size_t step,
			std::vector<BranchType>& branches,
			size_t first_good_branch,
			const std::vector<size_t>& good_branch_ids,
			const memory_t& memory) const
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

		inline void print() const
		{
			for (size_t step = 1; step < full_step(); ++step)
			{
				auto ops = generate_step(step);
				fmt::print("step={}\nentangle_max={}\n{}\n", step, layer_entangle_max(step), ops.to_string());
			}
		}

	};

	std::vector<bool> calc_pos(int pos, int layer);
	std::vector<size_t> get_nodes_in_layer(int layer);
	std::string pos2str(int pos, int layer);
	std::string type2str(OperationType type);

	constexpr const char* bool2char(bool x) 
	{
		return x ? "1" : "0";
	}

	constexpr const char* bool2str(bool x) 
	{
		return x ? "True" : "False";
	}

	constexpr int bool2int(bool x) 
	{
		return x ? 1 : 0;
	}

	constexpr const char* bool2char_pmbasis(bool x) 
	{
		return x ? "+" : "-";
	}

	constexpr char addr2str(int addr) 
	{
		if (addr == W) { return 'W'; }
		if (addr == L) { return 'L'; }
		if (addr == R) { return 'R'; }
		else { return '?'; }
	}

	constexpr const char* arch2str(int arch) 
	{
		// if (arch == arch_qubit) return "qubit";
		if (arch == arch_qutrit) return "qutrit";
		return "unknown_arch";
	}

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
