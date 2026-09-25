/* qubit-encoded QRAM paper experiment driver: regenerates the raw data for fig_fidelity / fig_perf / tab:equiv.
 *
 * Protocol identical to QRAMFidelityV2/QRAMSimulatorTest.cpp's test_qubit_compare:
 *   - memory and the 500-branch uniform-superposition input are fixed by the base seed (same for every trajectory);
 *   - each trajectory derives runseed via set_seed(base+it) then reseed(), driving that trajectory's noise sampling;
 *   - full vs pruned (normal) are cross-checked under the same runseed.
 * Noise convention: eps = gamma, i.e. Depolarizing and Damping are both set to the same value.
 * Exception: perf (fig 4) uses full-address-coverage, bus=0 data-loading task input (deterministic branch count d=2^n).
 *
 * Usage:
 *   QubitPaperScan fidelity [outdir]   # fig 3: qubit 3 noise levels × n=3..10 × 200 trajectories
 *                                      #        + qutrit 1e-5 (fig 3b comparison line)
 *   QubitPaperScan perf [outdir]       # fig 4: qubit n=4..12 × {0,1e-5,1e-4,1e-3} × 10 trajectories
 *   QubitPaperScan equiv [outdir]      # tab:equiv: 4 working points × 3 explicit seeds
 */

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "logger.h"
#include "state_manipulator.h"
#include "time_step.h"
#include "qram_circuit_qubit.h"

using namespace qram_simulator;

namespace {

constexpr size_t k_data_bits = 3;
constexpr size_t k_branches = 500;
constexpr seed_t k_base_seed = 123456789;

/* Explicit seeds for tab:equiv (carried over from the paper's original table) */
constexpr seed_t k_equiv_seeds[] = {880000, 880007, 880014};

/* Full address coverage + zero-bus input: data-loading task convention (one branch per address, bus=0, uniform weights).
 * Under this convention the full-mode branch count is exactly 2^n, and at ε=0 pruned mode evolves exactly 1 reference branch,
 * so fig_perf panel b carries no input-sampling fluctuations. */
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

void set_full_coverage_zero_bus(qram_qutrit::QRAMCircuit& qram, size_t n, size_t k)
{
	qram.get_branches().clear();
	qram.get_branch_probs().clear();
	for (size_t addr = 0; addr < pow2(n); ++addr) {
		qram.get_branches().emplace_back(addr << k, k);
		qram.get_branch_probs().push_back(1.0 / pow2(n));
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

template <typename QRAM_type>
QRAM_type configure_qram(size_t addr_sz, size_t data_sz, const std::map<OperationType, double>& noise,
	seed_t base_seed, size_t input_sz, bool zero_bus_full_coverage = false)
{
	random_engine::get_instance().set_seed(base_seed);

	QRAM_type qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	if (zero_bus_full_coverage)
		set_full_coverage_zero_bus(qram, addr_sz, data_sz);
	else
		qram.set_input_uniform(input_sz);

	return qram;
}

/* Total branch sub-states explicitly stored after the run (before sampling).
 * Counts only valid_branch_group_view (branches actually evolved: bad + reference good);
 * in pruned mode the good branches' input states are not evolved and must not be counted. */
size_t count_explicit_states(const qram_qubit::QRAMCircuit& qram)
{
	size_t count = 0;
	for (const auto* group : qram.valid_branch_group_view)
		for (const auto& branch : group->branches)
			count += branch.system_states_sz;
	return count;
}

/* Number of branches explicitly evolved (bad + reference good); equals all input branches in full mode. */
size_t count_explicit_branches(const qram_qubit::QRAMCircuit& qram)
{
	size_t count = 0;
	for (const auto* group : qram.valid_branch_group_view)
		count += group->branches.size();
	return count;
}

double now_ms_since(std::chrono::steady_clock::time_point t0)
{
	return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

struct QubitTrajectoryResult
{
	double fid = 0;
	double time_run_ms = 0;
	double time_sample_ms = 0;
	size_t states = 0;
	size_t branches = 0;
};

QubitTrajectoryResult run_qubit_once(size_t addr_sz, const std::map<OperationType, double>& noise,
	seed_t base_seed, size_t input_sz, seed_t run_seed, std::string version,
	bool zero_bus_full_coverage = false)
{
	QubitTrajectoryResult result;
	auto qram = configure_qram<qram_qubit::QRAMCircuit>(addr_sz, k_data_bits, noise, base_seed,
		input_sz, zero_bus_full_coverage);

	random_engine::get_instance().set_seed(run_seed);
	auto t0 = std::chrono::steady_clock::now();
	qram.run(version);
	result.time_run_ms = now_ms_since(t0);

	result.states = count_explicit_states(qram);
	result.branches = count_explicit_branches(qram);

	auto t1 = std::chrono::steady_clock::now();
	result.fid = qram.sample_and_get_fidelity();
	result.time_sample_ms = now_ms_since(t1);

	return result;
}

double run_qutrit_full_once(size_t addr_sz, const std::map<OperationType, double>& noise,
	seed_t base_seed, size_t input_sz, seed_t run_seed)
{
	auto qram = configure_qram<qram_qutrit::QRAMCircuit>(addr_sz, k_data_bits, noise, base_seed, input_sz);

	random_engine::get_instance().set_seed(run_seed);
	qram.run_full();
	return qram.sample_and_get_fidelity();
}

double run_qutrit_full_once_k(size_t addr_sz, size_t k, const std::map<OperationType, double>& noise,
	seed_t base_seed, size_t input_sz, seed_t run_seed)
{
	auto qram = configure_qram<qram_qutrit::QRAMCircuit>(addr_sz, k, noise, base_seed, input_sz);

	random_engine::get_instance().set_seed(run_seed);
	qram.run_full();
	return qram.sample_and_get_fidelity();
}

std::ofstream open_csv(const std::filesystem::path& outdir, const std::string& name)
{
	std::filesystem::create_directories(outdir);
	auto path = outdir / name;
	std::ofstream csv(path);
	if (!csv) {
		std::cerr << "cannot open " << path.string() << "\n";
		std::exit(1);
	}
	return csv;
}

seed_t derive_run_seed(size_t trajectory)
{
	random_engine::get_instance().set_seed(k_base_seed + trajectory);
	return random_engine::get_instance().reseed();
}

void run_fidelity(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "fidelity_scan.csv");
	csv << "arch,eps,n,traj,runseed,version,fid,time_run_ms,time_sample_ms,states,branches\n";

	constexpr size_t trials = 200;
	const std::vector<double> eps_list = {1e-5, 1e-4, 1e-3};

	/* qubit: 3 noise levels × n=3..10 (fig 3a; the 1e-5 rows also feed fig 3b) */
	for (double eps : eps_list) {
		auto noise = make_noise(eps);
		for (size_t n = 3; n <= 10; ++n) {
			for (size_t traj = 0; traj < trials; ++traj) {
				seed_t run_seed = derive_run_seed(traj);
				for (std::string version : {qram_qubit::QRAMCircuit::FULL_VER, qram_qubit::QRAMCircuit::NORMAL_VER}) {
					auto r = run_qubit_once(n, noise, k_base_seed, k_branches, run_seed, version);
					csv << std::setprecision(12) << "qubit," << eps << ',' << n << ',' << traj << ','
						<< run_seed << ',' << version << ',' << r.fid << ',' << r.time_run_ms << ','
						<< r.time_sample_ms << ',' << r.states << ',' << r.branches << '\n';
				}
				std::cout << "qubit eps=" << eps << " n=" << n << " traj " << traj + 1 << "/" << trials << "\r";
				std::cout.flush();
			}
			std::cout << "\n";
		}
	}

	/* qutrit: 1e-5, full mode (fig 3b comparison line) */
	{
		auto noise = make_noise(1e-5);
		for (size_t n = 3; n <= 10; ++n) {
			for (size_t traj = 0; traj < trials; ++traj) {
				seed_t run_seed = derive_run_seed(traj);
				double fid = run_qutrit_full_once(n, noise, k_base_seed, k_branches, run_seed);
				csv << std::setprecision(12) << "qutrit," << 1e-5 << ',' << n << ',' << traj << ','
					<< run_seed << ",full," << fid << ",0,0,0\n";
				std::cout << "qutrit eps=1e-5 n=" << n << " traj " << traj + 1 << "/" << trials << "\r";
				std::cout.flush();
			}
			std::cout << "\n";
		}
	}
}

void run_perf(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "perf_scan.csv");
	csv << "arch,eps,n,traj,runseed,version,fid,time_run_ms,time_sample_ms,states,branches\n";

	constexpr size_t trials = 50;
	const std::vector<double> eps_list = {0.0, 1e-5, 1e-4, 1e-3};
	const std::vector<size_t> n_list = {4, 6, 8, 10, 12};

	/* Input convention: full address coverage, bus=0 (data-loading task). The branch count is exactly d=2^n;
	 * at ε=0 pruned evolves only 1 reference branch, so panel b has no input-sampling fluctuations. */
	for (double eps : eps_list) {
		auto noise = make_noise(eps);
		for (size_t n : n_list) {
			for (size_t traj = 0; traj < trials; ++traj) {
				seed_t run_seed = derive_run_seed(traj);
				for (std::string version : {qram_qubit::QRAMCircuit::FULL_VER, qram_qubit::QRAMCircuit::NORMAL_VER}) {
					auto r = run_qubit_once(n, noise, k_base_seed, pow2(n), run_seed, version, true);
					csv << std::setprecision(12) << "qubit," << eps << ',' << n << ',' << traj << ','
						<< run_seed << ',' << version << ',' << r.fid << ',' << r.time_run_ms << ','
						<< r.time_sample_ms << ',' << r.states << ',' << r.branches << '\n';
				}
				std::cout << "eps=" << eps << " n=" << n << " traj " << traj + 1 << "/" << trials << "\r";
				std::cout.flush();
			}
			std::cout << "\n";
		}
	}
}

void run_equiv(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "equiv.csv");
	csv << "n,eps,seed,fid_full,fid_pruned,abs_delta\n";

	const std::vector<std::pair<size_t, double>> points = {
		{8, 1e-5}, {10, 1e-5}, {6, 1e-4}, {6, 1e-3}};

	for (auto [n, eps] : points) {
		auto noise = make_noise(eps);
		for (seed_t seed : k_equiv_seeds) {
			auto full = run_qubit_once(n, noise, k_base_seed, k_branches, seed,
				qram_qubit::QRAMCircuit::FULL_VER);
			auto pruned = run_qubit_once(n, noise, k_base_seed, k_branches, seed,
				qram_qubit::QRAMCircuit::NORMAL_VER);
			double delta = std::abs(full.fid - pruned.fid);
			csv << std::setprecision(12) << n << ',' << eps << ',' << seed << ',' << full.fid << ','
				<< pruned.fid << ',' << delta << '\n';
			std::cout << "n=" << n << " eps=" << eps << " seed=" << seed << std::setprecision(15)
				<< " fid_full=" << full.fid << " fid_pruned=" << pruned.fid << "\n";
		}
	}
}

/* Verify that at ε=0 pruned mode keeps only 1 branch group (the reference branch),
 * and tally the sub-state composition of the reference group */
void run_check0(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "check0.csv");
	csv << "n,groups_total,groups_valid,bad_groups,states_reference\n";

	for (size_t n : {4, 6, 8, 10, 12}) {
		auto qram = configure_qram<qram_qubit::QRAMCircuit>(
			n, k_data_bits, {}, k_base_seed, k_branches);
		random_engine::get_instance().set_seed(k_base_seed);
		qram.run_normal();

		size_t bad_groups = 0;
		for (auto* g : qram.valid_branch_group_view)
			if (g - qram.branch_groups.data() != qram.first_good_branch_group)
				++bad_groups;
		size_t states = count_explicit_states(qram);

		csv << n << ',' << qram.branch_groups.size() << ',' << qram.valid_branch_group_view.size()
			<< ',' << bad_groups << ',' << states << '\n';
		std::cout << "n=" << n << ": groups_total=" << qram.branch_groups.size()
			<< " valid=" << qram.valid_branch_group_view.size()
			<< " bad=" << bad_groups << " states=" << states << "\n";
	}
}

/* Mechanism breakdown: qubit vs qutrit fidelity under single channels (depol-only / damp-only).
 * ε=1e-4 (at 1e-5 damping has almost no events, so channel differences are invisible), n∈{8,10}, k∈{1,5} */
void run_mech(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "mech_scan.csv");
	csv << "arch,channel,eps,n,k,traj,fid\n";

	constexpr double eps = 1e-4;
	constexpr size_t trials = 200;

	for (size_t n : {8, 10}) {
		for (size_t k : {1, 5}) {
			size_t input_sz = std::min<size_t>(k_branches, pow2(n + k));
			for (int channel = 0; channel < 2; ++channel) {
				std::map<OperationType, double> noise;
				noise[channel == 0 ? OperationType::Depolarizing : OperationType::Damping] = eps;

				for (size_t traj = 0; traj < trials; ++traj) {
					seed_t run_seed = derive_run_seed(traj);
					double fq = run_qutrit_full_once_k(n, k, noise, k_base_seed, input_sz, run_seed);
					csv << std::setprecision(12) << "qutrit," << (channel ? "damp" : "depol") << ','
						<< eps << ',' << n << ',' << k << ',' << traj << ',' << fq << '\n';
				}
				for (size_t traj = 0; traj < trials; ++traj) {
					seed_t run_seed = derive_run_seed(traj);
					random_engine::get_instance().set_seed(k_base_seed);
					qram_qubit::QRAMCircuit qram(n, k);
					qram.set_memory_random();
					qram.set_noise_models(noise);
					qram.set_input_uniform(input_sz);
					random_engine::get_instance().set_seed(run_seed);
					qram.run_full();
					csv << std::setprecision(12) << "qubit," << (channel ? "damp" : "depol") << ','
						<< eps << ',' << n << ',' << k << ',' << traj << ','
						<< qram.sample_and_get_fidelity() << '\n';
				}
				std::cout << "n=" << n << " k=" << k << " channel="
					<< (channel ? "damp" : "depol") << " done\n";
			}
		}
	}
}

/* Deep-tree single-fault injection audit: inject a single X(BitFlip) fault at a given
 * node, measure the address set actually damaged in full mode, and compare with the analytic bad range (paper envelope) and the Hann left-port-chain propagation expectation */
void run_audit(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "audit.csv");
	csv << "arch,n,depth,side,node,pos,slot,slice,groups_total,damaged,min_addr,max_addr,"
		"analytic_lo,analytic_hi,analytic_size\n";

	constexpr size_t n = 10;
	constexpr size_t input_sz = pow2(n + k_data_bits); /* full address coverage */

	for (size_t depth : {4, 6, 8}) {
		for (size_t side : {0, 1}) { /* 0=leftmost chain, 1=rightmost chain */
			size_t node = side == 0 ? pow2(depth) - 1 : pow2(depth + 1) - 2;
			size_t pos = 2 * node; /* addr slot */

			for (double frac : {0.3, 0.6, 0.9}) {
				random_engine::get_instance().set_seed(k_base_seed);

				auto qram = std::make_unique<qram_qubit::QRAMCircuit>(n, k_data_bits);
				qram->set_memory_random();
				qram->set_noise_models({});
				qram->set_input_uniform(input_sz);
				qram->prepare_all();
				size_t n_slices = qram->operations.time_slices.size();
				size_t slice = static_cast<size_t>(frac * (n_slices - 1));
				qram->operations.time_slices[slice].append(
					Operation(OperationType::BitFlip, {pos}));
				qram->run_bad();

				size_t damaged = 0;
				size_t lo = SIZE_MAX, hi = 0;
				double expect = 1.0 / pow2(n);
				for (auto& g : qram->branch_groups) {
					double f = std::abs(g.get_fidelity(qram->get_memory()));
					if (f * f < 0.25 * expect * expect) {
						++damaged;
						lo = std::min(lo, g.address);
						hi = std::max(hi, g.address);
					}
				}
				auto [alo, ahi] = qram->time_step.get_bad_range_qubit(pos);
				csv << "qubit," << n << ',' << depth << ',' << side << ',' << node << ','
					<< pos << ",addr," << slice << ',' << qram->branch_groups.size() << ','
					<< damaged << ',' << lo << ',' << hi << ',' << alo << ',' << ahi
					<< ',' << (ahi - alo + 1) << '\n';
				std::cout << "qubit d=" << depth << " side=" << side << " slice_frac="
					<< frac << ": damaged " << damaged << " [" << lo << "," << hi
					<< "] analytic [" << alo << "," << ahi << "]\n";
			}
		}
	}
}

/* k scan: test whether the qubit/qutrit fidelity gap grows with data width k.
 * Scan A: n=8 fixed, k ∈ {1,2,3,5}, eps ∈ {1e-5, 1e-4}
 * Scan B: k ∈ {1,5}, n ∈ {4,6,10}, eps = 1e-4 (n-scaling slope) */
void run_kscan(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "kscan.csv");
	csv << "arch,eps,n,k,traj,fid,time_ms,states\n";

	constexpr size_t trials = 150;
	struct Cell { double eps; size_t n; size_t k; };
	std::vector<Cell> cells;
	for (double eps : {1e-5, 1e-4})
		for (size_t k : {1, 2, 3, 5})
			cells.push_back({eps, 8, k});
	for (size_t k : {1, 5})
		for (size_t n : {4, 6, 10})
			cells.push_back({1e-4, n, k});

	for (auto [eps, n, k] : cells) {
		auto noise = make_noise(eps);
		size_t input_sz = std::min<size_t>(k_branches, pow2(n + k));
		for (int arch = 0; arch < 2; ++arch) {
			for (size_t traj = 0; traj < trials; ++traj) {
				seed_t run_seed = derive_run_seed(traj);
				double fid;
				double time_ms = 0;
				size_t states = 0;
				if (arch == 0) {
					random_engine::get_instance().set_seed(k_base_seed);
					qram_qubit::QRAMCircuit qram(n, k);
					qram.set_memory_random();
					qram.set_noise_models(noise);
					qram.set_input_uniform(input_sz);
					random_engine::get_instance().set_seed(run_seed);
					auto t0 = std::chrono::steady_clock::now();
					qram.run_full();
					time_ms = now_ms_since(t0);
					states = count_explicit_states(qram);
					fid = qram.sample_and_get_fidelity();
				}
				else {
					fid = run_qutrit_full_once_k(n, k, noise, k_base_seed, input_sz, run_seed);
				}
				csv << std::setprecision(12) << (arch == 0 ? "qubit" : "qutrit") << ',' << eps
					<< ',' << n << ',' << k << ',' << traj << ',' << fid << ',' << time_ms
					<< ',' << states << '\n';
			}
			std::cout << (arch == 0 ? "qubit" : "qutrit") << " eps=" << eps << " n=" << n
				<< " k=" << k << " done\n";
		}
	}
}

/* Densified n scan: ε=3e-5 intermediate strength (avoiding 1e-4 saturation and 1e-5 tail noise),
 * n=4..10 point by point, k∈{1,5}, 300 trajectories — for asymptotic slope fitting */
void run_nscan(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "nscan.csv");
	csv << "arch,eps,n,k,traj,fid,time_ms,states\n";

	constexpr double eps = 3e-5;
	constexpr size_t trials = 300;
	auto noise = make_noise(eps);

	for (size_t k : {1, 5}) {
		for (size_t n = 4; n <= 10; ++n) {
			size_t input_sz = std::min<size_t>(k_branches, pow2(n + k));
			for (int arch = 0; arch < 2; ++arch) {
				for (size_t traj = 0; traj < trials; ++traj) {
					seed_t run_seed = derive_run_seed(traj);
					double fid;
					if (arch == 0) {
						random_engine::get_instance().set_seed(k_base_seed);
						qram_qubit::QRAMCircuit qram(n, k);
						qram.set_memory_random();
						qram.set_noise_models(noise);
						qram.set_input_uniform(input_sz);
						random_engine::get_instance().set_seed(run_seed);
						qram.run_full();
						fid = qram.sample_and_get_fidelity();
					}
					else {
						fid = run_qutrit_full_once_k(n, k, noise, k_base_seed, input_sz, run_seed);
					}
					csv << std::setprecision(12) << (arch == 0 ? "qubit" : "qutrit") << ','
						<< eps << ',' << n << ',' << k << ',' << traj << ',' << fid << ",0,0\n";
				}
				std::cout << (arch == 0 ? "qubit" : "qutrit") << " n=" << n << " k=" << k
					<< " done\n";
			}
		}
	}
}

/* Equal-qubit-count architecture comparison: A=(n,d) parallel vs B=(n+log2 d,1) address-widened,
 * data-loading task (full coverage, zero bus); at d=1 the two are identical, serving as a self-consistency check */
void run_archscan(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "archscan.csv");
	csv << "arch,scheme,eps,n,d,nprime,k,traj,fid\n";

	constexpr size_t trials = 150;
	struct Task { double eps; size_t n; size_t d; };
	std::vector<Task> tasks;
	for (size_t d : {1, 2, 4, 8})
		for (size_t n : {6, 8, 10})
			tasks.push_back({1e-4, n, d});
	for (size_t d : {1, 8})
		for (size_t n : {6, 8, 10})
			tasks.push_back({3e-5, n, d});

	for (auto [eps, n, d] : tasks) {
		auto noise = make_noise(eps);
		size_t logd = (d > 1) ? static_cast<size_t>(std::log2(d)) : 0;
		for (int scheme = 0; scheme < 2; ++scheme) { /* 0=A:(n,d), 1=B:(n+logd,1) */
			size_t np = scheme == 0 ? n : n + logd;
			size_t k = scheme == 0 ? d : 1;
			for (int arch = 0; arch < 2; ++arch) {
				for (size_t traj = 0; traj < trials; ++traj) {
					seed_t run_seed = derive_run_seed(traj);
					double fid;
					if (arch == 0) {
						random_engine::get_instance().set_seed(k_base_seed);
						qram_qubit::QRAMCircuit qram(np, k);
						qram.set_memory_random();
						qram.set_noise_models(noise);
						set_full_coverage_zero_bus(qram, np, k);
						random_engine::get_instance().set_seed(run_seed);
						qram.run_full();
						fid = qram.sample_and_get_fidelity();
					}
					else {
						random_engine::get_instance().set_seed(k_base_seed);
						qram_qutrit::QRAMCircuit qram(np, k);
						qram.set_memory_random();
						qram.set_noise_models(noise);
						set_full_coverage_zero_bus(qram, np, k);
						random_engine::get_instance().set_seed(run_seed);
						qram.run_full();
						fid = qram.sample_and_get_fidelity();
					}
					csv << std::setprecision(12) << (arch == 0 ? "qubit" : "qutrit") << ','
						<< (scheme == 0 ? "A" : "B") << ',' << eps << ',' << n << ',' << d << ','
						<< np << ',' << k << ',' << traj << ',' << fid << '\n';
				}
				std::cout << (arch == 0 ? "qubit" : "qutrit") << (scheme == 0 ? "A" : "B")
					<< " eps=" << eps << " n=" << n << " d=" << d << "\n";
			}
		}
	}
}

/* Channel breakdown of the architecture comparison: d=4, n=8, eps=1e-4, A and B each run depol-only / damp-only */
void run_archmech(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "archmech.csv");
	csv << "arch,scheme,channel,n,d,nprime,k,traj,fid\n";

	constexpr double eps = 1e-4;
	constexpr size_t trials = 200;
	constexpr size_t n = 8, d = 4, logd = 2;

	for (int scheme = 0; scheme < 2; ++scheme) {
		size_t np = scheme == 0 ? n : n + logd;
		size_t k = scheme == 0 ? d : 1;
		for (int channel = 0; channel < 2; ++channel) {
			std::map<OperationType, double> noise;
			noise[channel == 0 ? OperationType::Depolarizing : OperationType::Damping] = eps;
			for (int arch = 0; arch < 2; ++arch) {
				for (size_t traj = 0; traj < trials; ++traj) {
					seed_t run_seed = derive_run_seed(traj);
					double fid;
					if (arch == 0) {
						random_engine::get_instance().set_seed(k_base_seed);
						qram_qubit::QRAMCircuit qram(np, k);
						qram.set_memory_random();
						qram.set_noise_models(noise);
						set_full_coverage_zero_bus(qram, np, k);
						random_engine::get_instance().set_seed(run_seed);
						qram.run_full();
						fid = qram.sample_and_get_fidelity();
					}
					else {
						random_engine::get_instance().set_seed(k_base_seed);
						qram_qutrit::QRAMCircuit qram(np, k);
						qram.set_memory_random();
						qram.set_noise_models(noise);
						set_full_coverage_zero_bus(qram, np, k);
						random_engine::get_instance().set_seed(run_seed);
						qram.run_full();
						fid = qram.sample_and_get_fidelity();
					}
					csv << std::setprecision(12) << (arch == 0 ? "qubit" : "qutrit") << ','
						<< (scheme == 0 ? "A" : "B") << ',' << (channel ? "damp" : "depol")
						<< ',' << n << ',' << d << ',' << np << ',' << k << ',' << traj
						<< ',' << fid << '\n';
				}
			}
			std::cout << (scheme == 0 ? "A" : "B") << " " << (channel ? "damp" : "depol")
				<< " done\n";
		}
	}
}

} // namespace

int main(int argc, const char** argv)
{
	if (argc < 2) {
		std::cerr << "usage: QubitPaperScan <fidelity|perf|equiv> [outdir]\n";
		return 1;
	}
	std::filesystem::path outdir = (argc >= 3) ? argv[2] : "results";
	std::string mode = argv[1];

	if (mode == "fidelity")
		run_fidelity(outdir);
	else if (mode == "perf")
		run_perf(outdir);
	else if (mode == "equiv")
		run_equiv(outdir);
	else if (mode == "check0")
		run_check0(outdir);
	else if (mode == "mech")
		run_mech(outdir);
	else if (mode == "audit")
		run_audit(outdir);
	else if (mode == "kscan")
		run_kscan(outdir);
	else if (mode == "nscan")
		run_nscan(outdir);
	else if (mode == "archscan")
		run_archscan(outdir);
	else if (mode == "archmech")
		run_archmech(outdir);
	else {
		std::cerr << "unknown mode: " << mode << "\n";
		return 1;
	}
	return 0;
}
