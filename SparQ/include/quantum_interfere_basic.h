#pragma once
#include "system_operations.h"
#include "sort_state.h"

namespace qram_simulator
{
	struct StateHashExceptKey {
		size_t id;

		StateHashExceptKey(size_t id_)
			: id(id_)
		{}
		size_t operator()(const System& v) const;
	};

	struct StateHashExceptQubits {
		size_t id;
		std::set<size_t> qubit_positions;
		StateHashExceptQubits(size_t id_, std::set<size_t> qubit_positions_)
			: id(id_), qubit_positions(qubit_positions_)
		{}
		size_t operator()(const System& v) const;
	};

	//struct StateHashFromCache {
	//	StateHashFromCache() {}
	//	size_t operator()(const System& v) const {
	//		return v.cached_hash;
	//	}
	//};

	struct StateEqualExceptKey {
		size_t id;

		StateEqualExceptKey(size_t id_) : id(id_) {}
		size_t operator()(const System& v1, const System& v2) const;
	};

	struct StateEqualExceptQubits {
		size_t id;
		std::set<size_t> qubit_positions;
		StateEqualExceptQubits(size_t id_, std::set<size_t> qubit_positions_) : id(id_), qubit_positions(qubit_positions_) {}
		size_t operator()(const System& v1, const System& v2) const;
	};

	struct StateLessExceptKey {
		size_t id;

		StateLessExceptKey(size_t id_) : id(id_) {}
		size_t operator()(const System& v1, const System& v2) const;
	};

	struct StateLessExceptQubits {
		size_t id;
		size_t mask;
		std::set<size_t> qubit_ids;
		StateLessExceptQubits(size_t id_, std::set<size_t> qubit_ids_) : id(id_), qubit_ids(qubit_ids_)
		{
			mask = make_mask(qubit_ids);
		}
		inline size_t remove_digits(size_t val) const
		{
			return val & mask;
		}
		inline size_t make_mask(const std::set<size_t>& qubit_ids)
		{
			size_t mask = 0;
			for (auto id : qubit_ids)
			{
				mask += pow2(id);
			}
			mask = ~mask;
			return mask;
		}
		size_t operator()(const System& v1, const System& v2) const;
	};

}