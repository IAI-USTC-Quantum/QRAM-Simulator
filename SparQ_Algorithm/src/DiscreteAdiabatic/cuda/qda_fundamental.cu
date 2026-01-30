#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"
#include "DiscreteAdiabatic/qda_fundamental.h"

namespace qram_simulator {
	namespace QDA {

		namespace qda_cuda_detail {
			__global__ void GetOutput_dev(complex_t* state_ps, const System* state, size_t state_size,
				size_t main_reg, size_t anc_1, size_t anc_4,
				size_t main_reg_num, size_t anc_1_num, size_t anc_4_num)
			{
				int idx = blockIdx.x * blockDim.x + threadIdx.x;
				if (idx >= state_size)
					return;

				const System& s = state[idx];

				size_t main_reg_val = CuGetAsUint64(s, main_reg, main_reg_num);
				size_t anc_1_val = CuGetAsUint64(s, anc_1, anc_1_num);
				size_t anc_4_val = CuGetAsUint64(s, anc_4, anc_4_num);

				size_t final_idx = main_reg_val + anc_1_val * pow2(main_reg_num) + anc_4_val * pow2(main_reg_num + anc_1_num);

				cu_real(state_ps[final_idx]) = CuSystemAmplitude(s)[0];
				cu_imag(state_ps[final_idx]) = CuSystemAmplitude(s)[1];

			}
		}

		std::pair<std::vector<complex_t>, double> GetOutput::operator()(const CuSparseState& state) const
		{
			profiler _("QDA::Get Output");

			using namespace qda_cuda_detail;

			size_t main_reg_num = System::size_of(main_reg);
			size_t anc_1_num = System::size_of(anc_1);
			size_t anc_4_num = System::size_of(anc_4);

			thrust::device_vector<complex_t> state_ps(pow2(main_reg_num + 2), complex_t(0, 0));

			CuSparseState state_copy(state);

			// partial trace
			double prob = PartialTraceSelect(anc_registers, std::vector<size_t>(anc_registers.size(), 0))(state_copy);
			
			// Transform 
			size_t blocksize = CUDA_BLOCK_SIZE;
			size_t state_size = state_copy.size();
			size_t nblock = (state_size - 1) / blocksize + 1;
			GetOutput_dev <<< nblock, blocksize >>> (state_ps.data().get(), state_copy.sparse_state_gpu.data().get(), 
				state_size,	main_reg, anc_1, anc_4, main_reg_num, anc_1_num, anc_4_num);

			std::vector<complex_t> host_vec;
			thrust_device_to_std(host_vec, state_ps);

			return { host_vec, 1.0 / prob / prob };

		}
	}
}