#include "qasm_parser.h"
#include "sparse_state_simulator.h"
#include "matrix.h"
#include <argparse.h>

using namespace std;
using namespace qram_simulator;
using namespace qasm_parser;


int testParser(std::string filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open QASM file.");
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        QasmParser parser;
        Qasm qasm = parser.parse(content);

        // Output parsed QASM information
        std::cout << "\nQuantum Registers:" << std::endl;
        for (const auto& qreg : qasm.qregs) {
            std::cout << qreg.first << " qubit number: " << qreg.second << std::endl;
        }

        std::cout << "\nClassical Registers:" << std::endl;
        for (const auto& creg : qasm.cregs) {
            std::cout << creg.first << " qubit number: " << creg.second << std::endl;
        }

        std::cout << "\nGate Operations:" << std::endl;
        for (const auto& op : qasm.operations) {
            std::cout << op.gate << " ";
            for (const auto& param : op.angles) {
                std::cout << param << " ";
            }
            for (const auto& target : op.targets) {
                std::cout << target.first << "[" << target.second << "] ";
            }
            std::cout << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
    }

    return 1;
}

double testSimulator(std::string filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open QASM file.");
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        QasmParser parser;
        Qasm parsed_qasm = parser.parse(content);
        fmt::print("{}\n", parsed_qasm.qregs);
        QasmSimulator sim(parsed_qasm);
        auto time = sim.simulate();
        //StatePrint(0, 16)(state);
        sim.reset();
        return time;
    }
    catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
    }
    return 0;
}


int testGate(size_t nqubit, size_t digit) {
    // this is not finished yet because we choose to implement the gate without `extract_matrix` method;

    std::string reg = "circ";
    System::add_register(reg, UnsignedInteger, nqubit);

    auto gate = Y_Bool("circ", digit);
    DenseMatrix mat = gate.extract_matrix();
    ////gate.display();
    fmt::print("Extracted Matrix:\n{}\n", mat.to_string());

    System::remove_register(reg);
    return 1;
}

int testGate1Q(size_t nqubit, size_t digit)
{
    std::string reg = "circ";
    System::add_register(reg, UnsignedInteger, nqubit);

    //auto gate = Phase_Bool("circ", digit, pi/2);
    //auto gate = Y_Bool("circ", digit);
    //auto gate = Z_Bool("circ", digit);
    //auto gate = S_Bool("circ", digit);
    //auto gate = T_Bool("circ", digit);
    //auto gate = RX_Bool("circ", digit, pi/8);
    //auto gate = RY_Bool("circ", digit, pi/2);
    //auto gate = RZ_Bool("circ", digit, pi/2);
    //auto gate = SX_Bool("circ", digit);
    //auto gate = U2_Bool("circ", digit, pi/2, pi/2);
    auto gate = U3_Bool("circ", digit, pi/2, pi/2, pi/2);
    DenseMatrix mat = extract_matrix_from_1q_operation(gate);
    
    fmt::print("Extracted Matrix:\n{}\n", mat.to_string());
    //gate.display();
    System::remove_register(reg);
    return 1;
}



int testGateMCQ(std::string gate_type, size_t nqubit, std::vector<size_t> digits)
{
    std::string main_reg = "circ";
    System::add_register(main_reg, UnsignedInteger, nqubit);
    fmt::print("Add register: {}\n", main_reg);

    // Check if digits elements are valid
    if (digits.size() > nqubit) {
        throw std::runtime_error("Too many digits for the number of qubits.");
    }
    std::set<size_t> uniqueElements(digits.begin(), digits.end());
    if (uniqueElements.size() != digits.size()) {
        throw std::runtime_error("Digits are not unique.");
    }   
    std::vector<std::pair<std::string, size_t>> cond_variables;
    for (int i = 0; i < digits.size() - 1; i++)
    {
        // digits[-1] is for the target qubit
        cond_variables.push_back({ main_reg, digits[i]});
    }
    
    DenseMatrix<complex_t> mat(pow2(digits.size()));
    if (gate_type == "Phase_Bool")
    {
        Phase_Bool gate(main_reg, digits.back(), pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "Y_Bool")
    {
        Y_Bool gate(main_reg, digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "Z_Bool")
    {
        Z_Bool gate(main_reg, digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "S_Bool")
    {
        S_Bool gate(main_reg, digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "T_Bool")
    {
        T_Bool gate(main_reg, digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "Rot_Bool")
    {
        u22_t u22 = {cos(pi/4), -sin(pi/4), sin(pi/4), cos(pi/4)};
        Rot_Bool gate(main_reg, digits.back(), u22);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "RX_Bool")
    {
        RX_Bool gate(main_reg, digits.back(), pi / 8);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "RY_Bool")
    {
        RY_Bool gate(main_reg, digits.back(), pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "RZ_Bool")
    {
        RZ_Bool gate(main_reg, digits.back(), pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "SX_Bool")
    {
        SX_Bool gate(main_reg, digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "U2_Bool")
    {
        U2_Bool gate(main_reg, digits.back(), pi / 2, pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "U3_Bool")
    {
        U3_Bool gate(main_reg, digits.back(), pi / 4, pi /4, pi / 4);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else
    {
        throw std::runtime_error("Invalid gate type.");
    }
    fmt::print("Extracted Matrix:\n{}\n", mat.to_string());
    
    System::remove_register(main_reg);
    return 1;
}

int testGateMCQ(std::string gate_type, std::vector<std::string> qregs,
    std::vector<size_t> nqubits, std::vector<size_t> digits)
{
    // Check if digits elements are valid
    if (qregs.size() != nqubits.size()) {
        throw std::runtime_error("qregs and nqubits must have the same size.");
    }
    std::set<size_t> uniqueElements(digits.begin(), digits.end());
    if (uniqueElements.size() != digits.size()) {
        throw std::runtime_error("Digits are not unique.");
    }
    // define control and target qubits
    std::vector<std::pair<std::string, size_t>> cond_variables;
    
    for (int i = 0; i < qregs.size() - 1; i++)
    {
        System::add_register(qregs[i], UnsignedInteger, nqubits[i]);
        cond_variables.push_back({ qregs[i], digits[i]});
    }
    System::add_register(qregs.back(), UnsignedInteger, nqubits.back());

    DenseMatrix<complex_t> mat(pow2(digits.size()));
    if (gate_type == "Phase_Bool")
    {
        Phase_Bool gate(qregs.back(), digits.back(), pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "Y_Bool")
    {
        Y_Bool gate(qregs.back(), digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "Z_Bool")
    {
        Z_Bool gate(qregs.back(), digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "S_Bool")
    {
        S_Bool gate(qregs.back(), digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "T_Bool")
    {
        T_Bool gate(qregs.back(), digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "Rot_Bool")
    {
        u22_t u22 = {cos(pi/4), -sin(pi/4), sin(pi/4), cos(pi/4)};
        Rot_Bool gate(qregs.back(), digits.back(), u22);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "RX_Bool"){
        RX_Bool gate(qregs.back(), digits.back(), pi / 8);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "RY_Bool")
    {
        RY_Bool gate(qregs.back(), digits.back(), pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "RZ_Bool")
    {
        RZ_Bool gate(qregs.back(), digits.back(), pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "SX_Bool")
    {
        SX_Bool gate(qregs.back(), digits.back());
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "U2_Bool")
    {
        U2_Bool gate(qregs.back(), digits.back(), pi / 2, pi / 2);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else if (gate_type == "U3_Bool")
    {
        U3_Bool gate(qregs.back(), digits.back(), pi / 4, pi /4, pi / 4);
        gate.conditioned_by_bit(cond_variables);
        mat = extract_matrix_from_ctrl_1q_operation(gate, cond_variables);
        //gate.display();
    }
    else
    {
        throw std::runtime_error("Invalid gate type.");
    }
    fmt::print("Extracted Matrix:\n{}\n", mat.to_string());
    
    for (int i = 0; i < qregs.size(); i++)
    {
        System::remove_register(qregs[i]);
    }
    return 1;
}

int metaGateTest() {
    std::vector<std::string> gate_types = {"Phase_Bool", "Rot_Bool", "Y_Bool", "Z_Bool", 
                                        "S_Bool", "T_Bool", "RX_Bool", "RY_Bool",
                                         "RZ_Bool", "SX_Bool", "U2_Bool", "U3_Bool",
                                        };
    for (auto gate_type : gate_types)
    {
        testGateMCQ(gate_type, 4, {  1, 0 });
    }
    for (auto gate_type : gate_types)
    {
        testGateMCQ(gate_type, { "ctrl", "main" }, { 2, 2 }, { 1, 0 });
    }
    return 1;
}


struct QasmTestArguments
{
    std::string qasm_file;
};

inline QasmTestArguments argument_parse(int argc, const char** argv)
{
    QasmTestArguments args;
    argparse::ArgumentParser parser("qasm simulator test", "Construct quantum circuits with qasm and simulate them.");

    parser.add_argument()
        .names({ "-i", "--input" })
        .description("input qasm file")
        .required(false);

    auto err = parser.parse(argc, argv);

    if (err) {
        std::cout << err << std::endl;
        std::exit(2);
        return args;
    }
    args.qasm_file = parser.get<std::string>("input");
    return args;
}

int main(int argc, const char** argv) {
    std::string filename = "C:\\Users\\RGZN090201\\Documents\\GitHub\\Quantum-Sparse-State-Calculator\\test\\CPUTest\\QASMTest\\qasm_files\\knn_n25.qasm";
    QasmTestArguments args;
    if (argc == 1)
    {
        const char* test_argv[] = {
            ".",
            "--input", filename.c_str(),
        };
        argc = array_length<decltype(test_argv)>::value;
        argv = test_argv;
    }
    args = argument_parse(argc, argv);
    //testParser(args.qasm_file);
    double time  = testSimulator(args.qasm_file);
    //fmt::print("running time : {} s\n", time);
    //metaGateTest();
    //metaSimTest();
    return 0;
}   
