#pragma once

#include "time_step.h"

namespace qram_simulator {
	namespace qram_qutrit {
		/**
		 * @brief 单个 qutrit 树节点：地址能级 + 数据能级。
		 *
		 * 地址能级 addr ∈ {W=-1, L=0, R=1}（W 为总线/等待能级），
		 * 数据能级 data ∈ {0, 1}。内联门操作以能级跃迁实现：旋转门
		 * A1/A2（含 w 相位的三能级酉）、内部交换、数据翻转等。
		 */
		struct QRAMNode
		{
			/// 地址能级：W / L / R
			int addr = 0;
			/// 数据能级：0 / 1
			int data = 0;

			/// 转可读字符串 "(A,D)"，如 "(W,0)"
			operator std::string() const {
				return fmt::format("({},{})", addr2str(addr), data);
			}

			QRAMNode() = default;
			/// @brief 以能级构造。 @param a 地址能级 @param d 数据能级
			QRAMNode(int a, int d) : addr(a), data(d) {}

			/// 数据能级翻转 0↔1
			inline void data_flip() { data ^= 1; }
			/// 地址能级翻转 L↔R（W 不变）
			inline void addr_flip()
			{
				switch (addr) {
				case L:
					addr = R;
					return;
				case R:
					addr = L;
					return;
				default:
					return;
				}
			}
			/// 数据能级置 0
			inline void data_setzero() { data = 0; }
			/// 数据能级置 1
			inline void data_setone() { data = 1; }
			/// 数据能级置指定值
			inline void data_set(bool value) { data = bool2int(value); }
			/// 经典条件数据翻转：classical_data 为真时翻转
			inline void data_control_flip(bool classical_data)
			{
				if (classical_data) data_flip();
			}
			/// 节点内地址-数据能级交换（W 与 data 之间的搬运）
			inline void internal_swap()
			{
				if (addr == W)
				{
					addr = data;
					data = 0;
				}
				else {
					if (data == 0) {
						data = addr;
						addr = W;
					}
				}
			}

			/*
			A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
				 [ 0 0 1 ]	     [ 0 w 0  ]
				 [ 1 0 0 ]	     [ 0 0 w^2]
			A1   = L->W, W->R, R->L
			*/
			/// A1 旋转门（L→W→R→L）。返回是否产生 data==0 的可区分态
			inline bool rotate_A1()
			{
				if (addr == L) {
					addr = W;
					return data == 0;
				}
				if (addr == W) {
					addr = R;
					return false;
				}
				if (addr == R) {
					addr = L;
				}
				return false;
			}

			/// A2 旋转门（R→W，L→R，W→L）。返回是否产生 data==0 的可区分态
			inline bool rotate_A2()
			{
				if (addr == R) {
					addr = W;
					return data == 0;
				}
				if (addr == L) addr = R;
				if (addr == W) addr = L;
				return false;
			}

			/// 是否处于 (W, 0) 态
			inline bool check_0() const
			{
				if (addr == W && data == 0) return true;
				else return false;
			}
			/// 按 (addr, data) 字典序比较
			inline bool operator<(const QRAMNode& rhs) const
			{
				return std::tie(addr, data) < std::tie(rhs.addr, rhs.data);
			}

			/// 能级相等判定
			inline bool operator==(const QRAMNode& rhs) const
			{
				return std::tie(addr, data) == std::tie(rhs.addr, rhs.data);
			}
		};

		/**
		 * @brief 以非零节点映射表示的稀疏 qutrit 树态。
		 *
		 * 只记录非 (W,0) 基态的节点（nz_elements，按位置有序），其余
		 * 节点隐含为 (W,0)。门操作以映射的增删改实现。
		 */
		struct QRAMState
		{
			using value_type = QRAMNode;

			// sparse storage and keep sorted
			/// 非基态节点映射：节点位置 → 节点能级（保持有序）
			using element_type = std::map<size_t, QRAMNode>;
			using iterator_t = typename element_type::iterator;
			using const_iterator_t = typename element_type::const_iterator;

			element_type nz_elements;

			QRAMState() = default;

			/// 比较两个稀疏树态（映射字典序）
			bool operator<(const QRAMState& rhs) const;
			/// 稀疏树态相等判定
			bool operator==(const QRAMState& rhs) const;

			/// 清理振幅为零的元素（qutrit 树态下为兼容接口）
			void clear_zero_elements();

			/// @brief 由层号与层内位置编码节点位置（完全二叉树层序编号）。
			constexpr static size_t get_nnz_location(size_t layer, size_t pos)
			{
				size_t begin = pow2(layer) - 1;
				return (begin + pos);
			}

			/// 节点 node_location 的左子节点位置
			constexpr static size_t left_of(size_t node_location)
			{
				return 2 * node_location + 1;
			}
			/// 节点 node_location 的右子节点位置
			constexpr static size_t right_of(size_t node_location)
			{
				return 2 * node_location + 2;
			}
			/// 节点 node_location 的父节点位置
			constexpr static size_t parent_of(size_t node_location)
			{
				return (node_location - 1) / 2;
			}

			/* Time: O(log n) */
			/// 取位置 nnz_location 处的迭代器（O(log n)）
			iterator_t iterof(size_t nnz_location);
			/// 取位置 nnz_location 处的常量迭代器（O(log n)）
			const_iterator_t iterof(size_t nnz_location) const;

			/* Time: O(1) */
			/// 尾迭代器（O(1)）
			iterator_t iterend();
			/// 尾常量迭代器（O(1)）
			const_iterator_t iterend() const;

			/// 总线输入：把总线第 digit 位写入节点数据能级
			void busin(bus_t& bus, size_t digit);
			/// 总线输出：把节点数据能级读出到总线第 digit 位
			void busout(bus_t& bus, size_t digit);
			/// 地址拷贝：按方向 a 把地址能级写入数据能级（FirstCopy）
			void acopy(bool a);

			/// 无条件节点内交换（internal_swap 的底层实现）
			void _unconditional_internal_swap(size_t node_location);
			/// 条件节点内交换（数据能级为 0 时执行）
			void _impl_conditional_internal_swap(size_t node_location);
			/// 第 layerid 层全部节点的内部交换
			void internal_swap_layer(size_t layerid);
			/// 把迭代器 iter 处节点移动到 target 位置
			void _impl_iter_swap(iterator_t iter, size_t target);

			/// 节点 node_location 处的受控交换（CSWAP）
			void cswap(size_t node_location);
			/// 第 layerid 层全部节点的受控交换
			void cswap_layer(size_t layerid);
			/// 翻转位置 qubit_id 处节点的数据能级（噪声）
			void flip(size_t qubit_id);

			/*
			A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
				 [ 0 0 1 ]	     [ 0 w 0  ]
				 [ 1 0 0 ]	     [ 0 0 w^2]
			A1   = L->W, W->R, R->L
			A1^2 = L->R, W->L, R->W
			*/
			/// 在位置 qubit_id 处执行 A1 旋转门（相位 w）
			void rotate_A1(size_t qubit_id);
			/// 在位置 qubit_id 处执行 A2 旋转门（相位 w^2）
			void rotate_A2(size_t qubit_id);

			/// 判断位置 qubit_id 处节点是否为非基态（返回能级标记）
			int state_of(size_t qubit_id) const;
			/// 把位置 qubit_id 处节点置回基态 (W,0)（噪声 SetZero）
			void set_zero(size_t qubit_id);

			/// 返回可读字符串表示
			std::string to_string() const;

		};

		/**
		 * @brief 带数据总线与振幅的 qutrit 系统态（轨迹单元）。
		 *
		 * 由稀疏树态 QRAMState、数据总线 data_bus 与复振幅组成，
		 * run_* 系列执行门 / 噪声操作，演化出一条轨迹。
		 */
		struct SubBranch
		{
			/// 稀疏 qutrit 树态
			QRAMState state;
			/// 数据总线
			bus_t data_bus;
			/// 总线位宽
			size_t data_size;
			/// 轨迹振幅（噪声衰减作用于该系数）
			std::complex<double> amplitude = 1.0;

			using element_type = typename QRAMState::element_type;

			SubBranch() = default;
			/// @brief 以总线初值构造。 @param bus_ 总线值 @param bus_sz 总线位宽
			SubBranch(memory_entry_t bus_, size_t bus_sz) : data_size(bus_sz), data_bus(bus_)
			{ }

			/// 按 (总线值, 树态) 字典序比较
			inline bool operator<(const SubBranch& rhs) const noexcept
			{
				return std::tie(data_bus, state) < std::tie(rhs.data_bus, rhs.state);
			}

			/// 相等判定
			inline bool operator==(const SubBranch& rhs) const noexcept
			{
				return std::tie(data_bus, state) == std::tie(rhs.data_bus, rhs.state);
			}

			/// 地址拷贝：按方向 a 写入数据能级
			void run_acopy(bool a);
			/// 节点 node 处的普通交换
			void run_swap(size_t node);
			/// 节点 node 处的受控交换
			void run_cswap(size_t node);
			/// 位置 qubit_id 处的数据能级翻转（噪声）
			void run_bitflip(size_t qubit_id);
			/// 位置 qubit_id 处的 A1 旋转门
			void run_A1(size_t qubit_id);
			/// 位置 qubit_id 处的 A1^2 旋转门
			void run_A1_2(size_t qubit_id);
			/// 位置 qubit_id 处的 A2 旋转门
			void run_A2(size_t qubit_id);
			/// 位置 qubit_id 处的 A2^2 旋转门
			void run_A2_2(size_t qubit_id);
			/// 位置 qubit_id 处的相位翻转（噪声，振幅取负）
			void run_phaseflip(size_t qubit_id, double depol_id);
			/// 位置 qubit_id 处的比特-相位联合翻转（噪声）
			void run_bitphaseflip(size_t qubit_id);
			/// 位置 qubit_id 处的去极化（噪声）
			void run_depolarizing(size_t qubit_id, double depol_id);
			/// 位置 qubit_id 处置回基态（噪声）
			void set_zero(size_t qubit_id);
			/// 总线输入第 digit 位
			void run_busin(size_t digit);
			/// 总线输出第 digit 位
			void run_busout(size_t digit);
		};

		/**
		 * @brief 单条 (address, bus_input) 输入分支的轨迹容器（qutrit 架构）。
		 *
		 * 持有一组 SubBranch 轨迹，提供与数据树对照的保真度、输出采样
		 * 与 Damping 乘子计算；get_multiplier 转发到 TimeStep 的
		 * qutrit 实现。
		 */
		struct Branch {
			/// 架构标识：qutrit
			constexpr static int arch_type = arch_qutrit;
			/// Damping 相关操作数（qutrit 架构为 2）
			constexpr static int damp_op_num = 2;

			using state_type = SubBranch;
			using element_type = typename state_type::element_type;
			/// Damping 概率数组类型（qutrit 架构双元素）
			using damp_prob_type = std::array<double, 2>;

			/// 分支地址
			size_t address = 0;
			/// 总线位宽
			size_t bus_size = 1;
			/// 输入总线值
			memory_entry_t bus_input = 0;
			/// Damping 相对乘子（相对基准好分支）
			double relative_multiplier = 1;
			/// 好分支的预测基准（第一条好分支指针）
			Branch* good_ref = nullptr;
			/// 本分支的输入概率
			double branch_prob = 0.0;
		private:
			bool good = false;
			std::vector<SubBranch> system_states;
		public:
			Branch() = default;
			/// @brief 以显式 (地址, 总线值) 构造。
			Branch(size_t address_, size_t bus_sz_, memory_entry_t bus_input_)
				: address(address_), bus_size(bus_sz_), bus_input(bus_input_)
			{ }
			Branch(const Branch& old_branch) = default;
			/// @brief 以分支编号构造（自动拆出地址与总线值）。
			/// @param branchid (address << bus_sz) | bus_input
			/// @param bus_sz 总线位宽
			Branch(size_t branchid, size_t bus_sz)
			{
				bus_size = bus_sz;
				address = branchid >> bus_sz;
				bus_input = branchid - (address << bus_sz);
			}

			/// 分支编号：(address << bus_size) + bus_input
			inline size_t get_branchid() const
			{
				return (address << bus_size) + bus_input;
			}

			/// 轨迹起始迭代器（const）
			auto iterbeg() const { return system_states.begin(); }
			/// 轨迹终止迭代器（const）
			auto iterend() const { return system_states.end(); }
			/// 轨迹起始迭代器
			auto iterbeg() { return system_states.begin(); }
			/// 轨迹终止迭代器
			auto iterend() { return system_states.end(); }
			/// 轨迹条数
			auto system_size() const { return system_states.size(); }

			/// 按谓词 pred 移除不满足的轨迹（物理删除）
			template<typename Pred>
			inline void remove_if(Pred pred)
			{
				auto iter = std::remove_if(system_states.begin(), system_states.end(), pred);
				system_states.erase(iter, system_states.end());
			}

			/// 复位为单条初始轨迹（总线初值 bus_input），清除好分支标记
			inline void reset()
			{
				system_states.clear();
				system_states.push_back(SubBranch(bus_input, bus_size));
				good = false;
				relative_multiplier = 1;
			}

			/// 标记本分支为好分支并绑定预测基准
			void set_good(Branch* first_good_ptr);
			/// 本分支是否为好分支
			inline bool is_good() const { return good; }

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

			/// 本分支轨迹的概率总和
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

			/// 公共振幅衰减
			void run_damp_common(double gamma);

			/// Damping 下的轨迹概率
			damp_prob_type get_prob_damp(size_t qubit_id) const;

			/// 完整振幅衰减：qubit_id 处按衰减步数 k 分裂轨迹
			void run_damp_full(size_t qubit_id, size_t k);

			/// @brief 为好分支计算 Damping 相对乘子（转发 TimeStep::get_multiplier_qutrit）。
			/// @param gamma Damping 衰减率
			/// @param time_step 调度器
			/// @param step 时间片
			/// @param branches 分支集合
			/// @param first_good_branch 基准好分支下标
			/// @param good_branch_ids 其余好分支下标
			/// @param memory 数据树
			inline static void get_multiplier(
				double gamma,
				const TimeStep& time_step,
				size_t step,
				std::vector<Branch>& branches,
				size_t first_good_branch,
				const std::vector<size_t>& good_branch_ids,
				/*std::vector<double>& multipliers,*/
				const memory_t& memory
			)
			{
				time_step.get_multiplier_qutrit(gamma, step, branches, first_good_branch, good_branch_ids,/* multipliers,*/ memory);
			}

			/// 移除与目标树态不匹配的轨迹
			void remove_mismatch_state(const QRAMState::element_type& target_state);
			/// 移除全部轨迹
			void remove_all_state();

			/// 清空全部轨迹
			inline void clear_all_state() { system_states.clear(); }
		};
	} // namespace qram_qutrit
} // namespace qram_simulator

/// QRAMNode 的 fmt 格式化特化：输出 "(A,D)" 形式
template <> struct fmt::formatter<qram_simulator::qram_qutrit::QRAMNode> {
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}
	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(const qram_simulator::qram_qutrit::QRAMNode& p,	FormatContext& ctx) const -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "{}", std::string(p));
	}
};
