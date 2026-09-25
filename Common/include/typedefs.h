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
	/// 复数振幅类型（双精度）
	using complex_t = std::complex<double>;
	/// 单条内存槽的存储类型（无符号 64 位）
	using memory_entry_t = uint64_t;
	/// 数据树类型：memory[i] 为地址 i 处存放的整数
	using memory_t = std::vector<memory_entry_t>;
	/// 数据总线类型（无符号 64 位）
	using bus_t = uint64_t;
	// using u22_t = std::array<std::complex<double>, 4>;


	#ifdef __CUDACC__
	#define HOST_DEVICE __host__ __device__
	#else
	#define HOST_DEVICE
	#endif

	/// 复数共轭（CUDA 兼容实现）
	HOST_DEVICE inline complex_t cu_conj(complex_t x)
	{
		return complex_t(std::real(x), -std::imag(x));
	}

	/**
	 * @brief 2×2 酉矩阵（行主序四元素）。
	 *
	 * 为保证 CPU 与 CUDA 按位一致而采用自定义结构（避免 std::array
	 * 在 device 上的差异），提供迭代器、下标访问与 dagger（共轭转置）。
	 */
	struct u22_t
	{
		using data_type = std::array<std::complex<double>, 4>;
		data_type m_data;

		HOST_DEVICE u22_t() noexcept : m_data{ 0 } {}

		/// @brief 以四个元素（行主序 a,b,c,d）构造。
		HOST_DEVICE	u22_t(std::complex<double> a, std::complex<double> b,
			std::complex<double> c, std::complex<double> d) noexcept
			: m_data{ a, b, c, d } {}

		/// 以数组构造（拷贝）
		HOST_DEVICE	u22_t(const std::array<std::complex<double>, 4>& arr) noexcept
			: m_data(arr) {}

		/// 以数组构造（移动）
		HOST_DEVICE	u22_t(std::array<std::complex<double>, 4>&& arr) noexcept
			: m_data(arr) {}

		/// 拷贝构造
		HOST_DEVICE	u22_t(const u22_t& other) noexcept
			: m_data(other.m_data) {}

		/// 移动构造
		HOST_DEVICE	u22_t(u22_t&& other) noexcept
			: m_data(other.m_data) {}

		/// 拷贝赋值
		HOST_DEVICE	u22_t& operator=(const u22_t& other) noexcept {
			m_data = other.m_data;
			return *this;
		}

		/// 移动赋值
		HOST_DEVICE	u22_t& operator=(u22_t&& other) noexcept {
			m_data = other.m_data;
			return *this;
		}

		/// 起始迭代器
		HOST_DEVICE	data_type::iterator begin() noexcept {
			return m_data.begin();
		}

		/// 起始常量迭代器
		HOST_DEVICE	data_type::const_iterator begin() const noexcept {
			return m_data.begin();
		}

		/// 终止迭代器
		HOST_DEVICE	data_type::iterator end() noexcept {
			return m_data.end();
		}

		/// 终止常量迭代器
		HOST_DEVICE	data_type::const_iterator end() const noexcept {
			return m_data.end();
		}

		/// 反向起始迭代器
		HOST_DEVICE	data_type::reverse_iterator rbegin() noexcept {
			return m_data.rbegin();
		}

		/// 反向起始常量迭代器
		HOST_DEVICE	data_type::const_reverse_iterator rbegin() const noexcept {
			return m_data.rbegin();
		}

		/// 下标访问
		HOST_DEVICE	data_type::reference operator[](size_t index) noexcept {
			return m_data[index];
		}

		/// 下标常量访问
		HOST_DEVICE	data_type::const_reference operator[](size_t index) const noexcept {
			return m_data[index];
		}

		/// 裸数据指针
		HOST_DEVICE	data_type::value_type* data() noexcept
		{
			return m_data.data();
		}

		/// 共轭转置
		HOST_DEVICE	u22_t dagger() const noexcept {
			return u22_t(cu_conj(m_data[0]), cu_conj(m_data[2]), cu_conj(m_data[1]), cu_conj(m_data[3]));
		}

	};

	/* math constant */
	/// 圆周率 π
	constexpr double pi = 3.141592653589793238462643383279502884L;
	/// √2
	constexpr double sqrt2 = 1.41421356237309504880168872420969807856967L;
	/// 数值零容差（判断可忽略的小量）
	constexpr double epsilon = 1.e-14;
	/// 1/√2
	constexpr double sqrt2inv = 1.0 / sqrt2;

	/// qutrit 地址能级：W（总线 / 等待）
	constexpr int W = -1;
	/// qutrit 地址能级：L（左）
	constexpr int L = 0;
	/// qutrit 地址能级：R（右）
	constexpr int R = 1;
	/// 三次本源单位根 w = e^{2πi/3}
	static const complex_t w = { std::cos(pi * 2 / 3), std::sin(pi * 2 / 3) };
	/// w 的平方 w^2 = e^{4πi/3}
	static const complex_t w2 = w * w;

	/**
	 * @brief 量子寄存器的存储类型（SparQ 框架遗留，量子算术使用）。
	 */
	enum StateStorageType
	{
		General,        ///< 一般量子寄存器
		UnsignedInteger,///< 无符号整数
		SignedInteger,  ///< 有符号整数（补码）
		Boolean,        ///< 布尔
		Rational,       ///< 定点有理数
	};

	/// 存储类型的缩写名（"Reg"/"UInt"/"Int"/"Bool"/"Rat"）
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

/// StateStorageType 的 fmt 格式化特化（输出缩写名）
template <>
struct fmt::formatter<qram_simulator::StateStorageType> {
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(qram_simulator::StateStorageType type, FormatContext& ctx) const -> decltype(ctx.out()) {
		const char* type_str = qram_simulator::get_type_str(type);
		return fmt::format_to(ctx.out(), "{}", type_str);
	}
};

#ifdef _MSC_VER // Visual Studio
#pragma warning(disable : 4819 4996 4018)
#elif defined(__GNUC__) || defined(__clang__) // GCC/Clang
// suppress linux warnings
#endif
