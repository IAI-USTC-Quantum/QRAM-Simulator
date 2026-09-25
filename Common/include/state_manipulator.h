#pragma once
#include "qram_circuit_qutrit.h"
#include "simple_quantum_simulator.h"

namespace qram_simulator {

	/**
	 * @brief QRAM 电路到全振幅态向量的桥接器（仅适配 qram_qutrit::QRAMCircuit）。
	 *
	 * 外部全振幅模拟器提供态向量与比特映射，apply 负责把态向量分解
	 * 为 QRAM 输入分支（_set_branches）、在分支电路上运行一次含噪
	 * QRAM 装载、采样输出并把结果重建回完整态向量（_reconstruct），
	 * 从而将 QRAM 仿真嵌入更大的电路演化流程。
	 *
	 * 典型用法（addr 比特 {0,1}，data 比特 {2,3}）：
	 * @code
	 * QRAMFullAmp qram(2, 2, {0,1,2,3});
	 * qram->set_noise_models(noise);          // run(version) 需非空噪声
	 * state = qram.apply(state, {0,1}, {2,3}, {}, "new");
	 * @endcode
	 */
	class QRAMFullAmp
	{
		using QRAMCircuit = qram_qutrit::QRAMCircuit;

		/// 内部持有的 qutrit QRAM 电路（apply 时驱动）
		QRAMCircuit* qram = nullptr;
		/// 分支编号 → branches 下标的映射
		std::map<size_t, size_t> branchid_map;
		/// 地址位宽
		size_t address_size;
		/// 数据位宽
		size_t data_size;
	public:
		/**
		 * @brief 构造桥接器：内部创建 qutrit QRAMCircuit 并载入数据树。
		 * @param addr_sz 地址位宽
		 * @param data_sz 数据位宽
		 * @param memory 数据树（长度须为 2^addr_sz）
		 */
		QRAMFullAmp(size_t addr_sz, size_t data_sz, const memory_t& memory)
			:address_size(addr_sz), data_size(data_sz)
		{
			qram = new QRAMCircuit(addr_sz, data_sz);
			qram->set_memory(memory);
		}

		/// 释放内部电路
		~QRAMFullAmp() { delete qram; }

		/// 返回内部 QRAMCircuit 指针
		inline QRAMCircuit* get_instance() { return qram; }
		/// 便捷访问内部 QRAMCircuit 成员（如 set_noise_models）
		inline QRAMCircuit* operator->() { return qram; }

		/**
		 * @brief 全分支分解：把态向量按 (addr, data) 完整分解为 2^(addr+data) 条分支。
		 *
		 * 适用于分支总数不超过 25 比特的情形。
		 * @param state 全振幅态向量
		 * @param address_qubits 地址比特编号
		 * @param data_qubits 数据比特编号
		 */
		void _set_branches_full(const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits);

		/**
		 * @brief 稀疏分支分解：仅保留概率非零的分支（大于 25 比特时自动启用）。
		 * @param state 全振幅态向量
		 * @param address_qubits 地址比特编号
		 * @param data_qubits 数据比特编号
		 */
		void _set_branches(const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits);

		/**
		 * @brief 把分支上的装载结果重建回全振幅态向量。
		 *
		 * 好分支按基准轨迹 XOR 镜像预测（含 Damping 乘子），坏分支
		 * 逐轨迹映射；重建结果做归一化校验。
		 * @param ret 输出态向量（调用前置零）
		 * @param state 输入态向量
		 * @param address_qubits 地址比特编号
		 * @param data_qubits 数据比特编号
		 * @param other_qubits 其余旁观比特编号
		 */
		void _reconstruct(
			std::vector<std::complex<double>>& ret,
			const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits,
			const std::vector<size_t>& other_qubits);

		/**
		 * @brief 把一次 QRAM 装载复合到全振幅态向量上。
		 *
		 * @param state 输入态向量（长度 2^(addr+data+other)）
		 * @param address_qubits 地址比特编号（数量须等于 address_size）
		 * @param data_qubits 数据比特编号（数量须等于 data_size）
		 * @param other_qubits 旁观比特编号（不参与装载）
		 * @param version "new"/"normal"：剪枝；"old"/"full"：不剪枝
		 *               （内部走 run(version)，要求先设置非空噪声模型）
		 * @return 装载后的新态向量（归一化）
		 */
		std::vector<std::complex<double>> apply(
			const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits,
			const std::vector<size_t>& other_qubits,
			std::string version);
	};
}
