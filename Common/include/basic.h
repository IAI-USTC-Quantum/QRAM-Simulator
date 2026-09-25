#pragma once

#include "typedefs.h"
#include "error_handler.h"
#include "random_engine.h"
#include "fmt/core.h"
#include "logger.h"
#include "iterable.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

//#ifdef USE_CUDA
//#include "cuda/cuda_utils.cuh"
//#endif

namespace qram_simulator {

	/// 复数模方 |c|^2
	template<typename Ty>
	constexpr auto abs_sqr(const std::complex<Ty>& c) -> Ty {
		return std::real(c) * std::real(c) + std::imag(c) * std::imag(c);
	}

	/// 取整数 n 的第 digit 位（0 为最低位）
	HOST_DEVICE constexpr bool get_digit(uint64_t n, size_t digit)
	{ 
		return (n >> digit) & 1; 
	}

	/// 取第 digit 位并按 maxdigit 反转位序（最高位变最低位）
	HOST_DEVICE	constexpr bool get_digit_reverse(uint64_t n, size_t digit, size_t maxdigit)
	{
		return (n >> (maxdigit - digit - 1)) & 1;
	}

	/// 2 的 n 次幂（n < 64）
	HOST_DEVICE	constexpr uint64_t pow2(size_t n)
	{ 
		return (static_cast<uint64_t>(1ull)) << (n);
	}

	/// width 位全 1 掩码（width=64 时为全 1）
	HOST_DEVICE constexpr uint64_t width_mask(size_t width)
	{
		return width == 64 ? ~uint64_t{0} : pow2(width) - 1;
	}

	/// 整数以 2 为底的对数（向下取整，n>0）
	constexpr size_t log2(uint64_t n) {
		size_t ret = 0;
		while (n > 1) {
			ret++;
			n /= 2;
		}
		return ret;
	}

	/// 振幅向量概率和（模方和）
	template<typename Ty>
	Ty amp_sum(const std::vector<std::complex<Ty>>& amps)
	{
		Ty val = 0;
		for (auto& a : amps) { val += abs_sqr(a); }
		return val;
	}

	/// |v| < eps 判定（数值零）
	HOST_DEVICE	constexpr bool ignorable(const double v, double eps) {
		if (v > -eps && v < eps) return true;
		else return false;
	}

	/// |v| < epsilon 判定（默认容差）
	HOST_DEVICE	constexpr bool ignorable(const double v) {
		return ignorable(v, epsilon);
	}

	/// 复数模方可忽略判定
	template<typename Ty>
	constexpr bool ignorable(const std::complex<Ty>& v) {
		Ty value = abs_sqr(v);
		if (ignorable(value)) return true;
		else return false;
	}

	/// 归一化校验：概率和偏离 1 超出容差时抛出异常
	template<typename Ty>
	void check_normalization(const std::vector<std::complex<Ty>>& amps)
	{
		double A = amp_sum(amps);
		if (!ignorable(A - 1.0))
			throw_bad_result();
	}

	/// 整数 i 的第 digit 位是否为 1
	HOST_DEVICE	constexpr inline bool digit1(uint64_t i, size_t digit)
	{
		return (i >> digit) & 1;
	}

	/// 整数 i 的第 digit 位是否为 0
	HOST_DEVICE	constexpr inline bool digit0(uint64_t i, size_t digit)
	{
		return !digit1(i, digit);
	}

	/// 翻转整数 i 的第 digit 位（XOR 掩码）
	HOST_DEVICE	constexpr uint64_t flip_digit(uint64_t i, size_t digit)
	{
		auto m = pow2(digit);
		// return digit1(i, digit) ? (i -= m) : (i += m);
		
		// bitwise XOR is ok
		return i ^ m;
	}

	/// 补码编码：负数 data 映射到 data_sz 位无符号（补码）
	constexpr uint64_t make_complement(int64_t data, size_t data_sz)
	{
		if (data_sz == 64 || data >= 0) {
			return data;
		}
		return pow2(data_sz) + data;
	}

	/// 补码解码：data_sz 位无符号还原为有符号（符号位扩展）
	HOST_DEVICE	constexpr int64_t get_complement(uint64_t data, size_t data_sz)
	{
		return data_sz ? (int64_t)(data << (64 - data_sz)) >> (64 - data_sz) : 0;
	}

	// 整数平方根（下取整）：纯整数逐位算法，无浮点参与，
	// CPU 与 CUDA 结果按位一致（docs/operators.md《宽度与截断约定》）
	HOST_DEVICE inline uint64_t isqrt_u64(uint64_t n)
	{
		uint64_t rem = 0, root = 0;
		const int digits = 32;
		for (int i = digits - 1; i >= 0; --i) {
			rem = (rem << 2) | ((n >> (2 * i)) & 3ull);
			root <<= 1;
			const uint64_t trial = (root << 1) + 1;
			if (rem >= trial) { rem -= trial; root |= 1ull; }
		}
		return root;
	}

#ifdef __CUDACC__
	__device__ __host__
	inline double atan2_cuda(double y, double x) {
		return ::atan2(y, x);
	}
#endif

	/// @brief 定点有理数编码：把 [0,1) 的 data 编为 data_sz 位定点整数。
	///
	/// 越界（data ≥ 1 或 < 0）返回 0。
	HOST_DEVICE	constexpr uint64_t get_rational(double data, size_t data_sz)
	{
		// profiler _("Common");
		if (data >= 1 || data < 0) return 0;
		uint64_t ret = 0;
		for (size_t i = 0; i < data_sz; ++i)
		{
			ret <<= 1;
			data *= 2;
			if (data >= 1)
			{
				ret += 1;
				data -= 1;
			}
		}
		return ret;
	}

	/// complex_t 的字典序比较（set/map 键用）
	bool operator<(const complex_t& lhs, const complex_t& rhs);
	/// 十进制转二进制字符串（高位补零到 size 位）
	std::string dec2bin(uint64_t n, size_t size);

	/// get_rational 的 IEEE754 位级实现（按 double 尾数截断）
	size_t get_rational_IEEE754(double data, size_t data_sz);

	/* Helper class array_length */
	/// 数组长度萃取工具骨架（array_length<T>::value 为数组长度）
	template<typename Ty = void>
	struct array_length
	{};

	template<typename DTy, int sz>
	struct array_length<std::array<DTy, sz>>
	{
		static constexpr int value = sz;
	};

	template<typename DTy, int sz>
	struct array_length<DTy[sz]>
	{
		static constexpr int value = sz;
	};

	/// 第 layerid 层的满节点下标区间 [lower, upper]（层内编号）
	constexpr std::pair<size_t, size_t> get_layer_range(size_t layerid)
	{
		size_t lower = pow2(layerid + 1) - 2;
		size_t upper = pow2(layerid + 2) - 3;
		return { lower, upper };
	}

	/// 第 layerid 层的树节点编号区间 [lower, upper]（层序编号）
	constexpr std::pair<size_t, size_t> get_layer_node_range(size_t layerid) 
	{
		size_t lower = pow2(layerid) - 1;
		size_t upper = pow2(layerid + 1) - 2;
		return { lower, upper };
	}

	template<typename T>
	struct _remove_cvref {
		using type = std::remove_cv_t<std::remove_reference_t<T>>;
	};

	template<typename T>
	using _remove_cvref_t = typename _remove_cvref<T>::type;

	/// 任意类型指针统一转 void*（map2vec 等映射辅助用）
	template<typename Ty>
	void* to_voidptr(Ty ptr)  {
		using T_ptr_t = _remove_cvref_t<Ty>;
		using T = _remove_cvref_t<std::remove_pointer_t<T_ptr_t>>;
		using clear_pointer_type = T*;
		return reinterpret_cast<void*>(const_cast<clear_pointer_type>(ptr));
	}

	/// std::map 转为 (键指针, 值指针) 的向量（供 C 接口遍历）
	template<typename KeyTy, typename ValTy>
	void map2vec(std::vector<std::pair<void*, void*>>& vec, const std::map<KeyTy, ValTy>& map1) {
		vec.clear();
		vec.reserve(map1.size());
		for (const auto& item : map1) {
			void* keyptr = to_voidptr(&(item.first));
			void* valptr = to_voidptr(&(item.second));
			vec.push_back({ keyptr, valptr });
		}
	}

	/// @brief 用指定引擎随机填充数据树。
	///
	/// 每个内存槽独立采样 U(0, 2^memory_size - 1)。
	template<typename EngineType, typename MemoryContainer>
	void random_memory(MemoryContainer& memory, size_t memory_size, EngineType& engine) {
		size_t size = memory.size();
		std::uniform_int_distribution<memory_entry_t> ud(0, pow2(memory_size) - 1);

		for (auto iter = std::begin(memory); iter != std::end(memory); ++iter)
		{
			*iter = ud(engine);
		}
	}

	/// 用全局随机引擎随机填充数据树（size_t 槽版本）
	inline void random_memory(std::vector<size_t>& memory, size_t memory_size) {
		random_memory(memory, memory_size, random_engine::get_engine());
	}

	/// 相邻去重并合并：pred 判定相邻等值时用 fn 合并到前者
	template <typename FwdIt, typename Pred, typename Func>
	FwdIt unique_and_merge(FwdIt first, FwdIt last, Pred pred, Func fn)
	{
		if (first == last) return last;

		FwdIt result = first;
		while (++first != last)
		{
			if (!pred(*result, *first))
				*(++result) = *first;
			else
				fn(*result, *first);
		}
		return ++result;
	}

	/// 排序 → 相邻等值合并 → 按谓词移除 → 物理删除 的复合清理流程
	template<typename ContainerTy, typename PredLt, typename PredEq, typename MergeFn, typename EraseFn>
	void sort_merge_unique_erase(ContainerTy& vec, PredLt lt, PredEq eqn, MergeFn fn, EraseFn erase)
	{
		std::sort(vec.begin(), vec.end(), lt);
		auto iter = unique_and_merge(vec.begin(), vec.end(), eqn, fn);
		iter = std::remove_if(vec.begin(), iter, erase);
		vec.erase(iter, vec.end());
	}

	/// 按谓词删除 map 中满足条件的元素（C++20 std::erase_if 的 C++17 替代）
	template< class Key, class T, class Compare, class Alloc, class Pred >
	void erase_if(std::map<Key, T, Compare, Alloc>& c, Pred pred) {
		for (auto i = c.begin(), last = c.end(); i != last; ) {
			if (pred(*i)) {
				i = c.erase(i);
			}
			else {
				++i;
			}
		}
	}

	/// 从 [0, 2^size) 无放回采样 n_samples 个互异下标到 samples
	template<typename Rng>
	void choice_from(std::set<size_t>& samples, int size, size_t n_samples, Rng& g)
	{
		samples.clear();
		std::uniform_int_distribution<size_t> ud(0, 1ull << size);
		while (n_samples > 0) {
			if (samples.insert(ud(g)).second) { n_samples--; };
		}
	}

	/// 等距采样：[min, max] 上生成 points 个数（含端点）
	inline std::vector<double> linspace(double min, double max, size_t points) {
		double delta = (max - min) / (points - 1);
		std::vector<double> ret;
		ret.reserve(points);
		for (size_t i = 0; i < points; ++i) {
			ret.push_back(min + delta * i);
		}
		return ret;
	}

	/// 样本均值与标准差（无偏性未修正，按总体方差计算）
	inline std::pair<double, double> mean_std(const std::vector<double>& m) {
		auto sq = [](double m, double y) {
			return m + y * y;
		};

		double sum = std::accumulate(m.begin(), m.end(), 0.0);
		double sumsq = std::accumulate(m.begin(), m.end(), 0.0, sq);
		double mean = sum / m.size();
		double meansq = sumsq / m.size();
		return { mean, sqrt(meansq - mean * mean) };

	}

	/// 复数转 "a±bj" 字符串
	inline std::string complex2str(const std::complex<double>& x)
	{
		if (x.imag() > 0)
			return fmt::format("{}+{}j", x.real(), x.imag());
		else
			return fmt::format("{}-{}j", x.real(), -x.imag());
	}

	/// 复数向量转 "[a±bj, …]" 字符串
	inline std::string complex2str(const std::vector<complex_t>& vec)
	{
		std::string ret = "[";
		for (auto& x : vec)
		{
			if (x.imag() > 0)
				fmt::format_to(std::back_inserter(ret), " {}+{}j", x.real(), x.imag());
			else
				fmt::format_to(std::back_inserter(ret), " {}-{}j", x.real(), -x.imag());
		}
		ret += "]";
		return ret;
	}


	/// 向量转 "[v1,v2,…]" 字符串（可自定义括号与分隔符）
	template<typename Ty>
	std::string vec2str(const std::vector<Ty>& v, std::string lb = "[", std::string rb = "]", std::string sep = ",")
	{
		if (v.size() == 0) {
			return lb + rb;
		}
		std::stringstream ret;
		for (size_t i = 0; i < v.size() - 1; ++i) {
			ret << v[i] << sep;
		}
		ret << v.back();
		return lb + ret.str() + rb;
	}

	/// 数字转字符串（std::to_string 包装）
	template<typename Ty>
	std::string num2str(Ty num) {
		return std::to_string(num);
	}

	/// 64 位整数置 1 位计数（popcount）
	inline size_t bitcount(uint64_t n)
	{
#if defined(_MSC_VER)
		return __popcnt64(n);
#elif defined(__GNUC__) || defined(__clang__)
		return __builtin_popcountll(n);
#else
		size_t count = 0;
		while (n) {
			count++;
			n &= (n - 1);
		}
		return count;
#endif
	}

	/// 32 位整数置 1 位计数（popcount）
	inline size_t bitcount(uint32_t n)
	{
#if defined(_MSC_VER)
		return __popcnt(n);
#elif defined(__GNUC__) || defined(__clang__)
		return __builtin_popcount(n);
#else
		size_t count = 0;
		while (n) {
			count++;
			n &= (n - 1);
		}
		return count;
#endif
	}

	/// 64 位整数置 1 位奇偶性
	inline bool bit_parity(uint64_t n)
	{
#if defined(_MSC_VER)
		return __popcnt64(n) & 1;
#elif defined(__GNUC__) || defined(__clang__)
		return __builtin_parityll(n);
#else
		return bitcount(n) & 1;
#endif
	}

	/// 32 位整数置 1 位奇偶性
	inline bool bit_parity(uint32_t n)
	{
#if defined(_MSC_VER)
		return __popcnt(n) & 1;
#elif defined(__GNUC__) || defined(__clang__)
		return __builtin_parity(n);
#else
		return bitcount(n) & 1;
#endif
	}

	/// 态向量范数平方（概率和）
	inline double norm2(const std::vector<complex_t>& state)
	{
		double sum = 0;
		for (auto& s : state)
		{
			sum += abs_sqr(s);
		}
		return sum;
	}

	/// @brief 态向量对目标态的内积保真度 |<state|target>|。
	///
	/// 长度不一致抛出运行时异常；空向量返回 0。
	template<typename Ty>
	inline double get_fidelity(const std::vector<Ty>& state,
		const std::vector<complex_t>& target)
	{
		if (state.size() == 0)
			return 0;
		if (state.size() != target.size())
		{
			throw_general_runtime_error(
				fmt::format("Error: Vectors must be of the same size! size1 = {}, size2 = {}",
					state.size(), target.size()
				)
			);
		}

		complex_t sum = 0;
		for (size_t i = 0; i < state.size(); i++)
		{
			sum += state[i] * std::conj(target[i]);
		}

		return std::abs(sum);
	}

	/// 实向量对目标向量的重叠 |<state|target>|（经典分布版本）
	inline double get_fidelity(const std::vector<double>& state,
		const std::vector<double>& target)
	{
		if (state.size() == 0)
			return 0;
		if (state.size() != target.size())
			throw_general_runtime_error("Error: Vectors must be of the same size!");

		complex_t sum = 0;
		for (size_t i = 0; i < state.size(); i++)
		{
			sum += state[i] * target[i];
		}

		return std::abs(sum);
	}

	/// 校验容器严格递增且无重复（有序唯一性检查）
	template<typename ContainerTy>
	bool check_unique_sort(const ContainerTy& cont)
	{
		if (std::size(cont) <= 1)
			return true;

		auto iter = std::next(std::begin(cont));
		for (; iter != std::end(cont); ++iter)
		{
			if (*iter < *(std::prev(iter)))
				return false;
			if (*iter == *(std::prev(iter)))
				return false;
		}
		return true;
	}

	/// 校验迭代器区间严格递增且无重复（有序唯一性检查）
	template<typename Iter>
	bool check_unique_sort(Iter beg, Iter end)
	{
		if (std::distance(beg, end) <= 1)
			return true;

		auto iter = std::next(beg);
		for (; iter != end; ++iter)
		{
			if (*iter < *(std::prev(iter)))
				return false;
			if (*iter == *(std::prev(iter)))
				return false;
		}
		return true;
	}

	/* Concatenate value sequentially by its value and length(size)
	* Low to High
	*/
	/// @brief 把 (值, 位宽) 序列按低位到高位拼接为单一整数。
	///
	/// 如 [(5,3),(1,2)] → 5 | (1<<3) = 13。
	size_t concat_value(const std::vector<std::pair<size_t, size_t>>& values);

}

template <> struct fmt::formatter<std::complex<double>> {
	// Presentation format: 'f' - fixed, 'e' - exponential.
	char presentation = 'f';

	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
		// [ctx.begin(), ctx.end()) is a character range that contains a part of
		// the format string starting from the format specifications to be parsed,
		// e.g. in
		//
		//   fmt::format("{:f} - point of interest", point{1, 2});
		//
		// the range will contain "f} - point of interest". The formatter should
		// parse specifiers until '}' or the end of the range. In this example
		// the formatter should parse the 'f' specifier and return an iterator
		// pointing to '}'.

		// Parse the presentation format and store it in the formatter:
		auto it = ctx.begin(), end = ctx.end();
		if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

		// Check if reached the end of the range:
		if (it != end && *it != '}') throw format_error("invalid format");

		// Return an iterator past the end of the parsed range:
		return it;
	}

	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(const std::complex<double>& p, FormatContext& ctx) const -> decltype(ctx.out()) {
		// ctx.out() is an output iterator to write to.
		if (p.imag() >= 0)
			return presentation == 'f'
			? format_to(ctx.out(), "{:f}+{:f}i", p.real(), p.imag())
			: format_to(ctx.out(), "{:e}+{:e}i", p.real(), p.imag());
		else
			return presentation == 'f'
			? format_to(ctx.out(), "{:f}{:f}i", p.real(), p.imag())
			: format_to(ctx.out(), "{:e}{:e}i", p.real(), p.imag());
	}
};

namespace std {

	template<size_t sz>
	inline bool operator<(const std::bitset<sz>& lhs, const std::bitset<sz>& rhs)
	{
		return lhs.to_ullong() < rhs.to_ullong();
	}

	template<typename Ty>
	std::vector<Ty> operator+(const std::vector<Ty>& lhs, const std::vector<Ty>& rhs)
	{
		std::vector<Ty> ret;
		if (lhs.size() == rhs.size())
		{
			ret.resize(lhs.size());
			for (size_t i = 0; i < lhs.size(); ++i)
			{
				ret[i] = lhs[i] + rhs[i];
			}
		}
		return lhs;
	}

	template<typename Ty>
	std::vector<Ty>& operator+=(std::vector<Ty>& lhs, const std::vector<Ty>& rhs)
	{
		if (lhs.size() == rhs.size())
			for (size_t i = 0; i < lhs.size(); ++i)
			{
				lhs[i] += rhs[i];
			}
		return lhs;
	}
}
