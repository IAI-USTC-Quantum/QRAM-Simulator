#pragma once

#include "qram_circuit_qutrit.h"
#include "global_macros.h"
#include "cuda/cuda_utils.cuh"

namespace qram_simulator
{
	using StateInfoType = std::tuple<std::string, StateStorageType, size_t, bool>;

	const std::string& get_name(const StateInfoType& m);
	std::string& get_name(StateInfoType& m);
	const StateStorageType& get_type(const StateInfoType& m);
	StateStorageType& get_type(StateInfoType& m);
	size_t get_size(const StateInfoType& m);
	size_t& get_size(StateInfoType& m);
	bool get_status(const StateInfoType& m);
	bool& get_status(StateInfoType& m);

	struct StateStorage
	{
		/* The actual storage is only one uint64_t value*/
		uint64_t value = 0;

		/* Interpret the value as the "type"
		* Supported types:
		*	Signed integer
		*	Unsigned integer
		*	Floating point (with a fixed point storage and included in [0,1))
		*	Boolean
		*/
		template<typename Ty>
		Ty as(size_t size) const
		{
			uint64_t truncated_value = value & (pow2(size) - (size != 0));

			if constexpr (std::is_floating_point_v<Ty>)
			{
				if (size == 64) return truncated_value * 1.0 / 2 / pow2(63);
				return 1.0 * truncated_value / pow2(size);
			}
			else if constexpr (std::is_same_v<Ty, bool>)
			{
				return bool(truncated_value);
			}
			else if constexpr (std::is_integral_v<Ty>)
			{
				if constexpr (std::is_signed_v<Ty>)
				{
					return get_complement(truncated_value, size);
				}
				else
				{
					return truncated_value;
				}
			}
			else {
				throw_invalid_input();
			}
		}

		/* Comprehend the value as a boolean */
		//bool as_bool() const;

		HOST_DEVICE StateStorage() {}

		/* For a safe access to the value */
		HOST_DEVICE uint64_t& val(size_t size);
		HOST_DEVICE uint64_t val(size_t size) const;

		/* For comparison */
		HOST_DEVICE bool operator==(const StateStorage& rhs) const;
		HOST_DEVICE bool operator!=(const StateStorage& rhs) const;
		HOST_DEVICE bool operator<(const StateStorage& rhs) const;
		HOST_DEVICE bool operator>(const StateStorage& rhs) const;

		/* For string conversion */
		std::string to_string(const StateInfoType& info) const;
		std::string to_io_string(const StateInfoType& info) const;
		std::string to_binary_string(const StateInfoType& info) const;

		/* For bitwise operations */
		HOST_DEVICE void flip(size_t digit);
	};

	struct SparseState;
#ifdef USE_CUDA
	struct CuSparseState;
#endif

	struct System 
	{	
		/* Todo : auto increasing register size
		*
		* A cached (maximum) register size, if more than CachedRegisterSize
		* registers are allocated, then runtime_error will be raised.
		*/

#ifdef CACHED_REGISTER_SIZE
		constexpr static size_t CachedRegisterSize = CACHED_REGISTER_SIZE;
#else
		constexpr static size_t CachedRegisterSize = 40;
#endif

		/* Info for each register */
		inline static std::vector<StateInfoType> name_register_map;
		inline static uint64_t reg_status_bitmap = 0;

		/* Dynamical statistics for the quantum program */
		inline static size_t max_qubit_count = 0;
		inline static size_t max_register_count = 0;
		inline static size_t max_system_size = 0;

		/* For stack push/pop */
		inline static std::vector<size_t> temporal_registers;

		/* For automatic reusage of the register */
		inline static std::vector<size_t> reusable_registers;

		/* state's amplitude */		 
		complex_t amplitude = 1.0;

		/* |s_1>...|s_n>*/
		//std::vector<StateStorage> registers;		 
		std::array<StateStorage, CachedRegisterSize> registers;

		/* Get the corresponding state component */
		StateStorage& get(size_t id);
		const StateStorage& get(size_t id) const;

		/* Initialize the register's allocation infos */
		static void clear();

		/* Count the qubit */
		static size_t get_qubit_count();

		/* Count activated register */
		static size_t get_activated_register_size();

		static size_t get_last_activated_register();
		/* Access to the last activated register */
		StateStorage& last_register();
		const StateStorage& last_register() const;

		/* Update the maximum system size */
		static void update_max_size(size_t new_size);

		/* Get the register id by name */
		static size_t get(std::string_view name);

		/* Get the register info by name */
		static StateInfoType get_register_info(std::string_view name);

		/* Get the register name by id */
		static const std::string& name_of(size_t id);

		/* Get the register size by id */
		static size_t size_of(std::string_view name);

		/* Get the register size by id */
		static size_t size_of(size_t id);

		/* Get the register type by name */
		static StateStorageType type_of(std::string_view name);

		/* Get the register type by id */
		static StateStorageType type_of(size_t id);

		/* Get the register status by name (True = active, False = inactive)*/
		static bool status_of(std::string_view name);

		/* Get the register status by id (True = active, False = inactive)*/
		static bool status_of(size_t id);

		/* Add a new register */
		static void add_register_status_bitmap(size_t pos);
		static void remove_register_status_bitmap(size_t pos);
		static size_t add_register(std::string_view name, StateStorageType type, size_t size);

		/* add register, and make the initial state to be 0 */
		static size_t add_register_synchronous(
			std::string_view name, StateStorageType type, size_t size,
			std::vector<System>& system_states);

		/* add register, and make the initial state to be 0 */
		static size_t add_register_synchronous(
			std::string_view name, StateStorageType type, size_t size,
			SparseState& system_states);

		/* Remove a register by id */
		static void remove_register(size_t id);

		/* Remove a register by name */
		static void remove_register(std::string_view name);

		/* Remove a register by id, and make the corresponding state to be 0 */
		static void remove_register_synchronous(size_t id,
			std::vector<System>& state);

		/* Remove a register by name, and make the corresponding state to be 0 */
		static void remove_register_synchronous(std::string_view name,
			std::vector<System>& state);

		/* Remove a register by id, and make the corresponding state to be 0 */
		static void remove_register_synchronous(size_t id,
			SparseState& state);

		/* Remove a register by name, and make the corresponding state to be 0 */
		static void remove_register_synchronous(std::string_view name,
			SparseState& state);

		HOST_DEVICE System() {}

		/* For comparison */
		HOST_DEVICE bool operator<(const System& rhs) const;
		HOST_DEVICE bool operator==(const System& rhs) const;
		HOST_DEVICE bool operator!=(const System& rhs) const;

		/* For string conversion */
		std::string to_string() const;
		std::string to_string(int precision) const;
	};

	/* Merge two systems by adding amplitude of s2 to s1,
	 * and setting s2.amplitude to 0	*/
	void merge_system(System& s1, System& s2);

	/* Remove system if it is close to zero,
	 * return true if want to be removed */
	bool remove_system(const System& s);

	struct SparseState
	{
		std::vector<System> basis_states;

		using vector_type = std::vector<System>;

		SparseState() {
			basis_states.emplace_back();
		}
		SparseState(size_t size)
			: basis_states(size) {}

		SparseState(const std::vector<System>& basis_states_) 
			: basis_states(basis_states_) {}

		SparseState(std::vector<System>&& basis_states_)
			: basis_states(std::move(basis_states_)) {}

		SparseState(const SparseState& other)
			: basis_states(other.basis_states) {}

		SparseState(SparseState&& other)
			: basis_states(std::move(other.basis_states)) {}

		SparseState& operator=(const SparseState& other) {
			basis_states = other.basis_states;
			return *this;
		}

		SparseState& operator=(SparseState&& other) {
			basis_states = std::move(other.basis_states);
			return *this;
		}

		System& back() { return basis_states.back(); }
		const System& back() const { return basis_states.back(); }

		vector_type::iterator begin() { return basis_states.begin(); }
		vector_type::const_iterator begin() const { return basis_states.begin(); }
		vector_type::iterator end() { return basis_states.end(); }
		vector_type::const_iterator end() const { return basis_states.end(); }
		vector_type::reverse_iterator rbegin() { return basis_states.rbegin(); }
		vector_type::const_reverse_iterator rbegin() const { return basis_states.rbegin(); }
		vector_type::reverse_iterator rend() { return basis_states.rend(); }
		vector_type::const_reverse_iterator rend() const { return basis_states.rend(); }

		System& operator[](size_t i) { return basis_states[i]; }
		const System& operator[](size_t i) const { return basis_states[i]; }

		size_t size() const { return basis_states.size(); }
		bool empty() const { return basis_states.empty(); }
	};


	enum DeviceType { CPU, GPU, ANY };

	struct CuSparseState;

	struct BaseOperator
	{
		virtual void operator()(std::vector<System>& state) const = 0;
		inline virtual void dag(std::vector<System>& state) const
		{
			throw_not_implemented("Dagger is not implemented.");
		}

		inline void operator()(SparseState& state) const
		{
			(*this)(state.basis_states);
		}

		inline virtual void dag(SparseState& state) const
		{
			this->dag(state.basis_states);
		}
#ifdef USE_CUDA
		virtual void operator()(CuSparseState& state) const;
		virtual void dag(CuSparseState& state) const;
#endif
	};

	class SelfAdjointOperator : public BaseOperator {
	public:
		using BaseOperator::operator();
		using BaseOperator::dag;

		inline void dag(std::vector<System>& state) const override {
			(*this)(state);
		}
		inline void dag(SparseState& state) const override {
			(*this)(state);
		}

#ifdef USE_CUDA
		void dag(CuSparseState& state) const override;
#endif
	};


#ifdef SINGLE_THREAD
	constexpr auto exec_policy = std::execution::seq;
#else
	constexpr auto exec_policy = std::execution::par;
#endif


}
