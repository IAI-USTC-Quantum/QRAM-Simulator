/**
 * @file parallel_phase_operations.h
 * @brief 并行相位操作定义
 * @details 实现并行条件相位翻转和全局相位操作，
 *          支持零条件相位翻转、范围条件相位翻转和反射操作
 */

#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 零条件相位翻转
	 * @details 当指定寄存器全为零时施加相位翻转
	 */
	struct ZeroConditionalPhaseFlip : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID 列表 */
		std::vector<size_t> ids;

		ClassControllable

		/**
		 * @brief 构造函数（ID 列表版本）
		 * @param regs_ 寄存器 ID 列表
		 */
		ZeroConditionalPhaseFlip(const std::vector<size_t> &regs_)
			: ids(regs_)
		{
		}

		/**
		 * @brief 构造函数（名称列表版本）
		 * @param regs_ 寄存器名称列表
		 */
		ZeroConditionalPhaseFlip(const std::vector<std::string> &regs_)
		{
			ids.reserve(regs_.size());
			for (const auto& reg : regs_)
			{
				ids.push_back(System::get(reg));
			}
		}

		/**
		 * @brief 应用相位翻转操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用相位翻转操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 范围条件相位翻转
	 * @details 当寄存器值在指定范围内时施加相位翻转
	 */
	struct RangeConditionalPhaseFlip : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 值范围 */
		size_t value_range;

		/**
		 * @brief 构造函数（ID 版本）
		 * @param id_ 寄存器 ID
		 * @param range_ 值范围
		 */
		RangeConditionalPhaseFlip(size_t id_, size_t range_)
			:id(id_), value_range(range_)
		{}

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_ 寄存器名称
		 * @param range_ 值范围
		 */
		RangeConditionalPhaseFlip(std::string_view reg_, size_t range_)
			:id(System::get(reg_)), value_range(range_)
		{}

		/**
		 * @brief 应用相位翻转操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};

	/**
	 * @brief 布尔反射操作
	 * @details 实现 Grover 反射操作：(I - 2|0><0|) 或 (2|0><0| - I)
	 */
	struct Reflection_Bool : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID 列表 */
		std::vector<size_t> regs;

		/** @brief 是否反向（true: I-2|0><0|, false: 2|0><0|-I） */
		bool inverse;

		ClassControllable

		/**
		 * @brief 构造函数（单寄存器名称版本）
		 * @param reg_ 寄存器名称
		 * @param inverse_ 是否反向（默认 false）
		 */
		Reflection_Bool(std::string_view reg_, bool inverse_ = false)
			: inverse(inverse_)
		{
			regs.push_back(System::get(reg_));
		}

		/**
		 * @brief 构造函数（单寄存器 ID 版本）
		 * @param id_ 寄存器 ID
		 * @param inverse_ 是否反向（默认 false）
		 */
		Reflection_Bool(size_t id_, bool inverse_ = false)
			: inverse(inverse_)
		{
			regs.push_back(id_);
		}

		/**
		 * @brief 构造函数（多寄存器名称版本）
		 * @param regs_ 寄存器名称列表
		 * @param inverse_ 是否反向（默认 false）
		 */
		Reflection_Bool(const std::vector<std::string> &regs_, bool inverse_ = false)
			: inverse(inverse_)
		{
			for (auto& reg : regs_) regs.push_back(System::get(reg));
		}

		/**
		 * @brief 构造函数（多寄存器 ID 版本）
		 * @param ids_ 寄存器 ID 列表
		 * @param inverse_ 是否反向（默认 false）
		 */
		Reflection_Bool(const std::vector<size_t> &ids_, bool inverse_ = false)
			: inverse(inverse_)
		{
			regs = ids_;
		}

		/**
		 * @brief 应用反射操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用反射操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 全局相位操作（整数寄存器）
	 * @details 对整个状态施加全局相位
	 */
	struct GlobalPhase_Int : BaseOperator
	{
		using BaseOperator::operator();
		using BaseOperator::dag;

		/** @brief 相位因子 */
		complex_t c;

		ClassControllable

		/**
		 * @brief 构造函数
		 * @param c_ 相位因子
		 */
		GlobalPhase_Int(complex_t c_) : c(c_) {};

		/**
		 * @brief 应用全局相位操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用 dagger 操作（共轭相位）
		 * @param state 系统状态向量
		 */
		void dag(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用全局相位操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 dagger 操作
		 * @param state CUDA 稀疏状态
		 */
		void dag(CuSparseState& state) const;
#endif
	};

}
