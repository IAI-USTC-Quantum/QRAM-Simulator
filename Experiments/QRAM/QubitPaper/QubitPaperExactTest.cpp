/* Regression test: the pruned (NORMAL_VER) trajectory must reproduce the full
 * (FULL_VER) trajectory exactly, seed by seed.
 *
 * The pruning algorithm never evolves the unmarked good branch groups; their
 * contribution is reconstructed from the reference good branch by the XOR
 * mirror (materialize_good_branches). A fired damping (K1) jump acts on a
 * good branch exactly as on the reference: a good branch excited at the
 * fired node decays in lockstep with the reference and the mirror already
 * reproduces that, while a reference that is fully annihilated implies every
 * good branch is annihilated too. The simulator must therefore propagate
 * reference death to the reconstruction; historically it silently skipped
 * materialization instead, which showed up as 14/1000 seed-level fidelity
 * mismatches in perf_scan.csv. A blanket mid-run "kill all good groups on
 * fire" marker (an earlier fix attempt) is WRONG and broke 6 more
 * trajectories in which the reference survived the fire -- see the KNOWN
 * TRAP note in QRAMCircuit::run_damp_full.
 *
 * Batteries (both input conventions of the paper experiments: zero-bus full
 * coverage and perf_scan uniform branches):
 *   1. the 14 historically mismatching seeds plus 3 matched controls,
 *   2. a fired-jump stress grid (eps 1e-3 / 1e-2),
 *   3. low-noise and noise-free controls,
 *   4. the 6 trajectories broken by the blanket-marker attempt,
 *   5. a stress grid in the uniform-branch convention.
 * It asserts per-address weight agreement at 1e-12 and fidelity agreement at
 * 1e-9, equal fired-jump counts between the modes (same RNG stream), and that
 * the battery as a whole actually exercised the fired-jump path. Any failure
 * exits nonzero so ctest catches it. */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
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

qram_qubit::QRAMCircuit configure_qram(size_t n, const std::map<OperationType, double>& noise)
{
	random_engine::get_instance().set_seed(k_base_seed);
	qram_qubit::QRAMCircuit qram(n, k_data_bits);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	set_full_coverage_zero_bus(qram, n, k_data_bits);
	return qram;
}

/* The perf_scan input convention: uniform random input branches instead of
 * the zero-bus full-coverage task. Different good/bad compositions, so both
 * conventions belong in the battery. */
qram_qubit::QRAMCircuit configure_qram_uniform(size_t n, const std::map<OperationType, double>& noise)
{
	random_engine::get_instance().set_seed(k_base_seed);
	qram_qubit::QRAMCircuit qram(n, k_data_bits);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	qram.set_input_uniform(k_input_branches);
	return qram;
}

seed_t derive_run_seed(size_t trajectory)
{
	random_engine::get_instance().set_seed(k_base_seed + trajectory);
	return random_engine::get_instance().reseed();
}

/* per-address terminal weight: sum of |amplitude|^2 over the group's components */
std::vector<double> address_weights(const qram_qubit::QRAMCircuit& qram, size_t n)
{
	std::vector<double> w(pow2(n), 0.0);
	for (auto& g : qram.branch_groups) {
		for (auto& b : g.branches)
			for (auto it = b.iterbeg(); it != b.iterend(); ++it)
				w[g.address] += std::norm(it->amplitude);
	}
	return w;
}

struct Case {
	double eps;
	size_t n;
	seed_t run_seed;
	bool require_fire;   /* the trajectory must fire at least one K1 jump */
	bool scan_input;     /* perf_scan uniform-branch convention (else zero-bus full coverage) */
	double fid_full_csv; /* perf_scan.csv fidelities for the historical cases */
	double fid_pruned_csv;
	const char* tag;
};

std::vector<Case> build_battery()
{
	std::vector<Case> cases;

	/* 1) the 14 historically mismatching trajectories of perf_scan.csv
	      (fidelities copied from the pre-fix scan for before/after context) */
	const struct { double eps; size_t n; seed_t seed; double f_full; double f_pruned; } hist[] = {
		{1e-4, 10, 2090429186520196096ULL, 1.22082439254e-10, 0.00469132965686},
		{1e-4, 10, 9208654621621431296ULL, 0.124998782386, 0.0057859659738},
		{1e-3, 6, 7744644691974779904ULL, 0.0624919928332, 0.0234293609696},
		{1e-3, 6, 3874279414526882816ULL, 2.85579434329e-07, 0.00142045454545},
		{1e-3, 8, 4101528895484906496ULL, 0.0312418059225, 0.000901442307692},
		{1e-3, 8, 3886007424377267200ULL, 0.00195306243961, 0.00592105263158},
		{1e-3, 8, 4210972589829411840ULL, 3.11234726345e-08, 0.00123615506329},
		{1e-3, 8, 9208654621621431296ULL, 0.00048828125, 0.00448994252874},
		{1e-3, 10, 8975687211725193216ULL, 0.0001220703125, 0.00557544052863},
		{1e-3, 10, 6989413129363354624ULL, 0.0001220703125, 0.00322074142157},
		{1e-3, 10, 1207751192743994368ULL, 0.00210275362568, 0.00130890052356},
		{1e-3, 12, 8975687211725193216ULL, 4.39850712781e-09, 0.00032462654533},
		{1e-3, 12, 5681480801414385664ULL, 8.59148621484e-06, 0.000922509225092},
		{1e-3, 12, 1207751192743994368ULL, 0.000492180334447, 0.00134235395189},
	};
	for (const auto& h : hist)
		cases.push_back({h.eps, h.n, h.seed, true, false, h.f_full, h.f_pruned, "hist"});

	/* 2) matched controls from the same scan */
	const seed_t controls[] = {
		3217773524253467648ULL,
		5244930501339431936ULL,
		6798469447420720128ULL,
	};
	for (seed_t s : controls)
		cases.push_back({1e-3, 4, s, false, false, 0.0, 0.0, "ctrl"});

	/* 3) fired-jump stress grid (same seed derivation as perf_scan).
	   Per-case fire expectations would be flaky here (single trajectories
	   legitimately fire zero jumps); the battery-level total below enforces
	   that the fired-jump path is exercised. */
	for (size_t traj = 0; traj < 8; ++traj) {
		seed_t s = derive_run_seed(traj);
		for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)})
			cases.push_back({1e-3, n, s, false, false, 0.0, 0.0, "stress"});
		for (size_t n : {size_t(4), size_t(6), size_t(8)})
			cases.push_back({1e-2, n, s, false, false, 0.0, 0.0, "stress"});
	}

	/* 4) low-noise and noise-free controls */
	for (size_t traj = 0; traj < 4; ++traj) {
		seed_t s = derive_run_seed(100 + traj);
		for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)}) {
			cases.push_back({0.0, n, s, false, false, 0.0, 0.0, "quiet"});
			cases.push_back({1e-5, n, s, false, false, 0.0, 0.0, "quiet"});
		}
	}

	/* 5) the six trajectories that the first, blanket version of the fix
	      (kill every good group on fire) broke: their good branches are
	      EXCITED at the fired node -- routing bit 1 -- and decay in lockstep
	      with the reference, so they must stay predictable. Pinned in the
	      perf_scan uniform-branch convention as a regression guard. */
	const struct { size_t n; seed_t seed; } scan_hist[] = {
		{10, 7744644691974779904ULL},
		{12, 7744644691974779904ULL},
		{12, 1183008555922881536ULL},
		{12, 3227135189013113856ULL},
		{12, 3287329390135220224ULL},
		{12, 4366419402265368576ULL},
	};
	for (const auto& h : scan_hist)
		cases.push_back({1e-3, h.n, h.seed, true, true, 0.0, 0.0, "scanhist"});

	/* 6) stress grid in the perf_scan uniform-branch convention */
	for (size_t traj = 0; traj < 4; ++traj) {
		seed_t s = derive_run_seed(traj);
		for (size_t n : {size_t(4), size_t(6), size_t(8), size_t(10), size_t(12)})
			cases.push_back({1e-3, n, s, false, true, 0.0, 0.0, "scanstress"});
		for (size_t n : {size_t(4), size_t(6)})
			cases.push_back({1e-2, n, s, false, true, 0.0, 0.0, "scanstress"});
	}

	return cases;
}

/* runs one case in both modes; returns true iff all checks pass */
bool run_case(const Case& c, size_t& pruned_fires)
{
	auto noise = make_noise(c.eps);

	auto qf = c.scan_input ? configure_qram_uniform(c.n, noise)
		: configure_qram(c.n, noise);
	random_engine::get_instance().set_seed(c.run_seed);
	qf.run(qram_qubit::QRAMCircuit::FULL_VER);
	double f_full = qf.sample_and_get_fidelity();
	/* weights AFTER sample_and_get_fidelity: sampling collapses both modes
	   onto the same tree configuration (identical RNG streams) and
	   normalizes, which is the comparable state — before sampling, the
	   pruned mode's good groups still hold their un-materialized inputs */
	auto wf = address_weights(qf, c.n);

	auto qp = c.scan_input ? configure_qram_uniform(c.n, noise)
		: configure_qram(c.n, noise);
	random_engine::get_instance().set_seed(c.run_seed);
	qp.run(qram_qubit::QRAMCircuit::NORMAL_VER);
	double f_pruned = qp.sample_and_get_fidelity();
	auto wp = address_weights(qp, c.n);
	pruned_fires = qp.fired_jump_count;

	double dw = 0;
	for (size_t a = 0; a < pow2(c.n); ++a)
		dw = std::max(dw, std::abs(wf[a] - wp[a]));
	double df = std::abs(f_full - f_pruned);
	bool fires_agree = qf.fired_jump_count == qp.fired_jump_count;

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
		<< " seed=" << c.run_seed
		<< " fires=" << qp.fired_jump_count << "/" << qf.fired_jump_count
		<< " dW=" << dw << " dF=" << df;
	if (c.fid_full_csv > 0.0 || c.fid_pruned_csv > 0.0)
		std::cout << " (pre-fix dF=" << std::abs(c.fid_full_csv - c.fid_pruned_csv) << ")";
	std::cout << "\n";

	const std::string tag(c.tag);
	check(dw <= k_weight_tol, "per-address weights", tag);
	check(df <= k_fid_tol, "fidelity", tag);
	if (c.eps > 0.0)
		check(fires_agree, "fired-jump counts agree", tag);
	if (c.require_fire && pruned_fires == 0)
		check(false, "expected at least one fired jump", tag);

	return dw <= k_weight_tol && df <= k_fid_tol && fires_agree
		&& (!c.require_fire || pruned_fires > 0);
}

} // namespace

int main()
{
	std::cout << "pruned-vs-full exactness regression battery\n";
	auto cases = build_battery();

	size_t passed = 0, total_fires = 0;
	for (const Case& c : cases) {
		size_t fires = 0;
		if (run_case(c, fires)) ++passed;
		total_fires += fires;
	}

	std::cout << "\n" << passed << "/" << cases.size() << " case(s) exact\n";
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
