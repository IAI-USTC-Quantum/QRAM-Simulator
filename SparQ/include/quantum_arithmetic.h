#include "basic_components.h"

namespace qram_simulator
{
	/* Flip every digit in a General-Purpose register. */
	/* Data type : General Purpose. */
	/* Possible unsafety: the remaining digits are also flipped */
	struct FlipBools : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		ClassControllable
		size_t id;
		FlipBools(std::string_view reg)
			:id(System::get(reg)) { }
		FlipBools(size_t id_)
			:id(id_)
		{}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/* */

	/* Swap two digits in two different registers */
	/* Data type : General Purpose. */
	/* Possible unsafety: Input digit may overflow. */
	struct Swap_Bool_Bool : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t lhs;
		size_t rhs;
		size_t digit1;
		size_t digit2;
		ClassControllable
		Swap_Bool_Bool(std::string_view reg1, size_t d1,
			std::string_view reg2, size_t d2)
			: lhs(System::get(reg1)), rhs(System::get(reg2)),
			digit1(d1), digit2(d2)
		{
		}
		Swap_Bool_Bool(size_t id1, size_t d1,
			size_t id2, size_t d2) : lhs(id1), rhs(id2), digit1(d1), digit2(d2)
		{
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/* Shift left. Rotate the highest digits to the last. */
	/* Data type : Unsigned Int is suggested. */
	/* Possible unsafety: */
	struct ShiftLeft : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t register_1;
		size_t digit;
		ClassControllable
		ShiftLeft(std::string_view reg1, size_t d) 
			:register_1(System::get(reg1)), digit(d)
		{
		}
		ShiftLeft(size_t reg1, size_t d)
			:register_1(reg1), digit(d)
		{
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};


	/* Shift right. Rotate the lowest digits to the top. */
	/* Data type : Unsigned Int is suggested. */
	/* Possible unsafety: */
	struct ShiftRight : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t register_1;
		size_t digit;
		ClassControllable
		ShiftRight(std::string_view reg1, size_t d)
			:register_1(System::get(reg1)), digit(d)
		{
		}
		ShiftRight(size_t reg1, size_t d)
			:register_1(reg1), digit(d)
		{
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/* z = x * y */
	/* Data type: Unsigned Int, Const Unsigned Int, Unsigned Int */
	/* Possible unsafety: Result may overflow.
	   High digits are not immediately truncated.
	*/
	struct Mult_UInt_ConstUInt : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t mult_int;
		size_t lhs;
		size_t res;
		ClassControllable		
		Mult_UInt_ConstUInt(std::string_view reg_in, size_t mult, std::string_view reg_out)
			: lhs(System::get(reg_in)), res(System::get(reg_out)), mult_int(mult)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}
		Mult_UInt_ConstUInt(size_t reg_in, size_t mult, size_t reg_out)
			: lhs(reg_in), res(reg_out), mult_int(mult)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/* z += x * y */
	/* Data type: Unsigned Int, Const Unsigned Int, Unsigned Int */
	/* Possible unsafety: Result may overflow.
	*/
	struct Add_Mult_UInt_ConstUInt : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t mult_int;
		size_t lhs;
		size_t res;
		ClassControllable
		Add_Mult_UInt_ConstUInt(std::string_view reg_in, size_t mult, std::string_view reg_out)
			: mult_int(mult), lhs(System::get(reg_in)), res(System::get(reg_out))
		{
		}
		Add_Mult_UInt_ConstUInt(size_t reg_in, size_t mult, size_t reg_out)
			: mult_int(mult), lhs(reg_in), res(reg_out)
		{
		}
		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
		void dag(CuSparseState& state) const;
#endif
	};

	/* z = x + y */
	/* Data type: Unsigned Int, Unsigned Int, Unsigned Int */
	/* Possible unsafety: Result may overflow.
	   High digits are not immediately truncated.
	*/
	struct Add_UInt_UInt : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t lhs;
		size_t rhs;
		size_t res;
		ClassControllable		
		Add_UInt_UInt(std::string_view lhs_, std::string_view rhs_, std::string_view res_)
			: lhs(System::get(lhs_)), rhs(System::get(rhs_)), res(System::get(res_))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(rhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}
		Add_UInt_UInt(size_t lhs_, size_t rhs_, size_t res_)
			: lhs(lhs_), rhs(rhs_), res(res_)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(rhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif	
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct Add_UInt_UInt_InPlace : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t lhs;
		size_t rhs;
		ClassControllable
		Add_UInt_UInt_InPlace(std::string_view lhs_, std::string_view rhs_)
			:lhs(System::get(lhs_)), rhs(System::get(rhs_))
		{
		}
		Add_UInt_UInt_InPlace(size_t lhs_, size_t rhs_)
			: lhs(lhs_), rhs(rhs_)
		{
		}
		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
		void dag(CuSparseState& state) const;
#endif
	};

	/* z = x + y */
	/* Data type: Unsigned Int, Const Unsigned Int, Unsigned Int */
	/* Possible unsafety: Result may overflow.
	   High digits are not immediately truncated.
	*/
	struct Add_UInt_ConstUInt : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t add_int;
		size_t lhs;
		size_t res;
		ClassControllable		
		Add_UInt_ConstUInt(std::string_view reg_in, size_t add, std::string_view reg_out)
			: lhs(System::get(reg_in)), res(System::get(reg_out)), add_int(add)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif		
		}
		Add_UInt_ConstUInt(size_t reg_in, size_t add, size_t reg_out)
			: lhs(reg_in), res(reg_out), add_int(add)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(lhs) != UnsignedInteger ||
				System::type_of(res) != UnsignedInteger)
				throw_invalid_input();
#endif		
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct Add_ConstUInt : BaseOperator {
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t add_int;
		size_t reg_in;
		ClassControllable
		Add_ConstUInt(std::string_view reg_in_, size_t add) :
			reg_in(System::get(reg_in_)), add_int(add)
		{
		}

		Add_ConstUInt(size_t reg_in, size_t add)
			: reg_in(reg_in), add_int(add)
		{
		}

		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
		void dag(CuSparseState& state) const;
#endif
	};

	/* z = arccos(sqrt (y / x)) */
	/* Data type: Unsigned Int, Unsigned Int, Rational */
	/* Possible unsafety: zero input may be observed.
	*/
	struct Div_Sqrt_Arccos_Int_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		ClassControllable		
		Div_Sqrt_Arccos_Int_Int(std::string_view register_lhs, std::string_view register_rhs, std::string_view register_out)
			:register_lhs(System::get(register_lhs)),
			register_rhs(System::get(register_rhs)),
			register_out(System::get(register_out))
		{
			/* Type check */
#ifndef QRAM_Release
		/*if (System::type_of(register_lhs) != UnsignedInteger ||
			System::type_of(register_rhs) != UnsignedInteger ||
			System::type_of(register_out) != Rational)
			throw_invalid_input();*/
#endif		
		}

		Div_Sqrt_Arccos_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_lhs) != UnsignedInteger ||
				System::type_of(register_rhs) != UnsignedInteger ||
				System::type_of(register_out) != Rational)
				throw_invalid_input();
#endif		
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/* z = arccos(sqrt (y) / x) */
	/* Data type: Unsigned Int, Int, Rational */
	/* Possible unsafety: zero input may be observed.
	*/
	struct Sqrt_Div_Arccos_Int_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		ClassControllable		
		Sqrt_Div_Arccos_Int_Int(std::string_view lhs, std::string_view rhs, std::string_view out)
			:register_lhs(System::get(lhs)),
			register_rhs(System::get(rhs)),
			register_out(System::get(out))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_lhs) != SignedInteger ||
				System::type_of(register_rhs) != UnsignedInteger ||
				System::type_of(register_out) != Rational)
				throw_invalid_input();
#endif
		}
		Sqrt_Div_Arccos_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_lhs) != SignedInteger ||
				System::type_of(register_rhs) != UnsignedInteger ||
				System::type_of(register_out) != Rational)
				throw_invalid_input();
#endif
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};


	/* ? */
	struct GetRotateAngle_Int_Int : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t register_lhs;
		size_t register_rhs;
		size_t register_out;
		ClassControllable		
		GetRotateAngle_Int_Int(std::string_view reg_lhs, std::string_view reg_rhs, std::string_view reg_out)
			:register_lhs(System::get(reg_lhs)),
			register_rhs(System::get(reg_rhs)),
			register_out(System::get(reg_out))
		{
		}
		GetRotateAngle_Int_Int(size_t reg_lhs, size_t reg_rhs, size_t reg_out)
			:register_lhs(reg_lhs),
			register_rhs(reg_rhs),
			register_out(reg_out)
		{
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/* y += x */
	/* Data type: (U)Int, (U)Int */
	/* Possible unsafety: Overflow. */
	/* Todo: Check constructor's type and size check. */
	struct AddAssign_AnyInt_AnyInt : BaseOperator
	{
		using BaseOperator::operator();
		using BaseOperator::dag;

		size_t lhs_id;
		size_t rhs_id;
		size_t lhs_size;
		size_t rhs_size;
		StateStorageType lhs_type;
		StateStorageType rhs_type;
		AddAssign_AnyInt_AnyInt(std::string_view reg_lhs, std::string_view reg_rhs)
			: lhs_id(System::get(reg_lhs)), rhs_id(System::get(reg_rhs)),
			lhs_size(System::size_of(lhs_id)), rhs_size(System::size_of(rhs_id)),
			lhs_type(System::type_of(lhs_id)), rhs_type(System::type_of(rhs_id))
		{
			/* Type check */
#ifndef QRAM_Release
			if (lhs_type != UnsignedInteger && lhs_type != SignedInteger)
				throw_invalid_input();
			if (rhs_type != UnsignedInteger && rhs_type != SignedInteger)
				throw_invalid_input();
#endif
		}
		AddAssign_AnyInt_AnyInt(size_t reg_lhs, size_t reg_rhs)
			: lhs_id(reg_lhs), rhs_id(reg_rhs),
			lhs_size(System::size_of(lhs_id)), rhs_size(System::size_of(rhs_id)),
			lhs_type(System::type_of(lhs_id)), rhs_type(System::type_of(rhs_id))
		{
			/* Type check */
#ifndef QRAM_Release
			if (lhs_type != UnsignedInteger && lhs_type != SignedInteger)
				throw_invalid_input();
			if (rhs_type != UnsignedInteger && rhs_type != SignedInteger)
				throw_invalid_input();
#endif
		}

		ClassControllable

		/* should make computation reversible */

		static void _operate_uint_uint(uint64_t& lhs, size_t l_size, uint64_t rhs, size_t r_size);
		static void _operate_uint_uint_dag(uint64_t& lhs, size_t l_size, uint64_t rhs, size_t r_size);

		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
		void dag(CuSparseState& state) const;
#endif
	};

	/* y ^= x */
	/* Data type: General, General */
	/* Possible unsafety:  High digits are not immediately truncated. */
	struct Assign : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t register_1;
		size_t register_2;
		Assign(std::string_view reg1, std::string_view reg2)
			:register_1(System::get(reg1)), register_2(System::get(reg2))
		{
		}
		Assign(size_t reg1, size_t reg2)
			:register_1(reg1), register_2(reg2)
		{
		}

		ClassControllable		

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/*
	|l> |r> |0>      |0>	  ->
	|l> |r> |l < r?> |l == r?>
	*/
	struct Compare_UInt_UInt : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t left_id;
		size_t right_id;
		size_t compare_less_id;
		size_t compare_equal_id;
		Compare_UInt_UInt(
			std::string_view left_register,
			std::string_view right_register,
			std::string_view compare_less,
			std::string_view compare_equal)
			: left_id(System::get(left_register)), right_id(System::get(right_register)),
			compare_less_id(System::get(compare_less)), compare_equal_id(System::get(compare_equal))
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean ||
				System::type_of(compare_equal_id) != Boolean)
				throw_invalid_input();
#endif
		}

		Compare_UInt_UInt(size_t lreg, size_t rreg,
			size_t compare_less, size_t compare_equal)
			:left_id(lreg), right_id(rreg),
			compare_less_id(compare_less), compare_equal_id(compare_equal)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean ||
				System::type_of(compare_equal_id) != Boolean)
				throw_invalid_input();
#endif
		}

		ClassControllable		

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/*
	|l> |r> |0>    	  ->
	|l> |r> |l < r?> 
	*/
	struct Less_UInt_UInt : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t left_id;
		size_t right_id;
		size_t compare_less_id;
		Less_UInt_UInt(
			std::string_view lreg,
			std::string_view rreg,
			std::string_view compare_less)
		{
			left_id = System::get(lreg);
			right_id = System::get(rreg);
			compare_less_id = System::get(compare_less);
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean)
				throw_invalid_input();
#endif
		}

		Less_UInt_UInt(size_t lreg, size_t rreg, size_t compare_less)
			: left_id(lreg), right_id(rreg), compare_less_id(compare_less)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(compare_less_id) != Boolean)
				throw_invalid_input();
#endif
		}

		ClassControllable		

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct Swap_General_General : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id1;
		size_t id2;
		Swap_General_General(std::string_view regname1, std::string_view regname2)
		{
			id1 = System::get(regname1);
			id2 = System::get(regname2);

			/* Type check */
#ifndef QRAM_Release
			if (System::size_of(id1) != System::size_of(id2))
				throw_invalid_input();
#endif		
		}
		Swap_General_General(size_t regname1, size_t regname2)
			: id1(regname1), id2(regname2)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::size_of(id1) != System::size_of(id2))
				throw_invalid_input();
#endif		
		}

		ClassControllable		

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	/*
	|l>|r>|0>	->
	|l>|r>|mid>
	*/
	struct GetMid_UInt_UInt : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t left_id;
		size_t right_id;
		size_t mid_id;

		GetMid_UInt_UInt(
			std::string_view left_register_,
			std::string_view right_register_,
			std::string_view mid_register_) :
			left_id(System::get(left_register_)),
			right_id(System::get(right_register_)),
			mid_id(System::get(mid_register_))

		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(mid_id) != UnsignedInteger)
				throw_invalid_input();

			if (System::size_of(left_id) != System::size_of(right_id))
				throw_invalid_input();
			if (System::size_of(right_id) != System::size_of(mid_id))
				throw_invalid_input();
#endif
		}

		GetMid_UInt_UInt(
			size_t left_register_, size_t right_register_, size_t mid_register_)
			:left_id(left_register_), right_id(right_register_),
			mid_id(mid_register_)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(left_id) != UnsignedInteger ||
				System::type_of(right_id) != UnsignedInteger ||
				System::type_of(mid_id) != UnsignedInteger)
				throw_invalid_input();

			if (System::size_of(left_id) != System::size_of(right_id))
				throw_invalid_input();
			if (System::size_of(right_id) != System::size_of(mid_id))
				throw_invalid_input();
#endif
		}

		ClassControllable
		
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	using GenericArithmetic = std::function<std::vector<size_t>(const std::vector<size_t>&)>;

	struct CustomArithmetic : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		std::vector<size_t> input_ids;
		std::vector<size_t> output_ids;
		GenericArithmetic func;

		CustomArithmetic(const std::vector<size_t>& input_registers,
			size_t input_size, size_t output_size,
			GenericArithmetic func)
			: func(func)
		{
			if (input_ids.size() != output_ids.size())
			{
				throw std::invalid_argument("Input and output registers size does not match.");
			}
			// the first "input_size" registers are input registers, the rest are output registers.
			input_ids.resize(input_size);
			output_ids.resize(output_size);
			for (size_t i = 0; i < input_size; ++i)
			{
				input_ids[i] = input_registers[i];
			}
			for (size_t i = 0; i < output_size; ++i)
			{
				output_ids[i] = input_registers[i + input_size];
			}
		}
		CustomArithmetic(const std::vector<std::string>& input_registers,
			size_t input_size, size_t output_size, GenericArithmetic func)
			: func(func)
		{
			if ((input_size + output_size) != input_registers.size())
			{
				throw std::invalid_argument("Input registers size does not match input size + output size.");
			}
			// the first "input_size" registers are input registers, the rest are output registers.
			input_ids.resize(input_size);
			output_ids.resize(output_size);
			for (size_t i = 0; i < input_size; ++i)
			{
				input_ids[i] = System::get(input_registers[i]);
			}
			for (size_t i = 0; i < output_size; ++i)
			{
				output_ids[i] = System::get(input_registers[i + input_size]);
			}
		}

		ClassControllable		

		void operator()(std::vector<System>& state) const
		{
#ifdef SINGLE_THREAD
			for (auto& s : state)
			{
#else
#pragma omp parallel for
			for (int64_t i = 0; i < state.size(); ++i)
			{
				auto& s = state[i];
#endif
				if (ConditionNotSatisfied(s))
					continue;

				/* obtain the value of the input registers */
				std::vector<size_t> input_values(input_ids.size());
				for (size_t i = 0; i < input_ids.size(); ++i)
				{
					input_values[i] = s.GetAs(input_ids[i], uint64_t);
				}
				/* apply the custom function */
				std::vector<size_t> output_values = func(input_values);

				/* write the output reversibly via XOR */
				for (size_t i = 0; i < output_ids.size(); ++i)
				{
					s.get(output_ids[i]).value ^= output_values[i];
				}
			}
		}
	};
} // namespace qram_simulator