// Include error_handler.h FIRST to access the TEST macro, then undefine it
// so that gtest's TEST macro can be used instead
#include <string>
#include <string_view>
#include "error_handler.h"
#undef TEST

#include <gtest/gtest.h>
#include "sparse_state_simulator.h"

using namespace qram_simulator;

// Test Hadamard gate on |0> -> |+> (should give equal superposition of |0> and |1> with +1/sqrt(2) each)
TEST(HadamardTest, HadamardOnZero)
{
    System::clear();
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // Start with |0>

    Hadamard_Int_Full("q")(state);

    // After H|0>, we should have |+> = (|0> + |1>)/sqrt(2)
    // Two basis states, each with amplitude 1/sqrt(2)
    ASSERT_EQ(state.size(), 2);

    // Find |0> and |1> states
    complex_t amp0(0, 0), amp1(0, 0);
    for (const auto& s : state) {
        uint64_t val = s.get(q).as<uint64_t>(1);
        if (val == 0) amp0 = s.amplitude;
        else if (val == 1) amp1 = s.amplitude;
    }

    double expected_amp = 1.0 / std::sqrt(2);
    EXPECT_NEAR(std::abs(amp0), expected_amp, 1e-9);
    EXPECT_NEAR(std::abs(amp1), expected_amp, 1e-9);
    // Phase should be +1 for both in |+>
    EXPECT_NEAR(std::abs(amp0 - complex_t(expected_amp, 0)), 0.0, 1e-9);
    EXPECT_NEAR(std::abs(amp1 - complex_t(expected_amp, 0)), 0.0, 1e-9);
    System::clear();
}

// Test Hadamard gate on |1> -> |-> (should give equal superposition of |0> and |1> with +1/sqrt(2) and -1/sqrt(2))
TEST(HadamardTest, HadamardOnOne)
{
    System::clear();
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // Start with |0>
    Init_Unsafe("q", 1)(state);  // Flip to |1>

    Hadamard_Int_Full("q")(state);

    // After H|1>, we should have |-> = (|0> - |1>)/sqrt(2)
    ASSERT_EQ(state.size(), 2);

    complex_t amp0(0, 0), amp1(0, 0);
    for (const auto& s : state) {
        uint64_t val = s.get(q).as<uint64_t>(1);
        if (val == 0) amp0 = s.amplitude;
        else if (val == 1) amp1 = s.amplitude;
    }

    double expected_amp = 1.0 / std::sqrt(2);
    EXPECT_NEAR(std::abs(amp0), expected_amp, 1e-9);
    EXPECT_NEAR(std::abs(amp1), expected_amp, 1e-9);
    // |0> should have positive phase, |1> should have negative phase for |->
    EXPECT_NEAR(std::abs(amp0 - complex_t(expected_amp, 0)), 0.0, 1e-9);
    EXPECT_NEAR(std::abs(amp1 - complex_t(-expected_amp, 0)), 0.0, 1e-9);
    System::clear();
}

// Test H^2 = I (applying H twice returns to original state)
TEST(HadamardTest, HadamardTwiceIsIdentity)
{
    System::clear();
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0>

    Hadamard_Int_Full("q")(state);
    Hadamard_Int_Full("q")(state);

    // Should return to |0> with amplitude 1
    ASSERT_EQ(state.size(), 1);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-9);
    uint64_t val = state[0].get(q).as<uint64_t>(1);
    EXPECT_EQ(val, 0);
    System::clear();
}

// Test Hadamard on 2-qubit register: H|00> -> |++> (tensor product of |+> with itself)
TEST(HadamardTest, HadamardOnTwoQubits)
{
    System::clear();
    auto q = System::add_register("q", UnsignedInteger, 2);
    std::vector<System> state;
    state.emplace_back();  // |00>

    Hadamard_Int_Full("q")(state);

    // Should have 4 basis states, each with amplitude 1/2
    ASSERT_EQ(state.size(), 4);

    double expected_amp = 0.5;
    for (const auto& s : state) {
        EXPECT_NEAR(std::abs(s.amplitude), expected_amp, 1e-9);
    }
    System::clear();
}
