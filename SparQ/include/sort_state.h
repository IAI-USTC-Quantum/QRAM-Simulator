#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	/* Sort with some key expected to be moved to the last. */
	struct SortExceptKey : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id;
		SortExceptKey(std::string_view key_)
			: id(System::get(key_))
		{}

		SortExceptKey(size_t key_)
			: id(key_)
		{}

		void operator()(std::vector<System>& states) const;
	};

	struct SortByKey : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t register_key;
		SortByKey(std::string_view key);
		SortByKey(size_t key);
		void operator()(std::vector<System>& state) const;
	};

	struct SortExceptBit : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id;
		size_t digit;
		SortExceptBit(std::string_view key_, size_t digit_)
			: id(System::get(key_)), digit(digit_)
		{}

		SortExceptBit(size_t key_, size_t digit_)
			: id(key_), digit(digit_)
		{}

		void operator()(std::vector<System>& states) const;
	};

	inline uint64_t make_mask(const std::set<size_t>& qubit_ids)
	{
		uint64_t mask = 0;
		for (auto id : qubit_ids)
		{
			mask += pow2(id);
		}
		mask = ~mask;
		return mask;
	}

	/* Sort with some key expected to be moved to the last. */
	struct SortExceptKeyHadamard : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id;
		uint64_t mask;
		std::set<size_t> qubit_ids;

		SortExceptKeyHadamard(std::string_view key_, std::set<size_t> qubit_ids_)
			: id(System::get(key_)), qubit_ids(qubit_ids_)
		{
			mask = make_mask(qubit_ids);
		}

		SortExceptKeyHadamard(size_t key_, std::set<size_t> qubit_ids_)
			: id(key_), qubit_ids(qubit_ids_)
		{
			mask = make_mask(qubit_ids);
		}

		size_t remove_digits(size_t val) const;

		void operator()(std::vector<System>& states) const;
	};

	/* Sort unconditionally in parallel */
	struct SortUnconditional : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		SortUnconditional() {}
		void operator()(std::vector<System>& states) const;
	};

	/* Sort unconditionally in parallel */
	struct SortByAmplitude : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		SortByAmplitude() {}
		void operator()(std::vector<System>& states) const;
	};


	/* Sort with some key expected to be moved to the last. */
	struct SortByKey2 : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id1;
		size_t id2;
		SortByKey2(std::string_view key1_, std::string_view key2_)
			: id1(System::get(key1_)), id2(System::get(key2_))
		{}

		SortByKey2(size_t key1_, size_t key2_)
			: id1(key1_), id2(key2_)
		{}

		void operator()(std::vector<System>& states) const;
	};

	/* The utility function to compute conditional rotation.
	Compare two states without comparing the target register.

	The comparison is performed reversely.

	Used in CondRot_General_Bool, CondRot_General_Bool_QW, Hadamard_Int
	*/
	bool compare_equal(const System& a, const System& b, size_t out_id);

	/* The utility function to compute QRAM.
	Compare two states without comparing the target register.

	The comparison is performed reversely.

	Used in QRAM::set_branches
	*/
	bool compare_equal2(const System& a, const System& b, size_t out_id1, size_t out_id2);

	/* TODO: Document
	*/
	bool compare_equal_rot(const System& a, const System& b, size_t out_id, uint64_t mask);

	/* The utility function to compute partial Hadamard.
	Compare two states without comparing the target register as well as the masked states
	(i.e. the acting qubits)

	The comparison is performed reversely.

	Used in Hadamard_PartialQubit
	*/
	bool compare_equal_hadamard(const System& a, const System& b, size_t out_id, uint64_t mask);

}