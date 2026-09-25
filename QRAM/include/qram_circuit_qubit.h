#pragma once

#include "qram_branch_qubit.h"

namespace qram_simulator {
	namespace qram_qubit {
		/**
		 * @brief qubit 架构 QRAM 装载电路的分支化稀疏仿真器。
		 *
		 * 以 (address, bus_input) 输入分支为轨迹载体，沿 TimeStep 生成的
		 * 含噪操作序列演化稀疏 SystemState，并通过 good/bad 分支剪枝把
		 * 仿真规模从全部分支压缩到"坏分支全算 + 一条好分支作基准"。
		 *
		 * 典型工作流：
		 * 1. 构造电路并设置数据树（set_memory_random / set_memory）
		 * 2. 可选注入噪声（set_noise_models）
		 * 3. 采样输入分支（set_input_random / set_input_uniform）
		 * 4. 运行（run_normal 剪枝 / run_full 不剪枝基准）
		 * 5. 采样输出并读取保真度（sample_and_get_fidelity）
		 */
		struct QRAMCircuit
		{
			using SystemState = typename Branch::state_type;
			using qram_state_t = typename SystemState::element_type;

		public:
			/// 地址位宽（数据树深度）
			size_t addr_size;
			/// 数据位宽（每个内存槽的比特数）
			size_t data_size;
			/// 时序与噪声调度器（generate 生成含噪操作序列）
			TimeStep time_step;
			/// 数据树：memory[i] 为地址 i 处存放的 data_size 位整数
			memory_t memory;

			/* Internal operations */
			/// 未注入噪声的原始操作序列（调试用）
			TimeSlices raw_operations;
			/// 实际执行的含噪操作序列（initialize_system 生成）
			TimeSlices operations;
			/// 噪声模型：{操作类型: 发生概率}
			noise_t noise_parameters;

			/// 全部分支组（每个地址一个 BranchGroup）
			std::vector<BranchGroup> branch_groups;
			/// 本次运行需要演化的分支组视图（剪枝后）
			std::vector<BranchGroup*> valid_branch_group_view;
			/// 第一条好分支组的下标（作为其余好分支的预测基准），-1 表示无
			ptrdiff_t first_good_branch_group = -1;
			/// 除基准外的其余好分支组下标
			std::vector<size_t> good_branch_group_ids;

		public:
			/// 采样输出分支的概率总和（归一化检查用）
			double total_prob = 0;
			/// 采样得到的最终系统态
			qram_state_t final_system_state;

			/**
			 * @brief 构造 QRAM 电路（内存未初始化）。
			 * @param address_sz 地址位宽
			 * @param data_sz 数据位宽
			 */
			QRAMCircuit(size_t address_sz, size_t data_sz);
			/**
			 * @brief 构造 QRAM 电路并以给定数据树初始化。
			 * @param address_sz 地址位宽
			 * @param data_sz 数据位宽
			 * @param memory 数据树（长度须为 2^address_sz）
			 */
			QRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory);

			/// 以全局随机引擎随机填充整棵数据树
			inline void set_memory_random() { random_memory(memory, data_size); }
			/// 设置数据树（拷贝）
			void set_memory(const memory_t& new_memory);
			/// 设置数据树（移动）
			void set_memory(memory_t&& new_memory);
			/// 返回数据树引用
			inline auto& get_memory() { return memory; }
			/// 返回数据树 const 引用
			inline auto& get_memory() const { return memory; }

			/// 电路占用的物理比特数：2 * (2^addr_size - 1)
			inline size_t get_qubit_num() const { return 2 * (pow2(addr_size) - 1); }
			/// 内存槽总数
			inline size_t memory_size() const { return memory.size(); }

			/**
			 * @brief 直接注入分支组（供 state_manipulator / 测试使用，见 init_state）。
			 * @param branch_groups_ 分支组集合
			 */
			inline void set_branches(const std::vector<BranchGroup>& branch_groups_) { branch_groups = branch_groups_; }
			/// @overload
			inline void set_branches(std::vector<BranchGroup>&& branch_groups_) { branch_groups = std::move(branch_groups_); }

			/**
			 * @brief 采样 n 条 (addr, bus) 输入分支填充 branch_groups。
			 *
			 * 每条分支的输入概率在最后统一归一化。
			 * @param n_inputs 采样的输入分支数
			 * @param uniform true: 每条分支输入概率相等；false: 输入概率 ~ U(0,1)
			 */
			void _set_input_impl(size_t n_inputs, bool uniform);
			/// 随机采样 n 条输入分支，输入概率 ~ U(0,1) 后归一化
			inline void set_input_random(size_t n_inputs) { _set_input_impl(n_inputs, false); }
			/// 随机采样 n 条输入分支，每条输入概率相等
			inline void set_input_uniform(size_t n_inputs) { _set_input_impl(n_inputs, true); }

			/// 返回全部分支组（const）
			inline auto& get_branch_groups() const { return branch_groups; }
			/// 返回实际执行的含噪操作序列（const）
			inline auto& get_operations() const { return operations; }
			/// 返回全部分支组
			inline auto& get_branch_groups() { return branch_groups; }
			/// 返回实际执行的含噪操作序列
			inline auto& get_operations() { return operations; }

			/// @deprecated 清空噪声模型，改用 set_noise_models
			[[deprecated]] inline void clear_noise() { noise_parameters.clear(); }

			/// @deprecated 追加单条噪声，改用 set_noise_models 整体设置
			[[deprecated]] inline void add_noise_model(OperationType t, double v) {
				noise_parameters[t] = v;
			}

			/**
			 * @brief 设置噪声模型。
			 *
			 * 概率参数做范围校验：所有概率须在 [0,1]；Damping 额外要求
			 * gamma < 1（gamma = 1 会在一步内衰灭全部激发，使轨迹范数为 0，
			 * 破坏 sample_output / normalization），越界抛出 std::runtime_error。
			 * @param noises {操作类型: 发生概率} 映射
			 */
			inline void set_noise_models(const std::map<OperationType, double>& noises) {
				for (auto& [type, p] : noises)
				{
					if (!(p >= 0.0 && p <= 1.0) ||
						(type == OperationType::Damping && p >= 1.0))
						throw std::runtime_error(
							"set_noise_models: noise probability out of range "
							"(must be in [0,1]; Damping requires gamma < 1)");
				}
				noise_parameters = noises;
			}

			/// 当前是否未设置任何噪声
			inline bool is_noise_free() const { return noise_parameters.size() == 0; }
			/// 噪声模型是否包含 Damping
			inline bool has_damping() const { return noise_parameters.find(OperationType::Damping) != noise_parameters.end(); }

			/**
			 * @brief 生成含噪操作序列并复位全部分支组。
			 *
			 * 由 time_step.generate(noise_parameters, arch_qubit) 生成操作
			 * 序列，并清空好分支索引与有效分支视图（run 系列内部调用）。
			 */
			void initialize_system();

			/**
			 * @brief 采样输出分支并计算装载保真度 |<理想|实际>|^2。
			 * @return 采样轨迹下的保真度；无分支时返回 0
			 */
			double sample_and_get_fidelity();
			/// sample_and_get_fidelity 的别名
			double get_fidelity() { return sample_and_get_fidelity(); }

			/**
			 * @brief 剪枝运行的准备阶段（run() 内部调用）。
			 *
			 * 1. 构造 valid_branch_group_view（仅纳入需要演化的分支组）
			 * 2. 确定 first_good_branch_group（预测基准）
			 * 3. 标记 branches[i].good 并设置其总线输出
			 * 4. 收集 good_branch_group_ids
			 */
			void prepare_bad();

			/**
			 * @brief 不剪枝运行的准备阶段（run_all() 内部调用）。
			 *
			 * 构造 valid_branch_group_view（纳入全部分支组）。
			 */
			void prepare_all();

			/**
			 * @brief 仅好分支运行的准备阶段（run_good_only() 内部调用）。
			 *
			 * 1. 构造 good_only 的 valid_branch_group_view
			 * 2. 确定 first_good_branch_group
			 * 3. 标记 branches[i].good 并设置其总线输出
			 * 4. 收集 good_branch_group_ids
			 */
			void prepare_good_only();

			/* Execution of operations */
			/// 执行地址拷贝操作：第 layer 层节点写入方向 a
			void run_acopy(int layer);
			/// 执行第 layer_id 层的普通交换
			void run_swap(size_t layer_id);
			/// 执行第 layer_id 层的受控交换
			void run_cswap(size_t layer_id);
			/// 在 qubit_id 比特上执行比特翻转（噪声）
			void run_bitflip(size_t qubit_id);
			/// 在 qubit_id 比特上执行相位翻转（噪声，depol_id 为关联参数）
			void run_phaseflip(size_t qubit_id, double depol_id);
			/// 在 qubit_id 比特上执行比特-相位联合翻转（噪声）
			void run_bitphaseflip(size_t qubit_id);
			/// 在 qubit_id 比特上执行去极化（噪声）
			void run_depolarizing(size_t qubit_id, double depol_id);
			/// 执行总线上的 Hadamard 变换
			void run_hadamard();
			/// 从数据树取第 digit 位到总线（FetchData）
			void run_fetchdata(size_t digit);
			/// 总线输入第 digit 位
			void run_busin(size_t digit);
			/// 总线输出第 digit 位
			void run_busout(size_t digit);
			/// 指定比特的完整振幅衰减（噪声，gamma 为衰减率）
			void run_damp_full(size_t qubit_id, size_t step, double gamma);
			/// 全系统公共振幅衰减（噪声，按非零元素计数乘衰减因子）
			void run_damp_common(double gamma);

			/// 清理振幅为零的稀疏元素
			void clear_zero_elements();

			/// 返回电路概要信息字符串
			std::string to_string() const;
			/// 返回含分支细节的完整信息字符串
			std::string to_string_full_info() const;

			/// 不含 Damping 的归一化因子
			double get_normalization_factor() const;
			/// 含 Damping 的归一化因子
			double get_normalization_factor_with_damping() const;
			/// 按当前噪声模型归一化全部分支振幅
			void normalization();

			/// 把基准好分支的系统态物化为其余好分支（XOR 镜像预测）
			void materialize_good_branches();

			/**
			 * @brief 依据 branch_probs 采样输出总线态。
			 *
			 * 采样结果写入各分支组与 final_system_state，随后的
			 * sample_and_get_fidelity 基于该采样轨迹计算保真度。
			 */
			void sample_output();

			/// @deprecated 使用 fmt::format("{}", memory) 替代
			[[deprecated]] inline std::string mem_to_string() const {
				return fmt::format("{}", memory);
			}

			/// 仅演化好分支（内部调试入口）
			void run_good();
			/// 演化 valid_branch_group_view 中的分支（run 系列的实际执行核）
			void run_bad();

			/// 生成操作序列并按剪枝模式运行（等价于 run_normal）
			void run();

			/// 不剪枝运行全部分支（调试用 plain 版本，帮助验证 run()）
			void run_all();

			/// 仅好分支运行（调试用 plain 版本，帮助验证 run()）
			void run_good_only();

			/// 不剪枝运行全部分支：无剪枝的 ground truth 基准
			void run_full();

			/// 剪枝运行：坏分支全算 + 一条好分支作基准（日常默认入口）
			void run_normal();

			/// run(version="full") 的版本字符串常量
			static constexpr auto FULL_VER = "full";
			/// run(version="normal") 的版本字符串常量
			static constexpr auto NORMAL_VER = "normal";

			/**
			 * @brief 按版本字符串运行。
			 * @param version "full"/"old"：不剪枝；"normal"/"new"：剪枝；
			 *                "fast"：仅好分支；其余抛出无效输入异常
			 */
			void run(std::string version);
		};
	} // namespace qram_qubit
} // namespace qram_simulator
