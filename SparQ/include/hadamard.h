/**
 * @file hadamard.h
 * @brief Hadamard 门操作定义
 * @details 实现 Hadamard 门的多种变体，包括整数寄存器 Hadamard、
 *          完整 Hadamard、单比特 Hadamard 和部分量子位 Hadamard
 */

#pragma once
#include "quantum_interfere_basic.h"


namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 整数 Hadamard 门
	 * @details 对整个整数寄存器应用 Hadamard 变换，将计算基态转换为叠加态
	 */
	struct Hadamard_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 位掩码 */
		uint64_t mask;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 * @param n_digits_ 量子位数
		 */
		Hadamard_Int(std::string_view reg_in, size_t n_digits_)
			: Hadamard_Int(System::get(reg_in), n_digits_)
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 * @param n_digits_ 量子位数
		 */
		Hadamard_Int(size_t reg_in, size_t n_digits_)
		{
			id = reg_in;
			n_digits = n_digits_;
			mask = pow2(n_digits);
			mask--;
			mask = ~mask;
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
		 * @brief 在指定范围执行操作
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 */
		void operate(size_t l, size_t r, std::vector<System>& state) const;

		/**
		 * @brief 应用 Hadamard 门操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const; 

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用 Hadamard 门操作
		 * @param s CUDA 稀疏状态
		 */
		void operator()(CuSparseState& s) const;
#endif
	};

	/**
	 * @brief 完整 Hadamard 门
	 * @details 对整个寄存器应用完整的 Hadamard 变换，包含所有可能的输出状态
	 */
	struct Hadamard_Int_Full : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位数 */
		size_t n_digits;

		/** @brief 阈值 */
		const size_t few_threshold = n_digits - 1;

		/** @brief 完整状态大小 */
		size_t full_size;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 */
		Hadamard_Int_Full(std::string_view reg_in)
			: id(System::get(reg_in)), n_digits(System::size_of(reg_in)),
			full_size(pow2(n_digits)) {}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 */
		Hadamard_Int_Full(size_t reg_in)
			: id(reg_in), n_digits(System::size_of(reg_in)),
			full_size(pow2(n_digits)) {}

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
		 * @brief 应用 Hadamard 门操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 稀疏桶操作
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 */
		void operate_bucket_sparse(const std::vector<size_t>& positions, std::vector<System>& state) const;

		/**
		 * @brief 原地桶操作
		 * @param positions 位置列表
		 * @param state 系统状态向量
		 */
		void operate_bucket_inplace(const std::vector<size_t>& positions, std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用 Hadamard 门操作
		 * @param s CUDA 稀疏状态
		 */
		void operator()(CuSparseState& s) const;
#endif
	};

	/**
	 * @brief 单比特 Hadamard 门
	 * @details 仅适用于单比特寄存器的 Hadamard 门
	 */
	struct Hadamard_Bool : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 输出寄存器 ID */
		size_t out_id;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 * @throws 当寄存器大小不为1时抛出异常
		 */
		Hadamard_Bool(std::string_view reg_in)
			: out_id(System::get(reg_in))
		{
			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 * @throws 当寄存器大小不为1时抛出异常
		 */
		Hadamard_Bool(size_t reg_in)
			: out_id(reg_in)
		{
			if (System::size_of(out_id) != 1)
				throw_invalid_input("Hadamard_Bool: size of output register must be 1");
		}

		/**
		 * @brief 成对操作（V2 版本）
		 * @param zero |0> 分支索引
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const;

		/**
		 * @brief 单独操作 |0> 分支
		 * @param zero |0> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_zero(size_t zero, std::vector<System>& state) const;

		/**
		 * @brief 单独操作 |1> 分支
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_one(size_t one, std::vector<System>& state) const;

		/**
		 * @brief 应用 Hadamard 门操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用 Hadamard 门操作
		 * @param s CUDA 稀疏状态
		 */
		void operator()(CuSparseState& s) const;
#endif
	};

	/**
	 * @brief 部分量子位 Hadamard 门
	 * @details 只对寄存器中的部分量子位应用 Hadamard 变换
	 */
	struct Hadamard_PartialQubit : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 位掩码 */
		size_t mask;

		/** @brief 量子位位置集合 */
		std::set<size_t> qubit_positions;

		ClassControllable

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 * @param qubit_positions_ 量子位位置集合
		 */
		Hadamard_PartialQubit(std::string_view reg_in, std::set<size_t>& qubit_positions_)
			: id(System::get(reg_in)), qubit_positions(qubit_positions_)
		{
			mask = make_mask(qubit_positions_);
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 * @param qubit_positions_ 量子位位置集合
		 */
		Hadamard_PartialQubit(size_t reg_in, std::set<size_t>& qubit_positions_)
			: id(reg_in), qubit_positions(qubit_positions_)
		{
			mask = make_mask(qubit_positions_);
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
		 * @brief 成对操作
		 * @param zero |0> 分支索引
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_pair(size_t zero, size_t one, std::vector<System>& state) const;

		/**
		 * @brief 单独操作 |0> 分支
		 * @param zero |0> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_zero(size_t zero, std::vector<System>& state) const;

		/**
		 * @brief 单独操作 |1> 分支
		 * @param one |1> 分支索引
		 * @param state 系统状态向量
		 */
		void operate_alone_one(size_t one, std::vector<System>& state) const;

		/**
		 * @brief 在指定范围执行操作
		 * @param l 左边界
		 * @param r 右边界
		 * @param state 系统状态向量
		 */
		void operate(size_t l, size_t r, std::vector<System>& state) const;

		/**
		 * @brief 应用部分 Hadamard 门操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};

}
