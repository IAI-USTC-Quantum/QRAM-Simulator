/* QubitPaperVerify.cpp — targeted numerical verification of the core propositions of the qubit-encoded QRAM paper.
 *
 * Same protocol as QubitPaperScan.cpp: k_data_bits=3, k_base_seed fixes memory+input,
 * per-trajectory derive_run_seed(traj) fixes the noise history; all branch-level readouts happen before sample_output().
 *
 * Usage: Experiment_QRAM_QubitPaperVerify <mode> [outdir]
 *
 *   hamming  Propositions 1+2 (H→K0^d→H closed form; second-order no-jump conditional error):
 *            single-address (addr=0) single-bus (j=0) input branch, damping-only, rejection-sample no-jump
 *            trajectories (no Damp_Full ops), dumping (word, amplitude) of all system_states.
 *            When rejection sampling is infeasible at large gamma, fall back to "directly constructing a pure K0 circuit"
 *            (bitwise equivalent to a no-jump trajectory; equivalence measured and printed where both are sampleable).
 *   avgfid   Proposition 2 (first/second-order separation): damping-only, 500 uniformly drawn branches, n∈{3,5,8};
 *            per trajectory, after run_full and before sampling, compute final-state fidelity F=|<Ψ_ideal|ψ>|²/<ψ|ψ>
 *            (Ψ_ideal keeps only tree-back-to-idle components), and record whether a Damp_Full was drawn.
 *            Each point first runs 100 trajectories (brief protocol); in the rare-event window (1e-4≤γ≤3e-3, and
 *            γ=1e-5 for n≤5) adaptively extend to ≥120 fired (inf>0.5) trajectories,
 *            so the first-order slope of the ensemble-mean infidelity has enough statistics (rows beyond 100 still go
 *            into the same CSV, trajectory indices continue).
 *   qutrit   Proposition 3 (qutrit scalar prediction): n=k=3, all 64 branches covered; per branch on no-jump trajectories
 *            cross-check the explicit weight ratio |amp_i|²/|amp_ref|² vs the analytic counter (1-γ)^Δc,
 *            plus the fidelity to the ideal output after normalization (should be 1).
 *   auditn3  Proposition 4 (family-divergence rule): n=3 exhaustive single-fault injection
 *            (7 nodes × {addr,data} slots × all slices × {BitFlip, Damp_Full}),
 *            measured damaged-address set vs predicted bad range (BitFlip→get_bad_range_qubit,
 *            Damp_Full→get_bad_range_qutrit). Damage = wrong output word ∪ config family deviating from the reference
 *            (reference = terminal tree-config set of the first address outside the predicted range; a uniform whole-tree
 *            stray counts as one predictable family, not damage). match=1 means measured⊆predicted (containment),
 *            match=-1 means the jump firing probability for that slot is identically 0 (no branch has that bit
 *            excited at that time, so the jump is physically impossible there — a vacuous case).
 *            BitFlip uses the same append+run_bad as audit; Damp_Full uses the per-slice driver, which at the injection
 *            point first sums the exact firing probability Σ get_prob_damp (the true firing probability absent other loss sources),
 *            then applies the deterministic projection Branch::run_damp_full to all branches — identical,
 *            operation by operation, to QRAMCircuit::run_damp_full roulette-"firing" the branches, minus the retries.
 *            The driver itself was cross-checked bitwise against run_bad before the audit.
 *            memory[i]=i (pairwise distinct words rule out missed detections from "wrong cell but coincidentally equal content").
 */

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"
#include "time_step.h"
#include "qram_circuit_qubit.h"
#include "qram_circuit_qutrit.h"

using namespace qram_simulator;

namespace {

constexpr size_t k_data_bits = 3;
constexpr size_t k_branches = 500;
constexpr seed_t k_base_seed = 123456789;

const std::vector<double> k_gamma_grid =
	{1e-5, 3e-5, 1e-4, 3e-4, 1e-3, 3e-3, 1e-2, 3e-2};

std::map<OperationType, double> damp_only(double gamma)
{
	return {{OperationType::Damping, gamma}};
}

seed_t derive_run_seed(size_t trajectory)
{
	random_engine::get_instance().set_seed(k_base_seed + trajectory);
	return random_engine::get_instance().reseed();
}

template <typename Circuit>
bool has_damp_full_op(const Circuit& qram)
{
	for (const auto& pack : qram.get_operations().time_slices)
		for (const auto& op : pack.operations)
			if (op.type == OperationType::Damp_Full)
				return true;
	return false;
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
	csv << std::setprecision(17);
	return csv;
}

std::string join_set(const std::set<size_t>& s)
{
	if (s.empty()) return "nil";
	std::string ret;
	for (size_t a : s) {
		if (!ret.empty()) ret += "-";
		ret += std::to_string(a);
	}
	return ret;
}

/* Single-branch input: one branch group (address), one branch (bus input j) */
void set_single_branch(qram_qubit::QRAMCircuit& qram, size_t address, bus_t bus)
{
	auto& groups = qram.get_branch_groups();
	groups.clear();
	groups.emplace_back(address);
	auto& g = groups.back();
	g.branches_input.emplace_back(address, qram.data_size, bus);
	g.branch_probs.push_back(1.0);
	g.state_probs.push_back(1.0);
}

/* Directly construct a pure K0 (no-jump) operation sequence: fault-free slices + one Damp_Common(gamma) per slice.
 * Identical, operation by operation, to "a trajectory sampled with zero faults" (noise_one_step appends only
 * Damp_Common on zero faults too); used at grid points where rejection sampling is infeasible at large γ. */
void overwrite_with_k0_only(qram_qubit::QRAMCircuit& qram, double gamma)
{
	TimeSlices k0ops;
	for (const auto& pack : qram.time_step.time_slices_noise_free.time_slices) {
		OperationPack p = pack;
		p.append(Operation(OperationType::Damp_Common, {}, {gamma}));
		k0ops.append(p);
	}
	qram.operations = k0ops;
}

using WordAmps = std::map<bus_t, complex_t>;

WordAmps collect_word_amps(qram_qubit::QRAMCircuit& qram, size_t& non_idle)
{
	WordAmps ret;
	non_idle = 0;
	auto& br = qram.branch_groups[0].branches[0];
	for (auto it = br.iterbeg(); it != br.iterend(); ++it) {
		if (!it->state.nz_elements.empty()) ++non_idle;
		ret[it->data_bus] += it->amplitude;
	}
	return ret;
}

/* ------------------------------ mode hamming ------------------------------ */

void run_hamming(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "verify_hamming.csv");
	csv << "n,gamma,k,traj,bus_in,b_out,word,amp_re,amp_im\n";

	constexpr size_t k = k_data_bits;
	constexpr size_t target_traj = 10;
	constexpr size_t attempt_cap = 30000;
	const std::vector<size_t> n_list = {2, 3, 4, 5, 6, 8};

	/* Smoke self-check: at gamma=0, prob(b_out)=1 and all other words are 0 */
	{
		const size_t n = 3;
		random_engine::get_instance().set_seed(k_base_seed);
		qram_qubit::QRAMCircuit qram(n, k);
		qram.set_memory_random();
		qram.set_noise_models(damp_only(0.0));
		set_single_branch(qram, 0, 0);
		qram.run_full();
		bus_t bout = qram.get_memory()[0];
		double norm = qram.get_normalization_factor();
		double p_corr = 0, p_other_max = 0;
		size_t non_idle = 0;
		for (auto& [word, amp] : collect_word_amps(qram, non_idle)) {
			double p = abs_sqr(amp) / norm;
			if (word == bout) p_corr = p;
			else p_other_max = std::max(p_other_max, p);
		}
		if (!(std::abs(p_corr - 1.0) < 1e-12 && p_other_max < 1e-12 && non_idle == 0)) {
			std::cerr << "[hamming] SMOKE FAILED: p(b_out)=" << p_corr
				<< " p_other_max=" << p_other_max << " non_idle=" << non_idle << "\n";
			std::exit(1);
		}
		std::cout << "[hamming] smoke gamma=0 passed: p(b_out)=1\n";
	}

	for (size_t n : n_list) {
		for (double gamma : k_gamma_grid) {
			qram_qubit::QRAMCircuit qram(n, k);
			qram.set_noise_models(damp_only(gamma));
			set_single_branch(qram, 0, 0);

			size_t accepted = 0, attempts = 0;
			bool equiv_checked = false;
			size_t non_idle_total = 0;

			while (accepted < target_traj && attempts < attempt_cap) {
				seed_t seed = k_base_seed + 1000003 * attempts;
				++attempts;
				random_engine::get_instance().set_seed(seed);
				qram.set_memory_random();   /* re-randomize memory per trajectory: b_out changes with it */
				qram.prepare_all();
				if (has_damp_full_op(qram))
					continue;               /* reject: trajectory contains a jump */

				qram.run_bad();

				/* First accepted trajectory: cross-check against the direct K0 construction (same memory) */
				if (!equiv_checked) {
					size_t ni1 = 0;
					auto amps_ref = collect_word_amps(qram, ni1);

					random_engine::get_instance().set_seed(seed);
					qram.set_memory_random();
					qram.prepare_all();
					overwrite_with_k0_only(qram, gamma);
					qram.run_bad();
					size_t ni2 = 0;
					auto amps_k0 = collect_word_amps(qram, ni2);

					double maxdiff = 0;
					for (auto& [word, amp] : amps_ref)
						maxdiff = std::max(maxdiff,
							std::abs(amp - amps_k0[word]));
					std::cout << "[hamming] n=" << n << " gamma=" << gamma
						<< ": rejection-vs-directK0 max|damp|=" << maxdiff
						<< " (non_idle " << ni1 << " vs " << ni2 << ")\n";
					equiv_checked = true;

					/* Restore the state clobbered by the cross-check: rerun the accepted trajectory */
					random_engine::get_instance().set_seed(seed);
					qram.set_memory_random();
					qram.prepare_all();
					qram.run_bad();
				}

				bus_t bout = qram.get_memory()[0]; /* bus_in = 0 */
				size_t non_idle = 0;
				auto amps = collect_word_amps(qram, non_idle);
				non_idle_total += non_idle;
				for (auto& [word, amp] : amps) {
					csv << n << ',' << gamma << ',' << k << ',' << accepted
						<< ",0," << bout << ',' << word << ','
						<< amp.real() << ',' << amp.imag() << '\n';
				}
				++accepted;
			}

			if (accepted == 0) {
				/* Large γ: no-jump trajectories are exponentially unlikely, fall back to the direct K0 construction (operation-wise equivalent) */
				random_engine::get_instance().set_seed(k_base_seed + 424242);
				qram.set_memory_random();
				qram.prepare_all();
				overwrite_with_k0_only(qram, gamma);
				qram.run_bad();
				bus_t bout = qram.get_memory()[0];
				size_t non_idle = 0;
				auto amps = collect_word_amps(qram, non_idle);
				non_idle_total += non_idle;
				for (auto& [word, amp] : amps) {
					csv << n << ',' << gamma << ',' << k << ",0,0," << bout << ','
						<< word << ',' << amp.real() << ',' << amp.imag() << '\n';
				}
				std::cout << "[hamming] n=" << n << " gamma=" << gamma
					<< ": 0/" << attempts << " accepted -> direct-K0 fallback"
					<< " (non_idle=" << non_idle << ")\n";
			}
			else {
				std::cout << "[hamming] n=" << n << " gamma=" << gamma
					<< ": accepted " << accepted << " no-jump traj in "
					<< attempts << " attempts (non_idle=" << non_idle_total << ")\n";
			}
		}
	}
}

/* ------------------------------ mode avgfid ------------------------------ */

void run_avgfid(const std::filesystem::path& outdir)
{
	/* Resume support: tally (trajectories, fired count) per (n,gamma) already in the CSV;
	 * completed points are skipped, incomplete points append from the saved trajectory index.
	 * Note: the CSV prints doubles with setprecision(17); std::stod round-trips exactly,
	 * so (n,gamma) is safe to match as a double key. */
	std::map<std::pair<size_t, double>, std::pair<size_t, size_t>> done;
	auto path = outdir / "verify_avgfid.csv";
	if (std::filesystem::exists(path)) {
		std::ifstream in(path);
		std::string line;
		std::getline(in, line); // header
		while (std::getline(in, line)) {
			if (line.empty()) continue;
			std::stringstream ss(line);
			std::string fn, fg, ft, fh, fi;
			std::getline(ss, fn, ','); std::getline(ss, fg, ',');
			std::getline(ss, ft, ','); std::getline(ss, fh, ',');
			std::getline(ss, fi, ',');
			auto& rec = done[{std::stoul(fn), std::stod(fg)}];
			++rec.first;
			if (!fi.empty() && std::stod(fi) > 0.5) ++rec.second;
		}
	}
	std::ofstream csv;
	if (std::filesystem::exists(path)) {
		csv.open(path, std::ios::app);
		csv << std::setprecision(17);
	} else {
		csv = open_csv(outdir, "verify_avgfid.csv");
		csv << "n,gamma,traj,has_fault,infidelity\n";
	}

	constexpr size_t k = k_data_bits;
	constexpr size_t trials = 100;
	const std::vector<size_t> n_list = {3, 5, 8};

	/* Overlap of the trajectory final state with the ideal output (pre-sampling; only tree-idle, correct-word components contribute) */
	auto ideal_overlap = [](qram_qubit::QRAMCircuit& qram) -> complex_t {
		complex_t overlap = 0;
		for (auto& g : qram.branch_groups) {
			bus_t mem = qram.get_memory()[g.address];
			for (size_t b = 0; b < g.branches.size(); ++b) {
				bus_t ideal = g.branches[b].bus_input ^ mem;
				for (auto it = g.branches[b].iterbeg(); it != g.branches[b].iterend(); ++it) {
					if (it->data_bus == ideal && it->state.nz_elements.empty())
						overlap += g.branch_probs[b] * it->amplitude;
				}
			}
		}
		return overlap;
	};

	/* Smoke self-check: infidelity = 0 at gamma=0 */
	{
		random_engine::get_instance().set_seed(k_base_seed);
		qram_qubit::QRAMCircuit qram(3, k);
		qram.set_memory_random();
		qram.set_noise_models(damp_only(0.0));
		qram.set_input_uniform(k_branches);
		for (size_t traj = 0; traj < 2; ++traj) {
			random_engine::get_instance().set_seed(derive_run_seed(traj));
			qram.run_full();
			double norm = qram.get_normalization_factor();
			double inf = 1.0 - abs_sqr(ideal_overlap(qram)) / norm;
			if (!(std::abs(inf) < 1e-15)) {
				std::cerr << "[avgfid] SMOKE FAILED: infidelity=" << inf << "\n";
				std::exit(1);
			}
		}
		std::cout << "[avgfid] smoke gamma=0 passed: |infidelity|<1e-15\n";
	}

	for (size_t n : n_list) {
		random_engine::get_instance().set_seed(k_base_seed);
		qram_qubit::QRAMCircuit qram(n, k);
		qram.set_memory_random();
		qram.set_noise_models(damp_only(0.0));
		qram.set_input_uniform(k_branches);

		/* Inside the rare-event window, extend up to target_fires fired trajectories (capped);
		 * fired := infidelity > 0.5 (fired jump → inf≈1; no fire → ≤1e-3,
		 * 3 orders of magnitude apart, a robust criterion). At large γ, 100 trajectories saturate; no extension. */
		const size_t target_fires = 120;
		const size_t traj_cap = (n == 3 ? 150000 : n == 5 ? 15000 : 10000);

		for (double gamma : k_gamma_grid) {
			bool extend = (gamma >= 1e-4 && gamma <= 3e-3)
				|| (gamma == 1e-5 && n <= 5);
			size_t base_traj = 0, base_fired = 0;
			if (auto it = done.find({n, gamma}); it != done.end()) {
				base_traj = it->second.first;
				base_fired = it->second.second;
			}
			if (base_traj >= trials &&
				(!extend || base_fired >= target_fires || base_traj >= traj_cap)) {
				std::cout << "[avgfid] n=" << n << " gamma=" << gamma
					<< " already complete (" << base_traj << " traj, "
					<< base_fired << " fired), skip\n";
				continue;
			}
			qram.set_noise_models(damp_only(gamma));
			size_t n_fault = 0, n_fired = base_fired, traj = base_traj;
			while (traj < trials ||
				   (extend && n_fired < target_fires && traj < traj_cap)) {
				random_engine::get_instance().set_seed(derive_run_seed(traj));
				qram.run_full();
				double norm = qram.get_normalization_factor();
				double inf = 1.0 - abs_sqr(ideal_overlap(qram)) / norm;
				bool hf = has_damp_full_op(qram);
				n_fault += hf ? 1 : 0;
				n_fired += (inf > 0.5) ? 1 : 0;
				csv << n << ',' << gamma << ',' << traj << ','
					<< (hf ? 1 : 0) << ',' << inf << '\n';
				++traj;
			}
			std::cout << "[avgfid] n=" << n << " gamma=" << gamma
				<< " done (fault traj: " << n_fault << "/" << traj
				<< ", fired: " << n_fired << ")\n";
		}
	}
}

/* ------------------------------ mode qutrit ------------------------------ */

void run_qutrit(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "verify_qutrit.csv");
	csv << "n,gamma,traj,address,bus_in,weight_ratio_meas,"
		"weight_ratio_theory,fidelity_to_ideal\n";

	constexpr size_t n = 3, k = 3;
	constexpr size_t target_traj = 5;
	constexpr size_t attempt_cap = 100000;
	const size_t n_branches = pow2(n + k); /* full coverage: 8 addresses × 8 bus values */

	auto process = [&](qram_qutrit::QRAMCircuit& qram, double gamma, size_t traj) {
		/* Analytic counter: branch 0 is the reference, relative_multiplier = (1-gamma)^Δc */
		std::vector<size_t> ids;
		for (size_t i = 1; i < qram.branches.size(); ++i)
			ids.push_back(i);
		qram.time_step.get_multiplier_qutrit(gamma, qram.time_step.last_step(),
			qram.branches, 0, ids, qram.get_memory());

		double w_ref = qram.branches[0].get_prob();
		for (size_t i = 0; i < qram.branches.size(); ++i) {
			auto& br = qram.branches[i];
			double w_i = br.get_prob();
			double ratio_meas = w_i / w_ref;
			double ratio_theory = (i == 0) ? 1.0 : br.relative_multiplier;

			bus_t ideal = br.bus_input ^ qram.get_memory()[br.address];
			complex_t ov = 0;
			for (auto it = br.iterbeg(); it != br.iterend(); ++it)
				if (it->data_bus == ideal && it->state.nz_elements.empty())
					ov += it->amplitude;
			double fid = abs_sqr(ov) / w_i;

			csv << n << ',' << gamma << ',' << traj << ',' << br.address << ','
				<< br.bus_input << ',' << ratio_meas << ',' << ratio_theory << ','
				<< fid << '\n';
		}
	};

	/* Smoke self-check: ratio=1 and fidelity=1 at gamma=0 */
	{
		random_engine::get_instance().set_seed(k_base_seed);
		qram_qutrit::QRAMCircuit qram(n, k);
		qram.set_memory_random();
		qram.set_noise_models(damp_only(0.0));
		qram.set_input_uniform(n_branches);
		qram.run_full();
		std::vector<size_t> ids;
		for (size_t i = 1; i < qram.branches.size(); ++i) ids.push_back(i);
		qram.time_step.get_multiplier_qutrit(0.0, qram.time_step.last_step(),
			qram.branches, 0, ids, qram.get_memory());
		double w_ref = qram.branches[0].get_prob();
		double max_ratio_dev = 0, max_fid_dev = 0;
		for (size_t i = 0; i < qram.branches.size(); ++i) {
			auto& br = qram.branches[i];
			double ratio_meas = br.get_prob() / w_ref;
			double ratio_theory = (i == 0) ? 1.0 : br.relative_multiplier;
			max_ratio_dev = std::max(max_ratio_dev,
				std::abs(ratio_meas - ratio_theory));
			bus_t ideal = br.bus_input ^ qram.get_memory()[br.address];
			complex_t ov = 0;
			for (auto it = br.iterbeg(); it != br.iterend(); ++it)
				if (it->data_bus == ideal && it->state.nz_elements.empty())
					ov += it->amplitude;
			max_fid_dev = std::max(max_fid_dev,
				std::abs(abs_sqr(ov) / br.get_prob() - 1.0));
		}
		if (!(max_ratio_dev < 1e-13 && max_fid_dev < 1e-13)) {
			std::cerr << "[qutrit] SMOKE FAILED: ratio_dev=" << max_ratio_dev
				<< " fid_dev=" << max_fid_dev << "\n";
			std::exit(1);
		}
		std::cout << "[qutrit] smoke gamma=0 passed: ratio=1, fidelity=1\n";
	}

	for (double gamma : {1e-4, 1e-3, 1e-2}) {
		size_t accepted = 0, attempts = 0;
		while (accepted < target_traj && attempts < attempt_cap) {
			seed_t seed = k_base_seed + 1000003 * attempts;
			++attempts;
			random_engine::get_instance().set_seed(seed);
			qram_qutrit::QRAMCircuit qram(n, k);
			qram.set_memory_random();       /* re-randomize memory per trajectory */
			qram.set_noise_models(damp_only(gamma));
			qram.set_input_uniform(n_branches);
			qram.pick_all();
			if (has_damp_full_op(qram))
				continue;
			qram.run_valid_branches();
			process(qram, gamma, accepted);
			++accepted;
		}
		std::cout << "[qutrit] gamma=" << gamma << ": accepted " << accepted
			<< " no-jump traj in " << attempts << " attempts\n";
	}
}

/* ------------------------------ mode auditn3 ------------------------------ */

/* Per-slice driver: replicates QRAMCircuit::run_bad's operator dispatch (only the 6
 * operator kinds of a noise-free circuit; anything else errors out), applying the injection
 * right after slice inject_slice ends:
 *   fault=0 (BitFlip): run_bitflip(pos) (same effect as append+run_bad);
 *   fault=1 (Damp_Full): first sum the exact firing probability exc = Σ_groups get_prob_damp(pos)[0]
 *     (noise-free ⇒ norm=1, so exc is the roulette firing probability of run_damp_full),
 *     then apply the deterministic projection Branch::run_damp_full(pos,0) to all branches —
 *     identical, operation by operation, to the roulette "firing" the branches.
 * Returns exc (always 1 for BitFlip). */
double run_with_injection(qram_qubit::QRAMCircuit& qram, size_t inject_slice,
	size_t pos, int fault)
{
	double exc = 1.0;
	size_t slice = 0;
	for (auto& pack : qram.operations.time_slices) {
		for (auto& op : pack.operations) {
			switch (op.type) {
			case OperationType::ControlSwap:
				qram.run_cswap(op.targets[0]); break;
			case OperationType::CopyIn:
				if (op.targets[0] == 0) qram.run_hadamard();
				qram.run_busin(op.targets[0]);
				break;
			case OperationType::CopyOut:
				qram.run_busout(op.targets[0]);
				if (op.targets[0] == qram.data_size - 1) qram.run_hadamard();
				break;
			case OperationType::SwapInternal:
				qram.run_swap(op.targets[0]); break;
			case OperationType::FirstCopy:
				qram.run_acopy(op.targets[0]); break;
			case OperationType::FetchData:
				qram.run_fetchdata(op.targets[0]); break;
			default:
				throw std::runtime_error("audit driver: unexpected op type");
			}
		}
		if (slice == inject_slice) {
			if (fault == 0) {
				qram.run_bitflip(pos);
			}
			else {
				exc = 0;
				for (auto* g : qram.valid_branch_group_view)
					exc += g->get_prob_damp(pos)[0];
				if (exc > 1e-15) {
					for (auto* g : qram.valid_branch_group_view)
						for (auto& b : g->branches)
							b.run_damp_full(pos, 0);
				}
			}
		}
		++slice;
	}
	return exc;
}

/* Full-state signature (for the bitwise driver-vs-run_bad cross-check) */
std::string state_signature(qram_qubit::QRAMCircuit& qram)
{
	std::string sig;
	for (auto& g : qram.branch_groups)
		for (auto& b : g.branches)
			for (auto it = b.iterbeg(); it != b.iterend(); ++it) {
				char buf[64];
				std::snprintf(buf, sizeof buf, "%zu/%zu/%a/%a|",
					g.address, (size_t)it->data_bus,
					it->amplitude.real(), it->amplitude.imag());
				sig += buf;
				for (size_t e : it->state.nz_elements)
					sig += std::to_string(e) + ".";
				sig += ";";
			}
	return sig;
}

void run_auditn3(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "verify_audit_n3.csv");
	csv << "fault_type,pos,slot,slice,predicted_bad_set,measured_bad_set,match\n";

	constexpr size_t n = 3, k = 3;

	random_engine::get_instance().set_seed(k_base_seed);
	qram_qubit::QRAMCircuit qram(n, k);
	{
		/* Memory with pairwise distinct words: reading the wrong cell necessarily yields a wrong word, ruling out coincidental misses */
		memory_t mem(pow2(n));
		for (size_t i = 0; i < mem.size(); ++i) mem[i] = i;
		qram.set_memory(mem);
	}
	qram.set_noise_models({});
	qram.set_input_uniform(pow2(n + k));    /* all 64 branches covered, uniform weights */

	qram.prepare_all();
	const size_t n_slices = qram.operations.time_slices.size();
	const size_t n_pos = 2 * (pow2(n) - 1); /* 7 nodes × {addr,data} slots = 14 */
	std::cout << "[auditn3] n=3, pos=" << n_pos << " slots, slices=" << n_slices
		<< " (full_step=" << qram.time_step.full_step() << ")\n";

	/* Driver cross-check: the per-slice driver without injection must match run_bad bitwise */
	{
		qram.prepare_all();
		qram.run_bad();
		std::string sig_ref = state_signature(qram);
		qram.prepare_all();
		run_with_injection(qram, SIZE_MAX, 0, 0);
		std::string sig_drv = state_signature(qram);
		if (sig_ref != sig_drv) {
			std::cerr << "[auditn3] SMOKE FAILED: slice driver != run_bad\n";
			std::exit(1);
		}
		std::cout << "[auditn3] driver-vs-run_bad bitwise check passed\n";
	}

	struct Stats {
		size_t total = 0, contained = 0, equal = 0, vacuous = 0, violations = 0;
	};
	std::map<std::string, Stats> stats;
	std::map<std::string, std::map<size_t, std::set<size_t>>> union_measured;

	for (int fault = 0; fault < 2; ++fault) {
		const char* fault_name = fault == 0 ? "BitFlip" : "Damp_Full";
		for (size_t pos = 0; pos < n_pos; ++pos) {
			size_t node = pos / 2, slot = pos % 2;
			auto [plo, phi] = fault == 0
				? qram.time_step.get_bad_range_qubit(pos)
				: qram.time_step.get_bad_range_qutrit(pos);
			std::set<size_t> predicted;
			for (size_t a = plo; a <= phi; ++a) predicted.insert(a);

			for (size_t slice = 0; slice < n_slices; ++slice) {
				qram.prepare_all();
				double fire_prob;
				if (fault == 0) {
					qram.operations.time_slices[slice].append(
						Operation(OperationType::BitFlip, {pos}));
					qram.run_bad();
					fire_prob = 1.0;
				}
				else {
					fire_prob = run_with_injection(qram, slice, pos, 1);
				}

				auto& st = stats[fault_name];
				++st.total;

				if (fire_prob < 1e-15) {
				/* The excitation probability of this bit is identically 0 across all
				 * branches at this time: the jump is physically impossible (a vacuous case), excluded from the consistency stats */
					++st.vacuous;
					csv << fault_name << ',' << pos << ',' << slot << ',' << slice
						<< ',' << join_set(predicted) << ",vacuous,-1\n";
					continue;
				}

				/* Measured: wrong-bus set + terminal config families per address */
				std::set<size_t> wrongbus;
				std::map<size_t, std::set<std::set<size_t>>> families;
				for (auto& g : qram.branch_groups) {
					bus_t mem = qram.get_memory()[g.address];
					for (auto& b : g.branches) {
						bus_t ideal = b.bus_input ^ mem;
						for (auto it = b.iterbeg(); it != b.iterend(); ++it) {
							if (std::abs(it->amplitude) < 1e-10)
								continue;
							families[g.address].insert(it->state.nz_elements);
							if (it->data_bus != ideal)
								wrongbus.insert(g.address);
						}
					}
				}

				/* Reference family = the config family of the first address outside the
				 * predicted range; damage = wrong-bus ∪ config family deviating from the reference family */
				std::set<size_t> measured = wrongbus;
				size_t ref_addr = SIZE_MAX;
				for (size_t a = 0; a < pow2(n); ++a)
					if (!predicted.count(a)) { ref_addr = a; break; }
				if (ref_addr != SIZE_MAX) {
					const auto& fam_ref = families[ref_addr];
					for (size_t a = 0; a < pow2(n); ++a)
						if (families[a] != fam_ref)
							measured.insert(a);
				}

				bool contained = std::includes(predicted.begin(), predicted.end(),
					measured.begin(), measured.end());
				bool equal = (measured == predicted);
				st.contained += contained ? 1 : 0;
				st.equal += equal ? 1 : 0;
				for (size_t a : measured)
					union_measured[fault_name][pos].insert(a);

				csv << fault_name << ',' << pos << ',' << slot << ',' << slice
					<< ',' << join_set(predicted) << ',' << join_set(measured)
					<< ',' << (contained ? 1 : 0) << '\n';

				if (!contained) {
					++st.violations;
					std::cout << "[auditn3] VIOLATION " << fault_name
						<< " node=" << node << " slot=" << slot
						<< " slice=" << slice
						<< " predicted={" << join_set(predicted)
						<< "} measured={" << join_set(measured) << "}\n";
				}
			}
		}
	}

	std::cout << "\n[auditn3] ==== summary ====\n";
	for (auto& [name, st] : stats) {
		std::cout << name << ": cases=" << st.total
			<< " contained=" << st.contained
			<< " equal=" << st.equal
			<< " vacuous=" << st.vacuous
			<< " violations=" << st.violations << "\n";
	}
	std::cout << "[auditn3] per-position union of measured sets over slices:\n";
	for (int fault = 0; fault < 2; ++fault) {
		const char* fault_name = fault == 0 ? "BitFlip" : "Damp_Full";
		for (size_t pos = 0; pos < n_pos; ++pos) {
			size_t node = pos / 2;
			auto [plo, phi] = fault == 0
				? qram.time_step.get_bad_range_qubit(pos)
				: qram.time_step.get_bad_range_qutrit(pos);
			std::set<size_t> predicted;
			for (size_t a = plo; a <= phi; ++a) predicted.insert(a);
			std::cout << "  " << fault_name << " pos=" << pos
				<< " (node=" << node << (node % 2 == 1 && node > 0 ? ",left-child" :
					node % 2 == 0 && node > 0 ? ",right-child" : ",root")
				<< (pos % 2 ? ",data" : ",addr") << ")"
				<< " predicted={" << join_set(predicted)
				<< "} measured_union={" << join_set(union_measured[fault_name][pos])
				<< "}\n";
		}
	}
}

} // namespace

int main(int argc, const char** argv)
{
	if (argc < 2) {
		std::cerr << "usage: QubitPaperVerify <hamming|avgfid|qutrit|auditn3> [outdir]\n";
		return 1;
	}
	std::filesystem::path outdir = (argc >= 3) ? argv[2] : "results";
	std::string mode = argv[1];

	if (mode == "hamming")
		run_hamming(outdir);
	else if (mode == "avgfid")
		run_avgfid(outdir);
	else if (mode == "qutrit")
		run_qutrit(outdir);
	else if (mode == "auditn3")
		run_auditn3(outdir);
	else {
		std::cerr << "unknown mode: " << mode << "\n";
		return 1;
	}
	return 0;
}
