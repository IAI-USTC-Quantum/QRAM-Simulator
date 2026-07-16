#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "basic_components.h"
#include "system_operations.h"

using namespace qram_simulator;

namespace
{
	uint64_t width_mask(size_t width)
	{
		return width == 64 ? ~uint64_t{0} : (uint64_t{1} << width) - 1;
	}

	struct RegisterModel
	{
		struct Register
		{
			size_t width;
			bool active;
		};

		explicit RegisterModel(size_t basis_count)
			: values(basis_count)
		{}

		size_t add(size_t width)
		{
			size_t id;
			if (reusable.empty())
			{
				id = registers.size();
				registers.push_back({width, true});
				for (auto& basis : values)
					basis.push_back(0);
			}
			else
			{
				id = reusable.back();
				reusable.pop_back();
				registers[id] = {width, true};
				for (auto& basis : values)
					basis[id] = 0;
			}
			return id;
		}

		void remove(size_t id, bool clear_value = true)
		{
			registers[id].active = false;
			reusable.push_back(id);
			if (clear_value)
				for (auto& basis : values)
					basis[id] = 0;
		}

		size_t split(size_t first, size_t second_width)
		{
			const size_t second = add(second_width);
			registers[first].width -= second_width;
			for (auto& basis : values)
			{
				basis[second] = basis[first] & width_mask(second_width);
				basis[first] >>= second_width;
			}
			return second;
		}

		void combine(size_t first, size_t second)
		{
			const size_t first_width = registers[first].width;
			const size_t second_width = registers[second].width;
			registers[first].width += second_width;
			for (auto& basis : values)
			{
				const uint64_t low = basis[second] & width_mask(second_width);
				basis[first] = second_width == 64
					? low
					: ((basis[first] & width_mask(first_width)) << second_width) | low;
			}
			remove(second, false);
		}

		std::vector<Register> registers;
		std::vector<size_t> reusable;
		std::vector<std::vector<uint64_t>> values;
	};

	void expect_matches(const SparseState& state, const RegisterModel& model)
	{
		ASSERT_EQ(state.size(), model.values.size());
		for (size_t basis = 0; basis < state.size(); ++basis)
		{
			for (size_t id = 0; id < model.registers.size(); ++id)
			{
				EXPECT_EQ(state[basis].get(id).value, model.values[basis][id])
					<< "basis=" << basis << " register=" << id;
				EXPECT_EQ(System::status_of(id), model.registers[id].active)
					<< "register=" << id;
			}
		}
	}
}

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

TEST_F(RegisterStorageTest, CapacityBoundariesAndLazyConstAccessPreserveReferences)
{
	System basis;
	EXPECT_EQ(basis.registers.size(), 0);
	EXPECT_GE(basis.registers.capacity(), System::InitialRegisterCapacity);

	const std::vector<size_t> boundaries = {
		System::InitialRegisterCapacity - 1,
		System::InitialRegisterCapacity,
		System::InitialRegisterCapacity + 1,
		System::InitialRegisterCapacity + 137,
	};
	for (size_t index = 0; index <= boundaries.back(); ++index)
	{
		EXPECT_EQ(System::add_register("r" + std::to_string(index), General, 8), index);
		if (std::find(boundaries.begin(), boundaries.end(), index) != boundaries.end())
			EXPECT_EQ(System::get("r" + std::to_string(index)), index);
	}

	const System& const_basis = basis;
	const auto& first = const_basis.get(0);
	const auto* first_address = &first;
	EXPECT_EQ(const_basis.get(boundaries.back()).value, 0);
	EXPECT_EQ(first_address, &const_basis.get(0));
	EXPECT_EQ(basis.registers.size(), boundaries.back() + 1);
	EXPECT_GE(basis.registers.capacity(), basis.registers.size());
}

TEST_F(RegisterStorageTest, GeneratedSequenceMatchesIndependentMultiBasisModel)
{
	const size_t register_count = System::InitialRegisterCapacity + 137;
	SparseState state(4);
	RegisterModel model(state.size());

	for (size_t id = 0; id < register_count; ++id)
	{
		EXPECT_EQ(AddRegister("r" + std::to_string(id), General, 8)(state), model.add(8));
	}

	const std::vector<size_t> preserved = {
		0,
		System::InitialRegisterCapacity - 1,
		System::InitialRegisterCapacity,
		System::InitialRegisterCapacity + 1,
		register_count - 1,
	};
	for (size_t basis = 0; basis < state.size(); ++basis)
	{
		for (size_t offset = 0; offset < preserved.size(); ++offset)
		{
			const size_t id = preserved[offset];
			const uint64_t value = (basis * 37 + offset * 19 + 3) & 0xff;
			state[basis].get(id).value = value;
			model.values[basis][id] = value;
		}
	}
	expect_matches(state, model);

	const std::vector<size_t> removal_order = {
		System::InitialRegisterCapacity,
		1,
		register_count - 1,
		System::InitialRegisterCapacity - 1,
	};
	for (size_t offset = 0; offset < removal_order.size(); ++offset)
	{
		const uint64_t removable_value = 0x40 + offset;
		for (size_t basis = 0; basis < state.size(); ++basis)
		{
			state[basis].get(removal_order[offset]).value = removable_value;
			model.values[basis][removal_order[offset]] = removable_value;
		}
	}
	System::remove_register_synchronous(removal_order.front(), state);
	model.remove(removal_order.front());
	for (size_t index = 1; index < removal_order.size(); ++index)
	{
		const size_t id = removal_order[index];
		RemoveRegister{id}(state);
		model.remove(id);
	}
	for (auto it = removal_order.rbegin(); it != removal_order.rend(); ++it)
	{
		const size_t expected_id = model.add(8);
		EXPECT_EQ(expected_id, *it);
		EXPECT_EQ(AddRegister("reuse" + std::to_string(expected_id), General, 8)(state),
			expected_id);
	}
	expect_matches(state, model);

	RemoveRegister{removal_order[1]}(state);
	model.remove(removal_order[1]);
	RemoveRegister{removal_order[3]}(state);
	model.remove(removal_order[3]);
	EXPECT_EQ(AddRegister("reuse_again_a", General, 8)(state), model.add(8));
	EXPECT_EQ(AddRegister("reuse_again_b", General, 8)(state), model.add(8));
	expect_matches(state, model);

	const size_t packed = AddRegister("packed", General, 12)(state);
	EXPECT_EQ(packed, model.add(12));
	for (size_t basis = 0; basis < state.size(); ++basis)
	{
		const uint64_t value = (basis * 0x321 + 0x5a3) & 0xfff;
		state[basis].get(packed).value = value;
		model.values[basis][packed] = value;
	}
	const size_t low = SplitRegister("packed", "low", 5)(state);
	EXPECT_EQ(low, model.split(packed, 5));
	expect_matches(state, model);
	EXPECT_EQ(CombineRegister("packed", "low")(state), packed);
	model.combine(packed, low);
	expect_matches(state, model);

	const size_t recycled_low = AddRegister("recycled_low", General, 5)(state);
	EXPECT_EQ(recycled_low, model.add(5));
	expect_matches(state, model);
}

TEST_F(RegisterStorageTest, SplitAndCombineMaskFullWidthAndSentinelValues)
{
	SparseState state(3);
	const size_t packed = AddRegister("packed", General, 12)(state);
	const std::vector<uint64_t> sentinel_values = {
		0xfffffffffffff5a3ULL,
		0xaaaaaaaaaaaaa321ULL,
		0x5555555555555fffULL,
	};
	for (size_t basis = 0; basis < state.size(); ++basis)
		state[basis].get(packed).value = sentinel_values[basis];

	const size_t low = SplitRegister("packed", "low", 5)(state);
	for (size_t basis = 0; basis < state.size(); ++basis)
	{
		EXPECT_EQ(state[basis].get(low).value, sentinel_values[basis] & 0x1f);
		EXPECT_EQ(state[basis].get(packed).value, sentinel_values[basis] >> 5);
	}
	CombineRegister("packed", "low")(state);
	for (size_t basis = 0; basis < state.size(); ++basis)
		EXPECT_EQ(state[basis].get(packed).value, sentinel_values[basis] & 0xfff);

	const size_t full = AddRegister("full", General, 64)(state);
	for (size_t basis = 0; basis < state.size(); ++basis)
		state[basis].get(full).value = sentinel_values[basis];
	const size_t all_bits = SplitRegister("full", "all_bits", 64)(state);
	for (size_t basis = 0; basis < state.size(); ++basis)
	{
		EXPECT_EQ(state[basis].get(full).value, 0);
		EXPECT_EQ(state[basis].get(all_bits).value, sentinel_values[basis]);
	}
	CombineRegister("full", "all_bits")(state);
	for (size_t basis = 0; basis < state.size(); ++basis)
		EXPECT_EQ(state[basis].get(full).value, sentinel_values[basis]);
}
