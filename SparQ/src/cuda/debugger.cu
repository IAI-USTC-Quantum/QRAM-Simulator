#include "debugger.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"

namespace qram_simulator {

	void ModuleInheritance_Test::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		fmt::print("ModuleInheritance_Test::cu_apply()\n");
	}

	void ModuleInheritance_Test_SelfAdjoint::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		fmt::print("ModuleInheritance_Test_SelfAdjoint::cu_apply()\n");
	}

	struct ParallelOperationTest_Functor {

		double real, imag;
		uint64_t value;

		ParallelOperationTest_Functor(double real_, double imag_, uint64_t value_)
			: real(real_), imag(imag_), value(value_) {}

		__host__ __device__ void operator()(System& s) const {
			double* amplitude = (reinterpret_cast<double*>(&s));
			printf("1 System: %p, Amplitude: %lf + %lfi\n", &s, amplitude[0], amplitude[1]);
			// double* amplitude = CuSystemAmplitude(s);
			amplitude[0] = real;
			amplitude[1] = imag;
			printf("2 System: %p, Amplitude: %lf + %lfi\n", &s, amplitude[0], amplitude[1]);
		}
	};

	void ParallelOperationTest::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		thrust::for_each(thrust::device, state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			ParallelOperationTest_Functor(real, imag, value));
	}


	void CheckNormalization::operator()(CuSparseState& state) const
	{
#ifndef QRAM_Release
		if (state.on_cpu())
		{
			(*this)(state.sparse_state_cpu);
			return;
		}

		double factor = thrust::transform_reduce(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			AbsSqrFunctor(),
			0.0, 
			thrust::plus<double>()
		);

		cudaDeviceSynchronize();

		if ((!ignorable(factor - 1.0, threshold)) ||
			(std::isnan(factor)))
		{
			fmt::print("\n Error! Factor = {}\n", factor);
			throw_bad_result();
		}
#endif
	}

	void CheckNormalization_Renormalize::operator()(CuSparseState& state) const
	{
#ifndef QRAM_Release
		if (state.on_cpu())
		{
			(*this)(state.sparse_state_cpu);
			return;
		}

		double factor = thrust::transform_reduce(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			AbsSqrFunctor(),
			0.0,
			thrust::plus<double>()
		);

		if ((!ignorable(factor - 1.0, threshold)) ||
			(std::isnan(factor)))
		{
			fmt::print("\n Error! Factor = {}\n", factor);
			throw_bad_result("State is not normalized.");
		}

		double sum_prob = 1.0 / std::sqrt(factor);

		thrust::for_each(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			Normalize_Functor(sum_prob)
		);
#endif
	}

	struct CheckNan_Functor {
		__host__ __device__ bool operator()(const System& s) const {
			const double* amplitude = CuSystemAmplitude(s);
			return ::isnan(amplitude[0]) || ::isnan(amplitude[1]);
		}
	};

	void CheckNan::operator()(CuSparseState& state) const
	{
#ifndef QRAM_Release
		if (state.on_cpu())
		{
			(*this)(state.sparse_state_cpu);
			return;
		}

		bool has_nan = thrust::any_of(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			CheckNan_Functor()
		);

		cudaDeviceSynchronize();

		if (has_nan)
		{
			throw_bad_result();
		}
#endif
	}

	void ViewNormalization::operator()(CuSparseState& state) const
	{
		if (state.on_cpu())
		{
			(*this)(state.sparse_state_cpu);
			return;
		}
				
		double factor = thrust::transform_reduce(
			thrust::device,
			state.sparse_state_gpu.begin(),
			state.sparse_state_gpu.end(),
			AbsSqrFunctor(),
			0.0,
			thrust::plus<double>()
		);

		cudaDeviceSynchronize();

		fmt::print("Factor = {}\n", factor);
	}

	std::string StatePrint::to_string(CuSparseState& state) const
	{
		state.copy_to_cpu();
		return to_string(state.sparse_state_cpu);
	}

	void StatePrint::operator()(CuSparseState& state) const
	{
		fmt::print("StatePrint CUDA\n");
		fmt::print("{}", to_string(state));
	}

	struct TestRemovable_Functor {
		uint64_t remove_value;
		int register_id;
		size_t reg_sz;

		TestRemovable_Functor(uint64_t remove_value_, int register_id_) : 
			remove_value(remove_value_), register_id(register_id_),
			reg_sz(System::size_of(register_id)) {}

		__host__ __device__ bool operator()(const System& s) const {
			return CuGetAsUint64(s, register_id, reg_sz) == remove_value;
		}
	};

	void TestRemovable::operator()(CuSparseState& state) const
	{
		if (state.sparse_state_gpu.size() == 0) return;

		System first_element;
		thrust::copy_n(state.sparse_state_gpu.begin(), 1, &first_element);
		uint64_t remove_value = first_element.GetAs(register_id, uint64_t);

		bool all_same = thrust::all_of(
			thrust::device,
			state.sparse_state_gpu.begin() + 1, 
			state.sparse_state_gpu.end(),
			TestRemovable_Functor(remove_value, register_id)
		);

		cudaDeviceSynchronize(); 

		if (!all_same)
		{
			throw_general_runtime_error(fmt::format("Removing failed. RegName: {}\n", System::name_of(register_id)));
		}
	}

	void CheckDuplicateKey::operator()(CuSparseState& state) const
	{
		// 1. Sort on GPU
		thrust::sort(thrust::device, state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			SystemLess_Functor());

		//// 2. Check for duplicates on GPU
		bool has_duplicates = thrust::any_of(
			thrust::device,
			thrust::make_zip_iterator(thrust::make_tuple(state.sparse_state_gpu.begin(), state.sparse_state_gpu.begin() + 1)),
			thrust::make_zip_iterator(thrust::make_tuple(state.sparse_state_gpu.end() - 1, state.sparse_state_gpu.end())),
			SystemEqual_Functor()
		);
		if (has_duplicates)
		{
			throw_general_runtime_error();
		}
	}

} // namespace qram_simulator
