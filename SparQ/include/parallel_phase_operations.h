#pragma once
#include "basic_components.h"
namespace qram_simulator
{
	
	struct ZeroConditionalPhaseFlip : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		// std::vector<std::string> regs;
		std::vector<size_t> ids;

		ClassControllable

		ZeroConditionalPhaseFlip(const std::vector<size_t> &regs_)
			: ids(regs_)
		{
		}
		ZeroConditionalPhaseFlip(const std::vector<std::string> &regs_)
		{
			ids.reserve(regs_.size());
			for (const auto& reg : regs_)
			{
				ids.push_back(System::get(reg));
			}
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct RangeConditionalPhaseFlip : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		size_t id;
		size_t value_range;
		RangeConditionalPhaseFlip(size_t id_, size_t range_)
			:id(id_), value_range(range_)
		{}
		RangeConditionalPhaseFlip(std::string_view reg_, size_t range_)
			:id(System::get(reg_)), value_range(range_)
		{}
		void operator()(std::vector<System>& state) const;
	};

	/*  inverse is `true`: (I-2|0><0|)*phase
		inverse is `false`: (2|0><0|-I)*phase
		phase is `1.0` by default. */
	struct Reflection_Bool : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		std::vector<size_t> regs;
		bool inverse;
		ClassControllable
		
		Reflection_Bool(std::string_view reg_, bool inverse_ = false)
			: inverse(inverse_)
		{
			regs.push_back(System::get(reg_));
		}
		Reflection_Bool(size_t id_, bool inverse_ = false)
			: inverse(inverse_)
		{
			regs.push_back(id_);
		}
		Reflection_Bool(const std::vector<std::string> &regs_, bool inverse_ = false)
			: inverse(inverse_)
		{
			for (auto& reg : regs_) regs.push_back(System::get(reg));
		}
		Reflection_Bool(const std::vector<size_t> &ids_, bool inverse_ = false)
			: inverse(inverse_)
		{
			regs = ids_;
		}
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct GlobalPhase_Int : BaseOperator
	{
		using BaseOperator::operator();
		using BaseOperator::dag;

		complex_t c;
		ClassControllable
		
		GlobalPhase_Int(complex_t c_) : c(c_) {};
		void operator()(std::vector<System>& state) const;
		void dag(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
		void dag(CuSparseState& state) const;
#endif
	};

}