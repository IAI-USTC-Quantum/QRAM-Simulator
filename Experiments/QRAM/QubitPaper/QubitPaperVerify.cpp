/* QubitPaperVerify.cpp —— qubit 编码 QRAM 论文核心命题的针对性数值验证。
 *
 * 与 QubitPaperScan.cpp 同一协议：k_data_bits=3、k_base_seed 定 memory+输入、
 * 每轨迹 derive_run_seed(traj) 定噪声历史；所有分支级读数在 sample_output() 之前完成。
 *
 * 用法：Experiment_QRAM_QubitPaperVerify <mode> [outdir]
 *
 *   hamming  命题1+2（H→K0^d→H 闭式、no-jump 条件错误二阶）：
 *            单地址(addr=0)单总线(j=0)输入分支，damping-only，拒绝采样 no-jump
 *            轨迹（无 Damp_Full 操作），dump 全部 system_states 的 (word, amplitude)。
 *            大 γ 拒绝采样不可行时退化为"直接构造纯 K0 电路"（与 no-jump 轨迹逐比特
 *            等价，等价性在可同时采样的点上实测验证并打印）。
 *   avgfid   命题2（一阶/二阶分离）：damping-only，500 分支均匀输入，n∈{3,5,8}，
 *            每条轨迹 run_full 后采样前计算末态保真度 F=|<Ψ_ideal|ψ>|²/<ψ|ψ>
 *            （Ψ_ideal 只含树回 idle 的分量），记录该轨迹是否采到 Damp_Full。
 *            每点先跑 100 条（简报协议）；在 rare-event 窗口（1e-4≤γ≤3e-3，及
 *            n≤5 的 γ=1e-5）自适应加跑至 ≥120 条 fired（inf>0.5）轨迹，
 *            使全体平均 infidelity 的一阶斜率有足够统计（超出 100 条的行仍写入
 *            同一 CSV，traj 序号延续）。
 *   qutrit   命题3（qutrit 标量预测）：n=k=3 全覆盖 64 分支，no-jump 轨迹下逐分支
 *            对拍 显式权重比 |amp_i|²/|amp_ref|² vs 解析计数器 (1-γ)^Δc，
 *            以及归一化后与理想输出的保真度（应为 1）。
 *   auditn3  命题4（family-divergence 规则）：n=3 穷举单故障注入
 *            （7 节点 × {addr,data} 槽 × 全部切片 × {BitFlip, Damp_Full}），
 *            实测损伤地址集 vs 预测坏区间（BitFlip→get_bad_range_qubit，
 *            Damp_Full→get_bad_range_qutrit）。损伤 = 输出字错误 ∪ 配置族偏离参考族
 *            （参考族 = 预测区间外第一个地址的终态树配置集合；全树均匀 stray 视为
 *            可预测的单一家族，不计入损伤）。match=1 表示实测⊆预测（containment），
 *            match=-1 表示该槽位 jump 点火概率恒为 0（该时刻该比特全体分支均无
 *            激发，jump 在此物理上不可能发生，属空场景）。
 *            BitFlip 用 audit 同款 append+run_bad；Damp_Full 用逐切片驱动器在注入
 *            点先做精确点火概率求和 Σ get_prob_damp（无其它损耗源时即真点火概率），
 *            再对所有分支施加确定性投影 Branch::run_damp_full —— 与
 *            QRAMCircuit::run_damp_full 轮盘赌"点火"分支逐操作相同，但免掉重试。
 *            驱动器本身在审计前与 run_bad 做了逐比特对拍。
 *            memory[i]=i（字两两不同，排除"读错单元格但内容巧合相同"的漏检）。
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

/* 单分支输入：一个 branch group（address），一个 branch（bus 输入 j） */
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

/* 直接构造纯 K0（no-jump）操作序列：无故障切片 + 每片一个 Damp_Common(gamma)。
 * 与"采样到零故障的轨迹"逐操作相同（noise_one_step 在零故障时也只追加
 * Damp_Common），用于大 γ 下拒绝采样不可行的网格点。 */
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

	/* 冒烟自检：gamma=0 时 prob(b_out)=1，其余字为 0 */
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
				qram.set_memory_random();   /* 每条轨迹重新随机 memory：b_out 随之变化 */
				qram.prepare_all();
				if (has_damp_full_op(qram))
					continue;               /* 拒绝：含 jump 的轨迹 */

				qram.run_bad();

				/* 首个被接收轨迹：与直接 K0 构造对拍（同一 memory） */
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

					/* 恢复被对拍覆盖的状态：重跑被接收的轨迹 */
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
				/* 大 γ：no-jump 轨迹概率指数小，退化为直接 K0 构造（逐操作等价） */
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
	/* 续跑支持：统计已有 CSV 中每个 (n,gamma) 的 (轨迹数, fired 数)；
	 * 已完成的点跳过，未完成的点从断点轨迹号继续追加。
	 * 注意：CSV 用 setprecision(17) 打印 double，std::stod 精确往返，
	 * 因此 (n,gamma) 可作为 double 键安全匹配。 */
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

	/* 轨迹末态与理想输出的重叠（采样前；只有树回 idle 且字正确的分量有贡献） */
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

	/* 冒烟自检：gamma=0 时 infidelity = 0 */
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

		/* rare-event 窗口内加跑至 target_fires 条 fired 轨迹（受 cap 限制）；
		 * fired 判定 = infidelity > 0.5（fired jump → inf≈1；未点火 → ≤1e-3，
		 * 间隔 3 个数量级，判定稳健）。大 γ 点 100 条即饱和，不加跑。 */
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
	const size_t n_branches = pow2(n + k); /* 全覆盖 8 地址 × 8 总线值 */

	auto process = [&](qram_qutrit::QRAMCircuit& qram, double gamma, size_t traj) {
		/* 解析计数器：branch 0 为参考，relative_multiplier = (1-gamma)^Δc */
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

	/* 冒烟自检：gamma=0 时 ratio=1、fidelity=1 */
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
			qram.set_memory_random();       /* 每条轨迹重新随机 memory */
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

/* 逐切片驱动器：复刻 QRAMCircuit::run_bad 的算符分派（仅 noise-free 电路的
 * 6 种算符，其它算符直接报错），在 inject_slice 切片结束后施加注入：
 *   fault=0 (BitFlip)：run_bitflip(pos)（与 append+run_bad 相同的效果）；
 *   fault=1 (Damp_Full)：先求和精确点火概率 exc = Σ_groups get_prob_damp(pos)[0]
 *     （noise-free 下 norm=1，exc 即 run_damp_full 轮盘赌的点火概率），
 *     再对所有分支施加确定性投影 Branch::run_damp_full(pos,0) —— 与轮盘赌
 *     "点火"分支逐操作相同。
 * 返回 exc（BitFlip 恒为 1）。 */
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

/* 全状态签名（驱动器 vs run_bad 的逐比特对拍用） */
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
		/* 字两两不同的 memory：读错单元格必然给出错误字，排除巧合漏检 */
		memory_t mem(pow2(n));
		for (size_t i = 0; i < mem.size(); ++i) mem[i] = i;
		qram.set_memory(mem);
	}
	qram.set_noise_models({});
	qram.set_input_uniform(pow2(n + k));    /* 全覆盖 64 分支，均匀权重 */

	qram.prepare_all();
	const size_t n_slices = qram.operations.time_slices.size();
	const size_t n_pos = 2 * (pow2(n) - 1); /* 7 节点 × {addr,data} 槽 = 14 */
	std::cout << "[auditn3] n=3, pos=" << n_pos << " slots, slices=" << n_slices
		<< " (full_step=" << qram.time_step.full_step() << ")\n";

	/* 驱动器对拍：无注入的逐切片驱动必须与 run_bad 逐比特一致 */
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
					/* 该时刻该比特在全体分支中激发概率恒为 0：
					 * jump 物理上不可能发生（空场景），不计入一致性统计 */
					++st.vacuous;
					csv << fault_name << ',' << pos << ',' << slot << ',' << slice
						<< ',' << join_set(predicted) << ",vacuous,-1\n";
					continue;
				}

				/* 实测：wrong-bus 集 + 每个地址的终态配置族 */
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

				/* 参考族 = 预测区间外第一个地址的配置族；
				 * 损伤 = wrong-bus ∪ 配置族偏离参考族 */
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
