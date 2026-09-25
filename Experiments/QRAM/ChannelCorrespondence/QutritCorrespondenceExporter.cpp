/*
 * QutritCorrespondenceExporter
 *
 * C++ export side of the correspondence experiment between qutrit QRAM (QRAM-Simulator,
 * TimeStep trajectory-level noise model) and circuit-level channel simulation
 * (UnifiedQuantum/uniqc, OriginIR-ext inline noise channels + density-matrix backend):
 *   1. Build qram_qutrit::QRAMCircuit; set memory / noise_t / seed / uniform input;
 *   2. Extract from QRAMCircuit::operations (TimeStep::generate output):
 *        - per-step logical operators → gate sequence after qutrit→qubit encoding (with control polarities);
 *        - sampled noise operators {step, type, pos, coef} (the position is the "noise channel position");
 *   3. run_full() exact trajectory evolution → reference output distributions (single run + multi-trajectory average + built-in fidelity);
 *   4. Write everything to schedule.json / reference.json for the Python-side (uniqc) driver to consume.
 *
 * Encoding conventions (strictly consistent with run_correspondence.py):
 *   - address register: addr_size qubits, address bit b ↔ qubit b;
 *   - bus register: data_size qubits, bus digit d ↔ qubit addr_size + d;
 *   - routing node v (heap-indexed, v ∈ [0, 2^addr-1)):
 *       a1 = addr+data+3v, a0 = addr+data+3v+1 (qutrit level encoding), d = addr+data+3v+2;
 *       W=(a1,a0)=|00> (ground state), L=|01>, R=|10>, |11> is an unreachable dead state;
 *   - noise position pos: node = pos/2, subsystem = pos%2 (0 → (a1,a0) of the addr-qutrit, 1 → the data qubit).
 *
 * Gate IR: {"op": "X"|"SWAP"|"CNOT"|"U1", "q": [...], "ctrl": [[qubit, expected value], ...], "theta": ...}
 *   CNOT takes q=[control, target]; U1 = diag(1, e^{i·theta}); empty ctrl list = no controls.
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

/* Emit the basis transposition (pi ↔ pj) on qubit list qs (bit i ↔ qs[i]), controlled overall by cs.
 * Standard conjugation: X wrapping + CNOT conjugation reduces the permutation to multi-controlled X on single-bit flips. */
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

/* internal_swap of node v: (0,0,0)↔(0,1,0) and (0,0,1)↔(1,0,0) on (a1,a0,d).
 * Bit order (a1,a0,d) = (bit0,bit1,bit2). */
void emit_internal_swap(vector<string>& out, const Encoding& enc, size_t v, const vector<Ctrl>& cs)
{
	vector<size_t> qs = { enc.node_a1(v), enc.node_a0(v), enc.node_d(v) };
	emit_transposition(out, qs, 0b000, 0b010, cs);
	emit_transposition(out, qs, 0b100, 0b001, cs);
}

/* Logical operators → gate sequence. Mirrors the dispatch semantics of QRAMCircuit::run_valid_branches. */
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
		/* SWAP(bus[digit], node0.data); the accompanying try_merge is a no-op */
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
			/* each node of layer ℓ-1 applies internal_swap to its child in the addr direction */
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

/* Set input branches: uniform = full uniform superposition of address+bus (the output distribution degenerates to uniform; control only);
 * zerobus = uniform superposition of addresses with bus=0 (main configuration: outputs (a, memory[a]) are nontrivial and the distribution is checkable) */
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

/* schedule.json: config + encoding + input + per-step (gates + noise operators, preserving in-pack order).
 * damp_outcomes: results of Damp_Full's second draw (key = (step, pos); -2 means not recorded). */
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
	/* get_fidelity's expect_bus XORs the raw memory value directly, so only values within
	 * data_size bits are legal inputs (same convention as set_memory_random) — out-of-range values silently count as 0 contribution */
	for (auto v : memory)
		if (v >= pow2(data_size))
			throw std::runtime_error("memory entry exceeds data_size bits");
	return memory;
}

/* Model-level (comment semantics) noise operators: first normalize explicit (W,0) to absent, then apply as pure state functions.
 * After the library-side fixes to rotate_A1 passthrough / state_of(absent) / the closed sampling interval, this trajectory should agree
 * bitwise with native run_valid_branches — kept as a cross-validation of those fixes. */
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
	/* ground_target: A1 → R (W→R→L→W), A1² → L (W→L→R→W) */
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

/* Sampled result of Damp_Full (second draw): outcome ∈ {-1=no jump, 0=L jump/data jump, 1=R jump} */
struct DampOutcome
{
	size_t step;
	size_t pos;
	int outcome;
};

/* Replicates QRAMCircuit::run_damp_full's sampling (pick_all path: first_good_branch==-1,
   multiplier preprocessing auto-skipped). Aligned per RNG draw: prob_damp computation uses no RNG,
   exactly one uniform01. Returns the sampled result and logs it. */
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

/* Model-semantics trajectory: logical operators go through the native dispatch (representation-insensitive), noise operators through model_noise_op;
 * Damp operators go through the replica interception (Damp_Common passes through native, Damp_Full replicates the sampling and logs it). */
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

/* Debug aid: replicates run_valid_branches's dispatch, printing branch state per pack */
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

	/* 1) Noise-free reference (Stage 0 baseline) */
	map<string, double> noisefree_dist;
	{
		qram_qutrit::QRAMCircuit q0(addr_size, data_size, memory);
		set_input(q0, input_mode);
		q0.run_full();
		noisefree_dist = collect_output_distribution(q0);
	}

	/* 2) Noisy configuration: the first export_runs trajectories each export their schedule (schedule_r{i}.json, for the
	 *    driver's exact per-random-case cross-check), r=0 additionally exported as schedule.json; after run_valid_branches per trajectory:
	 *      - output distribution (realization dist, Stage 1 per-case ground-truth reference + trajectory average);
	 *      - three post-selection-free reference fidelity quantities (computed before sample_and_get_fidelity clobbers the branches):
	 *          fid_nopost   = |Σ_i p_i·F_i|², F_i = amplitude sum with correct bus (no tree-configuration sampling)
	 *          fid_overlap  = |Σ_i p_i·G_i|², G_i = amplitude sum with correct bus and tree back to ground
	 *                         = |⟨ψ_ideal|ψ_traj⟩|², strictly the same convention as the driver's F_quantum
	 *          fid_incoh    = Σ_i p_i|F_i|², incoherent combination of branch projections
	 *      - only then call sample_and_get_fidelity (per-shot tree-post-selected version, avg_fidelity). */
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
		random_engine::set_seed(seed);   /* native trajectories start from the given seed (the model loop replays the same sequence) */
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

			/* The three post-selection-free fidelities (read the branches; pure computation, no RNG) */
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

			/* sample_and_get_fidelity samples and truncates subbranches, so it must come last */
			fidelity_sum += q.sample_and_get_fidelity();
		}
		for (auto& [k, v] : avg_dist)
			v /= (double)runs;
	}

	/* 3) Replica trajectories (model semantics + Damp_Full second-draw interception):
	 *    replay exactly the same random sequence as native, RNG draw by draw; the first export_runs trajectories export
	 *    their schedules (schedule_r{i}.json, Damp_Full carries the outcome field) and are asserted
	 *    consistent with the native per-trajectory distributions — a mismatch means the replica is wrong. */
	map<string, double> model_single_run_dist;
	map<string, double> model_avg_dist;
	double model_fidelity_sum = 0.0;
	{
		qram_qutrit::QRAMCircuit q(addr_size, data_size, memory);
		q.set_noise_models(noise);
		random_engine::set_seed(seed);   /* replay exactly the same schedule sequence as native */
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
				/* Replica vs native per-trajectory consistency check */
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
