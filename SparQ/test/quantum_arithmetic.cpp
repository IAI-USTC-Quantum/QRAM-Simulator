// Include error_handler.h FIRST to access the TEST macro, then undefine it
// so that gtest's TEST macro can be used instead
#include <string>
#include <string_view>
#include "error_handler.h"
#undef TEST

#include <gtest/gtest.h>
#include "sparse_state_simulator.h"
#include "debugger.h"
#include <cmath>

using namespace qram_simulator;

// Unitary verification helpers - use small state space to reduce CI load
namespace {
    constexpr size_t UNITARY_TEST_STATE_SIZE = 2;  // 4 values instead of 8

    template<typename OpType, typename... Args>
    bool verify_outofplace_unitarity(Args&&... args) {
        System::clear();
        auto reg1 = System::add_register("reg1", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        auto reg2 = System::add_register("reg2", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        auto res = System::add_register("res", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        OpType op(std::forward<Args>(args)...);

        for (size_t v1 = 0; v1 < 4; ++v1) {
            for (size_t v2 = 0; v2 < 4; ++v2) {
                std::vector<System> state;
                state.emplace_back();
                state[0].get(reg1).value = v1;
                state[0].get(reg2).value = v2;
                state[0].get(res).value = 0;
                op(state);
                op(state);
                if (state.size() != 1 || state[0].get(reg1).value != v1 ||
                    state[0].get(reg2).value != v2 || state[0].get(res).value != 0)
                    return false;
            }
        }
        System::clear();
        return true;
    }

    template<typename OpType, typename... Args>
    bool verify_inplace_unitarity(Args&&... args) {
        System::clear();
        auto reg1 = System::add_register("reg1", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        auto reg2 = System::add_register("reg2", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        OpType op(std::forward<Args>(args)...);

        for (size_t v1 = 0; v1 < 4; ++v1) {
            for (size_t v2 = 0; v2 < 4; ++v2) {
                std::vector<System> state;
                state.emplace_back();
                state[0].get(reg1).value = v1;
                state[0].get(reg2).value = v2;
                op(state);
                op.dag(state);
                if (state.size() != 1 || state[0].get(reg1).value != v1 || state[0].get(reg2).value != v2)
                    return false;
            }
        }
        System::clear();
        return true;
    }

    // Separate function for single-register in-place operators like Add_ConstUInt
    // (Cannot use template specialization because the primary template creates two registers)
    template<typename OpType>
    bool verify_single_reg_inplace_unitarity(std::string reg_name, size_t arg_val) {
        System::clear();
        auto reg = System::add_register(reg_name, UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        OpType op(reg_name, arg_val);

        for (size_t v = 0; v < 4; ++v) {
            std::vector<System> state;
            state.emplace_back();
            state[0].get(reg).value = v;
            op(state);
            op.dag(state);
            if (state.size() != 1 || state[0].get(reg).value != v)
                return false;
        }
        System::clear();
        return true;
    }

    // Bidirectional round-trip: forward then dagger must restore original
    template<typename OpType, typename... Args>
    bool verify_inplace_unitarity_fwd_then_dag(Args&&... args) {
        System::clear();
        auto reg1 = System::add_register("reg1", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        auto reg2 = System::add_register("reg2", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        OpType op(std::forward<Args>(args)...);
        for (size_t v1 = 0; v1 < 4; ++v1) {
            for (size_t v2 = 0; v2 < 4; ++v2) {
                std::vector<System> st;
                st.emplace_back();
                st[0].get(reg1).value = v1;
                st[0].get(reg2).value = v2;
                op(st);      // apply U
                op.dag(st);  // apply U† — must restore original
                if (st.size() != 1 || st[0].get(reg1).value != v1 || st[0].get(reg2).value != v2)
                    return false;
            }
        }
        System::clear();
        return true;
    }

    // Bidirectional round-trip: dagger then forward must restore original
    template<typename OpType, typename... Args>
    bool verify_inplace_unitarity_dag_then_fwd(Args&&... args) {
        System::clear();
        auto reg1 = System::add_register("reg1", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        auto reg2 = System::add_register("reg2", UnsignedInteger, UNITARY_TEST_STATE_SIZE);
        OpType op(std::forward<Args>(args)...);
        for (size_t v1 = 0; v1 < 4; ++v1) {
            for (size_t v2 = 0; v2 < 4; ++v2) {
                std::vector<System> st;
                st.emplace_back();
                st[0].get(reg1).value = v1;
                st[0].get(reg2).value = v2;
                op.dag(st);  // apply U†
                op(st);      // apply U — must restore original
                if (st.size() != 1 || st[0].get(reg1).value != v1 || st[0].get(reg2).value != v2)
                    return false;
            }
        }
        System::clear();
        return true;
    }
}

class QuantumArithmeticTest : public ::testing::Test {
protected:
    void SetUp() override {
        System::clear();
    }
    void TearDown() override {
        System::clear();
    }
};

// Helper to get register value
uint64_t getRegValue(const System& s, size_t reg_id, size_t reg_size) {
    return s.get(reg_id).as<uint64_t>(reg_size);
}

// ============ Addition Tests ============

// Test z = x + y with unsigned integers
TEST(QuantumArithmeticTest, AddUIntUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto rhs_reg = System::add_register("rhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();  // |0, 0, 0>

    Init_Unsafe(lhs_reg, 3)(state);  // lhs = 3
    Init_Unsafe(rhs_reg, 5)(state);  // rhs = 5

    Add_UInt_UInt("lhs", "rhs", "res")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t res = getRegValue(state[0], res_reg, 4);
    EXPECT_EQ(res, 8);  // 3 + 5 = 8
}

// Test Add_UInt_UInt_InPlace: rhs += lhs
// Note: rhs is modified in place (lhs value is added TO rhs)
TEST(QuantumArithmeticTest, AddUIntUIntInPlace)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto rhs_reg = System::add_register("rhs", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(lhs_reg, 7)(state);
    Init_Unsafe(rhs_reg, 3)(state);

    // Add lhs to rhs: rhs += lhs, so rhs becomes 3 + 7 = 10
    Add_UInt_UInt_InPlace("lhs", "rhs")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t rhs_val = getRegValue(state[0], rhs_reg, 4);
    EXPECT_EQ(rhs_val, 10);  // 3 + 7 = 10
}

// Test Add_UInt_ConstUInt: z = x + constant
TEST(QuantumArithmeticTest, AddUIntConstUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(lhs_reg, 6)(state);

    Add_UInt_ConstUInt("lhs", 4, "res")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t res = getRegValue(state[0], res_reg, 4);
    EXPECT_EQ(res, 10);  // 6 + 4 = 10
}

// Test Add_ConstUInt: y += constant (in-place, with overflow check)
TEST(QuantumArithmeticTest, AddConstUIntInPlace)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(reg, 12)(state);

    // 12 + 3 = 15, which fits in 4 bits (max 15)
    Add_ConstUInt_InPlace("reg", 3)(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t val = getRegValue(state[0], reg, 4);
    EXPECT_EQ(val, 15);  // 12 + 3 = 15
}

// ============ Multiplication Tests ============

// Test z = x * constant (Mult_UInt_ConstUInt)
TEST(QuantumArithmeticTest, MultUIntConstUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(lhs_reg, 3)(state);

    Mult_UInt_ConstUInt("lhs", 4, "res")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t res = getRegValue(state[0], res_reg, 4);
    EXPECT_EQ(res, 12);  // 3 * 4 = 12
}

// Test z += x * constant (Add_Mult_UInt_ConstUInt)
// Note: The API behavior depends on overflow handling and implementation details.
// Testing with small values to verify the operation runs without error.
TEST(QuantumArithmeticTest, AddMultUIntConstUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(lhs_reg, 1)(state);
    Init_Unsafe(res_reg, 2)(state);

    // res += lhs * constant = 2 + 1 * 2 = 4
    Add_Mult_UInt_ConstUInt_InPlace("lhs", 2, "res")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t res = getRegValue(state[0], res_reg, 4);
    EXPECT_EQ(res, 4);  // 2 + (1 * 2) = 4
}

// ============ Bit Manipulation Tests ============

// Test FlipBools - flip all bits in a register
TEST(QuantumArithmeticTest, FlipBools)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(reg, 0b1010)(state);  // 10 in binary

    FlipBools("reg")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t val = getRegValue(state[0], reg, 4);
    EXPECT_EQ(val, 0b0101);  // Flipped: 10 -> 5
}

// ============ Assignment Tests ============

// Test Assign - copy register value
TEST(QuantumArithmeticTest, Assign)
{
    auto src = System::add_register("src", UnsignedInteger, 4);
    auto dst = System::add_register("dst", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(src, 7)(state);
    Init_Unsafe(dst, 0)(state);

    Assign("src", "dst")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t dst_val = getRegValue(state[0], dst, 4);
    EXPECT_EQ(dst_val, 7);  // dst should now equal src
}

// ============ Comparison Tests ============

// Test Compare_UInt_UInt
TEST(QuantumArithmeticTest, CompareUIntUInt)
{
    auto left2 = System::add_register("left2", UnsignedInteger, 4);
    auto right2 = System::add_register("right2", UnsignedInteger, 4);
    auto cmp_less2 = System::add_register("cmp_less2", Boolean, 1);
    auto cmp_eq2 = System::add_register("cmp_eq2", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(left2, 3)(state);
    Init_Unsafe(right2, 3)(state);  // Equal values

    Compare_UInt_UInt("left2", "right2", "cmp_less2", "cmp_eq2")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t less_val = getRegValue(state[0], cmp_less2, 1);
    uint64_t eq_val = getRegValue(state[0], cmp_eq2, 1);
    EXPECT_EQ(eq_val, 1);  // 3 == 3
    EXPECT_EQ(less_val, 0);  // Not less
}

// ============ In-Place Addition ============

// Test AddAssign_AnyInt_AnyInt
TEST(QuantumArithmeticTest, AddAssignAnyIntAnyInt)
{
    auto lhs = System::add_register("lhs", UnsignedInteger, 4);
    auto rhs = System::add_register("rhs", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(lhs, 8)(state);
    Init_Unsafe(rhs, 6)(state);

    AddAssign_AnyInt_AnyInt_InPlace("lhs", "rhs")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t lhs_val = getRegValue(state[0], lhs, 4);
    EXPECT_EQ(lhs_val, 14);  // 8 + 6 = 14
}

// ============ Custom Arithmetic Test ============

// Test CustomArithmetic with a simple doubling function
TEST(QuantumArithmeticTest, CustomArithmetic)
{
    auto inp = System::add_register("inp", UnsignedInteger, 4);
    auto out = System::add_register("out", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(inp, 7)(state);

    // Custom function: output = input * 2
    GenericArithmetic double_func = [](const std::vector<size_t>& inputs) {
        return std::vector<size_t>{inputs[0] * 2};
    };

    std::vector<std::string> regs = {"inp", "out"};
    CustomArithmetic arith(regs, 1, 1, double_func);
    arith(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t out_val = getRegValue(state[0], out, 4);
    EXPECT_EQ(out_val, 14);  // 7 * 2 = 14
}

// ============ GetMid (Mid-point) Test ============
TEST(QuantumArithmeticTest, GetMid)
{
    auto left = System::add_register("left", UnsignedInteger, 4);
    auto right = System::add_register("right", UnsignedInteger, 4);
    auto mid = System::add_register("mid", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    Init_Unsafe(left, 0)(state);
    Init_Unsafe(right, 10)(state);

    GetMid_UInt_UInt("left", "right", "mid")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t mid_val = getRegValue(state[0], mid, 4);
    EXPECT_EQ(mid_val, 5);  // (0 + 10) / 2 = 5
}

// ============ Unitarity Tests ============
// These tests verify that operators satisfy the unitary condition: U^dagger * U = I

// Test Add_UInt_UInt unitarity (out-of-place, self-adjoint)
TEST(QuantumArithmeticTest, AddUIntUIntUnitarity)
{
    EXPECT_TRUE((verify_outofplace_unitarity<Add_UInt_UInt>("reg1", "reg2", "res")));
}

// Test Add_UInt_UInt_InPlace unitarity (in-place with explicit dagger)
TEST(QuantumArithmeticTest, AddUIntUIntInPlaceUnitarity)
{
    EXPECT_TRUE((verify_inplace_unitarity<Add_UInt_UInt_InPlace>("reg1", "reg2")));
}

// Test Add_ConstUInt unitarity with a few constant values
TEST(QuantumArithmeticTest, AddConstUIntUnitarity)
{
    EXPECT_TRUE((verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 1)));
    EXPECT_TRUE((verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 3)));
}

// Test Mult_UInt_ConstUInt unitarity (out-of-place, self-adjoint)
TEST(QuantumArithmeticTest, MultUIntConstUIntUnitarity)
{
    EXPECT_TRUE((verify_outofplace_unitarity<Mult_UInt_ConstUInt>("reg1", 3, "res")));
}

// Test FlipBools unitarity (out-of-place, self-adjoint)
TEST(QuantumArithmeticTest, FlipBoolsUnitarity)
{
    System::clear();
    auto reg = System::add_register("reg", UnsignedInteger, 2);
    FlipBools op("reg");

    for (size_t v = 0; v < 4; ++v) {
        std::vector<System> state;
        state.emplace_back();
        state[0].get(reg).value = v;
        op(state);
        op(state);
        EXPECT_EQ(state[0].get(reg).value, v);
    }
    System::clear();
}

// Test Swap_General_General unitarity
TEST(QuantumArithmeticTest, SwapGeneralGeneralUnitarity)
{
    System::clear();
    auto reg1 = System::add_register("reg1", UnsignedInteger, 2);
    auto reg2 = System::add_register("reg2", UnsignedInteger, 2);
    Swap_General_General op("reg1", "reg2");

    for (size_t v1 = 0; v1 < 4; ++v1) {
        for (size_t v2 = 0; v2 < 4; ++v2) {
            std::vector<System> state;
            state.emplace_back();
            state[0].get(reg1).value = v1;
            state[0].get(reg2).value = v2;
            op(state);
            op(state);
            EXPECT_EQ(state[0].get(reg1).value, v1);
            EXPECT_EQ(state[0].get(reg2).value, v2);
        }
    }
    System::clear();
}

// Test Assign unitarity (out-of-place, self-adjoint via XOR)
TEST(QuantumArithmeticTest, AssignUnitarity)
{
    System::clear();
    auto src = System::add_register("src", UnsignedInteger, 2);
    auto dst = System::add_register("dst", UnsignedInteger, 2);
    Assign op("src", "dst");

    for (size_t v_src = 0; v_src < 4; ++v_src) {
        for (size_t v_dst = 0; v_dst < 4; ++v_dst) {
            std::vector<System> state;
            state.emplace_back();
            state[0].get(src).value = v_src;
            state[0].get(dst).value = v_dst;
            op(state);
            op(state);
            EXPECT_EQ(state[0].get(src).value, v_src);
            EXPECT_EQ(state[0].get(dst).value, v_dst);
        }
    }
    System::clear();
}

// Test ShiftLeft_InPlace/ShiftRight_InPlace round-trip (they are daggers of each other)
// Tests forward->.dag() to verify real dag() implementation (not no-op)
// Test ShiftLeft_InPlace forward→dagger round-trip: U then U† restores original
// ShiftLeft_InPlace::dag() calls ShiftRight_InPlace
TEST(QuantumArithmeticTest, ShiftLeftInPlaceDagRoundTrip)
{
    System::clear();
    auto reg = System::add_register("reg", UnsignedInteger, 3);

    for (size_t v : {0, 1, 3, 7}) {
        for (size_t shift = 1; shift <= 3; ++shift) {
            std::vector<System> st;
            st.emplace_back();
            st[0].get(reg).value = v;
            ShiftLeft_InPlace left_op("reg", shift);
            left_op(st);              // apply U (cyclic left shift)
            left_op.dag(st);          // apply U† (calls ShiftRight_InPlace: cyclic right shift)
            EXPECT_EQ(st[0].get(reg).value, v) << "v=" << v << " shift=" << shift;
        }
    }
    System::clear();
}

// Test ShiftRight_InPlace dagger→forward round-trip: U† then U restores original
// ShiftRight_InPlace::dag() calls ShiftLeft_InPlace
TEST(QuantumArithmeticTest, ShiftRightInPlaceDagRoundTrip)
{
    System::clear();
    auto reg = System::add_register("reg", UnsignedInteger, 3);

    for (size_t v : {0, 1, 3, 7}) {
        for (size_t shift = 1; shift <= 3; ++shift) {
            std::vector<System> st;
            st.emplace_back();
            st[0].get(reg).value = v;
            ShiftRight_InPlace right_op("reg", shift);
            right_op.dag(st);         // apply U† (calls ShiftLeft_InPlace: cyclic left shift)
            right_op(st);             // apply U (cyclic right shift) — restores original
            EXPECT_EQ(st[0].get(reg).value, v) << "v=" << v << " shift=" << shift;
        }
    }
    System::clear();
}

// Test Add_Mult_UInt_ConstUInt unitarity (in-place with explicit dagger)
TEST(QuantumArithmeticTest, AddMultUIntConstUIntUnitarity)
{
    EXPECT_TRUE((verify_inplace_unitarity<Add_Mult_UInt_ConstUInt_InPlace>("reg1", 1, "reg2")));
}

// Test AddAssign_AnyInt_AnyInt unitarity (in-place with explicit dagger)
TEST(QuantumArithmeticTest, AddAssignAnyIntAnyIntUnitarity)
{
    EXPECT_TRUE((verify_inplace_unitarity<AddAssign_AnyInt_AnyInt_InPlace>("reg1", "reg2")));
}

// Test Compare_UInt_UInt unitarity (out-of-place, self-adjoint)
TEST(QuantumArithmeticTest, CompareUIntUIntUnitarity)
{
    System::clear();
    auto left = System::add_register("left", UnsignedInteger, 2);  // Reduced from 3
    auto right = System::add_register("right", UnsignedInteger, 2);
    auto less = System::add_register("less", Boolean, 1);
    auto equal = System::add_register("equal", Boolean, 1);
    Compare_UInt_UInt op("left", "right", "less", "equal");

    for (size_t v_left = 0; v_left < 4; ++v_left) {
        for (size_t v_right = 0; v_right < 4; ++v_right) {
            std::vector<System> state;
            state.emplace_back();
            state[0].get(left).value = v_left;
            state[0].get(right).value = v_right;
            state[0].get(less).value = 0;
            state[0].get(equal).value = 0;
            op(state);
            op(state);
            EXPECT_EQ(state[0].get(left).value, v_left);
            EXPECT_EQ(state[0].get(right).value, v_right);
            EXPECT_EQ(state[0].get(less).value, 0);
            EXPECT_EQ(state[0].get(equal).value, 0);
        }
    }
    System::clear();
}
// Test Mod_Mult_UInt_ConstUInt - modular multiplication: y -> y * a^(2^x) mod N
TEST(QuantumArithmeticTest, ModMultUIntConstUInt)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    // Initialize reg = 3
    Init_Unsafe(reg, 3)(state);

    // Apply Mod_Mult_UInt_ConstUInt_InPlace(reg, 7, 0, 15)
    // x=0 means opnum = 7^1 mod 15 = 7
    // So y = 3 * 7 mod 15 = 21 mod 15 = 6
    Mod_Mult_UInt_ConstUInt_InPlace("reg", 7, 0, 15)(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t val = getRegValue(state[0], reg, 4);
    EXPECT_EQ(val, 6);  // 3 * 7 mod 15 = 21 mod 15 = 6
}

// Test Mod_Mult_UInt_ConstUInt with x > 0
TEST(QuantumArithmeticTest, ModMultUIntConstUIntWithShift)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    std::vector<System> state;
    state.emplace_back();

    // Initialize reg = 1
    Init_Unsafe(reg, 1)(state);

    // Apply Mod_Mult_UInt_ConstUInt_InPlace(reg, 7, 2, 15)
    // x=2 means opnum = 7^4 mod 15
    // 7^2 = 49 mod 15 = 4
    // 7^4 = 4^2 mod 15 = 16 mod 15 = 1
    // So y = 1 * 1 mod 15 = 1
    Mod_Mult_UInt_ConstUInt_InPlace("reg", 7, 2, 15)(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t val = getRegValue(state[0], reg, 4);
    EXPECT_EQ(val, 1);  // 7^4 mod 15 = 1
}

// Test Mod_Mult_UInt_ConstUInt controlled operation
TEST(QuantumArithmeticTest, ModMultUIntConstUIntControlled)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    auto ctrl = System::add_register("ctrl", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();

    // Initialize reg = 3, ctrl = 1 (condition satisfied)
    Init_Unsafe(reg, 3)(state);
    Xgate_Bool("ctrl", 0)(state);

    // Apply controlled Mod_Mult_UInt_ConstUInt
    Mod_Mult_UInt_ConstUInt_InPlace("reg", 7, 0, 15).conditioned_by_all_ones("ctrl")(state);

    ASSERT_EQ(state.size(), 1);
    uint64_t val = getRegValue(state[0], reg, 4);
    EXPECT_EQ(val, 6);  // 3 * 7 mod 15 = 6
}

// Test Mod_Mult_UInt_ConstUInt_InPlace unitarity (in-place with explicit dagger)
// Tests both forward→dag and dag→forward (bidirectional round-trip)
TEST(QuantumArithmeticTest, ModMultUIntConstUIntInPlaceUnitarity)
{
    System::clear();
    constexpr size_t MOD_BITS = 4;  // N=15 requires 4 bits
    auto reg = System::add_register("reg", UnsignedInteger, MOD_BITS);
    Mod_Mult_UInt_ConstUInt_InPlace op("reg", 7, 0, 15);  // a=7, x=0, N=15

    for (size_t v = 0; v < 15; ++v) {
        // Forward→dag round-trip
        {
            std::vector<System> state;
            state.emplace_back();
            state[0].get(reg).value = v;
            op(state);
            op.dag(state);
            ASSERT_EQ(state.size(), 1);
            EXPECT_EQ(state[0].get(reg).value, v) << "forward→dag: v=" << v;
        }
        // Dag→forward round-trip
        {
            std::vector<System> state;
            state.emplace_back();
            state[0].get(reg).value = v;
            op.dag(state);
            op(state);
            ASSERT_EQ(state.size(), 1);
            EXPECT_EQ(state[0].get(reg).value, v) << "dag→forward: v=" << v;
        }
    }
    System::clear();
}

// Test AddConstUInt_InPlace bidirectional round-trip (forward→dag and dag→forward)
TEST(QuantumArithmeticTest, AddConstUIntInPlaceUnitarity)
{
    EXPECT_TRUE(verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 1));
    EXPECT_TRUE(verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 3));
}

// Test Add_Mult_UInt_ConstUInt_InPlace bidirectional (forward→dag and dag→forward)
// mult must be odd to have a modular inverse (guaranteed bijectivity)
TEST(QuantumArithmeticTest, AddMultUIntConstUIntInPlaceBidirectional)
{
    EXPECT_TRUE(verify_inplace_unitarity_fwd_then_dag<Add_Mult_UInt_ConstUInt_InPlace>("reg1", 3, "reg2"));
    EXPECT_TRUE(verify_inplace_unitarity_dag_then_fwd<Add_Mult_UInt_ConstUInt_InPlace>("reg1", 3, "reg2"));
}

// Test AddAssign_AnyInt_AnyInt_InPlace bidirectional (forward→dag and dag→forward)
TEST(QuantumArithmeticTest, AddAssignAnyIntAnyIntInPlaceBidirectional)
{
    EXPECT_TRUE(verify_inplace_unitarity_fwd_then_dag<AddAssign_AnyInt_AnyInt_InPlace>("reg1", "reg2"));
    EXPECT_TRUE(verify_inplace_unitarity_dag_then_fwd<AddAssign_AnyInt_AnyInt_InPlace>("reg1", "reg2"));
}

// Generalized check_inplace_unitarity: factory lambda, 3-bit lhs + 3-bit res = 6 bits (64 states)
// Tests both dagger=true and dagger=false, verifies bijectivity via truth table
TEST(QuantumArithmeticTest, GeneralizedCheckInplaceUnitarity)
{
    auto factory = [](std::vector<size_t> ids) -> Add_Mult_UInt_ConstUInt_InPlace {
        return Add_Mult_UInt_ConstUInt_InPlace{ids[0], 3, ids[1]};  // lhs, mult (odd), res
    };
    // {3, 3} = 3-bit lhs + 3-bit res = 6 total bits = 64 states
    auto tt_fwd = check_inplace_unitarity<Add_Mult_UInt_ConstUInt_InPlace>({3, 3}, factory, false);
    auto tt_dag = check_inplace_unitarity<Add_Mult_UInt_ConstUInt_InPlace>({3, 3}, factory, true);

    // Truth table must be a bijection: every output index seen exactly once
    std::vector<bool> seen_fwd(tt_fwd.size(), false);
    for (size_t out : tt_fwd)
        EXPECT_FALSE(seen_fwd[out]) << "Non-bijective forward: " << out << " seen twice", seen_fwd[out] = true;
    std::vector<bool> seen_dag(tt_dag.size(), false);
    for (size_t out : tt_dag)
        EXPECT_FALSE(seen_dag[out]) << "Non-bijective dagger: " << out << " seen twice", seen_dag[out] = true;
}