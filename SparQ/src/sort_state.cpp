#include "sort_state.h"

namespace qram_simulator
{
	void SortExceptKey::operator()(std::vector<System>& states) const
	{
		profiler _("SortExceptKey");

		auto pred = [this](const System& lhs, const System& rhs)
			{
				// profiler _("SortExceptKey::Compare");
				for (size_t i = 0; i < System::name_register_map.size(); ++i)
				{
					if (!System::status_of(i))
						continue;

					if (i != id)
					{
						if (lhs.registers[i] < rhs.registers[i])
							return true;
						else if (lhs.registers[i] == rhs.registers[i])
							continue;
						else
							return false;
					}
				}
				return lhs.registers[this->id] < rhs.registers[this->id];
			};

		std::sort(exec_policy, states.begin(), states.end(), pred);
	}

	SortByKey::SortByKey(std::string_view key)
		:register_key(System::get(key))
	{ }

	SortByKey::SortByKey(size_t key)
		:register_key(key)
	{ }

	void SortByKey::operator()(std::vector<System>& state) const
	{
		std::sort(exec_policy, state.begin(), state.end(),
			[this](System& lhs, System& rhs) {
				return lhs.get(register_key) < rhs.get(register_key);
			}
		);
	}

	void SortExceptBit::operator()(std::vector<System>& states) const
	{
		profiler _("SortExceptBit");

		auto pred = [this](const System& lhs, const System& rhs)
			{
				uint64_t mask = ~pow2(digit);
				for (size_t i = 0; i < System::name_register_map.size(); ++i)
				{
					if (!System::status_of(i))
						continue;

					if (i != this->id)
					{
						if (lhs.registers[i] < rhs.registers[i])
							return true;
						else if (lhs.registers[i] > rhs.registers[i])
							return false;
					}
				}
				// Compare the special 'id' register with the digit ignored
				uint64_t lhs_value = lhs.registers[this->id].value & mask;
				uint64_t rhs_value = rhs.registers[this->id].value & mask;
				return lhs_value < rhs_value;
			};

		std::sort(exec_policy, states.begin(), states.end(), pred);
	}

	void SortUnconditional::operator()(std::vector<System>& states) const
	{
		std::sort(exec_policy, states.begin(), states.end());
	}


	void SortByAmplitude::operator()(std::vector<System>& states) const
	{
		auto pred = [](const System& s1, const System& s2)
			{
				return abs_sqr(s1.amplitude) > abs_sqr(s2.amplitude);
			};

		std::sort(exec_policy, states.begin(), states.end(), pred);
	}


	void SortByKey2::operator()(std::vector<System>& states) const
	{
		profiler _("SortExceptKey2");
		auto pred = [this](const System& lhs, const System& rhs)
			{
				if (lhs.registers[id1] < rhs.registers[id1])
					return true;
				if (lhs.registers[id1] > rhs.registers[id1])
					return false;
				if (lhs.registers[id2] < rhs.registers[id2])
					return true;
				if (lhs.registers[id2] > rhs.registers[id2])
					return false;
				for (size_t i = 0; i < System::name_register_map.size(); ++i)
				{
					if (!System::status_of(i))
						continue;

					if (i != id1 && i != id2)
					{
						if (lhs.registers[i] < rhs.registers[i])
							return true;
						if (lhs.registers[i] > rhs.registers[i])
							return false;
					}
				}
				return false;
			};

		std::sort(exec_policy, states.begin(), states.end(), pred);
	}

	size_t SortExceptKeyHadamard::remove_digits(size_t val) const
	{
		return val & mask;
	}

	void SortExceptKeyHadamard::operator()(std::vector<System>& states)  const
	{
		profiler _("SortExceptKeyHadamard");

		auto pred = [this](const System& lhs, const System& rhs)
			{
				for (size_t i = 0; i < System::name_register_map.size(); ++i)
				{
					if (!System::status_of(i))
						continue;

					if (i != id)
					{
						if (lhs.registers[i] < rhs.registers[i])
							return true;
						else if (lhs.registers[i] == rhs.registers[i])
							continue;
						else
							return false;
					}
					else {
						if (remove_digits(lhs.registers[i].value) < remove_digits(rhs.registers[i].value))
							return true;
						else if (remove_digits(lhs.registers[i].value) == remove_digits(rhs.registers[i].value))
							continue;
						else
							return false;
					}
				}
				return lhs.registers[this->id] < rhs.registers[this->id];
			};

		std::sort(exec_policy, states.begin(), states.end(), pred);
	}

	bool compare_equal(const System& a, const System& b, size_t out_id)
	{
		profiler _("compare_equal");
		size_t sz = System::name_register_map.size();

		for (size_t i = sz; i --> 0;)
		{
			if (!System::status_of(i))
				continue;
			if (i == out_id)
				continue;
			if (!(a.registers[i] == b.registers[i]))
				return false;
		}
		return true;
	}

	bool compare_equal2(const System& a, const System& b, size_t out_id1, size_t out_id2)
	{
		if (a.registers[out_id1] == b.registers[out_id1] &&
			a.registers[out_id2] == b.registers[out_id2])
			return true;
		else
			return false;

		//int sz = System::name_register_map.size();

		//for (int i = sz - 1; i >= 0; --i)
		//{
		//	if (!System::status_of(i))
		//		continue;
		//	if (i == out_id1)
		//		continue;
		//	if (i == out_id2)
		//		continue;
		//	if (!(a.registers[i] == b.registers[i]))
		//		return false;
		//}
		//return true;
	}

	bool compare_equal_rot(const System& a, const System& b, size_t out_id, uint64_t mask)
	{
		profiler _("compare_equal");
		size_t sz = System::name_register_map.size();

		for (size_t i = sz; i-- > 0;)
		{
			if (!System::status_of(i))
				continue;
			if (i == out_id) {
				if ((a.registers[out_id].value & mask) != (b.registers[out_id].value & mask))
				{
					return false;
				}
				continue;
			}
			if (a.registers[i] != b.registers[i])
				return false;
		}
		return true;
	}

	bool compare_equal_hadamard(const System& a, const System& b, size_t out_id, uint64_t mask)
	{
		profiler _("compare_equal");
		size_t sz = System::name_register_map.size();

		if ((a.registers[out_id].value & mask) != (b.registers[out_id].value & mask))
		{
			return false;
		}
		for (size_t i = sz; i --> 0;)
		{
			if (!System::status_of(i))
				continue;
			if (i == out_id)
				continue;
			if (a.registers[i] != b.registers[i])
				return false;
		}
		return true;
	}

}