/**
 * @file qft.h
 * @brief 量子傅里叶变换 (QFT) 定义
 * @details 实现量子傅里叶变换及其逆变换，支持标准版本和完整版本
 */

#pragma once
#include "quantum_interfere_basic.h"

namespace qram_simulator {

	/**
	 * @brief 量子傅里叶变换 (QFT)
	 * @details 对整数寄存器执行量子傅里叶变换
	 */
	struct QFT : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 单位根 omega = e^(2πi/2^n) */
		complex_t omega;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_ins 寄存器名称
		 */
		QFT(std::string_view reg_ins);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 */
		QFT(size_t reg_in);

		/**
		 * @brief 获取指定位置的值（辅助函数）
		 * @param i 索引
		 * @param state 系统状态向量
		 * @return 值的引用
		 */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}

		/**
		 * @brief 在指定范围执行操作
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 */
		void operate(size_t l, size_t r, std::vector<System>& state) const;

		/**
		 * @brief 应用 QFT 操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};

	/**
	 * @brief 逆量子傅里叶变换 (inverse QFT)
	 * @details 对整数寄存器执行逆量子傅里叶变换
	 */
	struct inverseQFT : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 单位根 omega = e^(2πi/2^n) */
		complex_t omega;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_ins 寄存器名称
		 */
		inverseQFT(std::string_view reg_ins);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 */
		inverseQFT(size_t reg_in);

		/**
		 * @brief 获取指定位置的值（辅助函数）
		 * @param i 索引
		 * @param state 系统状态向量
		 * @return 值的引用
		 */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}

		/**
		 * @brief 在指定范围执行操作
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 */
		void operate(size_t l, size_t r, std::vector<System>& state) const;

		/**
		 * @brief 应用逆 QFT 操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};

	/**
	 * @brief 完整量子傅里叶变换
	 * @details 使用 FFT 算法优化的完整 QFT 实现，包含位反转预处理
	 */
	struct QFT_Full : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 单位根 omega = e^(2πi/2^n) */
		complex_t omega;

		/** @brief 阈值 */
		const size_t few_threshold = n_digits - 1;

		/** @brief 完整状态大小 */
		size_t full_size;

		/** @brief 额外振幅因子 */
		double extra_amplitude;

		/** @brief 位反转表 */
		std::vector<uint64_t> bitrev;

		ClassControllable

		/**
		 * @brief 预计算位反转表
		 */
		inline void precompute_bitrev() {
			profiler _("prefcompute_bitrev");
			if (bitrev.empty())
			{
				bitrev.resize(full_size);
#ifndef SINGLE_THREAD
#pragma omp parallel for schedule(static)
#endif
				for (int64_t i = 0; i < full_size; ++i) {
					uint64_t x = i;
					uint64_t rev = 0;
					for (size_t j = 0; j < n_digits; ++j) {
						rev = (rev << 1) | (x & 1);
						x >>= 1;
					}
					bitrev[i] = rev;
				}
			}
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 */
		QFT_Full(size_t reg_in)
			: id(reg_in), n_digits(System::size_of(reg_in)), 
			full_size(pow2(n_digits)), extra_amplitude(1.0 / std::sqrt(full_size))
		{
			double theta = 2 * pi / pow2(n_digits);
			omega = complex_t{ cos(theta), sin(theta) };
			precompute_bitrev();
		}

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 */
		QFT_Full(std::string_view reg_in)
			: id(System::get(reg_in)), n_digits(System::size_of(id)),
			full_size(pow2(n_digits)), extra_amplitude(1.0 / std::sqrt(full_size))
		{
			double theta = 2 * pi / pow2(n_digits);
			omega = complex_t{ cos(theta), sin(theta) };
			precompute_bitrev();
		}

		/**
		 * @brief 获取指定位置的值（辅助函数）
		 * @param i 索引
		 * @param state 系统状态向量
		 * @return 值的引用
		 */
		inline size_t& val(size_t i, std::vector<System>& state) const
		{
			return state[i].get(id).value;
		}

		/**
		 * @brief 稀疏桶操作
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 */
		void operate_bucket_sparse(const std::vector<size_t>& positions, std::vector<System>& state) const;

		/**
		 * @brief FFT 实现
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 * @param inverse 是否为逆变换
		 */
		void fft(const std::vector<size_t>& positions, std::vector<System>& state, bool inverse) const;

		/**
		 * @brief 原地桶操作
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 */
		void operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state) const;

		/**
		 * @brief 原地逆桶操作
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 */
		void operate_bucket_inplace_inv(const std::vector<size_t>& positions, std::vector<System>& state) const;

		/**
		 * @brief 应用完整 QFT 操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger (逆 QFT) 操作
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;
	};
}
