/*
 * QubitCorrespondenceExporter
 *
 * qubit-based QRAM（QRAM/include/qram_circuit_qubit.h，phase-kickback 版 FetchData
 * + 真实 Hadamard）与电路级模拟对应的 C++ 导出端，与 QutritCorrespondenceExporter
 * 同构：提取 TimeStep 调度（门级翻译 + 采样噪声算子含 Damp_Full 的第二次抽样结果），
 * 输出原生/复刻双参考轨迹与四个 fidelity 口径。
 *
 * qubit 架构语义要点（与 qutrit 的差异）：
 *   - 节点 v 占 2 个普通 qubit：位置 = v*2+lr（0 → addr，1 → data），基态 = 未激发；
 *   - FetchData 为相位翻转（叶 data×addr 选 cell 的 −1 相位），CopyIn{0}/CopyOut{末}
 *     处 run_hadamard 对 bus 全位做 H —— 净语义仍为 bus_out = bus_in ⊕ memory[a]；
 *   - run_bitphaseflip 为 |1>→−|0>（−K1 跳变，非酉）→ Depolarizing k=2 的轨迹次归一化；
 *   - Damp_Full 只有单一跳变信道（k=0）。
 *
 * 编码：地址 addr_size 位（bit b ↔ qubit b）；bus data_size 位；节点 v：
 *   a = addr+data+2v，d = addr+data+2v+1。addr=2 → 9 个编码 qubit。
 */

#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "argparse.h"
#include "qram_circuit_qubit.h"
#include "time_step.h"

using namespace std;
using namespace qram_simulator;

namespace {

constexpr double MODEL_PI = 3.14159265358979323846;

struct Encoding
{
	size_t addr_size;
	size_t data_size;

	size_t num_qubits() const { return addr_size + data_size + 2 * (pow2(addr_size) - 1); }
	size_t addr_qubit(size_t bit) const { return bit; }
	size_t bus_qubit(size_t digit) const { return addr_size + digit; }
	size_t node_a(size_t v) const { return addr_size + data_size + 2 * v; }
	size_t node_d(size_t v) const { return addr_size + data_size + 2 * v + 1; }
};

string ctrls2str(const vector<pair<size_t, int>>& cs)
{
	string ret = "[";
	for (size_t i = 0; i < cs.size(); ++i)
	{
		if (i) ret += ",";
		ret += fmt::format("[{},{}]", cs[i].first, cs[i].second);
	}
	return ret + "]";
}

string gate_x(size_t t, const vector<pair<size_t, int>>& cs)
{
	return fmt::format("{{\"op\":\"X\",\"q\":[{}],\"ctrl\":{}}}", t, ctrls2str(cs));
}

string gate_h(size_t t)
{
	return fmt::format("{{\"op\":\"H\",\"q\":[{}],\"ctrl\":[]}}", t);
}

string gate_swap(size_t a, size_t b, const vector<pair<size_t, int>>& cs)
{
	return fmt::format("{{\"op\":\"SWAP\",\"q\":[{},{}],\"ctrl\":{}}}", a, b, ctrls2str(cs));
}

string gate_cnot(size_t c, size_t t)
{
	return fmt::format("{{\"op\":\"CNOT\",\"q\":[{},{}],\"ctrl\":[]}}", c, t);
}

string gate_u1(double theta, size_t t, const vector<pair<size_t, int>>& cs)
{
	return fmt::format("{{\"op\":\"U1\",\"q\":[{}],\"ctrl\":{},\"theta\":{:.17g}}}",
		t, ctrls2str(cs), theta);
}

/* 逻辑算子 → 门序列。镜像 qram_qubit::QRAMCircuit::run_bad 的 dispatch。
 * SwapInternal 对基态节点是恒等 → 无条件发射；ControlSwap 对两层非零集之外的
 * 节点也为恒等 → 无条件发射双方向受控 SWAP。 */
vector<string> translate_op(const Operation& op, const Encoding& enc, const memory_t& memory)
{
	vector<string> out;
	switch (op.type)
	{
	case OperationType::FirstCopy:
	{
		size_t bit = enc.addr_size - 1 - op.targets[0];
		out.push_back(gate_cnot(enc.addr_qubit(bit), enc.node_d(0)));
		break;
	}
	case OperationType::CopyIn:
	{
		if (op.targets[0] == 0)
			for (size_t dd = 0; dd < enc.data_size; ++dd)
				out.push_back(gate_h(enc.bus_qubit(dd)));
		out.push_back(gate_swap(enc.bus_qubit(op.targets[0]), enc.node_d(0), {}));
		break;
	}
	case OperationType::CopyOut:
	{
		out.push_back(gate_swap(enc.bus_qubit(op.targets[0]), enc.node_d(0), {}));
		if (op.targets[0] == enc.data_size - 1)
			for (size_t dd = 0; dd < enc.data_size; ++dd)
				out.push_back(gate_h(enc.bus_qubit(dd)));
		break;
	}
	case OperationType::SwapInternal:
	{
		size_t layer = op.targets[0];
		size_t lower = pow2(layer) - 1, upper = pow2(layer + 1) - 2;
		for (size_t v = lower; v <= upper; ++v)
			out.push_back(gate_swap(enc.node_a(v), enc.node_d(v), {}));
		break;
	}
	case OperationType::ControlSwap:
	{
		size_t layer = op.targets[0];
		size_t lower = pow2(layer) - 1, upper = pow2(layer + 1) - 2;
		for (size_t v = lower; v <= upper; ++v)
		{
			out.push_back(gate_swap(enc.node_d(v), enc.node_d(2 * v + 1),
				{ { enc.node_a(v), 0 } }));
			out.push_back(gate_swap(enc.node_d(v), enc.node_d(2 * v + 2),
				{ { enc.node_a(v), 1 } }));
		}
		break;
	}
	case OperationType::FetchData:
	{
		size_t digit = op.targets[0];
		size_t lower = pow2(enc.addr_size - 1) - 1, upper = pow2(enc.addr_size) - 2;
		for (size_t v = lower; v <= upper; ++v)
		{
			size_t off = v - lower;
			if (get_digit(memory[2 * off], digit))
				out.push_back(gate_u1(MODEL_PI, enc.node_d(v), { { enc.node_a(v), 0 } }));
			if (get_digit(memory[2 * off + 1], digit))
				out.push_back(gate_u1(MODEL_PI, enc.node_d(v), { { enc.node_a(v), 1 } }));
		}
		break;
	}
	default:
		throw std::runtime_error("translate_op(qubit): unexpected logical op " + op.to_string());
	}
	return out;
}

bool is_noise_op(OperationType t)
{
	switch (t)
	{
	case OperationType::SetZero:
	case OperationType::Damping:
	case OperationType::Damp_Common:
	case OperationType::Damp_Full:
	case OperationType::BitFlip:
	case OperationType::PhaseFlip:
	case OperationType::BitPhaseFlip:
	case OperationType::Depolarizing:
		return true;
	default:
		return false;
	}
}

string noise_type_name(OperationType t)
{
	switch (t)
	{
	case OperationType::BitFlip: return "BitFlip";
	case OperationType::PhaseFlip: return "PhaseFlip";
	case OperationType::BitPhaseFlip: return "BitPhaseFlip";
	case OperationType::Depolarizing: return "Depolarizing";
	case OperationType::Damp_Full: return "Damp_Full";
	case OperationType::Damp_Common: return "Damp_Common";
	case OperationType::SetZero: return "SetZero";
	default: return "Unknown";
	}
}

/* 设定输入：zerobus = 地址均匀叠加、bus=0（主配置）；uniform = 地址+bus 全均匀 */
void set_input(qram_qubit::QRAMCircuit& q, const string& mode)
{
	q.branch_groups.clear();
	size_t naddr = pow2(q.addr_size);
	if (mode == "uniform")
	{
		size_t total = pow2(q.addr_size + q.data_size);
		for (size_t a = 0; a < naddr; ++a)
		{
			q.branch_groups.emplace_back(a);
			auto& g = q.branch_groups.back();
			for (size_t b = 0; b < pow2(q.data_size); ++b)
			{
				g.branches_input.emplace_back(a, q.data_size, b);
				g.branch_probs.push_back(1.0 / (double)total);
				g.state_probs.push_back(1.0 / (double)total);
			}
		}
	}
	else
	{
		for (size_t a = 0; a < naddr; ++a)
		{
			q.branch_groups.emplace_back(a);
			auto& g = q.branch_groups.back();
			g.branches_input.emplace_back(a, q.data_size, 0);
			g.branch_probs.push_back(1.0 / (double)naddr);
			g.state_probs.push_back(1.0 / (double)naddr);
		}
	}
}

map<string, double> collect_output_distribution(const qram_qubit::QRAMCircuit& q)
{
	map<string, double> dist;
	for (auto& g : q.get_branch_groups())
	{
		for (size_t b = 0; b < g.branches.size(); ++b)
		{
			for (auto it = g.branches[b].iterbeg(); it != g.branches[b].iterend(); ++it)
			{
				dist[fmt::format("{}:{}", g.address, it->data_bus)]
					+= g.branch_probs[b] * abs_sqr(it->amplitude);
			}
		}
	}
	return dist;
}

string dist2str(const map<string, double>& dist)
{
	string ret = "{";
	bool first = true;
	for (auto& [k, v] : dist)
	{
		if (!first) ret += ",";
		first = false;
		ret += fmt::format("\"{}\":{:.17g}", k, v);
	}
	return ret + "}";
}

string memory2str(const memory_t& memory)
{
	string ret = "[";
	for (size_t i = 0; i < memory.size(); ++i)
	{
		if (i) ret += ",";
		ret += fmt::format("{}", memory[i]);
	}
	return ret + "]";
}

struct DampOutcome
{
	size_t step;
	size_t pos;
	int outcome;
};

/* 复刻 qram_qubit::QRAMCircuit::run_damp_full 的采样（prepare_all 路径：
 * first_good_branch_group == -1，multiplier 预处理跳过）。恰好一次 uniform01。 */
int replica_damp_full(qram_qubit::QRAMCircuit& q, size_t qubit_id, size_t step,
	std::vector<DampOutcome>* log)
{
	double prob_damp = 0;
	for (auto branch_ptr : q.valid_branch_group_view)
		prob_damp += branch_ptr->get_prob_damp(qubit_id)[0];
	double global_coef = q.get_normalization_factor_with_damping();
	double r = random_engine::get_instance().uniform01() * global_coef;
	int outcome = -1;
	if (r < prob_damp)
	{
		for (auto branch_group_ptr : q.valid_branch_group_view)
			for (auto& branch : branch_group_ptr->branches)
				branch.run_damp_full(qubit_id, 0);
		outcome = 0;
	}
	if (log)
		log->push_back({ step, qubit_id, outcome });
	return outcome;
}

/* 复刻执行：逻辑/噪声算子走公有 dispatch，Damp_Full 走拦截采样并记日志 */
void run_replica_trajectory(qram_qubit::QRAMCircuit& q, std::vector<DampOutcome>* damp_log)
{
	int step = 0;
	for (const OperationPack& ops : q.get_operations().time_slices)
	{
		++step;
		for (const Operation& op : ops.operations)
		{
			switch (op.type)
			{
			case OperationType::ControlSwap: q.run_cswap(op.targets[0]); break;
			case OperationType::CopyIn:
				if (op.targets[0] == 0) q.run_hadamard();
				q.run_busin(op.targets[0]);
				break;
			case OperationType::CopyOut:
				q.run_busout(op.targets[0]);
				if (op.targets[0] == q.data_size - 1) q.run_hadamard();
				break;
			case OperationType::SwapInternal: q.run_swap(op.targets[0]); break;
			case OperationType::FirstCopy: q.run_acopy(op.targets[0]); break;
			case OperationType::FetchData: q.run_fetchdata(op.targets[0]); break;
			case OperationType::BitFlip: q.run_bitflip(op.targets[0]); break;
			case OperationType::PhaseFlip: q.run_phaseflip(op.targets[0], op.coefficients[0]); break;
			case OperationType::BitPhaseFlip: q.run_bitphaseflip(op.targets[0]); break;
			case OperationType::Depolarizing: q.run_depolarizing(op.targets[0], op.coefficients[0]); break;
			case OperationType::Damp_Full:
				replica_damp_full(q, op.targets[0], (size_t)step, damp_log);
				q.clear_zero_elements();
				break;
			case OperationType::Damp_Common: q.run_damp_common(op.coefficients[0]); break;
			default:
				throw std::runtime_error("run_replica_trajectory(qubit): bad op");
			}
		}
	}
	q.clear_zero_elements();
}

string export_schedule(const qram_qubit::QRAMCircuit& q, const Encoding& enc,
	const noise_t& noise, seed_t seed, const string& input_mode,
	const std::map<std::pair<size_t, size_t>, int>& damp_outcomes)
{
	std::ostringstream out;
	out << "{\n";
	out << "  \"arch\": \"qubit\",\n";
	out << fmt::format("  \"addr_size\": {},\n  \"data_size\": {},\n", enc.addr_size, enc.data_size);
	out << fmt::format("  \"memory\": {},\n", memory2str(q.get_memory()));
	out << fmt::format("  \"seed\": {},\n", seed);
	string noise_str = "{";
	bool first = true;
	for (auto& [t, p] : noise)
	{
		if (!first) noise_str += ",";
		first = false;
		noise_str += fmt::format("\"{}\":{:.17g}", noise_type_name(t), p);
	}
	noise_str += "}";
	out << "  \"noise\": " << noise_str << ",\n";

	out << "  \"encoding\": {\n";
	out << fmt::format("    \"num_qubits\": {},\n", enc.num_qubits());
	out << "    \"address_qubits\": [";
	for (size_t b = 0; b < enc.addr_size; ++b)
		out << (b ? "," : "") << enc.addr_qubit(b);
	out << "],\n    \"bus_qubits\": [";
	for (size_t dd = 0; dd < enc.data_size; ++dd)
		out << (dd ? "," : "") << enc.bus_qubit(dd);
	out << "],\n    \"nodes\": [\n";
	for (size_t v = 0; v < pow2(enc.addr_size) - 1; ++v)
	{
		out << fmt::format("      {{\"id\":{}, \"a\":{}, \"d\":{}}}{}\n",
			v, enc.node_a(v), enc.node_d(v),
			(v + 1 < pow2(enc.addr_size) - 1) ? "," : "");
	}
	out << "    ],\n";
	out << "    \"noise_pos_convention\": \"node = pos/2, subsystem = pos%2 (0 -> addr qubit a, 1 -> data qubit d)\"\n";
	out << "  },\n";

	out << "  \"input\": {\"mode\": \"" << input_mode << "\", \"branches\": [\n";
	{
		auto& groups = q.get_branch_groups();
		size_t total = 0, done = 0;
		for (auto& g : groups) total += g.branches_input.size();
		for (auto& g : groups)
			for (size_t b = 0; b < g.branches_input.size(); ++b)
			{
				++done;
				out << fmt::format("    {{\"address\":{}, \"bus_input\":{}, \"prob\":{:.17g}}}{}\n",
					g.address, g.branches_input[b].bus_input, g.branch_probs[b],
					done < total ? "," : "");
			}
	}
	out << "  ]},\n";

	out << "  \"steps\": [\n";
	auto& slices = q.get_operations().time_slices;
	for (size_t s = 0; s < slices.size(); ++s)
	{
		size_t step = s + 1;
		vector<string> entries;
		vector<string> raw;
		for (const Operation& op : slices[s].operations)
		{
			raw.push_back(op.to_string());
			if (is_noise_op(op.type))
			{
				double coef = op.coefficients.empty() ? 0.0 : op.coefficients[0];
				size_t pos = op.targets.empty() ? (size_t)-1 : op.targets[0];
				if (op.type == OperationType::Damp_Common)
				{
					entries.push_back(fmt::format(
						"{{\"kind\":\"noise\",\"type\":\"Damp_Common\",\"pos\":-1,\"coef\":{:.17g},\"step\":{}}}",
						coef, step));
					continue;
				}
				if (op.type == OperationType::Damp_Full)
				{
					auto it = damp_outcomes.find({ step, pos });
					int outcome = it == damp_outcomes.end() ? -2 : it->second;
					entries.push_back(fmt::format(
						"{{\"kind\":\"noise\",\"type\":\"Damp_Full\",\"pos\":{},\"coef\":{:.17g},"
						"\"node\":{},\"sub\":\"{}\",\"step\":{},\"outcome\":{}}}",
						pos, coef, pos / 2, (pos % 2) ? "data" : "addr", step, outcome));
					continue;
				}
				entries.push_back(fmt::format(
					"{{\"kind\":\"noise\",\"type\":\"{}\",\"pos\":{},\"coef\":{:.17g},"
					"\"node\":{},\"sub\":\"{}\",\"step\":{}}}",
					noise_type_name(op.type), pos, coef, pos / 2, (pos % 2) ? "data" : "addr", step));
			}
			else
			{
				for (auto& g : translate_op(op, enc, q.get_memory()))
					entries.push_back("{\"kind\":\"gate\"," + g.substr(1));
			}
		}
		out << "    {\"step\":" << step << ",";
		out << "\"entangle_max\":" << q.time_step.layer_entangle_max(step) << ",";
		out << "\"raw\":[";
		for (size_t i = 0; i < raw.size(); ++i)
			out << (i ? "," : "") << fmt::format("\"{}\"", raw[i]);
		out << "],\"ops\":[";
		for (size_t i = 0; i < entries.size(); ++i)
			out << (i ? ",\n     " : "\n     ") << entries[i];
		out << (entries.empty() ? "" : "\n    ") << "]}";
		out << (s + 1 < slices.size() ? ",\n" : "\n");
	}
	out << "  ]\n}\n";
	return out.str();
}

memory_t parse_memory(const string& csv, size_t addr_size, size_t data_size)
{
	memory_t memory;
	stringstream ss(csv);
	string item;
	while (getline(ss, item, ','))
	{
		if (!item.empty())
			memory.push_back(stoull(item));
	}
	if (memory.size() != pow2(addr_size))
		throw std::runtime_error("memory size mismatch");
	for (auto v : memory)
		if (v >= pow2(data_size))
			throw std::runtime_error("memory entry exceeds data_size bits");
	return memory;
}

} // namespace

int main(int argc, const char** argv)
{
	size_t addr_size = 2;
	size_t data_size = 1;
	string memory_csv = "0,1,1,0";
	seed_t seed = 20260921;
	size_t runs = 200;
	double depol = 0.01;
	double damping = 0.0;
	string outdir = "results";
	string input_mode = "zerobus";
	size_t export_runs = 8;

	argparse::ArgumentParser parser("QubitCorrespondenceExporter",
		"Export qubit-architecture QRAM TimeStep schedule (gate-level + sampled noise ops) and reference output distributions");
	parser.add_argument().names({ "-a", "--addrsize" }).description("address size (>=2 for nonempty noise support)").required(false);
	parser.add_argument().names({ "-d", "--datasize" }).description("data size (default 1)").required(false);
	parser.add_argument().name("--memory").description("comma-separated memory cells (size = 2^addrsize)").required(false);
	parser.add_argument().name("--seed").description("random engine seed").required(false);
	parser.add_argument().name("--runs").description("number of trajectories for the averaged reference").required(false);
	parser.add_argument().name("--depolarizing").description("Depolarizing probability per active position per step").required(false);
	parser.add_argument().name("--damping").description("Damping probability").required(false);
	parser.add_argument().name("--exportruns").description("number of leading trajectories whose schedules are exported (default 8)").required(false);
	parser.add_argument().name("--input").description("input mode: zerobus (default) or uniform").required(false);
	parser.add_argument().name("--outdir").description("output directory (default: results)").required(false);
	parser.enable_help();
	auto err = parser.parse(argc, argv);
	if (err)
	{
		std::cout << err << std::endl;
		return 1;
	}
	if (parser.exists("help"))
	{
		parser.print_help();
		return 0;
	}
	if (parser.exists("addrsize")) addr_size = parser.get<size_t>("addrsize");
	if (parser.exists("datasize")) data_size = parser.get<size_t>("datasize");
	if (parser.exists("memory")) memory_csv = parser.get<string>("memory");
	if (parser.exists("seed")) seed = parser.get<seed_t>("seed");
	if (parser.exists("runs")) runs = parser.get<size_t>("runs");
	if (parser.exists("depolarizing")) depol = parser.get<double>("depolarizing");
	if (parser.exists("damping")) damping = parser.get<double>("damping");
	if (parser.exists("exportruns")) export_runs = parser.get<size_t>("exportruns");
	if (parser.exists("input")) input_mode = parser.get<string>("input");
	if (parser.exists("outdir")) outdir = parser.get<string>("outdir");
	if (input_mode != "zerobus" && input_mode != "uniform")
	{
		fmt::print("input mode must be zerobus or uniform\n");
		return 1;
	}
	if (addr_size < 1 || addr_size > 4 || data_size < 1 || data_size > 2)
	{
		fmt::print("addr/data size out of supported range\n");
		return 1;
	}

	memory_t memory = parse_memory(memory_csv, addr_size, data_size);
	bus_t bus_mask = pow2(data_size) - 1;
	noise_t noise;
	if (depol > 0.0) noise[OperationType::Depolarizing] = depol;
	if (damping > 0.0) noise[OperationType::Damping] = damping;

	Encoding enc{ addr_size, data_size };
	std::filesystem::create_directories(outdir);

	/* 1) 无噪声参考（Stage 0 基准） */
	map<string, double> noisefree_dist;
	{
		qram_qubit::QRAMCircuit q0(addr_size, data_size, memory_t(memory));
		set_input(q0, input_mode);
		q0.run_full();
		noisefree_dist = collect_output_distribution(q0);
	}

	/* 2) 原生轨迹：分布 + 四个 fidelity 口径（sample_and_get_fidelity 前计算） */
	string schedule_str;
	map<string, double> single_run_dist;
	map<string, double> avg_dist;
	map<size_t, map<string, double>> realization_dists;
	double fidelity_sum = 0.0;
	double fid_nopost_sum = 0.0;
	double fid_overlap_sum = 0.0;
	double fid_incoh_sum = 0.0;
	size_t native_sample_failures = 0;
	{
		qram_qubit::QRAMCircuit q(addr_size, data_size, memory_t(memory));
		q.set_noise_models(noise);
		random_engine::set_seed(seed);
		set_input(q, input_mode);

		for (size_t r = 0; r < runs; ++r)
		{
			q.prepare_all();
			q.run_bad();
			auto d = collect_output_distribution(q);
			if (r == 0)
				single_run_dist = d;
			if (r < export_runs)
				realization_dists[r] = d;
			for (auto& [k, v] : d)
				avg_dist[k] += v;

			complex_t coh = 0, ov = 0;
			double incoh = 0;
			for (auto& g : q.get_branch_groups())
			{
				for (size_t b = 0; b < g.branches.size(); ++b)
				{
					auto& br = g.branches[b];
					auto expect_bus = (br.bus_input ^ memory[g.address]) & bus_mask;
					complex_t Fa = 0, Ga = 0;
					for (auto it = br.iterbeg(); it != br.iterend(); ++it)
					{
						if (it->data_bus == expect_bus)
						{
							Fa += it->amplitude;
							if (it->state.nz_elements.empty())
								Ga += it->amplitude;
						}
					}
					coh += g.branch_probs[b] * Fa;
					ov += g.branch_probs[b] * Ga;
					incoh += g.branch_probs[b] * abs_sqr(Fa);
				}
			}
			fid_nopost_sum += abs_sqr(coh);
			fid_overlap_sum += abs_sqr(ov);
			fid_incoh_sum += incoh;

			try
			{
				fidelity_sum += q.sample_and_get_fidelity();
			}
			catch (const std::exception&)
			{
				/* sample_output 在全体振幅相干抵消为零时抛 Bad result
				   （qubit 架构 run_bitphaseflip 的簿记后果）——计为失败轨迹 */
				++native_sample_failures;
			}
		}
		for (auto& [k, v] : avg_dist)
			v /= (double)runs;
	}

	/* 3) 复刻轨迹：拦截 Damp_Full 第二次抽样 + 逐轨迹一致性校验 + 调度导出 */
	map<string, double> model_single_run_dist;
	map<string, double> model_avg_dist;
	double model_fidelity_sum = 0.0;
	size_t model_sample_failures = 0;
	{
		qram_qubit::QRAMCircuit q(addr_size, data_size, memory_t(memory));
		q.set_noise_models(noise);
		random_engine::set_seed(seed);
		set_input(q, input_mode);
		for (size_t r = 0; r < runs; ++r)
		{
			q.prepare_all();
			std::vector<DampOutcome> damp_log;
			run_replica_trajectory(q, &damp_log);
			auto d = collect_output_distribution(q);
			if (r == 0)
				model_single_run_dist = d;
			for (auto& [k, v] : d)
				model_avg_dist[k] += v;

			if (r < export_runs)
			{
				auto& nd = realization_dists[r];
				for (auto& [k, v] : nd)
					if (std::abs(d[k] - v) > 1e-12)
						throw std::runtime_error(
							fmt::format("replica/native mismatch at trajectory {}, key {}", r, k));
				std::map<std::pair<size_t, size_t>, int> outcomes;
				for (auto& o : damp_log)
					outcomes[{ o.step, o.pos }] = o.outcome;
				string s = export_schedule(q, enc, noise, seed, input_mode, outcomes);
				std::ofstream((std::filesystem::path(outdir)
					/ fmt::format("schedule_r{}.json", r)).string()) << s;
				if (r == 0)
					schedule_str = s;
			}
			try
			{
				model_fidelity_sum += q.sample_and_get_fidelity();
			}
			catch (const std::exception&)
			{
				++model_sample_failures;
			}
		}
		for (auto& [k, v] : model_avg_dist)
			v /= (double)runs;
	}

	std::ofstream((std::filesystem::path(outdir) / "schedule.json").string()) << schedule_str;

	std::ostringstream ref;
	ref << "{\n";
	ref << fmt::format("  \"seed\": {},\n  \"runs\": {},\n", seed, runs);
	ref << "  \"noise\": {";
	bool first = true;
	for (auto& [t, p] : noise)
	{
		if (!first) ref << ",";
		first = false;
		ref << fmt::format("\"{}\":{:.17g}", noise_type_name(t), p);
	}
	ref << "},\n";
	ref << "  \"noisefree_dist\": " << dist2str(noisefree_dist) << ",\n";
	ref << "  \"single_run_dist\": " << dist2str(single_run_dist) << ",\n";
	ref << "  \"avg_dist\": " << dist2str(avg_dist) << ",\n";
	ref << fmt::format("  \"avg_fidelity\": {:.17g},\n", fidelity_sum / (double)runs);
	ref << fmt::format("  \"avg_fid_nopost\": {:.17g},\n", fid_nopost_sum / (double)runs);
	ref << fmt::format("  \"avg_overlap_fid\": {:.17g},\n", fid_overlap_sum / (double)runs);
	ref << fmt::format("  \"avg_fid_incoh\": {:.17g},\n", fid_incoh_sum / (double)runs);
	ref << "  \"realizations\": {";
	{
		bool first_r = true;
		for (auto& [r, dist] : realization_dists)
		{
			if (!first_r) ref << ",";
			first_r = false;
			ref << fmt::format("\n    \"{}\": {}", r, dist2str(dist));
		}
		ref << "\n  },\n";
	}
	ref << "  \"model_single_run_dist\": " << dist2str(model_single_run_dist) << ",\n";
	ref << "  \"model_avg_dist\": " << dist2str(model_avg_dist) << ",\n";
	{
		double survival = 0.0, model_survival = 0.0;
		for (auto& [k, v] : avg_dist) survival += v;
		for (auto& [k, v] : model_avg_dist) model_survival += v;
		ref << fmt::format("  \"survival\": {:.17g},\n", survival);
		ref << fmt::format("  \"model_survival\": {:.17g},\n", model_survival);
	}
	ref << fmt::format("  \"sample_failures\": {},\n", native_sample_failures);
	ref << fmt::format("  \"model_avg_fidelity\": {:.17g}\n", model_fidelity_sum / (double)runs);
	if (native_sample_failures || model_sample_failures)
		fmt::print("note: {} native / {} replica trajectories hit the zero-amplitude "
			"Bad-result path (qubit bitphaseflip bookkeeping)\n",
			native_sample_failures, model_sample_failures);
	ref << "}\n";
	std::ofstream((std::filesystem::path(outdir) / "reference.json").string()) << ref.str();

	fmt::print("exported schedule.json + reference.json to {}\n", outdir);
	fmt::print("  noisefree  dist: {}\n", dist2str(noisefree_dist));
	fmt::print("  single run dist: {}\n", dist2str(single_run_dist));
	fmt::print("  avg fidelity    : {:.6f}\n", fidelity_sum / (double)runs);
	return 0;
}
