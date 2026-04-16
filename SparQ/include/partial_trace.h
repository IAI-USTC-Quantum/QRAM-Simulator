/**
 * @file partial_trace.h
 * @brief 偏迹运算定义
 * @details 实现量子态的偏迹操作，用于约化密度矩阵和概率计算
 */

#pragma once
#include "basic_components.h"
#ifdef USE_CUDA
#include "cuda/cuda_utils.cuh"
#endif

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 偏迹操作类
	 * @details 对指定寄存器执行偏迹运算
	 */
	struct PartialTrace {
		/** @brief 偏迹寄存器 ID 列表 */
		std::vector<size_t> partial_trace_registers;

		/**
		 * @brief 构造函数（名称列表版本）
		 * @param partial_trace_register_names 寄存器名称列表
		 */
		PartialTrace(const std::vector<std::string>& partial_trace_register_names);

		/**
		 * @brief 构造函数（ID 列表版本）
		 * @param partial_trace_register_names 寄存器 ID 列表
		 */
		PartialTrace(const std::vector<size_t>& partial_trace_register_names);

		/**
		 * @brief 构造函数（单个名称版本）
		 * @param partial_trace_register_name 寄存器名称
		 */
		PartialTrace(std::string_view partial_trace_register_name);

		/**
		 * @brief 构造函数（单个 ID 版本）
		 * @param partial_trace_register_name 寄存器 ID
		 */
		PartialTrace(size_t partial_trace_register_name);

		/**
		 * @brief 应用偏迹操作
		 * @param state 系统状态向量
		 * @return 偏迹后的值列表和概率
		 */
		std::pair<std::vector<uint64_t>, double> operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用偏迹操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 偏迹后的值列表和概率
		 */
		std::pair<std::vector<uint64_t>, double> operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用偏迹操作
		 * @param state CUDA 稀疏状态
		 * @return 偏迹后的值列表和概率
		 */
		std::pair<std::vector<uint64_t>, double> operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 选择性偏迹操作类
	 * @details 对指定寄存器执行偏迹运算，并选择特定值
	 */
	struct PartialTraceSelect {
		/** @brief 偏迹寄存器 ID 列表 */
		std::vector<size_t> partial_trace_registers;

		/** @brief 选择的值列表 */
		std::vector<uint64_t> select_values;

		/**
		 * @brief 构造函数（名称到值映射版本）
		 * @param partial_traces 寄存器名称到值的映射
		 */
		PartialTraceSelect(const std::map<std::string_view, uint64_t>& partial_traces);

		/**
		 * @brief 构造函数（ID 到值映射版本）
		 * @param partial_traces 寄存器 ID 到值的映射
		 */
		PartialTraceSelect(const std::map<size_t, uint64_t>& partial_traces);

		/**
		 * @brief 构造函数（名称列表和值列表版本）
		 * @param partial_trace_regs_ 寄存器名称列表
		 * @param select_values_ 选择的值列表
		 */
		PartialTraceSelect(const std::vector<std::string>& partial_trace_regs_,
			const std::vector<uint64_t> &select_values_);

		/**
		 * @brief 构造函数（ID 列表和值列表版本）
		 * @param partial_trace_regs_ 寄存器 ID 列表
		 * @param select_values_ 选择的值列表
		 */
		PartialTraceSelect(const std::vector<size_t>& partial_trace_regs_,
			const std::vector<uint64_t> &select_values_);

		/**
		 * @brief 应用选择性偏迹操作
		 * @param state 系统状态向量
		 * @return 选中状态的概率
		 */
		double operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用选择性偏迹操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 选中状态的概率
		 */
		double operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用选择性偏迹操作
		 * @param state CUDA 稀疏状态
		 * @return 选中状态的概率
		 */
		double operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 范围选择性偏迹操作类
	 * @details 对指定寄存器执行偏迹运算，并选择值范围
	 */
	struct PartialTraceSelectRange {
		/** @brief 偏迹寄存器 ID */
		size_t partial_trace_register;

		/** @brief 选择范围 */
		std::pair<size_t, size_t> select_range;

		/** @brief 结果概率 */
		double r = 0.0;

		/**
		 * @brief 构造函数（名称版本）
		 * @param partial_trace_register_ 寄存器名称
		 * @param select_range_ 选择范围 [min, max]
		 */
		PartialTraceSelectRange(std::string_view partial_trace_register_,
			std::pair<size_t, size_t> select_range_) :
			partial_trace_register(System::get(partial_trace_register_)),
			select_range(select_range_)
		{
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param partial_trace_register_ 寄存器 ID
		 * @param select_range_ 选择范围 [min, max]
		 */
		PartialTraceSelectRange(size_t partial_trace_register_,
			std::pair<size_t, size_t> select_range_) :
			partial_trace_register(partial_trace_register_),
			select_range(select_range_)
		{
		}

		/**
		 * @brief 应用范围选择性偏迹操作
		 * @param state 系统状态向量
		 * @return 选中状态的概率
		 */
		double operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用范围选择性偏迹操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 选中状态的概率
		 */
		double operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用范围选择性偏迹操作
		 * @param state CUDA 稀疏状态
		 * @return 选中状态的概率
		 */
		double operator()(CuSparseState& state);
#endif
	};


}
