#include "debugger.h"
#include "sort_state.h"

namespace qram_simulator {
	CheckNormalization::CheckNormalization() = default;

	void CheckNormalization::operator()(std::vector<System>& state) const
	{
#ifndef QRAM_Release
		double factor = 0;
		for (const auto& s : state)
		{
			factor += abs_sqr(s.amplitude);
		}
		if ((!ignorable(factor - 1.0, threshold)) ||
			(std::isnan(factor)))
		{
			fmt::print("\n Error! Factor = {}\n", factor);
			throw_bad_result("State is not normalized.");
		}
#endif
	}

	void CheckNormalization_Renormalize::operator()(std::vector<System>& state) const
	{
#ifndef QRAM_Release
		double factor = 0;
		for (const auto& s : state)
		{
			factor += abs_sqr(s.amplitude);
		}
		if ((!ignorable(factor - 1.0, threshold)) ||
			(std::isnan(factor)))
		{

			fmt::print("\n Error! Factor = {}\n", factor);
			throw_bad_result("State is not normalized.");
		}

		factor = 1.0 / sqrt(factor);

		// 3. renormalize
		std::for_each(exec_policy, state.begin(), state.end(),
			[factor](System& s) { s.amplitude *= factor; }
		);
#endif
	}

	CheckNan::CheckNan() = default;

	void CheckNan::operator()(std::vector<System>& state) const
	{
#ifndef QRAM_Release
		double factor = 0;
		for (const auto& s : state)
		{
			if (std::isnan(s.amplitude.real()) ||
				std::isnan(s.amplitude.imag()))
			{
				throw_bad_result();
			}
		}
#endif
	}

	ViewNormalization::ViewNormalization() = default;

	void ViewNormalization::operator()(std::vector<System>& state) const
	{
#ifndef QRAM_Release
		double factor = 0;
		for (const auto& s : state)
		{
			factor += abs_sqr(s.amplitude);
		}
		fmt::print("Factor = {}\n", factor);
#endif
	}

	bool StatePrint::on = true;

	std::string StatePrint::disp2str() const
	{
		auto concat = [](std::string& str, const std::string& m)
			{
				if (str.size() > 0) str = fmt::format("{}, {}", str, m);
				else str = m;
			};

		if (display == Default) return "Default";
		std::string ret;
		if (display & Detail) concat(ret, "Detail");
		if (display & Binary) concat(ret, "Binary");
		if (display & Prob) concat(ret, "Prob");

		return ret;
	}

	void StatePrint::operator()(std::vector<System>& state) const
	{
		if (!on)
			return;

		fmt::print("StatePrint (mode={})\n", disp2str());
		if (display == Default)
		{
			for (auto& system_state : state) {
				if (precision) {
					fmt::print("{}\n", system_state.to_string(precision));
				}
				else {
					fmt::print("{}\n", system_state.to_string());
				}
			}
			return;
		}

		if (display & Detail)
		{
			for (int i = 0; i < System::name_register_map.size(); ++i)
			{
				auto& v = System::name_register_map[i];
				if (get_status(v) == false)
					continue;
				fmt::print("|({}){} : {}{} | ", i, get_name(v),
					get_type_str(get_type(v)),
					std::get<2>(v));
			}
			fmt::print("\n");
		}

		for (size_t i = 0; i < state.size(); ++i) {

			if (i >= 64)
			{
				fmt::print("... (total {})\n", state.size());
				break;
			}

			auto& s = state[i];
			fmt::print("{} ", s.amplitude);

			if (display & Prob)
				fmt::print("(p = {}) ", abs_sqr(s.amplitude));

			auto& regs = s.registers;
			for (int id = 0; id < s.name_register_map.size(); ++id)
			{
				//Debug_CheckOverflow(id);
				if (!System::status_of(id))
					continue;
				auto& reg = regs[id];
				auto info = System::name_register_map[id];

				if (display & Detail)
					fmt::print(" {}=", get_name(System::name_register_map[id]));

				if (display & Binary)
					fmt::print(reg.to_binary_string(info));
				else
					fmt::print(reg.to_string(info));
			}
			fmt::print("\n");
		}
	}

	TestRemovable::TestRemovable(std::string_view register_name_)
	{
		register_id = System::get(register_name_);
	}

	TestRemovable::TestRemovable(size_t register_name_)
	{
		register_id = register_name_;
	}

	void TestRemovable::operator()(std::vector<System>& state) const
	{
		if (state.size() == 0) return;
		auto remove_value = state[0].GetAs(register_id, uint64_t);

		for (int i = 1; i < state.size(); ++i)
		{
			auto& s = state[i];
			if (s.GetAs(register_id, uint64_t) != remove_value)
			{
				fmt::print("Removing failed. RegName: {}, i = {}, val = {} (different from {})\n",
					System::name_of(register_id),
					i,
					s.GetAs(register_id, uint64_t),
					remove_value
				);
				(StatePrint(StatePrintDisplay::Detail))(state);

				throw_general_runtime_error();
			}
		}
	}


	void CheckDuplicateKey::operator()(std::vector<System>& system_states) const
	{
		SortUnconditional()(system_states);
		if (!check_unique_sort(system_states))
		{
			throw_general_runtime_error();
		}
	}


	bool CheckDuplicateKey::has_duplicate(std::vector<System>& system_states) const
	{
		SortUnconditional()(system_states);
		return !check_unique_sort(system_states);
	}
}