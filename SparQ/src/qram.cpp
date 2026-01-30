#include "qram.h"
#include "quantum_interfere_basic.h"
#include "dark_magic.h"
#include "system_operations.h"

namespace qram_simulator
{
	std::string QRAMLoad::version;

	void QRAMLoad::noise_free_impl(std::vector<System>& state) const
	{
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionSatisfied(s))
			{
				//Debug_CheckOverflow(register_addr);
				size_t addr = s.GetAs(register_addr, uint64_t) % pow2(qram->address_size);
				//Debug_CheckOverflow(register_data);
				s.get(register_data).value ^= qram->memory[addr];
				//Debug_CheckOverflow(register_data);
			}
		}
	}

	void QRAMLoad::_set_branches(qram_qutrit::QRAMCircuit* qram_,
		const std::vector<System>& state,
		std::vector<std::pair<size_t, size_t>> &groups) const
	{
		profiler _("_set_branches");

		if (state.size() == 0)
			return;

		auto data_sz = qram_->data_size;
		auto& branches = qram_->get_branches();
		auto& branch_probs = qram_->get_branch_probs();
		branches.clear();
		branch_probs.clear();

		size_t current_size = state.size();
		size_t iter_l = 0;
		size_t iter_r = 1;

		while (true)
		{
			if (iter_r == current_size)
			{
				_set_branches_impl(qram_, state, branches, branch_probs, iter_l, iter_r, groups);
				break;
			}
			if (!compare_equal2(state[iter_l], state[iter_r],
				register_addr, register_data))
			{
				_set_branches_impl(qram_, state, branches, branch_probs, iter_l, iter_r, groups);
				iter_l = iter_r;
				iter_r = iter_l + 1;
			}
			else
			{
				iter_r++;
			}
		}
	}

	void QRAMLoad::_set_branches_impl(qram_qutrit::QRAMCircuit* qram_,
		const std::vector<System>&state,
		decltype(qram_->get_branches()) branches,
		decltype(qram_->get_branch_probs()) branch_probs,
		size_t iter_l, size_t iter_r,
		std::vector<std::pair<size_t, size_t>>& groups) const
	{
		if (iter_l == iter_r)
			return;

		size_t addr = state[iter_l].GetAs(register_addr, size_t);
		size_t data = state[iter_l].GetAs(register_data, size_t);

		double sum_prob = 0;
		for (size_t i = iter_l; i < iter_r; ++i)
		{
			if (ConditionSatisfied(state[i]))
				sum_prob += abs_sqr(state[i].amplitude);
		}

		if (sum_prob > 0)
		{
			groups.emplace_back(iter_l, iter_r);
			branches.emplace_back(addr, qram_->data_size, data);
			branch_probs.push_back(sum_prob);
		}
	}

	void QRAMLoad::_reconstruct(qram_qutrit::QRAMCircuit* qram_,
		std::vector<System>& state,
		std::vector<std::pair<size_t, size_t>>& groups) const
	{
		profiler _("_reconstruct");

		for (size_t i = 0; i < qram_->branches.size(); ++i)
		{
			auto& group = groups[i];
			auto& branch = qram_->branches[i];
			auto& branch_prob = qram_->branch_probs[i];

			complex_t multiplier, good_phase;
			if (branch.is_good())
			{
				switch (branch.good_ref->system_size())
				{
				case 0:
					// erased
					for (auto i = group.first; i < group.second; ++i)
					{
						if (!ConditionSatisfied(state[i]))
							continue;
						state[i].amplitude = 0;
					}
					break;
				case 1:
					// direct map
					multiplier = std::sqrt(branch.relative_multiplier) * branch.good_ref->iterbeg()->amplitude;

					for (auto i = group.first; i < group.second; ++i)
					{
						if (!ConditionSatisfied(state[i]))
							continue;
						state[i].amplitude *= multiplier;
						state[i].registers[register_data].value ^= qram_->memory[branch.address];
					}
					break;
				default:
					throw_bad_result();
				}
			}
			else {
				size_t mapsz = branch.system_size();
				switch (mapsz)
				{
				case 0:
					// erased
					for (auto i = group.first; i < group.second; ++i)
					{
						if (!ConditionSatisfied(state[i]))
							continue;
						state[i].amplitude = 0;
					}
					break;
				case 1:
					// direct map
					for (auto i = group.first; i < group.second; ++i)
					{
						if (!ConditionSatisfied(state[i]))
							continue;
						state[i].amplitude *= branch.iterbeg()->amplitude;
						state[i].registers[register_data].value = branch.iterbeg()->data_bus;
					}
					break;
				default:
					throw_bad_result();
				}
			}
		}
	}

	void QRAMLoad::operator()(std::vector<System>& state) const
	{
		profiler _("QRAMLoad");

		if (state.size() == 0) return;

		// if version is not selected or selected as "noisefree"
		if (version == "" ||
			version == "noisefree" ||
			qram->is_noise_free())
		{
			noise_free_impl(state);
		}
		// A normal call to QRAMCircuit::run
		else if (version == "fast")
		{
			QRAMLoadFast(qram, register_addr, register_data)(state);
		}
		else if (version == "normal" || version == "new"
			|| version == "full" || version == "old")
		{
			(SortByKey2(register_addr, register_data))(state);

			// Make a copy of QRAM
			auto qram_copy = qram_qutrit::QRAMCircuit(*qram);

			/* A cache for grouping the input state with the same addr and data */
			std::vector<std::pair<size_t, size_t>> groups;

			_set_branches(&qram_copy, state, groups);
			if (qram_copy.branches.size() == 0)
				return;
			qram_copy.run(version);
			qram_copy.sample_output_without_normalization();

			_reconstruct(&qram_copy, state, groups);
			ClearZero()(state);
			Normalize()(state);
		}
		else {
			throw_bad_switch_case();
		}
	}


	QRAMLoadFast::QRAMLoadFast(const qram_qutrit::QRAMCircuit* qram_, size_t reg1, size_t reg2)
		:qram(qram_), register_addr(reg1), register_data(reg2)
	{ }

	QRAMLoadFast::QRAMLoadFast(const qram_qutrit::QRAMCircuit* qram_, std::string_view reg1, std::string_view reg2)
		:qram(qram_), register_addr(System::get(reg1)), register_data(System::get(reg2))
	{ }

	void QRAMLoadFast::noise_free_impl(std::vector<System>& state) const
	{
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionSatisfied(s))
			{
				size_t addr = s.GetAs(register_addr, uint64_t) % pow2(qram->address_size);
				s.get(register_data).value ^= qram->memory[addr];
			}
		}
		}

	void QRAMLoadFast::has_damping_impl(std::vector<System>& state, qram_qutrit::QRAMCircuit* qram, std::vector<System>& state_remove_cache) const
	{
		TimeStep& time_step = qram->time_step;
		constexpr int arch = std::decay_t<decltype(qram->branches[0])>::arch_type;
		time_step.append_noise_range_only(qram->noise_parameters, arch);
		const memory_t& mem = qram->memory;

		size_t step = time_step.last_step();
		auto gamma = qram->noise_parameters[OperationType::Damping];

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			if (ConditionSatisfied(s))
			{
				size_t addr = s.GetAs(register_addr, uint64_t);
				size_t data = s.GetAs(register_data, uint64_t);
				if constexpr (arch == arch_qutrit)
				{
					auto decay_step = time_step._get_multiplier_impl_qutrit(step, addr, data, mem);

					if (time_step.is_bad_branch(addr))
					{
						state_remove_cache.emplace_back(s);
						s.amplitude = 0;
					}
					else {
						s.amplitude *= std::pow(1 - gamma, decay_step);
						s.get(register_data).value ^= qram->memory[addr];
					}
				}
				else {
					throw_invalid_input();
				}
			}
		}

		}

	void QRAMLoadFast::no_damping_impl(std::vector<System>&state, qram_qutrit::QRAMCircuit* qram, std::vector<System>& state_remove_cache) const
	{
		TimeStep& time_step = qram->time_step;
		constexpr int arch = std::decay_t<decltype(qram->branches[0])>::arch_type;
		time_step.append_noise_range_only(qram->noise_parameters, arch);
		const memory_t& mem = qram->memory;
		size_t state_size = state.size();

#ifndef SINGLE_THREAD
#pragma omp parallel for
#endif
		for (int i = 0; i < state_size; ++i)
		{
			auto& s = state[i];
			if (ConditionSatisfied(s))
			{
				size_t addr = s.GetAs(register_addr, uint64_t);
				if (time_step.is_bad_branch(addr))
				{
					state_remove_cache.emplace_back(s);
					s.amplitude = 0;
				}
				else {
					s.get(register_data).value ^= qram->memory[addr];
				}
			}
		}
	}

	void QRAMLoadFast::operator()(std::vector<System>&state) const
	{
		// For fast mode
		profiler _("QRAMLoadFast::operator()");

		if (state.size() == 0) return;

		// if version is not selected or selected as "noisefree"
		if (qram->is_noise_free())
		{
			noise_free_impl(state);
		}
		// A normal call to QRAMCircuit::run
		else {

			auto qram_copy = qram_qutrit::QRAMCircuit(*qram);
			auto qram_ = &qram_copy;
			std::vector<System> state_remove_cache;

			if (qram->has_damping())
			{
				has_damping_impl(state, qram_, state_remove_cache);
			}
			else
			{
				no_damping_impl(state, qram_, state_remove_cache);
			}
			ClearZero()(state);

			if (state.size() == 0)
			{
				QRAMLoadFast(qram, register_addr, register_data)(state_remove_cache);
				std::swap(state, state_remove_cache);
			}
			Normalize()(state);
		}
	}

}