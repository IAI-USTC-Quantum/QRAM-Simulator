#include "cuda/sparse_state_simulator.cuh"
#include "DiscreteAdiabatic/qda_fundamental.h"

namespace qram_simulator {
	namespace QDA {
		template <typename Block_Encoding, typename State_Prep>
		DenseMatrix<complex_t> cu_extract_full_unitary(
			Block_Encoding_Hs<Block_Encoding, State_Prep> encHs,
			std::string main_reg, std::string anc_UA,
			std::string anc_1, std::string anc_2, std::string anc_3, std::string anc_4)
		{
			return _extract_full_unitary<Block_Encoding, State_Prep, CuSparseState>(encHs, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4);
		}

		template <typename Block_Encoding, typename State_Prep>
		DenseMatrix<complex_t> cu_extract_block_encoding_Hs(
			Block_Encoding_Hs<Block_Encoding, State_Prep> encHs,
			std::string main_reg, std::string anc_UA,
			std::string anc_1, std::string anc_2, std::string anc_3, std::string anc_4,
			size_t qubit_num)
		{
			return _extract_block_encoding_Hs<Block_Encoding, State_Prep, CuSparseState>(encHs, main_reg, anc_UA, anc_1, anc_2, anc_3, anc_4, qubit_num);
		}

		template<typename Block_Encoding, typename State_Prep>
		void Block_Encoding_Hs<Block_Encoding, State_Prep>::operator()(CuSparseState& state) const {
			impl<CuSparseState>(state);
		}

		template<typename Block_Encoding, typename State_Prep>
		void Block_Encoding_Hs<Block_Encoding, State_Prep>::dag(CuSparseState& state) const {
			impl_dag<CuSparseState>(state);
		}

		template<typename Block_Encoding, typename State_Prep, bool PD>
		void Walk_s<Block_Encoding, State_Prep, PD>::operator()(CuSparseState& state) const {
			impl<CuSparseState>(state);
		}

		template<typename Block_Encoding, typename State_Prep, bool PD>
		void Walk_s<Block_Encoding, State_Prep, PD>::dag(CuSparseState& state) const {
			impl_dag<CuSparseState>(state);
		}
	}
}
