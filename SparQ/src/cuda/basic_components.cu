#include "cuda/basic_components.cuh"
#include "cuda_utils.cuh"

#ifdef USE_CUDA
namespace qram_simulator {

	void BaseOperator::operator()(CuSparseState& state) const
	{
		profiler _("CUDA Default operator()");
		// by default, move to cpu and then apply
		state.move_to_cpu();
		(*this)(state.sparse_state_cpu);
	}

	void BaseOperator::dag(CuSparseState& state) const
	{
		// by default, move to cpu and then apply
		profiler _("CUDA Default dag");
		state.move_to_cpu();
		(*this).dag(state.sparse_state_cpu);
	}

	void SelfAdjointOperator::dag(CuSparseState& state) const
	{
		// by default, move to cpu and then apply
		(*this)(state);
	}

	CuSparseState::CuSparseState()		
	{
		profiler _("CuSparseState constructor");
		sparse_state_cpu.emplace_back(System());
		sparse_state_gpu.reserve(409600);
	}

	CuSparseState::CuSparseState(size_t size)
		: sparse_state_cpu(size)
	{
		sparse_state_gpu.reserve(409600);
	}

	void CuSparseState::move_to_cpu()
	{
		if (on_cpu())
			return;

		profiler _("move_to_cpu");
		// fmt::print("move to cpu.\n");

		CUDA_CHECK(cudaDeviceSynchronize());

		thrust_device_to_std(sparse_state_cpu, sparse_state_gpu);

		// update to cpu mode
		_on_gpu = false;
	}

	void CuSparseState::copy_to_cpu()
	{
		if (on_cpu())
			return;

		profiler _("copy_to_cpu");
		// fmt::print("copy to cpu.\n");

		CUDA_CHECK(cudaDeviceSynchronize());
		thrust_device_to_std(sparse_state_cpu, sparse_state_gpu);

		// avoid update to cpu mode, still have `_on_gpu=true` 
		// _on_gpu = false;
	}

	std::vector<System> CuSparseState::get_cpu_copy() const
	{
		if (on_cpu())
			return sparse_state_cpu;

		// fmt::print("copy to cpu.\n");

		CUDA_CHECK(cudaDeviceSynchronize());
		std::vector<System> cpu_state;
		thrust_device_to_std(cpu_state, sparse_state_gpu);

		return cpu_state;
	}

	void CuSparseState::move_to_gpu()
	{
		if (on_gpu())
			return;

		profiler _("move_to_gpu");
		// fmt::print("move to gpu.\n");

		std_to_thrust_device(sparse_state_gpu, sparse_state_cpu);

		// update to gpu mode
		_on_gpu = true;
	}

	bool CuSparseState::on_gpu() const { return _on_gpu; }
	bool CuSparseState::on_cpu() const { return!_on_gpu; }
	bool CuSparseState::empty() const {
		if (_on_gpu)
			return sparse_state_gpu.empty();
		else
			return sparse_state_cpu.empty();
	}
	size_t CuSparseState::size() const {
		if (_on_gpu)
			return sparse_state_gpu.size();
		else
			return sparse_state_cpu.size();
	}

} // namespce qram_simulator

#endif