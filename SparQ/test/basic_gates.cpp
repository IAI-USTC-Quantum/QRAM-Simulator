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

class BasicGatesTest : public ::testing::Test {
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

// ============ Pauli X Gate Tests ============
// Pauli X: |0> -> |1>, |1> -> |0> (bit flip)
TEST(BasicGatesTest, PauliXOnZero)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0>

    Xgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
}

TEST(BasicGatesTest, PauliXOnOne)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);  // |1>

    Xgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 0);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
}

TEST(BasicGatesTest, PauliXTwiceIsIdentity)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0>

    Xgate_Bool("q")(state);
    Xgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 0);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
}

// ============ Pauli Y Gate Tests ============
// Pauli Y: |0> -> i|1>, |1> -> -i|0>
TEST(BasicGatesTest, PauliYOnZero)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0>

    Ygate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    // Y|0> = i|1>, so amplitude should be i
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(0, 1)), 0.0, 1e-9);
}

TEST(BasicGatesTest, PauliYOnOne)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);  // |1>

    Ygate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 0);
    // Y|1> = -i|0>, so amplitude should be -i
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(0, -1)), 0.0, 1e-9);
}

// ============ Pauli Z Gate Tests ============
// Pauli Z: |0> -> |0>, |1> -> -|1> (phase flip)
TEST(BasicGatesTest, PauliZOnZero)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0>

    Zgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 0);
    // Z|0> = |0>, amplitude unchanged
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
}

TEST(BasicGatesTest, PauliZOnOne)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);  // |1>

    Zgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    // Z|1> = -|1>, amplitude should be -1
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(-1.0, 0)), 0.0, 1e-9);
}

// ============ S Gate (Phase Gate) Tests ============
// S = diag(1, i): |0> -> |0>, |1> -> i|1>
TEST(BasicGatesTest, SGateOnZero)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();

    Sgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 0);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
}

TEST(BasicGatesTest, SGateOnOne)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);

    Sgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(0, 1)), 0.0, 1e-9);
}

TEST(BasicGatesTest, SGateTwiceIsZGate)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);

    Sgate_Bool("q")(state);
    Sgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    // S^2 = diag(1, -1) = Z
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(-1.0, 0)), 0.0, 1e-9);
}

// ============ T Gate Tests ============
// T = diag(1, exp(i*pi/4)): |0> -> |0>, |1> -> exp(i*pi/4)|1>
TEST(BasicGatesTest, TGateOnZero)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();

    Tgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 0);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
}

TEST(BasicGatesTest, TGateOnOne)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);

    Tgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    // T|1> = exp(i*pi/4)|1>
    complex_t expected(std::cos(pi / 4), std::sin(pi / 4));
    EXPECT_NEAR(std::abs(state[0].amplitude - expected), 0.0, 1e-9);
}

TEST(BasicGatesTest, TGateFourTimesIsZGate)
{
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    Init_Unsafe("q", 1)(state);

    Tgate_Bool("q")(state);
    Tgate_Bool("q")(state);
    Tgate_Bool("q")(state);
    Tgate_Bool("q")(state);

    ASSERT_EQ(state.size(), 1);
    EXPECT_EQ(getRegValue(state[0], q, 1), 1);
    // T^4 = diag(1, -1) = Z
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(-1.0, 0)), 0.0, 1e-9);
}

// ============ Multi-Qubit Operations ============
TEST(BasicGatesTest, PauliXOnEachQubitOf2QubitRegister)
{
    auto q = System::add_register("q", UnsignedInteger, 2);
    std::vector<System> state;
    state.emplace_back();  // |00>

    // Flip bit 0: |00> -> |01>
    Xgate_Bool("q", 0)(state);
    EXPECT_EQ(getRegValue(state[0], q, 2), 1);

    // Flip bit 1: |01> -> |11>
    Xgate_Bool("q", 1)(state);
    EXPECT_EQ(getRegValue(state[0], q, 2), 3);

    // Flip bit 0 again: |11> -> |10>
    Xgate_Bool("q", 0)(state);
    EXPECT_EQ(getRegValue(state[0], q, 2), 2);
}
