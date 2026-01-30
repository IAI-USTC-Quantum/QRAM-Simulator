#include "time_step.h"

namespace qram_simulator {

	Operation Operation::reverse() {
		Operation m(*this);
		m.dagger = !m.dagger;
		return m;
	}

	std::string Operation::to_string() const {
		std::string name, oprands, ret;
		name = type2str(type);
		oprands = vec2str(targets);
		ret = name + oprands;
		if (coefficients.size() > 0)
			ret += vec2str(coefficients);
		if (dagger) { ret += "^"; }

		return ret;
	}

	std::string OperationPack::to_string() const {
		std::vector<std::string> strs;
		for (const Operation& op : operations) {
			strs.push_back(op.to_string());
		}
		return vec2str(strs, "", "", " ");
	}

	std::string TimeSlices::to_string() const {
		std::vector<std::string> strs;
		int i = 0;
		for (const OperationPack& time_slice : time_slices) {
			strs.push_back("Time " + num2str(i)
				+ " (" + time_slice.name + ") > \n  "
				+ time_slice.to_string() + "\n");
			++i;
		}
		return vec2str(strs, "", "", "\n");
	}

	ContinuousRange::ContinuousRange(int l, int r)
	{
		range_bound = std::vector<int>{ l, r };
	}

	void ContinuousRange::clear()
	{
		range_bound.clear();
	}

	void ContinuousRange::merge(int l, int r)
	{
		auto liter = std::lower_bound(range_bound.begin(), range_bound.end(), l);
		auto riter = std::upper_bound(range_bound.begin(), range_bound.end(), r);

		bool keep_left = std::distance(range_bound.begin(), liter) % 2;
		bool keep_right = std::distance(range_bound.begin(), riter) % 2;

		range_bound.erase(liter, riter);
		if (!keep_left)
		{
			range_bound.push_back(l);
		}
		if (!keep_right)
		{
			range_bound.push_back(r);
		}
		std::sort(range_bound.begin(), range_bound.end());
	}

	bool ContinuousRange::accept(int t) const
	{
		if (range_bound.size() == 0)
			return false;

		auto riter = std::lower_bound(range_bound.begin(), range_bound.end(), t);

		if (riter == range_bound.end())
			return false;
		if (*riter == t)
			return true;
		if (std::distance(range_bound.begin(), riter) % 2)
			return true;
		else
			return false;
	}

	ContinuousRange::operator std::string()
	{
		std::vector<char> buf;
		for (size_t i = 0; i < range_bound.size(); ++i)
		{
			if (i & 1)
			{
				fmt::format_to(std::back_inserter(buf),
					"{}]", range_bound[i]);
			}
			else {
				fmt::format_to(std::back_inserter(buf),
					"[{},", range_bound[i]);
			}

		}
		return { buf.data(), buf.size() };
	}

	TimeStep::TimeStep(size_t addr_sz, size_t data_sz)
		: addr_size(addr_sz), data_size(data_sz)
	{
		init_noise_free();
		// bad_branches.resize(pow2(addr_sz), 0);
		// bad_branches_2.resize(pow2(addr_sz), 0);
	}


	size_t TimeStep::full_step() const
	{
		return 6 * addr_size + 2 * data_size;
	}

	size_t TimeStep::out(size_t in_step) const
	{
		return full_step() - in_step;
	}

	size_t TimeStep::last_step() const
	{
		return full_step() - 1;
	}

	int TimeStep::acopy(size_t step) const
	{
		// Ai: 2i+1 (i in [0,addr_size-1]) 
		if ((step & 1) && (step < 2 * addr_size)) return step / 2;
		return -1;
	}

	int TimeStep::acopy_out(size_t step) const
	{
		return acopy(out(step));
	}

	int TimeStep::dcopy(size_t step) const
	{
		// Di: 2addr_size+2i+1 (i in [0,data_size-1])
		step -= 1;
		if (step & 1) { return -1; }
		if (step >= 2 * addr_size && step <= 2 * addr_size + 2 * data_size - 2) {
			return step / 2 - addr_size;
		}
		return -1;
	}

	int TimeStep::dcopy_out(size_t step) const
	{
		return dcopy(step - 2 * addr_size);
	}

	size_t TimeStep::route_begin(size_t layer) const
	{
		return 3 * (layer + 1) + 1;
	}

	size_t TimeStep::route_wait_begin(size_t layer) const
	{
		return 2 * addr_size + 2 * data_size + layer + 1;
	}

	bool TimeStep::route_odd(size_t step, size_t layer) const
	{
		if ((step & 1) == 0) return false;
		if (step < route_begin(layer) || step > out(route_begin(layer))) { return false; }
		if (step >= route_wait_begin(layer) && step <= out(route_wait_begin(layer))) { return false; }
		return true;
	}

	bool TimeStep::route_even(size_t step, size_t layer) const
	{
		if (step & 1) return false;
		if (step < route_begin(layer) || step > out(route_begin(layer))) { return false; }
		if (step >= route_wait_begin(layer) && step <= out(route_wait_begin(layer))) { return false; }
		return true;
	}

	bool TimeStep::route(size_t step, size_t layer) const
	{
		if (layer >= addr_size - 1) return false;
		if (layer & 1) return route_odd(step, layer);
		else return route_even(step, layer);
	}

	int TimeStep::memory_copy(size_t step) const
	{
		if (step <= (3 * addr_size)) return -1;
		step -= (3 * addr_size);
		step -= 1;
		if (step & 1) return -1;
		step /= 2;
		if (step < data_size) { return step; }
		return -1;
	}

	int TimeStep::internal_swap(size_t step) const
	{
		if (step <= 3 * addr_size - 1 && step % 3 == 2)
		{
			return step / 3;
		}
		return -1;
	}

	int TimeStep::internal_swap_out(size_t step) const
	{
		return internal_swap(out(step));
	}

	OperationPack TimeStep::generate_step(size_t step) const
	{
		OperationPack pack;

		int addr_copy = acopy(step);
		if (addr_copy >= 0) pack.append({ OperationType::FirstCopy,{ (size_t)addr_copy } });

		int data_copy_out = dcopy_out(step);
		if (data_copy_out >= 0) pack.append({ OperationType::CopyOut,{ (size_t)data_copy_out } });

		int data_copy = dcopy(step);
		if (data_copy >= 0) pack.append({ OperationType::CopyIn,{ (size_t)data_copy } });

		for (size_t layer = 0; layer <= addr_size; ++layer) {
			if (route(step, layer)) pack.append({ OperationType::ControlSwap,{ (size_t)layer } });
		}

		int inswap = internal_swap(step);
		if (inswap >= 0) pack.append({ OperationType::SwapInternal,{ (size_t)inswap } });

		int inswap_out = internal_swap_out(step);
		if (inswap_out >= 0) pack.append({ OperationType::SwapInternal,{ (size_t)inswap_out } });

		int addr_copy_out = acopy_out(step);
		if (addr_copy_out >= 0) pack.append({ OperationType::FirstCopy,{ (size_t)addr_copy_out } });
	
		int mcopy = memory_copy(step);
		if (mcopy >= 0) pack.append({ OperationType::FetchData,{ (size_t)mcopy } });

		return pack;
	}

	size_t TimeStep::layer_entangle_max(size_t time_step) const
	{
		if (time_step <= 3 * addr_size - 2)
		{
			return time_step / 3;
		}
		else if (out(time_step) <= 3 * addr_size - 2)
		{
			return out(time_step) / 3;
		}
		else
		{
			return addr_size - 1;
		}		
	}

	bool TimeStep::is_bad_branch(size_t addr) const
	{
		return cr.accept(addr);
	}

	std::pair<size_t, size_t> TimeStep::get_bad_range_qubit(size_t bad_qubit) const
	{
		bad_qubit /= 2;
		if (bad_qubit == 0)
		{
			return { 0, pow2(addr_size) - 1 };
		}
		if (bad_qubit == 1)
		{
			return { 0, pow2(addr_size - 1) - 1 };
		}
		if (bad_qubit == 2)
		{
			return { pow2(addr_size - 1) , pow2(addr_size) - 1 };
		}

		size_t parent = (bad_qubit + 1) / 2 - 1;
		if (bad_qubit % 2)
		{
			return get_bad_range_qubit(parent * 2);
		}
		else {
			return get_bad_range_qutrit(bad_qubit * 2);
		}
	}


	std::pair<size_t, size_t> TimeStep::get_bad_range_qutrit(size_t bad_qubit) const
	{
		size_t last_layer_min = pow2(addr_size) - 1;
		size_t last_layer_max = last_layer_min + pow2(addr_size) - 1;

		size_t node = bad_qubit / 2;
		static auto within_range = [last_layer_min, last_layer_max](size_t node) {
			return node >= last_layer_min && node <= last_layer_max;
		};

		size_t left_range = node;
		size_t right_range = node;

		while (true)
		{
			if (left_range >= last_layer_min && right_range <= last_layer_max)
				break;
			left_range = left_range * 2 + 1;
			right_range = right_range * 2 + 2;
		}

		if (last_layer_min > left_range)
		{
			fmt::print("{} {}", last_layer_min, left_range, right_range, last_layer_max);
		}

		return { left_range - last_layer_min, right_range - last_layer_min };
	}

	void TimeStep::fill_bad_range(size_t qubit, int arch_type)
	{
		int l, r;
		if (arch_type == arch_qutrit)
		{
			std::tie(l, r) = get_bad_range_qutrit(qubit);
		}
		else
			throw_bad_switch_case();

		cr.merge(l, r);
	}

	void TimeStep::noise_one_step(
		OperationPack & pack,
		size_t max_entangle_layer,
		const std::map<OperationType, double>& noises,
		int arch_type)
	{
		static std::uniform_real_distribution<double> ud(0, 1);
		for (auto&& [noise_type, noise_parameter] : noises)
		{
			size_t nqubits = 2 * (pow2(max_entangle_layer) - 1);
			std::binomial_distribution<size_t> bd(nqubits, noise_parameter);
			size_t nerror = bd(random_engine::get_engine());

			std::uniform_int_distribution<size_t> uid(0, nqubits);
			std::set<size_t> error_positions;
			while (error_positions.size() < nerror)
			{
				size_t pos = uid(random_engine::get_engine());
				auto&& [_, flag] = error_positions.insert(pos);
				if (flag) {
					fill_bad_range(pos, arch_type);
					switch (noise_type) {
					case OperationType::BitFlip:
					case OperationType::BitPhaseFlip:
						pack.append(Operation(noise_type, { pos }));
						break;
					case OperationType::PhaseFlip:
					case OperationType::Depolarizing:
						pack.append(Operation(noise_type, { pos }, { ud(random_engine::get_engine())}));
						break;
					case OperationType::Damping:
						pack.append(Operation(OperationType::Damp_Full, {pos}, {noise_parameter}));
						break;
					default:
						throw_bad_switch_case();
					}
				}
			}
		}
		auto it = noises.find(OperationType::Damping);
		if (it != noises.end())
		{
			pack.append({ OperationType::Damp_Common, {}, {it->second} });
		}
	}

	void TimeStep::noise_one_step(
		size_t max_entangle_layer,
		const std::map<OperationType, double>& noises,
		int arch_type)
	{
		static std::uniform_real_distribution<double> ud(0, 1);
		for (auto&& [noise_type, noise_parameter] : noises)
		{
			size_t nqubits = 2 * (pow2(max_entangle_layer) - 1);
			std::binomial_distribution<size_t> bd(nqubits, noise_parameter);
			size_t nerror = bd(random_engine::get_engine());

			std::uniform_int_distribution<size_t> uid(0, nqubits);
			std::set<size_t> error_positions;
			while (error_positions.size() < nerror)
			{
				size_t pos = uid(random_engine::get_engine());
				auto&& [_, flag] = error_positions.insert(pos);
				if (flag) {
					fill_bad_range(pos, arch_type);
				}
			}
		}
	}

	void TimeStep::init_noise_free()
	{
		profiler _("TimeStep::init_noise_free");
		TimeSlices &slices = time_slices_noise_free;
		cr.clear();
		for (size_t step = 1; step < full_step(); ++step)
		{
			auto&& ops = generate_step(step);
			size_t max_entangle_layer = layer_entangle_max(step);
			slices.append(ops);
		}
	}

	void TimeStep::append_noise_range_only(const std::map<OperationType, double>& noises, int arch_type)
	{
		profiler _("TimeStep::append_noise_range_only");
		if (noises.size() == 0)
			return;
		cr.clear();

		for (size_t step = 1; step < full_step(); ++step)
		{
			size_t max_entangle_layer = layer_entangle_max(step);
			noise_one_step(max_entangle_layer, noises, arch_type);
		}
		return;
	}

	TimeSlices TimeStep::append_noise(const std::map<OperationType, double>& noises, int arch_type)
	{
		profiler _("TimeStep::append_noise");
		TimeSlices slices = time_slices_noise_free;
		if (noises.size() == 0)
			return slices;
		cr.clear();

		for (size_t step = 1; step < full_step(); ++step)
		{
			auto &ops = slices.time_slices[step - 1];
			size_t max_entangle_layer = layer_entangle_max(step);
			noise_one_step(ops, max_entangle_layer, noises, arch_type);			
		}
		return slices;
	}

	TimeSlices TimeStep::generate(const std::map<OperationType, double>& noises, int arch_type)
	{
		return append_noise(noises, arch_type);
	}

	ptrdiff_t TimeStep::_get_multiplier_impl_qubit(size_t step, size_t addr) const
	{
		ptrdiff_t countret = 0;
		for (size_t i = 0; i < addr_size; ++i)
		{
			if (get_digit_reverse(addr, i, addr_size)) {
				ptrdiff_t ex_step = step - 2 * i - 1;
				if (ex_step < 0) continue;
				ex_step = std::min(ex_step, ptrdiff_t(out(2 * i + 1) - (2 * i + 1)));
				countret += ex_step;
			}
		}
		return countret;
	}

	ptrdiff_t TimeStep::_get_multiplier_impl_qutrit(
		size_t step, size_t branch_id, const memory_t& mem) const
	{
		size_t addr = branch_id >> data_size;
		size_t data = branch_id - (addr << data_size);
		return _get_multiplier_impl_qutrit(step, addr, data, mem);
	}


	ptrdiff_t TimeStep::_get_multiplier_impl_qutrit(
		size_t step, size_t addr, size_t data, const memory_t& mem) const
	{
		ptrdiff_t countret = 0;
		for (size_t i = 0; i < addr_size; ++i)
		{
			if (get_digit_reverse(addr, i, addr_size)) {
				{
					ptrdiff_t begin = 2 * i + 1;
					ptrdiff_t end = begin + i + 1;
					if (ptrdiff_t(step) < begin) continue;
					// if (step >= end) step = end;
					end = std::min((ptrdiff_t)(step), end);
					countret += (end - begin);
				}
				{
					ptrdiff_t end = out(2 * i + 1);
					ptrdiff_t begin = end - i - 1;
					if (ptrdiff_t(step) < begin) continue;
					end = std::min((ptrdiff_t)(step), end);
					countret += (end - begin);
				}
			}
		}
		for (size_t i = 0; i < data_size; ++i)
		{
			ptrdiff_t begin, end;
			bool b_in = (data >> i) & 1;
			bool b_m = (mem[addr] >> i) & 1;
			bool b_out = b_in ^ b_m;
			ptrdiff_t temp = 2 * addr_size + i * 2 + 1;
			if (b_in) {
				begin = temp;
			}
			else {
				begin = temp + addr_size;
			}
			if (b_out) {
				end = temp + 2 * addr_size;
			}
			else {
				end = temp + addr_size;
			}

			if (ptrdiff_t(step) < begin) continue;
			// if (step >= end) step = end;
			end = std::min((ptrdiff_t)(step), end - 1);
			countret += (end - begin + 1);
		}
		return countret;
	}


	std::vector<bool> calc_pos(int pos, int layer) {
		std::vector<bool> posv;
		posv.reserve(layer);

		for (int i = 0; i < layer; i++) {
			posv.push_back(get_digit(pos, i));
		}
		return posv;
	}

	std::vector<size_t> get_nodes_in_layer(int layer) {
		// layer -= 1;
		size_t begin = pow2(layer) - 1;
		size_t end = begin + pow2(layer);
		std::vector<size_t> ret;
		ret.resize(end - begin);
		for (size_t i = begin; i < end; ++i) {
			ret[i - begin] = i;
		}
		return ret;
	}

	std::string pos2str(int pos, int layer) {
		std::vector<bool> posv = calc_pos(pos, layer);
		std::string ret;
		for (bool i : posv) {
			char x = i ? '1' : '0';
			ret = x + ret;
		}
		return ret;
	}

	std::string type2str(OperationType type) {
		const static std::map<OperationType, std::string> _TypeNameMap = {
			{OperationType::ControlSwap, "cSwap"},
			{OperationType::HadamardData, "Hadamard"},
			{OperationType::SwapInternal, "Swap"},
			{OperationType::FirstCopy, "ACopy"},
			{OperationType::FetchData, "FetchData"},
			{OperationType::CopyIn, "CopyIn"},
			{OperationType::CopyOut, "CopyOut"},
			{OperationType::SetZero, "SetZero"},
			{OperationType::Damping, "Damping"},
			{OperationType::Damp_Full, "Damp_Full"},
			{OperationType::Damp_Common, "Damp_Common"},
			{OperationType::BitFlip, "BitFlip"},
			{OperationType::PhaseFlip, "PhaseFlip"},
			{OperationType::BitPhaseFlip, "BitPhaseFlip"},
			{OperationType::Depolarizing, "Depolarizing"},
		};
		if (type > OperationType::Begin && type < OperationType::End)
			return _TypeNameMap.at(type);
		else
			return "Unknown";
	}

} // namespace qram_simulator