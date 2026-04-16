/**
 * @file quantum_interfere_basic.h
 * @brief 量子干涉基础工具定义
 * @details 提供状态哈希、相等比较、小于比较等基础工具类，
 *          用于量子干涉操作中的状态管理和查找
 */

#pragma once
#include "system_operations.h"
#include "sort_state.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 排除指定键的状态哈希
	 * @details 计算状态哈希值时排除指定寄存器
	 */
	struct StateHashExceptKey {
		/** @brief 排除的键（寄存器 ID） */
		size_t id;

		/**
		 * @brief 构造函数
		 * @param id_ 排除的寄存器 ID
		 */
		StateHashExceptKey(size_t id_)
			: id(id_)
		{}

		/**
		 * @brief 计算哈希值
		 * @param v 系统状态
		 * @return 哈希值
		 */
		size_t operator()(const System& v) const;
	};

	/**
	 * @brief 排除指定量子位的状态哈希
	 * @details 计算状态哈希值时排除寄存器的指定量子位
	 */
	struct StateHashExceptQubits {
		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位位置集合 */
		std::set<size_t> qubit_positions;

		/**
		 * @brief 构造函数
		 * @param id_ 寄存器 ID
		 * @param qubit_positions_ 量子位位置集合
		 */
		StateHashExceptQubits(size_t id_, std::set<size_t> qubit_positions_)
			: id(id_), qubit_positions(qubit_positions_)
		{}

		/**
		 * @brief 计算哈希值
		 * @param v 系统状态
		 * @return 哈希值
		 */
		size_t operator()(const System& v) const;
	};

	/**
	 * @brief 排除指定键的状态相等比较
	 * @details 比较两个状态是否相等时排除指定寄存器
	 */
	struct StateEqualExceptKey {
		/** @brief 排除的键（寄存器 ID） */
		size_t id;

		/**
		 * @brief 构造函数
		 * @param id_ 排除的寄存器 ID
		 */
		StateEqualExceptKey(size_t id_) : id(id_) {}

		/**
		 * @brief 相等比较
		 * @param v1 第一个系统状态
		 * @param v2 第二个系统状态
		 * @return 是否相等
		 */
		size_t operator()(const System& v1, const System& v2) const;
	};

	/**
	 * @brief 排除指定量子位的状态相等比较
	 * @details 比较两个状态是否相等时排除寄存器的指定量子位
	 */
	struct StateEqualExceptQubits {
		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 量子位位置集合 */
		std::set<size_t> qubit_positions;

		/**
		 * @brief 构造函数
		 * @param id_ 寄存器 ID
		 * @param qubit_positions_ 量子位位置集合
		 */
		StateEqualExceptQubits(size_t id_, std::set<size_t> qubit_positions_) : id(id_), qubit_positions(qubit_positions_) {}

		/**
		 * @brief 相等比较
		 * @param v1 第一个系统状态
		 * @param v2 第二个系统状态
		 * @return 是否相等
		 */
		size_t operator()(const System& v1, const System& v2) const;
	};

	/**
	 * @brief 排除指定键的状态小于比较
	 * @details 比较两个状态大小时排除指定寄存器
	 */
	struct StateLessExceptKey {
		/** @brief 排除的键（寄存器 ID） */
		size_t id;

		/**
		 * @brief 构造函数
		 * @param id_ 排除的寄存器 ID
		 */
		StateLessExceptKey(size_t id_) : id(id_) {}

		/**
		 * @brief 小于比较
		 * @param v1 第一个系统状态
		 * @param v2 第二个系统状态
		 * @return 是否小于
		 */
		size_t operator()(const System& v1, const System& v2) const;
	};

	/**
	 * @brief 排除指定量子位的状态小于比较
	 * @details 比较两个状态大小时排除寄存器的指定量子位
	 */
	struct StateLessExceptQubits {
		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 位掩码 */
		size_t mask;

		/** @brief 量子位 ID 集合 */
		std::set<size_t> qubit_ids;

		/**
		 * @brief 构造函数
		 * @param id_ 寄存器 ID
		 * @param qubit_ids_ 量子位 ID 集合
		 */
		StateLessExceptQubits(size_t id_, std::set<size_t> qubit_ids_) : id(id_), qubit_ids(qubit_ids_)
		{
			mask = make_mask(qubit_ids);
		}

		/**
		 * @brief 移除指定位
		 * @param val 原始值
		 * @return 移除位后的值
		 */
		inline size_t remove_digits(size_t val) const
		{
			return val & mask;
		}

		/**
		 * @brief 创建位掩码
		 * @param qubit_ids 量子位 ID 集合
		 * @return 位掩码
		 */
		inline size_t make_mask(const std::set<size_t>& qubit_ids)
		{
			size_t mask = 0;
			for (auto id : qubit_ids)
			{
				mask += pow2(id);
			}
			mask = ~mask;
			return mask;
		}

		/**
		 * @brief 小于比较
		 * @param v1 第一个系统状态
		 * @param v2 第二个系统状态
		 * @return 是否小于
		 */
		size_t operator()(const System& v1, const System& v2) const;
	};

}
