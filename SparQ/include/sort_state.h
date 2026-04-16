/**
 * @file sort_state.h
 * @brief 状态排序定义
 * @details 实现量子态的各种排序操作，支持按键排序、
 *          无条件排序、按振幅排序等多种排序方式
 */

#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 排除键排序
	 * @details 排序时排除指定键，使其移到末尾
	 */
	struct SortExceptKey : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 排除的键（寄存器 ID） */
		size_t id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param key_ 寄存器名称
		 */
		SortExceptKey(std::string_view key_)
			: id(System::get(key_))
		{}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param key_ 寄存器 ID
		 */
		SortExceptKey(size_t key_)
			: id(key_)
		{}

		/**
		 * @brief 应用排序操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;
	};

	/**
	 * @brief 按键排序
	 * @details 按指定寄存器值排序
	 */
	struct SortByKey : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 排序键（寄存器 ID） */
		size_t register_key;

		/**
		 * @brief 构造函数（名称版本）
		 * @param key 寄存器名称
		 */
		SortByKey(std::string_view key);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param key 寄存器 ID
		 */
		SortByKey(size_t key);

		/**
		 * @brief 应用排序操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};

	/**
	 * @brief 排除位排序
	 * @details 排序时排除寄存器的指定位
	 */
	struct SortExceptBit : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 寄存器 ID */
		size_t id;

		/** @brief 位索引 */
		size_t digit;

		/**
		 * @brief 构造函数（名称版本）
		 * @param key_ 寄存器名称
		 * @param digit_ 位索引
		 */
		SortExceptBit(std::string_view key_, size_t digit_)
			: id(System::get(key_)), digit(digit_)
		{}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param key_ 寄存器 ID
		 * @param digit_ 位索引
		 */
		SortExceptBit(size_t key_, size_t digit_)
			: id(key_), digit(digit_)
		{}

		/**
		 * @brief 应用排序操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;
	};

	/**
	 * @brief 创建位掩码
	 * @param qubit_ids 量子位 ID 集合
	 * @return 位掩码
	 */
	inline uint64_t make_mask(const std::set<size_t>& qubit_ids)
	{
		uint64_t mask = 0;
		for (auto id : qubit_ids)
		{
			mask += pow2(id);
		}
		mask = ~mask;
		return mask;
	}

	/**
	 * @brief Hadamard 排除键排序
	 * @details 针对 Hadamard 操作优化的排除键排序
	 */
	struct SortExceptKeyHadamard : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 排除的键（寄存器 ID） */
		size_t id;

		/** @brief 位掩码 */
		uint64_t mask;

		/** @brief 量子位 ID 集合 */
		std::set<size_t> qubit_ids;

		/**
		 * @brief 构造函数（名称版本）
		 * @param key_ 寄存器名称
		 * @param qubit_ids_ 量子位 ID 集合
		 */
		SortExceptKeyHadamard(std::string_view key_, std::set<size_t> qubit_ids_)
			: id(System::get(key_)), qubit_ids(qubit_ids_)
		{
			mask = make_mask(qubit_ids);
		}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param key_ 寄存器 ID
		 * @param qubit_ids_ 量子位 ID 集合
		 */
		SortExceptKeyHadamard(size_t key_, std::set<size_t> qubit_ids_)
			: id(key_), qubit_ids(qubit_ids_)
		{
			mask = make_mask(qubit_ids);
		}

		/**
		 * @brief 移除指定位
		 * @param val 原始值
		 * @return 移除位后的值
		 */
		size_t remove_digits(size_t val) const;

		/**
		 * @brief 应用排序操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;
	};

	/**
	 * @brief 无条件排序
	 * @details 对系统状态进行无条件并行排序
	 */
	struct SortUnconditional : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 默认构造函数
		 */
		SortUnconditional() {}

		/**
		 * @brief 应用排序操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;
	};

	/**
	 * @brief 按振幅排序
	 * @details 按振幅对系统状态进行排序
	 */
	struct SortByAmplitude : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/**
		 * @brief 默认构造函数
		 */
		SortByAmplitude() {}

		/**
		 * @brief 应用排序操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;
	};


	/**
	 * @brief 双键排序
	 * @details 按两个寄存器值排序
	 */
	struct SortByKey2 : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 第一个键（寄存器 ID） */
		size_t id1;

		/** @brief 第二个键（寄存器 ID） */
		size_t id2;

		/**
		 * @brief 构造函数（名称版本）
		 * @param key1_ 第一个寄存器名称
		 * @param key2_ 第二个寄存器名称
		 */
		SortByKey2(std::string_view key1_, std::string_view key2_)
			: id1(System::get(key1_)), id2(System::get(key2_))
		{}

		/**
		 * @brief 构造函数（ID 版本）
		 * @param key1_ 第一个寄存器 ID
		 * @param key2_ 第二个寄存器 ID
		 */
		SortByKey2(size_t key1_, size_t key2_)
			: id1(key1_), id2(key2_)
		{}

		/**
		 * @brief 应用排序操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;
	};

	/**
	 * @brief 比较两个系统是否相等（排除指定键）
	 * @details 用于 CondRot_General_Bool, CondRot_General_Bool_QW, Hadamard_Int 等操作
	 * @param a 第一个系统
	 * @param b 第二个系统
	 * @param out_id 排除的键（寄存器 ID）
	 * @return 是否相等
	 */
	bool compare_equal(const System& a, const System& b, size_t out_id);

	/**
	 * @brief 比较两个系统是否相等（排除两个指定键）
	 * @details 用于 QRAM::set_branches
	 * @param a 第一个系统
	 * @param b 第二个系统
	 * @param out_id1 排除的第一个键（寄存器 ID）
	 * @param out_id2 排除的第二个键（寄存器 ID）
	 * @return 是否相等
	 */
	bool compare_equal2(const System& a, const System& b, size_t out_id1, size_t out_id2);

	/**
	 * @brief 比较两个系统是否相等（排除指定键和掩码位）
	 * @param a 第一个系统
	 * @param b 第二个系统
	 * @param out_id 排除的键（寄存器 ID）
	 * @param mask 位掩码
	 * @return 是否相等
	 */
	bool compare_equal_rot(const System& a, const System& b, size_t out_id, uint64_t mask);

	/**
	 * @brief 比较两个系统是否相等（Hadamard 版本）
	 * @details 用于 Hadamard_PartialQubit，比较时排除目标量子位和掩码位
	 * @param a 第一个系统
	 * @param b 第二个系统
	 * @param out_id 排除的键（寄存器 ID）
	 * @param mask 位掩码
	 * @return 是否相等
	 */
	bool compare_equal_hadamard(const System& a, const System& b, size_t out_id, uint64_t mask);

}
