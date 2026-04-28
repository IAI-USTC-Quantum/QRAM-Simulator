/**
 * @file debugger.h
 * @brief 调试工具定义
 * @details 提供量子态的调试、验证和检查工具，包括归一化检查、
 *          NaN 检查、状态打印、块编码提取等功能
 */

#pragma once
#include <functional>
#include <sstream>
#include "basic_components.h"
#include "matrix.h"
#include "partial_trace.h"
#include "sort_state.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 模块继承测试类
	 * @details 用于测试 BaseOperator 的继承机制
	 */
	struct ModuleInheritance_Test : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/**
		 * @brief 应用测试操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			fmt::print("ModuleInheritance_Test::operator()\n");
		}

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用测试操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 自伴模块继承测试类
	 * @details 用于测试 SelfAdjointOperator 的继承机制
	 */
	struct ModuleInheritance_Test_SelfAdjoint : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 应用测试操作
		 * @param state 系统状态向量
		 */
		inline void operator()(std::vector<System>& state) const
		{
			fmt::print("ModuleInheritance_Test_SelfAdjoint::operator()\n");
		}

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用测试操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 归一化检查类
	 * @details 检查量子态是否归一化（总概率为1）
	 */
	struct CheckNormalization : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 归一化检查阈值 */
		double threshold = 1e-5;

		/**
		 * @brief 默认构造函数（使用默认阈值）
		 */
		CheckNormalization();

		/**
		 * @brief 构造函数（指定阈值）
		 * @param threshold_ 检查阈值
		 */
		CheckNormalization(double threshold_) : threshold(threshold_) {}

		/**
		 * @brief 应用归一化检查
		 * @param state 系统状态向量
		 * @throws 当不归一化时抛出异常
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用归一化检查
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 归一化检查与重归一化类
	 * @details 检查量子态归一化，如不归一则重新归一化
	 */
	struct CheckNormalization_Renormalize : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 归一化检查阈值 */
		double threshold = 1e-5;

		/**
		 * @brief 默认构造函数
		 */
		CheckNormalization_Renormalize() {}

		/**
		 * @brief 构造函数（指定阈值）
		 * @param threshold_ 检查阈值
		 */
		CheckNormalization_Renormalize(double threshold_) : threshold(threshold_) {}

		/**
		 * @brief 应用检查与重归一化
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用检查与重归一化
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief NaN 检查类
	 * @details 检查量子态中是否存在 NaN 值
	 */
	struct CheckNan : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 默认构造函数
		 */
		CheckNan();

		/**
		 * @brief 应用 NaN 检查
		 * @param state 系统状态向量
		 * @throws 当发现 NaN 时抛出异常
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用 NaN 检查
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 归一化查看类
	 * @details 打印量子态的归一化信息
	 */
	struct ViewNormalization : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 默认构造函数
		 */
		ViewNormalization();

		/**
		 * @brief 应用归一化查看
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用归一化查看
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 状态打印显示模式枚举
	 */
	enum StatePrintDisplay : int32_t
	{
		Default = 0,   ///< 默认显示模式
		Detail = 1,    ///< 详细显示模式
		Binary = 2,    ///< 二进制显示模式
		Prob = 4,      ///< 概率显示模式
	};

	/**
	 * @brief 状态打印类
	 * @details 打印量子态信息到标准输出
	 */
	struct StatePrint : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 打印开关 */
		static bool on;

		/** @brief 显示模式 */
		int32_t display;

		/** @brief 精度 */
		int precision;

		/**
		 * @brief 构造函数（指定显示模式）
		 * @param disp 显示模式
		 */
		StatePrint(int32_t disp = 0) : display(disp), precision(0) {}

		/**
		 * @brief 构造函数（指定显示模式和精度）
		 * @param disp 显示模式
		 * @param precision 精度
		 */
		StatePrint(int32_t disp, int precision) : display(disp), precision(precision) {}

		/**
		 * @brief 构造函数（枚举显示模式）
		 * @param disp 显示模式枚举
		 */
		StatePrint(StatePrintDisplay disp) : display(static_cast<int32_t>(disp)), precision(0) {}

		/**
		 * @brief 将显示模式转换为字符串
		 * @return 显示模式字符串
		 */
		std::string disp2str() const;

		/**
		 * @brief 应用状态打印，返回格式化字符串
		 * @param state 系统状态向量
		 * @return 格式化状态字符串
		 */
		std::string to_string(std::vector<System>& state) const;

		/**
		 * @brief 应用状态打印（打印到标准输出）
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用状态打印，返回格式化字符串
		 * @param state CUDA 稀疏状态
		 * @return 格式化状态字符串
		 */
		std::string to_string(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用状态打印（打印到标准输出）
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 可移除性测试类
	 * @details 测试指定寄存器是否可以安全移除
	 */
	struct TestRemovable : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID */
		size_t register_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param register_name 寄存器名称
		 */
		TestRemovable(std::string_view register_name);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param register_name 寄存器 ID
		 */
		TestRemovable(size_t register_name);

		/**
		 * @brief 应用可移除性测试
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用可移除性测试
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 重复键检查类
	 * @details 检查量子态中是否存在重复的系统键
	 */
	struct CheckDuplicateKey : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 默认构造函数
		 */
		CheckDuplicateKey() {}

		/**
		 * @brief 检查是否存在重复键
		 * @param system_states 系统状态向量
		 * @return 是否存在重复键
		 */
		bool has_duplicate(std::vector<System>& system_states) const;

		/**
		 * @brief 应用重复键检查
		 * @param system_states 系统状态向量
		 * @throws 当发现重复键时抛出异常
		 */
		void operator()(std::vector<System>& system_states) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用重复键检查
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 检查原地操作的幺正性（通用版本）
	 * @details 对任意寄存器布局的原地操作进行幺正性验证：
	 *          1. 遍历所有 2^total_bits 个输入态
	 *          2. 执行 round-trip 测试：U then U† (或 U† then U)
	 *          3. 验证状态未分裂（state.size() == 1）
	 *          4. 验证 round-trip 恢复到原始值（U*U† = I）
	 *          5. 验证 bijectivity（无输出碰撞）
	 * @tparam Op 操作类型（必须继承 BaseOperator）
	 * @param reg_sizes 每个寄存器的大小（initializer_list），建议总位数 ≤ 16
	 * @param op_factory 工厂函数：std::vector<size_t>(reg_ids) → Op
	 * @param dagger 为 false 时执行 U then U†；为 true 时执行 U† then U
	 * @return 真值表（输入索引 → 输出索引）
	 * @throws 当操作非幺正（状态分裂）、非双射（输出碰撞）或 round-trip 失败时抛出异常
	 */
	template <typename Op>
	std::vector<size_t> check_inplace_unitarity(
		std::initializer_list<size_t> reg_sizes,
		std::function<Op(std::vector<size_t>)> op_factory,
		bool dagger = false)
	{
		System::clear();

		std::vector<size_t> reg_ids;
		std::vector<size_t> sizes_vec;
		std::stringstream sbuf;
		for (size_t i = 0; i < reg_sizes.size(); ++i) {
			sbuf.str(""); sbuf << "_qram_tmp_" << i;
			size_t sz = *(reg_sizes.begin() + i);
			size_t id = System::add_register(sbuf.str(), UnsignedInteger, sz);
			reg_ids.push_back(id);
			sizes_vec.push_back(sz);
		}

		size_t total_bits = 0;
		for (size_t sz : sizes_vec) total_bits += sz;
		if (total_bits > 16)
			fmt::print("WARNING: check_inplace_unitarity with {} total bits\n", total_bits);

		Op op = op_factory(reg_ids);
		const size_t N = pow2(total_bits);

		std::vector<bool>   output_seen(N, false);
		std::vector<size_t> truth_table(N, 0);

		for (size_t input = 0; input < N; ++input) {
			std::vector<System> st;
			st.emplace_back();

			size_t remaining = input;
			for (size_t ri = 0; ri < reg_ids.size(); ++ri) {
				st[0].get(reg_ids[ri]).value = remaining & (pow2(sizes_vec[ri]) - 1);
				remaining >>= sizes_vec[ri];
			}

			if (dagger) {
				op.dag(st);
				op(st);
			} else {
				op(st);
				op.dag(st);
			}

			if (st.size() != 1)
				throw_bad_result("check_inplace_unitarity: state split (non-unitary)");

			size_t roundtrip = 0, shift = 0;
			for (size_t ri = 0; ri < reg_ids.size(); ++ri) {
				roundtrip |= (st[0].get(reg_ids[ri]).value & (pow2(sizes_vec[ri]) - 1)) << shift;
				shift += sizes_vec[ri];
			}
			if (roundtrip != input)
				throw_bad_result("check_inplace_unitarity: U*U† ≠ I");

			size_t output = 0; shift = 0;
			for (size_t ri = 0; ri < reg_ids.size(); ++ri) {
				output |= (st[0].get(reg_ids[ri]).value & (pow2(sizes_vec[ri]) - 1)) << shift;
				shift += sizes_vec[ri];
			}
			if (output_seen[output])
				throw_bad_result("check_inplace_unitarity: non-bijective output collision");
			output_seen[output] = true;
			truth_table[input] = output;
		}

		System::clear();
		return truth_table;
	}

	/**
	 * @brief 提取块编码矩阵（内部实现）
	 * @tparam BlockEncoding 块编码类型
	 * @tparam StateType 状态类型
	 * @param encA 块编码对象
	 * @param main_reg 主寄存器名称
	 * @param anc_UA 辅助寄存器名称
	 * @param is_full 是否完整提取
	 * @param is_dag 是否为 dagger 版本
	 * @return 块编码的稠密矩阵表示
	 */
	template<typename BlockEncoding, typename StateType = SparseState>
	DenseMatrix<complex_t> _extract_block_encoding(BlockEncoding encA, std::string_view main_reg, std::string_view anc_UA,
		bool is_full = false, bool is_dag = false)
	{
		size_t main_reg_num = System::size_of(main_reg);
		size_t anc_UA_num = System::size_of(anc_UA);
		size_t main_reg_pos = System::get(main_reg);
		size_t anc_UA_pos = System::get(anc_UA);
		if (is_full == true)
		{
			DenseMatrix<complex_t> ret(pow2(main_reg_num + anc_UA_num));
			auto range_a = range(pow2(anc_UA_num));
			auto range_m = range(pow2(main_reg_num));

			for (auto [a, m] : product(range_a, range_m))
			{
				StateType state(1);

				state.back().get(main_reg_pos).value = m;
				state.back().get(anc_UA_pos).value = a;
				if (is_dag)
					encA.dag(state);
				else
					encA(state);
				std::vector<complex_t> vec(pow2(main_reg_num + anc_UA_num), 0);
				for (auto& s : state)
				{
					size_t _index = concat_value(
						{
							{s.get(main_reg_pos).value, main_reg_num},
							{s.get(anc_UA_pos).value, anc_UA_num},
						}
						);
					vec[_index] = s.amplitude;
				}
				size_t index = concat_value(
					{
						{m, main_reg_num},
						{a, anc_UA_num},
					}
					);
				for (size_t j = 0; j < pow2(main_reg_num + anc_UA_num); ++j)
				{
					ret(j, index) = vec[j];
				}
			}
			return ret;
		}
		else
		{
			DenseMatrix<complex_t> ret(pow2(main_reg_num));

			for (auto i : range(pow2(main_reg_num)))
			{
				StateType state(1);

				state.back().get(main_reg_pos).value = i;
				state.back().get(anc_UA_pos).value = 0;

				if (is_dag)
					encA.dag(state);
				else
					encA(state);

				double prob = PartialTraceSelect({ {anc_UA, 0} })(state);

				std::vector<complex_t> vec(pow2(main_reg_num), 0);
				for (auto& s : state)
				{
					vec[s.get(main_reg_pos).value] = s.amplitude;
				}
				for (size_t j = 0; j < pow2(main_reg_num); ++j)
				{
					ret(j, i) = vec[j] / prob;
				}
			}
			return ret;
		}
	}

	/**
	 * @brief 提取块编码矩阵
	 * @tparam BlockEncoding 块编码类型
	 * @param encA 块编码对象
	 * @param main_reg 主寄存器名称
	 * @param anc_UA 辅助寄存器名称
	 * @param is_full 是否完整提取
	 * @param is_dag 是否为 dagger 版本
	 * @return 块编码的稠密矩阵表示
	 */
	template<typename BlockEncoding>
	DenseMatrix<complex_t> extract_block_encoding(BlockEncoding encA, std::string_view main_reg, std::string_view anc_UA,
		bool is_full = false, bool is_dag = false)
	{
		return _extract_block_encoding<BlockEncoding, SparseState>(encA, main_reg, anc_UA, is_full, is_dag);
	}

	/**
	 * @brief 检查两个状态是否相等
	 * @param state1 第一个状态向量
	 * @param state2 第二个状态向量
	 * @throws 当状态不相等时抛出异常并打印差异
	 */
	inline void state_equal_check(std::vector<System> state1, std::vector<System> state2)
	{	
		SortUnconditional()(state1);
		SortUnconditional()(state2);
		if (state1.size() != state2.size())
		{
			fmt::print("Size not equal: {} vs {}", state1.size(), state2.size());
			goto CHECK_FAILED;
		}
		for (size_t i = 0; i < state1.size(); ++i)
		{
			if (state1[i] != state2[i])
			{
				fmt::print("State not equal at index {}:\n", i);
				goto CHECK_FAILED;
			}
		}
		return;
	CHECK_FAILED:
		fmt::print("State 1:\n");
		StatePrint(0 | Detail)(state1);
		fmt::print("State 2:\n");
		StatePrint(0 | Detail)(state2);
		throw_general_runtime_error("Hadamard_Bool: GPU-version failed!");
	}
}

#ifdef USE_CUDA

namespace qram_simulator {

	/**
	 * @brief CUDA 并行操作测试类
	 */
	struct ParallelOperationTest : BaseOperator {
		/** @brief 实部 */
		double real;

		/** @brief 虚部 */
		double imag;

		/** @brief 值 */
		uint64_t value;

		/**
		 * @brief 构造函数
		 * @param real_ 实部
		 * @param imag_ 虚部
		 * @param value_ 值
		 */
		ParallelOperationTest(double real_, double imag_, uint64_t value_) :
			real(real_), imag(imag_), value(value_) {
		}

		/**
		 * @brief CPU 版本（抛出未实现异常）
		 * @param state 系统状态向量
		 * @throws 总是抛出未实现异常
		 */
		void operator()(std::vector<System>& state) const {
			throw_not_implemented();
		}

		/**
		 * @brief CUDA 应用测试操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
	};

}

#endif
