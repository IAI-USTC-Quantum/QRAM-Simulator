#pragma once
#include "sparse_state_simulator.h"
#include "matrix.h"

namespace qram_simulator
{	
	namespace CKS {
		using walk_angle_function_t = std::function<u22_t(uint64_t, size_t row, size_t col)>;

		HOST_DEVICE	inline void _get_coef_positive_only(size_t mat_data_size, size_t v, size_t row, size_t col, double* mat)
		{
			uint64_t Amax_real = pow2(mat_data_size) - 1;
			v = v % (Amax_real + 1);
			double x = std::sqrt(v * 1.0 / Amax_real);
			double y = std::sqrt(1 - v * 1.0 / Amax_real);

			// return { x, -y,  y, x };
			// use mat index to represent the above matrix
			mat[0] = x;
			mat[1] = 0;
			mat[2] = -y;
			mat[3] = 0;
			mat[4] = y;
			mat[5] = 0;
			mat[6] = x;
			mat[7] = 0;
		}

		//u22_t _get_coef(const SparseMatrix& mat, size_t v, size_t row, size_t col);
		HOST_DEVICE	inline u22_t _get_coef_positive_only(size_t mat_data_size, size_t v, size_t row, size_t col)
		{
			u22_t mat;
			_get_coef_positive_only(mat_data_size, v, row, col, reinterpret_cast<double*>(mat.data()));
			return mat;
		}

		HOST_DEVICE	inline void _get_coef_common(size_t mat_data_size, uint64_t v, size_t row, size_t col, double* mat)
		{
			uint64_t Amax_real = pow2(mat_data_size - 1) - 1;
			v = v % pow2(mat_data_size);
			int64_t v_real = get_complement(v, mat_data_size);
			if (v_real >= 0)
			{
				double x = std::sqrt(v_real * 1.0 / Amax_real);
				double y = std::sqrt(1 - v_real * 1.0 / Amax_real);

				// return { x, -y,  y, x };
				// use mat index to represent the above matrix
				mat[0] = x;
				mat[1] = 0;
				mat[2] = -y;
				mat[3] = 0;
				mat[4] = y;
				mat[5] = 0;
				mat[6] = x;
				mat[7] = 0;
			}
			else
			{
				double x = std::sqrt(-v_real * 1.0 / Amax_real);
				double y = std::sqrt(1 + v_real * 1.0 / Amax_real);

				if (row > col)
				{
					//return { complex_t{0, x}, complex_t{y, 0},
					//		 complex_t{y, 0}, complex_t{0, x} };
					// use mat index to represent the above matrix
					mat[0] = 0;
					mat[1] = x;
					mat[2] = y;
					mat[3] = 0;
					mat[4] = y;
					mat[5] = 0;
					mat[6] = 0;
					mat[7] = x;
				}
				else
				{
					//return { complex_t{0, -x}, complex_t{y, 0},
					//		 complex_t{y, 0}, complex_t{0, -x} };
					// use mat index to represent the above matrix
					mat[0] = 0;
					mat[1] = -x;
					mat[2] = y;
					mat[3] = 0;
					mat[4] = y;
					mat[5] = 0;
					mat[6] = 0;
					mat[7] = -x;
				}
			}
		}

		HOST_DEVICE	inline u22_t _get_coef_common(size_t mat_data_size, uint64_t v, size_t row, size_t col)
		{
			u22_t mat;
			_get_coef_common(mat_data_size, v, row, col, reinterpret_cast<double*>(mat.data()));
			return mat;			
		}

		HOST_DEVICE	inline void u22_dagger(double* mat)
		{
			double tmp;
			mat[1] = -mat[1];
			mat[3] = -mat[3];
			mat[5] = -mat[5];
			mat[7] = -mat[7];

			tmp = mat[2];
			mat[2] = mat[4];
			mat[4] = tmp;

			tmp = mat[5];
			mat[3] = mat[5];
			mat[5] = tmp;	
		}

		HOST_DEVICE	inline void  _get_coef_positive_only_inv(size_t mat_data_size, uint64_t v, size_t row, size_t col, double* mat)
		{
			_get_coef_positive_only(mat_data_size, v, row, col, mat);
			u22_dagger(mat);
		}

		HOST_DEVICE	inline void _get_coef_common_inv(size_t mat_data_size, uint64_t v, size_t row, size_t col, double* mat)
		{
			_get_coef_common(mat_data_size, v, row, col, mat);
			u22_dagger(mat);
		}

		HOST_DEVICE	inline u22_t _get_coef_positive_only_inv(size_t mat_data_size, uint64_t v, size_t row, size_t col)
		{
			u22_t mat;
			_get_coef_positive_only_inv(mat_data_size, v, row, col, reinterpret_cast<double*>(mat.data()));
			return mat;
		}

		HOST_DEVICE	inline u22_t _get_coef_common_inv(size_t mat_data_size, uint64_t v, size_t row, size_t col)
		{
			u22_t mat;
			_get_coef_common_inv(mat_data_size, v, row, col, reinterpret_cast<double*>(mat.data()));
			return mat;
		}

		inline walk_angle_function_t make_func(const SparseMatrix& mat)
		{
			size_t mat_data_size = mat.data_size;
			if (mat.positive_only)
			{
				auto func = [mat_data_size] HOST_DEVICE	(uint64_t v, size_t row, size_t col)
					{
						return _get_coef_positive_only(mat_data_size, v, row, col);
					};
				return func;
			}
			else
			{
				auto func = [mat_data_size] HOST_DEVICE	(uint64_t v, size_t row, size_t col)
					{
						return _get_coef_common(mat_data_size, v, row, col);
					};
				return func;
			}
		}

		inline walk_angle_function_t make_func_inv(const SparseMatrix& mat)
		{
			size_t mat_data_size = mat.data_size;
			if (mat.positive_only)
			{
				auto func = [mat_data_size] HOST_DEVICE	(uint64_t v, size_t row, size_t col)
					{
						return _get_coef_positive_only_inv(mat_data_size, v, row, col);
					};
				return func;
			}
			else
			{
				auto func = [mat_data_size] HOST_DEVICE (uint64_t v, size_t row, size_t col)
					{
						return _get_coef_common_inv(mat_data_size, v, row, col);
					};
				return func;
			}
		}

		// Chebyshev approach
		struct ChebyshevPolynomialCoefficient
		{
			size_t b;
			ChebyshevPolynomialCoefficient(size_t b_)
				:b(b_)
			{ }

			// C(Big, Small) (pick Small from Big)
			// Big*...(Big-Small+1)/(Small*...1)
			double C(size_t Big, size_t Small);

			// Given b, provide j from 0 to b-1
			double coef(size_t j);

			// return true if - (odd)
			// return false if + (even)
			bool sign(size_t j);

			size_t step(size_t j);
		};

		/* For quantum walk */
		struct CondRot_General_Bool_QW : BaseOperator
		{
			using BaseOperator::operator();
			using BaseOperator::dag;

			std::string j;
			std::string k;
			std::string in_name;
			std::string out_name;
			size_t j_id;
			size_t k_id;
			size_t in_id;
			size_t out_id;
			const SparseMatrix* mat;

			CondRot_General_Bool_QW(
				std::string_view j_, std::string_view k_, std::string_view reg_in, std::string_view reg_out,
				const SparseMatrix* mat
			)
				: j(j_), k(k_), in_name(reg_in), out_name(reg_out),
				in_id(System::get(reg_in)), out_id(System::get(reg_out)),
				j_id(System::get(j)), k_id(System::get(k)), mat(mat)
			{
			}

			void operate(size_t l, size_t r, std::vector<System>& state, walk_angle_function_t func) const;

			static bool _is_diagonal(const u22_t& data);
			void _operate_diagonal(size_t l, size_t r,
				std::vector<System>& state, const u22_t& mat) const;

			static bool _is_off_diagonal(const u22_t& data);
			void _operate_off_diagonal(size_t l, size_t r,
				std::vector<System>& state, const u22_t& mat) const;

			void _operate_general(size_t l, size_t r,
				std::vector<System>& state, const u22_t& mat) const;

			void operator()(std::vector<System>& state) const;
			void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
			void operator()(CuSparseState& state) const;
			void dag(CuSparseState& state) const;
#endif
		};

		// quantum binary search
		struct QuantumBinarySearch : SelfAdjointOperator
		{
			using SelfAdjointOperator::operator();
			using SelfAdjointOperator::dag;

			qram_qutrit::QRAMCircuit* qram;
			size_t total_length;
			size_t max_step;

			size_t address_offset_id;
			size_t target_id;
			size_t result_id;

			QuantumBinarySearch(qram_qutrit::QRAMCircuit* qram,
				std::string_view address_offset_register,
				size_t total_length_,
				std::string_view target_register,
				std::string_view result_register);

			QuantumBinarySearch(qram_qutrit::QRAMCircuit* qram,
				size_t address_offset_register,
				size_t total_length_,
				size_t target_register,
				size_t result_register);

			template<typename Ty>
			void impl(Ty& state) const {
				profiler _("QBS");
				auto flag = AddRegister("flag", Boolean, 1)(state);
				Xgate_Bool(flag, 0)(state);

				auto compare_less = AddRegister("compare_less", Boolean, 1)(state);
				auto compare_equal = AddRegister("compare_equal", Boolean, 1)(state);
				auto left_register = AddRegister("left_register", UnsignedInteger, qram->address_size + 1)(state);
				auto right_register = AddRegister("right_register", UnsignedInteger, qram->address_size + 1)(state);
				auto mid_register = AddRegister("mid_register", UnsignedInteger, qram->address_size + 1)(state);
				auto midval_register = AddRegister("midval_register", UnsignedInteger, qram->address_size)(state);

				// int flag_id = System::get("flag");

				Assign(address_offset_id, left_register)(state);
				Add_UInt_ConstUInt(left_register, total_length, right_register)(state);

				for (size_t iteration_level = 0; iteration_level < max_step; ++iteration_level)
				{
					/* compute the mid */
					GetMid_UInt_UInt(left_register, right_register, mid_register)
						.conditioned_by_nonzeros(flag)(state);
					//(StatePrint(StatePrintDisplay::Detail))(state);

					/* load value */
					QRAMLoad(qram, mid_register, midval_register)
						.conditioned_by_nonzeros(flag)(state);
					//(StatePrint(StatePrintDisplay::Detail))(state);

					/* compare to decide the branch */
					Compare_UInt_UInt(midval_register, target_id, compare_less, compare_equal)
						.conditioned_by_nonzeros(flag)(state);

					/* if found, move the mid register to outside */
					Assign(mid_register, result_id)
						.conditioned_by_nonzeros({ compare_equal, flag })(state);

					if (iteration_level != max_step - 1)
					{
						/* update flag (unnecessary condition) */
						Assign(compare_equal, flag)(state);
						/* update the left/right register with mid register */
						Swap_General_General(left_register, mid_register)
							.conditioned_by_nonzeros({ compare_less, flag })(state);

						Xgate_Bool(compare_less, 0)(state);

						Swap_General_General(right_register, mid_register)
							.conditioned_by_nonzeros({ compare_less, flag })(state);

						Push(mid_register, fmt::format("{}-{}", "mid_register", iteration_level))(state);
						Push(midval_register, fmt::format("{}-{}", "midval_register", iteration_level))(state);
						Push(compare_less, fmt::format("{}-{}", "compare_less", iteration_level))(state);
						Push(compare_equal, fmt::format("{}-{}", "compare_equal", iteration_level))(state);
					}
				}

				// FlipBools(result_register).conditioned_by("flag")(state);

				// todo: auto-uncompute
				// uncompute all garbage variables
				for (size_t iteration_level = max_step; iteration_level --> 0;)
				{
					if (iteration_level != max_step - 1)
					{
						(Pop(compare_equal))(state);
						(Pop(compare_less))(state);
						(Pop(midval_register))(state);
						(Pop(mid_register))(state);

						Swap_General_General(right_register, mid_register)
							.conditioned_by_nonzeros({ compare_less, flag })(state);

						Xgate_Bool(compare_less, 0)(state);
						Swap_General_General(left_register, mid_register)
							.conditioned_by_nonzeros({ compare_less, flag })(state);

						Assign(compare_equal, flag)(state);
					}

					Compare_UInt_UInt(midval_register, target_id, compare_less, compare_equal)
						.conditioned_by_nonzeros(flag)(state);

					QRAMLoad(qram, mid_register, midval_register)
						.conditioned_by_nonzeros(flag)(state);

					GetMid_UInt_UInt(left_register, right_register, mid_register)
						.conditioned_by_nonzeros(flag)(state);
				}

				Add_UInt_ConstUInt(left_register, total_length, right_register)(state);
				Assign(address_offset_id, left_register)(state);
				Xgate_Bool(flag, 0)(state);
				(RemoveRegister(compare_less))(state);
				(RemoveRegister(compare_equal))(state);
				(RemoveRegister(left_register))(state);
				(RemoveRegister(right_register))(state);
				(RemoveRegister(mid_register))(state);
				(RemoveRegister(midval_register))(state);
				(RemoveRegister(flag))(state);
			}
		

			template<typename Ty>
			void impl_dag(Ty& state) const {
				impl<Ty>(state);
			}

			COMPOSITE_OPERATION
		};

		// quantum binary search
		struct QuantumBinarySearchFast : SelfAdjointOperator
		{
			using SelfAdjointOperator::operator();
			using SelfAdjointOperator::dag;

			qram_qutrit::QRAMCircuit* qram;
			size_t total_length;
			size_t max_step;

			size_t address_offset_id;
			size_t target_id;
			size_t result_id;

			//int iteration_level;

			QuantumBinarySearchFast(qram_qutrit::QRAMCircuit* qram,
				std::string_view address_offset_register,
				size_t total_length_,
				std::string_view target_register,
				std::string_view result_register);

			QuantumBinarySearchFast(qram_qutrit::QRAMCircuit* qram,
				size_t address_offset_register,
				size_t total_length_,
				size_t target_register,
				size_t result_register);

			size_t binary_search(size_t offset, size_t target) const;
			void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
			void operator()(CuSparseState& state) const;
#endif
		};

		struct GetRowAddr : SelfAdjointOperator
		{
			using SelfAdjointOperator::operator();
			using SelfAdjointOperator::dag;

			size_t offset_id;
			size_t row_id;
			size_t row_sz;
			size_t row_offset_id;
			GetRowAddr(std::string_view reg_offset,
				std::string_view reg_row,
				size_t row_sz_,
				std::string_view reg_row_offset)
			{
				offset_id = System::get(reg_offset);
				row_id = System::get(reg_row);
				row_offset_id = System::get(reg_row_offset);
				row_sz = row_sz_;
			}

			GetRowAddr(int reg_offset,
				int reg_row,
				size_t row_sz_,
				int reg_row_offset)
			{
				offset_id = reg_offset;
				row_id = reg_row;
				row_offset_id = reg_row_offset;
				row_sz = row_sz_;
			}

			void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
			void operator()(CuSparseState& state) const;
#endif
		};

		struct GetDataAddr : SelfAdjointOperator
		{
			using SelfAdjointOperator::operator();
			using SelfAdjointOperator::dag;

			size_t offset_id;
			size_t row_id;
			size_t row_sz;
			size_t col_sparse_id;
			size_t row_data_id;

			GetDataAddr(std::string_view reg_offset, std::string_view reg_row,
				std::string_view reg_col_sparse, size_t row_sz_, std::string_view reg_data_offset)
			{
				offset_id = System::get(reg_offset);
				row_id = System::get(reg_row);
				col_sparse_id = System::get(reg_col_sparse);
				row_data_id = System::get(reg_data_offset);
				row_sz = row_sz_;
			}

			GetDataAddr(size_t reg_offset, size_t reg_row,
				size_t reg_col_sparse, size_t row_sz_, size_t reg_data_offset)
			{
				offset_id = reg_offset;
				row_id = reg_row;
				col_sparse_id = reg_col_sparse;
				row_data_id = reg_data_offset;
				row_sz = row_sz_;
			}

			void operator()(std::vector<System>& state) const;

#ifdef USE_CUDA
			void operator()(CuSparseState& state) const;
#endif
		};

		/*
		|offset>|i>|s_j>|0>			->
		|offset>|i>|s_j>|a_{ij}>
		*/
		struct SparseMatrixOracle1 : SelfAdjointOperator
		{
			using SelfAdjointOperator::operator();
			using SelfAdjointOperator::dag;

			qram_qutrit::QRAMCircuit* qram;
			std::string reg_offset;
			std::string reg_row;
			std::string reg_col_id; // position in the sparse-compact storage
			std::string reg_output;
			size_t row_size;

			SparseMatrixOracle1(qram_qutrit::QRAMCircuit* qram,
				std::string_view reg_offset,
				std::string_view reg_row,
				std::string_view reg_col_id,
				std::string_view reg_output,
				size_t row_size_);

			template<typename Ty>
			void impl(Ty& state) const
			{
				AddRegister("data_addr", UnsignedInteger, qram->address_size)(state);
				GetDataAddr(reg_offset, reg_row, reg_col_id, row_size, "data_addr")(state);
				QRAMLoad(qram, "data_addr", reg_output)(state);
				GetDataAddr(reg_offset, reg_row, reg_col_id, row_size, "data_addr")(state);
				RemoveRegister("data_addr")(state);
			}

			template<typename Ty>
			void impl_dag(Ty& state) const
			{
				impl<Ty>(state);
			}

			COMPOSITE_OPERATION
		};

		/*
		|offset>|i>|j>|0>			->
		|offset>|i>|s_j>
		*/
		struct SparseMatrixOracle2 : BaseOperator
		{
			using BaseOperator::operator();
			using BaseOperator::dag;

			qram_qutrit::QRAMCircuit* qram;
			std::string reg_sparse_offset;
			std::string reg_row;
			std::string reg_col; // the column in the matrix
			std::string reg_search_result;
			size_t row_size;
			SparseMatrixOracle2(qram_qutrit::QRAMCircuit* qram,
				std::string_view reg_sparse_offset,
				std::string_view reg_row_,
				std::string_view reg_col_,
				std::string_view reg_search_result_,
				size_t row_size);

			template<typename Ty>
			void impl(Ty& state) const
			{
				// |i>|j> -> |i>|s_j>

				// |offset>|i>|j>
				AddRegister("row_addr", UnsignedInteger, qram->address_size)(state);

				// |offset>|i>|j>|row_addr = offset + i * row_size>
				GetRowAddr(reg_sparse_offset, reg_row, row_size, "row_addr")(state);

				// |offset>|i>|j>|row_addr>|result = s_j>
				//QuantumBinarySearch(qram, "row_addr", row_size, reg_col, reg_search_result)(state);
				QuantumBinarySearchFast(qram, "row_addr", row_size, reg_col, reg_search_result)(state);

				// |offset>|i>|0>|row_addr>|result> 
				QRAMLoad(qram, reg_search_result, reg_col)(state);

				// |offset>|i>|result>|row_addr>|0> 
				Swap_General_General(reg_col, reg_search_result)(state);

				// |offset>|i>|s_j>|row_addr>|0> 
				AddAssign_AnyInt_AnyInt(reg_col, "row_addr").dag(state);

				// |offset>|i>|s_j>|0>|0> 
				GetRowAddr(reg_sparse_offset, reg_row, row_size, "row_addr")(state);

				RemoveRegister("row_addr")(state);
			}


			template<typename Ty>
			void impl_dag(Ty& state) const
			{
				AddRegister("row_addr", UnsignedInteger, qram->address_size)(state);

				// |offset>|i>|s_j>|0>|0>|row_addr>
				GetRowAddr(reg_sparse_offset, reg_row, row_size, "row_addr")(state);
				AddAssign_AnyInt_AnyInt(reg_col, "row_addr")(state);
				Swap_General_General(reg_col, reg_search_result)(state);
				QRAMLoad(qram, reg_search_result, reg_col)(state);
				//QuantumBinarySearch(qram, "row_addr", row_size, reg_col, reg_search_result)(state);
				QuantumBinarySearchFast(qram, "row_addr", row_size, reg_col, reg_search_result)(state);
				GetRowAddr(reg_sparse_offset, reg_row, row_size, "row_addr")(state);

				RemoveRegister("row_addr")(state);
			}

			COMPOSITE_OPERATION
		};

		/*
		* To compute k from j,l, out-of-place
		* Implemented by QRAM query
		*
		* |offset>|l>|z>			->
		* |offset>|l>|z + k>
		*/

		struct SparseMatrixOracle2_ComputeCol : BaseOperator
		{
			using BaseOperator::operator();

			qram_qutrit::QRAMCircuit* qram;
			std::string sparse_offset;
			std::string k; // j
			std::string l; // s_j
			std::string addr_offset;
			size_t row_size;
			SparseMatrixOracle2_ComputeCol(qram_qutrit::QRAMCircuit* qram,
				std::string_view sparse_offset,
				std::string_view k,
				std::string_view l,
				std::string_view addr_offset,
				size_t row_size);
			void operator()(std::vector<System>& state) const;
		};

		/*
		* To compute l from j,k, out-of-place
		* Implemented by QBS
		*
		|offset>|k>|z>			->
		|offset>|k>|z + l>
		*/
		struct SparseMatrixOracle2_ComputeSparsity : BaseOperator
		{
			using BaseOperator::operator();
			using BaseOperator::dag;

			qram_qutrit::QRAMCircuit* qram;
			std::string sparse_offset;
			std::string k; // j
			std::string l; // s_j
			size_t row_size;
			SparseMatrixOracle2_ComputeSparsity(
				qram_qutrit::QRAMCircuit* qram,
				std::string_view sparse_offset,
				std::string_view k,
				std::string_view l,
				size_t row_size);
			void operator()(std::vector<System>& state) const;
		};

		// prepare from |j> to |\psi_j>
		struct T : BaseOperator
		{
			using BaseOperator::operator();
			using BaseOperator::dag;

			qram_qutrit::QRAMCircuit* qram;
			std::string reg_data_offset;
			std::string reg_sparse_offset;
			std::string reg_j;
			std::string reg_b1;
			std::string reg_k;
			std::string reg_b2;
			std::string reg_search_result;
			size_t nnz_col;
			size_t data_size;
			//walk_angle_function_t func;
			//walk_angle_function_t func_inv;
			const SparseMatrix* mat;

			T(qram_qutrit::QRAMCircuit* qram_,
				std::string_view reg_data_offset_,
				std::string_view reg_sparse_offset_,
				std::string_view reg_j_, std::string_view reg_b1_,
				std::string_view reg_k_, std::string_view reg_b2_,
				std::string_view reg_search_result_,
				size_t nnz_col_, size_t data_size_,
				const SparseMatrix* mat_)
				: qram(qram_), reg_data_offset(reg_data_offset_), reg_sparse_offset(reg_sparse_offset_),
				reg_j(reg_j_), reg_b1(reg_b1_), reg_k(reg_k_), reg_b2(reg_b2_),
				reg_search_result(reg_search_result_), nnz_col(nnz_col_), data_size(data_size_),
				mat(mat_)
			{
			}

			template<typename Ty>
			void impl(Ty& system_states) const
			{
				profiler _("T");
				AddRegister("data", UnsignedInteger, data_size)(system_states);

				// |j> -> |j>��|s_k>
				Hadamard_Int(reg_k, log2(nnz_col))(system_states);

				// |j>��|s_k> -> |j>��|s_k>|d[j,k]>
				SparseMatrixOracle1(qram, reg_data_offset,
					reg_j, reg_k, "data", nnz_col)(system_states);

				// |j>��|s_k>(a|0>+b|1>) -> |j>��|k>(a|0>+b|1>)
				SparseMatrixOracle2(qram, reg_sparse_offset,
					reg_j, reg_k, reg_search_result, nnz_col).dag(system_states);

				// |j>��|s_k>|d[j,k]> -> |j>��|s_k>|d[j,k]>(a|0>+b|1>)
				CondRot_General_Bool_QW(reg_j, reg_k, "data", reg_b2, mat)(system_states);

				// |j>��|s_k>(a|0>+b|1>) -> |j>��|k>(a|0>+b|1>)
				SparseMatrixOracle2(qram, reg_sparse_offset,
					reg_j, reg_k, reg_search_result, nnz_col)(system_states);

				// |j>��|s_k>|d[j,k]>(a|0>+b|1>) -> |j>��|s_k>(a|0>+b|1>)
				SparseMatrixOracle1(qram, reg_data_offset,
					reg_j, reg_k, "data", nnz_col)(system_states);
				RemoveRegister("data")(system_states);

				// |j>��|s_k>(a|0>+b|1>) -> |j>��|k>(a|0>+b|1>)
				SparseMatrixOracle2(qram, reg_sparse_offset,
					reg_j, reg_k, reg_search_result, nnz_col).dag(system_states);

				CheckNan()(system_states);
			}

			template<typename Ty>
			void impl_dag(Ty& system_states) const
			{
				profiler _("T.dag");
				AddRegister("data", UnsignedInteger, data_size)(system_states);

				SparseMatrixOracle2(qram, reg_sparse_offset,
					reg_j, reg_k, reg_search_result, nnz_col)(system_states);

				CheckNan()(system_states);

				SparseMatrixOracle1(qram, reg_data_offset,
					reg_j, reg_k, "data", nnz_col)(system_states);

				SparseMatrixOracle2(qram, reg_sparse_offset,
					reg_j, reg_k, reg_search_result, nnz_col)(system_states);

				CondRot_General_Bool_QW(reg_j, reg_k, "data", reg_b2, mat).dag(system_states);
				ClearZero()(system_states);

				SparseMatrixOracle2(qram, reg_sparse_offset,
					reg_j, reg_k, reg_search_result, nnz_col).dag(system_states);

				CheckNormalization()(system_states);

				SparseMatrixOracle1(qram, reg_data_offset,
					reg_j, reg_k, "data", nnz_col)(system_states);

				Hadamard_Int(reg_k, log2(nnz_col))(system_states);
				ClearZero()(system_states);

				RemoveRegister("data")(system_states);
			}

			COMPOSITE_OPERATION
		};

		struct QuantumWalk : BaseOperator
		{
			std::string j, b1, k, b2, j_comp, k_comp, data_offset, sparse_offset;
			qram_qutrit::QRAMCircuit* qram;
			SparseMatrix mat;

			QuantumWalk(
				qram_qutrit::QRAMCircuit* qram_,
				std::string_view j_, std::string_view b1_,
				std::string_view k_, std::string_view b2_,
				std::string_view j_comp_,
				std::string_view k_comp_,
				std::string_view data_offset_,
				std::string_view sparse_offset_,
				const SparseMatrix& mat_)
				:qram(qram_), j(j_), b1(b1_), k(k_), b2(b2_),
				j_comp(j_comp_), k_comp(k_comp_),
				data_offset(data_offset_), sparse_offset(sparse_offset_),
				mat(mat_)
			{}

			template<typename Ty>
			void impl(Ty& system_states) const
			{
				profiler _("QuantumWalk");
				size_t addr_size = log2(mat.get_data().size());
				size_t data_size = mat.data_size;
				size_t reg_size = std::max(addr_size, data_size);
				size_t offset = mat.get_sparsity_offset();
				//size_t n_row = mat.n_row;
				size_t nnz_col = mat.nnz_col;

				T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
					nnz_col, reg_size, &mat)
					.dag(system_states);

				// (StatePrint(StatePrintDisplay::Detail))(system_states);

				ZeroConditionalPhaseFlip({ b1, k, b2, k_comp })
					(system_states);

				//RangeConditionalPhaseFlip(j, n_row)
				//	(system_states);

				// (StatePrint(StatePrintDisplay::Detail))(system_states);

				T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
					nnz_col, reg_size, &mat)
					(system_states);

				// (StatePrint(StatePrintDisplay::Detail))(system_states);

				Swap_General_General(j, k)(system_states);
				Swap_General_General(b1, b2)(system_states);
				Swap_General_General(j_comp, k_comp)(system_states);
			}

			template<typename Ty>
			void impl_dag(Ty& system_states) const
			{
				throw_not_implemented("QuantumWalk::impl_dag is not implemented");
			}

			COMPOSITE_OPERATION
		};

		template<typename Ty = SparseState>
		class QuantumWalkNSteps
		{
		public:
			std::string data_offset = "data_offset";
			std::string sparse_offset = "sparse_offset";
			std::string j = "row_id";
			std::string b1 = "reg_b1";
			std::string k = "col_id";
			std::string b2 = "reg_b2";
			std::string j_comp = "j_comp";
			std::string k_comp = "k_comp";
			SparseMatrix mat;
			size_t addr_size, data_size, offset, n_row, nnz_col;
			size_t default_register_size;
			qram_qutrit::QRAMCircuit* qram;
			constexpr static int suggest_reserve = 1024000;

			QuantumWalkNSteps(const SparseMatrix& mat_,
				qram_qutrit::QRAMCircuit* qram_)
			{
				mat = mat_;
				auto&& data = mat.get_data();
				addr_size = log2(data.size());
				data_size = mat.data_size;
				offset = mat.get_sparsity_offset();
				n_row = mat.n_row;
				nnz_col = mat.nnz_col;
				default_register_size = std::max(addr_size, data_size);
				qram = qram_;
			}

			QuantumWalkNSteps(const SparseMatrix& mat_)
			{
				mat = mat_;
				auto&& data = mat.get_data();
				addr_size = log2(data.size());
				data_size = mat.data_size;
				offset = mat.get_sparsity_offset();
				n_row = mat.n_row;
				nnz_col = mat.nnz_col;

				default_register_size = std::max(addr_size, data_size);

				qram = new qram_qutrit::QRAMCircuit(addr_size, data_size, std::move(data));
			}

			~QuantumWalkNSteps()
			{
				delete qram;
			}

			std::string GetVecInputReg() const
			{
				return j;
			}

			size_t get_init_size() const
			{
				return log2(mat.n_row);
			}

			void InitEnvironment()
			{
				/* Register Init */
				System::add_register(data_offset, UnsignedInteger, default_register_size);
				System::add_register(sparse_offset, UnsignedInteger, default_register_size);
				System::add_register(j, UnsignedInteger, default_register_size);
				System::add_register(b1, Boolean, 1);
				System::add_register(k, UnsignedInteger, default_register_size);
				System::add_register(b2, Boolean, 1);
				System::add_register(j_comp, UnsignedInteger, default_register_size);
				System::add_register(k_comp, UnsignedInteger, default_register_size);
			}

			Ty CreateSys()
			{
				Ty system_states(1);
				Init_Unsafe(sparse_offset, offset)(system_states);
				return system_states;
			}

			Ty MakeNStepState(size_t n_steps)
			{
				profiler _("MakeNStepState");
				auto system_states = CreateSys();

				Hadamard_Int(j, get_init_size())(system_states);
				ClearZero()(system_states);
				if (n_steps == 0)
					return system_states;

				FirstStep(system_states);

				fmt::print("State size = {}\n", system_states.size());
				for (size_t i = 0; i < n_steps - 1; ++i)
				{
					StepImplOneStep(system_states);
				}
				return system_states;
			}

			void FirstStep(Ty& system_states)
			{
				T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
					nnz_col, default_register_size, &mat)
					(system_states);

				Swap_General_General(j, k)(system_states);
				Swap_General_General(b1, b2)(system_states);
				Swap_General_General(j_comp, k_comp)(system_states);

				T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
					nnz_col, default_register_size, &mat)
					.dag(system_states);
			}
			void StepImplOneStep(Ty& system_states)
			{
				ZeroConditionalPhaseFlip({ j_comp, k_comp, b1, k, b2 })
					(system_states);

				T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
					nnz_col, default_register_size, &mat)
					(system_states);

				CheckNan()(system_states);

				Swap_General_General(j, k)(system_states);
				Swap_General_General(b1, b2)(system_states);
				Swap_General_General(j_comp, k_comp)(system_states);

				T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
					nnz_col, default_register_size, &mat)
					.dag(system_states);

				CheckNan()(system_states);
			}
			void Step(Ty& system_states)
			{
				profiler _("Step");
				StepImplOneStep(system_states);
				StepImplOneStep(system_states);
			}
		};

		struct LCU_Container
		{
			std::vector<System> current_state;
			double kappa;
			double eps;
			size_t b;
			size_t j0;
			QuantumWalkNSteps<std::vector<System>> quantum_walk_obj;
			ChebyshevPolynomialCoefficient chebyshev_obj;

			LCU_Container(const SparseMatrix& mat, double kappa_, double eps_) :
				quantum_walk_obj(mat),
				kappa(kappa_), eps(eps_),
				b(static_cast<size_t>(kappa* kappa* (std::log(kappa) - std::log(eps)))),
				chebyshev_obj(b),
				j0(static_cast<size_t>(std::sqrt(b* (std::log(4 * b) - std::log(eps)))))
			{
				quantum_walk_obj.InitEnvironment();
			}

			auto GetInputVecReg()
			{
				return quantum_walk_obj.GetVecInputReg();
			}

			std::vector<System> state_of_j(size_t j);
			void add(std::vector<System> new_state, double coef, bool sign);
			void iterate();
		};

		/* The noisefree version */
		/* It will iterate over the original state,
		   without making extra copies.

		   This optimization cannot be applied to
		   noisy simulation.
		*/

		template<typename StateTy = SparseState>
		struct LCU_Container_NoiseFree
		{
			StateTy current_state;
			StateTy step_state;
			QuantumWalkNSteps<StateTy> quantum_walk_obj;
			double kappa;
			double eps;
			size_t b;
			size_t j0;
			size_t j = 0; // iteration variable
			double a = 0;
			ChebyshevPolynomialCoefficient chebyshev_obj;

			LCU_Container_NoiseFree(const SparseMatrix& mat, double kappa, double eps) :
				quantum_walk_obj(mat),
				kappa(kappa), eps(eps),
				b(static_cast<size_t>(kappa* kappa* (std::log(kappa) - std::log(eps)))),
				chebyshev_obj(b),
				j0(static_cast<size_t>(std::sqrt(b* (std::log(4 * b) - std::log(eps)))))
			{
				quantum_walk_obj.InitEnvironment();
				step_state = quantum_walk_obj.CreateSys();
			}

			auto GetInputVecReg() const
			{
				return quantum_walk_obj.GetVecInputReg();
			}

			size_t get_addr_size() const
			{
				return quantum_walk_obj.addr_size;
			}

			template<typename Ty>
			void ExternalInput()
			{
				(Ty(GetInputVecReg()))(step_state);
				ClearZero()(step_state);
				quantum_walk_obj.FirstStep(step_state);
				fmt::print("State size = {}\n", step_state.size());
			}

			template<typename Ty, typename ...Args>
			void ExternalInput(Args &&...args)
			{
				Ty(GetInputVecReg(), std::forward<Args>(args)...)(step_state);
				ClearZero()(step_state);
				quantum_walk_obj.FirstStep(step_state);
				fmt::print("State size = {}\n", step_state.size());
			}

			void ExternalInput_V2(const BaseOperator& op)
			{
				op(step_state);
				ClearZero()(step_state);
				quantum_walk_obj.FirstStep(step_state);
				fmt::print("State size = {}\n", step_state.size());
			}

			bool Step() {
				if (j <= j0) {
					if (j != 0)
					{
						quantum_walk_obj.Step(step_state);
					}
				
					a += chebyshev_obj.coef(j);
					Add(step_state, chebyshev_obj.coef(j), chebyshev_obj.sign(j));
					++j;
					return true;
				}
				else
					return false;
			}

			void Add(const StateTy& new_state, double coef, bool sign)
			{
				if (sign)
					coef *= -1;

				add_systems(current_state, new_state, coef);
			}

			// inline std::vector<complex_t> _impl_get_output(StateTy& state) const
			// {
			// 	std::vector<complex_t> m(quantum_walk_obj.n_row, 0);
			// 	auto id = System::get(GetInputVecReg());
			// 	for (System& s : state)
			// 	{
			// 		StateStorage& st = s.get(id);
			// 		uint64_t v = st.as<uint64_t>(System::size_of(id));
			// 		m[s.GetAs(id, uint64_t)] = s.amplitude;
			// 	}
			// 	return m;
			// }

			double _impl_partial_trace(StateTy& state) const
			{
				double ret = PartialTraceSelect({
					  {quantum_walk_obj.b1, 0},
					  {quantum_walk_obj.k, 0},
					  {quantum_walk_obj.b2, 0},
					  {quantum_walk_obj.j_comp, 0},
					  {quantum_walk_obj.k_comp, 0},
					  {quantum_walk_obj.sparse_offset, quantum_walk_obj.offset},
					  {quantum_walk_obj.data_offset, 0}
					})(state);
	
				ret *= PartialTraceSelectRange(
					quantum_walk_obj.j,
					{ 0, quantum_walk_obj.n_row - 1 })(state);
	
				return 1.0 / ret / ret / a / a;
			}

			double PartialTrace()
			{
				return _impl_partial_trace(current_state);
			}

			std::tuple<StateTy, double> PartialTrace_Nondestructive()  const
			{
				StateTy state = current_state;
				double ret = _impl_partial_trace(state);
				return { state, ret };
			}

		};

		/* Directly use the theory to validate */
		struct LCU_Container_Theory
		{
			SparseMatrix mat;
			DenseMatrix<complex_t> densemat;
			double kappa;
			double eps;
			size_t b;
			size_t j0;
			size_t j = 0; // iteration variable
			double a = 0;
			ChebyshevPolynomialCoefficient chebyshev_obj;
			DenseVector<complex_t> current_state;
			DenseVector<complex_t> step_state;
			DenseVector<complex_t> vec0; // for chebyshev iteration
			DenseVector<complex_t> vec1; // for chebyshev iteration

			LCU_Container_Theory(const SparseMatrix& mat_, double kappa_, double eps_) :
				mat(mat_),
				densemat(sparse2dense<complex_t>(mat)),
				kappa(kappa_),
				eps(eps_),
				b(static_cast<size_t>(kappa* kappa* (std::log(kappa) - std::log(eps)))),
				chebyshev_obj(b),
				j0(static_cast<size_t>(std::sqrt(b* (std::log(4 * b) - std::log(eps))))),
				current_state(mat.n_row),
				step_state(mat.n_row),
				vec0(mat.n_row),
				vec1(mat.n_row)
			{
				densemat = densemat / pow2(mat.data_size);
				densemat = densemat / mat.nnz_col;
				std::fill(step_state.data.begin(), step_state.data.end(), 1.0);
				step_state = step_state / step_state.norm2();
				vec0 = step_state;
				vec1 = densemat * vec0;
			}

			inline void Add(const DenseVector<complex_t>& new_state,
				double coef, bool sign)
			{
				if (sign)
					coef *= -1;

				for (size_t i = 0; i < current_state.data.size(); ++i)
				{
					current_state[i] += coef * new_state[i];
				}
			}

			DenseVector<complex_t> MakeStepState();
			bool Step();

			std::pair<DenseVector<complex_t>, double> GetOutput() const;
		};


		std::vector<complex_t> my_linear_solver_reference(const SparseMatrix& mat);
		std::vector<complex_t> my_linear_solver_reference(const SparseMatrix& mat, const DenseVector<double>& vec);

	} // namespace CKS
}