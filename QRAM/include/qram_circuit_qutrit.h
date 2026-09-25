#pragma once

#include "qram_branch_qutrit.h"

namespace qram_simulator {
	namespace qram_qutrit {
		/**
		 * @brief qutrit 架构 QRAM 装载电路的分支化稀疏仿真器。
		 *
		 * 每个树节点为三能级系统（qutrit）：地址能级 addr ∈ {W, L, R} 与
		 * 数据能级 data ∈ {0, 1}。以 SubBranch 轨迹沿含噪操作序列演化，
		 * 支持 good/bad 分支剪枝与输出采样保真度评估。
		 *
		 * 典型工作流与 qram_qubit::QRAMCircuit 一致：构造 → 设内存 →
		 * 设噪声 → set_input_* → run_normal / run_full →
		 * sample_and_get_fidelity。
		 *
		 * @note run(version) 要求先设置非空噪声模型；无噪声场景请直接
		 * 调用 run_normal / run_full（应在 QRAMCircuit 之外处理）。
		 */
		struct QRAMCircuit {
			using qram_state_t = typename SubBranch::element_type;

		public:
			/// 地址位宽（数据树深度）
			size_t address_size;
			/// 数据位宽（每个内存槽的比特数）
			size_t data_size;
			/// 时序与噪声调度器
			TimeStep time_step;
			/// 数据树：memory[i] 为地址 i 处存放的 data_size 位整数
			memory_t memory;

			/* Internal operations */
			/// 实际执行的含噪操作序列（initialize_system 生成）
			TimeSlices operations;

			/// 噪声模型：{操作类型: 发生概率}
			noise_t noise_parameters;

			/// 全部分支（归一化、解纠缠后的轨迹）
			std::vector<Branch> branches;

			/// 全部分支的概率（采样与 Damping 噪声评估使用）
			std::vector<double> branch_probs;

			/// 本次运行需要演化的分支视图。
			/// version: "full" 全部分支；"normal" 第一好分支 + 全部坏分支；
			/// "fast" 无分支需演化（短路计算）
			std::vector<Branch*> valid_branch_view;

			/// 第一条好分支的下标（作为其余好分支的预测基准），-1 表示无
			int64_t first_good_branch = -1;

			/// 除基准外的其余好分支下标
			std::vector<size_t> good_branch_ids;

		public:
			/// 采样得到的最终系统态
			qram_state_t final_system_state;

			/**
			 * @brief 构造 QRAM 电路（内存未初始化）。
			 * @param address_sz 地址位宽
			 * @param data_sz 数据位宽
			 */
			QRAMCircuit(size_t address_sz, size_t data_sz)
				: address_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(address_size));
			}
			/**
			 * @brief 构造 QRAM 电路并以给定数据树初始化（拷贝）。
			 * @param address_sz 地址位宽
			 * @param data_sz 数据位宽
			 * @param memory_ 数据树（长度须为 2^address_sz）
			 */
			QRAMCircuit(size_t address_sz, size_t data_sz, const memory_t& memory_)
				: address_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(address_size));
				set_memory(memory_);
			}
			/**
			 * @brief 构造 QRAM 电路并以给定数据树初始化（移动）。
			 * @param address_sz 地址位宽
			 * @param data_sz 数据位宽
			 * @param memory_ 数据树（右值）
			 */
			QRAMCircuit(size_t address_sz, size_t data_sz, memory_t&& memory_)
				: address_size(address_sz), data_size(data_sz),
				time_step(address_sz, data_sz)
			{
				memory.resize(pow2(address_size));
				set_memory(std::move(memory_));
			}

			/// 电路占用的物理比特数：2 * (2^address_size - 1)
			inline size_t get_qubit_num() const { return 2 * (pow2(address_size) - 1); }
			/// 内存槽总数
			inline size_t memory_size() const { return memory.size(); }

			/// 以全局随机引擎随机填充整棵数据树
			virtual void set_memory_random();
			/// 设置数据树（拷贝）
			virtual void set_memory(const memory_t& new_memory);
			/// 设置数据树（移动）
			virtual void set_memory(memory_t&& new_memory);
			/// 返回数据树引用
			inline auto& get_memory() { return memory; }
			/// 返回数据树 const 引用
			inline auto& get_memory() const { return memory; }

			/// 返回全部分支（const）
			inline auto& get_branches() const { return branches; }
			/// 返回全部分支
			inline auto& get_branches() { return branches; }
			/// 返回全部分支概率（const）
			inline auto& get_branch_probs() const { return branch_probs; }
			/// 返回全部分支概率
			inline auto& get_branch_probs() { return branch_probs; }
			/// 返回含噪操作序列（const）
			inline auto& get_operations() const { return operations; }
			/// 返回含噪操作序列
			inline auto& get_operations() { return operations; }

			/**
			 * @brief 设置噪声模型。
			 *
			 * 概率参数做范围校验：所有概率须在 [0,1]；Damping 额外要求
			 * gamma < 1（gamma = 1 会在一步内衰灭全部激发，使轨迹范数为 0，
			 * 破坏 sample_output / normalization），越界抛出 std::runtime_error。
			 * @param noises {操作类型: 发生概率} 映射
			 */
			inline void set_noise_models(const noise_t& noises)
			{
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
			/// 返回当前噪声模型
			inline auto& get_noise_models() { return noise_parameters; }
			/// 返回当前噪声模型（const）
			inline auto& get_noise_models() const { return noise_parameters; }
			/// 当前是否未设置任何噪声
			inline bool is_noise_free() const { return noise_parameters.size() == 0; }
			/// 噪声模型是否包含 Damping
			inline bool has_damping() const { return noise_parameters.find(OperationType::Damping) != noise_parameters.end(); }

			/**
			 * @brief 生成含噪操作序列并初始化分支结构（run 系列内部调用）。
			 */
			void initialize_system();

			/// 随机采样 n 条输入分支，随机填充分支概率
			void set_input_random(size_t n_inputs);

			/// 随机采样 n 条输入分支，每条输入概率相等
			void set_input_uniform(size_t n_inputs);

			/**
			 * @brief 采样输出分支并计算装载保真度 |<理想|实际>|^2。
			 * @return 采样轨迹下的保真度
			 */
			double sample_and_get_fidelity();

			/**
			 * @brief 剪枝运行的准备阶段（run_normal 内部调用）。
			 *
			 * 1. 构造 valid_branch_view（仅纳入需要演化的分支）
			 * 2. 确定 first_good_branch（预测基准）
			 * 3. 标记 branches[i].good 并设置其总线输出
			 * 4. 收集 good_branch_ids
			 */
			void pick_good_bad();

			/**
			 * @brief 不剪枝运行的准备阶段（run_full 内部调用）。
			 *
			 * 构造 valid_branch_view（纳入全部分支）。
			 */
			void pick_all();

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
			/// 全系统公共振幅衰减（噪声）
			void run_damp_common(double gamma);

			/// 清理振幅为零的稀疏元素
			void clear_zero_elements();

			/// 按当前噪声模型（含 Damping）的归一化因子
			double get_normalization_factor() const;
			/// 不含 Damping 的归一化因子
			double get_normalization_factor_without_damping() const;
			/// 含 Damping 的归一化因子
			double get_normalization_factor_with_damping() const;
			/// 按当前噪声模型归一化全部分支振幅
			void normalization();
			/// 不含 Damping 的归一化
			void normalization_without_damping();
			/// 含 Damping 的归一化
			void normalization_with_damping();

			/// 采样输出（跳过归一化的内部变体）
			void sample_output_without_normalization();
			/// 采样输出（含 Damping、跳过归一化的内部变体）
			void sample_output_without_normalization_with_damping();
			/// 采样输出（不含 Damping、跳过归一化的内部变体）
			void sample_output_without_normalization_without_damping();
			/// 依据 branch_probs 采样输出总线态（不含 Damping 路径）
			void sample_output();

			/// 标记好分支并确定预测基准（run 前置）
			void run_good();
			/// 演化 valid_branch_view 中的分支（run 系列的实际执行核）
			void run_valid_branches();

			/// 剪枝运行：坏分支全算 + 一条好分支作基准（日常默认入口）
			void run_normal();

			/// 不剪枝运行全部分支（调试用，帮助验证 run()）
			void run_full();

			/// 仅好分支运行（当前未实现）
			void run_good_only();
			/// 仅好分支运行（不含 Damping 变体，当前未实现）
			void run_good_only_without_damping();
			/// 仅好分支运行（含 Damping 变体，当前未实现）
			void run_good_only_with_damping();
			/**
			 * @brief 按版本字符串运行。
			 *
			 * 无输入分支时直接返回；未设置噪声时抛出无效输入异常
			 * （无噪场景应在 QRAMCircuit 之外处理）。
			 * @param version "full"/"old"：不剪枝；"new"/"normal"：剪枝；
			 *                其余抛出无效输入异常
			 */
			void run(const std::string& version);

			/// 返回电路概要信息字符串
			std::string to_string() const;
			/// 返回含分支细节的完整信息字符串
			std::string to_string_full_info() const;

			/// run(version="full") 的版本字符串常量
			static constexpr auto FULL_VER = "full";
			/// run(version="normal") 的版本字符串常量
			static constexpr auto NORMAL_VER = "normal";

			virtual ~QRAMCircuit() {}
		};
	} // namespace qram_qutrit

	// using QRAMCircuit = qram_qutrit::QRAMCircuit;

}
