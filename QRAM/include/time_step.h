#pragma once

#include "basic.h"

namespace qram_simulator {

	/// qutrit 架构标识常量（TimeStep::generate 的 arch_type 参数取值）
	constexpr int arch_qutrit = 0x1A1A;
	/// qubit 架构标识常量（TimeStep::generate 的 arch_type 参数取值）
	constexpr int arch_qubit = 0x1B1B;

	/**
	 * @brief QRAM 操作类型枚举。
	 *
	 * 既是噪声模型的键类型（noise_t 的键），也是 Operation 的类型标签。
	 * 噪声成员的取值为发生概率，须在 [0, 1] 内；其中 Damping 要求
	 * gamma < 1（gamma = 1 会在一步内衰灭全部激发，破坏采样与归一化）。
	 */
	enum class OperationType {
		Begin,             ///< 枚举哨兵（不使用）
		ControlSwap,       ///< 受控交换（路由）
		HadamardData,      ///< 数据总线上的 Hadamard
		CopyIn,            ///< 拷入（busin）
		CopyOut,           ///< 拷出（busout）
		SwapInternal,      ///< 节点内部交换
		FirstCopy,         ///< 首层拷贝（acopy）
		FetchData,         ///< 从数据树取数（fetchdata）
		// Noise Op
		SetZero,           ///< 噪声：置零
		Damping,           ///< 噪声：振幅衰减（gamma < 1）
		Damp_Common,       ///< 噪声：公共振幅衰减
		Damp_Full,         ///< 噪声：完整振幅衰减
		BitFlip,           ///< 噪声：比特翻转
		PhaseFlip,         ///< 噪声：相位翻转
		BitPhaseFlip,      ///< 噪声：比特-相位联合翻转
		Depolarizing,      ///< 噪声：去极化
		// Identity
		Identity_Active,   ///< 恒等操作（活跃比特）
		Identity_Inactive, ///< 恒等操作（非活跃比特）
		End                ///< 枚举哨兵（不使用）
	};

	/// 噪声模型：{操作类型: 发生概率}
	using noise_t = std::map<OperationType, double>;

	/**
	 * @brief 单个量子操作。
	 *
	 * 描述一次门 / 噪声事件：类型、目标比特（或节点）编号及可选的
	 * 噪声系数（如 Damping 的 gamma）。
	 */
	struct Operation {
		/// 操作类型
		OperationType type;
		/// 目标比特 / 节点编号
		std::vector<size_t> targets;
		/// 噪声系数（可选）
		std::vector<double> coefficients;
		/// 是否为共轭转置（逆）操作
		bool dagger = false;

		Operation(OperationType type_, std::vector<size_t> targets_)
			:type(type_), targets(targets_)
		{ }

		Operation(OperationType type_, std::vector<size_t> targets_, std::vector<double> coefs)
			:type(type_), targets(targets_), coefficients(coefs)
		{ }

		Operation(const Operation& oldop) = default;

		/// 返回目标编号列表
		inline std::vector<size_t> get_targets() const { return targets; }
		/// 返回本操作的逆操作
		Operation reverse();
		/// 返回可读字符串表示
		std::string to_string() const;
	};

	/**
	 * @brief 同一时间片内并发执行的操作集合。
	 *
	 * QRAM 树中互不相邻的节点可在同一时间片并行操作，OperationPack
	 * 把它们打包为一个调度单元，以 name 标识。
	 */
	struct OperationPack {
		/// 本时间片内的操作列表
		std::list<Operation> operations;
		/// 操作组名称（调度调试用）
		std::string name;

		/// 返回逆操作组：按原顺序逐个取逆后重排
		inline OperationPack reverse() {
			OperationPack ret;
			for (auto iter = operations.rbegin(); iter != operations.rend(); ++iter) {
				ret.operations.push_back(iter->reverse());
			}
			ret.name = name + "^";
			return ret;
		}
		/// 本操作组是否为空
		inline bool empty() const { return operations.size() == 0; }
		/// 设置操作组名称
		inline void set_name(std::string s) { name = s; }
		/// 追加单个操作
		inline void append(Operation op) { operations.push_back(op); }
		/// @brief 追加另一操作组的全部操作并拼接名称。
		/// @param ops 被并入的操作组
		inline void append(OperationPack ops) {
			for (auto& op : ops.operations) {
				append(op);
			}
			name += "->";
			name += ops.name;
		}
		/// 返回可读字符串表示
		std::string to_string() const;
	};

	/**
	 * @brief 完整 QRAM 装载调度：按时间片顺序排列的 OperationPack 序列。
	 */
	struct TimeSlices {
		/// 时间片列表（第 i 个元素为第 i 个时间片的操作组）
		std::vector<OperationPack> time_slices;

		/// 清空调度
		inline void clear() {
			time_slices.clear();
		}
		/// 追加单个时间片
		inline void append(const OperationPack &op) {
			time_slices.emplace_back(op);
		}
		/// 追加另一调度的全部时间片
		inline void append(const TimeSlices &ts) {
			for (auto& tslice : ts.time_slices) {
				append(tslice);
			}
		}
		/// 返回逆调度：整体反序并逐组取逆
		inline TimeSlices reverse() {
			TimeSlices ret;
			for (auto iter = time_slices.rbegin(); iter != time_slices.rend(); ++iter) {
				ret.time_slices.push_back(iter->reverse());
			}
			return ret;
		}

		/// 返回可读字符串表示
		std::string to_string() const;
	};

	/**
	 * @brief 连续区间集合（坏分支区间的合并表示）。
	 *
	 * 以 [l1,r1],[l2,r2],... 的有序不相交区间集合表示地址集合，
	 * merge 按 4 种相交情形归并新区间 [l,r]，accept 判断地址 t 是否
	 * 落在集合内。用于 TimeStep 的 bad range 计算。
	 */
	struct ContinuousRange
	{
		/// 区间端点序列：[l1,r1,l2,r2,...]
		std::vector<int> range_bound;

		// [l1,r1],[l2,r2],[l3,r3],[l4,r4] new=[l5,r5]
		// (in x, out y)
		// (in x, in y)
		// (out x, out y)
		// (out x, in y)
		ContinuousRange() = default;
		/// 构造单区间 [l, r]
		ContinuousRange(int l, int r);

		/// 清空全部区间
		void clear();
		/// 归并新区间 [l, r]（保持有序不相交）
		void merge(int l, int r);
		/// 判断 t 是否落在任一区间内
		bool accept(int t) const;
		/// 返回可读字符串表示
		operator std::string();
	};

	/**
	 * @brief QRAM 装载电路的时序与噪声调度器。
	 *
	 * 依据地址 / 数据宽度推导每个时间片的路由、拷贝与交换操作
	 * （generate_step），并可按噪声模型在每个时间片后插入噪声操作，
	 * 生成完整 TimeSlices 调度（generate）。QRAMCircuit 内部持有
	 * TimeStep 实例。另提供坏分支区间（bad range）推导：给定某一
	 * 出错的比特 / 节点，得到会装载出错误数据的地址区间。
	 */
	struct TimeStep
	{
		/// 地址位宽
		size_t addr_size;
		/// 数据位宽
		size_t data_size;

		/// 坏分支区间集合（fill_bad_range 填充）
		ContinuousRange cr;
		/// 无噪调度缓存（init_noise_free 生成）
		TimeSlices time_slices_noise_free;

		/**
		 * @brief 构造调度器。
		 * @param addr_sz 地址位宽
		 * @param data_sz 数据位宽
		 */
		TimeStep(size_t addr_sz, size_t data_sz);
		/// 完整装载流程的总时间片数
		size_t full_step() const;
		/// 返回第 in_step 步对应的输出（拷出）时间片编号
		size_t out(size_t in_step) const;
		/// 最后一个时间片编号
		size_t last_step() const;

		/// 第 step 步的地址拷贝目标层（方向），无操作返回 -1
		int acopy(size_t step) const;
		/// 第 step 步地址拷贝对应的输出时间片，无则返回 -1
		int acopy_out(size_t step) const;
		/// 第 step 步的数据拷贝目标位，无操作返回 -1
		int dcopy(size_t step) const;
		/// 第 step 步数据拷贝对应的输出时间片，无则返回 -1
		int dcopy_out(size_t step) const;
		/// 第 layer 层路由阶段的起始时间片
		size_t route_begin(size_t layer) const;
		/// 第 layer 层路由等待阶段的起始时间片
		size_t route_wait_begin(size_t layer) const;
		/// 第 step 步在 layer 层是否走奇数（右）子树路由
		bool route_odd(size_t step, size_t layer) const;
		/// 第 step 步在 layer 层是否走偶数（左）子树路由
		bool route_even(size_t step, size_t layer) const;
		/// 第 step 步在 layer 层的路由判定（奇偶分发）
		bool route(size_t step, size_t layer) const;
		/// 第 step 步的内存拷贝位，无操作返回 -1
		int memory_copy(size_t step) const;
		/// 第 time_step 步的节点内部交换目标，无操作返回 -1
		int internal_swap(size_t time_step) const;
		/// 第 time_step 步内部交换对应的输出时间片，无则返回 -1
		int internal_swap_out(size_t time_step) const;

		/// 生成第 step 个时间片的操作组（不含噪声）
		OperationPack generate_step(size_t step) const;
		/// 第 time_step 步可纠缠的最大层数（噪声插入范围依据）
		size_t layer_entangle_max(size_t time_step) const;

		/// 判断地址 addr 是否落在坏分支区间内
		bool is_bad_branch(size_t addr) const;

		/// qubit 架构：由坏比特 bad_qubit 推导坏分支地址区间 [lo, hi]
		std::pair<size_t, size_t> get_bad_range_qubit(size_t bad_qubit) const;
		/// qutrit 架构：由坏比特 bad_qubit 推导坏分支地址区间 [lo, hi]
		std::pair<size_t, size_t> get_bad_range_qutrit(size_t bad_qubit) const;

		/// @brief 填充坏分支区间集合 cr。
		/// @param qubit 出错的比特 / 节点
		/// @param arch_type 架构常量（arch_qubit / arch_qutrit）
		void fill_bad_range(size_t qubit, int arch_type);
		/// @brief 向 pack 追加一个时间片后的噪声操作。
		/// @param pack 目标操作组
		/// @param max_entangle_layer 该时间片可纠缠的最大层数
		/// @param noises 噪声模型
		/// @param arch_type 架构常量
		void noise_one_step(OperationPack& pack, size_t max_entangle_layer,
			const std::map<OperationType, double>& noises, int arch_type);

		/// @brief 生成一个时间片的噪声操作（不插入任何 OperationPack）。
		/// @param max_entangle_layer 可纠缠的最大层数
		/// @param noises 噪声模型
		/// @param arch_type 架构常量
		void noise_one_step(size_t max_entangle_layer,
			const std::map<OperationType, double>& noises, int arch_type);

		/// 生成无噪调度并缓存到 time_slices_noise_free
		void init_noise_free();
		/// 仅追加噪声区间（不生成含噪操作序列）
		void append_noise_range_only(const std::map<OperationType, double>& noises, int arch_type);
		/// 在缓存的无噪调度上插入噪声，返回含噪调度
		TimeSlices append_noise(const std::map<OperationType, double>& noises, int arch_type);
		/**
		 * @brief 生成完整含噪调度。
		 * @param noises 噪声模型（可为空，即无噪调度）
		 * @param arch_type 架构常量（arch_qubit / arch_qutrit）
		 * @return 含噪的 TimeSlices 调度
		 */
		TimeSlices generate(const std::map<OperationType, double>& noises, int arch_type);

		/// qubit 架构：第 step 步地址 addr 的阻尼乘子指数实现
		ptrdiff_t _get_multiplier_impl_qubit(
			size_t step, size_t addr) const;

		/// qutrit 架构：按 branchid 计算阻尼乘子指数实现
		ptrdiff_t _get_multiplier_impl_qutrit(
			size_t step, size_t branch_id, const memory_t& mem) const;

		/// qutrit 架构：按 (addr, data) 计算阻尼乘子指数实现
		ptrdiff_t _get_multiplier_impl_qutrit(
			size_t step, size_t addr, size_t data, const memory_t& mem) const;

		/**
		 * @brief qubit 架构：为好分支计算 Damping 相对乘子。
		 *
		 * 以第一条好分支为基准，按各好分支地址的阻尼指数差设置
		 * branch.relative_multiplier = (1-gamma)^(指数差)。
		 * @param gamma Damping 衰减率
		 * @param step 时间片
		 * @param branches 分支集合
		 * @param first_good_branch 基准好分支下标
		 * @param good_branch_ids 其余好分支下标
		 */
		template<typename BranchType>
		inline void get_multiplier_qubit(
			double gamma,
			size_t step,
			std::vector<BranchType>& branches,
			size_t first_good_branch,
			const std::vector<size_t>& good_branch_ids
		) const
		{
			ptrdiff_t addr_ref_count, addr_count;
			addr_ref_count = _get_multiplier_impl_qubit(step,
				branches[first_good_branch].address);

			for (size_t i = 0; i < good_branch_ids.size(); ++i)
			{
				size_t id = good_branch_ids[i];
				auto& branch = branches[id];

				addr_count = _get_multiplier_impl_qubit(step,
					branches[id].address);
				branch.relative_multiplier = std::pow(1 - gamma, addr_count - addr_ref_count);
			}
		}

		/**
		 * @brief qutrit 架构：为好分支计算 Damping 相对乘子。
		 *
		 * 语义同 get_multiplier_qubit，阻尼乘子按 branchid 与数据树
		 * 内容推导。
		 * @param gamma Damping 衰减率
		 * @param step 时间片
		 * @param branches 分支集合
		 * @param first_good_branch 基准好分支下标
		 * @param good_branch_ids 其余好分支下标
		 * @param memory 数据树
		 */
		template<typename BranchType>
		void get_multiplier_qutrit(
			double gamma,
			size_t step,
			std::vector<BranchType>& branches,
			size_t first_good_branch,
			const std::vector<size_t>& good_branch_ids,
			const memory_t& memory
		) const
		{
			ptrdiff_t addr_ref_count, addr_count;
			addr_ref_count = _get_multiplier_impl_qutrit(step,
				branches[first_good_branch].get_branchid(),
				memory);

			//double l = std::sqrt(1 - gamma);
			double l = 1 - gamma;
			for (size_t i = 0; i < good_branch_ids.size(); ++i)
			{
				size_t id = good_branch_ids[i];
				auto& branch = branches[id];

				addr_count = _get_multiplier_impl_qutrit(step,
					branches[good_branch_ids[i]].get_branchid(), memory);
				branch.relative_multiplier = std::pow(l, addr_count - addr_ref_count);
			}
		}

		/// 打印全部时间片的调度详情到 stdout（fmt 输出）
		inline void print() const
		{
			for (size_t step = 1; step < full_step(); ++step)
			{
				auto ops = generate_step(step);
				fmt::print("step={}\nentangle_max={}\n{}\n", step, layer_entangle_max(step), ops.to_string());
			}
		}

	};

	/// 返回第 pos 个位置在第 layer 层的左右路由序列（左右子树走向）
	std::vector<bool> calc_pos(int pos, int layer);
	/// 返回第 layer 层的全部节点编号
	std::vector<size_t> get_nodes_in_layer(int layer);
	/// 返回第 pos 个位置在第 layer 层的二进制路由串
	std::string pos2str(int pos, int layer);
	/// 返回操作类型的缩写名
	std::string type2str(OperationType type);

	/// bool 转 "1"/"0"
	constexpr const char* bool2char(bool x)
	{
		return x ? "1" : "0";
	}

	/// bool 转 "True"/"False"
	constexpr const char* bool2str(bool x)
	{
		return x ? "True" : "False";
	}

	/// bool 转 0/1
	constexpr int bool2int(bool x)
	{
		return x ? 1 : 0;
	}

	/// bool 转 Pauli 基符号 "+"/"-"
	constexpr const char* bool2char_pmbasis(bool x)
	{
		return x ? "+" : "-";
	}

	/// qutrit 地址能级转字符：W→'W'，L→'L'，R→'R'，其余→'?'
	constexpr char addr2str(int addr)
	{
		if (addr == W) { return 'W'; }
		if (addr == L) { return 'L'; }
		if (addr == R) { return 'R'; }
		else { return '?'; }
	}

	/// 架构常量转名称："qutrit" / "qubit" / "unknown_arch"
	constexpr const char* arch2str(int arch)
	{
		if (arch == arch_qutrit) return "qutrit";
		if (arch == arch_qubit) return "qubit";
		return "unknown_arch";
	}

	/// 噪声模型转可读串，如 "[BitFlip=0.01,Damping=0.001]"
	inline std::string noise2str(const noise_t& noises)
	{
		std::string ret = "[";
		for (auto& noise : noises)
		{
			fmt::format_to(std::back_inserter(ret),
				"{}={},", type2str(noise.first), noise.second
			);
		}
		if (noises.size() > 0)
			ret.back() = ']';
		else
			ret += "]";
		return ret;
	}

}
