#include "qram_branch_qutrit.h"

using namespace std;

namespace qram_simulator
{
	namespace qram_qutrit
	{
		bool QRAMState::operator<(const QRAMState& rhs) const
		{
			return std::forward_as_tuple(nz_elements.size(), nz_elements)
				< std::forward_as_tuple(rhs.nz_elements.size(), rhs.nz_elements);
		}

		bool QRAMState::operator==(const QRAMState& rhs) const
		{
			return std::forward_as_tuple(nz_elements.size(), nz_elements)
				== std::forward_as_tuple(rhs.nz_elements.size(), rhs.nz_elements);
		}

		void QRAMState::clear_zero_elements()
		{
			auto pred = [](auto&& elem)
			{
				return elem.second.check_0();
			};
			erase_if(nz_elements, pred);
		}

		QRAMState::iterator_t QRAMState::iterof(size_t nnz_location)
		{
			return nz_elements.find(nnz_location);
		}

		QRAMState::const_iterator_t QRAMState::iterof(size_t nnz_location) const
		{
			return nz_elements.find(nnz_location);
		}

		QRAMState::iterator_t QRAMState::iterend()
		{
			return nz_elements.end();
		}

		QRAMState::const_iterator_t QRAMState::iterend() const
		{
			return nz_elements.end();
		}

		void QRAMState::busin(bus_t& bus, size_t digit)
		{
			auto iter = iterof(0);

			auto busdigit = get_digit(bus, digit);

			// SWAP [0].data with bus[digit]
			if (iter == iterend())
			{
				if (busdigit) {
					// bus=1 data=0
					nz_elements.insert({ 0, QRAMNode(W,1) });
					bus -= pow2(digit);
				}
			}
			else
			{
				if (iter->second.data == 0 && busdigit)
				{
					// bus=1 data=0
					iter->second.data = 1;
					bus -= pow2(digit);
				}
				else if (iter->second.data == 1 && !busdigit)
				{
					// bus=0 data=1
					iter->second.data = 0;
					bus += pow2(digit);
				}
			}
		}

		void QRAMState::busout(bus_t& bus, size_t digit)
		{
			return busin(bus, digit);
		}

		void QRAMState::acopy(bool a)
		{
			auto iter = iterof(0);

			if (iter == iterend())
			{
				if (a) {
					// bus=1 data=0
					nz_elements.insert({ 0, QRAMNode(W,1) });
				}
			}
			else
			{
				iter->second.data ^= bool2int(a);
			}
		}

		void QRAMState::_unconditional_internal_swap(size_t node_location)
		{
			auto&& [iter, flag] = nz_elements.insert({ node_location, { L, 0 } });
			if (!flag) { iter->second.internal_swap(); }
		}

		void QRAMState::_impl_conditional_internal_swap(size_t node_location)
		{
			auto siter = iterof(node_location);
			auto l = left_of(node_location);
			auto r = right_of(node_location);

			if (siter == iterend()) { /*do nothing*/ }
			switch (siter->second.addr)
			{
			case L:
				_unconditional_internal_swap(l);
				break;
			case R:
				_unconditional_internal_swap(r);
				break;
			default:
				break;
			}
		}

		void QRAMState::internal_swap_layer(size_t layerid)
		{
			if (layerid == 0)
			{
				_unconditional_internal_swap(0);
			}
			auto&& [lower, upper] = get_layer_node_range(layerid - 1);
			auto liter = nz_elements.lower_bound(lower);
			auto riter = nz_elements.upper_bound(upper);
			std::vector<size_t> nz_ids;
			for (auto it = liter; it != riter; ++it)
			{
				if (it->second.addr != W)
					nz_ids.push_back(it->first);
			}
			for (auto id : nz_ids)
			{
				_impl_conditional_internal_swap(id);
			}
		}

		void QRAMState::_impl_iter_swap(iterator_t iter, size_t target)
		{
			auto&& data = iter->second.data;
			auto&& ret = nz_elements.insert({ target,{ W, data } });

			auto&& target_iter = ret.first;
			auto&& flag = ret.second;
			auto&& target_data = target_iter->second.data;

			if (flag) {
				// does not exist
				data = 0;
			}
			else {
				// existed
				std::swap(data, target_data);
			}
		}

		void QRAMState::cswap(size_t node_location)
		{
			auto siter = iterof(node_location);
			auto l = left_of(node_location);
			auto r = right_of(node_location);

			if (siter == iterend()) { /*do nothing*/ }
			switch (siter->second.addr)
			{
			case W: // do nothing
				break;
			case L:
				_impl_iter_swap(siter, l);
				break;
			case R:
				_impl_iter_swap(siter, r);
				break;
			}
		}

		void QRAMState::cswap_layer(size_t layerid)
		{
			auto&& [lower, upper] = get_layer_node_range(layerid);
			auto liter = nz_elements.lower_bound(lower);
			auto riter = nz_elements.upper_bound(upper);
			std::vector<size_t> cswap_ids;
			for (auto it = liter; it != riter; ++it)
			{
				cswap_ids.push_back(it->first);
			}
			for (auto id : cswap_ids)
			{
				cswap(id);
			}
		}

		void QRAMState::flip(size_t qubit_id)
		{
			size_t nodeid = qubit_id / 2;
			if (qubit_id & 1)
			{
				auto&& [iter, flag] = nz_elements.insert({ nodeid, {W, 1} });
				if (!flag) {
					iter->second.data_flip();
				}
			}
			else
			{
				auto iter = nz_elements.find(nodeid);
				if (iter != iterend())
				{
					iter->second.addr_flip();
				}
			}
		}

		void QRAMState::rotate_A1(size_t qubit_id)
		{
			size_t nodeid = qubit_id / 2;
			auto&& [iter, flag] = nz_elements.insert({ nodeid, { R, 0 } });
			if (!flag) {
				if (iter->second.rotate_A1())
					nz_elements.erase(iter);
			}
		}

		void QRAMState::rotate_A2(size_t qubit_id)
		{
			size_t nodeid = qubit_id / 2;
			auto&& [iter, flag] = nz_elements.insert({ nodeid, { L, 0 } });
			if (!flag) {
				if (iter->second.rotate_A2())
					nz_elements.erase(iter);
			}
		}

		void QRAMState::set_zero(size_t qubit_id)
		{
			size_t nodeid = qubit_id / 2;
			auto iter = iterof(nodeid);
			if (iter == iterend()) return;
			if (qubit_id & 1)
				iter->second.data = 0;
			else
				iter->second.addr = W;
		}

		int QRAMState::state_of(size_t qubit_id) const
		{
			size_t nodeid = qubit_id / 2;
			auto iter = iterof(nodeid);
			if (iter == iterend()) return 0;
			if (qubit_id & 1)
				return iter->second.data;
			else
				return iter->second.addr;
		}

		std::string QRAMState::to_string() const
		{
			std::vector<char> buffer;
			fmt::format_to(std::back_inserter(buffer),
				"[");

			for (size_t i = 0; i < nz_elements.size(); ++i)
			{
				if (i != 0)
					fmt::format_to(std::back_inserter(buffer),
						", ");
				fmt::format_to(std::back_inserter(buffer),
					"{}: {}", i, std::string(nz_elements.at(i)));
			}

			fmt::format_to(std::back_inserter(buffer),
				"]");

			return { buffer.data(), buffer.size() };
		}

		void SubBranch::run_acopy(bool a)
		{
			state.acopy(a);
		}

		void SubBranch::run_swap(size_t layer_id)
		{
			state.internal_swap_layer(layer_id);
		}

		void SubBranch::run_cswap(size_t layer_id)
		{
			state.cswap_layer(layer_id);
		}

		void SubBranch::run_bitflip(size_t qubit_id)
		{
			state.flip(qubit_id);
		}

		void SubBranch::run_A1(size_t qubit_id)
		{
			state.rotate_A1(qubit_id);
		}

		void SubBranch::run_A1_2(size_t qubit_id)
		{
			state.rotate_A2(qubit_id);
		}

		void SubBranch::run_A2(size_t qubit_id)
		{
			auto st = state.state_of(qubit_id);
			if (st == W) return;
			if (st == L) amplitude *= w;
			else amplitude *= w2;
		}

		void SubBranch::run_A2_2(size_t qubit_id)
		{
			auto st = state.state_of(qubit_id);
			if (st == W) return;
			if (st == L) amplitude *= w2;
			else amplitude *= w;
		}

		void SubBranch::run_phaseflip(size_t qubit_id, double depol_id)
		{

			if (qubit_id % 2)
			{
				if (state.state_of(qubit_id) == 1)
					amplitude *= -1;
			}
			else {
				auto st = state.state_of(qubit_id);
				if (st == W) return;

				switch (int(std::floor(depol_id * 2)))
				{
				case 0:
				{
					if (st == L) amplitude *= w;
					else amplitude *= w2;
				}
				break;
				case 1:
				{
					if (st == L) amplitude *= w2;
					else amplitude *= w;
				}
				break;
				default:
					throw_bad_switch_case();
				}
			}

		}

		void SubBranch::run_bitphaseflip(size_t qubit_id)
		{
			state.flip(qubit_id);
			if (state.state_of(qubit_id) == 1)
				amplitude *= -1;
		}

		void SubBranch::run_depolarizing(size_t qubit_id, double depol_id)
		{
			if (qubit_id % 2)
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

			else
			{
				/*
				A1 = [ 0 1 0 ]	A2 = [ 1 0 0  ]
					 [ 0 0 1 ]	     [ 0 w 0  ]
					 [ 1 0 0 ]	     [ 0 0 w^2]

				A1   = L->W, W->R, R->L
				A1^2 = L->R, W->L, R->W
				*/

				static const complex_t w = { cos(pi * 2 / 3), sin(pi * 2 / 3) };
				static const complex_t w2 = w * w;

				switch (int(std::floor(depol_id * 8)))
				{
				case 0:	// A1
					run_A1(qubit_id);
					break;
				case 1:	// A2
					run_A2(qubit_id);
					break;
				case 2: // A1^2
					run_A1_2(qubit_id);
					break;
				case 3: // A2^2
					run_A2_2(qubit_id);
					break;
				case 4: // A1A2
					run_A2(qubit_id);
					run_A1(qubit_id);
					break;
				case 5: // A1^2 A2
					run_A2(qubit_id);
					run_A1_2(qubit_id);
					break;
				case 6: // A1 A2^2
					run_A2_2(qubit_id);
					run_A1(qubit_id);
					break;
				case 7: // A1^2 A2^2
					run_A1_2(qubit_id);
					run_A2_2(qubit_id);
					break;
				default:
					throw_bad_switch_case();
				}
			}
		}

		void SubBranch::set_zero(size_t qubit_id)
		{
			// TODO: (same as qubit)
			state.set_zero(qubit_id);
		}

		void SubBranch::run_busin(size_t digit)
		{
			state.busin(data_bus, digit);
		}

		void SubBranch::run_busout(size_t digit)
		{
			state.busout(data_bus, digit);
		}

		void Branch::set_good(Branch* first_good_ptr)
		{
			good = true;
			good_ref = first_good_ptr;
			if (first_good_ptr == this)
				throw_bad_switch_case();
		}

		double Branch::get_prob() const
		{
			if (!good) {
				double ret = 0;
				for (auto iter = iterbeg(); iter != iterend(); ++iter) {
					ret += abs_sqr(iter->amplitude);
				}
				return ret;
			}
			else {
				return good_ref->get_prob();
			}
		}

		complex_t Branch::get_fidelity(const memory_t& memory) const
		{
			if (good) return good_ref->get_fidelity(memory);

			auto expect_bus = bus_input ^ (memory[address]);
			complex_t ret = 0;
			for (auto iter = iterbeg(); iter != iterend(); ++iter)
			{
				if (iter->data_bus == expect_bus) {
					ret += iter->amplitude;
				}
			}
			return ret;
		}

		std::string Branch::to_string() const
		{
			if (good)
				return "good_ref";

			std::vector<char> buf;
			fmt::format_to(std::back_inserter(buf), "|{}> (input=|{}>)", address, bus_input);
			if (system_states.size() > 1) {
				fmt::format_to(std::back_inserter(buf), "\n(\n");
			}
			int i = 1;
			for (const auto& state : system_states) {
				if (i > 1) fmt::format_to(std::back_inserter(buf), "\n");
				fmt::format_to(std::back_inserter(buf), "\t{} |{}> Sys={}",
					state.amplitude, state.data_bus, state.state.nz_elements);
				/* for debug */
				// fmt::format_to(std::back_inserter(buf), " counter={}", debug_counter);
				++i;
			}
			if (system_states.size() > 1) {
				fmt::format_to(std::back_inserter(buf), "\n)");
			}
			return { buf.data(), buf.size() };
		}

		void Branch::run_fetchdata(memory_t& memory, size_t digit)
		{
			size_t sz = memory.size();
			size_t lowerbound = (sz >> 1) - 1;
			for (auto& system_state : system_states) {

				auto& state_impl = system_state.state.nz_elements;
				auto it_low = state_impl.lower_bound(lowerbound);
				std::vector<size_t> last_layer;

				for (auto it = it_low; it != state_impl.end(); ++it)
				{
					int memoffset = (it->first - lowerbound) * 2;
					switch (it->second.addr)
					{
					case W:
						continue;
					case L:
						break;
					case R:
						memoffset += 1;
					}
					it->second.data_control_flip(get_digit(memory[memoffset], digit));
				}
			}
		}

		void Branch::run_hadamard()
		{
			try_merge();
		}

		void Branch::try_merge()
		{
			// try to merge after 2 hadamards
			if (system_states.size() >= 2)
			{
				static auto fn = [](SubBranch& t1, const SubBranch& t2)
				{
					t1.amplitude += t2.amplitude;
				};
				static auto remove = [](const SubBranch& s)
				{
					return abs_sqr(s.amplitude) < epsilon;
				};

				// sort(system_states.begin(), system_states.end(), std::less<SubBranch>());
				/*system_states.sort();
				auto iter = unique_and_merge(system_states.begin(), system_states.end(), std::equal_to<SubBranch>(), fn);
				iter = std::remove_if(system_states.begin(), iter, remove);
				system_states.resize(distance(system_states.begin(), iter));*/
				sort_merge_unique_erase(system_states, std::less<SubBranch>(), std::equal_to<SubBranch>(), fn, remove);
			}
		}

		std::optional<Branch::element_type> Branch::get_good_branch_system() const
		{
			if (system_states.size() == 0) return std::nullopt;
			return system_states.begin()->state.nz_elements;
		}

		void Branch::run_damp_common(double gamma)
		{
			for (auto it = iterbeg(); it != iterend(); ++it)
			{
				auto& state = *it;
				auto& nz_elements = state.state.nz_elements;
				size_t nz_count = 0;
				for (const auto& item : nz_elements)
				{
					if (item.second.addr != W) nz_count++;
					if (item.second.data != 0) nz_count++;
				}

				state.amplitude *= std::pow(std::sqrt(1 - gamma), nz_count);
			}
		}

		Branch::damp_prob_type Branch::get_prob_damp(size_t qubit_id) const
		{
			Branch::damp_prob_type ret;
			ret[0] = 0;
			ret[1] = 0;

			for (auto it = iterbeg(); it != iterend(); ++it)
			{
				auto& state = *it;

				auto& nz_elements = state.state.nz_elements;
				auto iter = nz_elements.find(qubit_id / 2);
				if (iter == nz_elements.end())
					continue;
				if (qubit_id % 2) {
					switch (iter->second.data)
					{
					case 0:
						continue;
					case 1:
						ret[0] += abs_sqr(state.amplitude);
						break;
					default:
						break;
					}
				}
				else {
					switch (iter->second.addr)
					{
					case W:
						continue;
					case L:
						ret[0] += abs_sqr(state.amplitude);
						break;
					case R:
						ret[1] += abs_sqr(state.amplitude);
						break;
					default:
						break;
					}
				}
			}
			return ret;
		}
		void Branch::run_damp_full(size_t qubit_id, size_t k)
		{
			size_t target_state;
			if (qubit_id % 2) { target_state = 1; }
			else { target_state = W + 1 + k; }

			auto pred = [target_state, qubit_id](SubBranch& state)
			{
				size_t nodeid = qubit_id / 2;
				auto iter = state.state.iterof(nodeid);
				if (iter == state.state.iterend()) return true;
				if (qubit_id & 1)
				{
					if (iter->second.data != target_state)
						return true;

					iter->second.data = 0;
					return false;
				}
				else
				{
					if (iter->second.addr != target_state)
						return true;

					iter->second.addr = W;
					return false;
				}
			};

			remove_if(pred);

			// try_merge();
		}
		void Branch::remove_mismatch_state(const QRAMState::element_type& target_state)
		{
			if (good)
				return;

			auto pred = [&target_state](const SubBranch& this_state)
			{
				bool ret = this_state.state.nz_elements != target_state;
				//if (ret)
				//{
				//	fmt::print("Removed\n");
				//}
				return ret;
			};
			remove_if(pred);
		}

		void Branch::remove_all_state()
		{
			system_states.clear();
		}
	} // namespace qram_qutrit
} // namespace qram_simulator 

