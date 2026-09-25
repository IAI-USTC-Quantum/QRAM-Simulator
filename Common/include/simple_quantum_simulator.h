#pragma once

/**
 * @file simple_quantum_simulator.h
 * @brief 简单的全振幅量子电路模拟器（态向量法）与比特序工具。
 *
 * 提供态向量上的单比特门（unitary1q）、Hadamard、测量、比特序
 * 抽取 / 重建（extract_binary / reconstruct_binary）等原语，
 * 主要服务于 QRAMFullAmp 的全振幅桥接与小规模验证实验。
 */

#include "basic.h"

namespace qram_simulator {
	namespace quantum_simulator
	{
		/**
		 * @brief 从 index 中按 digits 列表抽取比特。
		 *
		 * 例：index=10101（二进制），digits=[1,3] → 输出 111
		 * （digits[0] 对应输出最低位）。
		 * @param index 输入整数
		 * @param digits 要抽取的比特下标列表
		 * @return 抽取比特拼成的整数
		 */
		size_t extract_binary(size_t index, const std::vector<size_t>& digits);

		/**
		 * @brief digits 列表的掩码。
		 *
		 * 例：digits=[1,3] → 输出 1010（二进制）。
		 */
		size_t mask(const std::vector<size_t>& digits);

		/**
		 * @brief extract_binary 的逆运算：把 value 的比特按 digits 展开。
		 *
		 * 例：index=111，digits=[1,3] → 输出 10101。
		 */
		size_t reconstruct_binary(size_t index, const std::vector<size_t>& digits);

		/**
		 * @brief 清除 index 中 mask_digits 对应的比特位。
		 *
		 * 例：index=11111，mask=[1,3] → 输出 10101。
		 */
		size_t mask_remain(size_t index, const std::vector<size_t>& mask);

		/**
		 * @brief 用 new_value 替换 index 中 mask 对应的比特位。
		 *
		 * 等价于 (index & ~mask) + reconstruct_binary(new_value, mask)。
		 */
		size_t mask_replace(size_t index, const std::vector<size_t>& mask, size_t new_value);

		/// 把 number 的比特按 digits 写入 remain
		void insert_binary(size_t& remain, const std::vector<size_t>& digits, size_t number);
		/// 求总比特数为 sz 时除地址 / 数据比特外的旁观比特列表
		std::vector<size_t> _get_remain_qubits(
			size_t sz,
			std::vector<size_t> address_qubits,
			const std::vector<size_t>& data_qubits);

		/// 丢弃 index 中 digits 对应的比特位（压缩剩余比特）
		size_t discard(size_t n, std::vector<size_t> digits);
		/// 在态向量上按 mask_target 丢弃指定比特（维度压缩）
		void discard(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n, size_t mask_target);

		/// 返回态向量的可读字符串
		std::string get_state(const std::vector<complex_t>& state);
		/// 返回态向量在位置 pos 附近的可读字符串
		std::string get_state(const std::vector<complex_t>& state, size_t pos);

		/// 把 state 初始化为 n 比特的 |0…0>（长度 2^n，首元素为 1）
		void init_n_state(std::vector<complex_t>& state, size_t n);

		/// 按玻恩规则从态向量采样一个基矢（全局随机引擎）
		size_t measure(const std::vector<complex_t>& state);

		/**
		 * @brief 对指定位集合的测量结果（塌缩 + 归一化）。
		 */
		struct MeasureResult
		{
			/// 测量到的经典值
			size_t measure_result;
			/// 被测比特掩码
			size_t measure_mask;
			/// 该结果的玻恩概率
			double probability;
		};

		/**
		 * @brief 在态向量上测量 qn 列出的 n 个比特。
		 *
		 * 采样一个结果，把态塌缩到该分支并归一化。
		 * @param state 态向量（就地修改）
		 * @param qn 被测比特下标列表
		 * @param n 比特数（qn 的长度）
		 * @return 测量结果（经典值 / 掩码 / 概率）
		 */
		MeasureResult measure(std::vector<complex_t>& state, std::vector<size_t> qn, size_t n);

		/**
		 * @brief 对 digit 比特作用单比特酉门 func。
		 *
		 * func 接受该比特 |0> / |1> 的一对振幅引用并就地更新。
		 * @param state 态向量
		 * @param digit 目标比特下标
		 * @param func 二元可调用对象（门作用函数）
		 */
		template<typename Fn>
		void unitary1q(std::vector<complex_t>& state, size_t digit, Fn func)
		{
			for (size_t i = 0; i < state.size(); ++i)
			{
				if (digit1(i, digit)) continue;
				func(state[i], state[i + pow2(digit)]);
			}
		}

		/// Hadamard 门：(a,b) ← ((a+b)/√2, (a−b)/√2)
		void hadamard(complex_t& a, complex_t& b);
		/// NOT 门（X 门）：(a,b) ← (b,a)
		void not_gate(complex_t& a, complex_t& b);

		/// 对前 n 个比特逐个作用 Hadamard
		inline void hadamard_all(std::vector<complex_t>& state, size_t n)
		{
			for (size_t i = 0; i < n; ++i) {
				unitary1q(state, i, hadamard);
			}
		}

		/**
		 * @brief 取态向量中最大的 n_max 个元素（保持 map 有序）。
		 * @param state 输入序列
		 * @param n_max 保留个数
		 * @return {下标: 元素} 映射
		 */
		template<typename Ty>
		std::map<size_t, Ty> get_max_elements(const std::vector<Ty>& state, size_t n_max)
		{
			std::map<size_t, Ty> maximums;
			for (size_t i = 0; i < std::size(state); ++i)
			{
				if (maximums.size() < n_max) {
					maximums[i] = state[i];
					continue;
				}
				auto&& mini = maximums.begin()->second;
				if (mini < state[i])
				{
					maximums.erase(maximums.begin());
					maximums[i] = state[i];
				}
			}
			return maximums;
		}

		/// 取态向量中振幅最大的 n_max 个基矢（按模方比较）
		std::map<size_t, complex_t> get_max_state(const std::vector<complex_t>& state, size_t n_max);
		/// 最大 n_max 个基矢的可读字符串
		std::string max_state2str(const std::vector<complex_t>& state, size_t n_max);

		/// 打印态向量（check 为真时顺带做归一化校验）
		inline void print_state(const std::vector<complex_t>& state, bool check = true)
		{
			fmt::print(get_state(state));
			if (check) {
				if (std::abs(amp_sum(state) - 1.0) > epsilon)
					throw_bad_result();
			}
		}

		/// 打印态向量在位置 pos 附近的内容
		inline void print_state(const std::vector<complex_t>& state, size_t pos)
		{
			fmt::print(get_state(state, pos));
		}

		/// 打印最大的 n_max 个基矢
		inline void print_max_state(const std::vector<complex_t>& state, size_t n_max)
		{
			fmt::print(max_state2str(state, n_max));
		}
	} // namespace quantum_simulator
} // namespace qram_simulator
