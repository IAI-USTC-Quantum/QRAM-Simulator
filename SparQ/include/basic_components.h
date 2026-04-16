/**
 * @file basic_components.h
 * @brief 基础组件定义
 * @details 定义稀疏态模拟器的核心数据结构和基础类，包括状态存储、系统管理、算子基类等
 */

#pragma once

#include "qram_circuit_qutrit.h"
#include "global_macros.h"
#include "cuda/cuda_utils.cuh"

namespace qram_simulator
{
	/** @namespace qram_simulator
	 * @brief QRAM 稀疏态模拟器命名空间
	 */

	/** @typedef StateInfoType
	 * @brief 状态信息类型
	 * @details 包含寄存器名称、状态存储类型、大小和激活状态的元组
	 */
	using StateInfoType = std::tuple<std::string, StateStorageType, size_t, bool>;

	/**
	 * @brief 获取状态信息中的名称（const 版本）
	 * @param m 状态信息元组
	 * @return 寄存器名称的 const 引用
	 */
	const std::string& get_name(const StateInfoType& m);

	/**
	 * @brief 获取状态信息中的名称（非 const 版本）
	 * @param m 状态信息元组
	 * @return 寄存器名称的引用
	 */
	std::string& get_name(StateInfoType& m);

	/**
	 * @brief 获取状态信息中的类型（const 版本）
	 * @param m 状态信息元组
	 * @return 状态存储类型的 const 引用
	 */
	const StateStorageType& get_type(const StateInfoType& m);

	/**
	 * @brief 获取状态信息中的类型（非 const 版本）
	 * @param m 状态信息元组
	 * @return 状态存储类型的引用
	 */
	StateStorageType& get_type(StateInfoType& m);

	/**
	 * @brief 获取状态信息中的大小
	 * @param m 状态信息元组
	 * @return 寄存器大小
	 */
	size_t get_size(const StateInfoType& m);

	/**
	 * @brief 获取状态信息中的大小引用
	 * @param m 状态信息元组
	 * @return 寄存器大小的引用
	 */
	size_t& get_size(StateInfoType& m);

	/**
	 * @brief 获取状态信息中的激活状态
	 * @param m 状态信息元组
	 * @return 寄存器激活状态
	 */
	bool get_status(const StateInfoType& m);

	/**
	 * @brief 获取状态信息中的激活状态引用
	 * @param m 状态信息元组
	 * @return 寄存器激活状态的引用
	 */
	bool& get_status(StateInfoType& m);

	/**
	 * @brief 状态存储结构
	 * @details 量子寄存器状态的实际存储单元，使用 uint64_t 存储基础值
	 */
	struct StateStorage
	{
		/** @brief 实际存储的值 */
		uint64_t value = 0;

		/**
		 * @brief 将值解释为指定类型
		 * @tparam Ty 目标类型（支持有符号/无符号整数、浮点数、布尔值）
		 * @param size 位数
		 * @return 转换后的值
		 * @throws 当类型不支持时抛出异常
		 */
		template<typename Ty>
		Ty as(size_t size) const
		{
			uint64_t truncated_value = value & (pow2(size) - (size != 0));

			if constexpr (std::is_floating_point_v<Ty>)
			{
				if (size == 64) return truncated_value * 1.0 / 2 / pow2(63);
				return 1.0 * truncated_value / pow2(size);
			}
			else if constexpr (std::is_same_v<Ty, bool>)
			{
				return bool(truncated_value);
			}
			else if constexpr (std::is_integral_v<Ty>)
			{
				if constexpr (std::is_signed_v<Ty>)
				{
					return get_complement(truncated_value, size);
				}
				else
				{
					return truncated_value;
				}
			}
			else {
				throw_invalid_input();
			}
		}

		/**
		 * @brief 构造函数
		 */
		HOST_DEVICE StateStorage() {}

		/**
		 * @brief 安全访问值（非 const 版本）
		 * @param size 位数
		 * @return 值的引用
		 */
		HOST_DEVICE uint64_t& val(size_t size);

		/**
		 * @brief 安全访问值（const 版本）
		 * @param size 位数
		 * @return 值
		 */
		HOST_DEVICE uint64_t val(size_t size) const;

		/**
		 * @brief 相等比较运算符
		 * @param rhs 右侧操作数
		 * @return 是否相等
		 */
		HOST_DEVICE bool operator==(const StateStorage& rhs) const;

		/**
		 * @brief 不等比较运算符
		 * @param rhs 右侧操作数
		 * @return 是否不等
		 */
		HOST_DEVICE bool operator!=(const StateStorage& rhs) const;

		/**
		 * @brief 小于比较运算符
		 * @param rhs 右侧操作数
		 * @return 是否小于
		 */
		HOST_DEVICE bool operator<(const StateStorage& rhs) const;

		/**
		 * @brief 大于比较运算符
		 * @param rhs 右侧操作数
		 * @return 是否大于
		 */
		HOST_DEVICE bool operator>(const StateStorage& rhs) const;

		/**
		 * @brief 转换为字符串
		 * @param info 状态信息
		 * @return 字符串表示
		 */
		std::string to_string(const StateInfoType& info) const;

		/**
		 * @brief 转换为 IO 字符串
		 * @param info 状态信息
		 * @return IO 格式的字符串
		 */
		std::string to_io_string(const StateInfoType& info) const;

		/**
		 * @brief 转换为二进制字符串
		 * @param info 状态信息
		 * @return 二进制格式的字符串
		 */
		std::string to_binary_string(const StateInfoType& info) const;

		/**
		 * @brief 翻转指定位
		 * @param digit 位索引
		 */
		HOST_DEVICE void flip(size_t digit);
	};

	/** @brief 前向声明：稀疏状态 */
	struct SparseState;

#ifdef USE_CUDA
	/** @brief 前向声明：CUDA 稀疏状态 */
	struct CuSparseState;
#endif

	/**
	 * @brief 系统类
	 * @details 管理量子寄存器和系统状态的核心类，包含静态寄存器信息和动态状态数据
	 */
	struct System 
	{
#ifdef CACHED_REGISTER_SIZE
		/** @brief 缓存寄存器大小 */
		constexpr static size_t CachedRegisterSize = CACHED_REGISTER_SIZE;
#else
		/** @brief 默认缓存寄存器大小 */
		constexpr static size_t CachedRegisterSize = 40;
#endif

		/** @brief 寄存器信息映射表 */
		inline static std::vector<StateInfoType> name_register_map;

		/** @brief 寄存器状态位图 */
		inline static uint64_t reg_status_bitmap = 0;

		/** @brief 最大量子比特数统计 */
		inline static size_t max_qubit_count = 0;

		/** @brief 最大寄存器数统计 */
		inline static size_t max_register_count = 0;

		/** @brief 最大系统大小统计 */
		inline static size_t max_system_size = 0;

		/** @brief 临时寄存器栈 */
		inline static std::vector<size_t> temporal_registers;

		/** @brief 可重用寄存器列表 */
		inline static std::vector<size_t> reusable_registers;

		/** @brief 状态振幅 */
		complex_t amplitude = 1.0;

		/** @brief 寄存器存储数组 */
		std::array<StateStorage, CachedRegisterSize> registers;

		/**
		 * @brief 获取指定位置的状态组件（非 const 版本）
		 * @param id 寄存器 ID
		 * @return 状态存储的引用
		 */
		StateStorage& get(size_t id);

		/**
		 * @brief 获取指定位置的状态组件（const 版本）
		 * @param id 寄存器 ID
		 * @return 状态存储的 const 引用
		 */
		const StateStorage& get(size_t id) const;

		/**
		 * @brief 清除寄存器分配信息
		 */
		static void clear();

		/**
		 * @brief 获取量子比特总数
		 * @return 量子比特数
		 */
		static size_t get_qubit_count();

		/**
		 * @brief 获取已激活寄存器数量
		 * @return 激活寄存器数
		 */
		static size_t get_activated_register_size();

		/**
		 * @brief 获取最后一个激活的寄存器 ID
		 * @return 寄存器 ID
		 */
		static size_t get_last_activated_register();

		/**
		 * @brief 访问最后一个激活的寄存器（非 const 版本）
		 * @return 状态存储的引用
		 */
		StateStorage& last_register();

		/**
		 * @brief 访问最后一个激活的寄存器（const 版本）
		 * @return 状态存储的 const 引用
		 */
		const StateStorage& last_register() const;

		/**
		 * @brief 更新最大系统大小
		 * @param new_size 新大小
		 */
		static void update_max_size(size_t new_size);

		/**
		 * @brief 根据名称获取寄存器 ID
		 * @param name 寄存器名称
		 * @return 寄存器 ID
		 */
		static size_t get(std::string_view name);

		/**
		 * @brief 根据名称获取寄存器信息
		 * @param name 寄存器名称
		 * @return 状态信息
		 */
		static StateInfoType get_register_info(std::string_view name);

		/**
		 * @brief 根据 ID 获取寄存器名称
		 * @param id 寄存器 ID
		 * @return 寄存器名称的 const 引用
		 */
		static const std::string& name_of(size_t id);

		/**
		 * @brief 根据名称获取寄存器大小
		 * @param name 寄存器名称
		 * @return 寄存器大小
		 */
		static size_t size_of(std::string_view name);

		/**
		 * @brief 根据 ID 获取寄存器大小
		 * @param id 寄存器 ID
		 * @return 寄存器大小
		 */
		static size_t size_of(size_t id);

		/**
		 * @brief 根据名称获取寄存器类型
		 * @param name 寄存器名称
		 * @return 状态存储类型
		 */
		static StateStorageType type_of(std::string_view name);

		/**
		 * @brief 根据 ID 获取寄存器类型
		 * @param id 寄存器 ID
		 * @return 状态存储类型
		 */
		static StateStorageType type_of(size_t id);

		/**
		 * @brief 根据名称获取寄存器激活状态
		 * @param name 寄存器名称
		 * @return 激活状态（true = 激活）
		 */
		static bool status_of(std::string_view name);

		/**
		 * @brief 根据 ID 获取寄存器激活状态
		 * @param id 寄存器 ID
		 * @return 激活状态（true = 激活）
		 */
		static bool status_of(size_t id);

		/**
		 * @brief 添加寄存器状态位图标记
		 * @param pos 位置
		 */
		static void add_register_status_bitmap(size_t pos);

		/**
		 * @brief 移除寄存器状态位图标记
		 * @param pos 位置
		 */
		static void remove_register_status_bitmap(size_t pos);

		/**
		 * @brief 添加新寄存器
		 * @param name 寄存器名称
		 * @param type 状态存储类型
		 * @param size 寄存器大小
		 * @return 寄存器 ID
		 */
		static size_t add_register(std::string_view name, StateStorageType type, size_t size);

		/**
		 * @brief 同步添加寄存器（初始状态为0）
		 * @param name 寄存器名称
		 * @param type 状态存储类型
		 * @param size 寄存器大小
		 * @param system_states 系统状态向量
		 * @return 寄存器 ID
		 */
		static size_t add_register_synchronous(
			std::string_view name, StateStorageType type, size_t size,
			std::vector<System>& system_states);

		/**
		 * @brief 同步添加寄存器（初始状态为0，SparseState 版本）
		 * @param name 寄存器名称
		 * @param type 状态存储类型
		 * @param size 寄存器大小
		 * @param system_states 稀疏状态
		 * @return 寄存器 ID
		 */
		static size_t add_register_synchronous(
			std::string_view name, StateStorageType type, size_t size,
			SparseState& system_states);

		/**
		 * @brief 根据 ID 移除寄存器
		 * @param id 寄存器 ID
		 */
		static void remove_register(size_t id);

		/**
		 * @brief 根据名称移除寄存器
		 * @param name 寄存器名称
		 */
		static void remove_register(std::string_view name);

		/**
		 * @brief 同步移除寄存器（根据 ID）
		 * @param id 寄存器 ID
		 * @param state 系统状态向量
		 */
		static void remove_register_synchronous(size_t id,
			std::vector<System>& state);

		/**
		 * @brief 同步移除寄存器（根据名称）
		 * @param name 寄存器名称
		 * @param state 系统状态向量
		 */
		static void remove_register_synchronous(std::string_view name,
			std::vector<System>& state);

		/**
		 * @brief 同步移除寄存器（SparseState 版本，根据 ID）
		 * @param id 寄存器 ID
		 * @param state 稀疏状态
		 */
		static void remove_register_synchronous(size_t id,
			SparseState& state);

		/**
		 * @brief 同步移除寄存器（SparseState 版本，根据名称）
		 * @param name 寄存器名称
		 * @param state 稀疏状态
		 */
		static void remove_register_synchronous(std::string_view name,
			SparseState& state);

		/**
		 * @brief 构造函数
		 */
		HOST_DEVICE System() {}

		/**
		 * @brief 小于比较运算符
		 * @param rhs 右侧系统
		 * @return 是否小于
		 */
		HOST_DEVICE bool operator<(const System& rhs) const;

		/**
		 * @brief 相等比较运算符
		 * @param rhs 右侧系统
		 * @return 是否相等
		 */
		HOST_DEVICE bool operator==(const System& rhs) const;

		/**
		 * @brief 不等比较运算符
		 * @param rhs 右侧系统
		 * @return 是否不等
		 */
		HOST_DEVICE bool operator!=(const System& rhs) const;

		/**
		 * @brief 转换为字符串
		 * @return 字符串表示
		 */
		std::string to_string() const;

		/**
		 * @brief 转换为字符串（指定精度）
		 * @param precision 精度
		 * @return 字符串表示
		 */
		std::string to_string(int precision) const;
	};

	/**
	 * @brief 合并两个系统
	 * @details 将 s2 的振幅加到 s1，并将 s2.amplitude 设为 0
	 * @param s1 第一个系统（目标）
	 * @param s2 第二个系统（源）
	 */
	void merge_system(System& s1, System& s2);

	/**
	 * @brief 移除接近零的系统
	 * @param s 系统
	 * @return 如果应该被移除则返回 true
	 */
	bool remove_system(const System& s);

	/**
	 * @brief 稀疏状态类
	 * @details 使用基态向量表示的稀疏量子状态
	 */
	struct SparseState
	{
		/** @brief 基态向量 */
		std::vector<System> basis_states;

		/** @brief 向量类型别名 */
		using vector_type = std::vector<System>;

		/**
		 * @brief 默认构造函数
		 */
		SparseState() {
			basis_states.emplace_back();
		}

		/**
		 * @brief 指定大小构造函数
		 * @param size 大小
		 */
		SparseState(size_t size)
			: basis_states(size) {}

		/**
		 * @brief 拷贝构造函数
		 * @param basis_states_ 基态向量
		 */
		SparseState(const std::vector<System>& basis_states_) 
			: basis_states(basis_states_) {}

		/**
		 * @brief 移动构造函数
		 * @param basis_states_ 基态向量
		 */
		SparseState(std::vector<System>&& basis_states_)
			: basis_states(std::move(basis_states_)) {}

		/**
		 * @brief 拷贝构造函数
		 * @param other 另一个稀疏状态
		 */
		SparseState(const SparseState& other)
			: basis_states(other.basis_states) {}

		/**
		 * @brief 移动构造函数
		 * @param other 另一个稀疏状态
		 */
		SparseState(SparseState&& other)
			: basis_states(std::move(other.basis_states)) {}

		/**
		 * @brief 拷贝赋值运算符
		 * @param other 另一个稀疏状态
		 * @return 自身引用
		 */
		SparseState& operator=(const SparseState& other) {
			basis_states = other.basis_states;
			return *this;
		}

		/**
		 * @brief 移动赋值运算符
		 * @param other 另一个稀疏状态
		 * @return 自身引用
		 */
		SparseState& operator=(SparseState&& other) {
			basis_states = std::move(other.basis_states);
			return *this;
		}

		/**
		 * @brief 获取最后一个元素（非 const 版本）
		 * @return 最后一个系统的引用
		 */
		System& back() { return basis_states.back(); }

		/**
		 * @brief 获取最后一个元素（const 版本）
		 * @return 最后一个系统的 const 引用
		 */
		const System& back() const { return basis_states.back(); }

		/**
		 * @brief 获取起始迭代器
		 * @return 起始迭代器
		 */
		vector_type::iterator begin() { return basis_states.begin(); }
		vector_type::const_iterator begin() const { return basis_states.begin(); }
		vector_type::iterator end() { return basis_states.end(); }
		vector_type::const_iterator end() const { return basis_states.end(); }
		vector_type::reverse_iterator rbegin() { return basis_states.rbegin(); }
		vector_type::const_reverse_iterator rbegin() const { return basis_states.rbegin(); }
		vector_type::reverse_iterator rend() { return basis_states.rend(); }
		vector_type::const_reverse_iterator rend() const { return basis_states.rend(); }

		/**
		 * @brief 下标访问运算符
		 * @param i 索引
		 * @return 系统的引用
		 */
		System& operator[](size_t i) { return basis_states[i]; }
		const System& operator[](size_t i) const { return basis_states[i]; }

		/**
		 * @brief 获取大小
		 * @return 基态数量
		 */
		size_t size() const { return basis_states.size(); }

		/**
		 * @brief 检查是否为空
		 * @return 是否为空
		 */
		bool empty() const { return basis_states.empty(); }
	};


	/** @brief 设备类型枚举 */
	enum DeviceType { CPU, GPU, ANY };

	/** @brief 前向声明：CUDA 稀疏状态 */
	struct CuSparseState;

	/**
	 * @brief 算子基类
	 * @details 所有量子算子的抽象基类，定义了算子的基本接口
	 */
	struct BaseOperator
	{
		/**
		 * @brief 应用算子（纯虚函数）
		 * @param state 系统状态向量
		 */
		virtual void operator()(std::vector<System>& state) const = 0;

		/**
		 * @brief 应用共轭转置（ dagger ）操作
		 * @param state 系统状态向量
		 * @throws 默认抛出未实现异常
		 */
		inline virtual void dag(std::vector<System>& state) const
		{
			throw_not_implemented("Dagger is not implemented.");
		}

		/**
		 * @brief 应用算子到 SparseState
		 * @param state 稀疏状态
		 */
		inline void operator()(SparseState& state) const
		{
			(*this)(state.basis_states);
		}

		/**
		 * @brief 应用 dagger 到 SparseState
		 * @param state 稀疏状态
		 */
		inline virtual void dag(SparseState& state) const
		{
			this->dag(state.basis_states);
		}
#ifdef USE_CUDA
		/**
		 * @brief 应用算子到 CuSparseState（CUDA 版本）
		 * @param state CUDA 稀疏状态
		 */
		virtual void operator()(CuSparseState& state) const;

		/**
		 * @brief 应用 dagger 到 CuSparseState（CUDA 版本）
		 * @param state CUDA 稀疏状态
		 */
		virtual void dag(CuSparseState& state) const;
#endif
	};

	/**
	 * @brief 自伴算子类
	 * @details 继承自 BaseOperator，自伴算子的 dagger 等于自身
	 */
	class SelfAdjointOperator : public BaseOperator {
	public:
		using BaseOperator::operator();
		using BaseOperator::dag;

		/**
		 * @brief 应用 dagger 操作（自伴算子 dagger 等于自身）
		 * @param state 系统状态向量
		 */
		inline void dag(std::vector<System>& state) const override {
			(*this)(state);
		}

		/**
		 * @brief 应用 dagger 到 SparseState
		 * @param state 稀疏状态
		 */
		inline void dag(SparseState& state) const override {
			(*this)(state);
		}

#ifdef USE_CUDA
		/**
		 * @brief 应用 dagger 到 CuSparseState
		 * @param state CUDA 稀疏状态
		 */
		void dag(CuSparseState& state) const override;
#endif
	};


#ifdef SINGLE_THREAD
	/** @brief 执行策略：单线程 */
	constexpr auto exec_policy = std::execution::seq;
#else
	/** @brief 执行策略：并行 */
	constexpr auto exec_policy = std::execution::par;
#endif


}
