#pragma once

#include "time_step.h"

namespace qram_simulator {
	namespace qram_qutrit {
		struct QRAMNode
		{
			int addr = 0;
			int data = 0;

			operator std::string() const {
				return fmt::format("({},{})", addr2str(addr), data);
			}

			QRAMNode() = default;
			QRAMNode(int a, int d) : addr(a), data(d) {}

			inline void data_flip() { data ^= 1; }
			inline void addr_flip()
			{
				switch (addr) {
				case L:
					addr = R;
					return;
				case R:
					addr = L;
					return;
				default:
					return;
				}
			}
			inline void data_setzero() { data = 0; }
			inline void data_setone() { data = 1; }
			inline void data_set(bool value) { data = bool2int(value); }
			inline void data_control_flip(bool classical_data)
			{
				if (classical_data) data_flip();
			}
			inline void internal_swap()
			{
				if (addr == W)
				{
					addr = data;
					data = 0;
				}
				else {
					if (data == 0) {
						data = addr;
						addr = W;
					}
				}
			}

			/*
			A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
				 [ 0 0 1 ]	     [ 0 w 0  ]
				 [ 1 0 0 ]	     [ 0 0 w^2]
			A1   = L->W, W->R, R->L
			*/
			inline bool rotate_A1()
			{
				if (addr == L) {
					addr = W;
					return data == 0;
				}
				if (addr == W) addr = R;
				if (addr == R) addr = L;
				return false;
			}

			inline bool rotate_A2()
			{
				if (addr == R) {
					addr = W;
					return data == 0;
				}
				if (addr == L) addr = R;
				if (addr == W) addr = L;
				return false;
			}

			inline bool check_0() const
			{
				if (addr == W && data == 0) return true;
				else return false;
			}
			inline bool operator<(const QRAMNode& rhs) const
			{
				return std::tie(addr, data) < std::tie(rhs.addr, rhs.data);
			}

			inline bool operator==(const QRAMNode& rhs) const
			{
				return std::tie(addr, data) == std::tie(rhs.addr, rhs.data);
			}
		};

		/* QRAM internal state */
		struct QRAMState
		{
			using value_type = QRAMNode;

			// sparse storage and keep sorted
			using element_type = std::map<size_t, QRAMNode>;
			using iterator_t = typename element_type::iterator;
			using const_iterator_t = typename element_type::const_iterator;

			element_type nz_elements;

			QRAMState() = default;

			/* Compare two QRAM state */
			bool operator<(const QRAMState& rhs) const;
			bool operator==(const QRAMState& rhs) const;

			void clear_zero_elements();

			inline constexpr static size_t get_nnz_location(size_t layer, size_t pos)
			{
				size_t begin = pow2(layer) - 1;
				return (begin + pos);
			}

			inline constexpr static size_t left_of(size_t node_location)
			{
				return 2 * node_location + 1;
			}
			inline constexpr static size_t right_of(size_t node_location)
			{
				return 2 * node_location + 2;
			}
			inline constexpr static size_t parent_of(size_t node_location)
			{
				return (node_location - 1) / 2;
			}

			/* Time: O(log n) */
			iterator_t iterof(size_t nnz_location);
			const_iterator_t iterof(size_t nnz_location) const;

			/* Time: O(1) */
			iterator_t iterend();
			const_iterator_t iterend() const;

			void busin(bus_t& bus, size_t digit);
			void busout(bus_t& bus, size_t digit);
			void acopy(bool a);

			void _unconditional_internal_swap(size_t node_location);
			void _impl_conditional_internal_swap(size_t node_location);
			void internal_swap_layer(size_t layerid);
			void _impl_iter_swap(iterator_t iter, size_t target);

			void cswap(size_t node_location);
			void cswap_layer(size_t layerid);
			void flip(size_t qubit_id);

			/*
			A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
				 [ 0 0 1 ]	     [ 0 w 0  ]
				 [ 1 0 0 ]	     [ 0 0 w^2]
			A1   = L->W, W->R, R->L
			A1^2 = L->R, W->L, R->W
			*/
			void rotate_A1(size_t qubit_id);
			void rotate_A2(size_t qubit_id);

			int state_of(size_t qubit_id) const;
			void set_zero(size_t qubit_id);

			std::string to_string() const;

		};

		struct SubBranch
		{
			QRAMState state;
			bus_t data_bus;
			size_t data_size;
			std::complex<double> amplitude = 1.0;

			using element_type = typename QRAMState::element_type;

			SubBranch() = default;
			SubBranch(memory_entry_t bus_, size_t bus_sz) : data_size(bus_sz), data_bus(bus_)
			{ }

			inline bool operator<(const SubBranch& rhs) const noexcept
			{
				return std::tie(data_bus, state) < std::tie(rhs.data_bus, rhs.state);
			}

			inline bool operator==(const SubBranch& rhs) const noexcept
			{
				return std::tie(data_bus, state) == std::tie(rhs.data_bus, rhs.state);
			}

			void run_acopy(bool a);
			void run_swap(size_t node);
			void run_cswap(size_t node);
			void run_bitflip(size_t qubit_id);
			void run_A1(size_t qubit_id);
			void run_A1_2(size_t qubit_id);
			void run_A2(size_t qubit_id);
			void run_A2_2(size_t qubit_id);
			void run_phaseflip(size_t qubit_id, double depol_id);
			void run_bitphaseflip(size_t qubit_id);
			void run_depolarizing(size_t qubit_id, double depol_id);
			void set_zero(size_t qubit_id);
			void run_busin(size_t digit);
			void run_busout(size_t digit);
		};

		struct Branch {
			constexpr static int arch_type = arch_qutrit;
			constexpr static int damp_op_num = 2;

			using state_type = SubBranch;
			using element_type = typename state_type::element_type;
			using damp_prob_type = std::array<double, 2>;

			size_t address = 0;
			size_t bus_size = 1;
			memory_entry_t bus_input = 0;
			double relative_multiplier = 1;
			Branch* good_ref = nullptr;
			double branch_prob = 0.0;
		private:
			bool good = false;
			std::vector<SubBranch> system_states;
		public:
			Branch() = default;
			Branch(size_t address_, size_t bus_sz_, memory_entry_t bus_input_)
				: address(address_), bus_size(bus_sz_), bus_input(bus_input_)
			{ }
			Branch(const Branch& old_branch) = default;
			Branch(size_t branchid, size_t bus_sz)
			{
				bus_size = bus_sz;
				address = branchid >> bus_sz;
				bus_input = branchid - (address << bus_sz);
			}

			inline size_t get_branchid() const
			{
				return (address << bus_size) + bus_input;
			}

			auto iterbeg() const { return system_states.begin(); }
			auto iterend() const { return system_states.end(); }
			auto iterbeg() { return system_states.begin(); }
			auto iterend() { return system_states.end(); }
			auto system_size() const { return system_states.size(); }

			template<typename Pred>
			inline void remove_if(Pred pred)
			{
				auto iter = std::remove_if(system_states.begin(), system_states.end(), pred);
				system_states.erase(iter, system_states.end());
			}

			inline void reset()
			{
				system_states.clear();
				system_states.push_back(SubBranch(bus_input, bus_size));
				good = false;
				relative_multiplier = 1;
			}

			void set_good(Branch* first_good_ptr);
			inline bool is_good() const { return good; }

			inline bool operator<(const Branch& other) const {
				return std::tie(address, bus_input, system_states) <
					std::tie(other.address, other.bus_input, other.system_states);
			}

			inline bool operator==(const Branch& other) const {
				return std::tie(address, bus_input, system_states) ==
					std::tie(other.address, other.bus_input, other.system_states);
			}

			double get_prob() const;
			complex_t get_fidelity(const memory_t& memory) const;
			std::string to_string() const;
			void run_fetchdata(memory_t& memory, size_t digit);
			void run_hadamard();
			void try_merge();

			std::optional<element_type> get_good_branch_system() const;

			void run_damp_common(double gamma);

			damp_prob_type get_prob_damp(size_t qubit_id) const;

			void run_damp_full(size_t qubit_id, size_t k);

			inline static void get_multiplier(
				double gamma,
				const TimeStep& time_step,
				size_t step,
				std::vector<Branch>& branches,
				size_t first_good_branch,
				const std::vector<size_t>& good_branch_ids,
				/*std::vector<double>& multipliers,*/
				const memory_t& memory
			)
			{
				time_step.get_multiplier_qutrit(gamma, step, branches, first_good_branch, good_branch_ids,/* multipliers,*/ memory);
			}

			void remove_mismatch_state(const QRAMState::element_type& target_state);
			void remove_all_state();

			inline void clear_all_state() { system_states.clear(); }
		};
	} // namespace qram_qutrit
} // namespace qram_simulator

template <> struct fmt::formatter<qram_simulator::qram_qutrit::QRAMNode> {
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}
	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(const qram_simulator::qram_qutrit::QRAMNode& p,	FormatContext& ctx) const -> decltype(ctx.out())
	{
		return format_to(ctx.out(), "{}", std::string(p));
	}
};
