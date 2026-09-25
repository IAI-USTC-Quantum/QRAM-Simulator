#pragma once

#include "time_step.h"

namespace qram_simulator {
	namespace qram_qubit {

		/// State 内部布局常量：地址槽位下标（相对 nz_location）
		constexpr size_t Addr = 0;
		/// State 内部布局常量：数据槽位下标（相对 nz_location）
		constexpr size_t Data = 1;
		/// 比特基态常量：|0>
		constexpr bool ZeroState = false;
		/// 比特基态常量：|1>
		constexpr bool OneState = true;

		/**
		 * @brief 以非零基态节点集合表示的稀疏计算基态。
		 *
		 * 完整二叉 QRAM 树的状态只需记录处于 |1> 的节点位置
		 * （nz_elements），其余节点隐含为 |0>；树的层序编号满足
		 * left/right/parent 的位运算关系。所有门操作以稀疏集合的
		 * 增删实现，复杂度与非零元素数同阶。
		 */
		struct State
		{
			/// 处于 |1> 的节点位置集合（非零元素）
			std::set<size_t> nz_elements;

			using element_type = decltype(nz_elements);
			using data_type = element_type::key_type;
			using iterator_type = decltype(nz_elements.begin());

			/// 按字典序比较（set 的自然序）
			bool operator<(const State& rhs) const;
			/// 集合相等判定
			bool operator==(const State& rhs) const;
			/// 返回第 layerid 层的元素迭代器区间 [begin, end)
			std::pair<iterator_type, iterator_type> get_layer_iter(size_t layerid);
			/// 收集第 layerid 层的非零节点编号
			void get_layer_nonzero_nodes(size_t layerid, std::set<size_t>& nodes);
			/// 收集第 layerid 层的非零父节点编号
			void get_layer_nonzero_parent_nodes(size_t layerid, std::set<size_t>& nodes);
			/// 兼容接口：qubit 基态无零振幅元素可清理，恒为空操作
			inline void clear_zero_elements() {}

			/// @brief 由层号与层内位置编码非零元素位置。
			/// @param layer 层号 @param pos 层内位置 @param lr 左(0)/右(1)子节点
			/// @return 节点在 nz_elements 中的位置键
			constexpr static size_t get_nz_location(size_t layer, size_t pos, size_t lr);
			/// @brief 由节点位置与其左右子标志编码非零元素位置。
			/// @param node_location 节点位置 @param lr 左(0)/右(1)子节点
			constexpr static size_t get_nz_location(size_t node_location, size_t lr);
			/// 节点 node_location 的左子节点位置
			constexpr static size_t left_of(size_t node_location);
			/// 节点 node_location 的右子节点位置
			constexpr static size_t right_of(size_t node_location);
			/// 节点 node_location 的父节点位置
			constexpr static size_t parent_of(size_t node_location);
			/// 判断位置 nz_location 处的比特是否为 |1>
			bool state_of(size_t nz_location) const;
			/// 总线输入：把总线第 digit 位写入地址槽
			void busin(bus_t& bus, size_t digit);
			/// 总线输出：把地址槽读出到总线第 digit 位
			void busout(bus_t& bus, size_t digit);
			/// 翻转位置 nz_location 处的比特（X 门）
			void flip(size_t nz_location);
			/// 条件非门：condition 为真时翻转 nz_location
			void cnot(size_t nz_location, bool condition);
			/// 任意两个位置 nz_l / nz_r 的一般交换
			void general_swap(size_t nz_l, size_t nz_r);
			/// 节点 node_location 内部（地址槽-数据槽）交换
			void internal_swap(size_t node_location);
			/// 第 layerid 层全部节点的内部交换
			void internal_swap_layer(size_t layerid);
			/// 地址拷贝：按方向 a 把地址槽写入数据槽（FirstCopy）
			void acopy(bool a);
			/// 节点 node_location 处的受控交换（CSWAP）
			void cswap(size_t node_location);
			/// 第 layerid 层全部节点的受控交换
			void cswap_layer(size_t layerid);
			/// 把位置 qubit_id 处的比特置零（噪声 SetZero）
			void set_zero(size_t qubit_id);
			/// 清空全部非零元素（回到全 |0>）
			inline void clear() { nz_elements.clear(); }
		};

		/**
		 * @brief 带数据总线与振幅的系统态（分支轨迹的单个元素）。
		 *
		 * 由稀疏树态 State、data_size 位数据总线 data_bus 与复振幅
		 * amplitude 组成；run_* 系列把门 / 噪声操作同时作用在树态与
		 * 总线上，演化出一条轨迹。
		 */
		struct SystemState
		{
			/// 稀疏树态
			State state;
			/// 数据总线（data_size 位整数）
			bus_t data_bus;
			/// 总线位宽
			size_t bus_size;
			/// 轨迹振幅（噪声衰减作用于该系数）
			std::complex<double> amplitude = 1.0;

			using element_type = typename State::element_type;

			SystemState() = default;
			/// @brief 以给定总线初值构造。
			/// @param bus_ 总线值 @param bus_sz 总线位宽
			SystemState(bus_t bus_, size_t bus_sz) : data_bus(bus_), bus_size(bus_sz)
			{
			}
			SystemState(const SystemState& other) = default;

			/// 按 (总线值, 树态) 字典序比较
			inline bool operator<(const SystemState& rhs) const noexcept
			{
				return std::tie(data_bus, state) < std::tie(rhs.data_bus, rhs.state);
			}
			/// 相等判定（总线值与树态均相同）
			inline bool operator==(const SystemState& rhs) const noexcept
			{
				return std::tie(data_bus, state) == std::tie(rhs.data_bus, rhs.state);
			}

			/// 地址拷贝：按方向 a 写入数据槽
			void run_acopy(bool a);
			/// 节点 node 处的普通交换
			void run_swap(size_t node);
			/// 节点 node 处的受控交换
			void run_cswap(size_t node);
			/// 位置 qubit_id 处的比特翻转（噪声）
			void run_bitflip(size_t qubit_id);
			/// 位置 qubit_id 处的相位翻转（噪声，振幅取负）
			void run_phaseflip(size_t qubit_id, double depol_id);
			/// 位置 qubit_id 处的比特-相位联合翻转（噪声）
			void run_bitphaseflip(size_t qubit_id);
			/// 位置 qubit_id 处的去极化（噪声）
			void run_depolarizing(size_t qubit_id, double depol_id);
			/// 位置 qubit_id 处置零（噪声）
			void set_zero(size_t qubit_id);
			/// 总线输入第 digit 位
			void run_busin(size_t digit);
			/// 总线输出第 digit 位
			void run_busout(size_t digit);

			/// 公共振幅衰减：按非零元素数乘 sqrt(1-gamma)^|nz|
			inline void run_damp_common(double gamma)
			{
				amplitude *= std::pow(std::sqrt(1 - gamma), state.nz_elements.size());
			}
			/// 清空树态并复位振幅
			inline void clear()
			{
				state.clear();
				amplitude = 1.0;
			}
		};

		/**
		 * @brief 单条 (address, bus_input) 输入分支的轨迹容器。
		 *
		 * 持有一组 SystemState（同一输入在噪声 / 合并下分裂出的多条
		 * 轨迹），支持与数据树对照的保真度计算与输出采样。
		 */
		struct Branch {

			/// 架构标识：qubit
			constexpr static int qunit_type = arch_qubit;

			using state_type = SystemState;
			using element_type = typename state_type::element_type;
			/// Damping 概率数组类型（qubit 架构单元素）
			using damp_prob_type = std::array<double, 1>;

			/// 分支地址
			size_t address = 0;
			/// 总线位宽
			size_t bus_size = 1;
			/// 输入总线值
			bus_t bus_input = 0;
			/// Damping 相对乘子（相对基准好分支，预测振幅用）
			double relative_multiplier = 1;
			/// 好分支的预测基准（第一条好分支指针）
			Branch* good_ref = nullptr;
		private:
			bool good = false;
		public:

			/// 该分支的全部轨迹（含分裂）
			std::vector<SystemState> system_states;
			/// 有效轨迹条数（system_states 的前 system_states_sz 条有效）
			size_t system_states_sz = 0;
			/// 分支编号：(address << bus_size) + bus_input
			inline size_t get_branchid() const { return (address << bus_size) + bus_input; }

			/// 标记本分支为好分支并绑定预测基准
			void set_good(Branch* first_good_ptr);

			/// 本分支是否为好分支（装载结果与数据树一致）
			inline bool is_good() const
			{
				return good;
			}

			/// 有效轨迹起始迭代器
			inline auto iterbeg()
			{
				return system_states.begin();
			}

			/// 有效轨迹起始迭代器（const）
			inline auto iterbeg() const
			{
				return system_states.begin();
			}

			/// 有效轨迹终止迭代器
			inline auto iterend()
			{
				return system_states.begin() + system_states_sz;
			}

			/// 有效轨迹终止迭代器（const）
			inline auto iterend() const
			{
				return system_states.begin() + system_states_sz;
			}

			/// 按谓词 pred 移除不满足的轨迹并收缩有效长度
			template<typename Pred>
			inline void remove_if(Pred pred)
			{
				auto iter = std::remove_if(iterbeg(), iterend(), pred);
				system_states_sz = iter - iterbeg();
			}

			/// 复位为单条初始轨迹（总线初值 bus_input），清除好分支标记
			inline void reset() {
				system_states_sz = 1;
				system_states[0] = std::move(SystemState(bus_input, bus_size));
				good = false;
				relative_multiplier = 1;
			}

			Branch() = default;
			/// @brief 以显式 (地址, 总线值) 构造。
			/// @param addr 地址 @param bus_sz 总线位宽 @param bus 总线值
			Branch(size_t addr, size_t bus_sz, bus_t bus);
			/// @brief 以分支编号构造（自动拆出地址与总线值）。
			/// @param branchid (address << bus_sz) | bus_input
			/// @param bus_sz 总线位宽
			Branch(size_t branchid, size_t bus_sz);
			Branch(const Branch& old_branch) = default;

			/// 按 (地址, 输入总线, 轨迹集合) 字典序比较
			inline bool operator<(const Branch& other) const {
				return std::tie(address, bus_input, system_states) <
					std::tie(other.address, other.bus_input, other.system_states);
			}

			/// 相等判定
			inline bool operator==(const Branch& other) const {
				return std::tie(address, bus_input, system_states) ==
					std::tie(other.address, other.bus_input, other.system_states);
			}

			/// 本分支轨迹的概率总和（振幅模方和）
			double get_prob() const;
			/// 与数据树对照的保真度振幅（未取模方）
			complex_t get_fidelity(const memory_t& memory) const;
			/// 返回可读字符串表示
			std::string to_string() const;
			/// 从数据树 memory 取第 digit 位到总线（FetchData）
			void run_fetchdata(memory_t& memory, size_t digit);
			/// 总线 Hadamard 变换
			void run_hadamard();
			/// 合并相同的轨迹（振幅相加）
			void try_merge();

			/// 取好分支基准轨迹的树态（本分支为好分支时有效）
			std::optional<element_type> get_good_branch_system() const;

			/// 公共振幅衰减（同 SystemState::run_damp_common）
			void run_damp_common(double gamma);

			/// 返回 Damping 下的轨迹概率（按 qubit_id 处激发数修正）
			damp_prob_type get_prob_damp(size_t qubit_id) const;

			/// 完整振幅衰减：qubit_id 处按衰减步数 k 分裂轨迹
			void run_damp_full(size_t qubit_id, size_t k);

			/// 移除与目标树态不匹配的轨迹（输出采样后清理）
			void remove_mismatch_state(const State::element_type& target_state);
			/// 移除全部轨迹
			void remove_all_state();

			/// 清空有效轨迹（置 system_states_sz = 0）
			inline void clear_all_state() { system_states_sz = 0; }

		};

		/**
		 * @brief 同一地址的全部输入分支聚合（qubit 架构的剪枝单元）。
		 *
		 * BranchGroup 以地址为单位聚合 Branch：好地址的组只需物化基准
		 * 分支（XOR 镜像预测），坏地址的组逐条演化；组级提供概率、
		 * 保真度与输出采样接口，QRAMCircuit 以 BranchGroup 为粒度
		 * 管理 good/bad 剪枝。
		 */
		struct BranchGroup
		{
			/// 组地址
			size_t address;
			/// 输入侧分支 |ui(input)>
			std::vector<Branch> branches_input; // |ui(input)>
			/// 演化后的分支 |ui>
			std::vector<Branch> branches;       // |ui>
			/// 各分支概率 <ui|ui>
			std::vector<double> branch_probs;   // <ui|ui>
			/// 各态概率 <ψi|ψi>
			std::vector<double> state_probs;    // <ψi|ψi>

			/// 本组是否为好组（地址不在坏分支区间）
			bool is_good = false;
			/// 好组的预测基准（第一好组指针）
			BranchGroup* good_ref = nullptr;
			/// Damping 相对乘子（相对基准好组）
			double relative_multiplier = 1.0;
			/* set once the good group's final states have been materialized
			from the reference branch (XOR mirror), after which the generic
			prob/fidelity paths apply */
			/// 基准好分支的系统态已物化（XOR 镜像）后置位，此后走通用概率/保真度路径
			bool predicted = false;

			/// @brief 以地址 addr 构造空组。
			BranchGroup(size_t addr) : address(addr)
			{}

			/// 复位全部分支与标记
			void reset();
			/// 标记为好组并绑定预测基准
			void set_good(BranchGroup* good_ref_);
			/// 清空全部分支态（保留分支框架）
			void set_empty_state();

			/// 与数据树对照的保真度振幅（未取模方）
			complex_t get_fidelity(const memory_t& memory) const;

			/// Damping 下的轨迹概率
			Branch::damp_prob_type get_prob_damp(size_t qubit_id) const;

			/// 本组全部分支的概率总和
			double get_prob() const;

			/// 无 Damping 输出采样：r 为均匀随机数，返回是否采样成功
			bool sample_output_no_damping(Branch::element_type& output, double& r);
			/// 含 Damping 输出采样：r 为均匀随机数，返回是否采样成功
			bool sample_output_with_damping(Branch::element_type& output, double& r);

			/// 移除与目标树态不匹配的分支态
			void remove_mismatch_state(const Branch::element_type& target);
		};
	} // namespace qram_qubit
} // namespace qram_simulator
