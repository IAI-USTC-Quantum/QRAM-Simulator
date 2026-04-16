/**
 * @file system_operations.h
 * @brief 系统操作定义
 * @details 实现量子系统的基本操作，包括系统分割、合并、重置、
 *          寄存器分割/合并/移动/添加/移除等操作
 */

#pragma once
#include "basic_components.h"
#include "debugger.h"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/**
	 * @brief 分割系统（原地修改版本）
	 * @param new_state 新状态向量
	 * @param old_state 旧状态向量
	 * @param condition_variable_nonzeros 非零条件变量
	 * @param condition_variable_all_ones 全1条件变量
	 * @param condition_variable_by_bit 按位条件变量
	 * @param condition_variable_by_value 按值条件变量
	 */
	void split_systems(std::vector<System>& new_state, std::vector<System>& old_state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	/**
	 * @brief 分割系统（返回值版本）
	 * @param state 状态向量
	 * @param condition_variable_nonzeros 非零条件变量
	 * @param condition_variable_all_ones 全1条件变量
	 * @param condition_variable_by_bit 按位条件变量
	 * @param condition_variable_by_value 按值条件变量
	 * @return 分割后的状态向量
	 */
	std::vector<System> split_systems(std::vector<System>& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	/**
	 * @brief 分割稀疏状态
	 * @param state 稀疏状态
	 * @param condition_variable_nonzeros 非零条件变量
	 * @param condition_variable_all_ones 全1条件变量
	 * @param condition_variable_by_bit 按位条件变量
	 * @param condition_variable_by_value 按值条件变量
	 * @return 分割后的稀疏状态
	 */
	SparseState split_systems(SparseState& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	/**
	 * @brief 合并系统（到目标）
	 * @param to 目标状态向量
	 * @param from 源状态向量
	 */
	void combine_systems(std::vector<System>& to, const std::vector<System>& from);

	/**
	 * @brief 合并稀疏状态（到目标）
	 * @param to 目标稀疏状态
	 * @param from 源稀疏状态
	 */
	void combine_systems(SparseState& to, const SparseState& from);

#ifdef USE_CUDA
	/**
	 * @brief CUDA 分割稀疏状态
	 * @param state CUDA 稀疏状态
	 * @param condition_variable_nonzeros 非零条件变量
	 * @param condition_variable_all_ones 全1条件变量
	 * @param condition_variable_by_bit 按位条件变量
	 * @param condition_variable_by_value 按值条件变量
	 * @return 分割后的 CUDA 稀疏状态
	 */
	CuSparseState split_systems(CuSparseState& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	/**
	 * @brief CUDA 合并稀疏状态（到目标）
	 * @param to 目标 CUDA 稀疏状态
	 * @param from 源 CUDA 稀疏状态
	 */
	void combine_systems(CuSparseState& to, CuSparseState& from);
#endif

	/**
	 * @def SPLIT_BY_CONDITIONS
	 * @brief 按条件分割系统的宏
	 */
#define SPLIT_BY_CONDITIONS \
	std::decay_t<decltype(state)> unconditioned_state(0);\
	if (HasCondition)\
	{\
		unconditioned_state = split_systems(state,\
			condition_variable_nonzeros,\
			condition_variable_all_ones,\
			condition_variable_by_bit,\
			condition_variable_by_value\
		);\
	}\
	if (state.size())

	/**
	 * @def MERGE_BY_CONDITIONS
	 * @brief 按条件合并系统的宏
	 */
#define MERGE_BY_CONDITIONS \
	if (!unconditioned_state.empty()) {\
		combine_systems(state, unconditioned_state);\
	}

	/**
	 * @brief 重置系统
	 * @param state 系统状态向量
	 */
	void reset_systems(std::vector<System>& state);

	/**
	 * @brief 重置稀疏状态
	 * @param state 稀疏状态
	 */
	void reset_systems(SparseState& state);

#ifdef USE_CUDA
	/**
	 * @brief CUDA 重置稀疏状态
	 * @param state CUDA 稀疏状态
	 */
	void reset_systems(CuSparseState& state);
#endif

	/**
	 * @brief 添加系统（带系数）
	 * @param current_state 当前状态向量
	 * @param new_state 新状态向量
	 * @param coef 系数
	 */
	inline void add_systems(std::vector<System>& current_state, const std::vector<System>& new_state, double coef)
	{
		if (new_state.size() == 0) return;
		size_t original_size = current_state.size();		

		current_state.insert(current_state.end(), new_state.begin(), new_state.end());

		if (std::abs(coef - 1.0) > epsilon)
		{
			for (auto iter = current_state.begin() + original_size; iter != current_state.end(); ++iter)
			{
				iter->amplitude *= coef;
			}
		}

		if (original_size != 0)
			sort_merge_unique_erase(current_state, std::less<System>(),
				std::equal_to<System>(), merge_system, remove_system);
	}

	/**
	 * @brief 添加稀疏状态（带系数）
	 * @param current 当前稀疏状态
	 * @param new_state 新稀疏状态
	 * @param coef 系数
	 */
	inline void add_systems(SparseState& current, const SparseState& new_state, double coef)
	{
		return add_systems(current.basis_states, new_state.basis_states, coef);
	}

#ifdef USE_CUDA
	/**
	 * @brief CUDA 添加稀疏状态（带系数）
	 * @param current 当前 CUDA 稀疏状态
	 * @param new_state 新 CUDA 稀疏状态
	 * @param coef 系数
	 */
	void add_systems(CuSparseState& current, const CuSparseState& new_state, double coef);
#endif

	/**
	 * @brief 分割寄存器操作
	 * @details 将一个寄存器分割为两个寄存器
	 */
	struct SplitRegister {
		/** @brief 第一个寄存器名称 */
		std::string first_name;

		/** @brief 第二个寄存器名称 */
		std::string second_name;

		/** @brief 第二个寄存器大小 */
		size_t second_size;

		/**
		 * @brief 构造函数（ID 版本）
		 * @param first_id_ 第一个寄存器 ID
		 * @param second_name_ 第二个寄存器名称
		 * @param second_size_ 第二个寄存器大小
		 */
		SplitRegister(size_t first_id_, std::string_view second_name_, size_t second_size_)
			: first_name(System::name_of(first_id_)), second_name(second_name_), second_size(second_size_) { }

		/**
		 * @brief 构造函数（名称版本）
		 * @param first_name_ 第一个寄存器名称
		 * @param second_name_ 第二个寄存器名称
		 * @param second_size_ 第二个寄存器大小
		 */
		SplitRegister(std::string_view first_name_, std::string_view second_name_, size_t second_size_)
			: first_name(first_name_), second_name(second_name_), second_size(second_size_) {}

		/**
		 * @brief 应用分割操作
		 * @param state 系统状态向量
		 * @return 新寄存器 ID
		 */
		size_t operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用分割操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 新寄存器 ID
		 */
		size_t operator()(SparseState& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用分割操作
		 * @param state CUDA 稀疏状态
		 * @return 新寄存器 ID
		 */
		size_t operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 合并寄存器操作
	 * @details 合并两个寄存器（移除第二个）
	 */
	struct CombineRegister {
		/** @brief 第一个寄存器名称 */
		std::string first_name;

		/** @brief 第二个寄存器名称 */
		std::string second_name;

		/**
		 * @brief 构造函数（ID 版本）
		 * @param first_id_ 第一个寄存器 ID
		 * @param second_id_ 第二个寄存器 ID
		 */
		CombineRegister(size_t first_id_, size_t second_id_)
			: first_name(System::name_of(first_id_)), second_name(System::name_of(second_id_)) { }

		/**
		 * @brief 构造函数（名称版本）
		 * @param first_name_ 第一个寄存器名称
		 * @param second_name_ 第二个寄存器名称
		 */
		CombineRegister(std::string_view first_name_, std::string_view second_name_)
			: first_name(first_name_), second_name(second_name_) { }

		/**
		 * @brief 应用合并操作
		 * @param state 系统状态向量
		 * @return 合并后寄存器 ID
		 */
		size_t operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用合并操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 合并后寄存器 ID
		 */
		size_t operator()(SparseState& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用合并操作
		 * @param state CUDA 稀疏状态
		 * @return 合并后寄存器 ID
		 */
		size_t operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 寄存器后移操作
	 * @details 改变寄存器位置（不安全，不应作为子流程调用）
	 */
	struct MoveBackRegister
	{
		/** @brief 寄存器 ID */
		size_t register_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_in 寄存器名称
		 */
		MoveBackRegister(std::string_view reg_in);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_in 寄存器 ID
		 */
		MoveBackRegister(size_t reg_in);

		/**
		 * @brief 应用后移操作
		 * @param states 系统状态向量
		 */
		void operator()(std::vector<System>& states) const;

		/**
		 * @brief 应用后移操作（稀疏状态版本）
		 * @param state 稀疏状态
		 */
		void operator()(SparseState& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用后移操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 添加寄存器操作
	 * @details 添加新寄存器到系统
	 */
	struct AddRegister {
		/** @brief 寄存器名称 */
		std::string register_name;

		/** @brief 寄存器类型 */
		StateStorageType type;

		/** @brief 寄存器大小 */
		size_t size;

		/**
		 * @brief 构造函数
		 * @param register_name_ 寄存器名称
		 * @param type_ 寄存器类型
		 * @param size_ 寄存器大小
		 */
		AddRegister(std::string_view register_name_, StateStorageType type_, size_t size_);

		/**
		 * @brief 应用添加操作
		 * @param state 系统状态向量
		 * @return 新寄存器 ID
		 */
		size_t operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用添加操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 新寄存器 ID
		 */
		size_t operator()(SparseState& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用添加操作
		 * @param state CUDA 稀疏状态
		 * @return 新寄存器 ID
		 */
		size_t operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 添加寄存器并应用 Hadamard 操作
	 * @details 添加新寄存器并初始化为均匀叠加态
	 */
	struct AddRegisterWithHadamard {
		/** @brief 寄存器名称 */
		std::string register_name;

		/** @brief 寄存器类型 */
		StateStorageType type;

		/** @brief 寄存器大小 */
		size_t size;

		/**
		 * @brief 构造函数
		 * @param register_name_ 寄存器名称
		 * @param type_ 寄存器类型
		 * @param size_ 寄存器大小
		 */
		AddRegisterWithHadamard(std::string_view register_name_, StateStorageType type_, size_t size_);

		/**
		 * @brief 应用添加并 Hadamard 操作
		 * @param state 系统状态向量
		 * @return 新寄存器 ID
		 */
		size_t operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用添加并 Hadamard 操作（稀疏状态版本）
		 * @param state 稀疏状态
		 * @return 新寄存器 ID
		 */
		size_t operator()(SparseState& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用添加并 Hadamard 操作
		 * @param state CUDA 稀疏状态
		 * @return 新寄存器 ID
		 */
		size_t operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 移除寄存器操作
	 * @details 从系统中移除指定寄存器
	 */
	struct RemoveRegister {
		/** @brief 寄存器 ID */
		size_t register_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param register_name 寄存器名称
		 */
		RemoveRegister(std::string_view register_name);

		/**
		 * @brief 构造函数（ID 版本）
		 * @param register_name_ 寄存器 ID
		 */
		RemoveRegister(size_t register_name_);

		/**
		 * @brief 应用移除操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

		/**
		 * @brief 应用移除操作（稀疏状态版本）
		 * @param state 稀疏状态
		 */
		void operator()(SparseState& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用移除操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 压栈操作
	 * @details 将寄存器状态压入临时栈
	 */
	struct Push : BaseOperator
	{
		using BaseOperator::operator();

		/** @brief 临时寄存器名称 */
		std::string garbage_name;

		/** @brief 寄存器 ID */
		size_t reg_id;

		/**
		 * @brief 构造函数（名称版本）
		 * @param regname_ 寄存器名称
		 * @param garbage_name_ 临时寄存器名称
		 */
		Push(std::string_view regname_, std::string_view garbage_name_)
			:reg_id(System::get(regname_)), garbage_name(garbage_name_)
		{ }

		/**
		 * @brief 构造函数（ID 版本）
		 * @param regname_ 寄存器 ID
		 * @param garbage_name_ 临时寄存器名称
		 */
		Push(size_t regname_, std::string_view garbage_name_)
			: reg_id(regname_), garbage_name(garbage_name_)
		{ }

		/**
		 * @brief 应用压栈操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用压栈操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 出栈操作
	 * @details 从临时栈恢复寄存器状态
	 */
	struct Pop : BaseOperator
	{
		using BaseOperator::operator();

		/** @brief 寄存器 ID */
		size_t reg_id;

		/** @brief 寄存器名称 */
		std::string reg_name;

		/**
		 * @brief 构造函数（名称版本）
		 * @param reg_name_ 寄存器名称
		 */
		Pop(std::string_view reg_name_) : reg_id(System::get(reg_name_)), reg_name(reg_name_)
		{ }

		/**
		 * @brief 构造函数（ID 版本）
		 * @param reg_name_ 寄存器 ID
		 */
		Pop(size_t reg_name_) : reg_id(reg_name_)
		{ }

		/**
		 * @brief 应用出栈操作
		 * @param state 系统状态向量
		 */
		void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用出栈操作
		 * @param state CUDA 稀疏状态
		 */
		void operator()(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 清零操作
	 * @details 移除振幅接近零的状态分量
	 */
	struct ClearZero : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		/** @brief 清零阈值 */
		double eps;

		/**
		 * @brief 默认构造函数（使用默认 epsilon）
		 */
		ClearZero() :eps(epsilon) {};

		/**
		 * @brief 构造函数（指定阈值）
		 * @param eps_ 清零阈值
		 */
		ClearZero(double eps_) :eps(eps_) {};

		/**
		 * @brief 应用清零操作
		 * @param system_states 系统状态向量
		 */
		void operator()(std::vector<System>& system_states) const;

#ifdef USE_CUDA
		/**
		 * @brief CUDA 应用清零操作
		 * @param s CUDA 稀疏状态
		 */
		void operator()(CuSparseState& s) const;
#endif
	};

	/**
	 * @brief 状态加载操作
	 * @details 从文件加载量子状态
	 */
	struct StateLoad
	{
		/** @brief 主寄存器名称 */
		std::string main_reg;

		/** @brief 辅助寄存器 UA 名称 */
		std::string anc_UA;

		/** @brief 辅助寄存器 4 名称 */
		std::string anc_4;

		/** @brief 辅助寄存器 3 名称 */
		std::string anc_3;

		/** @brief 辅助寄存器 2 名称 */
		std::string anc_2;

		/** @brief 辅助寄存器 1 名称 */
		std::string anc_1;

		/** @brief 数据大小 */
		size_t data_size;

		/** @brief 有理数大小 */
		size_t rational_size;

		/** @brief 保存名称 */
		std::string savename;

		/**
		 * @brief 构造函数
		 * @param main_reg 主寄存器名称
		 * @param anc_UA 辅助寄存器 UA 名称
		 * @param anc_4 辅助寄存器 4 名称
		 * @param anc_3 辅助寄存器 3 名称
		 * @param anc_2 辅助寄存器 2 名称
		 * @param anc_1 辅助寄存器 1 名称
		 * @param ds 数据大小
		 * @param rs 有理数大小
		 */
		StateLoad(
			std::string main_reg,
			std::string anc_UA,
			std::string anc_4,
			std::string anc_3,
			std::string anc_2,
			std::string anc_1,
			size_t ds,
			size_t rs) :
			main_reg(main_reg), anc_UA(anc_UA), anc_4(anc_4), anc_3(anc_3), anc_2(anc_2), anc_1(anc_1),
			data_size(ds), rational_size(rs) {}

		/**
		 * @brief 从文件加载状态
		 * @param savename_ 文件名
		 * @return 系统状态向量
		 */
		std::vector<System> operator()(const std::string& savename_) const;

		/**
		 * @brief 解析振幅值
		 * @param line 文件行
		 * @return 复数振幅
		 */
		complex_t load_amplitude(const std::string& line) const;

		/**
		 * @brief 解析寄存器值
		 * @param line 文件行
		 * @param reg 寄存器名称
		 * @return 寄存器值
		 */
		size_t load_reg(const std::string& line, const std::string& reg) const;

		/**
		 * @brief 解析分支
		 * @param line 文件行
		 * @return 系统状态
		 */
		System load_branch(const std::string& line) const;

		/**
		 * @brief 检查是否为分支行
		 * @param line 文件行
		 * @return 是否为分支行
		 */
		bool is_branch(const std::string& line) const;
	};

	/**
	 * @brief 将状态打印到文件
	 * @param state 系统状态向量
	 * @param filename 文件名
	 * @param precision 精度（默认16）
	 * @throws 当文件无法打开时抛出异常
	 */
	inline void print_state_to_file(std::vector<System>& state, const std::string &filename, int precision = 16)
	{
		{
			std::ofstream f_state(filename);
			if (!f_state.is_open()) {
				throw std::runtime_error("Failed to open file.");
			}
			f_state.close();
		}
		if (precision == 0)
			throw_invalid_input();

		{
			std::ofstream f_state(filename, std::ios_base::app);
			if (!f_state.is_open()) {
				throw std::runtime_error("Failed to open file.");
			}
			f_state << fmt::format("Print State To File:\n");

			for (auto& s : state) {
				f_state << fmt::format("{}\n", s.to_string(precision));
			}

			f_state.close();
		}
	}
}
