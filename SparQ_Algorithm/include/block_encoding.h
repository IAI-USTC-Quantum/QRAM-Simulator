#pragma once
#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include <iostream>
#include <fstream>
#include "matrix.h"
#include <Eigen/Eigen>
#include <filesystem>

// #define OPTIMIZE_HADAM
#define OPTIMIZE_HADAM_INT
#define OPTIMIZE_ROT
// #define OPTIMIZE_PREP

namespace qram_simulator {
	inline void print_complex_vec(const std::vector<complex_t>& zvec, int precision)
	{
		fmt::print("\n");
		for (auto& z : zvec)
		{
			fmt::print("({:.{}f}, {:.{}f}i)  ", z.real(), precision, z.imag(), precision);
		}
		fmt::print("\n");
	}

	inline std::vector<complex_t> get_output(const std::vector<complex_t> &vec, int size)
	{
		int dim = 4 * size; //size is the dimensionality of main_reg space.
		std::vector<complex_t> newVec(dim, complex_t(0.0, 0.0));
		std::copy(vec.begin(), vec.end(), newVec.begin());
		return newVec;
	}

	inline std::vector<double> get_output(const std::vector<double> &vec, int size)
	{
		int dim = 4 * size; //size is the dimensionality of main_reg space.
		std::vector<double> newVec(dim, 0.0);
		std::copy(vec.begin(), vec.end(), newVec.begin());
		return newVec;
	}
}
