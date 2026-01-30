#include "qram_branch_qubit.h"

namespace qram_simulator
{
	namespace qram_qubit
	{
		bool State::operator<(const State& rhs) const
		{
			return std::forward_as_tuple(nz_elements.size(), nz_elements)
				< std::forward_as_tuple(rhs.nz_elements.size(), rhs.nz_elements);
		}

		bool State::operator==(const State& rhs) const
		{
			return std::forward_as_tuple(nz_elements.size(), nz_elements)
				== std::forward_as_tuple(rhs.nz_elements.size(), rhs.nz_elements);
		}

		std::pair<State::iterator_type, State::iterator_type> State::get_layer_iter(size_t layerid)
		{
			auto&& [lower, upper] = get_layer_range(layerid);
			auto liter = nz_elements.lower_bound(lower);
			if (liter == nz_elements.end()) {
				return { nz_elements.end(), nz_elements.end() };
			}
			else {
				return { liter, nz_elements.upper_bound(upper) };
			}
		}

		void State::get_layer_nonzero_nodes(size_t layerid, std::set<size_t>& nodes)
		{
			auto&& [lower, upper] = get_layer_range(layerid);
			auto liter = nz_elements.lower_bound(lower);
			for (auto it = liter; it != nz_elements.end(); ++it)
			{
				if (*it > upper) break;
				nodes.insert(*it / 2);
			}
		}

		void State::get_layer_nonzero_parent_nodes(size_t layerid, std::set<size_t>& nodes)
		{
			auto&& [lower, upper] = get_layer_range(layerid);
			auto liter = nz_elements.lower_bound(lower);
			for (auto it = liter; it != nz_elements.end(); ++it)
			{
				if (*it > upper) break;
				nodes.insert(parent_of(*it / 2));
			}
		}

		constexpr size_t State::get_nz_location(size_t layer, size_t pos, size_t lr)
		{
			size_t begin = pow2(layer) - 1;
			return (begin + pos) * 2 + lr;
		}

		constexpr size_t State::get_nz_location(size_t node_location, size_t lr)
		{
			return node_location * 2 + lr;
		}

		constexpr size_t State::left_of(size_t node_location)
		{
			return 2 * node_location + 1;
		}

		constexpr size_t State::right_of(size_t node_location)
		{
			return 2 * node_location + 2;
		}

		constexpr size_t State::parent_of(size_t node_location)
		{
			return (node_location - 1) / 2;
		}

		bool State::state_of(size_t nz_location) const
		{
			return nz_elements.find(nz_location) != nz_elements.end();
		}

		void State::busin(bus_t& bus, size_t digit)
		{
			auto bus_digit = (bus >> digit) & 1;
			// swap bus <-> data
			auto iter = nz_elements.find(1);

			if (iter == nz_elements.end())
			{
				if (bus_digit)
				{
					// bus = 1 data = 0
					nz_elements.insert(1);
					bus -= pow2(digit);
				}
			}
			else
			{
				// bus = 0 data = 1
				if (!bus_digit)
				{
					nz_elements.erase(iter);
					bus += pow2(digit);
				}
			}
		}

		void State::busout(bus_t& bus, size_t digit)
		{
			busin(bus, digit);
		}

		void State::flip(size_t nz_location)
		{
			auto it = nz_elements.find(nz_location);
			if (it != nz_elements.end())
				nz_elements.erase(it);
			else
				nz_elements.insert(nz_location);
		}

		void State::cnot(size_t nz_location, bool condition)
		{
			if (condition) flip(nz_location);
		}

		void State::general_swap(size_t nz_l, size_t nz_r)
		{
			auto it_l = nz_elements.find(nz_l);
			auto it_r = nz_elements.find(nz_r);
			auto it_end = nz_elements.end();
			if (it_l == it_r)
			{
				// do nothing
				return;
			}
			else if (it_l != it_end && it_r == it_end)
			{
				nz_elements.erase(it_l);
				nz_elements.insert(nz_r);
			}
			else if (it_l == it_end && it_r != it_end)
			{
				nz_elements.insert(nz_l);
				nz_elements.erase(it_r);
			}
		}

		void State::internal_swap(size_t node_location)
		{
			auto nnz_l = get_nz_location(node_location, 0);
			auto nnz_r = get_nz_location(node_location, 1);
			general_swap(nnz_l, nnz_r);
		}

		void State::internal_swap_layer(size_t layerid)
		{
			std::set<size_t> nodes;
			get_layer_nonzero_nodes(layerid, nodes);
			for (auto nodeid : nodes)
				internal_swap(nodeid);
		}

		void State::acopy(bool a)
		{
			cnot(get_nz_location(0, Data), a);
		}

		void State::cswap(size_t node_location)
		{
			auto addr = get_nz_location(node_location, Addr);
			auto data = get_nz_location(node_location, Data);
			auto ldata = get_nz_location(left_of(node_location), Data);
			auto rdata = get_nz_location(right_of(node_location), Data);

			auto it_addr = nz_elements.find(addr);
			if (state_of(addr) == ZeroState)
				// for |0> state
				general_swap(data, ldata);
			else
				// for |1> state
				general_swap(data, rdata);
		}

		void State::cswap_layer(size_t layerid) {
			std::set<size_t> nodes;
			get_layer_nonzero_parent_nodes(layerid + 1, nodes);
			get_layer_nonzero_nodes(layerid, nodes);
			for (auto node : nodes)
				cswap(node);
		}

		void State::set_zero(size_t qubit_id)
		{
			auto it = nz_elements.find(qubit_id);
			if (it != nz_elements.end())
			{
				nz_elements.erase(it);
			}
		}

		void SystemState::run_acopy(bool a)
		{
			state.acopy(a);
		}

		void SystemState::run_swap(size_t layer_id)
		{
			state.internal_swap_layer(layer_id);
		}

		void SystemState::run_cswap(size_t layer_id)
		{
			state.cswap_layer(layer_id);
		}

		void SystemState::run_bitflip(size_t qubit_id)
		{
			state.flip(qubit_id);
		}

		void SystemState::run_phaseflip(size_t qubit_id, double depol_id)
		{
			if (state.state_of(qubit_id) == OneState)
				amplitude *= -1;
		}

		void SystemState::run_bitphaseflip(size_t qubit_id)
		{
			auto iter = state.nz_elements.find(qubit_id);
			if (iter != state.nz_elements.end())
			{
				amplitude *= -1;
				state.nz_elements.erase(iter);
			}
		}

		void SystemState::run_depolarizing(size_t qubit_id, double depol_id)
		{
			switch (int(std::floor(depol_id * 3)))
			{
			case 0:
				run_bitflip(qubit_id); break;
			case 1:
				run_phaseflip(qubit_id, 0); break;
			case 2:
				run_bitphaseflip(qubit_id); break;
			default:
				throw_bad_switch_case();
			}
		}

		double Branch::get_prob() const
		{
			double ret = 0;
			for (auto iter = iterbeg(); iter != iterend(); ++iter) {
				ret += abs_sqr(iter->amplitude);
			}
			return ret;
		}

		//complex_t Branch::get_fidelity(const memory_t &memory) const
		//{
		//	auto expect_bus = bus_input ^ (memory[address]);
		//	complex_t ret = 0;
		//	for (auto iter = iterbeg(); iter != iterend(); ++iter)
		//	{
		//		if (iter->bus == expect_bus) {
		//			ret += iter->amplitude;
		//		}
		//	}
		//	return ret;
		//}

		std::string Branch::to_string() const
		{
			// return fmt::format("|{}> |{:d}>\n{}", complex2str(amplitude), address, bus, state.nz_elements);
			std::vector<char> buf;
			fmt::format_to(std::back_inserter(buf), "|{}>", address);
			if (system_states_sz > 1) {
				fmt::format_to(std::back_inserter(buf), "\n(\n");
			}
			int i = 1;
			for (auto iter = iterbeg(); iter != iterend(); ++iter) {
				if (i > 1) fmt::format_to(std::back_inserter(buf), "\n");
				fmt::format_to(std::back_inserter(buf), "\t{} |{}> Sys={}", iter->amplitude, iter->data_bus, iter->state.nz_elements);
				++i;
			}
			if (system_states_sz > 1) {
				fmt::format_to(std::back_inserter(buf), "\n)");
			}
			return { buf.data(), buf.size() };
		}

		void Branch::run_fetchdata(memory_t& memory, size_t digit)
		{
			size_t sz = memory.size();
			size_t lowerbound = sz - 2;
			for (auto iter = iterbeg(); iter != iterend(); ++iter) {

				auto& state_impl = iter->state.nz_elements;
				auto it_low = state_impl.lower_bound(lowerbound);
				std::vector<size_t> last_layer;

				for (auto it = it_low; it != state_impl.end(); ++it)
				{
					last_layer.push_back(*it);
				}

				for (size_t i = 0; i < last_layer.size(); ++i)
				{
					size_t d = last_layer[last_layer.size() - 1 - i];
					if (d & 1)
					{
						// is data qubit
						if (i + 1 != last_layer.size() &&
							d - last_layer[last_layer.size() - 1 - i - 1] == 1)
						{
							// if |11>, get d/2*2+1 (right mem)
							size_t memid = (d / 2 + 1 - (sz >> 1)) * 2 + 1;
							if (get_digit(memory[memid], digit))
								iter->amplitude *= -1;
						}
						else
						{
							// if |01>, get d/2*2 (left mem)
							size_t memid = (d / 2 + 1 - (sz >> 1)) * 2;
							if (get_digit(memory[memid], digit))
								iter->amplitude *= -1;
						}
					}
				}
			}
		}

		void SystemState::run_busin(size_t digit)
		{
			state.busin(data_bus, digit);
		}

		void SystemState::run_busout(size_t digit)
		{
			state.busout(data_bus, digit);
		}

		void SystemState::set_zero(size_t qubit_id)
		{
			state.set_zero(qubit_id);
		}

		void Branch::run_hadamard()
		{
			for (size_t digit = 0; digit < bus_size; ++digit) {
				if (system_states_sz * 2 > system_states.size())
					try_merge();

				if (system_states_sz * 2 > system_states.size())
					system_states.resize(system_states_sz * 2);

				size_t sz = system_states_sz;
				for (size_t i = 0; i < sz; ++i) {
					system_states[i].amplitude *= sqrt2inv;

					SystemState new_state(system_states[i]);
					bool busdigit = get_digit(new_state.data_bus, digit);
					if (busdigit)
					{
						system_states[i].amplitude *= -1;
						new_state.data_bus -= pow2(digit);
					}
					else {
						new_state.data_bus += pow2(digit);
					}

					system_states[sz + i] = (std::move(new_state));
				}

				system_states_sz = 2 * sz;
			}
		}

		void Branch::try_merge()
		{
			// try to merge after 2 hadamards
			if (system_states_sz >= 2)
			{
				auto fn = [](SystemState& t1, const SystemState& t2)
				{
					t1.amplitude += t2.amplitude;
				};
				auto remove = [](const SystemState& s)
				{
					return ignorable(std::abs(s.amplitude));
				};

				// sort(system_states.begin(), system_states.end(), std::less<SystemState>());

				std::sort(iterbeg(), iterend());
				auto iter = unique_and_merge(iterbeg(), iterend(), std::equal_to<SystemState>(), fn);
				iter = std::remove_if(iterbeg(), iter, remove);
				// system_states.resize(distance(system_states.begin(), iter));
				system_states_sz = std::distance(iterbeg(), iter);
			}
		}

		void Branch::run_damp_common(double gamma)
		{
			for (auto iter = iterbeg(); iter != iterend(); ++iter) {
				iter->run_damp_common(gamma);
			}
		}

		Branch::damp_prob_type Branch::get_prob_damp(size_t qubit_id) const
		{
			Branch::damp_prob_type ret;
			ret[0] = 0;
			for (auto iter = iterbeg(); iter != iterend(); ++iter)
			{
				if (iter->state.state_of(qubit_id) == 1)
					ret[0] += abs_sqr(iter->amplitude);
			}
			return ret;
		}

		void Branch::run_damp_full(size_t qubit_id, size_t k)
		{
			/*for (auto& state : system_states)
			{
			state.set_zero(qubit_id);
			}
			try_merge();*/

			auto&& pred = [qubit_id](SystemState& state)
			{
				return state.state.state_of(qubit_id) == 0;
			};
			remove_if(pred);
		}

		void Branch::remove_mismatch_state(const State::element_type& target_state)
		{
			auto iter = std::remove_if(iterbeg(), iterend(),
				[&target_state, this](const SystemState& this_state)
				{
					return this_state.state.nz_elements != target_state;
				}
			);

			system_states_sz = std::distance(iterbeg(), iter);
		}

		void Branch::remove_all_state()
		{
			system_states_sz = 0;
		}

	} // namespace qram_qubit
} // namespace qram_simulator 

