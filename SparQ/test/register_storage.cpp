#include <gtest/gtest.h>

#include "basic_components.h"
#include "system_operations.h"

using namespace qram_simulator;

class RegisterStorageTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		System::clear();
	}

	void TearDown() override
	{
		System::clear();
	}
};

TEST_F(RegisterStorageTest, ReservesAnInitialBlockAndGrowsOnDemand)
{
	System basis;
	EXPECT_EQ(basis.registers.size(), 0);
	EXPECT_GE(basis.registers.capacity(), System::InitialRegisterCapacity);

	const size_t register_count = System::InitialRegisterCapacity + 137;
	for (size_t index = 0; index < register_count; ++index)
	{
		EXPECT_EQ(
			System::add_register(
				"r" + std::to_string(index),
				General,
				1),
			index);
	}

	EXPECT_EQ(basis.get(register_count - 1).value, 0);
	EXPECT_EQ(basis.registers.size(), register_count);
	EXPECT_GE(basis.registers.capacity(), register_count);
}

TEST_F(RegisterStorageTest, SynchronousGrowthPreservesValuesAndReusesSlots)
{
	SparseState state;
	const size_t register_count = System::InitialRegisterCapacity + 137;

	for (size_t index = 0; index < register_count; ++index)
	{
		EXPECT_EQ(
			AddRegister(
				"r" + std::to_string(index),
				General,
				1)(state),
			index);
	}

	state[0].get(0).value = 1;
	state[0].get(register_count - 1).value = 1;
	EXPECT_EQ(state[0].registers.size(), register_count);
	EXPECT_EQ(state[0].get(0).value, 1);
	EXPECT_EQ(state[0].get(register_count - 1).value, 1);

	state[0].get(127).value = 0;
	RemoveRegister(127)(state);
	const size_t reused = AddRegister("reused", General, 1)(state);

	EXPECT_EQ(reused, 127);
	EXPECT_EQ(state[0].registers.size(), register_count);
	EXPECT_EQ(state[0].get(reused).value, 0);
	EXPECT_EQ(state[0].get(register_count - 1).value, 1);
}
