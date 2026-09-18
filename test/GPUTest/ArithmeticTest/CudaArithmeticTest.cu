// CUDA-side arithmetic operator tests (docs/operators.md《宽度与截断约定》).
// Expected values are hardcoded (hand-derived) so this test never links or
// runs the CPU operator implementations.
#include <array>
#include <utility>
#include "error_handler.h"
#include "cuda/sparse_state_simulator.cuh"

using namespace qram_simulator;

namespace {
	// Read a register value from basis state 0 (moves the state to the CPU).
	uint64_t reg_value(CuSparseState& s, size_t id)
	{
		s.move_to_cpu();
		return s.sparse_state_cpu[0].get(id).value;
	}
}

// Add_UInt_UInt XOR-out semantics on CuSparseState, including a nonzero
// initial res register: res = res_init ^ ((lhs + rhs) & mask).
// Case 1: 3 + 5 = 8, res_init = 0 → 0 ^ 8 = 8
// Case 2: 3 + 5 = 8, res_init = 6 → 6 ^ 8 = 14
auto add_uint_uint_xor_semantics()
{
	System::clear();
	auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	auto res = System::add_register("res", UnsignedInteger, 4);

	{
		CuSparseState s;
		Init_Unsafe(lhs, 3)(s);
		Init_Unsafe(rhs, 5)(s);
		Init_Unsafe(res, 0)(s);
		Add_UInt_UInt("lhs", "rhs", "res")(s);
		if (reg_value(s, res) != 8) TEST_FAIL("Add_UInt_UInt zero-init res failed.");
		if (reg_value(s, lhs) != 3 || reg_value(s, rhs) != 5)
			TEST_FAIL("Add_UInt_UInt modified inputs.");
	}
	{
		CuSparseState s;
		Init_Unsafe(lhs, 3)(s);
		Init_Unsafe(rhs, 5)(s);
		Init_Unsafe(res, 6)(s);
		Add_UInt_UInt(lhs, rhs, res)(s);
		if (reg_value(s, res) != 14) TEST_FAIL("Add_UInt_UInt nonzero-init res failed.");
	}
	fmt::print("Test passed.\n");
}

// Add_UInt_UInt_InPlace with unequal widths: rhs += lhs, masked to the rhs
// width (lhs is read as an integer, rhs wraps mod 2^rhs_width).
// Case 1: lhs(3b)=7, rhs(5b)=3 → rhs = (3 + 7) & 0x1F = 10
// Case 2: lhs(5b)=21, rhs(3b)=5 → rhs = (5 + 21) & 0x7 = 26 & 7 = 2
auto add_uint_uint_inplace_unequal_width()
{
	System::clear();
	auto lhs_w3 = System::add_register("lhs_w3", UnsignedInteger, 3);
	auto rhs_w5 = System::add_register("rhs_w5", UnsignedInteger, 5);
	auto lhs_w5 = System::add_register("lhs_w5", UnsignedInteger, 5);
	auto rhs_w3 = System::add_register("rhs_w3", UnsignedInteger, 3);

	{
		CuSparseState s;
		Init_Unsafe(lhs_w3, 7)(s);
		Init_Unsafe(rhs_w5, 3)(s);
		Add_UInt_UInt_InPlace("lhs_w3", "rhs_w5")(s);
		if (reg_value(s, rhs_w5) != 10) TEST_FAIL("Add_UInt_UInt_InPlace 3b→5b failed.");
		if (reg_value(s, lhs_w3) != 7) TEST_FAIL("Add_UInt_UInt_InPlace modified lhs.");
	}
	{
		CuSparseState s;
		Init_Unsafe(lhs_w5, 21)(s);
		Init_Unsafe(rhs_w3, 5)(s);
		Add_UInt_UInt_InPlace(lhs_w5, rhs_w3)(s);
		if (reg_value(s, rhs_w3) != 2) TEST_FAIL("Add_UInt_UInt_InPlace 5b→3b mask failed.");
	}
	fmt::print("Test passed.\n");
}

// Add_AnyInt_AnyInt_InPlace with a signed rhs read sign-extended:
// lhs 6-bit UInt, rhs 3-bit SInt bits 0b111 = -1 → lhs += -1 (mod 64).
// lhs = 0 → 63, lhs = 5 → 4; dagger restores the original value.
auto add_anyint_signed_rhs_extension()
{
	System::clear();
	auto lhs = System::add_register("lhs", UnsignedInteger, 6);
	auto rhs = System::add_register("rhs", SignedInteger, 3);

	{
		CuSparseState s;
		Init_Unsafe(lhs, 0)(s);
		Init_Unsafe(rhs, 0b111)(s);
		Add_AnyInt_AnyInt_InPlace("lhs", "rhs")(s);
		if (reg_value(s, lhs) != 63) TEST_FAIL("Add_AnyInt signed-rhs 0-1 failed.");
	}
	{
		CuSparseState s;
		Init_Unsafe(lhs, 5)(s);
		Init_Unsafe(rhs, 0b111)(s);
		Add_AnyInt_AnyInt_InPlace(lhs, rhs)(s);
		if (reg_value(s, lhs) != 4) TEST_FAIL("Add_AnyInt signed-rhs 5-1 failed.");
		if (reg_value(s, rhs) != 0b111) TEST_FAIL("Add_AnyInt modified rhs bits.");
	}
	{
		// dagger round-trip: (0 + (-1)) then subtract (-1) restores 0
		CuSparseState s;
		Init_Unsafe(lhs, 0)(s);
		Init_Unsafe(rhs, 0b111)(s);
		Add_AnyInt_AnyInt_InPlace op(lhs, rhs);
		op(s);
		op.dag(s);
		if (reg_value(s, lhs) != 0) TEST_FAIL("Add_AnyInt signed-rhs dagger failed.");
	}
	fmt::print("Test passed.\n");
}

// Div_UInt_UInt zero-divisor total-domain convention (quotient 0) plus a
// normal floor division: 13/0 = 0, 13/5 = 2.
auto div_uint_uint_zero_divisor()
{
	System::clear();
	auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	auto rhs = System::add_register("rhs", UnsignedInteger, 3);
	auto res = System::add_register("res", UnsignedInteger, 4);

	{
		CuSparseState s;
		Init_Unsafe(lhs, 13)(s);
		Init_Unsafe(rhs, 0)(s);
		Init_Unsafe(res, 0)(s);
		Div_UInt_UInt("lhs", "rhs", "res")(s);
		if (reg_value(s, res) != 0) TEST_FAIL("Div zero-divisor must yield 0.");
	}
	{
		CuSparseState s;
		Init_Unsafe(lhs, 13)(s);
		Init_Unsafe(rhs, 5)(s);
		Init_Unsafe(res, 0)(s);
		Div_UInt_UInt(lhs, rhs, res)(s);
		if (reg_value(s, res) != 2) TEST_FAIL("Div 13/5 must yield 2.");
	}
	fmt::print("Test passed.\n");
}

// Mul_UInt_UInt truncation to the res width: 15 * 15 = 225.
// 4-bit res: 225 mod 16 = 1; 8-bit res: 225 fits → 225.
auto mul_uint_uint_truncation()
{
	System::clear();
	auto lhs = System::add_register("lhs", UnsignedInteger, 4);
	auto rhs = System::add_register("rhs", UnsignedInteger, 4);
	auto res_w4 = System::add_register("res_w4", UnsignedInteger, 4);
	auto res_w8 = System::add_register("res_w8", UnsignedInteger, 8);

	CuSparseState s;
	Init_Unsafe(lhs, 15)(s);
	Init_Unsafe(rhs, 15)(s);
	Init_Unsafe(res_w4, 0)(s);
	Init_Unsafe(res_w8, 0)(s);
	Mul_UInt_UInt("lhs", "rhs", "res_w4")(s);
	Mul_UInt_UInt(lhs, rhs, res_w8)(s);
	if (reg_value(s, res_w4) != 1) TEST_FAIL("Mul 4-bit truncation failed.");
	if (reg_value(s, res_w8) != 225) TEST_FAIL("Mul 8-bit product failed.");
	fmt::print("Test passed.\n");
}

// Sqrt_UInt integer square root: isqrt(48) = 6, isqrt(63) = 7, isqrt(0) = 0.
auto sqrt_uint_values()
{
	System::clear();
	auto reg = System::add_register("reg", UnsignedInteger, 6);
	auto res = System::add_register("res", UnsignedInteger, 3);

	const std::array<std::pair<uint64_t, uint64_t>, 3> cases = {{
		{48, 6}, {63, 7}, {0, 0},
	}};
	for (const auto& [input, expected] : cases)
	{
		CuSparseState s;
		Init_Unsafe(reg, input)(s);
		Init_Unsafe(res, 0)(s);
		Sqrt_UInt("reg", "res")(s);
		if (reg_value(s, res) != expected) TEST_FAIL("Sqrt_UInt value failed.");
	}
	fmt::print("Test passed.\n");
}

int main()
{
	try
	{
		TEST(add_uint_uint_xor_semantics);
		TEST(add_uint_uint_inplace_unequal_width);
		TEST(add_anyint_signed_rhs_extension);
		TEST(div_uint_uint_zero_divisor);
		TEST(mul_uint_uint_truncation);
		TEST(sqrt_uint_values);

		fmt::print("All tests passed.\n");
		return 0;
	}
	catch (const TestFailException& e)
	{
		fmt::print("Test failed: {}\n", e.what());
		return 1;
	}
	catch (const std::runtime_error& e)
	{
		fmt::print("Runtime error: {}\n", e.what());
		return 2;
	}
	catch (const std::exception& e)
	{
		fmt::print("Error: {}\n", e.what());
		return 3;
	}

	return 0;
}
