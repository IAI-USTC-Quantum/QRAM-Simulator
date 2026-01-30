#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "cuda/sparse_state_simulator.cuh"

namespace qram_simulator {
	namespace block_encoding {
		namespace block_encoding_tridiagonal
		{
			struct PlusOneAndOverflow_Functor_Control {
				int main_reg_idx;
				int overflow_reg_idx;
				size_t main_reg_size;

				CuCondition_Functor

					PlusOneAndOverflow_Functor_Control(int main_reg_idx_, int overflow_reg_idx_, size_t main_reg_size_, CuCondition_Params)
					: main_reg_idx(main_reg_idx_), overflow_reg_idx(overflow_reg_idx_), main_reg_size(main_reg_size_), CuCondition_Init{
				}

				__host__ __device__ void operator()(System& s) const {
					CuConditionSatisfied(s) {
						auto& val = CuGet(s, main_reg_idx).value;
						auto& overflow_val = CuGet(s, overflow_reg_idx).value;
						if (val == pow2(main_reg_size) - 1)
						{
							overflow_val = (overflow_val + 1) % 2;
							val = 0;
						}
						else {
							val += 1;
						}
					}
				}
			};

			struct PlusOneAndOverflow_Functor {
				int main_reg_idx;
				int overflow_reg_idx;
				size_t main_reg_size;

				PlusOneAndOverflow_Functor(int main_reg_idx_, int overflow_reg_idx_, size_t main_reg_size_)
					: main_reg_idx(main_reg_idx_), overflow_reg_idx(overflow_reg_idx_), main_reg_size(main_reg_size_) 
				{
				}

				__host__ __device__ void operator()(System& s) const {
					auto& val = CuGet(s, main_reg_idx).value;
					auto& overflow_val = CuGet(s, overflow_reg_idx).value;
					if (val == pow2(main_reg_size) - 1)
					{
						overflow_val = (overflow_val + 1) % 2;
						val = 0;
					}
					else {
						val += 1;
					}
				}
			};

			void PlusOneAndOverflow::operator()(CuSparseState& state) const
			{
				state.move_to_gpu();
				auto main_reg_idx = System::get(main_reg);
				auto overflow_reg_idx = System::get(overflow);
				auto main_reg_size = System::size_of(main_reg);

				if (!HasCondition)
				{
					thrust::for_each(thrust::device,
						state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
						PlusOneAndOverflow_Functor(main_reg_idx, overflow_reg_idx, main_reg_size)
					);
				}
				else
				{
					CuCondition_Host_Prepare

					thrust::for_each(thrust::device,
						state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
						PlusOneAndOverflow_Functor_Control(main_reg_idx, overflow_reg_idx, main_reg_size, CuCondition_Args)
					);
				}
			}


			struct PlusOneAndOverflow_Functor_Control_Dag {
				int main_reg_idx;
				int overflow_reg_idx;
				size_t main_reg_size;

				CuCondition_Functor

				PlusOneAndOverflow_Functor_Control_Dag(int main_reg_idx_, int overflow_reg_idx_, size_t main_reg_size_, CuCondition_Params)
				: main_reg_idx(main_reg_idx_), overflow_reg_idx(overflow_reg_idx_), main_reg_size(main_reg_size_), CuCondition_Init
				{
				}

				__host__ __device__ void operator()(System& s) const {
					CuConditionSatisfied(s) {
						auto& val = CuGet(s, main_reg_idx).value;
						auto& overflow_val = CuGet(s, overflow_reg_idx).value;
						if (val == 0)
						{
							overflow_val = (overflow_val + 1) % 2;
							val = pow2(main_reg_size) - 1;
						}
						else {
							val -= 1;
						}
					}
				}
			};

			struct PlusOneAndOverflow_Functor_Dag {
				int main_reg_idx;
				int overflow_reg_idx;
				size_t main_reg_size;

				PlusOneAndOverflow_Functor_Dag(int main_reg_idx_, int overflow_reg_idx_, size_t main_reg_size_)
					: main_reg_idx(main_reg_idx_), overflow_reg_idx(overflow_reg_idx_), main_reg_size(main_reg_size_)
				{
				}

				__host__ __device__ void operator()(System& s) const {
					auto& val = CuGet(s, main_reg_idx).value;
					auto& overflow_val = CuGet(s, overflow_reg_idx).value;
					if (val == 0)
					{
						overflow_val = (overflow_val + 1) % 2;
						val = pow2(main_reg_size) - 1;
					}
					else {
						val -= 1;
					}
				}
			};

			void PlusOneAndOverflow::dag(CuSparseState& state) const
			{
				state.move_to_gpu();
				auto main_reg_idx = System::get(main_reg);
				auto overflow_reg_idx = System::get(overflow);
				auto main_reg_size = System::size_of(main_reg);

				if (!HasCondition)
				{
					thrust::for_each(thrust::device,
						state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
						PlusOneAndOverflow_Functor_Dag(main_reg_idx, overflow_reg_idx, main_reg_size)
					);
				}
				else
				{
					CuCondition_Host_Prepare

					thrust::for_each(thrust::device,
						state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
						PlusOneAndOverflow_Functor_Control_Dag(main_reg_idx, overflow_reg_idx, main_reg_size, CuCondition_Args)
					);
				}
			}


			CU_IMPL(Block_Encoding_Tridiagonal)
		}
	}
}