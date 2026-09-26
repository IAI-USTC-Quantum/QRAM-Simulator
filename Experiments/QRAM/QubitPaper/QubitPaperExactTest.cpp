/* Seed-by-seed equivalence of pruned and full qubit-QRAM trajectories.
 *
 * Compare complete input-weighted complex components before and after tree
 * measurement, address probabilities, fidelity, the sampled tree, operation
 * history and RNG state. Numerical tolerances account for floating-point
 * accumulation order; they do not approximate the simulated evolution.
 *
 * The default battery contains 155 pinned, stress and zero-noise cases across
 * two input conventions and must fire at least 50 jumps. Fixed operation
 * histories additionally verify reference death, survival and circuit reuse.
 * --channels covers five noise channels, three bus widths and nonuniform input.
 * Historical seeds remain stress inputs; candidate-domain and joint-sampler
 * changes intentionally change their old jump outcomes. Fixed-history fixtures
 * enforce projection coverage on every platform. Compare actual joint Kraus
 * labels, not only the number of fired jumps.
 *
 * --case n eps seed [uniform] replays one case. --battery-shard i count and
 * --perf-shard i count partition independent trajectories across processes.
 * The performance grid contains 1000 cases with the paper's exact input and
 * seed conventions. See the bilingual paper/exactness_validation.md guide.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "logger.h"
#include "state_manipulator.h"
#include "time_step.h"
#include "qram_circuit_qubit.h"

using namespace qram_simulator;

namespace {

constexpr size_t k_data_bits = 3;
constexpr seed_t k_base_seed = 123456789;
constexpr size_t k_input_branches = 500;   /* perf_scan input convention */

constexpr double k_weight_tol = 1e-12;
constexpr double k_fid_tol = 1e-9;
constexpr double k_state_tol = 1e-12;
constexpr size_t k_min_total_fires = 50;

int g_failures = 0;

void check(bool ok, const std::string& name, const std::string& detail)
{
	std::cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << name
		<< " (" << detail << ")\n";
	if (!ok) ++g_failures;
}

void set_full_coverage_zero_bus(qram_qubit::QRAMCircuit& qram, size_t n, size_t k)
{
	auto& groups = qram.get_branch_groups();
	groups.clear();
	for (size_t addr = 0; addr < pow2(n); ++addr) {
		groups.emplace_back(addr);
		auto& g = groups.back();
		g.branches_input.emplace_back(addr, k, static_cast<bus_t>(0));
		g.branch_probs.push_back(1.0 / pow2(n));
		g.state_probs.push_back(1.0 / pow2(n));
	}
}

std::map<OperationType, double> make_noise(double eps)
{
	std::map<OperationType, double> noise;
	if (eps > 0.0) {
		noise[OperationType::Depolarizing] = eps;
		noise[OperationType::Damping] = eps;
	}
	return noise;
}

qram_qubit::QRAMCircuit configure_qram(size_t n, const std::map<OperationType, double>& noise,
	size_t k = k_data_bits)
{
	random_engine::get_instance().set_seed(k_base_seed);
	qram_qubit::QRAMCircuit qram(n, k);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	set_full_coverage_zero_bus(qram, n, k);
	return qram;
}

/* The perf_scan input convention: uniform random input branches instead of
 * the zero-bus full-coverage task. Different good/bad compositions, so both
 * conventions belong in the battery. */
qram_qubit::QRAMCircuit configure_qram_uniform(size_t n, const std::map<OperationType, double>& noise,
	size_t k = k_data_bits, bool random_weights = false)
{
	random_engine::get_instance().set_seed(k_base_seed);
	qram_qubit::QRAMCircuit qram(n, k);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	if (random_weights) qram.set_input_random(k_input_branches);
	else qram.set_input_uniform(k_input_branches);
	return qram;
}

seed_t derive_run_seed(size_t trajectory)
{
	random_engine::get_instance().set_seed(k_base_seed + trajectory);
	return random_engine::get_instance().reseed();
}

/* Physical address probability includes each input branch's weight. */
std::vector<double> address_weights(const qram_qubit::QRAMCircuit& qram, size_t n)
{
	std::vector<double> w(pow2(n), 0.0);
	for (auto& g : qram.branch_groups) {
		for (size_t bid = 0; bid < g.branches.size(); ++bid)
			for (auto it = g.branches[bid].iterbeg(); it != g.branches[bid].iterend(); ++it)
				w[g.address] += g.branch_probs[bid] * std::norm(it->amplitude);
	}
	return w;
}

/* Preserve the input-branch label as well as the complete output basis key.
 * This checks each column of the trajectory map, including complex phases
 * and residual tree excitations, rather than only one fidelity statistic. */
using ComponentKey = std::tuple<size_t, bus_t, bus_t, qram_qubit::State::element_type>;
using Components = std::map<ComponentKey, complex_t>;

Components components(const qram_qubit::QRAMCircuit& qram)
{
	Components out;
	for (const auto& g : qram.branch_groups) {
		for (size_t bid = 0; bid < g.branches.size(); ++bid) {
			const auto& b = g.branches[bid];
			const double input_amp = std::sqrt(g.branch_probs[bid]);
			for (auto it = b.iterbeg(); it != b.iterend(); ++it)
				out[{g.address, b.bus_input, it->data_bus, it->state.nz_elements}]
					+= input_amp * it->amplitude;
		}
	}
	return out;
}

double component_error(const Components& lhs, const Components& rhs)
{
	double error = 0;
	auto l = lhs.begin();
	auto r = rhs.begin();
	while (l != lhs.end() || r != rhs.end()) {
		if ((l != lhs.end() && !std::isfinite(std::abs(l->second)))
			|| (r != rhs.end() && !std::isfinite(std::abs(r->second))))
			return std::numeric_limits<double>::infinity();
		if (r == rhs.end() || (l != lhs.end() && l->first < r->first)) {
			error = std::max(error, std::abs(l++->second));
		} else if (l == lhs.end() || r->first < l->first) {
			error = std::max(error, std::abs(r++->second));
		} else {
			error = std::max(error, std::abs(l->second - r->second));
			++l;
			++r;
		}
	}
	return error;
}

void materialize(qram_qubit::QRAMCircuit& qram, double gamma)
{
	if (gamma > 0 && qram.first_good_branch_group >= 0)
		qram.time_step.get_multiplier_qubit(gamma, qram.time_step.last_step(),
			qram.branch_groups, qram.first_good_branch_group, qram.good_branch_group_ids);
	qram.materialize_good_branches();
}

struct Case {
	double eps;
	size_t n;
	seed_t run_seed;
	bool require_fire;   /* the trajectory must fire at least one K1 jump */
	bool scan_input;     /* perf_scan uniform-branch convention (else zero-bus full coverage) */
	const char* tag;
	size_t k = k_data_bits;
	bool random_weights = false;
	// An empty optional selects the mixed-noise convention.
	std::optional<OperationType> channel;
};

std::vector<Case> build_battery()
{
	std::vector<Case> cases;

	// Pinned projection cases with full-address, zero-bus input.
	const struct { double eps; size_t n; seed_t seed; } projection_cases[] = {
		{1e-4, 10, 2090429186520196096ULL},
		{1e-4, 10, 9208654621621431296ULL},
		{1e-3, 6, 7744644691974779904ULL},
		{1e-3, 6, 3874279414526882816ULL},
		{1e-3, 8, 4101528895484906496ULL},
		{1e-3, 8, 3886007424377267200ULL},
		{1e-3, 8, 4210972589829411840ULL},
		{1e-3, 8, 9208654621621431296ULL},
		{1e-3, 10, 8975687211725193216ULL},
		{1e-3, 10, 6989413129363354624ULL},
		{1e-3, 10, 1207751192743994368ULL},
		{1e-3, 12, 8975687211725193216ULL},
		{1e-3, 12, 5681480801414385664ULL},
		{1e-3, 12, 1207751192743994368ULL},
	};
	for (const auto& c : projection_cases)
		cases.push_back({c.eps, c.n, c.seed, true, false, "projection"});

	// Pinned low-event controls.
	const seed_t controls[] = {
		3217773524253467648ULL,
		5244930501339431936ULL,
		6798469447420720128ULL,
	};
	for (seed_t s : controls)
		cases.push_back({1e-3, 4, s, false, false, "ctrl"});

	/* 3) fired-jump stress grid (same seed derivation as perf_scan).
	   Per-case fire expectations would be flaky here (single trajectories
	   legitimately fire zero jumps); the battery-level total below enforces
	   that the fired-jump path is exercised. */
	for (size_t traj = 0; traj < 8; ++traj) {
		seed_t s = derive_run_seed(traj);
		for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)})
			cases.push_back({1e-3, n, s, false, false, "stress"});
		for (size_t n : {size_t(4), size_t(6), size_t(8)})
			cases.push_back({1e-2, n, s, false, false, "stress"});
	}

	/* 4) low-noise and noise-free controls */
	for (size_t traj = 0; traj < 4; ++traj) {
		seed_t s = derive_run_seed(100 + traj);
		for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)}) {
			cases.push_back({0.0, n, s, false, false, "quiet"});
			cases.push_back({1e-5, n, s, false, false, "quiet"});
		}
	}

	// Pinned excited-reference cases with uniform input branches. A jump
	// projects the reference and the corresponding good components together;
	// surviving reference components must remain available for prediction.
	const struct { size_t n; seed_t seed; } survival_cases[] = {
		{10, 7744644691974779904ULL},
		{12, 7744644691974779904ULL},
		{12, 1183008555922881536ULL},
		{12, 3227135189013113856ULL},
		{12, 3287329390135220224ULL},
		{12, 4366419402265368576ULL},
	};
	for (const auto& h : survival_cases)
		cases.push_back({1e-3, h.n, h.seed, true, true, "survival"});

	/* 6) stress grid in the perf_scan uniform-branch convention */
	for (size_t traj = 0; traj < 4; ++traj) {
		seed_t s = derive_run_seed(traj);
		for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)})
			cases.push_back({1e-3, n, s, false, true, "scanstress"});
		for (size_t n : {size_t(4), size_t(6)})
			cases.push_back({1e-2, n, s, false, true, "scanstress"});
	}

	return cases;
}

std::vector<Case> channel_battery()
{
	std::vector<Case> cases;
	for (OperationType channel : {OperationType::Damping, OperationType::Depolarizing,
		OperationType::BitFlip, OperationType::PhaseFlip, OperationType::BitPhaseFlip})
		for (size_t n : {size_t(3), size_t(5)})
			for (size_t k : {size_t(1), size_t(2), size_t(4)})
				for (size_t trajectory = 0; trajectory < 4; ++trajectory) {
					Case c{trajectory < 2 ? 1e-3 : 1e-2, n,
						derive_run_seed(200 + trajectory), false, true, "channel"};
					c.k = k;
					c.random_weights = (trajectory % 2) != 0;
					c.channel = channel;
					cases.push_back(c);
				}
	return cases;
}

/* runs one case in both modes; returns true iff all checks pass */
bool run_case(const Case& c, size_t& pruned_fires)
{
	auto noise = make_noise(c.eps);
	if (c.channel) noise = {{*c.channel, c.eps}};
	const double gamma = noise.count(OperationType::Damping) ? noise.at(OperationType::Damping) : 0;

	auto qf = c.scan_input ? configure_qram_uniform(c.n, noise, c.k, c.random_weights)
		: configure_qram(c.n, noise, c.k);
	random_engine::get_instance().set_seed(c.run_seed);
	qf.run(qram_qubit::QRAMCircuit::FULL_VER);
	const auto full_rng = random_engine::get_engine();
	const auto full_before = components(qf);
	double f_full = qf.sample_and_get_fidelity();
	/* weights AFTER sample_and_get_fidelity: sampling collapses both modes
	   onto the same tree configuration (identical RNG streams) and
	   normalizes, which is the comparable state — before sampling, the
	   pruned mode's good groups still hold their un-materialized inputs */
	auto wf = address_weights(qf, c.n);
	const auto full_after = components(qf);
	const auto full_final_rng = random_engine::get_engine();

	auto qp = c.scan_input ? configure_qram_uniform(c.n, noise, c.k, c.random_weights)
		: configure_qram(c.n, noise, c.k);
	random_engine::get_instance().set_seed(c.run_seed);
	qp.run(qram_qubit::QRAMCircuit::NORMAL_VER);
	const bool run_rng_agrees = full_rng == random_engine::get_engine();
	materialize(qp, gamma);
	const double before_error = component_error(full_before, components(qp));
	double f_pruned = qp.sample_and_get_fidelity();
	const double after_error = component_error(full_after, components(qp));
	const bool final_rng_agrees = full_final_rng == random_engine::get_engine();
	const bool tree_agrees = qf.final_system_state == qp.final_system_state;
	auto wp = address_weights(qp, c.n);
	pruned_fires = qp.fired_jump_count;

	double dw = 0;
	for (size_t a = 0; a < pow2(c.n); ++a)
		dw = std::max(dw, std::abs(wf[a] - wp[a]));
	double df = std::abs(f_full - f_pruned);
	bool fires_agree = qf.fired_jump_count == qp.fired_jump_count;
    bool outcomes_agree = qf.damping_history.size() == qp.damping_history.size();
    if (outcomes_agree) for (size_t i = 0; i < qf.damping_history.size(); ++i) {
        const auto& f = qf.damping_history[i];
        const auto& p = qp.damping_history[i];
        outcomes_agree &= f.step == p.step && f.candidates == p.candidates && f.jumps == p.jumps;
    }
    const bool history_agrees = qf.operations.to_string() == qp.operations.to_string() && outcomes_agree;
	const bool normalized = std::abs(std::accumulate(wp.begin(), wp.end(), 0.0) - 1)
		<= k_weight_tol;

	if (dw > k_weight_tol) {
		/* dump the worst addresses to make the discrepancy diagnosable */
		std::vector<std::pair<double, size_t>> order;
		for (size_t a = 0; a < pow2(c.n); ++a)
			order.push_back({std::abs(wf[a] - wp[a]), a});
		std::sort(order.begin(), order.end(), [](const auto& x, const auto& y)
			{ return x.first > y.first; });
		for (size_t i = 0; i < 8 && i < order.size() && order[i].first > 0; ++i)
			std::cout << "    addr " << order[i].second << ": full="
				<< wf[order[i].second] << " pruned=" << wp[order[i].second]
				<< " ratio=" << (wf[order[i].second] != 0
					? wp[order[i].second] / wf[order[i].second] : 0.0) << "\n";
	}

	std::cout << c.tag << " eps=" << c.eps << " n=" << c.n
		<< " k=" << c.k << " seed=" << c.run_seed
		<< " fires=" << qp.fired_jump_count << "/" << qf.fired_jump_count
		<< " dW=" << dw << " dF=" << df
		<< " dState=" << before_error << "/" << after_error;
	std::cout << "\n";

	const std::string tag(c.tag);
	check(dw <= k_weight_tol, "per-address weights", tag);
	check(df <= k_fid_tol, "fidelity", tag);
	check(before_error <= k_state_tol, "complete state before measurement", tag);
	check(after_error <= k_state_tol, "complete state after measurement", tag);
	check(tree_agrees, "sampled tree configuration", tag);
	check(run_rng_agrees && final_rng_agrees, "RNG stream position", tag);
	check(history_agrees, "sampled operation history", tag);
	check(normalized, "normalized output weight", tag);
	if (c.eps > 0.0)
		check(fires_agree, "fired-jump counts agree", tag);
    // Historical individual-seed jump expectations belong to the retired
    // sampler. Coverage is enforced by fixed histories and the total-jump floor.
    const bool require_fire = false;
	if (require_fire && pruned_fires == 0)
		check(false, "expected at least one fired jump", tag);

	return dw <= k_weight_tol && df <= k_fid_tol && fires_agree
		&& before_error <= k_state_tol && after_error <= k_state_tol
		&& tree_agrees && run_rng_agrees && final_rng_agrees && history_agrees && normalized
		&& (!require_fire || pruned_fires > 0);
}

/* Fixed operation histories keep the two regression mechanisms portable:
 * death: a data jump on the left route after bus injection kills every good
 *        (right-route) branch while an excited bad component survives;
 * survival: X then K1 at an idle left-child slot leaves all good branches
 *           alive, so an unconditional kill-on-fire would be detected.
 * No stochastic noise distribution is involved in constructing the history. */
qram_qubit::QRAMCircuit forced_history(bool pruned, bool survives)
{
	auto qram = configure_qram(3, {});
	qram.prepare_all();
	constexpr size_t position = 3; // left child's data slot
	qram.time_step.fill_bad_range(position, arch_qubit);
	if (pruned) {
		qram.valid_branch_group_view.clear();
		for (size_t id = 0; id < qram.branch_groups.size(); ++id) {
			auto& group = qram.branch_groups[id];
			if (!qram.time_step.is_bad_branch(group.address)) {
				if (qram.first_good_branch_group < 0) {
					qram.first_good_branch_group = id;
				} else {
					group.set_good(&qram.branch_groups[qram.first_good_branch_group]);
					qram.good_branch_group_ids.push_back(id);
					continue;
				}
			}
			qram.valid_branch_group_view.push_back(&group);
		}
	}
	const size_t step = survives ? 3 : 8;
	auto& pack = qram.operations.time_slices[step - 1];
	if (survives)
		pack.append({OperationType::BitFlip, {position}});
	pack.append({OperationType::Damp_Full, {position}, {0.0}});
	return qram;
}

void check_forced_histories()
{
	for (bool survives : {false, true}) {
		const std::string tag = survives ? "forced survival" : "forced death";
		bool exercised = false;
		// Search only the roulette uniform, never a platform-specific noise
		// distribution. Keep the first fired history for all state comparisons.
		for (seed_t seed = 0; seed < 256 && !exercised; ++seed) {
			auto full = forced_history(false, survives);
			random_engine::get_instance().set_seed(seed);
			full.run_bad();
			if (full.fired_jump_count == 0) continue;
			exercised = true;
			const auto full_rng = random_engine::get_engine();
			const auto before = components(full);
			const auto fid = full.sample_and_get_fidelity();
			const auto after = components(full);

			auto pruned = forced_history(true, survives);
			random_engine::get_instance().set_seed(seed);
			pruned.run_bad();
			check(pruned.fired_jump_count == 1, "exactly one forced jump", tag);
			check(full_rng == random_engine::get_engine(), "forced RNG parity", tag);
			const auto& ref = pruned.branch_groups[pruned.first_good_branch_group];
			check((ref.get_prob() > 0) == survives, "reference survival matches fixture", tag);
			materialize(pruned, 0);
			check(component_error(before, components(pruned)) <= k_state_tol,
				"forced complete state before measurement", tag);
			for (size_t id : pruned.good_branch_group_ids) {
				const auto& group = pruned.branch_groups[id];
				check((group.get_prob() > 0) == survives, "predicted group survival", tag);
			}
			check(std::abs(fid - pruned.sample_and_get_fidelity()) <= k_fid_tol,
				"forced fidelity", tag);
			check(full.final_system_state == pruned.final_system_state,
				"forced sampled tree", tag);
			check(component_error(after, components(pruned)) <= k_state_tol,
				"forced complete state after measurement", tag);

			// Reuse the same objects after projection. Input weights and the
			// annihilation flag must reset on the next query.
			random_engine::get_instance().set_seed(42);
			full.run_full();
			const double repeat_fid = full.sample_and_get_fidelity();
			random_engine::get_instance().set_seed(42);
			pruned.run_normal();
			check(std::abs(repeat_fid - pruned.sample_and_get_fidelity()) <= k_fid_tol,
				"reused circuit fidelity", tag);
			check(component_error(components(full), components(pruned)) <= k_state_tol,
				"reused circuit complete state", tag);
		}
		check(exercised, "forced jump path exercised", tag);
	}
}

} // namespace

int main(int argc, char** argv)
{
	std::cout << std::unitbuf;
	std::cout << "pruned-vs-full exactness regression battery\n";
	auto cases = build_battery();
	const bool single_case = argc >= 5 && std::string(argv[1]) == "--case";
	const bool fixtures_only = argc == 2 && std::string(argv[1]) == "--fixtures";
	const bool channels_only = argc == 2 && std::string(argv[1]) == "--channels";
	const bool perf_shard = argc == 4 && std::string(argv[1]) == "--perf-shard";
	const bool battery_shard = argc == 4 && std::string(argv[1]) == "--battery-shard";
	if (argc != 1 && !single_case && !fixtures_only && !channels_only
		&& !perf_shard && !battery_shard) {
		std::cerr << "usage: QubitPaperExactTest [--fixtures|--channels|"
			"--case n eps seed [uniform]|--battery-shard i count|--perf-shard i count]\n";
		return 2;
	}
	if (single_case) {
		cases = {{std::stod(argv[3]), static_cast<size_t>(std::stoull(argv[2])),
			static_cast<seed_t>(std::stoull(argv[4])), false,
			argc > 5 && std::string(argv[5]) == "uniform", "replay"}};
	}
	if (fixtures_only) cases.clear();
	if (channels_only) cases = channel_battery();
	if (perf_shard) {
		cases.clear();
		for (double eps : {0.0, 1e-5, 1e-4, 1e-3})
			for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)})
				for (size_t trajectory = 0; trajectory < 50; ++trajectory)
					cases.push_back({eps, n, derive_run_seed(trajectory), false, false,
						"perf"});
	}
	if (perf_shard || battery_shard) {
		const size_t shard = std::stoull(argv[2]);
		const size_t count = std::stoull(argv[3]);
		if (count == 0 || shard >= count) return 2;
		std::vector<Case> selected;
		for (size_t i = shard; i < cases.size(); i += count) selected.push_back(cases[i]);
		cases = std::move(selected);
	}
	if (!single_case && !perf_shard && !battery_shard && !channels_only) check_forced_histories();

	size_t passed = 0, total_fires = 0;
	for (const Case& c : cases) {
		size_t fires = 0;
		if (run_case(c, fires)) ++passed;
		total_fires += fires;
	}

	std::cout << "\n" << passed << "/" << cases.size() << " case(s) exact\n";
	if (!single_case && !fixtures_only && !perf_shard && !battery_shard && !channels_only)
		check(total_fires >= k_min_total_fires, "battery exercised the fired-jump path",
			"total fires " + std::to_string(total_fires)
			+ " >= " + std::to_string(k_min_total_fires));
	std::cout << g_failures << " assertion(s) failed\n";
	if (g_failures) {
		std::cout << "EXACTNESS REGRESSION FAILED\n";
		return 1;
	}
	std::cout << "EXACTNESS REGRESSION PASSED\n";
	return 0;
}
