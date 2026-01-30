#include "system_operations.h"
#include "quantum_arithmetic.h"

namespace qram_simulator
{
	/* Split system state with `condition_variable`;
	*  Static variables of new state: Unchanged.
	*  Possible unsafety: Both split states are invalid and they appear only
	*  in the intermediate process.
	*/
	//std::vector<System> SplitSystems::operator()(std::vector<System>& state) const
	//{
	//	profiler _("SplitSystems");
	//	double factor = 0;
	//	if (!HasCondition)
	//		return std::vector<System>{};

	//	auto _conditionsatisfied = [this](const System& s) {
	//		return ConditionSatisfied(s);
	//	};
	//	auto p = std::partition(state.begin(), state.end(), _conditionsatisfied);
	//	std::vector<System> unconditioned_state(p, state.end());
	//	state.erase(p, state.end());
	//	return unconditioned_state;
	//}

	/* Combine systems without changing each brach.
	*  Possible unsafety: Assuming the two state are in different subspaces.
	*/
	//void CombineSystems::operator()(std::vector<System>& state_original,
	//	std::vector<System>& state_tomove) const
	//{
	//	profiler _("CombineSystems");
	//	double coeff = 1.0;
	//	double amp_sqrt_original = 0.0;
	//	double amp_sqrt_tomove = 0.0;

	//	std::copy(state_tomove.begin(), state_tomove.end(), std::back_inserter(state_original));
	//}

	//void ResetSystems::operator()(std::vector<System>& state) const
	//{
	//	state.clear();
	//	state.emplace_back();
	//}

	void split_systems(std::vector<System>& new_state, std::vector<System>& old_state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	)
	{
		auto _conditionsatisfied = [&](const System& s) {
			return ConditionSatisfied(s);
			};
		auto p = std::partition(old_state.begin(), old_state.end(), _conditionsatisfied);

		new_state.clear();
		new_state = std::vector<System>(p, old_state.end());
		old_state.erase(p, old_state.end());
	}

	std::vector<System> split_systems(std::vector<System>& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	)
	{
		std::vector<System> unconditioned_state;
		split_systems(unconditioned_state, state,
			condition_variable_nonzeros,
			condition_variable_all_ones,
			condition_variable_by_bit,
			condition_variable_by_value);
		return unconditioned_state;
	}

	SparseState split_systems(SparseState& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	)
	{
		SparseState unconditioned_state(0);
		split_systems(unconditioned_state.basis_states,
			state.basis_states,
			condition_variable_nonzeros,
			condition_variable_all_ones,
			condition_variable_by_bit,
			condition_variable_by_value);
		return unconditioned_state;
	}

	void combine_systems(std::vector<System>& to, const std::vector<System>& from)
	{
		std::copy(from.begin(), from.end(), std::back_inserter(to));
	}

	void combine_systems(SparseState& to, const SparseState& from)
	{
		combine_systems(to.basis_states, from.basis_states);
	}

	void reset_systems(std::vector<System>& state)
	{
		state.clear();
		state.emplace_back();
	}

	void reset_systems(SparseState& state)
	{
		reset_systems(state.basis_states);
	}

	size_t SplitRegister::operator()(std::vector<System>& state) const
	{
		profiler _("SplitRegister");
		size_t first_pos = System::get(first_name);

		size_t original_size = System::size_of(first_pos);
		if (original_size < second_size)
		{
			throw_invalid_input();
		}

		size_t first_size = original_size - second_size;
		StateInfoType info = System::name_register_map[first_pos];

		StateStorageType type = get_type(info);

		// add second register
		size_t second_pos = AddRegister(second_name, type, second_size)(state);

		// modify the first size
		get_size(System::name_register_map[first_pos]) = first_size;

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			auto& value = s.get(first_pos).value;
			s.get(second_pos).value = value & (pow2(second_size) - 1);
			value >>= second_size;
		}

		return second_pos;
	}

	size_t SplitRegister::operator()(SparseState& state) const
	{
		return (*this)(state.basis_states);
	}

	size_t CombineRegister::operator()(std::vector<System>& state) const
	{
		profiler _("CombineRegister");
		size_t first_pos = System::get(first_name);
		size_t second_pos = System::get(second_name);

		size_t first_size = System::size_of(first_pos);
		size_t second_size = System::size_of(second_pos);

		size_t combine_size = first_size + second_size;
		StateInfoType info = System::name_register_map[first_pos];
		StateStorageType type = std::get<1>(info);

		// modify the first size
		std::get<2>(System::name_register_map[first_pos]) = combine_size;

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			auto& value = s.get(first_pos).value;
			value <<= second_size;
			value += s.get(second_pos).value;
		}
		System::remove_register(second_name);
		return first_pos;
	}

	size_t CombineRegister::operator()(SparseState& state) const
	{
		return (*this)(state.basis_states);
	}

	MoveBackRegister::MoveBackRegister(std::string_view reg_in)
		: register_id(System::get(reg_in))
	{ }

	MoveBackRegister::MoveBackRegister(size_t reg_in)
		: register_id(reg_in)
	{ }

	void MoveBackRegister::operator()(std::vector<System>& state) const
	{
		auto& name_register_map = System::name_register_map;

		if (register_id == name_register_map.size() - 1)
			// already at back, do nothing
			return;

		std::swap(name_register_map.back(), name_register_map[register_id]);

#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			std::swap(s.get(register_id), s.last_register());
		}
	}

	void MoveBackRegister::operator()(SparseState& state) const
	{
		(*this)(state.basis_states);
	}

	AddRegister::AddRegister(std::string_view register_name_,
		StateStorageType type_, size_t size_)
	{
		register_name = register_name_;
		type = type_;
		size = size_;

		if (size > 64)
			throw_invalid_input();
	}

	size_t AddRegister::operator()(std::vector<System>& state) const
	{
		profiler _("AddRegister");
		return System::add_register_synchronous(register_name, type, size, state);
	}

	size_t AddRegister::operator()(SparseState& state) const
	{
		return (*this)(state.basis_states);
	}

	AddRegisterWithHadamard::AddRegisterWithHadamard(std::string_view register_name_,
		StateStorageType type_, size_t size_)
	{
		register_name = register_name_;
		type = type_;
		size = size_;
	}

	size_t AddRegisterWithHadamard::operator()(std::vector<System>& state) const
	{
		profiler _("AddRegister");
		size_t ret = System::add_register_synchronous(register_name, type, size, state);

		size_t fullsize = pow2(size);
		size_t original_size = state.size();
		state.resize(original_size * fullsize);

		for (size_t i = 1; i < fullsize; ++i)
		{
			std::copy(exec_policy, state.begin(), state.begin() + original_size,
				state.begin() + i * original_size);

#ifndef SINGLE_THREAD
#pragma omp parallel for
#endif
			for (int64_t j = i * original_size; j < (i + 1) * original_size; ++j)
			{
				state[j].get(ret).value = i;
			}
		}

		return ret;
	}

	size_t AddRegisterWithHadamard::operator()(SparseState& state) const
	{
		return (*this)(state.basis_states);
	}

	RemoveRegister::RemoveRegister(std::string_view register_name_)
		: register_id(System::get(register_name_))
	{	}

	RemoveRegister::RemoveRegister(size_t register_name_)
		: register_id(register_name_)
	{	}

	void RemoveRegister::operator()(std::vector<System>& state) const
	{
		/* discard this line if everything is OK. */
		(TestRemovable(register_id))(state);
		profiler _("RemoveRegister");
		/*fmt::print("\nbefore partial_trace:\n");
		StatePrint()(state);*/
		// (PartialTrace(register_id))(state);
		/*fmt::print("\npartial_trace:\n");
		StatePrint()(state);*/
		System::remove_register_synchronous(register_id, state);
	}

	void RemoveRegister::operator()(SparseState& state) const
	{
		(*this)(state.basis_states);
	}

	void Push::operator()(std::vector<System>& state) const
	{
		profiler _("Push");
		auto&& info = System::name_register_map[reg_id];
		size_t garbage_id = AddRegister(garbage_name, std::get<1>(info), std::get<2>(info))(state);

		/* FOR DEBUG */
		//// move the content into the garbage register
		Swap_General_General(reg_id, garbage_id)(state);

		//// add the name into stack (for whenever resuming)
		System::temporal_registers.push_back(garbage_id);
		//fmt::print("Push {} ({})\n", garbage_id, garbage_name);

		//Swap_General_General(reg_name, garbage_name)(state);
		//System::temporal_register_name.push_back(garbage_name);
	}

	void Pop::operator()(std::vector<System>& state) const
	{
		profiler _("Pop");

		/* FOR DEBUG */
		size_t garbage_id = System::temporal_registers.back();
		Swap_General_General(reg_id, garbage_id)(state);

		/* remove if you hope faster (and with trust) */
		// (PartialTrace(garbage_name))(state);

		(RemoveRegister(garbage_id))(state);
		System::temporal_registers.pop_back();
	}

	void ClearZero::operator()(std::vector<System>& system_states) const
	{
		profiler _("ClearZero");
		auto self_defined_epsilon = eps;

		auto iter = std::remove_if(system_states.begin(), system_states.end(),
			[self_defined_epsilon](const System& sys) {
				return abs_sqr(sys.amplitude) < self_defined_epsilon;
			}
		);
		system_states.erase(iter, system_states.end());
	}

#ifdef _MSC_VER
#pragma warning(disable : 4715) // warning: 不是所有的控件路径都返回值
#endif

	complex_t StateLoad::load_amplitude(const std::string& line) const
	{
		std::regex amplitude_pattern(R"((-?\d*\.?\d+)\s+([-+]?\d*\.?\d+)i)");
		std::smatch amplitude_match;

		if (std::regex_search(line, amplitude_match, amplitude_pattern) && amplitude_match.size() == 3)
		{
			double real_part = std::stod(amplitude_match[1].str());
			double imag_part = std::stod(amplitude_match[2].str());
			return complex_t(real_part, imag_part);
		}
		throw_invalid_input();
	}

	size_t StateLoad::load_reg(const std::string& line, const std::string& reg) const
	{
		std::string reg_pattern_str = R"(\|)" + reg + R"(:([^>]+)>)";
		std::regex reg_pattern(reg_pattern_str);
		std::smatch reg_match;

		if (std::regex_search(line, reg_match, reg_pattern) && reg_match.size() == 2) {
			return std::stoull(reg_match[1].str());
		}
		throw_invalid_input();
	}

#ifdef _MSC_VER
#pragma warning(default : 4715) // cancel warning suppression
#endif

	System StateLoad::load_branch(const std::string& line) const
	{
		std::vector<std::string> reg_list = { main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4 };
		System s;
		s.amplitude = load_amplitude(line);
		for (auto reg : reg_list)
		{
			s.get(System::get(reg)).value = load_reg(line, reg);
		}
		return s;
	}

	bool StateLoad::is_branch(const std::string& line) const
	{
		std::regex pattern(R"(\|[^>]*>)");

		// Use std::regex_search to check if the pattern `|...>` exists in the line
		return std::regex_search(line, pattern);
	}

	std::vector<System> StateLoad::operator()(const std::string& savename_) const
	{
		std::ifstream f_load(savename_);
		std::string line;
		std::vector<System> state;
		bool brach_mode = false;

		if (!std::filesystem::exists(savename_))
		{
			fmt::print("file `{}` does not exist.\n", savename_);
			std::exit(5);
		}

		{
			if (!f_load.is_open()) {
				throw std::runtime_error("Failed to open file.\n");
			}

			while (std::getline(f_load, line))
			{
				if (line.empty())
				{
					continue;
				}

				if (!brach_mode)
				{
					brach_mode = is_branch(line);
				}
				if (brach_mode)
				{
					state.push_back(load_branch(line));
				}
				else {
					continue;
				}
			}
		}
		if (!brach_mode) { // The state file is void!
			fmt::print("file `{}` is not a record of quantum state.\n", savename_);
			std::exit(3);
		}
		CheckNormalization(1e-14)(state);
		return state;
	}
	
}