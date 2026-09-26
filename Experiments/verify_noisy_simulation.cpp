/* Independent encoded-circuit verification of QRAM trajectories.
 * Qubit: standard whole-tree amplitude damping, with one normalized joint
 * Kraus outcome per layer. Check noiseless gates, fixed-outcome replay, the
 * full averaged density matrix, trace, output TVD and ideal-state fidelity.
 * The schedule is shared with the engine; the matrix state/channel backend
 * is independent. Qutrit "faithful" tests retain the historical sampler's
 * behavior only and do not certify a physical amplitude-damping channel.
 * See docs/sphinx/source/en/paper/joint_damping.md for scope and derivation.
 */

#include <cmath>
#include <complex>
#include <cstdlib>
#include <cstdint>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "basic.h"
#include "qram_circuit_qubit.h"
#include "qram_circuit_qutrit.h"
#include "time_step.h"

using namespace std;
using namespace qram_simulator;

namespace {

using cplx = complex<double>;
using Matrix = vector<cplx>;  // row-major d×d
using Ctrl = pair<size_t, int>;  // (qubit, expected value)

constexpr double PI_ = 3.14159265358979323846;
constexpr double SQRT2_INV = 0.70710678118654752440;

/* ==========================================================================
 * 1. Self-contained circuit simulation engine (qubit q ↔ index bit q; local-matrix bit0 ↔ qs[0])
 * ========================================================================== */

void apply_local_state(vector<cplx>& psi, const vector<size_t>& qs, const Matrix& M,
	const vector<Ctrl>& ctrls)
{
	size_t d = qs.size(), dim = 1ull << d;
	vector<cplx> out(psi.size(), { 0.0, 0.0 });
	for (size_t i = 0; i < psi.size(); ++i)
	{
		bool ok = true;
		for (auto& [q, v] : ctrls)
			if (((i >> q) & 1) != (unsigned)v) { ok = false; break; }
		if (!ok)
		{
			out[i] += psi[i];
			continue;
		}
		size_t loc = 0;
		for (size_t k = 0; k < d; ++k)
			loc |= ((i >> qs[k]) & 1) << k;
		for (size_t lo = 0; lo < dim; ++lo)
		{
			cplx a = M[lo * dim + loc];
			if (a == cplx(0.0, 0.0)) continue;
			size_t j = i;
			for (size_t k = 0; k < d; ++k)
				j = (j & ~(1ull << qs[k])) | (((lo >> k) & 1) << qs[k]);
			out[j] += a * psi[i];
		}
	}
	psi = std::move(out);
}

using RhoMap = unordered_map<uint64_t, cplx>;  // key = (i << n) | j

/* ρ ← Σ_k K_k ρ K_k† (local Kraus family; identity where the control is unsatisfied — judged independently per row/column) */
void apply_kraus_rho(RhoMap& rho, size_t n, const vector<size_t>& qs,
	const vector<Matrix>& Ks, const vector<Ctrl>& ctrls = {})
{
	size_t d = qs.size(), dim = 1ull << d;
	auto local_of = [&](size_t idx)
	{
		size_t loc = 0;
		for (size_t k = 0; k < d; ++k)
			loc |= ((idx >> qs[k]) & 1) << k;
		return loc;
	};
	auto with_local = [&](size_t idx, size_t lo)
	{
		for (size_t k = 0; k < d; ++k)
			idx = (idx & ~(1ull << qs[k])) | (((lo >> k) & 1) << qs[k]);
		return idx;
	};
	/* A = P·K·P + (I−P)⊗I: identity (amplitude 1) on the side where the control is
	 * unsatisfied, K on the satisfied side. For a channel (no control set) both sides always hold; multiple Kraus accumulate independently per Σ_k K_k ρ K_k† (no mixing across k). */
	RhoMap out;
	out.reserve(rho.size() * 2);
	for (auto& [key, v] : rho)
	{
		size_t i = key >> n, j = key & ((1ull << n) - 1);
		bool ok_i = true, ok_j = true;
		for (auto& [q, val] : ctrls)
		{
			if (((i >> q) & 1) != (unsigned)val) ok_i = false;
			if (((j >> q) & 1) != (unsigned)val) ok_j = false;
		}
		if (!ok_i && !ok_j)
		{
			out[key] += v;
			continue;
		}
		size_t li = local_of(i), lj = local_of(j);
		for (auto& K : Ks)
		{
			for (size_t loi = 0; loi < dim; ++loi)
			{
				size_t ip = i;
				cplx ai = 1.0;
				if (ok_i)
				{
					if (K[loi * dim + li] == cplx(0.0, 0.0)) continue;
					ip = with_local(i, loi);
					ai = K[loi * dim + li];
				}
				else if (loi != li) continue;
				for (size_t loj = 0; loj < dim; ++loj)
				{
					size_t jp = j;
					cplx aj = 1.0;
					if (ok_j)
					{
						if (K[loj * dim + lj] == cplx(0.0, 0.0)) continue;
						jp = with_local(j, loj);
						aj = K[loj * dim + lj];
					}
					else if (loj != lj) continue;
					out[(ip << n) | jp] += v * ai * conj(aj);
				}
			}
		}
	}
	rho = std::move(out);
}

double rho_trace(const RhoMap& rho, size_t n)
{
	double s = 0;
	for (auto& [key, v] : rho)
		if ((key >> n) == (key & ((1ull << n) - 1)))
			s += v.real();
	return s;
}

/* Sum of diagonal weights satisfying mask (list of (qubit, value)) */
double rho_mask_weight(const RhoMap& rho, size_t n, const vector<Ctrl>& mask)
{
	double s = 0;
	for (auto& [key, v] : rho)
	{
		size_t i = key >> n;
		if (i != (key & ((1ull << n) - 1))) continue;
		bool ok = true;
		for (auto& [q, val] : mask)
			if (((i >> q) & 1) != (unsigned)val) { ok = false; break; }
		if (ok) s += v.real();
	}
	return s;
}

/* ==========================================================================
 * 2. Matrix library
 * ========================================================================== */

Matrix mat_id(size_t d)
{
	Matrix m(d * d);
	for (size_t i = 0; i < d; ++i) m[i * d + i] = 1.0;
	return m;
}

Matrix mat_x() { return { 0.0, 1.0, 1.0, 0.0 }; }
Matrix mat_z() { return { 1.0, 0.0, 0.0, -1.0 }; }
Matrix mat_h() { return { SQRT2_INV, SQRT2_INV, SQRT2_INV, -SQRT2_INV }; }

/* qutrit data-qubit depolarizing k=2 (bitphaseflip): |0>→−|1>, |1>→|0> */
Matrix mat_xz() { return { 0.0, 1.0, -1.0, 0.0 }; }

Matrix mat_swap2()  // 2q SWAP, qs={x,y} (bit0=x)
{
	Matrix m(16);
	m[0] = 1.0; m[4 * 2 + 1] = 1.0; m[4 * 1 + 2] = 1.0; m[4 * 3 + 3] = 1.0;
	return m;
}

Matrix scaled(Matrix m, cplx s)
{
	for (auto& v : m) v *= s;
	return m;
}

/* qutrit 4×4 (basis (a1,a0): W=0, L=1, R=2, dead=3; qs={a1,a0}) */

Matrix qutrit_perm(bool a1_first)  /* true: A1 (W→R→L→W); false: A1² (W→L→R→W) */
{
	Matrix m(16);
	if (a1_first)
	{
		m[4 * 2 + 0] = 1.0;
		m[4 * 1 + 2] = 1.0;
		m[4 * 0 + 1] = 1.0;
	}
	else
	{
		m[4 * 1 + 0] = 1.0;
		m[4 * 2 + 1] = 1.0;
		m[4 * 0 + 2] = 1.0;
	}
	m[4 * 3 + 3] = 1.0;
	return m;
}

Matrix qutrit_phase(int s)
{
	Matrix m = mat_id(4);
	m[4 * 1 + 1] = polar(1.0, 2.0 * PI_ * s / 3.0);
	m[4 * 2 + 2] = polar(1.0, 2.0 * PI_ * 2 * s / 3.0);
	return m;
}

Matrix qutrit_weyl(int k)  /* 8-element Weyl: phase attached to the pre-permutation level (mirrors run_depolarizing order) */
{
	bool do_phase = k == 1 || k == 4 || k == 5 || k == 6 || k == 7;
	int s = (k == 3 || k == 6 || k == 7) ? 2 : 1;
	Matrix perm = (k == 0 || k == 4 || k == 6) ? qutrit_perm(true)
		: ((k == 2 || k == 5 || k == 7) ? qutrit_perm(false) : mat_id(4));
	Matrix ph = do_phase ? qutrit_phase(s) : mat_id(4);
	Matrix out(16);
	for (size_t col = 0; col < 4; ++col)
		for (size_t row = 0; row < 4; ++row)
			if (abs(perm[row * 4 + col]) != 0.0)
				out[row * 4 + col] = ph[col * 4 + col];
	return out;
}

/* qutrit internal swap: 3q local qs={a1,a0,d} (bit0=a1): (W,0)↔(L,0), (W,1)↔(R,0) */
Matrix qutrit_internal_swap()
{
	Matrix m(64);
	auto setp = [&](size_t row, size_t col) { m[row * 8 + col] = 1.0; };
	setp(0b010, 0b000); setp(0b000, 0b010);
	setp(0b001, 0b100); setp(0b100, 0b001);
	setp(0b110, 0b110); setp(0b101, 0b101);
	setp(0b011, 0b011); setp(0b111, 0b111);
	return m;
}

Matrix qutrit_k0(double gamma)
{
	double s = sqrt(1.0 - gamma);
	return { 1.0, 0.0, 0.0, 0.0,
			 0.0, s, 0.0, 0.0,
			 0.0, 0.0, s, 0.0,
			 0.0, 0.0, 0.0, 1.0 };
}

Matrix qubit_k0(double gamma)
{
	return { 1.0, 0.0, 0.0, sqrt(1.0 - gamma) };
}

Matrix jump_2q()  /* |0><1| */
{
	return { 0.0, 1.0, 0.0, 0.0 };
}

Matrix qutrit_jump(int level)  /* |W><L| (0) or |W><R| (1) */
{
	Matrix m(16);
	m[level == 0 ? 1 : 2] = 1.0;
	return m;
}

/* qubit-architecture bitphaseflip (library already fixed to Y flip): |0>→−|1>, |1>→|0> (ZX = iY, unitary) */
Matrix qubit_M() { return mat_xz(); }

/* ==========================================================================
 * 3. Encoding and schedule → local operator sequence (mirrors the logical-operator part of both architectures' dispatch)
 * ========================================================================== */

struct Encoding
{
	size_t addr_size = 2, data_size = 1;
	bool qutrit = true;
	size_t per_node() const { return qutrit ? 3 : 2; }
	size_t num_qubits() const { return addr_size + data_size + per_node() * (pow2(addr_size) - 1); }
	size_t bus_q(size_t d) const { return addr_size + d; }
	size_t node_base(size_t v) const { return addr_size + data_size + per_node() * v; }
	size_t node_dq(size_t v) const { return node_base(v) + (qutrit ? 2 : 1); }
};

struct LocalOp
{
	vector<size_t> qs;
	Matrix M;
	vector<Ctrl> ctrls;
};

struct NoiseEntry
{
	OperationType type;
	size_t pos;
	double coef;
	int outcome;
	size_t step;
};

struct StepPlan
{
	vector<LocalOp> gates;
	vector<NoiseEntry> noises;
	size_t entangle_max = 0;
};

vector<StepPlan> translate_schedule(const TimeSlices& slices, const Encoding& enc,
	const memory_t& memory, const map<pair<size_t, size_t>, int>* outcomes = nullptr)
{
	vector<StepPlan> plan(slices.time_slices.size());
	for (size_t s = 0; s < slices.time_slices.size(); ++s)
	{
		size_t step = s + 1;
		for (auto& op : slices.time_slices[s].operations)
		{
			auto emit = [&](vector<size_t> qs, Matrix M, vector<Ctrl> ctrls)
			{
				plan[s].gates.push_back({ std::move(qs), std::move(M), std::move(ctrls) });
			};
			switch (op.type)
			{
			case OperationType::FirstCopy:
			{
				size_t bit = enc.addr_size - 1 - op.targets[0];
				emit({ enc.node_dq(0) }, mat_x(), { { bit, 1 } });
				break;
			}
			case OperationType::CopyIn:
			case OperationType::CopyOut:
			{
				bool is_in = op.type == OperationType::CopyIn;
				bool do_h = is_in ? (op.targets[0] == 0)
					: (op.targets[0] == enc.data_size - 1);
				if (do_h && is_in && !enc.qutrit)
					for (size_t dd = 0; dd < enc.data_size; ++dd)
						emit({ enc.bus_q(dd) }, mat_h(), {});
				emit({ enc.node_dq(0), enc.bus_q(op.targets[0]) }, mat_swap2(), {});
				if (do_h && !is_in && !enc.qutrit)
					for (size_t dd = 0; dd < enc.data_size; ++dd)
						emit({ enc.bus_q(dd) }, mat_h(), {});
				break;
			}
			case OperationType::SwapInternal:
			{
				size_t layer = op.targets[0];
				size_t lower = pow2(layer) - 1, upper = pow2(layer + 1) - 2;
				for (size_t v = lower; v <= upper; ++v)
				{
					size_t base = enc.node_base(v);
					if (enc.qutrit)
					{
						if (layer == 0)
						{
							emit({ base, base + 1, base + 2 }, qutrit_internal_swap(), {});
						}
						else
						{
							/* QRAM applies internal_swap only to the child selected by the
							 * parent node's routing (L→left child, R→right child); the unselected ground (W,0) stays put */
							size_t pb = enc.node_base((v - 1) / 2);
							bool is_left = (v % 2 == 1);
							emit({ base, base + 1, base + 2 }, qutrit_internal_swap(),
								is_left ? vector<Ctrl>{ { pb, 0 }, { pb + 1, 1 } }
										: vector<Ctrl>{ { pb, 1 }, { pb + 1, 0 } });
						}
					}
					else
					{
						emit({ base + 1, base }, mat_swap2(), {});
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
					size_t base = enc.node_base(v);
					size_t da = enc.node_dq(v);
					size_t dl = enc.node_dq(2 * v + 1), dr = enc.node_dq(2 * v + 2);
					if (enc.qutrit)
					{
						emit({ da, dl }, mat_swap2(), { { base, 0 }, { base + 1, 1 } });
						emit({ da, dr }, mat_swap2(), { { base, 1 }, { base + 1, 0 } });
					}
					else
					{
						emit({ da, dl }, mat_swap2(), { { base, 0 } });
						emit({ da, dr }, mat_swap2(), { { base, 1 } });
					}
				}
				break;
			}
			case OperationType::FetchData:
			{
				size_t digit = op.targets[0];
				size_t lower = pow2(enc.addr_size - 1) - 1, upper = pow2(enc.addr_size) - 2;
				for (size_t v = lower; v <= upper; ++v)
				{
					size_t off = v - lower, base = enc.node_base(v);
					if (enc.qutrit)
					{
						if (get_digit(memory[2 * off], digit))
							emit({ enc.node_dq(v) }, mat_x(), { { base, 0 }, { base + 1, 1 } });
						if (get_digit(memory[2 * off + 1], digit))
							emit({ enc.node_dq(v) }, mat_x(), { { base, 1 }, { base + 1, 0 } });
					}
					else
					{
						if (get_digit(memory[2 * off], digit))
							emit({ enc.node_dq(v) }, mat_z(), { { base, 0 } });
						if (get_digit(memory[2 * off + 1], digit))
							emit({ enc.node_dq(v) }, mat_z(), { { base, 1 } });
					}
				}
				break;
			}
			default:
			{
				double coef = op.coefficients.empty() ? 0.0 : op.coefficients[0];
				size_t pos = op.targets.empty() ? (size_t)-1 : op.targets[0];
				int outcome = -2;
				if (outcomes)
				{
					auto it = outcomes->find({ step, pos });
					if (it != outcomes->end()) outcome = it->second;
				}
				plan[s].noises.push_back({ op.type, pos, coef, outcome, step });
				break;
			}
			}
		}
			plan[s].entangle_max = 0;  // overwritten by the builder
		(void)step;
	}
	return plan;
}

/* ==========================================================================
 * 4. Circuit-level primitives of noise operators
 * ========================================================================== */

/* S1 replay: noise operators → local matrix sequence (incl. single Kraus, sub-normalized) */
vector<pair<vector<size_t>, Matrix>> replay_noise(const NoiseEntry& o, const Encoding& enc)
{
	vector<pair<vector<size_t>, Matrix>> out;
	size_t v = o.pos / 2, base = enc.node_base(v);
	bool sub_data = o.pos % 2 == 1;
	if (o.type == OperationType::Depolarizing)
	{
		if (enc.qutrit && !sub_data)
		{
			out.push_back({ { base + 1, base }, qutrit_weyl(int(floor(8 * o.coef))) });
		}
		else
		{
			int k = int(floor(3 * o.coef));
			Matrix m = k == 0 ? mat_x() : (k == 1 ? mat_z()
				: (enc.qutrit ? mat_xz() : qubit_M()));
			out.push_back({ { sub_data ? enc.node_dq(v) : base }, m });
		}
	}
	else if (o.type == OperationType::Damp_Full)
	{
		if (o.outcome < 0) return out;
		if (enc.qutrit && !sub_data)
			out.push_back({ { base + 1, base }, qutrit_jump(o.outcome) });
		else
			out.push_back({ { sub_data ? enc.node_dq(v) : base }, jump_2q() });
	}
	else if (o.type == OperationType::Damp_Common)
	{
		double s = sqrt(1.0 - o.coef);
		Matrix k0q = qutrit_k0(o.coef);
		Matrix k0d = qubit_k0(o.coef);
		for (size_t vv = 0; vv < pow2(enc.addr_size) - 1; ++vv)
		{
			size_t b = enc.node_base(vv);
			if (enc.qutrit)
			{
				out.push_back({ { b, b + 1 }, k0q });
				out.push_back({ { b + 2 }, k0d });
			}
			else
			{
				out.push_back({ { b }, k0d });
				out.push_back({ { b + 1 }, k0d });
			}
		}
		(void)s;
	}
	return out;
}

/* ==========================================================================
 * 5. Cross-check utilities and reference side
 * ========================================================================== */

int g_failures = 0;

void check(bool ok, const string& name, const string& detail)
{
	fmt::print("  [{}] {} ({})\n", ok ? "PASS" : "FAIL", name, detail);
	if (!ok) ++g_failures;
}

map<string, double> dist_of_psi(const vector<cplx>& psi, const Encoding& enc)
{
	map<string, double> dist;
	for (size_t i = 0; i < psi.size(); ++i)
	{
		double p = norm(psi[i]);
		if (p == 0.0) continue;
		size_t a = 0, b = 0;
		for (size_t k = 0; k < enc.addr_size; ++k) a |= ((i >> k) & 1) << k;
		for (size_t k = 0; k < enc.data_size; ++k) b |= ((i >> enc.bus_q(k)) & 1) << k;
		dist[fmt::format("{}:{}", a, b)] += p;
	}
	return dist;
}

map<string, double> dist_of_rho(const RhoMap& rho, size_t n, const Encoding& enc)
{
	map<string, double> dist;
	for (auto& [key, v] : rho)
	{
		size_t i = key >> n;
		if (i != (key & ((1ull << n) - 1))) continue;
		size_t a = 0, b = 0;
		for (size_t k = 0; k < enc.addr_size; ++k) a |= ((i >> k) & 1) << k;
		for (size_t k = 0; k < enc.data_size; ++k) b |= ((i >> enc.bus_q(k)) & 1) << k;
		dist[fmt::format("{}:{}", a, b)] += v.real();
	}
	return dist;
}

double classical_fidelity(const map<string, double>& p, const map<string, double>& q)
{
	double s = 0;
	for (auto& [k, v] : p)
	{
		auto it = q.find(k);
		s += sqrt(v * (it == q.end() ? 0.0 : it->second));
	}
	return s * s;
}

double tvd(const map<string, double>& p, const map<string, double>& q)
{
	double s = 0;
	for (auto& [k, v] : p) s += fabs(v - (q.count(k) ? q.at(k) : 0.0));
	for (auto& [k, v] : q) if (!p.count(k)) s += v;
	return 0.5 * s;
}

double max_diff(const map<string, double>& p, const map<string, double>& q)
{
	double m = 0;
	for (auto& [k, v] : p) m = max(m, fabs(v - (q.count(k) ? q.at(k) : 0.0)));
	for (auto& [k, v] : q) if (!p.count(k)) m = max(m, v);
	return m;
}

vector<cplx> initial_state(const Encoding& enc)
{
	vector<cplx> psi(1ull << enc.num_qubits());
	size_t naddr = pow2(enc.addr_size);
	for (size_t a = 0; a < naddr; ++a)
	{
		size_t idx = 0;
		for (size_t k = 0; k < enc.addr_size; ++k) idx |= ((a >> k) & 1) << k;
		psi[idx] = 1.0 / sqrt((double)naddr);
	}
	return psi;
}

vector<cplx> replay_statevector(const vector<StepPlan>& plan, const Encoding& enc)
{
	vector<cplx> psi = initial_state(enc);
	size_t dbg_step = 0;
	for (auto& st : plan)
	{
		++dbg_step;
		for (auto& g : st.gates)
			apply_local_state(psi, g.qs, g.M, g.ctrls);
		for (auto& o : st.noises)
			for (auto& [qs, M] : replay_noise(o, enc))
				apply_local_state(psi, qs, M, {});
        if (!enc.qutrit && std::any_of(st.noises.begin(), st.noises.end(),
                [](const NoiseEntry& o) { return o.type == OperationType::Damp_Common; })) {
            double norm2 = 0;
            for (auto a : psi) norm2 += norm(a);
            if (!(norm2 > 0)) throw std::runtime_error("zero norm in circuit replay");
            for (auto& a : psi) a /= sqrt(norm2);
        }
		if (getenv("VNS_TRACE"))
		{
			double tot = 0;
			for (auto& a : psi) tot += norm(a);
			auto w = [&](size_t q)
			{
				double s = 0;
				for (size_t i = 0; i < psi.size(); ++i)
					if ((i >> q) & 1) s += norm(psi[i]);
				return s;
			};
			double exc = 0;
			string pernode;
			for (size_t v = 0; v < pow2(enc.addr_size) - 1; ++v)
			{
				size_t b = enc.node_base(v);
				if (enc.qutrit)
				{
					exc += w(b) + w(b + 1) + w(b + 2);
					pernode += fmt::format(" n{}({:.2f},{:.2f},{:.2f})", v, w(b), w(b + 1), w(b + 2));
				}
				else
				{
					exc += w(b) + w(b + 1);
					pernode += fmt::format(" n{}(a={:.2f},d={:.2f})", v, w(b), w(b + 1));
				}
			}
			fmt::print("    step {:2d} em={} norm={:.4f} exc={:.3f} addrw=({:.2f},{:.2f}) busw={:.2f}{}\n",
				dbg_step, st.entangle_max, tot, exc,
				w(0), w(1), w(enc.bus_q(0)), pernode);
		}
	}
	return psi;
}

double overlap_fid(const vector<cplx>& psi_ideal, const RhoMap& rho, size_t n)
{
	cplx s = { 0.0, 0.0 };
	for (auto& [key, v] : rho)
		s += conj(psi_ideal[key >> n]) * v * psi_ideal[key & ((1ull << n) - 1)];
	return s.real();
}

/* ---------- qutrit reference side ---------- */

void set_input_qutrit(qram_qutrit::QRAMCircuit& q)
{
	q.branches.clear();
	q.branch_probs.clear();
	size_t naddr = pow2(q.address_size);
	for (size_t a = 0; a < naddr; ++a)
	{
		q.branches.emplace_back(a, q.data_size, 0);
		q.branch_probs.push_back(1.0 / (double)naddr);
	}
}

map<string, double> dist_qutrit(const qram_qutrit::QRAMCircuit& q)
{
	map<string, double> dist;
	auto& branches = q.get_branches();
	auto& probs = q.get_branch_probs();
	for (size_t i = 0; i < branches.size(); ++i)
		for (auto it = branches[i].iterbeg(); it != branches[i].iterend(); ++it)
			dist[fmt::format("{}:{}", branches[i].address, it->data_bus)]
				+= probs[i] * abs_sqr(it->amplitude);
	return dist;
}

/* qutrit replica execution: intercepts Damp_Full sampling (aligned per RNG draw, exactly one uniform01) */
void replica_run_qutrit(qram_qutrit::QRAMCircuit& q, int step0,
	vector<NoiseEntry>* noise_log)
{
	int step = step0;
	for (auto& ops : q.get_operations().time_slices)
	{
		++step;
		for (auto& op : ops.operations)
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
			{
				size_t qid = op.targets[0];
				double prob0 = 0, prob1 = 0;
				auto& branches = q.get_branches();
				auto& probs = q.get_branch_probs();
				for (size_t i = 0; i < branches.size(); ++i)
				{
					auto pr = branches[i].get_prob_damp(qid);
					prob0 += pr[0] * probs[i];
					prob1 += pr[1] * probs[i];
				}
				double global = q.get_normalization_factor_with_damping();
				double r = random_engine::get_instance().uniform01() * global;
				int outcome = -1;
				if (r < prob0) { outcome = 0; }
				else { r -= prob0; if (r < prob1) outcome = 1; }
				if (outcome >= 0)
					for (auto& br : branches)
						br.run_damp_full(qid, outcome);
				if (noise_log)
					noise_log->push_back({ OperationType::Damp_Full, qid,
						op.coefficients[0], outcome, (size_t)step });
				q.clear_zero_elements();
				break;
			}
			case OperationType::Damp_Common: q.run_damp_common(op.coefficients[0]); break;
			default: throw std::runtime_error("replica qutrit: bad op");
			}
		}
	}
	q.clear_zero_elements();
}

/* qutrit post-selection-free overlap fidelity (tree-sensitive): |Σ p_a G_a|² */
double overlap_fid_qutrit(const qram_qutrit::QRAMCircuit& q, const memory_t& memory)
{
	auto& branches = q.get_branches();
	auto& probs = q.get_branch_probs();
	bus_t mask = pow2(q.data_size) - 1;
	cplx s = 0;
	for (size_t i = 0; i < branches.size(); ++i)
	{
		auto expect = (branches[i].bus_input ^ memory[branches[i].address]) & mask;
		for (auto it = branches[i].iterbeg(); it != branches[i].iterend(); ++it)
		{
			if (it->data_bus != expect) continue;
			bool ground = true;
			for (auto& [id, node] : it->state.nz_elements)
				if (!node.check_0()) { ground = false; break; }
			if (ground) s += probs[i] * it->amplitude;
		}
	}
	return abs_sqr(s);
}

/* ---------- qubit reference side ---------- */

void set_input_qubit(qram_qubit::QRAMCircuit& q)
{
	q.branch_groups.clear();
	size_t naddr = pow2(q.addr_size);
	for (size_t a = 0; a < naddr; ++a)
	{
		q.branch_groups.emplace_back(a);
		auto& g = q.branch_groups.back();
		g.branches_input.emplace_back(a, q.data_size, 0);
		g.branch_probs.push_back(1.0 / (double)naddr);
		g.state_probs.push_back(1.0 / (double)naddr);
	}
}

map<string, double> dist_qubit(const qram_qubit::QRAMCircuit& q)
{
	map<string, double> dist;
	for (auto& g : q.get_branch_groups())
		for (size_t b = 0; b < g.branches.size(); ++b)
			for (auto it = g.branches[b].iterbeg(); it != g.branches[b].iterend(); ++it)
				dist[fmt::format("{}:{}", g.address, it->data_bus)]
					+= g.branch_probs[b] * abs_sqr(it->amplitude);
	return dist;
}

// Replay consumes the production layer's recorded environment outcomes.
// The matrix backend below independently applies those operators; it does not
// reproduce the auxiliary sampler, so this is a fixed-history bridge test.
void replica_run_qubit(qram_qubit::QRAMCircuit& q, int step0,
    vector<NoiseEntry>* noise_log)
{
    q.run_bad();
    if (!noise_log) return;
    for (const auto& layer : q.damping_history)
        for (size_t pos : layer.candidates)
            noise_log->push_back({OperationType::Damp_Full, pos,
                q.noise_parameters.count(OperationType::Damping) ? q.noise_parameters.at(OperationType::Damping) : 0,
                std::binary_search(layer.jumps.begin(), layer.jumps.end(), pos) ? 0 : -1,
                layer.step + static_cast<size_t>(step0)});
}

double overlap_fid_qubit(const qram_qubit::QRAMCircuit& q, const memory_t& memory)
{
	bus_t mask = pow2(q.data_size) - 1;
	cplx s = 0;
	for (auto& g : q.get_branch_groups())
		for (size_t b = 0; b < g.branches.size(); ++b)
		{
			auto& br = g.branches[b];
			auto expect = (br.bus_input ^ memory[g.address]) & mask;
			for (auto it = br.iterbeg(); it != br.iterend(); ++it)
				if (it->data_bus == expect && it->state.nz_elements.empty())
					s += g.branch_probs[b] * it->amplitude;
		}
	return abs_sqr(s);
}

/* ==========================================================================
 * 6. Experiment driver
 * ========================================================================== */

struct Experiment
{
	string name;
	bool qutrit = true;
	double depol = 0.0;
	double damping = 0.0;
	seed_t seed = 20260921;
	size_t runs = 300;
	size_t cases = 6;
	int stages = 0;  // bit0: S0; bit1: S1; bit2: S2
	double f_min = 0.99;
	double tvd_max = 0.05;
	double trace_gap = 0.02;
	double fid_gap = 0.05;
};

struct References
{
	map<string, double> noisefree, avg;
	double survival = 1.0, overlap_fid = 0.0;
    RhoMap avg_rho;
	vector<map<string, double>> case_dist;
	vector<vector<StepPlan>> case_plan;
	vector<cplx> psi_ideal;
};

References build_references(const Experiment& E, const memory_t& memory)
{
	References R;
	Encoding enc;
	enc.qutrit = E.qutrit;
	noise_t noise;
	if (E.depol > 0) noise[OperationType::Depolarizing] = E.depol;
	if (E.damping > 0) noise[OperationType::Damping] = E.damping;
	auto entangle_fill = [&](auto& plan, auto& q)
	{
		for (size_t s = 0; s < plan.size(); ++s)
			plan[s].entangle_max = q.time_step.layer_entangle_max(s + 1);
	};

	if (E.qutrit)
	{
		{
			qram_qutrit::QRAMCircuit q0(2, 1, memory);
			set_input_qutrit(q0);
			q0.run_full();
			R.noisefree = dist_qutrit(q0);
			auto plan = translate_schedule(q0.get_operations(), enc, memory);
			entangle_fill(plan, q0);
			R.psi_ideal = replay_statevector(plan, enc);
		}
		qram_qutrit::QRAMCircuit q(2, 1, memory);
		q.set_noise_models(noise);
		random_engine::set_seed(E.seed);
		set_input_qutrit(q);
		for (size_t r = 0; r < E.runs; ++r)
		{
			q.pick_all();
			q.run_valid_branches();
			auto d = dist_qutrit(q);
			for (auto& [k, v] : d) R.avg[k] += v;
			R.overlap_fid += overlap_fid_qutrit(q, memory);
			try { q.sample_and_get_fidelity(); }
			catch (const std::exception&) {}
		}
		for (auto& [k, v] : R.avg) v /= (double)E.runs;
		R.overlap_fid /= (double)E.runs;
		R.survival = 0;
		for (auto& [k, v] : R.avg) R.survival += v;

		/* Replica trajectories: outcome interception + per-trajectory consistency + case export */
		random_engine::set_seed(E.seed);
		set_input_qutrit(q);
		for (size_t r = 0; r < E.runs; ++r)
		{
			q.pick_all();
			vector<NoiseEntry> log;
			replica_run_qutrit(q, 0, &log);
			auto d = dist_qutrit(q);
			if (r < E.cases)
			{
				map<pair<size_t, size_t>, int> outcomes;
				for (auto& o : log) outcomes[{ o.step, o.pos }] = o.outcome;
				auto plan = translate_schedule(q.get_operations(), enc, memory, &outcomes);
				entangle_fill(plan, q);
				R.case_plan.push_back(std::move(plan));
				R.case_dist.push_back(d);
			}
			try { q.sample_and_get_fidelity(); }
			catch (const std::exception&) {}
		}
	}
	else
	{
		{
			qram_qubit::QRAMCircuit q0(2, 1, memory_t(memory));
			set_input_qubit(q0);
			q0.run_full();
			R.noisefree = dist_qubit(q0);
			auto plan = translate_schedule(q0.get_operations(), enc, memory);
			entangle_fill(plan, q0);
			R.psi_ideal = replay_statevector(plan, enc);
		}
		qram_qubit::QRAMCircuit q(2, 1, memory_t(memory));
		q.set_noise_models(noise);
		random_engine::set_seed(E.seed);
		set_input_qubit(q);
		for (size_t r = 0; r < E.runs; ++r)
		{
			q.prepare_all();
			q.run_bad();
			auto d = dist_qubit(q);
			for (auto& [k, v] : d) R.avg[k] += v;
			R.overlap_fid += overlap_fid_qubit(q, memory);
            // Reconstruct the coherent address+bus+tree state for the supported
            // one-input-bus-column-per-address loading experiment.
            map<size_t, cplx> psi;
            for (const auto& group : q.branch_groups)
                for (size_t b = 0; b < group.branches.size(); ++b)
                    for (auto it = group.branches[b].iterbeg(); it != group.branches[b].iterend(); ++it) {
                        size_t index = group.address | (it->data_bus << q.addr_size);
                        for (size_t pos : it->state.nz_elements)
                            index |= size_t(1) << (q.addr_size + q.data_size + pos);
                        psi[index] += sqrt(group.branch_probs[b]) * it->amplitude;
                    }
            for (const auto& [i, ai] : psi)
                for (const auto& [j, aj] : psi)
                    R.avg_rho[(uint64_t(i) << enc.num_qubits()) | j] += ai * conj(aj);

			try { q.sample_and_get_fidelity(); }
			catch (const std::exception&) {}
		}
		for (auto& [k, v] : R.avg) v /= (double)E.runs;
		R.overlap_fid /= (double)E.runs;
		R.survival = 0;
		for (auto& [k, v] : R.avg) R.survival += v;

		random_engine::set_seed(E.seed);
		set_input_qubit(q);
		size_t probe_count = 0;
		for (size_t r = 0; r < E.runs; ++r)
		{
			q.prepare_all();
			vector<NoiseEntry> log;
			if (getenv("VNS_TRACE2") && r < E.cases)
				fmt::print("    [qram r{}]:\n", r);
			replica_run_qubit(q, 0, &log);
			auto d = dist_qubit(q);
			if (r < E.cases)
			{
				map<pair<size_t, size_t>, int> outcomes;
				for (auto& o : log) outcomes[{ o.step, o.pos }] = o.outcome;
				auto plan = translate_schedule(q.get_operations(), enc, memory, &outcomes);
				entangle_fill(plan, q);
				R.case_plan.push_back(std::move(plan));
				R.case_dist.push_back(d);
			}
			try { q.sample_and_get_fidelity(); }
			catch (const std::exception&) {}
		}
	}
    for (auto& [key, value] : R.avg_rho) value /= static_cast<double>(E.runs);
	return R;
}

/* S2 channel level (faithful; mode: full / nojump / faithful / depol_textbook) */
pair<RhoMap, map<string, double>> run_stage2(const vector<StepPlan>& plan,
	const Encoding& enc, size_t n, const string& mode,
	double depol, double damping)
{
	RhoMap rho;
	vector<cplx> psi0 = initial_state(enc);
	/* Coherent initial state: |ψ><ψ| */
	for (size_t i = 0; i < psi0.size(); ++i)
	{
		if (psi0[i] == cplx(0.0, 0.0)) continue;
		for (size_t j = 0; j < psi0.size(); ++j)
		{
			if (psi0[j] == cplx(0.0, 0.0)) continue;
			rho[(uint64_t(i) << n) | j] = psi0[i] * conj(psi0[j]);
		}
	}

	for (auto& st : plan)
	{
		for (auto& g : st.gates)
			apply_kraus_rho(rho, n, g.qs, { g.M }, g.ctrls);
		/* Depolarizing channel (active positions [0, 2(2^L−1)), marginal probability p) */
		if (depol > 0)
		{
			size_t n_active = 2 * ((1ull << st.entangle_max) - 1);
			for (size_t pos = 0; pos < n_active; ++pos)
			{
				size_t v = pos / 2, base = enc.node_base(v);
				bool sub_data = pos % 2 == 1;
				vector<Matrix> Ks;
				double c0 = sqrt(1.0 - depol), c1 = sqrt(depol / 3.0);
				if (enc.qutrit && !sub_data)
				{
					Ks.push_back(scaled(mat_id(4), c0));
					for (int k = 0; k < 8; ++k)
						Ks.push_back(scaled(qutrit_weyl(k), sqrt(depol / 8.0)));
					apply_kraus_rho(rho, n, { base + 1, base }, Ks);
				}
				else
				{
					Matrix third = (mode == "depol_textbook")
						? Matrix{ 0.0, cplx(0, -1), cplx(0, 1), 0.0 }  /* Y */
						: (enc.qutrit ? mat_xz() : qubit_M());
					Ks = { scaled(mat_id(2), c0), scaled(mat_x(), c1),
						   scaled(mat_z(), c1), scaled(third, c1) };
					apply_kraus_rho(rho, n, { sub_data ? enc.node_dq(v) : base }, Ks);
				}
			}
		}
		/* Damping (applied to the whole tree wherever Damp_Common occurs) */
		for (auto& o : st.noises)
		{
			if (o.type != OperationType::Damp_Common) continue;
			double gamma = o.coef;
			double S = rho_trace(rho, n);
			if (S <= 0) continue;
			size_t n_active_nodes = (1ull << st.entangle_max) - 1;
			for (size_t v = 0; v < pow2(enc.addr_size) - 1; ++v)
			{
				size_t base = enc.node_base(v);
				bool active = v < n_active_nodes;
				if (mode == "full")
				{
					if (enc.qutrit)
					{
						apply_kraus_rho(rho, n, { base + 1, base },
							{ qutrit_k0(gamma), scaled(qutrit_jump(0), sqrt(gamma)),
							  scaled(qutrit_jump(1), sqrt(gamma)) });
						apply_kraus_rho(rho, n, { base + 2 },
							{ qubit_k0(gamma), scaled(jump_2q(), sqrt(gamma)) });
					}
					else
					{
						apply_kraus_rho(rho, n, { base },
							{ qubit_k0(gamma), scaled(jump_2q(), sqrt(gamma)) });
						apply_kraus_rho(rho, n, { base + 1 },
							{ qubit_k0(gamma), scaled(jump_2q(), sqrt(gamma)) });
					}
					continue;
				}
					/* nojump / faithful: take the current weights first */
				auto w_of = [&](const vector<Ctrl>& mask)
				{ return rho_mask_weight(rho, n, mask); };
				if (enc.qutrit)
				{
					double wL = w_of({ { base, 0 }, { base + 1, 1 } });
					double wR = w_of({ { base, 1 }, { base + 1, 0 } });
					if (mode == "nojump")
					{
						apply_kraus_rho(rho, n, { base + 1, base }, { qutrit_k0(gamma) });
					}
					else
					{
						/* The K0 branch is scaled overall by a = sqrt(1 − γw/S) (incl. the ground-state part — trajectory-ensemble weight semantics) */
						double a = active ? sqrt(max(1.0 - gamma * (wL + wR) / S, 0.0)) : 1.0;
						apply_kraus_rho(rho, n, { base + 1, base },
							{ scaled(qutrit_k0(gamma), a),
							  scaled(qutrit_jump(0), active ? sqrt(gamma * wL / S) : 0.0),
							  scaled(qutrit_jump(1), active ? sqrt(gamma * wR / S) : 0.0) });
					}
					double wd = w_of({ { base + 2, 1 } });
					if (mode == "nojump")
					{
						apply_kraus_rho(rho, n, { base + 2 }, { qubit_k0(gamma) });
					}
					else
					{
						double ad = active ? sqrt(max(1.0 - gamma * wd / S, 0.0)) : 1.0;
						apply_kraus_rho(rho, n, { base + 2 },
							{ scaled(qubit_k0(gamma), ad),
							  scaled(jump_2q(), active ? sqrt(gamma * wd / S) : 0.0) });
					}
				}
				else
				{
					for (int bit = 0; bit < 2; ++bit)
					{
						size_t qb = base + bit;
						double w = w_of({ { qb, 1 } });
						if (mode == "nojump")
						{
							apply_kraus_rho(rho, n, { qb }, { qubit_k0(gamma) });
						}
						else
						{
							double a = active ? sqrt(max(1.0 - gamma * w / S, 0.0)) : 1.0;
							apply_kraus_rho(rho, n, { qb },
								{ scaled(qubit_k0(gamma), a),
								  scaled(jump_2q(), active ? sqrt(gamma * w / S) : 0.0) });
						}
					}
				}
			}
		}
	}
	return { std::move(rho), dist_of_rho(rho, n, enc) };
}

void run_experiment(const Experiment& E)
{
	fmt::print("== {} ==\n", E.name);
	memory_t memory = { 0, 1, 1, 0 };
	References R = build_references(E, memory);
	Encoding enc;
	enc.qutrit = E.qutrit;
	size_t n = enc.num_qubits();

	if (E.stages & 1)
	{
		auto d = dist_of_psi(R.psi_ideal, enc);
		double md = max_diff(d, R.noisefree);
		check(md < 1e-9, "S0 noise-free bridge", fmt::format("max|ΔP|={:.2e}", md));
	}
	if (E.stages & 2)
	{
		bool all_ok = true;
		double worst = 0;
		for (size_t c = 0; c < R.case_plan.size(); ++c)
		{
			if (getenv("VNS_TRACE"))
				fmt::print("    [replay case {}]:\n", c);
			auto psi = replay_statevector(R.case_plan[c], enc);
			auto d = dist_of_psi(psi, enc);
			double md = max_diff(d, R.case_dist[c]);
			worst = max(worst, md);
			all_ok &= md < 1e-9;
			if (md >= 1e-9 && getenv("VNS_DEBUG"))
			{
				fmt::print("    [debug case {}] max|ΔP|={:.3e}\n", c, md);
				for (auto& [k, v] : d)
					if (fabs(v - (R.case_dist[c].count(k) ? R.case_dist[c].at(k) : 0.0)) > 1e-9)
						fmt::print("      {}: replay={:.6f} qram={:.6f}\n", k, v,
							R.case_dist[c].count(k) ? R.case_dist[c].at(k) : 0.0);
				for (auto& st : R.case_plan[c])
					for (auto& o : st.noises)
						fmt::print("      noise@{}: type={} pos={} coef={:.4f} outcome={}\n",
							st.entangle_max, (int)o.type, o.pos, o.coef, o.outcome);
				fmt::print("      replay sum={:.6f} qram sum={:.6f}\n",
					[&]{ double t=0; for (auto& [k,v]:d) t+=v; return t; }(),
					[&]{ double t=0; for (auto& [k,v]:R.case_dist[c]) t+=v; return t; }());
			}
		}
		check(all_ok, "S1 per-random-case replay", fmt::format("{} cases, worst={:.2e}",
			R.case_plan.size(), worst));
	}
	if (E.stages & 4)
	{
		if (getenv("VNS_DEBUG"))
		{
			auto [rho0, d0] = run_stage2(R.case_plan[0], enc, n, "none", 0.0, 0.0);
			fmt::print("    [debug] S2 gates-only: trace={:.6f} max|ΔP|vs noisefree={:.2e}\n",
				rho_trace(rho0, n), max_diff(d0, R.noisefree));
		}
		// Qubit validation uses the standard linear, trace-preserving Kraus
        // channel. The qutrit legacy mirror is retained as an implementation
        // regression only; it is NOT a physical amplitude-damping certificate.
        string mode = E.qutrit ? "faithful" : "full";
		auto [rho, d] = run_stage2(R.case_plan[0], enc, n, mode, E.depol, E.damping);
		double tr = rho_trace(rho, n);
		double sr = R.survival;
		double f = classical_fidelity(d, tr > 0
			? [&]{ map<string, double> m; for (auto& [k, v] : d) m[k] = v / tr; return m; }()
			: d);
		map<string, double> avg_n;
		for (auto& [k, v] : R.avg) avg_n[k] = v / sr;
		double f_cls = classical_fidelity(avg_n, tr > 0
			? [&]{ map<string, double> m; for (auto& [k, v] : d) m[k] = v / tr; return m; }()
			: d);
		double t = tvd(avg_n, tr > 0
			? [&]{ map<string, double> m; for (auto& [k, v] : d) m[k] = v / tr; return m; }()
			: d);
		double fq = overlap_fid(R.psi_ideal, rho, n);
		check(f_cls > E.f_min, E.qutrit ? "S2 legacy-mirror F_cls" : "S2 standard F_cls", fmt::format("{:.4f}", f_cls));
		check(t < E.tvd_max, E.qutrit ? "S2 legacy-mirror TVD" : "S2 standard TVD", fmt::format("{:.4f}", t));
		check(fabs(tr - sr) < E.trace_gap, E.qutrit ? "S2 legacy trace↔raw norm" : "S2 trace↔normalized ensemble",
			fmt::format("trace={:.4f} survival={:.4f}", tr, sr));
		check(fabs(fq - R.overlap_fid) < E.fid_gap, "S2 F_quantum↔avg_overlap_fid",
			fmt::format("{:.4f} vs {:.4f}", fq, R.overlap_fid));
        if (!E.qutrit) {
            check(fabs(tr - 1) < 1e-10 && fabs(sr - 1) < 1e-10,
                "S2 standard channel trace=1", fmt::format("rho={:.12f} trajectories={:.12f}", tr, sr));
            double purity = 0, error2 = 0;
            RhoMap difference = rho;
            for (const auto& [key, value] : R.avg_rho) {
                purity += norm(value);
                difference[key] -= value;
            }
            for (const auto& [key, value] : difference) error2 += norm(value);
            // Every trajectory is a normalized pure state. Its full-density
            // sample mean has estimated squared Frobenius standard error
            // (1 - purity(mean))/(runs - 1). Includes ALL coherences and tree.
            double se = sqrt(max(0.0, 1-purity) / static_cast<double>(E.runs-1));
            check(sqrt(error2) < 5*se + 1e-10, "S2 complete density matrix",
                fmt::format("Frobenius={:.5g} estimated_RMS_SE={:.5g}", sqrt(error2), se));
        }
        (void)f;
	}
}

} // namespace

int main()
{
	fmt::print("verify_noisy_simulation: qutrit/qubit QRAM ↔ self-contained circuit noise simulator cross-check\n\n");

	/* qutrit architecture */
	run_experiment({ "qutrit / noise-free bridge", true, 0, 0, 20260921, 8, 0, 1 });
	run_experiment({ "qutrit / depol p=0.02 / S1", true, 0.02, 0, 20260921, 8, 6, 2 });
	run_experiment({ "qutrit / depol p=0.3 / S1", true, 0.3, 0, 777, 8, 6, 2 });
	run_experiment({ "qutrit / damp γ=0.05 / S1", true, 0, 0.05, 20260921, 8, 6, 2 });
	run_experiment({ "qutrit / depol p=0.02 / S2", true, 0.02, 0, 20260921, 300, 1, 4,
		0.995, 0.05, 0.02, 0.05 });
	run_experiment({ "qutrit / damp γ=0.05 / S2", true, 0, 0.05, 20260921, 300, 1, 4,
		0.99, 0.05, 0.02, 0.05 });
	run_experiment({ "qutrit / mixed p=γ=0.02 / S2", true, 0.02, 0.02, 20260921, 300, 1, 4,
		0.995, 0.05, 0.02, 0.05 });

	/* qubit architecture */
	run_experiment({ "qubit / noise-free bridge", false, 0, 0, 20260921, 8, 0, 1 });
	run_experiment({ "qubit / depol p=0.02 / S1", false, 0.02, 0, 20260921, 8, 6, 2 });
	run_experiment({ "qubit / depol p=0.3 / S1", false, 0.3, 0, 777, 8, 6, 2 });
	run_experiment({ "qubit / damp γ=0.05 / S1", false, 0, 0.05, 20260921, 8, 6, 2 });
    run_experiment({ "qubit / mixed p=gamma=0.2 / S1", false, 0.2, 0.2, 777, 8, 6, 2 });
	run_experiment({ "qubit / depol p=0.02 / S2", false, 0.02, 0, 20260921, 2000, 1, 4,
		0.995, 0.05, 0.01, 0.05 });
	run_experiment({ "qubit / damp γ=0.05 / S2", false, 0, 0.05, 20260921, 2000, 1, 4,
		0.99, 0.05, 0.01, 0.05 });
	run_experiment({ "qubit / mixed p=γ=0.02 / S2", false, 0.02, 0.02, 20260921, 2000, 1, 4,
		0.995, 0.05, 0.01, 0.05 });

    run_experiment({ "qubit / damp gamma=0.2 / standard channel", false, 0, 0.2,
        777, 2000, 1, 4, 0.99, 0.05, 0.01, 0.05 });

	fmt::print("\n{} assertion(s) failed\n", g_failures);
	return g_failures ? 1 : 0;
}
