#pragma once
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <omp.h>
#include <functional>
#include <error_handler.h>
#include <fmt/core.h>
#include <chrono>
#include "global_macros.h"
#include "sparse_state_simulator.h"
#include "basic_gates.h"

using namespace std;

namespace qram_simulator
{
    namespace qasm_parser
    {
        // Data structure to store gate operations
        struct GateOperation {
            std::string gate;
            std::vector<double> angles;
            std::vector<std::pair<std::string, size_t>> targets;
            GateOperation() : gate(""), angles(), targets() {};
            GateOperation(std::string gate_, std::vector<double> angles_, std::vector<std::pair<std::string, size_t>> targets_)
                : gate(gate_), angles(angles_), targets(targets_)
            {
                // Check if gate is supported
            };
        };

        // Data structure to store parsed QASM information
        struct Qasm {
            std::vector<std::pair<std::string, int>> qregs;
            std::vector<std::pair<std::string, int>> cregs;
            std::vector<GateOperation> operations;

            Qasm(std::vector<std::pair<std::string, int>> qregs_, std::vector<std::pair<std::string, int>> cregs_, std::vector<GateOperation> ops_)
                : qregs(qregs_), cregs(cregs_), operations(ops_) {}
        };

        struct QasmParser {
            Qasm parse(const std::string& content) {
                static const std::regex format_spec_regex("OPENQASM\\s+([\\d+\\.\\d+]);");
                static const std::regex include_regex("include\\s+\"qelib1.inc\";");
                static const std::regex qreg_regex("qreg\\s+(\\w+)\\[(\\d+)\\];");
                static const std::regex creg_regex("creg\\s+(\\w+)\\[(\\d+)\\];");
                static const std::regex gate_regex(
                    R"((h|y|z|s|sdg|t|tdg|x|sxdg|sx|cx|cz|ccx|phase|cphase|rx|ry|rz|swap|cswap|u|u1|u2|u3)\s*(?:\(([^)]*)\))?\s+(.*?);)",
                    std::regex::icase
                );
                //static const std::regex gate_regex("(h|y|z|s|sdg|t|tdg|x|sxdg|sx|cx|cz|ccx|phase|cphase|rx|ry|rz|swap|cswap|u|u1|u2|u3)\s*\(?(.*?)(?<!\))\)?\s+(.*?);", std::regex::icase); 
                static const std::regex meas_regex("measure\\s+.*?;", std::regex::icase);
                static const std::regex barrier_regex("barrier\\s+.*?;", std::regex::icase);
                static const std::regex comment_regex(R"(//.*)");
                static const std::regex empty_line_regex("\\s*$");

                // Detect user-defined gate and throw an error
                static const std::regex user_gate_regex(R"(gate\\s+(\\w+)\\s*\(?.*?\)?\\s*\{)");

                if (std::regex_search(content, user_gate_regex)) {
                    throw std::runtime_error("User-defined gates are not supported.");
                }

                // Split content by semicolon and remove empty lines
                std::vector<std::string> lines;
                size_t start = 0;
                size_t end = content.find('\n');


                while (end != std::string::npos) {
                    if (start == end)
                    {
                        start = end + 1;
                        if (start >= content.size()) break;
                        end = content.find('\n', start);
                        continue;
                    }
                    std::string line = content.substr(start, end - start);
                    /*line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());*/
                    //fmt::print("Before: {}\n", line);
                    line.erase(0, line.find_first_not_of(" \t\r\n"));
                    line.erase(line.find_last_not_of(" \t\r\n") + 1);
                    //fmt::print("Mid: {}\n", line);
                    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (std::regex_match(line, empty_line_regex)) {
                        continue;
                    }

                    if (!line.empty()) {
                        lines.push_back(line);
                        //fmt::print("End: {}\n", line);
                    }
                    start = end + 1;
                    if (start >= content.size()) {
                        break;  // Prevent endless loop if start reaches beyond the content size
                    }
                    end = content.find('\n', start);


                }

                /*for (auto s : lines) {
                    fmt::print("{}\n", s);
                }*/

                std::vector<std::pair<std::string, int>> qregs;
                std::vector<std::pair<std::string, int>> cregs;
                std::vector<GateOperation> operations;

                // Parse each line
                for (const auto& line : lines) {
                    // Check if line is a comment
                    if (std::regex_match(line, comment_regex)) {
                        //fmt::print("Type is {}\n", 1);
                        continue;
                    }

                    // Check format specification
                    std::smatch match;
                    if (std::regex_match(line, match, format_spec_regex)) {
                        if (match[1] != "2.0") {
                            throw std::runtime_error("Unsupported QASM version. Only version 2.0 is supported.");
                        }
                        //fmt::print("Type is {}\n", 2);
                        continue;
                    }

                    // Check include statement
                    if (std::regex_match(line, include_regex)) {
                        //fmt::print("Type is {}\n", 3);
                        continue;
                    }

                    // Check quantum register declaration
                    if (std::regex_match(line, match, qreg_regex)) {
                        qregs.emplace_back(match[1], std::stoi(match[2]));
                        //fmt::print("Type is {}\n", 4);
                        continue;
                    }

                    // Check classical register declaration
                    if (std::regex_match(line, match, creg_regex)) {
                        cregs.emplace_back(match[1], std::stoi(match[2]));
                        //fmt::print("Type is {}\n", 5);
                        continue;
                    }

                    // Check gate operation
                    if (std::regex_match(line, match, gate_regex)) {
                        GateOperation operation;
                        operation.gate = match[1];

                        // Parse angles if present
                        std::string angle_str = match[2];
                        std::vector<double> angles;
                        if (!angle_str.empty()) {
                            std::regex angle_regex(R"(-?\d*\.?\d+(?:e-?\d+)?|pi)");
                            std::sregex_iterator angle_it(angle_str.begin(), angle_str.end(), angle_regex);
                            std::sregex_iterator angle_end;
                            while (angle_it != angle_end) {
                                if ((*angle_it).str() == "pi") {
                                    angles.push_back(pi);
                                }
                                else {
                                    angles.push_back(std::stod((*angle_it).str()));
                                }
                                ++angle_it;
                            }
                        }

                        // Parse targets
                        std::string targets_str = match[3];
                        std::regex target_regex(R"((\w+)\[(\d+)\])");
                        std::sregex_iterator target_it(targets_str.begin(), targets_str.end(), target_regex);
                        std::sregex_iterator target_end;
                        while (target_it != target_end) {
                            operation.targets.emplace_back((*target_it)[1], std::stoi((*target_it)[2]));
                            ++target_it;
                        }

                        operation.angles = angles;
                        operations.push_back(operation);
                        continue;
                    }

                    // Check measure operation
                    if (std::regex_match(line, match, meas_regex)) {
                        // Measurement operations can be processed or ignored as needed
                        //fmt::print("Type is {}\n", 7);
                        continue;
                    }

                    // Check barrier
                    if (std::regex_match(line, barrier_regex)) {
                        // Barrier operations can be processed or ignored as needed
                        //fmt::print("Type is {}\n", 8);
                        continue;
                    }
                }

                return Qasm(qregs, cregs, operations);
            }
        };

        // Here we define a class for quantum circuit implementation and simulation from a Qasm class;
        // The implementation consists of three steps:
        // 1. Initalize the quantum registers and quantum state;
        // 2. Match each operation in Qasm class to its corresponding implementation and apply it to the quantum state;
        // 3. (Optional) Partial trace select qubits to obtain the measurement results.
        struct QasmSimulator {
            Qasm qasm;
            QasmSimulator(const Qasm& qasm)
                : qasm(qasm)
            {     }
            void init() {
                // Initialize quantum registers and quantum state
                for (const auto& qreg : qasm.qregs) {
                    System::add_register(qreg.first, UnsignedInteger, qreg.second);
                }
            }
            void reset() {
                // Reset quantum registers and quantum state
                for (const auto& qreg : qasm.qregs) {
                    System::remove_register(qreg.first);
                }
            }
            double simulate() {
                // Initialize quantum state
                init();
                std::vector<System> state;
                state.emplace_back();
                // Temporary container for storing consecutive 'h' operations
                std::vector<std::pair<std::string, size_t>> h_positions;
                std::string current_register;

                auto start = std::chrono::high_resolution_clock::now();

                // Apply each operation to the quantum state
                for (const auto& operation : qasm.operations) {
#ifdef GROUP_OF_GATE
                    if (operation.gate == "h") {
                        // Check if this 'h' is on the same register as previous 'h' operations
                        if (h_positions.empty() || operation.targets[0].first == current_register) {
                            // Record the position of the 'h' operation
                            h_positions.emplace_back(operation.targets[0].first, operation.targets[0].second);
                            current_register = operation.targets[0].first;
                        }
                        else {
                            // Different register detected, apply the accumulated 'h' operations
                            std::set<size_t> positions;
                            for (const auto& target : h_positions) {
                                positions.insert(target.second);
                            }
                            Hadamard_PartialQubit(current_register, positions)(state);

                            // Clear the temporary container and start new sequence with current 'h'
                            h_positions.clear();
                            h_positions.emplace_back(operation.targets[0].first, operation.targets[0].second);
                            current_register = operation.targets[0].first;
                        }
                    }
                    else {
                        // If we have consecutive 'h' operations, merge them and apply a single large 'H' operation
                        if (!h_positions.empty()) {
                            // Create a set of all the positions where 'h' needs to be applied
                            std::set<size_t> positions;
                            for (const auto& pos : h_positions) {
                                positions.insert(pos.second);
                            }
                            // Apply a large Hadamard operation
                            Hadamard_PartialQubit(h_positions[0].first, positions)(state);
                            // Clear the temporary container
                            h_positions.clear();
                        }
#else
                    if (operation.gate == "h") {
                        std::set<size_t> qubit_pos = { operation.targets[0].second };
                        Hadamard_PartialQubit(operation.targets[0].first, qubit_pos)(state);
                    }
                    else {
#endif
                        if (operation.gate == "x") {
                            Xgate_Bool(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "y") {
                            Ygate_Bool(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "z") {
                            Zgate_Bool(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "s") {
                            Sgate_Bool(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "sdg") {
                            Sgate_Bool(operation.targets[0].first, operation.targets[0].second).dag(state);
                        }
                        else if (operation.gate == "t") {
                            Tgate_Bool(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "tdg") {
                            Tgate_Bool(operation.targets[0].first, operation.targets[0].second).dag(state);
                        }
                        else if (operation.gate == "sx") {
                            SXgate_Bool(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "sxdg") {
                            SXgate_Bool(operation.targets[0].first, operation.targets[0].second).dag(state);
                        }
                        else if (operation.gate == "cx") {
                            Xgate_Bool(operation.targets[1].first, operation.targets[1].second)
                                .conditioned_by_bit(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "cz") {
                            Zgate_Bool(operation.targets[1].first, operation.targets[1].second)
                                .conditioned_by_bit(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "ccx") {
                            std::vector<std::pair<std::string_view, size_t>> cond_vars = { {operation.targets[0].first, operation.targets[0].second},
                                                                            {operation.targets[1].first, operation.targets[1].second} };
                            Xgate_Bool(operation.targets[2].first, operation.targets[2].second)
                                .conditioned_by_bit(cond_vars)(state);
                        }
                        else if (operation.gate == "phase" || operation.gate == "p") {
                            Phase_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0])(state);
                        }
                        else if (operation.gate == "cphase") {
                            Phase_Bool(operation.targets[1].first, operation.targets[1].second, operation.angles[0])
                                .conditioned_by_bit(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "rx") {
                            RXgate_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0])(state);
                        }
                        else if (operation.gate == "ry") {
                            RYgate_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0])(state);
                        }
                        else if (operation.gate == "rz") {
                            RZgate_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0])(state);
                        }
                        else if (operation.gate == "swap") {
                            Swap_Bool_Bool(operation.targets[0].first, operation.targets[0].second, operation.targets[1].first, operation.targets[1].second)(state);
                        }
                        else if (operation.gate == "cswap") {
                            Swap_Bool_Bool(operation.targets[1].first, operation.targets[1].second, operation.targets[2].first, operation.targets[2].second)
                                .conditioned_by_bit(operation.targets[0].first, operation.targets[0].second)(state);
                        }
                        else if (operation.gate == "u") {
                            U3gate_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0], operation.angles[1], operation.angles[2])(state);
                        }
                        else if (operation.gate == "u1") {
                            Phase_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0])(state);
                        }
                        else if (operation.gate == "u2") {
                            U2gate_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0], operation.angles[1])(state);
                        }
                        else if (operation.gate == "u3") {
                            U3gate_Bool(operation.targets[0].first, operation.targets[0].second, operation.angles[0], operation.angles[1], operation.angles[2])(state);
                        }
                        else {
                            throw std::runtime_error("Unsupported gate operation: " + operation.gate);
                        }
                    }
                }
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> dur = end - start;
                fmt::print("{}\n", profiler::get_all_profiles_v2());
                // Partial trace select qubits to obtain the measurement results
                // (Optional)
                //StatePrint(0, 16)(state);
                SortByAmplitude()(state);
                size_t n = std::min(state.size(), size_t(10));
                std::vector<System> first_ten(state.begin(), state.begin() + n);
                StatePrint(0, 16)(first_ten);
                return dur.count();
            }

        };

        template<typename GateType>
        inline DenseMatrix<complex_t> extract_matrix_from_1q_operation(const GateType& gate)
        {
            static_assert(std::is_base_of<GateBase, GateType>::value, "GateType must be a subclass of GateBase");

            auto id = gate.id;
            size_t nqubit = System::size_of(id);

            DenseMatrix<complex_t> ret(pow2(nqubit));

            for (size_t i = 0; i < pow2(nqubit); i++)
            {
                std::vector<System> state;
                state.emplace_back();
                state.back().get(id).value = i;

                std::vector<complex_t> vec(pow2(nqubit), 0);
                gate(state);
#ifdef SINGLE_THREAD
                for (auto& s : state)
                {
#else
#pragma omp parallel for
                for (int i = 0; i < state.size(); ++i)
                {
                    auto& s = state[i];
#endif
                    size_t _index = s.get(id).value;
                    vec[_index] = s.amplitude;
                }
                for (size_t j = 0; j < pow2(nqubit); j++) {
                    ret(j, i) = vec[j];
                }
            }
            return ret;
        }

        template<typename GateType>
        inline DenseMatrix<complex_t> extract_matrix_from_ctrl_1q_operation(const GateType& gate, std::vector<std::pair<int, size_t>> cond_variables)
        {
            static_assert(std::is_base_of<GateBase, GateType>::value, "GateType must be a subclass of GateBase");

            /* 1.The ctrl order is set to be the same as the order in the QASM file, i.e., the control qubit is the first one.
                    e.g. `ccx`, the tensor order is (I - |11><11|)\otimes I + |11><11| \otimes X. 
               2.The extracted matrix does not include unrelevants qubits, i.e., the qubits either not involved in the gate or not involved in the control.
               */
            auto id = gate.id;
            auto n_ctrl = cond_variables.size();
            size_t nqubit = 1 + n_ctrl;

            DenseMatrix<complex_t> ret(pow2(nqubit));
            size_t k = 0;
            for (size_t i = 0; i < pow2(nqubit); i++)
            {
                std::vector<System> state;
                state.emplace_back();
                k = i & 1;
                if (k == 0)
                    state.back().get(id).value &= ~(1 << gate.digit);
                else
                    state.back().get(id).value |= (1 << gate.digit) ;

                for (size_t j = 0; j < n_ctrl; j++)
                {
                    // set control bits
                    k = (i >> (j + 1)) & 1;
                    if (k == 0)
                        state.back().get(cond_variables[j].first).value &= ~pow2(cond_variables[j].second);
                    else
                        state.back().get(cond_variables[j].first).value |= pow2(cond_variables[j].second);
                }

                std::vector<complex_t> vec(pow2(nqubit), 0);
                gate(state);
#ifdef SINGLE_THREAD
                for (auto& s : state)
                {
#else
#pragma omp parallel for
                for (int i = 0; i < state.size(); ++i)
                {
                    auto& s = state[i];
#endif
                    size_t _index = (s.get(id).value >> gate.digit)  & 1;
                    for (size_t j = 0; j < n_ctrl; j++)
                    {
                        // set control bits
                        k = (s.get(cond_variables[j].first).value >> cond_variables[j].second) & 1;
                        _index += (k << (j + 1)) ;
                    }
                    vec[_index] = s.amplitude;
                }
                for (size_t j = 0; j < pow2(nqubit); j++) {
                    ret(j, i) = vec[j];
                }
            }
            return ret;
        }

        template<typename GateType>
        inline DenseMatrix<complex_t> extract_matrix_from_ctrl_1q_operation(const GateType& gate, std::vector<std::pair<std::string, size_t>> cond_variables)
        {
            static_assert(std::is_base_of<GateBase, GateType>::value, "GateType must be a subclass of GateBase");

            std::vector<std::pair<int, size_t>> cond_variables_int;
            for (const auto& var : cond_variables)
            {
                auto id = System::get(var.first);
                if (id == SIZE_MAX)
                    throw std::runtime_error("Undefined qubit: " + var.first);
                cond_variables_int.emplace_back(id, var.second);
            }
            return extract_matrix_from_ctrl_1q_operation(gate, cond_variables_int);
        }
    }


}


 