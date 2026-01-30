#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <bitset>
#include <chrono>
#include <cmath>
#include <complex>
#include <ctime>
#include <deque>
#include <execution>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <fmt/ostream.h>

namespace qram_simulator{
	
	/* typedefs */
	using complex_t = std::complex<double>;
	using memory_entry_t = uint64_t;
	using memory_t = std::vector<memory_entry_t>;
	using bus_t = uint64_t;
	// using u22_t = std::array<std::complex<double>, 4>;


#ifdef __CUDACC__
#define HOST_DEVICE __host__ __device__
#else
#define HOST_DEVICE
#endif

	HOST_DEVICE inline complex_t cu_conj(complex_t x)
	{
		return complex_t(std::real(x), -std::imag(x));
	}

	struct u22_t
	{
		using data_type = std::array<std::complex<double>, 4>;
		data_type m_data;

		HOST_DEVICE u22_t() noexcept : m_data{ 0 } {}

		HOST_DEVICE	u22_t(std::complex<double> a, std::complex<double> b, 
			std::complex<double> c, std::complex<double> d) noexcept
			: m_data{ a, b, c, d } {}

		HOST_DEVICE	u22_t(const std::array<std::complex<double>, 4>& arr) noexcept 
			: m_data(arr) {}

		HOST_DEVICE u22_t(std::array<std::complex<double>, 4>&& arr) noexcept 
			: m_data(arr) {}

		HOST_DEVICE	u22_t(const u22_t& other) noexcept 
			: m_data(other.m_data) {}

		HOST_DEVICE	u22_t(u22_t&& other) noexcept 
			: m_data(other.m_data) {}

		HOST_DEVICE	u22_t& operator=(const u22_t& other) noexcept {
			m_data = other.m_data;
			return *this;
		}

		HOST_DEVICE	u22_t& operator=(u22_t&& other) noexcept {
			m_data = other.m_data;
			return *this;
		}

		HOST_DEVICE	data_type::iterator begin() noexcept {
			return m_data.begin();
		}

		HOST_DEVICE	data_type::const_iterator begin() const noexcept {
			return m_data.begin();
		}

		HOST_DEVICE	data_type::iterator end() noexcept {
			return m_data.end();
		}

		HOST_DEVICE	data_type::const_iterator end() const noexcept {
			return m_data.end();
		}

		HOST_DEVICE	data_type::reverse_iterator rbegin() noexcept {
			return m_data.rbegin();
		}

		HOST_DEVICE	data_type::const_reverse_iterator rbegin() const noexcept {
			return m_data.rbegin();
		}

		HOST_DEVICE	data_type::reference operator[](size_t index) noexcept {
			return m_data[index];
		}

		HOST_DEVICE	data_type::const_reference operator[](size_t index) const noexcept {
			return m_data[index];
		}

		HOST_DEVICE	data_type::value_type* data() noexcept
		{
			return m_data.data();
		}

		HOST_DEVICE	u22_t dagger() const noexcept {
			return u22_t(cu_conj(m_data[0]), cu_conj(m_data[2]), cu_conj(m_data[1]), cu_conj(m_data[3]));
		}

	};

	/* math constant */
	constexpr double pi = 3.141592653589793238462643383279502884L;
	constexpr double sqrt2 = 1.41421356237309504880168872420969807856967L;
	constexpr double epsilon = 1.e-14;
	constexpr double sqrt2inv = 1.0 / sqrt2;

	constexpr int W = -1;
	constexpr int L = 0;
	constexpr int R = 1;
	static const complex_t w = { std::cos(pi * 2 / 3), std::sin(pi * 2 / 3) };
	static const complex_t w2 = w * w;

	enum StateStorageType
	{
		General,
		UnsignedInteger,
		SignedInteger,
		Boolean,
		Rational,
	};

	inline const char* get_type_str(StateStorageType type) {
		static const char* typestr[] = {
			"Reg",
			"UInt",
			"Int",
			"Bool",
			"Rat"
		};
		return typestr[type];
	}

}

template <>
struct fmt::formatter<qram_simulator::StateStorageType> {
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(qram_simulator::StateStorageType type, FormatContext& ctx) const -> decltype(ctx.out()) {
		const char* type_str = get_type_str(type);
		return fmt::format_to(ctx.out(), "{}", type_str);
	}
};

#ifdef _MSC_VER // Visual Studio
#pragma warning(disable : 4819 4996 4018)
#elif defined(__GNUC__) || defined(__clang__) // GCC/Clang
// suppress linux warnings
#endif