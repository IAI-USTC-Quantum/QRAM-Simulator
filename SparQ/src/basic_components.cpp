#include "basic_components.h"

namespace qram_simulator
{

	const std::string& get_name(const StateInfoType& m)
	{
		return std::get<0>(m);
	}

	std::string& get_name(StateInfoType& m)
	{
		return std::get<0>(m);
	}

	const StateStorageType& get_type(const StateInfoType& m)
	{
		return std::get<1>(m);
	}

	StateStorageType& get_type(StateInfoType& m)
	{
		return std::get<1>(m);
	}

	size_t get_size(const StateInfoType& m)
	{
		return std::get<2>(m);
	}

	size_t& get_size(StateInfoType& m)
	{
		return std::get<2>(m);
	}

	bool get_status(const StateInfoType& m)
	{
		return std::get<3>(m);
	}

	bool& get_status(StateInfoType& m)
	{
		return std::get<3>(m);
	}

	//bool StateStorage::as_bool() const
	//{
	//	return bool(value & 1);
	//}

	uint64_t& StateStorage::val(size_t size)
	{
		if (size == 64)
		{
			return value;
		}
		else
		{
			value = value & pow2(size);
			return value;
		}
	}

	uint64_t StateStorage::val(size_t size) const
	{
		if (size == 64)
		{
			return value;
		}
		else
		{
			return value & pow2(size);
		}
	}

	bool StateStorage::operator==(const StateStorage& rhs) const
	{
		return value == rhs.value;
	}

	bool StateStorage::operator!=(const StateStorage& rhs) const
	{
		return value != rhs.value;
	}

	bool StateStorage::operator<(const StateStorage& rhs) const
	{
		return value < rhs.value;
	}

	bool StateStorage::operator>(const StateStorage& rhs) const
	{
		return value > rhs.value;
	}

	std::string StateStorage::to_string(const StateInfoType& info) const
	{
		if (!get_status(info))
			return std::string();

		switch (get_type(info))
		{
		case General:
			return fmt::format("|{}>", dec2bin(value, get_size(info)));
		case SignedInteger:
			return fmt::format("|{}>", get_complement(value, get_size(info)));
		case UnsignedInteger:
			return fmt::format("|{}>", as<uint64_t>(get_size(info)));
		case Boolean:
			return fmt::format("|{}>", as<bool>(get_size(info)));
		case Rational:
			return fmt::format("|{}>", as<double>(get_size(info)));
		default:
			throw_bad_switch_case();
			return "";
		}
	}

	std::string StateStorage::to_io_string(const StateInfoType& info) const
	{
		return fmt::format("|{}:{}>", get_name(info), value);
	}

	std::string StateStorage::to_binary_string(const StateInfoType& info) const
	{
		return fmt::format("|{}>", dec2bin(value, get_size(info)));
	}

	void StateStorage::flip(size_t digit)
	{
		value = flip_digit(value, digit);
	}

	/* Static variables */
	//std::vector<StateInfoType> System::name_register_map;
	//size_t System::max_qubit_count = 0;
	//size_t System::max_register_count = 0;
	//size_t System::max_system_size = 0;
	//size_t System::max_system_size = 0;
	//std::vector<size_t> System::temporal_registers;
	//std::vector<size_t> System::reusable_registers;

	void System::clear()
	{
		name_register_map.clear();
		temporal_registers.clear();
		reusable_registers.clear();
		max_qubit_count = 0;
		max_register_count = 0;
		max_system_size = 0;
	}

	size_t System::get_qubit_count()
	{
		size_t count = 0;
		for (const auto& m : name_register_map)
		{
			if (get_status(m))
				count += get_size(m);
		}
		return count;
	}

	size_t System::get_activated_register_size()
	{
		size_t count = 0;
		for (const auto& m : name_register_map)
		{
			if (get_status(m))
				count += 1;
		}
		return count;
	}

	size_t System::get_last_activated_register()
	{
		if (name_register_map.size() == 0)
			throw_general_runtime_error("No register is activated.");

		for (size_t i = name_register_map.size(); i-- > 0; )
		{
			if (status_of(i))
				return i;
		}
		return SIZE_MAX;
	}

	StateStorage& System::last_register()
	{
		if (name_register_map.size() == 0)
			throw_general_runtime_error();

		size_t id = get_last_activated_register();

		return registers[id];
	}

	const StateStorage& System::last_register() const
	{
		if (name_register_map.size() == 0)
			throw_general_runtime_error();

		size_t id = get_last_activated_register();

		return registers[id];
	}

	void System::update_max_size(size_t new_size)
	{
		if (new_size > max_system_size)
			max_system_size = new_size;
	}

	size_t System::get(std::string_view name)
	{
#ifdef SINGLE_THREAD
		profiler _("System::get");
#endif
		for (size_t i = name_register_map.size(); i --> 0; )
		{
			if (!status_of(i))
				continue;
			if (name == name_of(i))
				return i;
		}
		return SIZE_MAX;
	}

	StateInfoType System::get_register_info(std::string_view name)
	{
		auto iter = std::find_if(name_register_map.rbegin(), name_register_map.rend(),
			[name](const StateInfoType& n)
			{
				return get_name(n) == name;
			}
		);

		if (iter == name_register_map.rend())
			throw_general_runtime_error("Register not found.");

		return *iter;
	}

	const std::string& System::name_of(size_t id)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
		const auto& info = name_register_map[id];
		return get_name(info);
	}

	size_t System::size_of(std::string_view name)
	{
		profiler _("System::size_of");
		auto&& info = get_register_info(name);
		return get_size(info);
	}

	size_t System::size_of(size_t id)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
		const auto& info = name_register_map[id];
		return get_size(info);
	}

	StateStorageType System::type_of(std::string_view name)	
	{
		profiler _("System::type_of");
		auto&& info = get_register_info(name);
		return get_type(info);
	}

	StateStorageType System::type_of(size_t id)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
		const auto& info = name_register_map[id];
		return get_type(info);
	}

	bool System::status_of(std::string_view name)
	{
		size_t id = get(name);
		return status_of(id);
	}

	bool System::status_of(size_t id)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
		return get_status(name_register_map[id]);
	}

	std::string System::to_string() const
	{
		std::string ret = fmt::format("{} ", amplitude);
		for (size_t i = 0; i < name_register_map.size(); ++i)
		{
			if (System::status_of(i))
			{
				auto& reg = registers[i];
				ret += reg.to_string(name_register_map[i]);
			}
		}
		return ret;
	}

	void System::add_register_status_bitmap(size_t pos)
	{
		reg_status_bitmap |= pow2(pos);
	}

	void System::remove_register_status_bitmap(size_t pos)
	{
		reg_status_bitmap -= pow2(pos);
	}

	size_t System::add_register(std::string_view name, StateStorageType type, size_t size)
	{
		if (name_register_map.size() >= CachedRegisterSize)
		{
			// try to reuse the space
			if (reusable_registers.size() == 0)
				// all spaces are run out
				throw_general_runtime_error("All spaces are run out when adding new register.");
			else {
				size_t reg_id = reusable_registers.back();
				name_register_map[reg_id] = { std::string(name), type, size, true };								
				reusable_registers.pop_back();
				add_register_status_bitmap(reg_id);
				return reg_id;
			}
		}
		else {
			name_register_map.emplace_back(name, type, size, true);
			if (get_qubit_count() > max_qubit_count)
				max_qubit_count = get_qubit_count();
			if (get_activated_register_size() > max_register_count)
				max_register_count = get_activated_register_size();

			size_t reg_id = name_register_map.size() - 1;
			add_register_status_bitmap(reg_id);
			return reg_id;
		}

		return SIZE_MAX;
	}

	size_t System::add_register_synchronous(
		std::string_view name, StateStorageType type, size_t size,
		std::vector<System>& system_states)
	{
		size_t ret = add_register(name, type, size);
		for (auto& s : system_states)
		{
			s.get(ret).value = 0;
		}
		return ret;
	}

	size_t System::add_register_synchronous(
		std::string_view name, StateStorageType type, size_t size,
		SparseState& system_states)
	{
		return add_register_synchronous(name, type, size, system_states.basis_states);
	}

	void System::remove_register(size_t id)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
		if (!get_status(name_register_map[id]))
			throw_general_runtime_error("Register is not activated.");

		get_status(name_register_map[id]) = false;
		reusable_registers.push_back(id);
		remove_register_status_bitmap(id);
	}

	void System::remove_register(std::string_view name)	
	{
		size_t id = get(name);
		remove_register(id);
	}

	void System::remove_register_synchronous(size_t id, std::vector<System>& state)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
#ifdef SINGLE_THREAD
		for (auto& s : state)
		{
#else
#pragma omp parallel for
		for (int i = 0; i < state.size(); ++i)
		{
			auto& s = state[i];
#endif
			// Warning: Unchecked removal.
			s.registers[id].value = 0;

			// FOR DEBUG
			/*s.registers[id] = s.last_register();
			s.last_register().value = 0;*/
		}
		remove_register(id);
	}

	void System::remove_register_synchronous(size_t id, SparseState& state)
	{
		if (id >= name_register_map.size())
			throw_general_runtime_error("Register not found.");
		remove_register_synchronous(id, state.basis_states);
	}

	void System::remove_register_synchronous(std::string_view name, std::vector<System>& state)
	{
		size_t id = get(name);
		remove_register_synchronous(id, state);
	}

	void System::remove_register_synchronous(std::string_view name, SparseState& state)
	{
		size_t id = get(name);
		remove_register_synchronous(id, state.basis_states);
	}

	bool System::operator<(const System& rhs) const	
	{
		for (size_t i = 0; i < name_register_map.size(); ++i)
		{
			if (!status_of(i))
				continue;
			auto& regl = registers[i];
			auto& regr = rhs.registers[i];
			if (regl < regr) return true;
			if (regr < regl) return false;
		}
		return false;
	}

	bool System::operator==(const System& rhs) const
	{
		for (size_t i = 0; i < name_register_map.size(); ++i)
		{
			if (!status_of(i))
				continue;
			auto& regl = registers[i];
			auto& regr = rhs.registers[i];
			if (regl == regr)
				continue;
			else
				return false;
		}
		return true;
	}

	bool System::operator!=(const System& rhs) const
	{
		return !(*this == rhs);
	}

	std::string System::to_string(int precision) const
	{
		if (precision < 2 || precision > 18)
		{
			throw_invalid_input();
		}
		int width = 1;
		std::string format_string = fmt::format("{{: {}.{}f}} {{:+{}.{}f}}i", width, precision, width, precision);
		std::string ret = fmt::format(format_string, amplitude.real(), amplitude.imag());
		for (size_t i = 0; i < name_register_map.size(); ++i)
		{
			if (System::status_of(i))
			{
				auto& reg = registers[i];
				ret += reg.to_io_string(name_register_map[i]);
			}
		}
		return ret;
	}

	void merge_system(System& s1, System& s2)	
	{
		s1.amplitude += s2.amplitude;
		s2.amplitude = 0;
	}

	bool remove_system(const System& s)
	{
		return abs_sqr(s.amplitude) < epsilon;
	}
}