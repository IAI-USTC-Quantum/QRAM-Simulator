/* qubit-encoded QRAM 论文实验驱动：重跑 fig_fidelity / fig_perf / tab:equiv 的原始数据。
 *
 * 协议与 QRAMFidelityV2/QRAMSimulatorTest.cpp 的 test_qubit_compare 一致：
 *   - memory 与 500 分支均匀叠加输入由 base seed 决定（每条轨迹相同）；
 *   - 每条轨迹 set_seed(base+it) 后 reseed() 得到 runseed，控制该轨迹的噪声采样；
 *   - full 与 pruned（normal）在同一 runseed 下对拍。
 * 噪声约定：eps = gamma，即 Depolarizing 与 Damping 同时置为同一数值。
 *
 * 用法：
 *   QubitPaperScan fidelity [outdir]   # fig 3：qubit 3 档噪声 × n=3..10 × 200 轨迹
 *                                      #        + qutrit 1e-5（fig 3b 对比线）
 *   QubitPaperScan perf [outdir]       # fig 4：qubit n=4..12 × {0,1e-5,1e-4,1e-3} × 10 轨迹
 *   QubitPaperScan equiv [outdir]      # tab:equiv：4 个工作点 × 3 个显式种子
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

/* tab:equiv 的显式种子（沿用论文原表） */
constexpr seed_t k_equiv_seeds[] = {880000, 880007, 880014};

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
	seed_t base_seed, size_t input_sz)
{
	random_engine::get_instance().set_seed(base_seed);

	QRAM_type qram(addr_sz, data_sz);
	qram.set_memory_random();
	qram.set_noise_models(noise);
	qram.set_input_uniform(input_sz);

	return qram;
}

/* run 结束后（采样前）显式存储的 branch sub-state 总数。
 * 只统计 valid_branch_group_view（实际被演化的分支：bad + 参考 good），
 * pruned 模式下 good 分支的输入态未被演化，不应计入。 */
size_t count_explicit_states(const qram_qubit::QRAMCircuit& qram)
{
	size_t count = 0;
	for (const auto* group : qram.valid_branch_group_view)
		for (const auto& branch : group->branches)
			count += branch.system_states_sz;
	return count;
}

/* 实际被显式演化的分支数（bad + 参考 good）；full 模式下等于全部输入分支。 */
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
	seed_t base_seed, size_t input_sz, seed_t run_seed, std::string version)
{
	QubitTrajectoryResult result;
	auto qram = configure_qram<qram_qubit::QRAMCircuit>(addr_sz, k_data_bits, noise, base_seed, input_sz);

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

	/* qubit：3 档噪声 × n=3..10（fig 3a；1e-5 行同时供 fig 3b 使用） */
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

	/* qutrit：1e-5，full 模式（fig 3b 对比线） */
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

	constexpr size_t trials = 10;
	const std::vector<double> eps_list = {0.0, 1e-5, 1e-4, 1e-3};
	const std::vector<size_t> n_list = {4, 6, 8, 10, 12};

	for (double eps : eps_list) {
		auto noise = make_noise(eps);
		for (size_t n : n_list) {
			for (size_t traj = 0; traj < trials; ++traj) {
				seed_t run_seed = derive_run_seed(traj);
				for (std::string version : {qram_qubit::QRAMCircuit::FULL_VER, qram_qubit::QRAMCircuit::NORMAL_VER}) {
					auto r = run_qubit_once(n, noise, k_base_seed, k_branches, run_seed, version);
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

/* 验证 ε=0 时 pruned 模式只保留 1 个 branch group（参考分支），
 * 并统计参考组的 sub-state 构成 */
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

/* 机制分解：单通道（depol-only / damp-only）下 qubit vs qutrit 的保真度。
 * ε=1e-4（1e-5 下 damping 几乎无事件，看不见通道差异），n∈{8,10}，k∈{1,5} */
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

/* 深树单故障注入审计：在指定节点注入单个 X(BitFlip) 故障，测 full 模式
 * 实际损伤的地址集合，对比解析 bad range（论文包络）与 Hann 左端口链传播预期 */
void run_audit(const std::filesystem::path& outdir)
{
	auto csv = open_csv(outdir, "audit.csv");
	csv << "arch,n,depth,side,node,pos,slot,slice,groups_total,damaged,min_addr,max_addr,"
		"analytic_lo,analytic_hi,analytic_size\n";

	constexpr size_t n = 10;
	constexpr size_t input_sz = pow2(n + k_data_bits); /* 全地址覆盖 */

	for (size_t depth : {4, 6, 8}) {
		for (size_t side : {0, 1}) { /* 0=最左链, 1=最右链 */
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

/* k 扫描：验证 qubit/qutrit 保真度差距是否随数据位 k 增长。
 * Scan A：n=8 固定，k ∈ {1,2,3,5}，eps ∈ {1e-5, 1e-4}
 * Scan B：k ∈ {1,5}，n ∈ {4,6,10}，eps = 1e-4（n 标度斜率） */
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

/* 加密 n 扫描：ε=3e-5 中等强度（避免 1e-4 饱和与 1e-5 尾部噪声），
 * n=4..10 逐点，k∈{1,5}，300 轨迹 —— 用于渐近斜率拟合 */
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

/* 全地址覆盖 + 零总线输入：数据加载任务约定（每个地址一个分支，bus=0，均匀权重） */
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

/* 等比特数架构对比：A=(n,d) 并行 vs B=(n+log2 d,1) 扩址，
 * 数据加载任务（全地址覆盖、零总线），d=1 时两者全同作为自洽检查 */
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

/* 架构对比的通道分解：d=4, n=8, eps=1e-4, A 与 B 各跑 depol-only / damp-only */
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
