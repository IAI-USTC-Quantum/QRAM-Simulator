// Include error_handler.h FIRST to access the TEST macro, then undefine it
// so that gtest's TEST macro can be used instead
#include <string>
#include <string_view>
#include "error_handler.h"
#undef TEST

#include <gtest/gtest.h>
#include "sparse_state_simulator.h"
#include <cmath>

using namespace qram_simulator;

class ConditionalRotationTest : public ::testing::Test {
protected:
    void SetUp() override {
        System::clear();
    }
    void TearDown() override {
        System::clear();
    }
};

// Test CondRot_Rational_Bool - conditional rotation based on rational input
// This rotates a boolean qubit conditioned on the value of a rational register
TEST(ConditionalRotationTest, CondRotOnZeroCondition)
{
    auto reg_in = System::add_register("reg_in", Rational, 1);
    auto reg_out = System::add_register("reg_out", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0> in rational register, |0> in boolean

    // Apply conditional rotation with condition = 0
    CondRot_Rational_Bool("reg_in", "reg_out")(state);

    // With condition 0, the rotation should be identity (no change)
    ASSERT_EQ(state.size(), 1);
    uint64_t val_out = state[0].get(reg_out).as<uint64_t>(1);
    EXPECT_EQ(val_out, 0);
}

// Test that conditional rotation works correctly on superposition
TEST(ConditionalRotationTest, CondRotPreservesSuperposition)
{
    auto reg_in = System::add_register("reg_in", Rational, 1);
    auto reg_out = System::add_register("reg_out", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();

    // Create superposition in rational register
    Hadamard_Int_Full("reg_in")(state);

    // Apply conditional rotation
    CondRot_Rational_Bool("reg_in", "reg_out")(state);

    // Should maintain superposition structure
    // (this is a basic sanity check)
    EXPECT_GT(state.size(), 1);
}

// Test phase rotation with Phase_Bool gate
TEST(ConditionalRotationTest, PhaseBoolGate)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);  // |1>

    // Apply phase gate with lambda = pi/2 (S gate)
    Phase_Bool("q", pi / 2)(state);

    ASSERT_EQ(state.size(), 1);
    complex_t expected(std::cos(pi / 2), std::sin(pi / 2));
    EXPECT_NEAR(std::abs(state[0].amplitude - expected), 0.0, 1e-9);
}

// Test phase gate with different angles
TEST(ConditionalRotationTest, PhaseBoolGateVariousAngles)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);  // |1>

    // Apply phase gate with lambda = pi (Z gate)
    Phase_Bool("q", pi)(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(-1, 0)), 0.0, 1e-9);
}

// Test conditional rotation with multi-digit rational register
TEST(ConditionalRotationTest, CondRotMultiDigit)
{
    auto reg_in = System::add_register("reg_in", Rational, 2);
    auto reg_out = System::add_register("reg_out", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |00> rational, |0> boolean

    // Apply conditional rotation
    CondRot_Rational_Bool("reg_in", "reg_out")(state);

    // With input |00> (value 0), rotation should be identity
    ASSERT_EQ(state.size(), 1);
    uint64_t val_out = state[0].get(reg_out).as<uint64_t>(1);
    EXPECT_EQ(val_out, 0);
}
