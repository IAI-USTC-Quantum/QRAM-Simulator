/* Exact joint-damping regression against independent dense Kraus channels.
 * No QRAM routing or branch prediction enters the dense reference. Enumerate
 * every candidate mask and every auxiliary-mask outcome, including coherent
 * off-diagonal matrix entries; this is stronger than population-only checks.
 */
#include "qram_circuit_qubit.h"
#include <cmath>
#include <iostream>
#include <random>

using namespace qram_simulator;
using namespace qram_simulator::qram_qubit;
using Vector = std::vector<std::complex<double>>;
using Density = std::vector<Vector>;

std::vector<size_t> positions(size_t x) {
    std::vector<size_t> result;
    for (size_t q = 0; x; ++q, x >>= 1) if (x & 1) result.push_back(q);
    return result;
}
QRAMCircuit make_state(const Vector& psi) {
    QRAMCircuit q(3, 1);
    q.branch_groups.emplace_back(0);
    auto& group = q.branch_groups.back();
    group.branch_probs = {1.0};
    group.branches.emplace_back(0, 1, static_cast<bus_t>(0));
    auto& branch = group.branches.back();
    branch.system_states.clear();
    for (size_t x = 0; x < psi.size(); ++x) {
        if (std::norm(psi[x]) == 0) continue;
        SystemState state(0, 1);
        auto bits = positions(x);
        state.state.nz_elements.insert(bits.begin(), bits.end());
        state.amplitude = psi[x];
        branch.system_states.push_back(state);
    }
    branch.system_states_sz = branch.system_states.size();
    q.valid_branch_group_view = {&group};
    return q;
}
Vector export_state(const QRAMCircuit& q, size_t dim) {
    Vector out(dim);
    for (const auto& group : q.branch_groups)
        for (const auto& branch : group.branches)
            for (auto it = branch.iterbeg(); it != branch.iterend(); ++it) {
                size_t x = 0;
                for (size_t bit : it->state.nz_elements) x |= size_t(1) << bit;
                out.at(x) += it->amplitude;
            }
    return out;
}
Density outer(const Vector& psi) {
    Density rho(psi.size(), Vector(psi.size()));
    for (size_t i = 0; i < psi.size(); ++i)
        for (size_t j = 0; j < psi.size(); ++j) rho[i][j] = psi[i] * std::conj(psi[j]);
    return rho;
}
Density dense_channel(Density rho, size_t n, double gamma) {
    // Independently apply rho <- K0 rho K0^dagger + K1 rho K1^dagger,
    // one bit at a time. This reference does not sample candidates.
    const size_t dim = rho.size();
    for (size_t q = 0; q < n; ++q) {
        Density out(dim, Vector(dim));
        const size_t bit = size_t(1) << q;
        for (size_t i = 0; i < dim; ++i)
            for (size_t j = 0; j < dim; ++j) {
                const double ai = (i & bit) ? std::sqrt(1-gamma) : 1;
                const double aj = (j & bit) ? std::sqrt(1-gamma) : 1;
                out[i][j] += ai * aj * rho[i][j];
                if ((i & bit) && (j & bit)) out[i ^ bit][j ^ bit] += gamma * rho[i][j];
            }
        rho = std::move(out);
    }
    return rho;
}
int main() {
    std::mt19937_64 rng(20260926);
    std::uniform_real_distribution<double> uniform(-1, 1);
    double worst = 0, worst_trace = 0;
    size_t cases = 0;
    for (size_t n = 1; n <= 4; ++n) {
        const size_t dim = size_t(1) << n;
        for (double gamma : {0.0, 1e-5, 0.2, 0.7, 0.95}) {
            for (int input = 0; input < 4; ++input) {
                Vector psi(dim);
                if (input == 0) psi[0] = 1;
                if (input == 1) psi[dim-1] = 1;
                if (input == 2) psi[0] = psi[dim-1] = 1/std::sqrt(2.0);
                if (input == 3) for (auto& z : psi) z = {uniform(rng), uniform(rng)};
                double norm = 0;
                for (auto z : psi) norm += std::norm(z);
                for (auto& z : psi) z /= std::sqrt(norm);
                Density observed(dim, Vector(dim));
                for (size_t mask = 0; mask < dim; ++mask) {
                    const auto candidates = positions(mask);
                    const double pc = std::pow(gamma, candidates.size())
                        * std::pow(1-gamma, n-candidates.size());
                    if (pc == 0) continue;
                    auto q = make_state(psi);
                    const auto weights = q.damping_mask_weights(candidates, 1, gamma);
                    for (const auto& [jumps, weight] : weights) {
                        if (weight <= 0) continue;
                        auto conditional = make_state(psi);
                        conditional.apply_damping_outcome(jumps, gamma);
                        auto output = export_state(conditional, dim);
                        double output_norm = 0;
                        for (auto z : output) output_norm += std::norm(z);
                        if (!(output_norm > 0)) return 2;
                        for (size_t i = 0; i < dim; ++i)
                            for (size_t j = 0; j < dim; ++j)
                                observed[i][j] += pc * weight * output[i] * std::conj(output[j]) / output_norm;
                    }
                }
                auto expected = dense_channel(outer(psi), n, gamma);
                double trace = 0;
                for (size_t i = 0; i < dim; ++i) {
                    trace += observed[i][i].real();
                    for (size_t j = 0; j < dim; ++j)
                        worst = std::max(worst, std::abs(observed[i][j]-expected[i][j]));
                }
                worst_trace = std::max(worst_trace, std::abs(trace-1));
                ++cases;
            }
        }
    }
    // End-to-end roulette, projection and once-per-layer normalization.
    Vector bell{1/std::sqrt(2.0), 0, 0, 1/std::sqrt(2.0)};
    size_t counts[3] = {};
    constexpr size_t shots = 60000;
    random_engine::set_seed(20260926);
    std::bernoulli_distribution candidate(0.2);
    for (size_t i = 0; i < shots; ++i) {
        std::vector<size_t> c;
        for (size_t bit = 0; bit < 2; ++bit)
            if (candidate(random_engine::get_engine())) c.push_back(bit);
        auto q = make_state(bell);
        q.run_damping_layer(c, 1, 0.2);
        ++counts[q.fired_jump_count];
        worst_trace = std::max(worst_trace, std::abs(q.get_normalization_factor_with_damping()-1));
    }
    // In particular, the legacy 0.17 single-jump probability must be rejected.
    double single = double(counts[1])/shots;
    std::cout << "exact_cases=" << cases << " max_density_error=" << worst
              << " max_trace_error=" << worst_trace << " bell_single_jump=" << single
              << " expected=0.16 legacy=0.17 shots=" << shots << '\n';
    return worst < 2e-12 && worst_trace < 2e-12 && std::abs(single-0.16) < 0.005 ? 0 : 1;
}
