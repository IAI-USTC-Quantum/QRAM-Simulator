#include "system_operations.h"
#include "cuda_utils.cuh"
#include "quantum_arithmetic.h"
#include "cuda/basic_components.cuh"

#ifdef USE_CUDA

namespace qram_simulator {

	CuSparseState split_systems(CuSparseState& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	)
	{
		profiler _("split_systems Cuda");
		if (state.on_cpu())
		{
			state.move_to_cpu();
			auto&& systems = split_systems(state.sparse_state_cpu, condition_variable_nonzeros, condition_variable_all_ones, condition_variable_by_bit, condition_variable_by_value);
			return CuSparseState(systems);
		}
		else
		{
			CuConditionLambdaPrepare
			auto pred = [CuConditionLambdaCaptures]__host__ __device__(const System & s) {
				CuConditionSatisfied(s)
				{
					return true;
				}
				else
				{
					return false;
				}
			};
			auto iter = thrust::partition(state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(), pred);
			CuSparseState new_state(iter, state.sparse_state_gpu.end());
			state.sparse_state_gpu.erase(iter, state.sparse_state_gpu.end());
			return new_state;
		}
	}

	void combine_systems(CuSparseState& to, CuSparseState& from)
	{
		profiler _("combine_systems Cuda");
		if (to.on_cpu() || from.on_cpu())
		{
			to.move_to_cpu();
			from.move_to_cpu();
			combine_systems(to.sparse_state_cpu, from.sparse_state_cpu);
		}
		else
		{
			const size_t old_size = to.size();
			to.sparse_state_gpu.resize(old_size + from.size());
			thrust::copy(from.sparse_state_gpu.begin(), from.sparse_state_gpu.end(), to.sparse_state_gpu.begin() + old_size);
		}
	}

	void add_systems(CuSparseState& current, const CuSparseState& new_state, double coef)
	{
		profiler _("add_systems Cuda");
		if (current.on_cpu())
		{
			auto copy = new_state.get_cpu_copy();
			add_systems(current.sparse_state_cpu, copy, coef);
		}
		else
		{
			current.copy_to_cpu();
			auto copy = new_state.get_cpu_copy();
			add_systems(current.sparse_state_cpu, copy, coef);
		}
	}

	struct SplitRegisterFunctor {
		size_t first_pos;
		size_t second_pos;
		size_t second_size;
		uint64_t mask;

		SplitRegisterFunctor(size_t first, size_t second, size_t second_size)
			: first_pos(first), second_pos(second), second_size(second_size), 
			mask(pow2(second_size) - 1)
		{ }

		__host__ __device__
		void operator()(System& s) const {
			auto& value = CuGet(s, first_pos).value;
			CuGet(s, second_pos).value = value & mask;
			value >>= second_size;
		}
	};


	size_t SplitRegister::operator()(CuSparseState& state) const
	{
		profiler _("SplitRegister Cuda");
		state.move_to_gpu();

		size_t first_pos = System::get(first_name);

		size_t original_size = System::size_of(first_pos);
		if (original_size < second_size)
		{
			throw_invalid_input();
		}

		size_t first_size = original_size - second_size;
		StateInfoType info = System::name_register_map[first_pos];

		StateStorageType type = std::get<1>(info);

		// add second register
		size_t second_pos = AddRegister(second_name, type, second_size)(state);

		// modify the first size
		std::get<2>(System::name_register_map[first_pos]) = first_size;

		thrust::for_each(thrust::device,
			state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			SplitRegisterFunctor(first_pos, second_pos, second_size)
		);

		return second_pos;
	}

	struct CombineRegisterFunctor {
		size_t first_pos;
		size_t second_pos;
		size_t second_size;

		CombineRegisterFunctor(size_t first, size_t second, size_t second_size)
			: first_pos(first), second_pos(second), second_size(second_size)
		{}

		__host__ __device__
		void operator()(System& s) const {
			auto& value = CuGet(s, first_pos).value;
			value <<= second_size;
			value += CuGet(s, second_pos).value;
		}
	};

	size_t CombineRegister::operator()(CuSparseState& state) const
	{
		profiler _("CombineRegister Cuda");
		state.move_to_gpu();

		size_t first_pos = System::get(first_name);
		size_t second_pos = System::get(second_name);

		size_t first_size = System::size_of(first_pos);
		size_t second_size = System::size_of(second_pos);

		size_t combine_size = first_size + second_size;
		StateInfoType info = System::name_register_map[first_pos];
		StateStorageType type = std::get<1>(info);

		// modify the first size
		std::get<2>(System::name_register_map[first_pos]) = combine_size;
		
		thrust::for_each(state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			CombineRegisterFunctor(first_pos, second_pos, second_size)
		);

		System::remove_register(second_name);
		return first_pos;
	}

	struct MoveBackRegisterFunctor {
		size_t register_id;
		size_t last_register_id;

		MoveBackRegisterFunctor(size_t id, size_t last_id)
			: register_id(id), last_register_id(last_id)
		{
		}

		__host__ __device__
		void operator()(System& s) const {
			thrust::swap(CuGet(s, register_id), CuGet(s, last_register_id));
		}
	};

	void MoveBackRegister::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		auto& name_register_map = System::name_register_map;

		if (register_id == name_register_map.size() - 1)
			// already at back, do nothing
			return;

		size_t last_id = System::get_last_activated_register();

		std::swap(name_register_map[last_id], name_register_map[register_id]);

		thrust::for_each(thrust::device,
			state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			MoveBackRegisterFunctor(register_id, last_id)
		);
	}

	struct AddRegisterFunctor {
		size_t register_id;

		AddRegisterFunctor(size_t id)
			: register_id(id)
		{
		}

		HOST_DEVICE
		void operator()(System& s) const {
			CuGet(s, register_id).value = 0;
		}
	};

	size_t AddRegister::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t ret = System::add_register(register_name, type, size);

		thrust::for_each(thrust::device,
			state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
			AddRegisterFunctor(ret)
		);

		return ret;
	}

	void RemoveRegister::operator()(CuSparseState& state) const
	{
		//state.move_to_gpu();
		//thrust::for_each(thrust::device,
		//	state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
		//	AddRegisterFunctor(register_id)
		//);

		System::remove_register(register_id);
	}

	void Push::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		auto&& info = System::name_register_map[reg_id];
		size_t garbage_id = AddRegister(garbage_name, get_type(info), get_size(info))(state);

		Swap_General_General(reg_id, garbage_id)(state);
		System::temporal_registers.push_back(garbage_id);
	}


	void Pop::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t garbage_id = System::temporal_registers.back();
		Swap_General_General(reg_id, garbage_id)(state);

		(RemoveRegister(garbage_id))(state);
		System::temporal_registers.pop_back();
	}

	void ClearZero::operator()(CuSparseState& state) const
	{
		profiler _("ClearZero cuda");
		state.move_to_gpu();
		auto iter = thrust::remove_if(thrust::device, state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(), AmplitudeZero_Functor(eps));
		state.sparse_state_gpu.erase(iter, state.sparse_state_gpu.end());
	}

} // namespace qram_simulator

#endif // USE_CUDA