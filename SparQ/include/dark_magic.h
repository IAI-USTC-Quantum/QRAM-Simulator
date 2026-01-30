#pragma once
#include "basic_components.h"

namespace qram_simulator
{
	struct Normalize : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct Init_Unsafe : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t value;
		size_t id;

		Init_Unsafe(int id_, size_t value_) :
			value(value_), id(id_)
		{ }

		Init_Unsafe(std::string_view reg, size_t value_) :
			value(value_), id(System::get(reg))
		{ }

		void operator()(std::vector<System>& system_states) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};
}