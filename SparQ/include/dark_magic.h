/**
 * @file dark_magic.h
 * @brief 内部优化操作定义
 * @details 包含归一化和非安全初始化等底层优化操作，
 *          这些操作通常用于内部实现，使用时需谨慎
 */

#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 归一化操作
	 * @details 对量子态进行归一化，使总概率为1
	 */
	struct Normalize : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 应用归一化操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用归一化操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 非安全初始化操作
	 * @details 直接将寄存器设置为指定值，不进行安全检查
	 * @warning 此操作不检查值是否超出寄存器范围，使用时需谨慎
	 */
	struct Init_Unsafe : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 要设置的值 */
		size_t value;

		/** @brief 寄存器 ID */
		size_t id;

		/**
		 * @brief 构造函数（ID 版本）
		 * @param id_ 寄存器 ID
		 * @param value_ 要设置的值
		 */
		Init_Unsafe(int id_, size_t value_) :
			value(value_), id(id_)
		{ }

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg 寄存器名称
		 * @param value_ 要设置的值
		 */
		Init_Unsafe(std::string_view reg, size_t value_) :
			value(value_), id(System::get(reg))
		{ }

		/**
		 * @brief 应用非安全初始化操作
		 * @param system_states 系统状态向量
		 */
		void operator()(std::vector<System>& system_states) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用非安全初始化操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};
}
