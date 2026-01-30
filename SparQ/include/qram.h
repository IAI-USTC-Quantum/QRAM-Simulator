#pragma once
#include "basic_components.h"
#include "dark_magic.h"
#include "sort_state.h"

namespace qram_simulator 
{
	/*
	   Note that this class only adapts to qram_qutrit::QRAMCircuit
	   If looking for qubit version, use QRAMLoad_Qubit instead
	*/
	struct QRAMLoad : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		const qram_qutrit::QRAMCircuit* qram;
//#ifdef USE_CUDA
//		void* memory_dev = nullt;
//#endif
		size_t register_addr;
		size_t register_data;
		static std::string version;

		ClassControllable

		QRAMLoad(const qram_qutrit::QRAMCircuit* qram_, size_t reg1, size_t reg2)
			: register_addr(reg1), register_data(reg2)
		{
			qram = qram_;

			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(register_addr) != UnsignedInteger)
				throw_invalid_input();
#endif
		}
		QRAMLoad(const qram_qutrit::QRAMCircuit* qram, std::string_view reg1, std::string_view reg2)
			: QRAMLoad(qram, System::get(reg1), System::get(reg2))
		{ }

		void noise_free_impl(std::vector<System>& state) const;
		void _set_branches(qram_qutrit::QRAMCircuit* qram,
			const std::vector<System>& state,
			std::vector<std::pair<size_t, size_t>>& groups) const;
		void _set_branches_impl(qram_qutrit::QRAMCircuit* qram, const std::vector<System>& state,
			decltype(qram->get_branches()) branches,
			decltype(qram->get_branch_probs()) branch_probs,
			size_t iter_l, size_t iter_r,
			std::vector<std::pair<size_t, size_t>>& groups) const;
		void _reconstruct(qram_qutrit::QRAMCircuit* qram, 
			std::vector<System>& state,
			std::vector<std::pair<size_t, size_t>>& groups) const;
		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void noise_free_impl(CuSparseState& state) const;
		void operator()(CuSparseState& state) const;
#endif	
	};

		/*
		   Note that this class only adapts to qram_qutrit::QRAMCircuit
		   If looking for qubit version, use QRAMLoadFast_Qubit instead
		*/
	struct QRAMLoadFast : SelfAdjointOperator {
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		const qram_qutrit::QRAMCircuit* qram;
		size_t register_addr;
		size_t register_data;

		ClassControllable

		QRAMLoadFast(const qram_qutrit::QRAMCircuit* qram, size_t reg1, size_t reg2);
		QRAMLoadFast(const qram_qutrit::QRAMCircuit* qram, std::string_view reg1, std::string_view reg2);
		void noise_free_impl(std::vector<System>& state) const;
		void has_damping_impl(std::vector<System>& state, qram_qutrit::QRAMCircuit* qram, std::vector<System>& state_remove_cache) const;
		void no_damping_impl(std::vector<System>& state, qram_qutrit::QRAMCircuit* qram, std::vector<System>& state_remove_cache) const;
		void operator()(std::vector<System>& state) const;
	};


	struct QRAMInputGenerator
	{
		std::set<std::pair<size_t, size_t>> unique_set;
		size_t addr_sz;
		size_t data_sz;
		size_t input_sz;
		std::uniform_int_distribution<size_t> addr_dist;
		std::uniform_int_distribution<size_t> data_dist;
		size_t addr;
		size_t data;


		QRAMInputGenerator(size_t addr_sz_, size_t data_sz_, size_t input_size_)
			: addr_sz(addr_sz_), data_sz(data_sz_), input_sz(input_size_),
			addr(std::numeric_limits<size_t>::max()), data(std::numeric_limits<size_t>::max()),
			addr_dist(0, pow2(addr_sz) - 1), data_dist(0, pow2(data_sz) - 1)
		{
			if (input_sz > pow2(addr_sz + data_sz))
			{
				input_sz = pow2(addr_sz + data_sz);
			}
		}

		QRAMInputGenerator(size_t addr_sz_, size_t data_sz_, size_t input_size_, size_t addr_, size_t data_)
			: addr_sz(addr_sz_), data_sz(data_sz_), input_sz(input_size_),
			addr(addr_), data(data_),
			addr_dist(0, pow2(addr_sz) - 1), data_dist(0, pow2(data_sz) - 1)
		{
			// turn to full input case
			if (input_sz > pow2(addr_sz + data_sz))
			{
				input_sz = pow2(addr_sz + data_sz);
			}
		}

		std::pair<size_t, size_t> rand_input()
		{
			return {
				addr_dist(random_engine::get_engine()),
				data_dist(random_engine::get_engine())
			};
		}

		void _validate_registers(size_t addr_, size_t data_) const
		{
			if (addr_ >= System::CachedRegisterSize ||
				data_ >= System::CachedRegisterSize)
			{
				throw_invalid_input();
			}
		}

		void generate_input(std::vector<System>& s, size_t addr_, size_t data_)
		{
			/* addr/data registers are not preset */
			s.clear();

			if (input_sz == pow2(addr_sz + data_sz))
			{
				generate_full_input(s, addr_, data_);
				return;
			}

			unique_set.clear();
			for (size_t i = 0; i < input_sz; )
			{
				auto&& input = rand_input();
				auto&& res = unique_set.insert(input);
				if (res.second)
				{
					i++;
					s.emplace_back();
					s.back().get(addr_).value = input.first;
					s.back().get(data_).value = input.second;
				}
			}
			SortUnconditional()(s);
			Normalize()(s);
		}

		void generate_input(std::vector<System>& s)
		{
			_validate_registers(addr, data);
			generate_input(s, addr, data);
		}

		void generate_full_input(std::vector<System>& s, size_t addr_, size_t data_)
		{
			s.clear();
			double amplitude = 1.0 / std::sqrt(input_sz);
			for (size_t i = 0; i < pow2(addr_sz); ++i)
			{
				for (size_t j = 0; j < pow2(data_sz); ++j)
				{
					s.emplace_back();
					s.back().get(addr_).value = i;
					s.back().get(data_).value = j;
					s.back().amplitude = amplitude;

					unique_set.insert({ i,j });
				}
			}
		}
	};
}