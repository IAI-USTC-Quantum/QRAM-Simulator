/* Physical partial-trace observable, tested on explicit joint pure states.
 * Orthogonal environment labels must not interfere. Identical labels must
 * retain complex cross-address coherence. No noise-theory fitting enters.
 */
#include "qram_circuit_qubit.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace qram_simulator;
using namespace qram_simulator::qram_qubit;

void close(double actual, double expected, const char* label) {
    if (!std::isfinite(actual) || std::abs(actual-expected) > 2e-12)
        throw std::runtime_error(std::string(label)+": "+std::to_string(actual)+
                                 " != "+std::to_string(expected));
}

QRAMCircuit state(double first_weight = 0.5) {
    QRAMCircuit q(2, 1, memory_t{0, 1, 1, 0});
    for (size_t address : {0, 1}) {
        q.branch_groups.emplace_back(address);
        auto& g = q.branch_groups.back();
        g.branch_probs = {address == 0 ? first_weight : 1-first_weight};
        g.branches_input.emplace_back(address, 1, static_cast<bus_t>(0));
        g.branches = g.branches_input;
        g.branches.front().system_states.front().data_bus = q.memory[address];
    }
    return q;
}

void reject(QRAMCircuit& q, const char* label) {
    try { q.logical_query_fidelity(); }
    catch (const std::invalid_argument&) { return; }
    throw std::runtime_error(std::string("accepted invalid input: ")+label);
}

int main() {
    try {
        auto q = state();
        close(q.logical_query_fidelity(), 1, "ideal");
        for (auto& g : q.branch_groups)
            g.branches.front().system_states.front().state.nz_elements.insert(3);
        close(q.logical_query_fidelity(), 1, "common tree excitation is traced out");
        q.branch_groups.front().branches.front().system_states.front().state.clear();
        close(q.logical_query_fidelity(), 0.5, "orthogonal environments destroy coherence");
        q = state(0.1);
        q.branch_groups.back().branches.front().system_states.front().state.nz_elements.insert(3);
        close(q.logical_query_fidelity(), 0.82, "unequal weights after dephasing");
        q = state();
        q.branch_groups.back().branches.front().system_states.front().amplitude = -1;
        close(q.logical_query_fidelity(), 0, "relative phase with perfect populations");
        for (auto& g : q.branch_groups)
            g.branches.front().system_states.front().amplitude = complex_t(0, 1);
        close(q.logical_query_fidelity(), 1, "common global phase");

        q = state();
        for (auto& g : q.branch_groups) {
            auto& b = g.branches.front();
            b.system_states.resize(2);
            b.system_states.front().amplitude = std::sqrt(0.5);
            b.system_states[1] = b.system_states.front();
            b.system_states[1].state.nz_elements.insert(3);
            b.system_states[1].amplitude = complex_t(0, std::sqrt(0.5));
            b.system_states_sz = 2;
        }
        close(q.logical_query_fidelity(), 1, "common environment superposition");
        q.branch_groups.back().branches.front().system_states[1].amplitude *= -1;
        close(q.logical_query_fidelity(), 0.5, "orthogonal complex environment superpositions");
        q = state();
        for (auto& g : q.branch_groups) {
            auto& b = g.branches.front();
            b.system_states.resize(2);
            b.system_states.front().amplitude = std::sqrt(0.7);
            b.system_states[1] = b.system_states.front();
            b.system_states[1].data_bus ^= 1;
            b.system_states[1].state.nz_elements.insert(3);
            b.system_states[1].amplitude = std::sqrt(0.3);
            b.system_states_sz = 2;
        }
        close(q.logical_query_fidelity(), 0.7, "wrong-bus support is excluded");

        q = state(); q.branch_groups.back().branch_probs = {-0.5}; reject(q, "negative weight");
        q = state(); q.branch_groups.back().branch_probs = {0.1}; reject(q, "normalization");
        q = state(); q.branch_groups.back().address = 0; reject(q, "duplicate address");
        q = state(); q.branch_groups.front().branches_input.front().bus_input = 1; reject(q, "nonzero bus");
        q = state(); q.branch_groups.front().branches_input.front().system_states.front().amplitude = complex_t(0, 1);
        reject(q, "noncanonical input phase");
        q = state(); q.branch_groups.front().branches.clear(); reject(q, "no completed run");

        const noise_t channels[] = {{}, {{OperationType::BitFlip, 0.04}},
            {{OperationType::PhaseFlip, 0.04}}, {{OperationType::Depolarizing, 0.04}},
            {{OperationType::Damping, 0.04}}};
        size_t cases = 0;
        for (const auto& noise : channels) for (seed_t seed = 1; seed <= 12; ++seed) {
            auto full = state(), pruned = state();
            full.set_noise_models(noise); pruned.set_noise_models(noise);
            random_engine::get_instance().set_seed(seed); full.run_full();
            auto reference = full.logical_query_fidelity();
            auto sampled = full.sample_and_get_fidelity();
            random_engine::get_instance().set_seed(seed); pruned.run_normal();
            auto rng_before = random_engine::get_engine();
            auto result = pruned.logical_query_fidelity();
            close(result, reference, "pruned/full partial trace");
            close(pruned.logical_query_fidelity(), result, "idempotence");
            if (!(rng_before == random_engine::get_engine()))
                throw std::runtime_error("observable changed the random-engine state");
            close(pruned.sample_and_get_fidelity(), sampled, "readout RNG unchanged");
            if (result < -1e-12 || result > 1+1e-12)
                throw std::runtime_error("physical fidelity outside [0,1]");
            ++cases;
        }
        std::cout << "PASS physical partial trace, invalid inputs and " << cases
                  << " native pruned/full/RNG cases\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
