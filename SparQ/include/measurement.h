/**
 * @file measurement.h
 * @brief 稀疏态可播种测量/复位/概率查询接口
 * @details 为动态执行器（mid-circuit MEASURE / RESET / QIF 等）提供第一类、
 *          可复现（seedable）的稀疏态操作：
 *            - `MeasureZ`：投影式 Z 基测量，按 Born 定则采样，坍缩并重新归一化；
 *            - `Reset`：测量后经典条件翻转，将寄存器强制复位到给定经典值；
 *            - `Probability`：只读概率查询（不改变状态），用于 QIF/QWHILE 等
 *              动态控制流条件判断，以及一致性/合规测试。
 *
 *          随机性来自 `qram_simulator::random_engine` 单例，可通过
 *          `random_engine::set_seed()` 显式播种，从而使 `MeasureZ`/`Reset`
 *          的采样结果可复现（对于 dynamic executor 的确定性回放/单元测试
 *          至关重要）。
 *
 * @details 输入校验契约（构造函数与 `operator()` 共同保证）：
 *          - 每个寄存器名称/ID 必须解析为 `System` 中当前**激活**的寄存器；
 *            未知名称、越界 ID、或已被 `RemoveRegister` 移除的 ID 一律在
 *            构造阶段抛出 `invalid_argument`（Python 侧为 `ValueError`）。
 *          - 同一构造调用中的寄存器列表不允许出现重复 ID（例如
 *            `MeasureZ({"a", "a"})`），否则抛出 `invalid_argument`。
 *          - `Reset` 的目标值、`Probability` 的比较值必须能被对应寄存器的
 *            位宽表示（即小于 `2^size_of(id)`，`size_of(id) == 64` 时不受
 *            限制），否则抛出 `invalid_argument`——这类矛盾目标/取值在旧实现
 *            中会被静默截断，属于本次加固修复的范围。
 *          - `MeasureZ`（进而 `Reset`）在采样前显式校验输入态的总概率
 *            （所有分支 `abs(amplitude)^2` 之和）是有限数值且与 1 的偏差在
 *            `kNormalizationThreshold` 之内；不满足时抛出 `runtime_error`
 *            （Python 侧为 `RuntimeError`），而不是像旧实现那样把
 *            `random_engine::uniform01()` 采样值直接与 `[0, 1)` 比较——那样
 *            当总概率显著偏离 1 时会静默地把多余/不足的概率质量都堆到最后
 *            一个分支上（fallback 分支），产生有偏采样且不报错。
 *
 * @warning `pysparq.dynamic_operator.compile_operator()` 编译得到的任意 C++
 *          算子不经过酉性证明；它只是运行时编译某个 `operator()`/`dag()` 对，
 *          编译器/绑定层不会（也不能）静态或动态验证该算子确实是酉的。
 *          QCFD 支持路径（QECC.Lang 驱动的 qfvm/qnls/qham）禁止使用
 *          `compile_operator`；所有语义必须通过本文件等具名、可测试的内建
 *          算子表达，并通过 `pysparq.conformance` 提供的一致性测试矩阵验证。
 */

#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	/**
	 * @brief 归一化校验阈值
	 * @details `MeasureZ` 采样前要求 `|sum(|amplitude|^2) - 1| < kNormalizationThreshold`，
	 *          与 `CheckNormalization` 的默认阈值（`1e-5`）保持一致。
	 */
	constexpr double kNormalizationThreshold = 1e-5;

	/**
	 * @brief 投影式 Z 基测量
	 * @details 对一个或多个寄存器执行计算基（Z 基）测量：
	 *          1. 校验输入态的总概率有限且约等于 1（见文件级文档）；
	 *          2. 使用 `random_engine::uniform01()`（可通过 `set_seed` 播种）
	 *             按 Born 定则在各基态分支间采样一个结果；
	 *          3. 移除与采样结果不符的分支，并重新归一化剩余振幅；
	 *          4. 返回采样得到的寄存器值以及该结果对应的测量概率。
	 *
	 *          该操作是非酉、不可逆的（测量坍缩），因此不提供 `dag()`。
	 */
	struct MeasureZ
	{
		/** @brief 被测量的寄存器 ID 列表 */
		std::vector<size_t> registers;

		/**
		 * @brief 构造函数（寄存器名称列表版本）
		 * @throws invalid_argument 名称未找到，或列表中含重复寄存器
		 */
		MeasureZ(const std::vector<std::string>& register_names);

		/**
		 * @brief 构造函数（寄存器 ID 列表版本）
		 * @throws invalid_argument ID 越界/未激活，或列表中含重复寄存器
		 */
		MeasureZ(const std::vector<size_t>& register_ids);

		/** @brief 构造函数（单个寄存器名称版本） */
		MeasureZ(std::string_view register_name);

		/** @brief 构造函数（单个寄存器 ID 版本） */
		MeasureZ(size_t register_id);

		/**
		 * @brief 执行测量
		 * @param state 系统状态向量（原地坍缩+重新归一化）
		 * @return {采样得到的寄存器值列表, 该结果的概率}
		 * @throws invalid_argument 当状态为空时
		 * @throws runtime_error 当总概率非有限或明显偏离 1 时（见文件级文档）
		 */
		std::pair<std::vector<uint64_t>, double> operator()(std::vector<System>& state) const;

		/** @brief SparseState 版本 */
		std::pair<std::vector<uint64_t>, double> operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}
	};

	/**
	 * @brief 可播种的 RESET（测量 + 经典条件翻转）
	 * @details 物理上，复位一个可能处于叠加态的寄存器只能通过
	 *          “测量后按经典结果条件翻转”实现（与真实硬件的 active reset
	 *          以及 OriginIR-ext 的 `RESET` 指令语义一致）：
	 *            1. 用 `MeasureZ` 对目标寄存器做一次投影测量（坍缩+归一化）；
	 *            2. 由于坍缩后所有剩余分支中该寄存器的值均等于测量结果，
	 *               直接将其覆盖为目标值等价于对一个确定值做经典位翻转，
	 *               不会与其他分支发生非法合并，因而是良定义的。
	 *
	 *          默认目标值为 0（对应 `RESET` 到 |0>）。
	 */
	struct Reset
	{
		/** @brief 被复位的寄存器 ID 列表 */
		std::vector<size_t> registers;

		/** @brief 复位目标值列表（与 registers 一一对应） */
		std::vector<uint64_t> target_values;

		/**
		 * @brief 构造函数（名称列表，默认全部复位到 0）
		 * @throws invalid_argument 名称未找到，或列表中含重复寄存器
		 */
		explicit Reset(const std::vector<std::string>& register_names);

		/**
		 * @brief 构造函数（名称列表 + 目标值列表）
		 * @throws invalid_argument 名称未找到、重复寄存器，或目标值超出对应
		 *         寄存器位宽可表示的范围
		 */
		Reset(const std::vector<std::string>& register_names, const std::vector<uint64_t>& targets);

		/**
		 * @brief 构造函数（ID 列表，默认全部复位到 0）
		 * @throws invalid_argument ID 越界/未激活，或列表中含重复寄存器
		 */
		explicit Reset(const std::vector<size_t>& register_ids);

		/**
		 * @brief 构造函数（ID 列表 + 目标值列表）
		 * @throws invalid_argument ID 越界/未激活、重复寄存器，或目标值超出
		 *         对应寄存器位宽可表示的范围
		 */
		Reset(const std::vector<size_t>& register_ids, const std::vector<uint64_t>& targets);

		/**
		 * @brief 构造函数（单个寄存器名称 + 目标值，默认 0）
		 * @throws invalid_argument 名称未找到，或目标值超出寄存器位宽
		 */
		explicit Reset(std::string_view register_name, uint64_t target = 0);

		/**
		 * @brief 构造函数（单个寄存器 ID + 目标值，默认 0）
		 * @throws invalid_argument ID 越界/未激活，或目标值超出寄存器位宽
		 */
		explicit Reset(size_t register_id, uint64_t target = 0);

		/**
		 * @brief 执行复位
		 * @param state 系统状态向量（原地坍缩+归一化+覆盖为目标值）
		 * @return 复位前测量得到的寄存器值列表（用于诊断/日志）
		 */
		std::vector<uint64_t> operator()(std::vector<System>& state) const;

		/** @brief SparseState 版本 */
		std::vector<uint64_t> operator()(SparseState& state) const
		{
			return (*this)(state.basis_states);
		}
	};

	/**
	 * @brief 只读 Born 概率查询
	 * @details 计算给定寄存器取给定值这一事件的概率，不对状态做任何修改。
	 *          用于动态执行器中的 `QIF`/`QWHILE` 条件判断、一致性测试中的
	 *          Born 定则校验，以及在真正测量/复位之前预估分支概率。
	 */
	struct Probability
	{
		/** @brief 参与判断的寄存器 ID 列表 */
		std::vector<size_t> registers;

		/** @brief 目标值列表（与 registers 一一对应） */
		std::vector<uint64_t> values;

		/**
		 * @brief 构造函数（名称->值映射版本）
		 * @throws invalid_argument 名称未找到，或值超出对应寄存器位宽
		 */
		explicit Probability(const std::map<std::string_view, uint64_t>& assignments);

		/**
		 * @brief 构造函数（ID->值映射版本）
		 * @throws invalid_argument ID 越界/未激活，或值超出对应寄存器位宽
		 */
		explicit Probability(const std::map<size_t, uint64_t>& assignments);

		/**
		 * @brief 构造函数（名称列表 + 值列表版本）
		 * @throws invalid_argument 名称未找到、重复寄存器，或值超出位宽
		 */
		Probability(const std::vector<std::string>& register_names, const std::vector<uint64_t>& target_values);

		/**
		 * @brief 构造函数（ID 列表 + 值列表版本）
		 * @throws invalid_argument ID 越界/未激活、重复寄存器，或值超出位宽
		 */
		Probability(const std::vector<size_t>& register_ids, const std::vector<uint64_t>& target_values);

		/**
		 * @brief 构造函数（单个寄存器名称 + 值）
		 * @throws invalid_argument 名称未找到，或值超出寄存器位宽
		 */
		Probability(std::string_view register_name, uint64_t value);

		/**
		 * @brief 构造函数（单个寄存器 ID + 值）
		 * @throws invalid_argument ID 越界/未激活，或值超出寄存器位宽
		 */
		Probability(size_t register_id, uint64_t value);

		/**
		 * @brief 计算该赋值组合的概率
		 * @param state 系统状态向量（只读，不修改）
		 * @return 概率（[0, 1] 之间；空约束返回 1）
		 */
		double operator()(const std::vector<System>& state) const;

		/** @brief SparseState 版本 */
		double operator()(const SparseState& state) const
		{
			return (*this)(state.basis_states);
		}

		/**
		 * @brief 计算单个寄存器的完整结果分布（只读）
		 * @param state 系统状态向量
		 * @param register_id 寄存器 ID
		 * @return 从寄存器取值到概率的映射
		 * @throws invalid_argument ID 越界/未激活
		 */
		static std::map<uint64_t, double> distribution(const std::vector<System>& state, size_t register_id);

		/** @brief SparseState 版本（按 ID） */
		static std::map<uint64_t, double> distribution(const SparseState& state, size_t register_id)
		{
			return distribution(state.basis_states, register_id);
		}

		/** @brief 按寄存器名称版本 */
		static std::map<uint64_t, double> distribution(const std::vector<System>& state, std::string_view register_name);

		/** @brief SparseState 版本（按名称） */
		static std::map<uint64_t, double> distribution(const SparseState& state, std::string_view register_name)
		{
			return distribution(state.basis_states, register_name);
		}
	};
}
