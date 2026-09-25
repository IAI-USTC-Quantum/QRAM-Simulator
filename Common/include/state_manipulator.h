#pragma once
#include "qram_circuit_qutrit.h"
#include "simple_quantum_simulator.h"

namespace qram_simulator {

	/**
	 * @brief Bridge from a QRAM circuit to a full-amplitude state vector (adapted only for qram_qutrit::QRAMCircuit).
	 *
	 * The external full-amplitude simulator provides the state vector and the
	 * qubit mapping; apply decomposes the state vector into QRAM input branches
	 * (_set_branches), runs one noisy QRAM loading on the branch circuit,
	 * samples the output, and reconstructs the result back into the complete
	 * state vector (_reconstruct), thereby embedding QRAM simulation into a
	 * larger circuit evolution workflow.
	 *
	 * Typical usage (addr qubits {0,1}, data qubits {2,3}):
	 * @code
	 * QRAMFullAmp qram(2, 2, {0,1,2,3});
	 * qram->set_noise_models(noise);          // run(version) requires a non-null noise model
	 * state = qram.apply(state, {0,1}, {2,3}, {}, "new");
	 * @endcode
	 */
	class QRAMFullAmp
	{
		using QRAMCircuit = qram_qutrit::QRAMCircuit;

		/// Internally held qutrit QRAM circuit (driven by apply)
		QRAMCircuit* qram = nullptr;
		/// Mapping from branch id to index into branches
		std::map<size_t, size_t> branchid_map;
		/// Address width
		size_t address_size;
		/// Data width
		size_t data_size;
	public:
		/**
		 * @brief Construct the bridge: internally creates a qutrit QRAMCircuit and loads the data tree.
		 * @param addr_sz Address width
		 * @param data_sz Data width
		 * @param memory Data tree (length must be 2^addr_sz)
		 */
		QRAMFullAmp(size_t addr_sz, size_t data_sz, const memory_t& memory)
			:address_size(addr_sz), data_size(data_sz)
		{
			qram = new QRAMCircuit(addr_sz, data_sz);
			qram->set_memory(memory);
		}

		/// Release the internal circuit
		~QRAMFullAmp() { delete qram; }

		/// Return the internal QRAMCircuit pointer
		inline QRAMCircuit* get_instance() { return qram; }
		/// Convenient access to internal QRAMCircuit members (e.g. set_noise_models)
		inline QRAMCircuit* operator->() { return qram; }

		/**
		 * @brief Full branch decomposition: fully decompose the state vector by (addr, data) into 2^(addr+data) branches.
		 *
		 * Applicable when the total number of branches does not exceed 25 bits.
		 * @param state Full-amplitude state vector
		 * @param address_qubits Address qubit indices
		 * @param data_qubits Data qubit indices
		 */
		void _set_branches_full(const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits);

		/**
		 * @brief Sparse branch decomposition: keep only branches with nonzero probability (enabled automatically above 25 bits).
		 * @param state Full-amplitude state vector
		 * @param address_qubits Address qubit indices
		 * @param data_qubits Data qubit indices
		 */
		void _set_branches(const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits);

		/**
		 * @brief Reconstruct the loading results on the branches back into a full-amplitude state vector.
		 *
		 * Good branches are predicted by XOR mirroring of the reference
		 * trajectory (with Damping multipliers); bad branches are mapped
		 * trajectory by trajectory; the reconstructed result is checked for normalization.
		 * @param ret Output state vector (zeroed before the call)
		 * @param state Input state vector
		 * @param address_qubits Address qubit indices
		 * @param data_qubits Data qubit indices
		 * @param other_qubits Indices of the remaining spectator qubits
		 */
		void _reconstruct(
			std::vector<std::complex<double>>& ret,
			const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits,
			const std::vector<size_t>& other_qubits);

		/**
		 * @brief Compose one QRAM loading onto a full-amplitude state vector.
		 *
		 * @param state Input state vector (length 2^(addr+data+other))
		 * @param address_qubits Address qubit indices (count must equal address_size)
		 * @param data_qubits Data qubit indices (count must equal data_size)
		 * @param other_qubits Spectator qubit indices (not involved in loading)
		 * @param version "new"/"normal": pruning; "old"/"full": no pruning
		 *               (internally calls run(version); a non-null noise model must be set first)
		 * @return New state vector after loading (normalized)
		 */
		std::vector<std::complex<double>> apply(
			const std::vector<std::complex<double>>& state,
			const std::vector<size_t>& address_qubits,
			const std::vector<size_t>& data_qubits,
			const std::vector<size_t>& other_qubits,
			std::string version);
	};
}
