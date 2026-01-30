#include "parallel_phase_operations.h"
#include "debugger.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"

#ifdef USE_CUDA

namespace qram_simulator {
	struct ZeroConditionalPhaseFlip_Functor_Control {
        int* ids;
        size_t* sizes;
        size_t n_regs;
		CuCondition_Functor

		ZeroConditionalPhaseFlip_Functor_Control(int* ids_, size_t* sizes_, size_t n_regs_, CuCondition_Params)
		: ids(ids_), sizes(sizes_), n_regs(n_regs_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				bool all_zero = true;
				for (size_t i = 0; i < n_regs; i++) {
					if (CuGetAsUint64(s, ids[i], sizes[i]) != 0) {
						all_zero = false;
						break;
					}
				}
				if (all_zero) {
					double* amplitude = CuSystemAmplitude(s);
					amplitude[0] = -amplitude[0];
					amplitude[1] = -amplitude[1];
				}
			}
		}
	};

	struct ZeroConditionalPhaseFlip_Functor {
		int* ids;
		size_t* sizes;
        size_t n_regs;

		ZeroConditionalPhaseFlip_Functor(int* ids_, size_t* sizes_, size_t n_regs_)
			: ids(ids_), sizes(sizes_), n_regs(n_regs_) {
		}

		__host__ __device__ void operator()(System& s) const {
			bool all_zero = true;
			for (size_t i = 0; i < n_regs; i++) {
				if (CuGetAsUint64(s, ids[i], sizes[i]) != 0) {
					all_zero = false;
					break;
				}
			}
			if (all_zero) {
				double* amplitude = CuSystemAmplitude(s);
				amplitude[0] = -amplitude[0];
				amplitude[1] = -amplitude[1];
			}
		}
	};

	void ZeroConditionalPhaseFlip::operator()(CuSparseState& state) const
	{
        profiler _("ZeroConditionalPhaseFlip cuda");
		state.move_to_gpu();
		thrust::device_vector<int> ids_dev(ids.begin(), ids.end());
		thrust::device_vector<size_t> sizes_dev;
        for (size_t i = 0; i < ids.size(); i++) {
            sizes_dev.push_back(System::size_of(ids[i]));
        }
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				ZeroConditionalPhaseFlip_Functor(ids_dev.data().get(), sizes_dev.data().get(), ids.size())
			);
		}
        else
        {
            CuCondition_Host_Prepare
                thrust::for_each(thrust::device,
                    state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                    ZeroConditionalPhaseFlip_Functor_Control(ids_dev.data().get(), sizes_dev.data().get(), ids.size(), CuCondition_Args)
                );
        }
	}

    struct Reflection_Bool_Functor_Control {
        int* regs;
        size_t* sizes;
        size_t n_regs;
        bool inverse;
        CuCondition_Functor

            Reflection_Bool_Functor_Control(int* regs_, size_t* sizes_, size_t n_regs_, bool inverse_, CuCondition_Params)
            : regs(regs_), sizes(sizes_), n_regs(n_regs_), inverse(inverse_), CuCondition_Init{
        }

        __host__ __device__ void operator()(System& s) const
        {
            CuConditionSatisfied(s) {
                bool _iszero = true;
                for (int i = 0; i < n_regs; i++) {
                    if (CuGetAsUint64(s, regs[i], sizes[i]) != 0) {
                        _iszero = false;
                        break;
                    }
                }
                
                if (!(inverse ^ _iszero)) {
                    double* amplitude = CuSystemAmplitude(s);
                    amplitude[0] = -amplitude[0];
                    amplitude[1] = -amplitude[1];
                }
            }
        }
    };

    struct Reflection_Bool_Functor {
        int* regs;
        size_t* sizes;
        size_t n_regs;
        bool inverse;

        Reflection_Bool_Functor(int* regs_, size_t* sizes_, size_t n_regs_, bool inverse_)
            : regs(regs_), sizes(sizes_), n_regs(n_regs_), inverse(inverse_) {
        }

        __host__ __device__ void operator()(System& s) const {
            bool _iszero = true;
            for (int i = 0; i < n_regs; i++) {
                if (CuGetAsUint64(s, regs[i], sizes[i]) != 0) {
                    _iszero = false;
                    break;
                }
            }
            if (!(inverse ^ _iszero)) {
                double* amplitude = CuSystemAmplitude(s);
                amplitude[0] = -amplitude[0];
                amplitude[1] = -amplitude[1];
            }
        }
    };

    void Reflection_Bool::operator()(CuSparseState& state) const
    {
        profiler _("Reflection_Bool cuda");
        state.move_to_gpu();
        thrust::device_vector<int> regs_dev(regs.begin(), regs.end());
        // thrust::device_vector<int> regs_dev;
        thrust::device_vector<size_t> sizes_dev;
        for (size_t i = 0; i < regs.size(); i++) {
            // regs_dev.push_back(regs[i]);
            sizes_dev.push_back(System::size_of(regs[i]));
        }

        if (!HasCondition)
        {
            thrust::for_each(thrust::device,
                state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                Reflection_Bool_Functor(regs_dev.data().get(), sizes_dev.data().get(), regs.size(), inverse)
            );
        }
        else
        {
            CuCondition_Host_Prepare
            thrust::for_each(thrust::device,
                state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                Reflection_Bool_Functor_Control(regs_dev.data().get(), sizes_dev.data().get(), regs.size(), inverse, CuCondition_Args)
            );
        }
    }

    struct GlobalPhase_Int_Functor_Control {
        double c_real;
        double c_imag;
        CuCondition_Functor

            GlobalPhase_Int_Functor_Control(double c_real, double c_imag, CuCondition_Params)
            : c_real(c_real), c_imag(c_imag), CuCondition_Init{
        }

        __host__ __device__ void operator()(System& s) const {
            CuConditionSatisfied(s) {
                double* amplitude = CuSystemAmplitude(s);
                double real_part = amplitude[0] * c_real - amplitude[1] * c_imag;
                double imag_part = amplitude[0] * c_imag + amplitude[1] * c_real;
                amplitude[0] = real_part;
                amplitude[1] = imag_part;
            }
        }
    };

    struct GlobalPhase_Int_Functor {
        double c_real;
        double c_imag;

        GlobalPhase_Int_Functor(double c_real, double c_imag)
            : c_real(c_real), c_imag(c_imag) {
        }

        __host__ __device__ void operator()(System& s) const {
            double* amplitude = CuSystemAmplitude(s);
            double real_part = amplitude[0] * c_real - amplitude[1] * c_imag;
            double imag_part = amplitude[0] * c_imag + amplitude[1] * c_real;
            amplitude[0] = real_part;
            amplitude[1] = imag_part;
        }
    };

    struct GlobalPhase_Int_Functor_Control_Dag {
        double c_real;
        double c_imag;
        CuCondition_Functor

            GlobalPhase_Int_Functor_Control_Dag(double c_real, double c_imag, CuCondition_Params)
            : c_real(c_real), c_imag(c_imag), CuCondition_Init{
        }

        __host__ __device__ void operator()(System& s) const {
            CuConditionSatisfied(s) {
                double* amplitude = CuSystemAmplitude(s);
                double real_part = amplitude[0] * c_real + amplitude[1] * c_imag;
                double imag_part = -amplitude[0] * c_imag + amplitude[1] * c_real;
                amplitude[0] = real_part;
                amplitude[1] = imag_part;
            }
        }
    };

    struct GlobalPhase_Int_Dag_Functor {
        double c_real;
        double c_imag;

        GlobalPhase_Int_Dag_Functor(double c_real, double c_imag)
            : c_real(c_real), c_imag(c_imag) {
        }

        __host__ __device__ void operator()(System& s) const {
            double* amplitude = CuSystemAmplitude(s);
            double real_part = amplitude[0] * c_real + amplitude[1] * c_imag;
            double imag_part = -amplitude[0] * c_imag + amplitude[1] * c_real;
            amplitude[0] = real_part;
            amplitude[1] = imag_part;
        }
    };

    void GlobalPhase_Int::operator()(CuSparseState& state) const
    {
        profiler _("GlobalPhase_Int cuda");
        state.move_to_gpu();
        if (!HasCondition)
        {
            thrust::for_each(thrust::device,
                state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                GlobalPhase_Int_Functor(c.real(), c.imag())
            );
        }
        else
        {
            CuCondition_Host_Prepare
            thrust::for_each(thrust::device,
                state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                GlobalPhase_Int_Functor_Control(c.real(), c.imag(), CuCondition_Args)
            );
        }
    }

    void GlobalPhase_Int::dag(CuSparseState& state) const
    {
        profiler _("GlobalPhase_Int(dag) cuda");
        state.move_to_gpu();
        if (!HasCondition)
        {
            thrust::for_each(thrust::device,
                state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                GlobalPhase_Int_Dag_Functor(c.real(), c.imag())
            );
        }
        else
        {
            CuCondition_Host_Prepare
            thrust::for_each(thrust::device,
                state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
                GlobalPhase_Int_Functor_Control_Dag(c.real(), c.imag(), CuCondition_Args)
            );
        }
    }
	
} // namespace qram_simulator

#endif // USE_CUDA