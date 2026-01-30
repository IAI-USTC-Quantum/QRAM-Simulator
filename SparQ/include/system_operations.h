#pragma once
#include "basic_components.h"
#include "debugger.h"

namespace qram_simulator
{
	void split_systems(std::vector<System>& new_state, std::vector<System>& old_state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	std::vector<System> split_systems(std::vector<System>& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	SparseState split_systems(SparseState& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	void combine_systems(std::vector<System>& to, const std::vector<System>& from);

	void combine_systems(SparseState& to, const SparseState& from);

#ifdef USE_CUDA
	CuSparseState split_systems(CuSparseState& state,
		const std::vector<size_t>& condition_variable_nonzeros,
		const std::vector<size_t>& condition_variable_all_ones,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_bit,
		const std::vector<std::pair<size_t, size_t>>& condition_variable_by_value
	);

	void combine_systems(CuSparseState& to, CuSparseState& from);
#endif

#define SPLIT_BY_CONDITIONS \
	std::decay_t<decltype(state)> unconditioned_state(0);\
	if (HasCondition)\
	{\
		unconditioned_state = split_systems(state,\
			condition_variable_nonzeros,\
			condition_variable_all_ones,\
			condition_variable_by_bit,\
			condition_variable_by_value\
		);\
	}\
	if (state.size())

#define MERGE_BY_CONDITIONS \
	if (!unconditioned_state.empty()) {\
		combine_systems(state, unconditioned_state);\
	}

	void reset_systems(std::vector<System>& state);
	void reset_systems(SparseState& state);
#ifdef USE_CUDA
	void reset_systems(CuSparseState& state);
#endif

	inline void add_systems(std::vector<System>& current_state, const std::vector<System>& new_state, double coef)
	{
		if (new_state.size() == 0) return;
		size_t original_size = current_state.size();		

		current_state.insert(current_state.end(), new_state.begin(), new_state.end());

		if (std::abs(coef - 1.0) > epsilon)
		{
			for (auto iter = current_state.begin() + original_size; iter != current_state.end(); ++iter)
			{
				iter->amplitude *= coef;
			}
		}

		if (original_size != 0)
			sort_merge_unique_erase(current_state, std::less<System>(),
				std::equal_to<System>(), merge_system, remove_system);
	}

	inline void add_systems(SparseState& current, const SparseState& new_state, double coef)
	{
		return add_systems(current.basis_states, new_state.basis_states, coef);
	}
#ifdef USE_CUDA
	void add_systems(CuSparseState& current, const CuSparseState& new_state, double coef);
#endif

	//Split register
	struct SplitRegister {
		std::string first_name;
		std::string second_name;
		size_t second_size;
		SplitRegister(size_t first_id_, std::string_view second_name_, size_t second_size_)
			: first_name(System::name_of(first_id_)), second_name(second_name_), second_size(second_size_) { }
		SplitRegister(std::string_view first_name_, std::string_view second_name_, size_t second_size_)
			: first_name(first_name_), second_name(second_name_), second_size(second_size_) {}

		size_t operator()(std::vector<System>& state) const;
		size_t operator()(SparseState& state) const;
#ifdef USE_CUDA
		size_t operator()(CuSparseState& state) const;
#endif
	};

	/* combine two registers (remove the second one)
	* value: (s.get(first_pos).value <<= second_size) + (s.get(second_pos).value); 
	*/
	struct CombineRegister {
		std::string first_name;
		std::string second_name;
		CombineRegister(size_t first_id_, size_t second_id_)
			: first_name(System::name_of(first_id_)), second_name(System::name_of(second_id_)) { }
		CombineRegister(std::string_view first_name_, std::string_view second_name_)
			: first_name(first_name_), second_name(second_name_) { }

		size_t operator()(std::vector<System>& state) const;
		size_t operator()(SparseState& state) const;
#ifdef USE_CUDA
		size_t operator()(CuSparseState& state) const;
#endif
	};

	/* change the position of the register */
	/* UNSAFE if called as a subprocess    */
	struct MoveBackRegister
	{
		size_t register_id;
		MoveBackRegister(std::string_view reg_in);
		MoveBackRegister(size_t reg_in);
		void operator()(std::vector<System>& states) const;
		void operator()(SparseState& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct AddRegister {
		std::string register_name;
		StateStorageType type;
		size_t size;

		AddRegister(std::string_view register_name_, StateStorageType type_, size_t size_);
		size_t operator()(std::vector<System>& state) const;
		size_t operator()(SparseState& state) const;
#ifdef USE_CUDA
		size_t operator()(CuSparseState& state) const;
#endif
	};

	struct AddRegisterWithHadamard {
		std::string register_name;
		StateStorageType type;
		size_t size;

		AddRegisterWithHadamard(std::string_view register_name_, StateStorageType type_, size_t size_);
		size_t operator()(std::vector<System>& state) const;
		size_t operator()(SparseState& state) const;
#ifdef USE_CUDA
		size_t operator()(CuSparseState& state) const;
#endif
	};

	struct RemoveRegister {
		size_t register_id;
		RemoveRegister(std::string_view register_name);
		RemoveRegister(size_t register_name_);
		void operator()(std::vector<System>& state) const;
		void operator()(SparseState& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};
	
	struct Push : BaseOperator
	{
		using BaseOperator::operator();

		std::string garbage_name;
		size_t reg_id;

		Push(std::string_view regname_, std::string_view garbage_name_)
			:reg_id(System::get(regname_)), garbage_name(garbage_name_)
		{ }

		Push(size_t regname_, std::string_view garbage_name_)
			: reg_id(regname_), garbage_name(garbage_name_)
		{ }

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct Pop : BaseOperator
	{
		using BaseOperator::operator();

		size_t reg_id;
		std::string reg_name;

		Pop(std::string_view reg_name_) : reg_id(System::get(reg_name_)), reg_name(reg_name_)
		{ }

		Pop(size_t reg_name_) : reg_id(reg_name_)
		{ }

		void operator()(std::vector<System>& state) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& state) const;
#endif
	};

	struct ClearZero : SelfAdjointOperator
	{
		using SelfAdjointOperator::operator();
		using SelfAdjointOperator::dag;

		double eps;
		ClearZero() :eps(epsilon) {};
		ClearZero(double eps_) :eps(eps_) {};
		void operator()(std::vector<System>& system_states) const;
#ifdef USE_CUDA
		void operator()(CuSparseState& s) const;
#endif
	};

	struct StateLoad
	{
		std::string main_reg;
		std::string anc_UA;
		std::string anc_4;
		std::string anc_3;
		std::string anc_2;
		std::string anc_1;
		size_t data_size;
		size_t rational_size;
		std::string savename;
		StateLoad(
			std::string main_reg,
			std::string anc_UA,
			std::string anc_4,
			std::string anc_3,
			std::string anc_2,
			std::string anc_1,
			size_t ds,
			size_t rs) :
			main_reg(main_reg), anc_UA(anc_UA), anc_4(anc_4), anc_3(anc_3), anc_2(anc_2), anc_1(anc_1),
			data_size(ds), rational_size(rs) {}
		std::vector<System> operator()(const std::string& savename_) const;
		complex_t load_amplitude(const std::string& line) const;
		size_t load_reg(const std::string& line, const std::string& reg) const;
		System load_branch(const std::string& line) const;
		bool is_branch(const std::string& line) const;
	};

	inline void print_state_to_file(std::vector<System>& state, const std::string &filename, int precision = 16)
	{
		{
			std::ofstream f_state(filename);
			if (!f_state.is_open()) {
				throw std::runtime_error("Failed to open file.");
			}
			f_state.close();
		}
		if (precision == 0)
			throw_invalid_input();

		{
			std::ofstream f_state(filename, std::ios_base::app);
			if (!f_state.is_open()) {
				throw std::runtime_error("Failed to open file.");
			}
			f_state << fmt::format("Print State To File:\n");

			for (auto& s : state) {
				f_state << fmt::format("{}\n", s.to_string(precision));
			}

			f_state.close();
		}
	}
}