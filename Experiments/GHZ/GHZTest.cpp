#include "sparse_state_simulator.h"
#include "global_macros.h"
#include <iostream>
#include <filesystem> // to get the current directory
#include "math.h"
#include <argparse.h>
#define USE_ARGPARSE


using namespace std;
using namespace qram_simulator;

void memory_test() {
    // Calculate number of doubles needed for ~10 MB
    size_t numDoubles = (10 * 1024 * 1024) / sizeof(double); // ~10 MB

    // Allocate memory
    std::vector<double> data(numDoubles);

    // Optional: Initialize the vector with some values
    for (size_t i = 0; i < numDoubles; ++i) {
        data[i] = static_cast<double>(i); // Example initialization
    }
}

std::chrono::duration<double> GHZTest(size_t nqubit)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<System> state;
    state.emplace_back();

    //The max size of a register is 64 bits
    size_t quotient = (nqubit - 1) / 64;
    size_t remainder = (nqubit - 1) % 64;
    
    System::add_register("ctrl", UnsignedInteger, 1);
    std::vector<std::string> reg_names;
    for (size_t k = 1; k < quotient + 1; k++)
    {
        std::string reg = "main" + std::to_string(k);
        reg_names.push_back(reg);
        System::add_register(reg, UnsignedInteger, remainder);
    }

    if (remainder!= 0)
    {
        System::add_register("main", UnsignedInteger, remainder);
    }
    
    FlipBools("ctrl")(state);
    Hadamard_Bool("ctrl")(state);
    for (int i = 1; i < remainder + 1; i++)
    {
        Xgate_Bool("main", i-1).conditioned_by_all_ones("ctrl")(state);
    }
    for (auto reg : reg_names) {
        FlipBools(System::get(reg)).conditioned_by_all_ones("ctrl")(state);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> running_time= end - start;

    StatePrint()(state);
    System::remove_register("ctrl");
    for (auto reg: reg_names) {
        System::remove_register(reg);
    }
    if (remainder != 0)
    {
        System::remove_register("main");
    }
    return running_time;
}


int main(int argc, char* argv[])
{
#ifdef USE_ARGPARSE
    if (argc != 2) {
        fmt::print("Usage: {} <nqubits>\n", argv[0]);
        return 1;
    }
    size_t nqubit = std::stoul(argv[1]);
    if (nqubit < 2) {
        fmt::print("Number of qubits must be greater than or equal to 2\n");
        return 2;
    }
    std::vector<size_t> nq_list = { nqubit };
#else
    std::vector<size_t> nq_list = { 5, 10, 15, 20, 25, 30, 300 };
#endif
    
    std::filesystem::path current_path = std::filesystem::current_path();
    std::string filename = current_path.string() + "\\ghz_running_info_ssc.csv";
    fmt::print("{}\n", filename);

    std::ifstream isfile(filename);
    if (!isfile) {
        std::ofstream file(filename, std::ios::out);
        file << "nqubit,running time,peak memory" << std::endl;
    }
    
    std::chrono::duration<double> running_time;
    
    for (auto nqubit : nq_list) {
        // Reset peak memory usage before each test
        running_time = GHZTest(nqubit);
       
        fmt::print("Running time for {} qubits: {} seconds\n", nqubit, running_time.count());

        // Write running information to file 
        std::ofstream file(filename, std::ios::app);
        file << fmt::format("{},{}\n", nqubit, running_time.count());
        file.close();
    }
    return 0;
}