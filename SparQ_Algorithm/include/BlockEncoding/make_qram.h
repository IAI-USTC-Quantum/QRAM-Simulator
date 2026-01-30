#pragma once
#include "sparse_state_simulator.h"
#include "qram_circuit_qutrit.h"
#include <iostream>
#include <fstream>
#include "matrix.h"
#include <Eigen/Eigen>
#include <filesystem>

namespace qram_simulator {
	inline std::vector<double> get_column_flatten(const std::vector<double>& row_vec)
	{
		size_t size = row_vec.size();
		size_t n = static_cast<size_t>(std::sqrt(size));
		if (n * n != size) {
			throw_general_runtime_error();
		}

		std::vector<double> col_vec(size);
		for (size_t i = 0; i < n; ++i) {
			for (size_t j = 0; j < n; ++j) {
				col_vec[j * n + i] = row_vec[i * n + j];
			}
		}
		return col_vec;
	}

	inline std::vector<uint64_t> scaleAndConvertVector(const std::vector<double>& input_vec, int exponent,
		size_t data_size, bool from_matrix = true)
	{
		std::vector<double> col_vec;
		// matrix transpose
		if (from_matrix) {
			col_vec = get_column_flatten(input_vec);
		}
		else {
			col_vec = input_vec;
		}
		// calculate the scale number
		double scale = std::pow(2.0, exponent);
		std::vector<uint64_t> outputVec;
		outputVec.reserve(col_vec.size());

		for (double value : col_vec) {
			// rescale and convert to unsigned __int64
			size_t scaledValue = make_complement(static_cast<int64_t>(std::llround(value * scale)), data_size);
			outputVec.push_back(scaledValue);
		}

		return outputVec;
	}

	inline std::vector<uint64_t> scaleAndConvertVector(const DenseVector<double>& input_vec, int exponent,
		size_t data_size)
	{
		const std::vector<double> &col_vec = input_vec.data;
		double scale = std::pow(2.0, exponent);
		std::vector<uint64_t> outputVec;
		outputVec.reserve(col_vec.size());
		for (double value : col_vec) {
			// rescale and convert to unsigned __int64
			size_t scaledValue = make_complement(static_cast<int64_t>(std::llround(value * scale)), data_size);
			outputVec.push_back(scaledValue);
		}

		return outputVec;
	}

	inline std::vector<uint64_t> scaleAndConvertVector(const DenseMatrix<double>& input_vec, int exponent,
		size_t data_size)
	{
		std::vector<double> col_vec = get_column_flatten(input_vec.data);
		double scale = std::pow(2.0, exponent);
		std::vector<uint64_t> outputVec;
		outputVec.reserve(col_vec.size());
		for (double value : col_vec) {
			// rescale and convert to unsigned __int64
			size_t scaledValue = make_complement(static_cast<int64_t>(std::llround(value * scale)), data_size);
			outputVec.push_back(scaledValue);
		}

		return outputVec;
	}

	inline std::vector<uint64_t> make_vector_tree(const std::vector<uint64_t>& dist, size_t data_size) {
		size_t dist_sz = dist.size();
		std::vector<uint64_t> temp_tree = dist;
		std::vector<uint64_t> tree;

		do {
			std::vector<uint64_t> temp;
			temp.reserve(dist_sz / 2);

			for (size_t i = 0; i < dist_sz; i += 2) {
				if (i + 1 < dist_sz) { // avoid overflow
					if (dist_sz == dist.size()) {
						// the leaf nodes，calculated with get_complement
						temp.push_back(
							get_complement(temp_tree[i], data_size) * get_complement(temp_tree[i], data_size) +
							get_complement(temp_tree[i + 1], data_size) * get_complement(temp_tree[i + 1], data_size)
						);
					}
					else {
						// other nodes, sum directly.
						temp.push_back(temp_tree[i] + temp_tree[i + 1]);
					}
				}
			}

			// combine all nodes
			temp.insert(temp.end(), temp_tree.begin(), temp_tree.end());
			temp_tree = std::move(temp);

		} while ((dist_sz = (dist_sz + 1) / 2) > 1); //update dist_sz to match the layers.
		temp_tree.push_back(0);
		tree.insert(tree.end(), temp_tree.begin(), temp_tree.end());
		return tree;
	}
}