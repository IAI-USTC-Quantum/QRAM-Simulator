/*
 * QutritCorrespondenceExporter
 *
 * qutrit QRAM（QRAM-Simulator，TimeStep 轨迹级噪声模型）与电路级 channel 模拟
 * （UnifiedQuantum/uniqc，OriginIR-ext 内联噪声信道 + 密度矩阵后端）对应实验的
 * C++ 导出端：
 *   1. 构造 qram_qutrit::QRAMCircuit，设定 memory / noise_t / seed / uniform 输入；
 *   2. 从 QRAMCircuit::operations（TimeStep::generate 产物）提取：
 *        - 逐步逻辑算子 → qutrit→qubit 编码后的门序列（含控制极性）；
 *        - 采样得到的噪声算子 {step, type, pos, coef}（位置即“噪声信道位置”）；
 *   3. run_full() 精确轨迹演化 → 参考 output 分布（单次 + 多轨迹平均 + 自带 fidelity）；
 *   4. 全部写入 schedule.json / reference.json，供 Python 侧（uniqc）驱动消费。
 *
 * 编码约定（与 run_correspondence.py 严格一致）：
 *   - address 寄存器：addr_size 个 qubit，地址 bit b ↔ qubit b；
 *   - bus 寄存器：data_size 个 qubit，bus digit d ↔ qubit addr_size + d；
 *   - 路由节点 v（堆编号，v ∈ [0, 2^addr-1)）：
 *       a1 = addr+data+3v, a0 = addr+data+3v+1（qutrit 能级编码）, d = addr+data+3v+2；
 *       W=(a1,a0)=|00>（基态），L=|01>，R=|10>，|11> 为不可达死态；
 *   - 噪声位 pos：node = pos/2，子系统 = pos%2（0 → addr-qutrit 的 (a1,a0)，1 → data 位）。
 *
 * 门 IR：{"op": "X"|"SWAP"|"CNOT"|"U1", "q": [...], "ctrl": [[qubit, 期望值], ...], "theta": ...}
 *   CNOT 的 q=[control, target]；U1 = diag(1, e^{i·theta})；ctrl 空表 = 无控制。
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
#include "qram_circuit_qutrit.h"
#include "time_step.h"

using namespace std;
using namespace qram_simulator;

namespace {

constexpr double MODEL_PI = 3.14159265358979323846;

inline complex_t qutrit_omega(int k)
{
	return std::polar(1.0, 2.0 * MODEL_PI * (k % 3) / 3.0);
}

struct Ctrl
{
	size_t q;
	int v;
};

struct Encoding
{
	size_t addr_size;
	size_t data_size;

	size_t num_qubits() const { return addr_size + data_size + 3 * (pow2(addr_size) - 1); }
	size_t addr_qubit(size_t bit) const { return bit; }
	size_t bus_qubit(size_t digit) const { return addr_size + digit; }
	size_t node_a1(size_t v) const { return addr_size + data_size + 3 * v; }
	size_t node_a0(size_t v) const { return addr_size + data_size + 3 * v + 1; }
	size_t node_d(size_t v) const { return addr_size + data_size + 3 * v + 2; }
};

string ctrls2str(const vector<Ctrl>& cs)
{
	string ret = "[";
	for (size_t i = 0; i < cs.size(); ++i)
	{
		if (i) ret += ",";
		ret += fmt::format("[{},{}]", cs[i].q, cs[i].v);
	}
	return ret + "]";
}

string gate_x(size_t t, const vector<Ctrl>& cs)
{
	return fmt::format("{{\"op\":\"X\",\"q\":[{}],\"ctrl\":{}}}", t, ctrls2str(cs));
}

string gate_swap(size_t a, size_t b, const vector<Ctrl>& cs)
{
	return fmt::format("{{\"op\":\"SWAP\",\"q\":[{},{}],\"ctrl\":{}}}", a, b, ctrls2str(cs));
}

string gate_cnot(size_t c, size_t t, const vector<Ctrl>& cs)
{
	return fmt::format("{{\"op\":\"CNOT\",\"q\":[{},{}],\"ctrl\":{}}}", c, t, ctrls2str(cs));
}

string gate_u1(double theta, size_t t, const vector<Ctrl>& cs)
{
	return fmt::format("{{\"op\":\"U1\",\"q\":[{}],\"ctrl\":{},\"theta\":{:.17g}}}",
		t, ctrls2str(cs), theta);
}

/* 在 qubit 列表 qs（bit i ↔ qs[i]）上发射基矢置换 (pi ↔ pj)，整体受 cs 控制。
 * 标准共轭法：X 包裹 + CNOT 共轭把置换化到单 bit 翻转的多控 X。 */
void emit_transposition(vector<string>& out, const vector<size_t>& qs,
	uint64_t pi, uint64_t pj, const vector<Ctrl>& cs)
{
	size_t k = qs.size();
	vector<size_t> D;
	for (size_t b = 0; b < k; ++b)
		if (((pi >> b) & 1) != ((pj >> b) & 1))
			D.push_back(b);
	if (D.empty())
		return;

	auto others_ctrl = [&](size_t exclude)
	{
		vector<Ctrl> ret = cs;
		for (size_t b = 0; b < k; ++b)
			if (b != exclude)
				ret.push_back({ qs[b], (int)((pi >> b) & 1) });
		return ret;
	};

	if (D.size() == 1)
	{
		out.push_back(gate_x(qs[D[0]], others_ctrl(D[0])));
		return;
	}

	size_t p = D[0];
	int s = (int)((pi >> p) & 1);
	vector<size_t> rest(D.begin() + 1, D.end());

	if (s)
		out.push_back(gate_x(qs[p], cs));
	for (size_t b : rest)
		out.push_back(gate_cnot(qs[p], qs[b], cs));
	out.push_back(gate_x(qs[p], others_ctrl(p)));
	for (auto it = rest.rbegin(); it != rest.rend(); ++it)
		out.push_back(gate_cnot(qs[p], qs[*it], cs));
	if (s)
		out.push_back(gate_x(qs[p], cs));
}

/* 节点 v 的 internal_swap：(a1,a0,d) 上 (0,0,0)↔(0,1,0) 与 (0,0,1)↔(1,0,0)。
 * bit 顺序 (a1,a0,d) = (bit0,bit1,bit2)。 */
void emit_internal_swap(vector<string>& out, const Encoding& enc, size_t v, const vector<Ctrl>& cs)
{
	vector<size_t> qs = { enc.node_a1(v), enc.node_a0(v), enc.node_d(v) };
	emit_transposition(out, qs, 0b000, 0b010, cs);
	emit_transposition(out, qs, 0b100, 0b001, cs);
}

/* 逻辑算子 → 门序列。镜像 QRAMCircuit::run_valid_branches 的 dispatch 语义。 */
vector<string> translate_op(const Operation& op, const Encoding& enc, const memory_t& memory)
{
	vector<string> out;
	switch (op.type)
	{
	case OperationType::FirstCopy:
	{
		/* node0.data ^= address bit (addr_size-1-ℓ) */
		size_t bit = enc.addr_size - 1 - op.targets[0];
		out.push_back(gate_cnot(enc.addr_qubit(bit), enc.node_d(0), {}));
		break;
	}
	case OperationType::CopyIn:
	case OperationType::CopyOut:
	{
		/* SWAP(bus[digit], node0.data)；伴随的 try_merge 为 no-op */
		out.push_back(gate_swap(enc.bus_qubit(op.targets[0]), enc.node_d(0), {}));
		break;
	}
	case OperationType::SwapInternal:
	{
		size_t layer = op.targets[0];
		if (layer == 0)
		{
			emit_internal_swap(out, enc, 0, {});
		}
		else
		{
			/* layer ℓ-1 的每个节点按 addr 方向对 child 做 internal_swap */
			size_t lower = pow2(layer - 1) - 1, upper = pow2(layer) - 2;
			for (size_t v = lower; v <= upper; ++v)
			{
				size_t lchild = 2 * v + 1, rchild = 2 * v + 2;
				emit_internal_swap(out, enc, lchild,
					{ { enc.node_a1(v), 0 }, { enc.node_a0(v), 1 } });
				emit_internal_swap(out, enc, rchild,
					{ { enc.node_a1(v), 1 }, { enc.node_a0(v), 0 } });
			}
		}
		break;
	}
	case OperationType::ControlSwap:
	{
		size_t layer = op.targets[0];
		size_t lower = pow2(layer) - 1, upper = pow2(layer + 1) - 2;
		for (size_t v = lower; v <= upper; ++v)
		{
			out.push_back(gate_swap(enc.node_d(v), enc.node_d(2 * v + 1),
				{ { enc.node_a1(v), 0 }, { enc.node_a0(v), 1 } }));
			out.push_back(gate_swap(enc.node_d(v), enc.node_d(2 * v + 2),
				{ { enc.node_a1(v), 1 }, { enc.node_a0(v), 0 } }));
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
				out.push_back(gate_x(enc.node_d(v),
					{ { enc.node_a1(v), 0 }, { enc.node_a0(v), 1 } }));
			if (get_digit(memory[2 * off + 1], digit))
				out.push_back(gate_x(enc.node_d(v),
					{ { enc.node_a1(v), 1 }, { enc.node_a0(v), 0 } }));
		}
		break;
	}
	default:
		throw std::runtime_error("translate_op: unexpected logical op " + op.to_string());
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

/* 设定输入分支：uniform = 地址+bus 全均匀叠加（输出分布退化为均匀，仅作对照）；
 * zerobus = 地址均匀叠加、bus=0（主配置：输出 (a, memory[a]) 非平凡，分布可检验） */
void set_input(qram_qutrit::QRAMCircuit& q, const string& mode)
{
	auto& branches = q.branches;
	auto& probs = q.branch_probs;
	branches.clear();
	probs.clear();
	size_t naddr = pow2(q.address_size);
	if (mode == "uniform")
	{
		size_t total = pow2(q.address_size + q.data_size);
		for (size_t id = 0; id < total; ++id)
		{
			branches.emplace_back(id >> q.data_size, q.data_size,
				id & (pow2(q.data_size) - 1));
			probs.push_back(1.0 / (double)total);
		}
	}
	else
	{
		for (size_t a = 0; a < naddr; ++a)
		{
			branches.emplace_back(a, q.data_size, 0);
			probs.push_back(1.0 / (double)naddr);
		}
	}
}

map<string, double> collect_output_distribution(const qram_qutrit::QRAMCircuit& q)
{
	map<string, double> dist;
	auto& branches = q.get_branches();
	auto& probs = q.get_branch_probs();
	for (size_t i = 0; i < branches.size(); ++i)
	{
		for (auto it = branches[i].iterbeg(); it != branches[i].iterend(); ++it)
		{
			dist[fmt::format("{}:{}", branches[i].address, it->data_bus)]
				+= probs[i] * abs_sqr(it->amplitude);
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

/* schedule.json：config + encoding + input + 逐步（门 + 噪声算子，保持 pack 内顺序）。
 * damp_outcomes：Damp_Full 的第二次抽样结果（key = (step, pos)；-2 表示未记录）。 */
string export_schedule(const qram_qutrit::QRAMCircuit& q, const Encoding& enc,
	const noise_t& noise, seed_t seed, const string& input_mode,
	const std::map<std::pair<size_t, size_t>, int>& damp_outcomes = {})
{
	std::ostringstream out;

	out << "{\n";
	out << fmt::format("  \"arch\": \"qutrit\",\n");
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
	for (size_t d = 0; d < enc.data_size; ++d)
		out << (d ? "," : "") << enc.bus_qubit(d);
	out << "],\n    \"nodes\": [\n";
	for (size_t v = 0; v < pow2(enc.addr_size) - 1; ++v)
	{
		out << fmt::format("      {{\"id\":{}, \"a1\":{}, \"a0\":{}, \"d\":{}}}{}\n",
			v, enc.node_a1(v), enc.node_a0(v), enc.node_d(v),
			(v + 1 < pow2(enc.addr_size) - 1) ? "," : "");
	}
	out << "    ],\n";
	out << "    \"addr_basis\": {\"W\": \"00\", \"L\": \"01\", \"R\": \"10\", \"dead\": \"11\"},\n";
	out << "    \"noise_pos_convention\": \"node = pos/2, subsystem = pos%2 (0 -> addr qutrit (a1,a0), 1 -> data qubit d)\"\n";
	out << "  },\n";

	auto& branches = q.get_branches();
	auto& probs = q.get_branch_probs();
	out << "  \"input\": {\"mode\": \"" << input_mode << "\", \"branches\": [\n";
	for (size_t i = 0; i < branches.size(); ++i)
	{
		out << fmt::format("    {{\"address\":{}, \"bus_input\":{}, \"prob\":{:.17g}}}{}\n",
			branches[i].address, branches[i].bus_input, probs[i],
			(i + 1 < branches.size()) ? "," : "");
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
		out << "\"raw\":[" ;
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
	/* get_fidelity 的 expect_bus 直接 XOR 原始 memory 值，只有 data_size 位以内的
	 * 取值才是合法输入（set_memory_random 同约定）——越界值会被静默判 0 贡献 */
	for (auto v : memory)
		if (v >= pow2(data_size))
			throw std::runtime_error("memory entry exceeds data_size bits");
	return memory;
}

/* 模型级（注释语义）噪声算子：先把显式 (W,0) 归一化为 absent，再按纯状态函数作用。
 * 库侧 rotate_A1 穿透 / state_of(absent) / 采样闭区间已修复后，本轨迹与原生
 * run_valid_branches 应逐位一致 —— 保留作为修复的交叉验证。 */
void model_normalize(qram_qutrit::QRAMState& st)
{
	for (auto it = st.nz_elements.begin(); it != st.nz_elements.end();)
	{
		if (it->second.check_0())
			it = st.nz_elements.erase(it);
		else
			++it;
	}
}

void model_addr_cycle(qram_qutrit::QRAMState& st, size_t v, int ground_target)
{
	/* ground_target: A1 → R（W→R→L→W），A1² → L（W→L→R→W） */
	auto it = st.nz_elements.find(v);
	if (it == st.nz_elements.end())
	{
		st.nz_elements[v] = { ground_target, 0 };
		return;
	}
	auto& addr = it->second.addr;
	if (addr == W)
		addr = ground_target;
	else if (ground_target == qram_simulator::R)   /* A1: R→L, L→W */
	{
		if (addr == qram_simulator::R) addr = qram_simulator::L;
		else { addr = W; if (it->second.data == 0) st.nz_elements.erase(it); }
	}
	else                                          /* A1²: L→R, R→W */
	{
		if (addr == qram_simulator::L) addr = qram_simulator::R;
		else { addr = W; if (it->second.data == 0) st.nz_elements.erase(it); }
	}
}

void model_noise_op(qram_qutrit::QRAMCircuit& q, const Operation& op)
{
	for (auto& branch : q.branches)
	{
		for (auto it = branch.iterbeg(); it != branch.iterend(); ++it)
		{
			auto& st = it->state.nz_elements;
			model_normalize(it->state);
			size_t v = op.targets.empty() ? 0 : op.targets[0] / 2;
			double c = op.coefficients.empty() ? 0.0 : op.coefficients[0];
			bool odd = !op.targets.empty() && (op.targets[0] & 1);
			switch (op.type)
			{
			case OperationType::Depolarizing:
			{
				if (odd)
				{
					int k = int(std::floor(3 * c));
					auto&& [iter, flag] = st.insert({ v, {W, 1} });
					if (k == 0 || k == 2)
					{
						if (flag) { if (k == 2) it->amplitude *= -1.0; }
						else
						{
							bool was0 = iter->second.data == 0;
							iter->second.data_flip();
							if (k == 2 && was0) it->amplitude *= -1.0;
						}
					}
					else if (!flag && iter->second.data == 1)
						it->amplitude *= -1.0;
				}
				else
				{
					int k = int(std::floor(8 * c));
					bool do_phase = k == 1 || k == 4 || k == 5 || k == 6 || k == 7;
					int s = (k == 3 || k == 6 || k == 7) ? 2 : 1;
					if (do_phase)
					{
						auto iter = st.find(v);
						if (iter != st.end())
						{
							if (iter->second.addr == qram_simulator::L) it->amplitude *= qutrit_omega(s);
							else if (iter->second.addr == qram_simulator::R) it->amplitude *= qutrit_omega(2 * s);
						}
					}
					if (k == 0 || k == 4 || k == 6) model_addr_cycle(it->state, v, qram_simulator::R);
					else if (k == 2 || k == 5 || k == 7) model_addr_cycle(it->state, v, qram_simulator::L);
				}
				break;
			}
			default:
				throw std::runtime_error("model_noise_op: only Depolarizing is modeled (Damping is M3)");
			}
		}
	}
}

/* Damp_Full 的采样结果（第二次抽样）：outcome ∈ {-1=不跳, 0=L跳/data跳, 1=R跳} */
struct DampOutcome
{
	size_t step;
	size_t pos;
	int outcome;
};

/* 复刻 QRAMCircuit::run_damp_full 的采样（pick_all 路径：first_good_branch==-1，
   multiplier 预处理自动跳过）。逐 RNG draw 对齐：prob_damp 计算无 RNG、
   恰好一次 uniform01。返回采样结果并记入日志。 */
int replica_damp_full(qram_qutrit::QRAMCircuit& q, size_t qubit_id, size_t step,
	std::vector<DampOutcome>* log)
{
	std::array<double, 2> prob_damp = { 0.0, 0.0 };
	auto& branches = q.get_branches();
	auto& probs = q.get_branch_probs();
	for (size_t i = 0; i < branches.size(); ++i)
	{
		auto&& prob = branches[i].get_prob_damp(qubit_id);
		for (size_t k = 0; k < prob_damp.size(); ++k)
			prob_damp[k] += prob[k] * probs[i];
	}
	double global_coef = q.get_normalization_factor_with_damping();
	double r = random_engine::get_instance().uniform01() * global_coef;
	int outcome = -1;
	for (size_t k = 0; k < prob_damp.size(); ++k)
	{
		if (r < prob_damp[k])
		{
			for (auto& branch : branches)
				branch.run_damp_full(qubit_id, k);
			outcome = (int)k;
			break;
		}
		r -= prob_damp[k];
	}
	if (log)
		log->push_back({ step, qubit_id, outcome });
	return outcome;
}

/* 模型语义轨迹：逻辑算子走原生 dispatch（对表示不敏感），噪声算子走 model_noise_op；
 * Damp 算子走复刻拦截（Damp_Common 直通原生、Damp_Full 复刻采样并记日志）。 */
void run_model_trajectory(qram_qutrit::QRAMCircuit& q, bool verbose,
	std::vector<DampOutcome>* damp_log)
{
	int stepno = 0;
	for (const OperationPack& ops : q.get_operations().time_slices)
	{
		++stepno;
		if (verbose)
			fmt::print("[model] step {} ops: {}\n", stepno, ops.to_string());
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
			case OperationType::Damp_Full:
				replica_damp_full(q, op.targets[0], (size_t)stepno, damp_log);
				q.clear_zero_elements();
				break;
			case OperationType::Damp_Common:
				q.run_damp_common(op.coefficients[0]);
				break;
			default:
				if (verbose)
					fmt::print("[model]   noise op type={} targets={} coefs={}\n",
						(int)op.type, vec2str(op.targets), vec2str(op.coefficients));
				model_noise_op(q, op);
				break;
			}
		}
		if (verbose)
			fmt::print("[model] after step {}:\n{}", stepno, q.to_string());
	}
	q.clear_zero_elements();
}

/* 调试用：复刻 run_valid_branches 的 dispatch，逐 pack 打印 branch 状态 */
void run_valid_branches_traced(qram_qutrit::QRAMCircuit& q)
{
	int step = 0;
	for (const OperationPack& ops : q.get_operations().time_slices)
	{
		step++;
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
			case OperationType::Damp_Full: q.run_damp_full(op.targets[0], step, op.coefficients[0]); q.clear_zero_elements(); break;
			case OperationType::Damp_Common: q.run_damp_common(op.coefficients[0]); break;
			default: throw std::runtime_error("Bad type.");
			}
		}
		fmt::print("--- after step {} [{}] ---\n{}", step, ops.to_string(), q.to_string());
	}
	q.clear_zero_elements();
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
	bool trace_steps = false;
	size_t export_runs = 8;

	argparse::ArgumentParser parser("QutritCorrespondenceExporter",
		"Export qutrit QRAM TimeStep schedule (gate-level + sampled noise ops) and reference output distributions");
	parser.add_argument().names({ "-a", "--addrsize" }).description("address size (>=2 for nonempty noise support)").required(false);
	parser.add_argument().names({ "-d", "--datasize" }).description("data size (default 1)").required(false);
	parser.add_argument().name("--memory").description("comma-separated memory cells (size = 2^addrsize)").required(false);
	parser.add_argument().name("--seed").description("random engine seed").required(false);
	parser.add_argument().name("--runs").description("number of trajectories for the averaged reference").required(false);
	parser.add_argument().name("--depolarizing").description("Depolarizing probability per active position per step").required(false);
	parser.add_argument().name("--damping").description("Damping probability (surface kept; channel-level correspondence is M3)").required(false);
	parser.add_argument().name("--exportruns").description("number of leading trajectories whose schedules are exported for per-case exact comparison (default 8)").required(false);
	parser.add_argument().name("--outdir").description("output directory (default: results)").required(false);
	parser.add_argument().name("--input").description("input mode: zerobus (default) or uniform").required(false);
	parser.add_argument().name("--tracesteps").description("print branch states after every time slice of the first run").required(false);
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
	if (parser.exists("outdir")) outdir = parser.get<string>("outdir");
	if (parser.exists("input")) input_mode = parser.get<string>("input");
	trace_steps = parser.exists("tracesteps");
	if (input_mode != "zerobus" && input_mode != "uniform")
	{
		fmt::print("input mode must be zerobus or uniform\n");
		return 1;
	}

	if (addr_size < 1 || addr_size > 4 || data_size < 1 || data_size > 2)
	{
		fmt::print("addr/data size out of supported range for the correspondence experiment\n");
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
		qram_qutrit::QRAMCircuit q0(addr_size, data_size, memory);
		set_input(q0, input_mode);
		q0.run_full();
		noisefree_dist = collect_output_distribution(q0);
	}

	/* 2) 噪声配置：前 export_runs 条轨迹逐条导出调度（schedule_r{i}.json，驱动逐随机
	 *    case 精确对拍），r=0 兼容导出 schedule.json；每条轨迹 run_valid_branches 后：
	 *      - 输出分布（realization dist，Stage 1 逐 case 基准 + 轨迹平均）；
	 *      - 三个无后选择 fidelity 参考量（在 sample_and_get_fidelity 破坏 branch 前计算）：
	 *          fid_nopost   = |Σ_i p_i·F_i|²，F_i = bus 正确的振幅和（不采样树构型）
	 *          fid_overlap  = |Σ_i p_i·G_i|²，G_i = bus 正确且树回基态的振幅和
	 *                         = |⟨ψ_ideal|ψ_traj⟩|²，与驱动 F_quantum 严格同口径
	 *          fid_incoh    = Σ_i p_i|F_i|²，分支投影（非相干）组合
	 *      - 最后才调 sample_and_get_fidelity（逐 shot 树后选择版，avg_fidelity）。 */
	string schedule_str;
	map<string, double> single_run_dist;
	map<string, double> avg_dist;
	map<size_t, map<string, double>> realization_dists;
	double fidelity_sum = 0.0;
	double fid_nopost_sum = 0.0;
	double fid_overlap_sum = 0.0;
	double fid_incoh_sum = 0.0;
	{
		qram_qutrit::QRAMCircuit q(addr_size, data_size, memory);
		q.set_noise_models(noise);
		random_engine::set_seed(seed);   /* 原生轨迹从指定种子起跑（模型循环重放同一序列） */
		set_input(q, input_mode);

		static auto tree_is_ground = [](const qram_qutrit::QRAMState& st)
		{
			for (auto& [id, node] : st.nz_elements)
				if (!node.check_0())
					return false;
			return true;
		};

		for (size_t r = 0; r < runs; ++r)
		{
			q.pick_all();
			q.run_valid_branches();
			auto d = collect_output_distribution(q);
			if (r == 0)
				single_run_dist = d;
			if (r < export_runs)
				realization_dists[r] = d;
			for (auto& [k, v] : d)
				avg_dist[k] += v;

			/* 三个无后选择 fidelity（读 branches，纯计算无 RNG） */
			complex_t coh = 0, ov = 0;
			double incoh = 0;
			auto& branches = q.get_branches();
			auto& probs = q.get_branch_probs();
			for (size_t i = 0; i < branches.size(); ++i)
			{
				auto expect_bus = (branches[i].bus_input ^ memory[branches[i].address]) & bus_mask;
				complex_t Fa = 0, Ga = 0;
				for (auto it = branches[i].iterbeg(); it != branches[i].iterend(); ++it)
				{
					if (it->data_bus == expect_bus)
					{
						Fa += it->amplitude;
						if (tree_is_ground(it->state))
							Ga += it->amplitude;
					}
				}
				coh += probs[i] * Fa;
				ov += probs[i] * Ga;
				incoh += probs[i] * abs_sqr(Fa);
			}
			fid_nopost_sum += abs_sqr(coh);
			fid_overlap_sum += abs_sqr(ov);
			fid_incoh_sum += incoh;

			/* sample_and_get_fidelity 会做采样并删失 subbranch，必须放在最后 */
			fidelity_sum += q.sample_and_get_fidelity();
		}
		for (auto& [k, v] : avg_dist)
			v /= (double)runs;
	}

	/* 3) 复刻轨迹（模型语义 + Damp_Full 第二次抽样拦截）：
	 *    逐 RNG draw 重放与原生完全相同的随机序列；前 export_runs 条轨迹导出调度
	 *    （schedule_r{i}.json，Damp_Full 带 outcome 字段）并与原生逐轨迹分布
	 *    断言一致 —— 不一致说明复刻有误。 */
	map<string, double> model_single_run_dist;
	map<string, double> model_avg_dist;
	double model_fidelity_sum = 0.0;
	{
		qram_qutrit::QRAMCircuit q(addr_size, data_size, memory);
		q.set_noise_models(noise);
		random_engine::set_seed(seed);   /* 重放与原生完全相同的调度序列 */
		set_input(q, input_mode);
		for (size_t r = 0; r < runs; ++r)
		{
			q.pick_all();
			std::vector<DampOutcome> damp_log;
			run_model_trajectory(q, r == 0 && trace_steps, &damp_log);
			auto d = collect_output_distribution(q);
			if (r == 0)
				model_single_run_dist = d;
			for (auto& [k, v] : d)
				model_avg_dist[k] += v;

			if (r < export_runs)
			{
				/* 复刻 vs 原生逐轨迹一致性校验 */
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
			model_fidelity_sum += q.sample_and_get_fidelity();
		}
		for (auto& [k, v] : model_avg_dist)
			v /= (double)runs;
	}

	std::ofstream((std::filesystem::path(outdir) / "schedule.json").string()) << schedule_str;

	std::ostringstream ref;
	ref << "{\n";
	ref << fmt::format("  \"seed\": {},\n  \"runs\": {},\n", seed, runs);
	ref << fmt::format("  \"noise\": {{");
	bool first = true;
	for (auto& [t, p] : noise)
	{
		if (!first) ref << ",";
		first = false;
		ref << fmt::format("\"{}\":{:.17g}", noise_type_name(t), p);
	}
	ref << fmt::format("}},\n");
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
	ref << fmt::format("  \"model_avg_fidelity\": {:.17g}\n", model_fidelity_sum / (double)runs);
	ref << "}\n";
	std::ofstream((std::filesystem::path(outdir) / "reference.json").string()) << ref.str();

	fmt::print("exported schedule.json + reference.json to {}\n", outdir);
	fmt::print("  noisefree  dist: {}\n", dist2str(noisefree_dist));
	fmt::print("  single run dist: {}\n", dist2str(single_run_dist));
	fmt::print("  avg fidelity   : {:.6f}\n", fidelity_sum / (double)runs);
	return 0;
}
