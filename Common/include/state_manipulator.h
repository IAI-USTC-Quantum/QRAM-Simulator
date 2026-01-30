#pragma once
#include "qram_circuit_qutrit.h"
#include "simple_quantum_simulator.h"

namespace qram_simulator {
	
	/* Note that this class only adapts to qram_qutrit::QRAMCircuit */
	class QRAMFullAmp
	{
		using QRAMCircuit = qram_qutrit::QRAMCircuit;

		QRAMCircuit* qram = nullptr;
		std::map<size_t, size_t> branchid_map;
		size_t address_size;
		size_t data_size;
	public:
		QRAMFullAmp(size_t addr_sz, size_t data_sz, const memory_t& memory)
			:address_size(addr_sz), data_size(data_sz)
		{
			qram = new QRAMCircuit(addr_sz, data_sz);
			qram->set_memory(memory);
		}

		~QRAMFullAmp() { delete qram; }

		inline QRAMCircuit* get_instance() { return qram; }
		inline QRAMCircuit* operator->() { return qram; }

		void _set_branches_full(const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits);		

		void _set_branches(const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits);
				
		void _reconstruct(
			std::vector<std::complex<double>>& ret,
			const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits,
			const std::vector<size_t>& other_qubits);

		std::vector<std::complex<double>> apply(
			const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits,
			const std::vector<size_t>& other_qubits,
			std::string version);
	};
} // namespace qram_simulator