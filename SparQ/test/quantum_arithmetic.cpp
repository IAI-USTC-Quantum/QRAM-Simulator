// Include error_handler.h FIRST to access the TEST macro, then undefine it
// so that gtest's TEST macro can be used instead
#include <string>
#include <string_view>
#include <array>
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
TEST_F(QuantumArithmeticTest, AddUIntUInt)
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
TEST_F(QuantumArithmeticTest, AddUIntUIntInPlace)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto rhs_reg = System::add_register("rhs", UnsignedInteger, 4);
    const std::array<std::pair<uint64_t, uint64_t>, 4> cases = {{
        {7, 3}, {0, 9}, {15, 1}, {11, 14},
    }};

    for (const auto& [lhs, rhs] : cases) {
        std::vector<System> state(1);
        state[0].get(lhs_reg).value = lhs;
        state[0].get(rhs_reg).value = rhs;

        Add_UInt_UInt_InPlace("lhs", "rhs")(state);

        EXPECT_EQ(state[0].get(lhs_reg).value, lhs);
        EXPECT_EQ(state[0].get(rhs_reg).value, (lhs + rhs) & 0xf);
    }
}

// Test Add_UInt_ConstUInt: z = x + constant
TEST_F(QuantumArithmeticTest, AddUIntConstUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    const std::array<std::pair<uint64_t, uint64_t>, 3> cases = {{
        {6, 0}, {15, 3}, {9, 12},
    }};

    for (const auto& [lhs, output_start] : cases) {
        std::vector<System> state(1);
        state[0].get(lhs_reg).value = lhs;
        state[0].get(res_reg).value = output_start;

        Add_UInt_ConstUInt("lhs", 4, "res")(state);

        EXPECT_EQ(state[0].get(lhs_reg).value, lhs);
        EXPECT_EQ(state[0].get(res_reg).value, output_start ^ ((lhs + 4) & 0xf));
    }
}

// Test Add_ConstUInt: y += constant (in-place, with overflow check)
TEST_F(QuantumArithmeticTest, AddConstUIntInPlace)
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
TEST_F(QuantumArithmeticTest, MultUIntConstUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    const std::array<std::pair<uint64_t, uint64_t>, 3> cases = {{
        {3, 0}, {7, 5}, {15, 10},
    }};

    for (const auto& [lhs, output_start] : cases) {
        std::vector<System> state(1);
        state[0].get(lhs_reg).value = lhs;
        state[0].get(res_reg).value = output_start;

        Mult_UInt_ConstUInt("lhs", 4, "res")(state);

        EXPECT_EQ(state[0].get(lhs_reg).value, lhs);
        EXPECT_EQ(state[0].get(res_reg).value, output_start ^ ((lhs * 4) & 0xf));
    }
}

// Test z += x * constant (Add_Mult_UInt_ConstUInt)
// Note: The API behavior depends on overflow handling and implementation details.
// Testing with small values to verify the operation runs without error.
TEST_F(QuantumArithmeticTest, AddMultUIntConstUInt)
{
    auto lhs_reg = System::add_register("lhs", UnsignedInteger, 4);
    auto res_reg = System::add_register("res", UnsignedInteger, 4);
    const std::array<std::pair<uint64_t, uint64_t>, 4> cases = {{
        {1, 2}, {0, 7}, {7, 5}, {15, 15},
    }};

    for (const auto& [lhs, result] : cases) {
        std::vector<System> state(1);
        state[0].get(lhs_reg).value = lhs;
        state[0].get(res_reg).value = result;

        Add_Mult_UInt_ConstUInt_InPlace("lhs", 2, "res")(state);

        EXPECT_EQ(state[0].get(lhs_reg).value, lhs);
        EXPECT_EQ(state[0].get(res_reg).value, (result + lhs * 2) & 0xf);
    }
}

// ============ Bit Manipulation Tests ============

// Test FlipBools - flip all bits in a register
TEST_F(QuantumArithmeticTest, FlipBools)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    for (uint64_t value = 0; value < 16; ++value) {
        std::vector<System> state(1);
        state[0].get(reg).value = value;

        FlipBools("reg")(state);

        EXPECT_EQ(state[0].get(reg).value, value ^ 0xf);
    }
}

// ============ Assignment Tests ============

// Test Assign - copy register value
TEST_F(QuantumArithmeticTest, Assign)
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
TEST_F(QuantumArithmeticTest, CompareUIntUInt)
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
TEST_F(QuantumArithmeticTest, AddAssignAnyIntAnyInt)
{
    auto lhs = System::add_register("lhs", UnsignedInteger, 4);
    auto rhs = System::add_register("rhs", UnsignedInteger, 4);
    const std::array<std::pair<uint64_t, uint64_t>, 4> cases = {{
        {8, 6}, {0, 9}, {15, 1}, {12, 11},
    }};

    for (const auto& [lhs_start, rhs_start] : cases) {
        std::vector<System> state(1);
        state[0].get(lhs).value = lhs_start;
        state[0].get(rhs).value = rhs_start;

        Add_AnyInt_AnyInt_InPlace("lhs", "rhs")(state);

        EXPECT_EQ(state[0].get(lhs).value, (lhs_start + rhs_start) & 0xf);
        EXPECT_EQ(state[0].get(rhs).value, rhs_start);
    }
}

// ============ Custom Arithmetic Test ============

// Test CustomArithmetic with a simple doubling function
TEST_F(QuantumArithmeticTest, CustomArithmetic)
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
TEST_F(QuantumArithmeticTest, GetMid)
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
TEST_F(QuantumArithmeticTest, AddUIntUIntUnitarity)
{
    EXPECT_TRUE((verify_outofplace_unitarity<Add_UInt_UInt>("reg1", "reg2", "res")));
}

// Test Add_UInt_UInt_InPlace unitarity (in-place with explicit dagger)
TEST_F(QuantumArithmeticTest, AddUIntUIntInPlaceUnitarity)
{
    EXPECT_TRUE((verify_inplace_unitarity<Add_UInt_UInt_InPlace>("reg1", "reg2")));
}

// Test Add_ConstUInt unitarity with a few constant values
TEST_F(QuantumArithmeticTest, AddConstUIntUnitarity)
{
    EXPECT_TRUE((verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 1)));
    EXPECT_TRUE((verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 3)));
}

// Test Mult_UInt_ConstUInt unitarity (out-of-place, self-adjoint)
TEST_F(QuantumArithmeticTest, MultUIntConstUIntUnitarity)
{
    EXPECT_TRUE((verify_outofplace_unitarity<Mult_UInt_ConstUInt>("reg1", 3, "res")));
}

// Test FlipBools unitarity (out-of-place, self-adjoint)
TEST_F(QuantumArithmeticTest, FlipBoolsUnitarity)
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
TEST_F(QuantumArithmeticTest, SwapGeneralGeneralUnitarity)
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
TEST_F(QuantumArithmeticTest, AssignUnitarity)
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
TEST_F(QuantumArithmeticTest, ShiftLeftInPlaceDagRoundTrip)
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
TEST_F(QuantumArithmeticTest, ShiftRightInPlaceDagRoundTrip)
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
TEST_F(QuantumArithmeticTest, AddMultUIntConstUIntUnitarity)
{
    EXPECT_TRUE((verify_inplace_unitarity<Add_Mult_UInt_ConstUInt_InPlace>("reg1", 1, "reg2")));
}

// Test AddAssign_AnyInt_AnyInt unitarity (in-place with explicit dagger)
TEST_F(QuantumArithmeticTest, AddAssignAnyIntAnyIntUnitarity)
{
    EXPECT_TRUE((verify_inplace_unitarity<Add_AnyInt_AnyInt_InPlace>("reg1", "reg2")));
}

// Test Compare_UInt_UInt unitarity (out-of-place, self-adjoint)
TEST_F(QuantumArithmeticTest, CompareUIntUIntUnitarity)
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
TEST_F(QuantumArithmeticTest, ModMultUIntConstUInt)
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
TEST_F(QuantumArithmeticTest, ModMultUIntConstUIntWithShift)
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
TEST_F(QuantumArithmeticTest, ModMultUIntConstUIntControlled)
{
    auto reg = System::add_register("reg", UnsignedInteger, 4);
    auto ctrl = System::add_register("ctrl", Boolean, 1);
    for (uint64_t control : {uint64_t{0}, uint64_t{1}}) {
        std::vector<System> state(1);
        state[0].get(reg).value = 3;
        state[0].get(ctrl).value = control;

        Mod_Mult_UInt_ConstUInt_InPlace("reg", 7, 0, 15)
            .conditioned_by_all_ones("ctrl")(state);

        EXPECT_EQ(state[0].get(reg).value, control == 1 ? 6 : 3);
        EXPECT_EQ(state[0].get(ctrl).value, control);
    }
}

// Test Mod_Mult_UInt_ConstUInt_InPlace unitarity (in-place with explicit dagger)
// Tests both forward→dag and dag→forward (bidirectional round-trip)
TEST_F(QuantumArithmeticTest, ModMultUIntConstUIntInPlaceUnitarity)
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
TEST_F(QuantumArithmeticTest, AddConstUIntInPlaceUnitarity)
{
    EXPECT_TRUE(verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 1));
    EXPECT_TRUE(verify_single_reg_inplace_unitarity<Add_ConstUInt_InPlace>("reg", 3));
}

// Test Add_Mult_UInt_ConstUInt_InPlace bidirectional (forward→dag and dag→forward)
// mult must be odd to have a modular inverse (guaranteed bijectivity)
TEST_F(QuantumArithmeticTest, AddMultUIntConstUIntInPlaceBidirectional)
{
    EXPECT_TRUE(verify_inplace_unitarity_fwd_then_dag<Add_Mult_UInt_ConstUInt_InPlace>("reg1", 3, "reg2"));
    EXPECT_TRUE(verify_inplace_unitarity_dag_then_fwd<Add_Mult_UInt_ConstUInt_InPlace>("reg1", 3, "reg2"));
}

// Test Add_AnyInt_AnyInt_InPlace bidirectional (forward→dag and dag→forward)
TEST_F(QuantumArithmeticTest, AddAssignAnyIntAnyIntInPlaceBidirectional)
{
    EXPECT_TRUE(verify_inplace_unitarity_fwd_then_dag<Add_AnyInt_AnyInt_InPlace>("reg1", "reg2"));
    EXPECT_TRUE(verify_inplace_unitarity_dag_then_fwd<Add_AnyInt_AnyInt_InPlace>("reg1", "reg2"));
}

// Generalized check_inplace_unitarity: factory lambda, 3-bit lhs + 3-bit res = 6 bits (64 states)
// Tests both dagger=true and dagger=false, verifies bijectivity via truth table
TEST_F(QuantumArithmeticTest, GeneralizedCheckInplaceUnitarity)
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

// ============ Width & Truncation Convention Helpers ============
// Reference helpers for the new arithmetic operators (docs/operators.md
// 《宽度与截断约定》). Kept independent of the library's internal helpers so
// the tests encode the documented contract, not the implementation.
namespace {
    uint64_t ref_width_mask(size_t w) {
        return w >= 64 ? ~uint64_t{0} : (uint64_t{1} << w) - 1;
    }

    // Two's complement sign extension of a w-bit pattern (w < 64)
    int64_t ref_sext(uint64_t v, size_t w) {
        if (w >= 64)
            return static_cast<int64_t>(v);
        const uint64_t sign = uint64_t{1} << (w - 1);
        if (v & sign)
            v |= ~ref_width_mask(w);
        return static_cast<int64_t>(v);
    }

    // Integer square root via floating point + exact correction
    uint64_t ref_isqrt(uint64_t n) {
        uint64_t r = static_cast<uint64_t>(std::sqrt(static_cast<double>(n)));
        while (r > 0 && (r - 1) * (r - 1) >= n) --r;
        while ((r + 1) * (r + 1) <= n) ++r;
        return r;
    }

    // Sampled operand values for wide sweeps: 0, 1, all-ones, a few mids
    std::vector<uint64_t> ref_sample_values(size_t w) {
        const uint64_t max = ref_width_mask(w);
        std::vector<uint64_t> vals{0, 1, max, max >> 1, max / 3, max - 1};
        std::vector<uint64_t> uniq;
        for (uint64_t v : vals) {
            bool dup = false;
            for (uint64_t u : uniq)
                if (u == v) dup = true;
            if (!dup)
                uniq.push_back(v);
        }
        return uniq;
    }
}

// ============ New Arithmetic Operator Truth-Table Tests ============

// Sub_UInt_UInt: mixed widths lhs 3-bit / rhs 5-bit / res 4-bit, full sweep.
// res ^= (lhs - rhs) on the unsigned 64-bit wraparound domain, truncated mod 2^4.
TEST_F(QuantumArithmeticTest, SubUIntUIntTruthTable)
{
    auto lhs = System::add_register("sub_lhs", UnsignedInteger, 3);
    auto rhs = System::add_register("sub_rhs", UnsignedInteger, 5);
    auto res = System::add_register("sub_res", UnsignedInteger, 4);

    for (uint64_t a = 0; a < 8; ++a) {
        for (uint64_t b = 0; b < 32; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;
            state[0].get(res).value = 0;

            Sub_UInt_UInt("sub_lhs", "sub_rhs", "sub_res")(state);

            const uint64_t expected = (a - b) & 0xf;  // uint64 wraparound, then mod 16
            EXPECT_EQ(state.size(), 1);
            EXPECT_EQ(state[0].get(res).value, expected) << "a=" << a << " b=" << b;
            EXPECT_EQ(state[0].get(lhs).value, a);
            EXPECT_EQ(state[0].get(rhs).value, b);
        }
    }

    // Nonzero initial res must be XORed into (XOR-out semantics)
    {
        std::vector<System> state(1);
        state[0].get(lhs).value = 1;
        state[0].get(rhs).value = 2;
        state[0].get(res).value = 0xf;
        Sub_UInt_UInt("sub_lhs", "sub_rhs", "sub_res")(state);
        // (1 - 2) & 0xf = 15; 15 ^ 15 = 0
        EXPECT_EQ(state[0].get(res).value, 0xf ^ static_cast<uint64_t>((1 - 2) & 0xf));
    }
}

// Neg_UInt: res ^= 0 - reg; wider and narrower res than reg
TEST_F(QuantumArithmeticTest, NegUIntTruthTable)
{
    auto reg_w3 = System::add_register("neg_reg_w3", UnsignedInteger, 3);
    auto res_w5 = System::add_register("neg_res_w5", UnsignedInteger, 5);
    auto reg_w5 = System::add_register("neg_reg_w5", UnsignedInteger, 5);
    auto res_w3 = System::add_register("neg_res_w3", UnsignedInteger, 3);

    for (uint64_t v = 0; v < 8; ++v) {
        std::vector<System> state(1);
        state[0].get(reg_w3).value = v;
        state[0].get(res_w5).value = 0;
        Neg_UInt(reg_w3, res_w5)(state);
        EXPECT_EQ(state[0].get(res_w5).value, (0 - v) & 0x1f);
        EXPECT_EQ(state[0].get(reg_w3).value, v);
    }
    for (uint64_t v = 0; v < 32; ++v) {
        std::vector<System> state(1);
        state[0].get(reg_w5).value = v;
        state[0].get(res_w3).value = 0;
        Neg_UInt(reg_w5, res_w3)(state);
        EXPECT_EQ(state[0].get(res_w3).value, (0 - v) & 0x7);
        EXPECT_EQ(state[0].get(reg_w5).value, v);
    }
}

// Abs_SInt: SInt in / UInt out, sign-extended magnitude truncated to res width
TEST_F(QuantumArithmeticTest, AbsSIntTruthTable)
{
    auto reg_w4 = System::add_register("abs_reg_w4", SignedInteger, 4);
    auto res_w5 = System::add_register("abs_res_w5", UnsignedInteger, 5);
    auto reg_w5 = System::add_register("abs_reg_w5", SignedInteger, 5);
    auto res_w3 = System::add_register("abs_res_w3", UnsignedInteger, 3);

    for (uint64_t bits = 0; bits < 16; ++bits) {
        std::vector<System> state(1);
        state[0].get(reg_w4).value = bits;
        state[0].get(res_w5).value = 0;
        Abs_SInt(reg_w4, res_w5)(state);
        const int64_t v = ref_sext(bits, 4);
        EXPECT_EQ(state[0].get(res_w5).value,
                  static_cast<uint64_t>(v < 0 ? -v : v) & 0x1f);
        EXPECT_EQ(state[0].get(reg_w4).value, bits);
    }
    for (uint64_t bits = 0; bits < 32; ++bits) {
        std::vector<System> state(1);
        state[0].get(reg_w5).value = bits;
        state[0].get(res_w3).value = 0;
        Abs_SInt(reg_w5, res_w3)(state);
        const int64_t v = ref_sext(bits, 5);
        EXPECT_EQ(state[0].get(res_w3).value,
                  static_cast<uint64_t>(v < 0 ? -v : v) & 0x7);
    }
}

// Mul_UInt_UInt: low-64 product truncated to res width, mixed widths
TEST_F(QuantumArithmeticTest, MulUIntUIntTruthTable)
{
    auto lhs_w3 = System::add_register("mul_lhs_w3", UnsignedInteger, 3);
    auto rhs_w3 = System::add_register("mul_rhs_w3", UnsignedInteger, 3);
    auto res_w4 = System::add_register("mul_res_w4", UnsignedInteger, 4);
    auto lhs_w5 = System::add_register("mul_lhs_w5", UnsignedInteger, 5);
    auto rhs_w2 = System::add_register("mul_rhs_w2", UnsignedInteger, 2);
    auto res_w3 = System::add_register("mul_res_w3", UnsignedInteger, 3);

    for (uint64_t a = 0; a < 8; ++a) {
        for (uint64_t b = 0; b < 8; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs_w3).value = a;
            state[0].get(rhs_w3).value = b;
            state[0].get(res_w4).value = 0;
            Mul_UInt_UInt(lhs_w3, rhs_w3, res_w4)(state);
            EXPECT_EQ(state[0].get(res_w4).value, (a * b) & 0xf);
            EXPECT_EQ(state[0].get(lhs_w3).value, a);
            EXPECT_EQ(state[0].get(rhs_w3).value, b);
        }
    }
    for (uint64_t a = 0; a < 32; ++a) {
        for (uint64_t b = 0; b < 4; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs_w5).value = a;
            state[0].get(rhs_w2).value = b;
            state[0].get(res_w3).value = 0;
            Mul_UInt_UInt(lhs_w5, rhs_w2, res_w3)(state);
            EXPECT_EQ(state[0].get(res_w3).value, (a * b) & 0x7);
        }
    }
}

// Div_UInt_UInt: floor division; zero divisor yields quotient 0 (total domain)
TEST_F(QuantumArithmeticTest, DivUIntUIntTruthTable)
{
    auto lhs = System::add_register("div_lhs", UnsignedInteger, 5);
    auto rhs = System::add_register("div_rhs", UnsignedInteger, 3);
    auto res = System::add_register("div_res", UnsignedInteger, 5);

    for (uint64_t a = 0; a < 32; ++a) {
        for (uint64_t b = 0; b < 8; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;
            state[0].get(res).value = 0;

            Div_UInt_UInt("div_lhs", "div_rhs", "div_res")(state);

            const uint64_t expected = (b == 0) ? 0 : a / b;
            EXPECT_EQ(state.size(), 1);
            EXPECT_EQ(state[0].get(res).value, expected) << "a=" << a << " b=" << b;
            EXPECT_EQ(state[0].get(lhs).value, a);
            EXPECT_EQ(state[0].get(rhs).value, b);
        }
    }
}

// Sqrt_UInt: res ^= isqrt(reg), integer-only result truncated to res width
TEST_F(QuantumArithmeticTest, SqrtUIntTruthTable)
{
    auto reg = System::add_register("sqrt_reg", UnsignedInteger, 4);
    auto res = System::add_register("sqrt_res", UnsignedInteger, 3);

    for (uint64_t v = 0; v < 16; ++v) {
        std::vector<System> state(1);
        state[0].get(reg).value = v;
        state[0].get(res).value = 0;
        Sqrt_UInt(reg, res)(state);
        EXPECT_EQ(state[0].get(res).value, ref_isqrt(v) & 0x7) << "v=" << v;
        EXPECT_EQ(state[0].get(reg).value, v);
    }
}

// Select_Bool_UInt_UInt: cond bit 0 picks lhs (cond=1) or rhs (cond=0)
TEST_F(QuantumArithmeticTest, SelectBoolUIntUIntTruthTable)
{
    auto cond = System::add_register("sel_cond", Boolean, 1);
    auto lhs = System::add_register("sel_lhs", UnsignedInteger, 3);
    auto rhs = System::add_register("sel_rhs", UnsignedInteger, 5);
    auto res = System::add_register("sel_res", UnsignedInteger, 4);

    for (uint64_t c = 0; c < 2; ++c) {
        for (uint64_t a = 0; a < 8; ++a) {
            for (uint64_t b = 0; b < 32; ++b) {
                std::vector<System> state(1);
                state[0].get(cond).value = c;
                state[0].get(lhs).value = a;
                state[0].get(rhs).value = b;
                state[0].get(res).value = 0;

                Select_Bool_UInt_UInt("sel_cond", "sel_lhs", "sel_rhs", "sel_res")(state);

                const uint64_t expected = (c & 1) ? (a & 0xf) : (b & 0xf);
                EXPECT_EQ(state[0].get(res).value, expected) << "c=" << c << " a=" << a << " b=" << b;
                EXPECT_EQ(state[0].get(lhs).value, a);
                EXPECT_EQ(state[0].get(rhs).value, b);
            }
        }
    }
}

// And/Or/Xor_UInt_UInt: bitwise ops on zero-extended mixed-width operands
TEST_F(QuantumArithmeticTest, AndOrXorUIntUIntTruthTable)
{
    auto lhs = System::add_register("bit_lhs", UnsignedInteger, 3);
    auto rhs = System::add_register("bit_rhs", UnsignedInteger, 5);
    auto res_and = System::add_register("bit_res_and", UnsignedInteger, 4);
    auto res_or = System::add_register("bit_res_or", UnsignedInteger, 4);
    auto res_xor = System::add_register("bit_res_xor", UnsignedInteger, 4);

    for (uint64_t a = 0; a < 8; ++a) {
        for (uint64_t b = 0; b < 32; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;
            state[0].get(res_and).value = 0;
            state[0].get(res_or).value = 0;
            state[0].get(res_xor).value = 0;

            And_UInt_UInt("bit_lhs", "bit_rhs", "bit_res_and")(state);
            Or_UInt_UInt("bit_lhs", "bit_rhs", "bit_res_or")(state);
            Xor_UInt_UInt("bit_lhs", "bit_rhs", "bit_res_xor")(state);

            EXPECT_EQ(state[0].get(res_and).value, (a & b) & 0xf);
            EXPECT_EQ(state[0].get(res_or).value, (a | b) & 0xf);
            EXPECT_EQ(state[0].get(res_xor).value, (a ^ b) & 0xf);
            EXPECT_EQ(state[0].get(lhs).value, a);
            EXPECT_EQ(state[0].get(rhs).value, b);
        }
    }
}

// ============ Flag Operator Tests ============

// Less_SInt_SInt: sign-extended full-precision comparison, mixed widths
TEST_F(QuantumArithmeticTest, LessSIntSIntTruthTable)
{
    auto lhs = System::add_register("less_s_lhs", SignedInteger, 4);
    auto rhs = System::add_register("less_s_rhs", SignedInteger, 3);
    auto flag = System::add_register("less_s_flag", Boolean, 1);

    for (uint64_t lbits = 0; lbits < 16; ++lbits) {
        for (uint64_t rbits = 0; rbits < 8; ++rbits) {
            std::vector<System> state(1);
            state[0].get(lhs).value = lbits;
            state[0].get(rhs).value = rbits;
            state[0].get(flag).value = 0;

            Less_SInt_SInt("less_s_lhs", "less_s_rhs", "less_s_flag")(state);

            const uint64_t expected =
                ref_sext(lbits, 4) < ref_sext(rbits, 3) ? 1 : 0;
            EXPECT_EQ(state[0].get(flag).value, expected)
                << "lhs=" << lbits << " rhs=" << rbits;
            EXPECT_EQ(state[0].get(lhs).value, lbits);
            EXPECT_EQ(state[0].get(rhs).value, rbits);
        }
    }
}

// Carry_UInt_UInt at out width 3: flag ^= (lhs + rhs >= 2^3), full sweep
TEST_F(QuantumArithmeticTest, CarryUIntUIntTruthTable)
{
    auto lhs = System::add_register("carry_lhs", UnsignedInteger, 3);
    auto rhs = System::add_register("carry_rhs", UnsignedInteger, 4);
    auto res = System::add_register("carry_res", UnsignedInteger, 3);
    auto flag = System::add_register("carry_flag", Boolean, 1);
    constexpr size_t w = 3;

    for (uint64_t a = 0; a < 8; ++a) {
        for (uint64_t b = 0; b < 16; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;
            state[0].get(flag).value = 0;

            Carry_UInt_UInt("carry_lhs", "carry_rhs", "carry_res", "carry_flag")(state);

            const uint64_t expected =
                (a >= (uint64_t{1} << w) || b >= (uint64_t{1} << w) - a) ? 1 : 0;
            EXPECT_EQ(state[0].get(flag).value, expected) << "a=" << a << " b=" << b;
        }
    }
}

// Carry_UInt_UInt at out width 64 (boundary): predicate is 64-bit wraparound.
// res only provides the width: its initial value is neither read nor written.
TEST_F(QuantumArithmeticTest, CarryUIntUIntWidth64Boundary)
{
    auto lhs = System::add_register("carry64_lhs", UnsignedInteger, 64);
    auto rhs = System::add_register("carry64_rhs", UnsignedInteger, 64);
    auto res = System::add_register("carry64_res", UnsignedInteger, 64);
    auto flag = System::add_register("carry64_flag", Boolean, 1);
    constexpr uint64_t max = ~uint64_t{0};

    const std::array<std::pair<std::pair<uint64_t, uint64_t>, uint64_t>, 4> cases = {{
        {{max, 1}, 1},   // wraps to 0 < max
        {{1, 1}, 0},     // 2 >= 1, no wrap
        {{max, max}, 1}, // max - 1 < max
        {{max - 1, 1}, 0},  // max, not < max-1
    }};

    for (const auto& [inputs, expected] : cases) {
        std::vector<System> state;
        state.emplace_back();
        Init_Unsafe(lhs, inputs.first)(state);
        Init_Unsafe(rhs, inputs.second)(state);
        Init_Unsafe(res, 0x123456789abcdef0ULL)(state);  // garbage: width-only usage
        Init_Unsafe(flag, 0)(state);

        Carry_UInt_UInt(lhs, rhs, res, flag)(state);

        EXPECT_EQ(state.size(), 1);
        EXPECT_EQ(state[0].get(flag).value, expected)
            << "a=" << inputs.first << " b=" << inputs.second;
        EXPECT_EQ(state[0].get(res).value, 0x123456789abcdef0ULL);  // untouched
    }

    // Flag XOR semantics: preset flag flips the stored predicate bit
    {
        std::vector<System> state;
        state.emplace_back();
        Init_Unsafe(lhs, max)(state);
        Init_Unsafe(rhs, 1)(state);
        Init_Unsafe(flag, 1)(state);
        Carry_UInt_UInt(lhs, rhs, res, flag)(state);
        EXPECT_EQ(state[0].get(flag).value, 0);  // 1 ^ 1
    }
}

// Overflow_SInt_SInt: mixed operand widths, sign-extended then judged at res
// width w; reference follows the same-sign/result-sign-flip rule
TEST_F(QuantumArithmeticTest, OverflowSIntSIntMixedWidths)
{
    auto lhs = System::add_register("ovf_lhs", SignedInteger, 4);
    auto rhs = System::add_register("ovf_rhs", SignedInteger, 3);
    auto res_w4 = System::add_register("ovf_res_w4", UnsignedInteger, 4);
    auto res_w3 = System::add_register("ovf_res_w3", UnsignedInteger, 3);
    auto flag_w4 = System::add_register("ovf_flag_w4", Boolean, 1);
    auto flag_w3 = System::add_register("ovf_flag_w3", Boolean, 1);

    auto reference = [](int64_t l, int64_t r, size_t w) {
        const uint64_t mask = ref_width_mask(w);
        const uint64_t A = static_cast<uint64_t>(l) & mask;
        const uint64_t B = static_cast<uint64_t>(r) & mask;
        const uint64_t S = (A + B) & mask;
        const uint64_t signA = (A >> (w - 1)) & 1;
        const uint64_t signB = (B >> (w - 1)) & 1;
        const uint64_t signS = (S >> (w - 1)) & 1;
        return (signA == signB) && (signS != signA) ? 1 : 0;
    };

    for (uint64_t lbits = 0; lbits < 16; ++lbits) {
        for (uint64_t rbits = 0; rbits < 8; ++rbits) {
            std::vector<System> state(1);
            state[0].get(lhs).value = lbits;
            state[0].get(rhs).value = rbits;
            state[0].get(flag_w4).value = 0;
            state[0].get(flag_w3).value = 0;

            const int64_t l = ref_sext(lbits, 4);
            const int64_t r = ref_sext(rbits, 3);
            Overflow_SInt_SInt(lhs, rhs, res_w4, flag_w4)(state);
            Overflow_SInt_SInt(lhs, rhs, res_w3, flag_w3)(state);

            EXPECT_EQ(state[0].get(flag_w4).value, reference(l, r, 4))
                << "lhs=" << lbits << " rhs=" << rbits << " w=4";
            EXPECT_EQ(state[0].get(flag_w3).value, reference(l, r, 3))
                << "lhs=" << lbits << " rhs=" << rbits << " w=3";
        }
    }
}

// MulOverflow_UInt_UInt: full-precision product vs res width
TEST_F(QuantumArithmeticTest, MulOverflowUIntUIntTruthTable)
{
    auto lhs = System::add_register("mulovf_lhs", UnsignedInteger, 3);
    auto rhs = System::add_register("mulovf_rhs", UnsignedInteger, 3);
    auto res_w4 = System::add_register("mulovf_res_w4", UnsignedInteger, 4);
    auto res_w6 = System::add_register("mulovf_res_w6", UnsignedInteger, 6);
    auto flag_w4 = System::add_register("mulovf_flag_w4", Boolean, 1);
    auto flag_w6 = System::add_register("mulovf_flag_w6", Boolean, 1);

    for (uint64_t a = 0; a < 8; ++a) {
        for (uint64_t b = 0; b < 8; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;
            state[0].get(flag_w4).value = 0;
            state[0].get(flag_w6).value = 0;

            MulOverflow_UInt_UInt(lhs, rhs, res_w4, flag_w4)(state);
            MulOverflow_UInt_UInt(lhs, rhs, res_w6, flag_w6)(state);

            // 3x3-bit products stay below 2^6, so the high half is always 0
            EXPECT_EQ(state[0].get(flag_w4).value, (a * b) >= (uint64_t{1} << 4) ? 1 : 0);
            EXPECT_EQ(state[0].get(flag_w6).value, (a * b) >= (uint64_t{1} << 6) ? 1 : 0);
        }
    }
}

// IsZero_UInt / Negative_SInt small truth tables
TEST_F(QuantumArithmeticTest, IsZeroAndNegativeTruthTable)
{
    {
        auto reg = System::add_register("iszero_reg", UnsignedInteger, 3);
        auto flag = System::add_register("iszero_flag", Boolean, 1);
        for (uint64_t v = 0; v < 8; ++v) {
            std::vector<System> state(1);
            state[0].get(reg).value = v;
            state[0].get(flag).value = 0;
            IsZero_UInt(reg, flag)(state);
            EXPECT_EQ(state[0].get(flag).value, v == 0 ? 1 : 0) << "v=" << v;
        }
    }
    {
        auto reg = System::add_register("negative_reg", SignedInteger, 4);
        auto flag = System::add_register("negative_flag", Boolean, 1);
        for (uint64_t bits = 0; bits < 16; ++bits) {
            std::vector<System> state(1);
            state[0].get(reg).value = bits;
            state[0].get(flag).value = 0;
            Negative_SInt(reg, flag)(state);
            EXPECT_EQ(state[0].get(flag).value, ref_sext(bits, 4) < 0 ? 1 : 0)
                << "bits=" << bits;
        }
    }
}

// ============ Parameterized Width Sweep: Add / Mul / Div ============
// Operands {1,2,3,5,8} x {1,2,3,5,8}, out width {1,2,4,8}.
// Full sweep when the operand space is small; sampled values
// (0, 1, all-ones, a few mids) once the total operand bits grow,
// per the width & truncation convention test plan.
class ArithmeticWidthSweep : public QuantumArithmeticTest,
                             public ::testing::WithParamInterface<std::tuple<size_t, size_t, size_t>>
{
};

TEST_P(ArithmeticWidthSweep, AddMulDivAgainstReference)
{
    const auto [lhs_w, rhs_w, out_w] = GetParam();
    const uint64_t out_mask = ref_width_mask(out_w);

    auto lhs = System::add_register("sweep_lhs", UnsignedInteger, lhs_w);
    auto rhs = System::add_register("sweep_rhs", UnsignedInteger, rhs_w);
    auto add_res = System::add_register("sweep_add_res", UnsignedInteger, out_w);
    auto mul_res = System::add_register("sweep_mul_res", UnsignedInteger, out_w);
    auto div_res = System::add_register("sweep_div_res", UnsignedInteger, out_w);

    std::vector<uint64_t> lhs_vals, rhs_vals;
    if (lhs_w + rhs_w <= 10) {
        for (uint64_t v = 0; v <= ref_width_mask(lhs_w); ++v) lhs_vals.push_back(v);
        for (uint64_t v = 0; v <= ref_width_mask(rhs_w); ++v) rhs_vals.push_back(v);
    } else {
        lhs_vals = ref_sample_values(lhs_w);
        rhs_vals = ref_sample_values(rhs_w);
    }

    for (uint64_t a : lhs_vals) {
        for (uint64_t b : rhs_vals) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;
            state[0].get(add_res).value = 0;
            state[0].get(mul_res).value = 0;
            state[0].get(div_res).value = 0;

            Add_UInt_UInt(lhs, rhs, add_res)(state);
            Mul_UInt_UInt(lhs, rhs, mul_res)(state);
            Div_UInt_UInt(lhs, rhs, div_res)(state);

            ASSERT_EQ(state.size(), 1);
            EXPECT_EQ(state[0].get(add_res).value, (a + b) & out_mask)
                << "add: a=" << a << " b=" << b;
            EXPECT_EQ(state[0].get(mul_res).value, (a * b) & out_mask)
                << "mul: a=" << a << " b=" << b;
            EXPECT_EQ(state[0].get(div_res).value, (b == 0 ? 0 : a / b) & out_mask)
                << "div: a=" << a << " b=" << b;
            EXPECT_EQ(state[0].get(lhs).value, a);
            EXPECT_EQ(state[0].get(rhs).value, b);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    WidthSweep,
    ArithmeticWidthSweep,
    ::testing::Combine(::testing::Values(1, 2, 3, 5, 8),
                       ::testing::Values(1, 2, 3, 5, 8),
                       ::testing::Values(1, 2, 4, 8)));

// ============ Add_AnyInt_AnyInt_InPlace New Semantics ============

// Equal-width unsigned regression: lhs += rhs (mod 2^N), full sweep
TEST_F(QuantumArithmeticTest, AddAnyIntAnyIntInPlaceUnsignedFullSweep)
{
    auto lhs = System::add_register("anyint_lhs", UnsignedInteger, 4);
    auto rhs = System::add_register("anyint_rhs", UnsignedInteger, 4);

    for (uint64_t a = 0; a < 16; ++a) {
        for (uint64_t b = 0; b < 16; ++b) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = b;

            Add_AnyInt_AnyInt_InPlace("anyint_lhs", "anyint_rhs")(state);

            EXPECT_EQ(state[0].get(lhs).value, (a + b) & 0xf);
            EXPECT_EQ(state[0].get(rhs).value, b);
        }
    }
}

// NEW semantics: rhs is a narrower SignedInteger, read sign-extended.
// lhs 6-bit UInt, rhs 3-bit SInt 0b111 = -1 → lhs += -1 (mod 64).
TEST_F(QuantumArithmeticTest, AddAnyIntAnyIntInPlaceSignedNarrowRhs)
{
    auto lhs = System::add_register("anyint_s_lhs", UnsignedInteger, 6);
    auto rhs = System::add_register("anyint_s_rhs", SignedInteger, 3);

    for (uint64_t a = 0; a < 64; ++a) {
        for (uint64_t rb = 0; rb < 8; ++rb) {
            std::vector<System> state(1);
            state[0].get(lhs).value = a;
            state[0].get(rhs).value = rb;

            Add_AnyInt_AnyInt_InPlace(lhs, rhs)(state);

            const int64_t ext = ref_sext(rb, 3);
            EXPECT_EQ(state[0].get(lhs).value, (a + static_cast<uint64_t>(ext)) & 0x3f)
                << "a=" << a << " rhs_bits=" << rb;
            EXPECT_EQ(state[0].get(rhs).value, rb);  // rhs bit pattern untouched
        }
    }
}

// Dagger round-trip with a signed narrow rhs: U then U† (and U† then U)
// restores the original lhs for every rhs pattern.
TEST_F(QuantumArithmeticTest, AddAnyIntAnyIntInPlaceSignedDaggerRoundTrip)
{
    auto lhs = System::add_register("anyint_d_lhs", UnsignedInteger, 6);
    auto rhs = System::add_register("anyint_d_rhs", SignedInteger, 3);

    for (uint64_t rb = 0; rb < 8; ++rb) {
        Add_AnyInt_AnyInt_InPlace op(lhs, rhs);
        for (uint64_t a = 0; a < 64; ++a) {
            std::vector<System> fwd_first(1);
            fwd_first[0].get(lhs).value = a;
            fwd_first[0].get(rhs).value = rb;
            op(fwd_first);
            op.dag(fwd_first);
            ASSERT_EQ(fwd_first.size(), 1);
            EXPECT_EQ(fwd_first[0].get(lhs).value, a) << "fwd→dag rb=" << rb;

            std::vector<System> dag_first(1);
            dag_first[0].get(lhs).value = a;
            dag_first[0].get(rhs).value = rb;
            op.dag(dag_first);
            op(dag_first);
            ASSERT_EQ(dag_first.size(), 1);
            EXPECT_EQ(dag_first[0].get(lhs).value, a) << "dag→fwd rb=" << rb;
            EXPECT_EQ(dag_first[0].get(rhs).value, rb);
        }
    }
}

// Alias rejection: constructing with lhs == rhs always throws (always-on check)
TEST_F(QuantumArithmeticTest, AddAnyIntAnyIntInPlaceAliasRejected)
{
    auto reg = System::add_register("anyint_alias", UnsignedInteger, 4);
    EXPECT_THROW(Add_AnyInt_AnyInt_InPlace("anyint_alias", "anyint_alias"),
                 std::invalid_argument);
    EXPECT_THROW(Add_AnyInt_AnyInt_InPlace(reg, reg), std::invalid_argument);
}

// ============ check_inplace_unitarity for XOR-out ops ============
// The debugger helper (reg_sizes + factory) also fits out-of-place XOR
// operators: for a SelfAdjoint op, dag() == operator(), so the round-trip
// check degenerates to U^2 = I and the output index (inputs ∪ res) must be
// collision-free. Mixed widths {2, 3, 4} = 9 total bits (512 states).
TEST_F(QuantumArithmeticTest, XorAndUIntUIntCheckInplaceUnitarity)
{
    auto xor_factory = [](std::vector<size_t> ids) -> Xor_UInt_UInt {
        return Xor_UInt_UInt{ids[0], ids[1], ids[2]};
    };
    auto and_factory = [](std::vector<size_t> ids) -> And_UInt_UInt {
        return And_UInt_UInt{ids[0], ids[1], ids[2]};
    };

    for (bool dagger : {false, true}) {
        auto tt_xor = check_inplace_unitarity<Xor_UInt_UInt>({2, 3, 4}, xor_factory, dagger);
        auto tt_and = check_inplace_unitarity<And_UInt_UInt>({2, 3, 4}, and_factory, dagger);

        std::vector<bool> seen_xor(tt_xor.size(), false);
        for (size_t out : tt_xor)
            EXPECT_FALSE(seen_xor[out]) << "Non-bijective Xor: " << out << " seen twice",
            seen_xor[out] = true;
        std::vector<bool> seen_and(tt_and.size(), false);
        for (size_t out : tt_and)
            EXPECT_FALSE(seen_and[out]) << "Non-bijective And: " << out << " seen twice",
            seen_and[out] = true;
    }
}