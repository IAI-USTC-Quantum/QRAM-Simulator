#include "sparse_state_simulator.h"
#include "global_macros.h"
#include "qft.h"
#include <iostream>
#include <filesystem> // to get the current directory
#include "math.h"
#include <argparse.h>
//#define USE_ARGPARSE


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

std::chrono::duration<double> QFT_Full_Test(size_t nqubit)
{
    std::vector<System> state;
    state.reserve(200000);
    auto start = std::chrono::high_resolution_clock::now();

    System::add_register("main", UnsignedInteger, nqubit);
    state.emplace_back();
    //StatePrint()(state);
    Init_Unsafe("main", 1)(state);
    QFT_Full("main")(state);
    QFT_Full("main")(state);
    QFT_Full("main").dag(state);
    SortUnconditional()(state);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> running_time = end - start;

    System::clear();
    fmt::print("{}\n", profiler::get_all_profiles_v2());
    profiler::init_profiler();
    return running_time;
}


std::chrono::duration<double> QFT_Test(size_t nqubit)
{
    std::vector<System> state;
    state.reserve(200000);
    auto start = std::chrono::high_resolution_clock::now();

    System::add_register("main", UnsignedInteger, nqubit);
    state.emplace_back();
    StatePrint()(state);
    Init_Unsafe("main", 1)(state);
    QFT("main")(state);
    QFT("main")(state);
    inverseQFT("main")(state);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> running_time = end - start;

    System::clear();
    fmt::print("{}\n", profiler::get_all_profiles_v2());
    profiler::init_profiler();
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
    if (nqubit < 2 || nqubit > 64) {
        fmt::print("Number of qubits must be greater than or equal to 2. And less than or equal to 64.\n");
        return 2;
    }
    std::vector<size_t> nq_list = { nqubit };
#else
    std::vector<size_t> nq_list = { 13 };
#endif

    std::filesystem::path current_path = std::filesystem::current_path();
    std::string filename = current_path.string() + "\\qft_running_info_ssc.csv";
    fmt::print("{}\n", filename);

    std::ifstream isfile(filename);
    if (!isfile) {
        std::ofstream file(filename, std::ios::out);
        file << "nqubit,running time,peak memory" << std::endl;
    }

    std::chrono::duration<double> running_time;

    for (auto nqubit : nq_list) {
        
        running_time = QFT_Test(nqubit);

        fmt::print("QFT_Test Running time for {} qubits: {} seconds\n", nqubit, running_time.count());

        running_time = QFT_Full_Test(nqubit);

        fmt::print("QFT_Full_Test Running time for {} qubits: {} seconds\n", nqubit, running_time.count());

        // Write running information to file 
        std::ofstream file(filename, std::ios::app);
        file << fmt::format("{},{}\n", nqubit, running_time.count());
        file.close();
    }

    return 0;
}