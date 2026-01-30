#include "quantum_interfere_basic.h"

namespace qram_simulator {

	size_t StateHashExceptKey::operator()(const System& v) const {
		//auto seed = v.registers.size();
		//for (int i = 0; i < System::name_register_map.size();++i) {
		//	if (System::status_of(i))
		//		seed ^= v.get(i).value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		//}
		//
		//return seed;

		//std::vector<size_t> vec;
		//for (int i = 0; i < System::name_register_map.size(); ++i) {
		//	if (System::status_of(i))
		//		vec.push_back(v.get(i).value);
		//}
		//return std::hash<std::vector<size_t>>(vec);

		//auto seed = 0;
		//for (int i = 0; i < System::name_register_map.size();++i) {
		//	size_t x = v.get(i).value;
		//	if (System::status_of(i))
		//	{
		//		x = ((x >> 16) ^ x) * 0x45d9f3b;
		//		x = ((x >> 16) ^ x) * 0x45d9f3b;
		//		x = (x >> 16) ^ x;
		//		seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		//	}
		//}
		//
		//return seed;

		std::basic_string<char32_t> s;
		for (size_t i = 0; i < System::name_register_map.size(); ++i) {
			if (System::status_of(i)) {
				if (i != id)
				{
					uint64_t value = v.get(i).value;
					s.push_back(value); value >>= 32;
					s.push_back(value); 
				}
			}
		}
		return std::hash<std::basic_string<char32_t>>()(s);

	}

	size_t StateHashExceptQubits::operator()(const System& v) const 
	{
		/*size_t mask = make_mask(qubit_positions);
		std::string s;
		for (int i = 0; i < System::name_register_map.size(); ++i) {
			if (System::status_of(i)) {
				if (i != id)
				{
					s.push_back(v.get(i).value);
				}
				else
				{
					s.push_back(v.get(i).value & mask);
				}
			}
		}
		return std::hash<std::string>()(s);*/
		size_t mask = make_mask(qubit_positions);
		size_t hash_value = 0;
		std::hash<size_t> hash_fn;  // use hash function from STL

		for (size_t i = 0; i < System::name_register_map.size(); ++i) {
			if (System::status_of(i)) {
				size_t value = v.get(i).value;
				// if i == id£¬use mask£»else£¬use value
				if (i == id) {
					value &= mask;
				}
				// use bitwise operations combination
				hash_value ^= hash_fn(value) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
			}
		}

		return hash_value;
	}

	size_t StateEqualExceptKey::operator()(const System& v1, const System& v2) const {
		for (size_t i = 0; i < System::name_register_map.size(); ++i)
		{
			if (!System::status_of(i))
				continue;

			if (i != id)
			{
				if (v1.registers[i] != v2.registers[i])
					return false;
			}
		}
		// equal
		return true;
	}

	size_t StateEqualExceptQubits::operator()(const System& v1, const System& v2) const {
		size_t mask = make_mask(qubit_positions);

		for (size_t i = 0; i < System::name_register_map.size(); ++i)
		{
			if (!System::status_of(i))
				continue;

			if (i != id)
			{
				if (v1.registers[i] != v2.registers[i])
					return false;
			}
			else
			{
				if ((v1.registers[i].value & mask) != (v2.registers[i].value & mask))
					return false;
			}
		}
		// equal
		return true;
	}

	size_t StateLessExceptKey::operator()(const System& v1, const System& v2) const
	{
		for (size_t i = 0; i < System::name_register_map.size(); ++i)
		{
			if (!System::status_of(i))
				continue;

			if (i != id)
			{
				if (v1.registers[i] < v2.registers[i])
					return true;
				else if (v1.registers[i] == v2.registers[i])
					continue;
				else
					return false;
			}
		}
		// equal
		return false;
	}

	size_t StateLessExceptQubits::operator()(const System& v1, const System& v2) const
	{
		for (size_t i = 0; i < System::name_register_map.size(); ++i)
		{
			if (!System::status_of(i))
				continue;

			if (i != id)
			{
				if (v1.registers[i] < v2.registers[i])
					return true;
				else if (v1.registers[i] == v2.registers[i])
					continue;
				else
					return false;
			}
			else {
				if (remove_digits(v1.registers[i].value) < remove_digits(v2.registers[i].value))
					return true;
				else if (remove_digits(v1.registers[i].value) == remove_digits(v2.registers[i].value))
					continue;
				else
					return false;
			}
		}
		return v1.registers[id] < v2.registers[id];
	}
}