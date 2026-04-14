#include "basic.h"
#include <cstring>
#include "error_handler.h"

namespace qram_simulator {

	bool operator<(const complex_t& lhs, const complex_t& rhs)
	{
		return (abs_sqr(lhs) < abs_sqr(rhs));
	}

	std::string dec2bin(uint64_t n, size_t size)
	{
		if (n >= pow2(size))
			throw_invalid_input();

		std::string binstr = "";
		for (size_t i = 0; i < size; ++i) {
			binstr = (char)((n & 1) + '0') + binstr;
			n >>= 1;
		}
		return binstr;
	}

	uint64_t get_rational_IEEE754(double data, size_t data_sz)
	{
		// profiler _("IEEE");
		if (data >= 1 || data < 0) return 0;
		uint64_t idata;
		std::memcpy(&idata, &data, sizeof(idata));
		uint64_t bias = 1022 - (idata >> 52);
		data_sz -= 1;
		if (bias > data_sz) return 0;
		if (bias == data_sz) return 1;
		data_sz -= bias;
		return ((idata << 12) >> (64 - data_sz)) + pow2(data_sz);
	}

	size_t concat_value(const std::vector<std::pair<size_t, size_t>>& values)
	{
		size_t current_pos = 0;
		size_t ret = 0; // return value
		for (auto&& [value, size] : values)
		{
			// check if the value is out of range
			if (value >= pow2(size))
			{
				throw_general_runtime_error("Error: Value is out of range!");
			}
			ret += (value << current_pos);
			current_pos += size;

			// check if current position is out of range
			if (current_pos >= 64)
			{
				throw_general_runtime_error("Error: Concatenation is out of range!");
			}
		}
		return ret;
	}

}