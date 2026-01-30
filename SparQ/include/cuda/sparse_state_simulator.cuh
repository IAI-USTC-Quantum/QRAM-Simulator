#pragma once
#include "sparse_state_simulator.h"
#include "cuda/qram_circuit_qutrit.cuh"
#include "basic_components.cuh"
#include "quantum_interfere_basic.cuh"
#include "condrot.cuh"

namespace qram_simulator {
	template<typename BlockEncoding>
	DenseMatrix<complex_t> cu_extract_block_encoding(BlockEncoding encA, std::string main_reg, std::string anc_UA,
		bool is_full = false, bool is_dag = false)
	{
		return _extract_block_encoding<BlockEncoding, CuSparseState>(encA, main_reg, anc_UA, is_full, is_dag);
	}

}