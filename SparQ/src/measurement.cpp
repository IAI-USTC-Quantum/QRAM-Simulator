#include "measurement.h"

namespace qram_simulator
{
	namespace
	{
		/**
		 * @brief 校验寄存器 ID 存在且处于激活状态
		 * @throws invalid_argument ID 越界或已被移除/未激活
		 */
		void validate_register_id(size_t id, const char* context)
		{
			if (id == SIZE_MAX || id >= System::name_register_map.size())
				throw_invalid_input(
					std::string(context) + ": register id " + std::to_string(id) +
					" is out of range (no such register).");

			if (!System::status_of(id))
				throw_invalid_input(
					std::string(context) + ": register '" + std::string(System::name_of(id)) +
					"' (id " + std::to_string(id) + ") is not active (it may have been removed).");
		}

		/**
		 * @brief 按名称解析寄存器 ID，并校验其存在且激活
		 * @throws invalid_argument 名称未找到（`System::get` 对未知/未激活
		 *         名称返回 `SIZE_MAX`，这里转换为一个可捕获的显式异常）
		 */
		size_t resolve_register(std::string_view name, const char* context)
		{
			size_t id = System::get(name);
			if (id == SIZE_MAX)
				throw_invalid_input(
					std::string(context) + ": register '" + std::string(name) + "' not found.");
			return id;
		}

		/**
		 * @brief 校验寄存器 ID 列表中没有重复项
		 * @throws invalid_argument 存在重复的寄存器 ID
		 */
		void validate_unique_registers(const std::vector<size_t>& ids, const char* context)
		{
			std::set<size_t> seen(ids.begin(), ids.end());
			if (seen.size() != ids.size())
				throw_invalid_input(
					std::string(context) + ": duplicate register id in the same register list.");
		}

		/**
		 * @brief 校验一个值能被寄存器的位宽表示
		 * @details `size_of(id) == 64` 时任意 `uint64_t` 都合法（避免
		 *          `1ull << 64` 的未定义行为）。
		 * @throws invalid_argument 值超出 `[0, 2^size_of(id))` 范围
		 */
		void validate_value_fits_register(size_t id, uint64_t value, const char* context)
		{
			size_t width = System::size_of(id);
			if (width >= 64)
				return;

			uint64_t limit = pow2(width);
			if (value >= limit)
				throw_invalid_input(
					std::string(context) + ": value " + std::to_string(value) +
					" does not fit in register '" + std::string(System::name_of(id)) +
					"' (width " + std::to_string(width) + ", valid range [0, " +
					std::to_string(limit) + ")).");
		}

		/**
		 * @brief 解析并校验一组寄存器名称，返回其 ID 列表（要求互不重复）
		 */
		std::vector<size_t> resolve_and_validate(const std::vector<std::string>& names, const char* context)
		{
			std::vector<size_t> ids;
			ids.reserve(names.size());
			for (const auto& name : names)
				ids.push_back(resolve_register(name, context));
			validate_unique_registers(ids, context);
			return ids;
		}

		/**
		 * @brief 校验一组寄存器 ID（均存在、激活、互不重复）
		 */
		void validate_ids(const std::vector<size_t>& ids, const char* context)
		{
			for (size_t id : ids)
				validate_register_id(id, context);
			validate_unique_registers(ids, context);
		}
	}

	// ------------------------------------------------------------------
	// MeasureZ
	// ------------------------------------------------------------------

	MeasureZ::MeasureZ(const std::vector<std::string>& register_names) :
		registers(resolve_and_validate(register_names, "MeasureZ"))
	{
	}

	MeasureZ::MeasureZ(const std::vector<size_t>& register_ids) :
		registers(register_ids)
	{
		validate_ids(registers, "MeasureZ");
	}

	MeasureZ::MeasureZ(std::string_view register_name) :
		registers{ resolve_register(register_name, "MeasureZ") }
	{
	}

	MeasureZ::MeasureZ(size_t register_id) :
		registers{ register_id }
	{
		validate_register_id(register_id, "MeasureZ");
	}

	std::pair<std::vector<uint64_t>, double> MeasureZ::operator()(std::vector<System>& state) const
	{
		profiler _("MeasureZ");

		if (state.empty())
			throw_invalid_input("MeasureZ: cannot measure an empty state.");

		// 0. Validate the input state's total Born-rule probability before
		//    sampling. Sampling `random_engine::uniform01()` against `[0, 1)`
		//    silently assumes `sum(|amplitude|^2) == 1`; if that assumption
		//    is violated (e.g. by an un-renormalized caller), the cumulative
		//    subtraction loop below either always exits early (total > 1,
		//    biasing towards early branches) or always falls through to the
		//    last-branch fallback (total < 1, biasing towards the last
		//    branch) without ever raising an error. Reject that case
		//    explicitly instead.
		double total_prob = 0.0;
		for (const auto& s : state)
			total_prob += abs_sqr(s.amplitude);

		if (!std::isfinite(total_prob) || total_prob <= 0.0 || !ignorable(total_prob - 1.0, kNormalizationThreshold))
			throw_bad_result(
				"MeasureZ: input state is not normalized (total probability = " +
				std::to_string(total_prob) + ", expected within " +
				std::to_string(kNormalizationThreshold) + " of 1.0). MeasureZ requires a "
				"normalized state; renormalize (e.g. via Normalize()) before measuring.");

		if (registers.empty())
			return { {}, 1.0 };

		// 1. Sample an outcome according to the Born rule using the
		//    (seedable) global random engine.
		double r = random_engine::uniform01();
		std::vector<uint64_t> outcome(registers.size(), 0);
		bool selected = false;

		for (const auto& s : state)
		{
			double prob = abs_sqr(s.amplitude);

			if (r < prob)
			{
				for (size_t i = 0; i < registers.size(); ++i)
					outcome[i] = s.get(registers[i]).value;
				selected = true;
				break;
			}
			r -= prob;
		}

		// Numerical fallback: floating-point round-off may leave a small
		// positive residual after the loop above even for a validated,
		// (near-)normalized state. Fall back to the last branch so that
		// measurement always returns a definite outcome.
		if (!selected)
		{
			const auto& s = state.back();
			for (size_t i = 0; i < registers.size(); ++i)
				outcome[i] = s.get(registers[i]).value;
		}

		// 2. Collapse the state onto the sampled outcome and renormalize.
		double sum_prob = 0;
		auto pred = [this, &outcome, &sum_prob](const System& s)
			{
				for (size_t i = 0; i < registers.size(); ++i)
				{
					if (s.get(registers[i]).value != outcome[i])
						return true;
				}
				sum_prob += abs_sqr(s.amplitude);
				return false;
			};

		state.erase(std::remove_if(state.begin(), state.end(), pred), state.end());

		if (sum_prob <= 0 || state.empty())
			throw_bad_result("MeasureZ: sampled outcome has zero support; the input state was not normalized.");

		double inv_norm = 1.0 / std::sqrt(sum_prob);
		std::for_each(state.begin(), state.end(),
			[inv_norm](System& s) { s.amplitude *= inv_norm; });

		return { outcome, sum_prob };
	}

	// ------------------------------------------------------------------
	// Reset
	// ------------------------------------------------------------------

	namespace
	{
		std::vector<uint64_t> default_targets(size_t count)
		{
			return std::vector<uint64_t>(count, 0);
		}

		void validate_targets_fit(const std::vector<size_t>& ids, const std::vector<uint64_t>& values, const char* context)
		{
			for (size_t i = 0; i < ids.size(); ++i)
				validate_value_fits_register(ids[i], values[i], context);
		}
	}

	Reset::Reset(const std::vector<std::string>& register_names) :
		Reset(register_names, default_targets(register_names.size()))
	{
	}

	Reset::Reset(const std::vector<std::string>& register_names, const std::vector<uint64_t>& targets)
	{
		if (register_names.size() != targets.size())
			throw_invalid_input("Reset: register_names and targets must have the same length.");

		registers = resolve_and_validate(register_names, "Reset");
		target_values = targets;
		validate_targets_fit(registers, target_values, "Reset");
	}

	Reset::Reset(const std::vector<size_t>& register_ids) :
		Reset(register_ids, default_targets(register_ids.size()))
	{
	}

	Reset::Reset(const std::vector<size_t>& register_ids, const std::vector<uint64_t>& targets) :
		registers(register_ids), target_values(targets)
	{
		if (registers.size() != target_values.size())
			throw_invalid_input("Reset: register_ids and targets must have the same length.");

		validate_ids(registers, "Reset");
		validate_targets_fit(registers, target_values, "Reset");
	}

	Reset::Reset(std::string_view register_name, uint64_t target) :
		registers{ resolve_register(register_name, "Reset") }, target_values{ target }
	{
		validate_targets_fit(registers, target_values, "Reset");
	}

	Reset::Reset(size_t register_id, uint64_t target) :
		registers{ register_id }, target_values{ target }
	{
		validate_register_id(register_id, "Reset");
		validate_targets_fit(registers, target_values, "Reset");
	}

	std::vector<uint64_t> Reset::operator()(std::vector<System>& state) const
	{
		profiler _("Reset");

		if (registers.empty())
			return {};

		// 1. Measure (collapse + renormalize) — this is the only physically
		//    valid way to force a possibly-superposed register to a definite
		//    classical value.
		auto [outcome, prob] = MeasureZ(registers)(state);

		// 2. After the measurement, every remaining branch shares the same
		//    `outcome` value for each measured register, so overwriting it
		//    to `target_values` is a well-defined deterministic correction
		//    (equivalent to a classically-conditioned bit flip) that cannot
		//    collide with, or require merging into, any other branch.
		std::for_each(state.begin(), state.end(),
			[this](System& s)
			{
				for (size_t i = 0; i < registers.size(); ++i)
					s.get(registers[i]).value = target_values[i];
			});

		return outcome;
	}

	// ------------------------------------------------------------------
	// Probability
	// ------------------------------------------------------------------

	Probability::Probability(const std::map<std::string_view, uint64_t>& assignments)
	{
		registers.reserve(assignments.size());
		values.reserve(assignments.size());
		for (auto&& [k, v] : assignments)
		{
			registers.push_back(resolve_register(k, "Probability"));
			values.push_back(v);
		}
		validate_targets_fit(registers, values, "Probability");
	}

	Probability::Probability(const std::map<size_t, uint64_t>& assignments)
	{
		registers.reserve(assignments.size());
		values.reserve(assignments.size());
		for (auto&& [k, v] : assignments)
		{
			registers.push_back(k);
			values.push_back(v);
		}
		validate_ids(registers, "Probability");
		validate_targets_fit(registers, values, "Probability");
	}

	Probability::Probability(const std::vector<std::string>& register_names, const std::vector<uint64_t>& target_values)
	{
		if (register_names.size() != target_values.size())
			throw_invalid_input("Probability: register_names and target_values must have the same length.");

		registers = resolve_and_validate(register_names, "Probability");
		values = target_values;
		validate_targets_fit(registers, values, "Probability");
	}

	Probability::Probability(const std::vector<size_t>& register_ids, const std::vector<uint64_t>& target_values) :
		registers(register_ids), values(target_values)
	{
		if (registers.size() != values.size())
			throw_invalid_input("Probability: register_ids and target_values must have the same length.");

		validate_ids(registers, "Probability");
		validate_targets_fit(registers, values, "Probability");
	}

	Probability::Probability(std::string_view register_name, uint64_t value) :
		registers{ resolve_register(register_name, "Probability") }, values{ value }
	{
		validate_targets_fit(registers, values, "Probability");
	}

	Probability::Probability(size_t register_id, uint64_t value) :
		registers{ register_id }, values{ value }
	{
		validate_register_id(register_id, "Probability");
		validate_targets_fit(registers, values, "Probability");
	}

	double Probability::operator()(const std::vector<System>& state) const
	{
		profiler _("Probability");

		if (registers.empty())
			return 1.0;

		double sum_prob = 0;
		for (const auto& s : state)
		{
			bool matches = true;
			for (size_t i = 0; i < registers.size(); ++i)
			{
				if (s.get(registers[i]).value != values[i])
				{
					matches = false;
					break;
				}
			}
			if (matches)
				sum_prob += abs_sqr(s.amplitude);
		}
		return sum_prob;
	}

	std::map<uint64_t, double> Probability::distribution(const std::vector<System>& state, size_t register_id)
	{
		validate_register_id(register_id, "Probability::distribution");

		std::map<uint64_t, double> dist;
		for (const auto& s : state)
			dist[s.get(register_id).value] += abs_sqr(s.amplitude);
		return dist;
	}

	std::map<uint64_t, double> Probability::distribution(const std::vector<System>& state, std::string_view register_name)
	{
		return distribution(state, resolve_register(register_name, "Probability::distribution"));
	}
}
