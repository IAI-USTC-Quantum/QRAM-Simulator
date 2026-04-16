/**
 * @file qram.h
 * @brief QRAM (Quantum Random Access Memory) 操作定义
 * @details 实现量子随机存取存储器的加载操作和输入生成器，
 *          支持 QRAMLoad、QRAMLoadFast 和 QRAMInputGenerator
 */

#pragma once
#include "basic_components.h"
#include "dark_magic.h"
#include "sort_state.h"

namespace qram_simulator 
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief QRAM 加载操作类
	 * @details 实现量子随机存取存储器的标准加载操作
	 * @note 仅适配 qram_qutrit::QRAMCircuit，如需 qubit 版本请使用 QRAMLoad_Qubit
	 */
	struct QRAMLoad : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief QRAM 电路指针 */
		const qram_qutrit::QRAMCircuit* qram;

		/** @brief 地址寄存器 ID */
		size_t register_addr;

		/** @brief 数据寄存器 ID */
		size_t register_data;

		/** @brief 版本字符串 */
		static std::string version;

		ClassControllable

		/**
		 * @brief 构造函数（ID 版本）
		 * @param qram_ QRAM 电路指针
		 * @param reg1 地址寄存器 ID
		 * @param reg2 数据寄存器 ID
		 * @throws 当地址寄存器类型不是无符号整数时抛出异常
		 */
		QRAMLoad(const qram_qutrit::QRAMCircuit* qram_, size_t reg1, size_t reg2)
			: register_addr(reg1), register_data(reg2)
		{
			qram = qram_;

			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_addr) != UnsignedInteger)
				throw_invalid_input();
#endif
		}

		/**
		 * @brief 构造函数（名称版本）
		 * @param qram QRAM 电路指针
		 * @param reg1 地址寄存器名称
		 * @param reg2 数据寄存器名称
		 */
		QRAMLoad(const qram_qutrit::QRAMCircuit* qram, std::string_view reg1, std::string_view reg2)
			: QRAMLoad(qram, System::get(reg1), System::get(reg2))
		{ }

		/**
		 * @brief 无噪声实现
		 * @param state 系统状态向量
		 */
		void noise_free_impl(std::vector<System>& state) const;

		/**
		 * @brief 设置分支（内部实现）
		 * @param qram QRAM 电路指针
		 * @param state 系统状态向量
		 * @param groups 分组信息
		 */
		void _set_branches(qram_qutrit::QRAMCircuit* qram,
			const std::vector<System>& state,
			std::vector<std::pair<size_t, size_t>>& groups) const;

		/**
		 * @brief 设置分支实现（递归）
		 * @param qram QRAM 电路指针
		 * @param state 系统状态向量
		 * @param branches 分支信息
		 * @param branch_probs 分支概率
		 * @param iter_l 左迭代边界
		 * @param iter_r 右迭代边界
		 * @param groups 分组信息
		 */
		void _set_branches_impl(qram_qutrit::QRAMCircuit* qram, const std::vector<System>& state,
			decltype(qram->get_branches()) branches,
			decltype(qram->get_branch_probs()) branch_probs,
			size_t iter_l, size_t iter_r,
			std::vector<std::pair<size_t, size_t>>& groups) const;

		/**
		 * @brief 重构操作
		 * @param qram QRAM 电路指针
		 * @param state 系统状态向量
		 * @param groups 分组信息
		 */
		void _reconstruct(qram_qutrit::QRAMCircuit* qram, 
			std::vector<System>& state,
			std::vector<std::pair<size_t, size_t>>& groups) const;

		/**
		 * @brief 应用 QRAM 加载操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 无噪声实现
		 * @param state CUDA 稀疏状态
		 */
		void noise_free_impl(CuSparseState& state) const;

		/**
		 * @brief CUDA 应用 QRAM 加载操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif	
	};

	/**
	 * @brief 快速 QRAM 加载操作类
	 * @details 优化版本的 QRAM 加载操作，性能更高
	 * @note 仅适配 qram_qutrit::QRAMCircuit
	 */
	struct QRAMLoadFast : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief QRAM 电路指针 */
		const qram_qutrit::QRAMCircuit* qram;

		/** @brief 地址寄存器 ID */
		size_t register_addr;

		/** @brief 数据寄存器 ID */
		size_t register_data;

		ClassControllable

		/**
		 * @brief 构造函数（ID 版本）
		 * @param qram QRAM 电路指针
		 * @param reg1 地址寄存器 ID
		 * @param reg2 数据寄存器 ID
		 */
		QRAMLoadFast(const qram_qutrit::QRAMCircuit* qram, size_t reg1, size_t reg2);

		/**
		 * @brief 构造函数（名称版本）
		 * @param qram QRAM 电路指针
		 * @param reg1 地址寄存器名称
		 * @param reg2 数据寄存器名称
		 */
		QRAMLoadFast(const qram_qutrit::QRAMCircuit* qram, std::string_view reg1, std::string_view reg2);

		/**
		 * @brief 无噪声实现
		 * @param state 系统状态向量
		 */
		void noise_free_impl(std::vector<System>& state) const;

		/**
		 * @brief 带阻尼实现
		 * @param state 系统状态向量
		 * @param qram QRAM 电路指针
		 * @param state_remove_cache 状态移除缓存
		 */
		void has_damping_impl(std::vector<System>& state, qram_qutrit::QRAMCircuit* qram, std::vector<System>& state_remove_cache) const;

		/**
		 * @brief 无阻尼实现
		 * @param state 系统状态向量
		 * @param qram QRAM 电路指针
		 * @param state_remove_cache 状态移除缓存
		 */
		void no_damping_impl(std::vector<System>& state, qram_qutrit::QRAMCircuit* qram, std::vector<System>& state_remove_cache) const;

		/**
		 * @brief 应用快速 QRAM 加载操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;
	};


	/**
	 * @brief QRAM 输入生成器
	 * @details 生成用于 QRAM 测试的随机输入状态
	 */
	struct QRAMInputGenerator
	{
		/** @brief 唯一输入集合 */
		std::set<std::pair<size_t, size_t>> unique_set;

		/** @brief 地址大小 */
		size_t addr_sz;

		/** @brief 数据大小 */
		size_t data_sz;

		/** @brief 输入大小 */
		size_t input_sz;

		/** @brief 地址分布 */
		std::uniform_int_distribution<size_t> addr_dist;

		/** @brief 数据分布 */
		std::uniform_int_distribution<size_t> data_dist;

		/** @brief 地址值 */
		size_t addr;

		/** @brief 数据值 */
		size_t data;

		/**
		 * @brief 构造函数（随机地址和数据）
		 * @param addr_sz_ 地址大小
		 * @param data_sz_ 数据大小
		 * @param input_size_ 输入大小
		 */
		QRAMInputGenerator(size_t addr_sz_, size_t data_sz_, size_t input_size_)
			: addr_sz(addr_sz_), data_sz(data_sz_), input_sz(input_size_),
			addr(std::numeric_limits<size_t>::max()), data(std::numeric_limits<size_t>::max()),
			addr_dist(0, pow2(addr_sz) - 1), data_dist(0, pow2(data_sz) - 1)
		{
			if (input_sz > pow2(addr_sz + data_sz))
			{
				input_sz = pow2(addr_sz + data_sz);
			}
		}

		/**
		 * @brief 构造函数（指定地址和数据）
		 * @param addr_sz_ 地址大小
		 * @param data_sz_ 数据大小
		 * @param input_size_ 输入大小
		 * @param addr_ 指定地址
		 * @param data_ 指定数据
		 */
		QRAMInputGenerator(size_t addr_sz_, size_t data_sz_, size_t input_size_, size_t addr_, size_t data_)
			: addr_sz(addr_sz_), data_sz(data_sz_), input_sz(input_size_),
			addr(addr_), data(data_),
			addr_dist(0, pow2(addr_sz) - 1), data_dist(0, pow2(data_sz) - 1)
		{
			if (input_sz > pow2(addr_sz + data_sz))
			{
				input_sz = pow2(addr_sz + data_sz);
			}
		}

		/**
		 * @brief 生成随机输入
		 * @return 地址和数据的随机对
		 */
		std::pair<size_t, size_t> rand_input()
		{
			return {
				addr_dist(random_engine::get_engine()),
				data_dist(random_engine::get_engine())
			};
		}

		/**
		 * @brief 验证寄存器有效性（内部使用）
		 * @param addr_ 地址寄存器 ID
		 * @param data_ 数据寄存器 ID
		 * @throws 当寄存器 ID 超出范围时抛出异常
		 */
		void _validate_registers(size_t addr_, size_t data_) const
		{
			if (addr_ >= System::CachedRegisterSize ||
				data_ >= System::CachedRegisterSize)
			{
				throw_invalid_input();
			}
		}

		/**
		 * @brief 生成输入状态（指定寄存器）
		 * @param s 系统状态向量
		 * @param addr_ 地址寄存器 ID
		 * @param data_ 数据寄存器 ID
		 */
		void generate_input(std::vector<System>& s, size_t addr_, size_t data_)
		{
			s.clear();

			if (input_sz == pow2(addr_sz + data_sz))
			{
				generate_full_input(s, addr_, data_);
				return;
			}

			unique_set.clear();
			for (size_t i = 0; i < input_sz; )
			{
				auto&& input = rand_input();
				auto&& res = unique_set.insert(input);
				if (res.second)
				{
					i++;
					s.emplace_back();
					s.back().get(addr_).value = input.first;
					s.back().get(data_).value = input.second;
				}
			}
			SortUnconditional()(s);
			Normalize()(s);
		}

		/**
		 * @brief 生成输入状态（使用预设寄存器）
		 * @param s 系统状态向量
		 */
		void generate_input(std::vector<System>& s)
		{
			_validate_registers(addr, data);
			generate_input(s, addr, data);
		}

		/**
		 * @brief 生成完整输入（所有可能的地址数据组合）
		 * @param s 系统状态向量
		 * @param addr_ 地址寄存器 ID
		 * @param data_ 数据寄存器 ID
		 */
		void generate_full_input(std::vector<System>& s, size_t addr_, size_t data_)
		{
			s.clear();
			double amplitude = 1.0 / std::sqrt(input_sz);
			for (size_t i = 0; i < pow2(addr_sz); ++i)
			{
				for (size_t j = 0; j < pow2(data_sz); ++j)
				{
					s.emplace_back();
					s.back().get(addr_).value = i;
					s.back().get(data_).value = j;
					s.back().amplitude = amplitude;

					unique_set.insert({ i,j });
				}
			}
		}
	};
}
