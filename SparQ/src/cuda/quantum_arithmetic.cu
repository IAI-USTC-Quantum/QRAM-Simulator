#include "quantum_arithmetic.h"
#include "cuda_utils.cuh"
#include "cuda/basic_components.cuh"

namespace qram_simulator {

	struct FlipBools_Functor_Control {
		size_t id;

		CuCondition_Functor

			FlipBools_Functor_Control(size_t id_, CuCondition_Params)
			: id(id_), CuCondition_Init
		{}

			__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				CuGet(s, id).value = ~CuGet(s, id).value;
			}
		}
	};

	struct FlipBools_Functor {
		size_t id;

		FlipBools_Functor(size_t id_)
			: id(id_) {
		}

		__host__ __device__ void operator()(System& s) const {
			CuGet(s, id).value = ~CuGet(s, id).value;
		}
	};

	void FlipBools::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				FlipBools_Functor(id)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					FlipBools_Functor_Control(id, CuCondition_Args)
				);
		}
	}

	// CUDA 
	struct Swap_Bool_Bool_Functor_Control {
		size_t lhs;
		size_t rhs;
		size_t digit1;
		size_t digit2;

		CuCondition_Functor

			Swap_Bool_Bool_Functor_Control(size_t lhs_, size_t rhs_, size_t digit1_, size_t digit2_, CuCondition_Params)
			: lhs(lhs_), rhs(rhs_), digit1(digit1_), digit2(digit2_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg1 = CuGet(s, lhs);
				auto& reg2 = CuGet(s, rhs);
				bool v1 = get_digit(reg1.value, digit1);
				bool v2 = get_digit(reg2.value, digit2);
				if (v1 && (!v2))
				{
					reg1.value -= pow2(digit1);
					reg2.value += pow2(digit2);
				}
				if (v2 && (!v1))
				{
					reg1.value += pow2(digit1);
					reg2.value -= pow2(digit2);
				}
			}
		}
	};

	struct Swap_Bool_Bool_Functor {
		size_t lhs;
		size_t rhs;
		size_t digit1;
		size_t digit2;

		Swap_Bool_Bool_Functor(size_t lhs_, size_t rhs_, size_t digit1_, size_t digit2_)
			: lhs(lhs_), rhs(rhs_), digit1(digit1_), digit2(digit2_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg1 = CuGet(s, lhs);
			auto& reg2 = CuGet(s, rhs);
			bool v1 = get_digit(reg1.value, digit1);
			bool v2 = get_digit(reg2.value, digit2);
			if (v1 && (!v2))
			{
				reg1.value -= pow2(digit1);
				reg2.value += pow2(digit2);
			}
			if (v2 && (!v1))
			{
				reg1.value += pow2(digit1);
				reg2.value -= pow2(digit2);
			}
		}
	};

	void Swap_Bool_Bool::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Swap_Bool_Bool_Functor(lhs, rhs, digit1, digit2)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Swap_Bool_Bool_Functor_Control(lhs, rhs, digit1, digit2, CuCondition_Args)
				);
		}
	}

	// CUDA 
	struct ShiftLeft_Functor_Control {
		size_t register_1;
		size_t digit;
		size_t size; // register size
		size_t register_1_size; // 新增变量，存储register_1的大小

		CuCondition_Functor

			ShiftLeft_Functor_Control(size_t register_1_, size_t digit_, size_t size_, size_t register_1_size_, CuCondition_Params)
			: register_1(register_1_), digit(digit_), size(size_), register_1_size(register_1_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				uint64_t value = CuGetAsUint64(s, register_1, register_1_size);
				size_t high = value >> (size - digit);
				size_t low = value - (high << (size - digit));
				CuGet(s, register_1).value = (low << digit) + high;
			}
		}
	};

	struct ShiftLeft_Functor {
		size_t register_1;
		size_t digit;
		size_t size; // register size
		size_t register_1_size; // 新增变量，存储register_1的大小

		ShiftLeft_Functor(size_t register_1_, size_t digit_, size_t size_, size_t register_1_size_)
			: register_1(register_1_), digit(digit_), size(size_), register_1_size(register_1_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			uint64_t value = CuGetAsUint64(s, register_1, register_1_size);
			size_t high = value >> (size - digit);
			size_t low = value - (high << (size - digit));
			CuGet(s, register_1).value = (low << digit) + high;
		}
	};

	void ShiftLeft::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t size = System::size_of(register_1);
		size_t register_1_size = System::size_of(register_1); // 获取register_1的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				ShiftLeft_Functor(register_1, digit, size, register_1_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					ShiftLeft_Functor_Control(register_1, digit, size, register_1_size, CuCondition_Args)
				);
		}
	}

	// CUDA
	struct ShiftRight_Functor_Control {
		size_t register_1;
		size_t digit;
		size_t size;
		size_t register_1_size; // 新增变量

		CuCondition_Functor

			ShiftRight_Functor_Control(size_t register_1_, size_t digit_, size_t size_, size_t register_1_size_, CuCondition_Params)
			: register_1(register_1_), digit(digit_), size(size_), register_1_size(register_1_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				size_t value = CuGetAsUint64(s, register_1, register_1_size);
				size_t high = value >> digit;
				size_t low = value - (high << (digit));
				CuGet(s, register_1).value = (low << (size - digit)) + high;
			}
		}
	};

	struct ShiftRight_Functor {
		size_t register_1;
		size_t digit;
		size_t size;
		size_t register_1_size; // 新增变量

		ShiftRight_Functor(size_t register_1_, size_t digit_, size_t size_, size_t register_1_size_)
			: register_1(register_1_), digit(digit_), size(size_), register_1_size(register_1_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			size_t value = CuGetAsUint64(s, register_1, register_1_size);
			size_t high = value >> digit;
			size_t low = value - (high << (digit));
			CuGet(s, register_1).value = (low << (size - digit)) + high;
		}
	};

	void ShiftRight::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t size = System::size_of(register_1);
		size_t register_1_size = System::size_of(register_1); // 获取register_1的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				ShiftRight_Functor(register_1, digit, size, register_1_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					ShiftRight_Functor_Control(register_1, digit, size, register_1_size, CuCondition_Args)
				);
		}
	}

	struct Mult_UInt_ConstUInt_Functor_Control {
		size_t lhs;
		size_t res;
		size_t mult_int;
		size_t lhs_size; // 新增变量

		CuCondition_Functor

			Mult_UInt_ConstUInt_Functor_Control(size_t lhs_, size_t res_, size_t mult_int_, size_t lhs_size_, CuCondition_Params)
			: lhs(lhs_), res(res_), mult_int(mult_int_), lhs_size(lhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_out = CuGet(s, res);
				reg_out.value ^= (CuGetAsUint64(s, lhs, lhs_size) * mult_int);
			}
		}
	};

	struct Mult_UInt_ConstUInt_Functor {
		size_t lhs;
		size_t res;
		size_t mult_int;
		size_t lhs_size; // 新增变量

		Mult_UInt_ConstUInt_Functor(size_t lhs_, size_t res_, size_t mult_int_, size_t lhs_size_)
			: lhs(lhs_), res(res_), mult_int(mult_int_), lhs_size(lhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_out = CuGet(s, res);
			reg_out.value ^= (CuGetAsUint64(s, lhs, lhs_size) * mult_int);
		}
	};

	void Mult_UInt_ConstUInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(lhs); // 获取lhs的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Mult_UInt_ConstUInt_Functor(lhs, res, mult_int, lhs_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Mult_UInt_ConstUInt_Functor_Control(lhs, res, mult_int, lhs_size, CuCondition_Args)
				);
		}
	}

	struct Add_Mult_UInt_ConstUInt_Functor_Control {
		size_t lhs;
		size_t res;
		size_t mult_int;
		size_t lhs_size; // 新增变量

		CuCondition_Functor

			Add_Mult_UInt_ConstUInt_Functor_Control(size_t lhs_, size_t res_, size_t mult_int_, size_t lhs_size_, CuCondition_Params)
			: lhs(lhs_), res(res_), mult_int(mult_int_), lhs_size(lhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_out = CuGet(s, res);
				reg_out.value += (mult_int * CuGetAsUint64(s, lhs, lhs_size));
			}
		}
	};

	struct Add_Mult_UInt_ConstUInt_Functor {
		size_t lhs;
		size_t res;
		size_t mult_int;
		size_t lhs_size; // 新增变量

		Add_Mult_UInt_ConstUInt_Functor(size_t lhs_, size_t res_, size_t mult_int_, size_t lhs_size_)
			: lhs(lhs_), res(res_), mult_int(mult_int_), lhs_size(lhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_out = CuGet(s, res);
			reg_out.value += (mult_int * CuGetAsUint64(s, lhs, lhs_size));
		}
	};

	void Add_Mult_UInt_ConstUInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(lhs); // 获取lhs的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_Mult_UInt_ConstUInt_Functor(lhs, res, mult_int, lhs_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_Mult_UInt_ConstUInt_Functor_Control(lhs, res, mult_int, lhs_size, CuCondition_Args)
				);
		}
	}


	struct Add_Mult_UInt_ConstUInt_Functor_Control_Dag {
		size_t lhs;
		size_t res;
		size_t mult_int;
		size_t lhs_size; // 新增变量
		size_t dim;

		CuCondition_Functor

			Add_Mult_UInt_ConstUInt_Functor_Control_Dag(size_t lhs_, size_t res_, size_t mult_int_, size_t lhs_size_, size_t dim_, CuCondition_Params)
			: lhs(lhs_), res(res_), mult_int(mult_int_), lhs_size(lhs_size_), dim(dim_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_out = CuGet(s, res);
				reg_out.value += (pow2(dim) - mult_int * CuGetAsUint64(s, lhs, lhs_size));
				reg_out.value %= pow2(dim);
			}
		}
	};

	struct Add_Mult_UInt_ConstUInt_Functor_Dag {
		size_t lhs;
		size_t res;
		size_t mult_int;
		size_t lhs_size; // 新增变量
		size_t dim;

		Add_Mult_UInt_ConstUInt_Functor_Dag(size_t lhs_, size_t res_, size_t mult_int_, size_t lhs_size_, size_t dim_)
			: lhs(lhs_), res(res_), mult_int(mult_int_), lhs_size(lhs_size_), dim(dim_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_out = CuGet(s, res);
			reg_out.value += (pow2(dim) - mult_int * CuGetAsUint64(s, lhs, lhs_size));
			reg_out.value %= pow2(dim);
		}
	};

	void Add_Mult_UInt_ConstUInt::dag(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(lhs); // 获取lhs的大小
		size_t dim = System::size_of(res);

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_Mult_UInt_ConstUInt_Functor_Dag(lhs, res, mult_int, lhs_size, dim)
			);
		}
		else
		{
			CuCondition_Host_Prepare

			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_Mult_UInt_ConstUInt_Functor_Control_Dag(lhs, res, mult_int, lhs_size, dim, CuCondition_Args)
			);
		}
	}

	struct Add_UInt_UInt_Functor_Control {
		size_t lhs;
		size_t rhs;
		size_t res;
		size_t lhs_size; // 新增变量
		size_t rhs_size; // 新增变量

		CuCondition_Functor

			Add_UInt_UInt_Functor_Control(size_t lhs_, size_t rhs_, size_t res_, size_t lhs_size_, size_t rhs_size_, CuCondition_Params)
			: lhs(lhs_), rhs(rhs_), res(res_), lhs_size(lhs_size_), rhs_size(rhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_out = CuGet(s, res);
				reg_out.value ^= (CuGetAsUint64(s, lhs, lhs_size) + CuGetAsUint64(s, rhs, rhs_size));
			}
		}
	};

	struct Add_UInt_UInt_Functor {
		size_t lhs;
		size_t rhs;
		size_t res;
		size_t lhs_size;
		size_t rhs_size;

		Add_UInt_UInt_Functor(size_t lhs_, size_t rhs_, size_t res_, size_t lhs_size_, size_t rhs_size_)
			: lhs(lhs_), rhs(rhs_), res(res_), lhs_size(lhs_size_), rhs_size(rhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_out = CuGet(s, res);
			reg_out.value = (CuGetAsUint64(s, lhs, lhs_size) + CuGetAsUint64(s, rhs, rhs_size));
		}
	};

	void Add_UInt_UInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(lhs);
		size_t rhs_size = System::size_of(rhs); 

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_UInt_UInt_Functor(lhs, rhs, res, lhs_size, rhs_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_UInt_UInt_Functor_Control(lhs, rhs, res, lhs_size, rhs_size, CuCondition_Args)
				);
		}
	}

	struct Add_UInt_UInt_InPlace_Functor_Control {
		size_t lhs;
		size_t rhs;
		size_t lhs_size; // 新增变量

		CuCondition_Functor

			Add_UInt_UInt_InPlace_Functor_Control(size_t lhs_, size_t rhs_, size_t lhs_size_, CuCondition_Params)
			: lhs(lhs_), rhs(rhs_), lhs_size(lhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_rhs = CuGet(s, rhs);
				reg_rhs.value += CuGetAsUint64(s, lhs, lhs_size);
			}
		}
	};

	struct Add_UInt_UInt_InPlace_Functor {
		size_t lhs;
		size_t rhs;
		size_t lhs_size; // 新增变量

		Add_UInt_UInt_InPlace_Functor(size_t lhs_, size_t rhs_, size_t lhs_size_)
			: lhs(lhs_), rhs(rhs_), lhs_size(lhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_rhs = CuGet(s, rhs);
			reg_rhs.value += CuGetAsUint64(s, lhs, lhs_size);
		}
	};

	void Add_UInt_UInt_InPlace::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(lhs); // 获取lhs的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_UInt_UInt_InPlace_Functor(lhs, rhs, lhs_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_UInt_UInt_InPlace_Functor_Control(lhs, rhs, lhs_size, CuCondition_Args)
				);
		}
	}

	// CUDA for dag()
	struct Add_UInt_UInt_InPlace_Functor_Control_Dag {
		size_t lhs;
		size_t rhs;
		size_t dim;
		size_t lhs_size; // 新增变量

		CuCondition_Functor

			Add_UInt_UInt_InPlace_Functor_Control_Dag(size_t lhs_, size_t rhs_, size_t dim_, size_t lhs_size_, CuCondition_Params)
			: lhs(lhs_), rhs(rhs_), dim(dim_), lhs_size(lhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_rhs = CuGet(s, rhs);
				reg_rhs.value += (pow2(dim) - CuGetAsUint64(s, lhs, lhs_size));
				reg_rhs.value = reg_rhs.value % pow2(dim);
			}
		}
	};

	struct Add_UInt_UInt_InPlace_Functor_Dag {
		size_t lhs;
		size_t rhs;
		size_t dim;
		size_t lhs_size; // 新增变量

		Add_UInt_UInt_InPlace_Functor_Dag(size_t lhs_, size_t rhs_, size_t dim_, size_t lhs_size_)
			: lhs(lhs_), rhs(rhs_), dim(dim_), lhs_size(lhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_rhs = CuGet(s, rhs);
			reg_rhs.value += (pow2(dim) - CuGetAsUint64(s, lhs, lhs_size));
			reg_rhs.value = reg_rhs.value % pow2(dim);
		}
	};

	void Add_UInt_UInt_InPlace::dag(CuSparseState& state) const
	{
		state.move_to_gpu();
		auto dim = System::size_of(rhs);
		size_t lhs_size = System::size_of(lhs); // 获取lhs的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_UInt_UInt_InPlace_Functor_Dag(lhs, rhs, dim, lhs_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_UInt_UInt_InPlace_Functor_Control_Dag(lhs, rhs, dim, lhs_size, CuCondition_Args)
				);
		}
	}

	struct Add_UInt_ConstUInt_Functor_Control {
		size_t lhs;
		size_t res;
		size_t add_int;
		size_t lhs_size; // 新增变量

		CuCondition_Functor

			Add_UInt_ConstUInt_Functor_Control(size_t lhs_, size_t res_, size_t add_int_, size_t lhs_size_, CuCondition_Params)
			: lhs(lhs_), res(res_), add_int(add_int_), lhs_size(lhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_out = CuGet(s, res);
				reg_out.value ^= (CuGetAsUint64(s, lhs, lhs_size) + add_int);
			}
		}
	};

	struct Add_UInt_ConstUInt_Functor {
		size_t lhs;
		size_t res;
		size_t add_int;
		size_t lhs_size; // 新增变量

		Add_UInt_ConstUInt_Functor(size_t lhs_, size_t res_, size_t add_int_, size_t lhs_size_)
			: lhs(lhs_), res(res_), add_int(add_int_), lhs_size(lhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_out = CuGet(s, res);
			reg_out.value ^= (CuGetAsUint64(s, lhs, lhs_size) + add_int);
		}
	};

	void Add_UInt_ConstUInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(lhs); // 获取lhs的大小

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Add_UInt_ConstUInt_Functor(lhs, res, add_int, lhs_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_UInt_ConstUInt_Functor_Control(lhs, res, add_int, lhs_size, CuCondition_Args)
				);
		}
	}

	struct Add_ConstUInt_Functor_Control {
		size_t reg_in;
		size_t add_int;
		size_t dim;
		CuCondition_Functor

			Add_ConstUInt_Functor_Control(size_t reg_in_, size_t add_int_, size_t dim_, CuCondition_Params)
			: reg_in(reg_in_), add_int(add_int_), dim(dim_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_ = CuGet(s, reg_in);
				reg_.value += add_int;
				reg_.value = reg_.value % pow2(dim);
			}
		}
	};

	struct Add_ConstUInt_Functor {
		size_t reg_in;
		size_t add_int;
		size_t dim;

		Add_ConstUInt_Functor(size_t reg_in_, size_t add_int_, size_t dim_)
			: reg_in(reg_in_), add_int(add_int_), dim(dim_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_ = CuGet(s, reg_in);
			reg_.value += add_int;
			reg_.value = reg_.value % pow2(dim);
		}
	};

	void Add_ConstUInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		auto dim = System::size_of(reg_in);
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_ConstUInt_Functor(reg_in, add_int, dim)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_ConstUInt_Functor_Control(reg_in, add_int, dim, CuCondition_Args)
				);
			}
	}

	// CUDA for dag()
	struct Add_ConstUInt_Functor_Control_Dag {
		size_t reg_in;
		size_t add_int;
		size_t dim;
		CuCondition_Functor

			Add_ConstUInt_Functor_Control_Dag(size_t reg_in_, size_t add_int_, size_t dim_, CuCondition_Params)
			: reg_in(reg_in_), add_int(add_int_), dim(dim_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg_ = CuGet(s, reg_in);
				reg_.value += (pow2(dim) - add_int);
				reg_.value = reg_.value % pow2(dim);
			}
		}
	};

	struct Add_ConstUInt_Functor_Dag {
		size_t reg_in;
		size_t add_int;
		size_t dim;

		Add_ConstUInt_Functor_Dag(size_t reg_in_, size_t add_int_, size_t dim_)
			: reg_in(reg_in_), add_int(add_int_), dim(dim_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg_ = CuGet(s, reg_in);
			reg_.value += (pow2(dim) - add_int);
			reg_.value = reg_.value % pow2(dim);
		}
	};

	void Add_ConstUInt::dag(CuSparseState& state) const
	{
		state.move_to_gpu();
		auto dim = System::size_of(reg_in);
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_ConstUInt_Functor_Dag(reg_in, add_int, dim)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Add_ConstUInt_Functor_Control_Dag(reg_in, add_int, dim, CuCondition_Args)
				);
			}
	}

	struct Div_Sqrt_Arccos_Int_Int_Functor_Control {
		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		size_t out_size;
		size_t lhs_size; // 新增变量
		size_t rhs_size; // 新增变量

		CuCondition_Functor

		Div_Sqrt_Arccos_Int_Int_Functor_Control(size_t register_lhs_, size_t register_rhs_, size_t register_out_,
			size_t lhs_size_, size_t rhs_size_, size_t out_size_, CuCondition_Params)
			: register_lhs(register_lhs_), register_rhs(register_rhs_), register_out(register_out_),
			lhs_size(lhs_size_), rhs_size(rhs_size_), out_size(out_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				uint64_t regl_v = CuGetAsUint64(s, register_lhs, lhs_size);
				uint64_t regr_v = CuGetAsUint64(s, register_rhs, rhs_size);
				double out = acos(sqrt(1.0 * regl_v / regr_v)) / pi / 2;
				auto& regout = CuGet(s, register_out);
				regout.value ^= get_rational(out, out_size);
			}
		}
	};

	struct Div_Sqrt_Arccos_Int_Int_Functor {
		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		size_t out_size;
		size_t lhs_size; // 新增变量
		size_t rhs_size; // 新增变量

		Div_Sqrt_Arccos_Int_Int_Functor(size_t register_lhs_, size_t register_rhs_, size_t register_out_, 
			size_t lhs_size_, size_t rhs_size_, size_t out_size_)
			: register_lhs(register_lhs_), register_rhs(register_rhs_), register_out(register_out_), 
			lhs_size(lhs_size_), rhs_size(rhs_size_), out_size(out_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			uint64_t regl_v = CuGetAsUint64(s, register_lhs, lhs_size);
			uint64_t regr_v = CuGetAsUint64(s, register_rhs, rhs_size);
			double out = acos(sqrt(1.0 * regl_v / regr_v)) / pi / 2;
			auto& regout = CuGet(s, register_out);
			regout.value ^= get_rational(out, out_size);
		}
	};

	void Div_Sqrt_Arccos_Int_Int::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(register_lhs);
		size_t rhs_size = System::size_of(register_rhs);
		size_t out_size = System::size_of(register_out);

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Div_Sqrt_Arccos_Int_Int_Functor(register_lhs, register_rhs, register_out, 
					lhs_size, rhs_size, out_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Div_Sqrt_Arccos_Int_Int_Functor_Control(register_lhs, register_rhs, register_out, 
					lhs_size, rhs_size, out_size, CuCondition_Args)
			);
		}
	}

	struct Sqrt_Div_Arccos_Int_Int_Functor_Control {
		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		size_t out_size;
		size_t lhs_size; // 新增变量
		size_t rhs_size; // 新增变量

		CuCondition_Functor

			Sqrt_Div_Arccos_Int_Int_Functor_Control(size_t register_lhs_, size_t register_rhs_, size_t register_out_,
				size_t lhs_size_, size_t rhs_size_, size_t out_size_, CuCondition_Params)
			: register_lhs(register_lhs_), register_rhs(register_rhs_), register_out(register_out_),
			lhs_size(lhs_size_), rhs_size(rhs_size_), out_size(out_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				int64_t lvalue = CuGetAsInt64(s, register_lhs, lhs_size);
				uint64_t rvalue = CuGetAsUint64(s, register_rhs, rhs_size);
				double out = acos(lvalue / sqrt(rvalue)) / pi / 2;
				auto& regout = CuGet(s, register_out);
				regout.value ^= get_rational(out, out_size);
			}
		}
	};

	struct Sqrt_Div_Arccos_Int_Int_Functor {
		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		size_t out_size;
		size_t lhs_size; // 新增变量
		size_t rhs_size; // 新增变量

		Sqrt_Div_Arccos_Int_Int_Functor(size_t register_lhs_, size_t register_rhs_, size_t register_out_,
			size_t lhs_size_, size_t rhs_size_, size_t out_size_)
			: register_lhs(register_lhs_), register_rhs(register_rhs_), register_out(register_out_),
			lhs_size(lhs_size_), rhs_size(rhs_size_), out_size(out_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			int64_t lvalue = CuGetAsInt64(s, register_lhs, lhs_size);
			uint64_t rvalue = CuGetAsUint64(s, register_rhs, rhs_size);
			double out = acos(lvalue / sqrt(rvalue)) / pi / 2;
			auto& regout = CuGet(s, register_out);
			regout.value ^= get_rational(out, out_size);
		}
	};

	void Sqrt_Div_Arccos_Int_Int::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t lhs_size = System::size_of(register_lhs);
		size_t rhs_size = System::size_of(register_rhs);
		size_t out_size = System::size_of(register_out);

		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Sqrt_Div_Arccos_Int_Int_Functor(register_lhs, register_rhs, register_out,
					lhs_size, rhs_size, out_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare

				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Sqrt_Div_Arccos_Int_Int_Functor_Control(register_lhs, register_rhs, register_out,
						lhs_size, rhs_size, out_size, CuCondition_Args)
				);
		}
	}



	struct GetRotateAngle_Int_Int_Functor_Control {
		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		size_t left_size;
		size_t right_size;
		size_t out_size;
		CuCondition_Functor

		GetRotateAngle_Int_Int_Functor_Control(size_t left_id_, size_t right_id_, size_t out_id_,
			size_t left_size_, size_t right_size_, size_t out_size_,
			CuCondition_Params)
		: register_lhs(left_id_), register_rhs(right_id_), register_out(out_id_),
			left_size(left_size_), right_size(right_size_), out_size(out_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& regl = CuGet(s, register_lhs);
				auto& regr = CuGet(s, register_rhs);
				auto l_complement = CuGetAsUint64(s, register_lhs, left_size);
				auto r_complement = CuGetAsUint64(s, register_rhs, right_size);
				auto l = get_complement(l_complement, left_size);
				auto r = get_complement(r_complement, right_size);
				double out;
				if (l == 0 && r >= 0) { out = 0.25; }
				else if (l == 0 && r < 0) { out = 0.75; }
				else {
					out = atan2_cuda(r, l) / pi / 2;
					if (out < 0) out += 1;
				}
				auto& regout = CuGet(s, register_out);
				regout.value ^= get_rational(out, out_size);
			}
		}
	};

	struct GetRotateAngle_Int_Int_Functor {
		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		size_t left_size;
		size_t right_size;
		size_t out_size;

		GetRotateAngle_Int_Int_Functor(size_t left_id_, size_t right_id_, size_t out_id_,
			size_t left_size_, size_t right_size_, size_t out_size_)
			: register_lhs(left_id_), register_rhs(right_id_), register_out(out_id_),
			left_size(left_size_), right_size(right_size_), out_size(out_size_){
		}

		__host__ __device__ void operator()(System& s) const {
			auto& regl = CuGet(s, register_lhs);
			auto& regr = CuGet(s, register_rhs);
			auto l_complement = CuGetAsUint64(s, register_lhs, left_size);
			auto r_complement = CuGetAsUint64(s, register_rhs, right_size);
			auto l = get_complement(l_complement, left_size);
			auto r = get_complement(r_complement, right_size);
			double out;
			if (l == 0 && r >= 0) { out = 0.25; }
			else if (l == 0 && r < 0) { out = 0.75; }
			else {
				out = atan2_cuda(r, l) / pi / 2;
				if (out < 0) out += 1;
			}
			auto& regout = CuGet(s, register_out);
			regout.value ^= get_rational(out, out_size);
		}
	};

	void GetRotateAngle_Int_Int::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t left_size = System::size_of(register_lhs);
		size_t right_size = System::size_of(register_rhs);
		size_t out_size = System::size_of(register_out);
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				GetRotateAngle_Int_Int_Functor(register_lhs, register_rhs, register_out, 
					left_size, right_size, out_size)
			);
		}
		else
		{
			CuCondition_Host_Prepare
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				GetRotateAngle_Int_Int_Functor_Control(register_lhs, register_rhs, register_out, 
					left_size, right_size, out_size, CuCondition_Args)
			);
		}
	}

	struct AddAssign_AnyInt_AnyInt_Functor_Control {
		size_t lhs_id;
		size_t rhs_id;
		size_t lhs_size;
		size_t rhs_size;

		CuCondition_Functor

			AddAssign_AnyInt_AnyInt_Functor_Control(size_t lhs_id_, size_t rhs_id_, size_t lhs_size_, size_t rhs_size_, CuCondition_Params)
			: lhs_id(lhs_id_), rhs_id(rhs_id_), lhs_size(lhs_size_), rhs_size(rhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				size_t rhs = CuGetAsUint64(s, rhs_id, rhs_size);
				size_t& lhs = CuGet(s, lhs_id).value;
				lhs += rhs;
				lhs %= pow2(lhs_size);
			}
		}
	};

	struct AddAssign_AnyInt_AnyInt_Functor {
		size_t lhs_id;
		size_t rhs_id;
		size_t lhs_size;
		size_t rhs_size;

		AddAssign_AnyInt_AnyInt_Functor(size_t lhs_id_, size_t rhs_id_, size_t lhs_size_, size_t rhs_size_)
			: lhs_id(lhs_id_), rhs_id(rhs_id_), lhs_size(lhs_size_), rhs_size(rhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			size_t rhs = CuGetAsUint64(s, rhs_id, rhs_size);
			size_t& lhs = CuGet(s, lhs_id).value;
			lhs += rhs;
			lhs %= pow2(lhs_size);
		}
	};

	struct AddAssign_AnyInt_AnyInt_Functor_Control_Dag {
		size_t lhs_id;
		size_t rhs_id;
		size_t lhs_size;
		size_t rhs_size;

		CuCondition_Functor

			AddAssign_AnyInt_AnyInt_Functor_Control_Dag(size_t lhs_id_, size_t rhs_id_, size_t lhs_size_, size_t rhs_size_, CuCondition_Params)
			: lhs_id(lhs_id_), rhs_id(rhs_id_), lhs_size(lhs_size_), rhs_size(rhs_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				size_t rhs = CuGetAsUint64(s, rhs_id, rhs_size);
				size_t& lhs = CuGet(s, lhs_id).value;
				lhs -= rhs;
				lhs %= pow2(lhs_size);
			}
		}
	};

	struct AddAssign_AnyInt_AnyInt_Functor_Dag {
		size_t lhs_id;
		size_t rhs_id;
		size_t lhs_size;
		size_t rhs_size;

		AddAssign_AnyInt_AnyInt_Functor_Dag(size_t lhs_id_, size_t rhs_id_, size_t lhs_size_, size_t rhs_size_)
			: lhs_id(lhs_id_), rhs_id(rhs_id_), lhs_size(lhs_size_), rhs_size(rhs_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			size_t rhs = CuGetAsUint64(s, rhs_id, rhs_size);
			size_t& lhs = CuGet(s, lhs_id).value;
			lhs -= rhs;
			lhs %= pow2(lhs_size);
		}
	};

	void AddAssign_AnyInt_AnyInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();

			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					AddAssign_AnyInt_AnyInt_Functor(lhs_id, rhs_id, lhs_size, rhs_size)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					AddAssign_AnyInt_AnyInt_Functor_Control(lhs_id, rhs_id, lhs_size, rhs_size, CuCondition_Args)
				);
			}
	}

	void AddAssign_AnyInt_AnyInt::dag(CuSparseState& state) const
	{
		state.move_to_gpu();
		CuCondition_Host_Prepare

			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					AddAssign_AnyInt_AnyInt_Functor_Dag(lhs_id, rhs_id, lhs_size, rhs_size)
				);
			}
			else
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					AddAssign_AnyInt_AnyInt_Functor_Control_Dag(lhs_id, rhs_id, lhs_size, rhs_size, CuCondition_Args)
				);
			}
	}


	struct Assign_Functor_Control {
		size_t register_1;
		size_t register_2;
		CuCondition_Functor

			Assign_Functor_Control(size_t register_1_, size_t register_2_, CuCondition_Params)
			: register_1(register_1_), register_2(register_2_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				auto& reg1 = CuGet(s, register_1);
				auto& reg2 = CuGet(s, register_2);
				reg2.value ^= reg1.value;
			}
		}
	};

	struct Assign_Functor {
		size_t register_1;
		size_t register_2;

		Assign_Functor(size_t register_1_, size_t register_2_)
			: register_1(register_1_), register_2(register_2_) {
		}

		__host__ __device__ void operator()(System& s) const {
			auto& reg1 = CuGet(s, register_1);
			auto& reg2 = CuGet(s, register_2);
			reg2.value ^= reg1.value;
		}
	};

	void Assign::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Assign_Functor(register_1, register_2)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Assign_Functor_Control(register_1, register_2, CuCondition_Args)
				);
			}
	}
	struct Compare_UInt_UInt_Functor_Control {
		size_t left_id;
		size_t right_id;
		size_t compare_less_id;
		size_t compare_equal_id;
		size_t left_size;
		size_t right_size;
		CuCondition_Functor

			Compare_UInt_UInt_Functor_Control(size_t left_id_, size_t right_id_, size_t compare_less_id_, size_t compare_equal_id_,
				size_t left_size_, size_t right_size_, CuCondition_Params)
			: left_id(left_id_), right_id(right_id_), compare_less_id(compare_less_id_), compare_equal_id(compare_equal_id_),
			left_size(left_size_), right_size(right_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				size_t l = CuGetAsUint64(s, left_id, left_size);
				size_t r = CuGetAsUint64(s, right_id, right_size);

				if (l == r)
				{
					CuGet(s, compare_equal_id).value ^= 1;
				}
				else
				{
					CuGet(s, compare_less_id).value ^= (l < r);
				}
			}
		}
	};

	struct Compare_UInt_UInt_Functor {
		size_t left_id;
		size_t right_id;
		size_t compare_less_id;
		size_t compare_equal_id;
		size_t left_size;
		size_t right_size;

		Compare_UInt_UInt_Functor(size_t left_id_, size_t right_id_, size_t compare_less_id_, size_t compare_equal_id_,
			size_t left_size_, size_t right_size_)
			: left_id(left_id_), right_id(right_id_), compare_less_id(compare_less_id_), compare_equal_id(compare_equal_id_),
			left_size(left_size_), right_size(right_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			size_t l = CuGetAsUint64(s, left_id, left_size);
			size_t r = CuGetAsUint64(s, right_id, right_size);

			if (l == r)
			{
				CuGet(s, compare_equal_id).value ^= 1;
			}
			else
			{
				CuGet(s, compare_less_id).value ^= (l < r);
			}
		}
	};

	void Compare_UInt_UInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t left_size = System::size_of(left_id);
		size_t right_size = System::size_of(right_id);
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Compare_UInt_UInt_Functor(left_id, right_id, compare_less_id, compare_equal_id, left_size, right_size)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Compare_UInt_UInt_Functor_Control(left_id, right_id, compare_less_id, compare_equal_id, left_size, right_size, CuCondition_Args)
				);
			}
	}

	struct Less_UInt_UInt_Functor_Control {
		size_t left_id;
		size_t right_id;
		size_t compare_less_id;
		size_t left_size;
		size_t right_size;
		CuCondition_Functor

			Less_UInt_UInt_Functor_Control(size_t left_id_, size_t right_id_, size_t compare_less_id_,
				size_t left_size_, size_t right_size_, CuCondition_Params)
			: left_id(left_id_), right_id(right_id_), compare_less_id(compare_less_id_),
			left_size(left_size_), right_size(right_size_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				size_t l = CuGetAsUint64(s, left_id, left_size);
				size_t r = CuGetAsUint64(s, right_id, right_size);
				CuGet(s, compare_less_id).value ^= (l < r);
			}
		}
	};

	struct Less_UInt_UInt_Functor {
		size_t left_id;
		size_t right_id;
		size_t compare_less_id;
		size_t left_size;
		size_t right_size;

		Less_UInt_UInt_Functor(size_t left_id_, size_t right_id_, size_t compare_less_id_,
			size_t left_size_, size_t right_size_)
			: left_id(left_id_), right_id(right_id_), compare_less_id(compare_less_id_),
			left_size(left_size_), right_size(right_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			size_t l = CuGetAsUint64(s, left_id, left_size);
			size_t r = CuGetAsUint64(s, right_id, right_size);
			CuGet(s, compare_less_id).value ^= (l < r);
		}
	};

	void Less_UInt_UInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t left_size = System::size_of(left_id);
		size_t right_size = System::size_of(right_id);
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Less_UInt_UInt_Functor(left_id, right_id, compare_less_id, left_size, right_size)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					Less_UInt_UInt_Functor_Control(left_id, right_id, compare_less_id, left_size, right_size, CuCondition_Args)
				);
			}
	}


	struct Swap_General_General_Functor_Control {
		size_t id1;
		size_t id2;
		CuCondition_Functor

			Swap_General_General_Functor_Control(size_t id1_, size_t id2_, CuCondition_Params)
			: id1(id1_), id2(id2_), CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s)
			{
				CuGet(s, id1).value ^= CuGet(s, id2).value;
				CuGet(s, id2).value ^= CuGet(s, id1).value;
				CuGet(s, id1).value ^= CuGet(s, id2).value;
			}
		}
	};

	struct Swap_General_General_Functor {
		size_t id1;
		size_t id2;

		Swap_General_General_Functor(size_t id1_, size_t id2_)
			: id1(id1_), id2(id2_) {
		}

		__host__ __device__ void operator()(System& s) const {
			CuGet(s, id1).value ^= CuGet(s, id2).value;
			CuGet(s, id2).value ^= CuGet(s, id1).value;
			CuGet(s, id1).value ^= CuGet(s, id2).value;
		}
	};

	void Swap_General_General::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		if (!HasCondition)
		{
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Swap_General_General_Functor(id1, id2)
			);
		}
		else
		{
			CuCondition_Host_Prepare
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Swap_General_General_Functor_Control(id1, id2, CuCondition_Args)
			);
		}
	}
	struct GetMid_UInt_UInt_Functor_Control {
		size_t left_id;
		size_t right_id;
		size_t mid_id;
		size_t left_size;
		size_t right_size;
		size_t mid_size;
		CuCondition_Functor

			GetMid_UInt_UInt_Functor_Control(size_t left_id_, size_t right_id_, size_t mid_id_,
				size_t left_size_, size_t right_size_, size_t mid_size_,
				CuCondition_Params)
			: left_id(left_id_), right_id(right_id_), mid_id(mid_id_),
			left_size(left_size_), right_size(right_size_), mid_size(mid_size_),
			CuCondition_Init{
		}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				size_t l = CuGetAsUint64(s, left_id, left_size);
				size_t r = CuGetAsUint64(s, right_id, right_size);
				CuGet(s, mid_id).value ^= (l + r) / 2;
			}
		}
	};

	struct GetMid_UInt_UInt_Functor {
		size_t left_id;
		size_t right_id;
		size_t mid_id;
		size_t left_size;
		size_t right_size;
		size_t mid_size;

		GetMid_UInt_UInt_Functor(size_t left_id_, size_t right_id_, size_t mid_id_,
			size_t left_size_, size_t right_size_, size_t mid_size_)
			: left_id(left_id_), right_id(right_id_), mid_id(mid_id_),
			left_size(left_size_), right_size(right_size_), mid_size(mid_size_) {
		}

		__host__ __device__ void operator()(System& s) const {
			size_t l = CuGetAsUint64(s, left_id, left_size);
			size_t r = CuGetAsUint64(s, right_id, right_size);
			CuGet(s, mid_id).value ^= (l + r) / 2;
		}
	};

	void GetMid_UInt_UInt::operator()(CuSparseState& state) const
	{
		state.move_to_gpu();
		size_t left_size = System::size_of(left_id);
		size_t right_size = System::size_of(right_id);
		size_t mid_size = System::size_of(mid_id);
			if (!HasCondition)
			{
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					GetMid_UInt_UInt_Functor(left_id, right_id, mid_id, left_size, right_size, mid_size)
				);
			}
			else
			{
				CuCondition_Host_Prepare
				thrust::for_each(thrust::device,
					state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
					GetMid_UInt_UInt_Functor_Control(left_id, right_id, mid_id, left_size, right_size, mid_size, CuCondition_Args)
				);
			}
	}

	// Mod_Mult_UInt_ConstUInt CUDA implementations
	struct Mod_Mult_UInt_ConstUInt_Functor {
		size_t reg_id;
		uint64_t opnum;
		uint64_t N;
		size_t reg_size;

		Mod_Mult_UInt_ConstUInt_Functor(size_t reg_id_, uint64_t opnum_, uint64_t N_, size_t reg_size_)
			: reg_id(reg_id_), opnum(opnum_), N(N_), reg_size(reg_size_) {}

		__host__ __device__ void operator()(System& s) const {
			uint64_t val = CuGetAsUint64(s, reg_id, reg_size);
			val = (val * opnum) % N;
			CuGet(s, reg_id).value = val;
		}
	};

	struct Mod_Mult_UInt_ConstUInt_Functor_Control {
		size_t reg_id;
		uint64_t opnum;
		uint64_t N;
		size_t reg_size;

		CuCondition_Functor

		Mod_Mult_UInt_ConstUInt_Functor_Control(size_t reg_id_, uint64_t opnum_, uint64_t N_, size_t reg_size_, CuCondition_Params)
			: reg_id(reg_id_), opnum(opnum_), N(N_), reg_size(reg_size_), CuCondition_Init {}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				uint64_t val = CuGetAsUint64(s, reg_id, reg_size);
				val = (val * opnum) % N;
				CuGet(s, reg_id).value = val;
			}
		}
	};

	struct Mod_Mult_UInt_ConstUInt_Functor_Dag {
		size_t reg_id;
		uint64_t inverse_opnum;
		uint64_t N;
		size_t reg_size;

		Mod_Mult_UInt_ConstUInt_Functor_Dag(size_t reg_id_, uint64_t inverse_opnum_, uint64_t N_, size_t reg_size_)
			: reg_id(reg_id_), inverse_opnum(inverse_opnum_), N(N_), reg_size(reg_size_) {}

		__host__ __device__ void operator()(System& s) const {
			uint64_t val = CuGetAsUint64(s, reg_id, reg_size);
			val = (val * inverse_opnum) % N;
			CuGet(s, reg_id).value = val;
		}
	};

	struct Mod_Mult_UInt_ConstUInt_Functor_Control_Dag {
		size_t reg_id;
		uint64_t inverse_opnum;
		uint64_t N;
		size_t reg_size;

		CuCondition_Functor

		Mod_Mult_UInt_ConstUInt_Functor_Control_Dag(size_t reg_id_, uint64_t inverse_opnum_, uint64_t N_, size_t reg_size_, CuCondition_Params)
			: reg_id(reg_id_), inverse_opnum(inverse_opnum_), N(N_), reg_size(reg_size_), CuCondition_Init {}

		__host__ __device__ void operator()(System& s) const {
			CuConditionSatisfied(s) {
				uint64_t val = CuGetAsUint64(s, reg_id, reg_size);
				val = (val * inverse_opnum) % N;
				CuGet(s, reg_id).value = val;
			}
		}
	};

	// Extended Euclidean algorithm for modular inverse
	__host__ __device__ static uint64_t compute_modular_inverse(uint64_t a, uint64_t n) {
		int64_t t = 0, new_t = 1;
		int64_t r_gcd = (int64_t)n;
		int64_t new_r_gcd = (int64_t)a;

		while (new_r_gcd != 0) {
			int64_t quotient = r_gcd / new_r_gcd;
			int64_t temp_t = t - quotient * new_t;
			int64_t temp_r = r_gcd - quotient * new_r_gcd;
			t = new_t;
			r_gcd = new_r_gcd;
			new_t = temp_t;
			new_r_gcd = temp_r;
		}

		if (r_gcd == 1) {
			int64_t t_normalized = t % (int64_t)n;
			if (t_normalized < 0) t_normalized += (int64_t)n;
			return (uint64_t)t_normalized;
		}
		return 1; // Should not happen if a and N are coprime
	}

	void Mod_Mult_UInt_ConstUInt::operator()(CuSparseState& state) const {
		state.move_to_gpu();
		size_t reg_size = System::size_of(reg);

		if (!HasCondition) {
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Mod_Mult_UInt_ConstUInt_Functor(reg, opnum, N, reg_size)
			);
		} else {
			CuCondition_Host_Prepare
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Mod_Mult_UInt_ConstUInt_Functor_Control(reg, opnum, N, reg_size, CuCondition_Args)
			);
		}
	}

	void Mod_Mult_UInt_ConstUInt::dag(CuSparseState& state) const {
		state.move_to_gpu();
		size_t reg_size = System::size_of(reg);
		uint64_t inverse_opnum = compute_modular_inverse(opnum, N);

		if (!HasCondition) {
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Mod_Mult_UInt_ConstUInt_Functor_Dag(reg, inverse_opnum, N, reg_size)
			);
		} else {
			CuCondition_Host_Prepare
			thrust::for_each(thrust::device,
				state.sparse_state_gpu.begin(), state.sparse_state_gpu.end(),
				Mod_Mult_UInt_ConstUInt_Functor_Control_Dag(reg, inverse_opnum, N, reg_size, CuCondition_Args)
			);
		}
	}

} // namespace qram_simulator
