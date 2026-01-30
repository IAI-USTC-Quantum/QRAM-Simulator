#include "qram.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"
#include "cuda/qram_circuit_qutrit.cuh"

#ifdef USE_CUDA

namespace qram_simulator {

    struct QRAMLoad_Functor_Control {
        int register_addr;
        int register_data;
        const uint64_t* memory;
        size_t address_size;
        size_t data_size;

        CuCondition_Functor

        QRAMLoad_Functor_Control(int register_addr_, int register_data_,
            const uint64_t* memory_, size_t address_size_, size_t data_size_,
            CuCondition_Params)
            : register_addr(register_addr_), register_data(register_data_),
            memory(memory_), address_size(address_size_), data_size(data_size_),
            CuCondition_Init
        {}

        __host__ __device__ void operator()(System& s) const {
            CuConditionSatisfied(s) {
                uint64_t addr = CuGetAsUint64(s, register_addr, address_size);
                uint64_t current_value = CuGetAsUint64(s, register_data, data_size);
                uint64_t new_value = current_value ^ memory[addr];
                CuGet(s, register_data).value = new_value;
            }
        }
    };


    struct QRAMLoad_Functor {
        int register_addr;
        int register_data;
        const uint64_t* memory;
        size_t address_size;
        size_t data_size;

        QRAMLoad_Functor(int register_addr_, int register_data_,
            const uint64_t* memory_, size_t address_size_, size_t data_size_
        )
            : register_addr(register_addr_), register_data(register_data_),
            memory(memory_), address_size(address_size_), data_size(data_size_)
        {}

        __host__ __device__ void operator()(System& s) const {
            uint64_t addr = CuGetAsUint64(s, register_addr, address_size);
            uint64_t current_value = CuGetAsUint64(s, register_data, data_size);
            uint64_t new_value = current_value ^ memory[addr];
            CuGet(s, register_data).value = new_value;
        }
    };


	void QRAMLoad::operator()(CuSparseState& state) const
	{
        profiler _("QRAMLoad cuda");
        state.move_to_gpu();
		// qram must be able to cast to CuQRAMCircuit
		auto cu_qram = dynamic_cast<const qram_qutrit::CuQRAMCircuit*>(qram);
		if (cu_qram == nullptr) {
			throw std::runtime_error("QRAM is not a CuQRAMCircuit.");
		}

		// if version is not selected or selected as "noisefree"
		if (version == "" ||
			version == "noisefree" ||
			qram->is_noise_free())
		{
			size_t addr_size = std::min(System::size_of(register_addr), qram->address_size);
			size_t data_size = System::size_of(register_data);
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					QRAMLoad_Functor(register_addr, register_data, cu_qram->memory_dev.data().get(),
						addr_size, data_size)
				);
			}
			else
			{
				CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					QRAMLoad_Functor_Control(register_addr, register_data, cu_qram->memory_dev.data().get(),
						addr_size, data_size, CuCondition_Args)
				);
			}
		}
		else {
			throw_bad_switch_case("GPU QRAM Simulator only supports noise-free QRAMs.");
		}
	}
} // namespace qram_simulator

#endif // USE_CUDA