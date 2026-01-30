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

	template<typename Ty>
	constexpr auto abs_sqr(const std::complex<Ty>& c) -> Ty {
		return std::real(c) * std::real(c) + std::imag(c) * std::imag(c);
	}

	HOST_DEVICE constexpr bool get_digit(uint64_t n, size_t digit) 
	{ 
		return (n >> digit) & 1; 
	}

	HOST_DEVICE	constexpr bool get_digit_reverse(uint64_t n, size_t digit, size_t maxdigit)
	{
		return (n >> (maxdigit - digit - 1)) & 1;
	}

	HOST_DEVICE	constexpr uint64_t pow2(size_t n) 
	{ 
		return (static_cast<uint64_t>(1ull)) << (n);
	}

	constexpr size_t log2(uint64_t n) {
		size_t ret = 0;
		while (n > 1) {
			ret++;
			n /= 2;
		}
		return ret;
	}

	template<typename Ty>
	Ty amp_sum(const std::vector<std::complex<Ty>>& amps)
	{
		Ty val = 0;
		for (auto& a : amps) { val += abs_sqr(a); }
		return val;
	}

	HOST_DEVICE	constexpr bool ignorable(const double v, double eps) {
		if (v > -eps && v < eps) return true;
		else return false;
	}

	HOST_DEVICE	constexpr bool ignorable(const double v) {
		return ignorable(v, epsilon);
	}

	template<typename Ty>
	constexpr bool ignorable(const std::complex<Ty>& v) {
		Ty value = abs_sqr(v);
		if (ignorable(value)) return true;
		else return false;
	}

	template<typename Ty>
	void check_normalization(const std::vector<std::complex<Ty>>& amps)
	{
		double A = amp_sum(amps);
		if (!ignorable(A - 1.0))
			throw_bad_result();
	}

	HOST_DEVICE	constexpr inline bool digit1(uint64_t i, size_t digit)
	{
		return (i >> digit) & 1;
	}

	HOST_DEVICE	constexpr inline bool digit0(uint64_t i, size_t digit)
	{
		return !digit1(i, digit);
	}

	HOST_DEVICE	constexpr uint64_t flip_digit(uint64_t i, size_t digit)
	{
		auto m = pow2(digit);
		// return digit1(i, digit) ? (i -= m) : (i += m);
		
		// bitwise XOR is ok
		return i ^ m;
	}

	constexpr uint64_t make_complement(int64_t data, size_t data_sz)
	{
		if (data_sz == 64 || data >= 0) {
			return data;
		}
		return pow2(data_sz) + data;
	}

	HOST_DEVICE	constexpr int64_t get_complement(uint64_t data, size_t data_sz)
	{
		return data_sz ? (int64_t)(data << (64 - data_sz)) >> (64 - data_sz) : 0;
	}

#ifdef __CUDACC__
	__device__ __host__
	inline double atan2_cuda(double y, double x) {
		return ::atan2(y, x);
	}
#endif

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

	bool operator<(const complex_t& lhs, const complex_t& rhs);
	std::string dec2bin(uint64_t n, size_t size);

	size_t get_rational_IEEE754(double data, size_t data_sz);

	/* Helper class array_length */
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

	constexpr std::pair<size_t, size_t> get_layer_range(size_t layerid) 
	{
		size_t lower = pow2(layerid + 1) - 2;
		size_t upper = pow2(layerid + 2) - 3;
		return { lower, upper };
	}

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

	template<typename Ty>
	void* to_voidptr(Ty ptr)  {
		using T_ptr_t = _remove_cvref_t<Ty>;
		using T = _remove_cvref_t<std::remove_pointer_t<T_ptr_t>>;
		using clear_pointer_type = T*;
		return reinterpret_cast<void*>(const_cast<clear_pointer_type>(ptr));
	}

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

	template<typename EngineType, typename MemoryContainer>
	void random_memory(MemoryContainer& memory, size_t memory_size, EngineType& engine) {
		size_t size = memory.size();
		std::uniform_int_distribution<memory_entry_t> ud(0, pow2(memory_size) - 1);

		for (auto iter = std::begin(memory); iter != std::end(memory); ++iter)
		{
			*iter = ud(engine);
		}
	}

	inline void random_memory(std::vector<size_t>& memory, size_t memory_size) {
		random_memory(memory, memory_size, random_engine::get_engine());
	}

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

	template<typename ContainerTy, typename PredLt, typename PredEq, typename MergeFn, typename EraseFn>
	void sort_merge_unique_erase(ContainerTy& vec, PredLt lt, PredEq eqn, MergeFn fn, EraseFn erase)
	{
		std::sort(vec.begin(), vec.end(), lt);
		auto iter = unique_and_merge(vec.begin(), vec.end(), eqn, fn);
		iter = std::remove_if(vec.begin(), iter, erase);
		vec.erase(iter, vec.end());
	}

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

	template<typename Rng>
	void choice_from(std::set<size_t>& samples, int size, size_t n_samples, Rng& g)
	{
		samples.clear();
		std::uniform_int_distribution<size_t> ud(0, 1ull << size);
		while (n_samples > 0) {
			if (samples.insert(ud(g)).second) { n_samples--; };
		}
	}

	inline std::vector<double> linspace(double min, double max, size_t points) {
		double delta = (max - min) / (points - 1);
		std::vector<double> ret;
		ret.reserve(points);
		for (size_t i = 0; i < points; ++i) {
			ret.push_back(min + delta * i);
		}
		return ret;
	}

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

	inline std::string complex2str(const std::complex<double>& x)
	{
		if (x.imag() > 0)
			return fmt::format("{}+{}j", x.real(), x.imag());
		else
			return fmt::format("{}-{}j", x.real(), -x.imag());
	}

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

	template<typename Ty>
	std::string num2str(Ty num) {
		return std::to_string(num);
	}

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

	inline double norm2(const std::vector<complex_t>& state)
	{
		double sum = 0;
		for (auto& s : state)
		{
			sum += abs_sqr(s);
		}
		return sum;
	}

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
