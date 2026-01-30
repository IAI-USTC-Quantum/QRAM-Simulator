#include "matrix.h"

namespace qram_simulator
{
	DenseMatrix<complex_t> dagger(const DenseMatrix<complex_t>& mat)
	{
		DenseMatrix<complex_t> ret(mat.size);
		for (size_t i = 0; i < mat.size; ++i)
		{
			for (size_t j = 0; j < mat.size; ++j)
			{
				ret(i, j) = std::conj(mat(j, i));
			}
		}
		return ret;
	}

	std::vector<size_t> SparseMatrix::get_data() const
	{
		std::vector<size_t> data = elements;
		data.insert(data.end(), sparsity.begin(), sparsity.end());
		return data;
	}

	size_t SparseMatrix::get_sparsity_offset() const
	{
		return elements.size();
	}


	double SparseMatrix::get_kappa() const
	{
		double min_eigval = get_min_eigval(to_eigen(), (nnz_col + 1) / 2);
		double kappa = 1.0 / min_eigval;
		return kappa;
	}

	double SparseMatrix::get_j0(double eps) const
	{
		double kappa = get_kappa();
		double b = kappa * kappa * (std::log(kappa) - std::log(eps));
		double j0 = std::sqrt(b * (std::log(4 * b) - std::log(eps)));
		return j0;
	}

	double get_min_eigval(const SparseMatrix& mat)
	{
		Eigen::MatrixXd mat_eigen = sparse2dense(mat).to_eigen() / mat.nnz_col;
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(mat_eigen);
		auto eigval = eigensolver.eigenvalues();
		eigval = eigval.cwiseAbs();
		if (eigensolver.info() != Eigen::Success)
		{
			fmt::print("Eigensolve Failed.\n");
			throw_bad_result();
		}
		if (eigval.maxCoeff() >= 1.00000)
		{
			fmt::print("Absolute Eigen Value >= 1.\n");
			std::cout << "Matrix:\n" << mat_eigen << std::endl;
			std::cout << "Abs(Eigenval):\n" << eigval << std::endl;
			throw_bad_result();
		}

		return eigval.minCoeff();
	}

	double get_min_eigval(const DenseMatrix<double>& mat, size_t l)
	{
		Eigen::MatrixXd mat_eigen = mat.to_eigen() / (2 * l - 1);
		fmt::print("Start Eigen::SelfAdjointEigenSolver\n");
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(mat_eigen);

		fmt::print("Finished Eigen::SelfAdjointEigenSolver\n");
		auto eigval = eigensolver.eigenvalues();
		eigval = eigval.cwiseAbs();
		if (eigensolver.info() != Eigen::Success)
		{
			fmt::print("Eigensolve Failed.\n");
			throw_bad_result();
		}
		if (eigval.maxCoeff() >= 1.10000)
		{
			fmt::print("Absolute Eigen Value >= 1.\n");
			std::cout << "Matrix:\n" << mat_eigen << std::endl;
			std::cout << "Abs(Eigenval):\n" << eigval << std::endl;
			throw_bad_result();
		}

		return eigval.minCoeff();
	}

	double get_min_eigval_from_sparsity(const DenseMatrix<double>& mat, size_t sparsity)
	{
		Eigen::MatrixXd mat_eigen = mat.to_eigen() / (sparsity);
		fmt::print("Start Eigen::SelfAdjointEigenSolver\n");
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(mat_eigen);

		fmt::print("Finished Eigen::SelfAdjointEigenSolver\n");
		auto eigval = eigensolver.eigenvalues();
		eigval = eigval.cwiseAbs();
		if (eigensolver.info() != Eigen::Success)
		{
			fmt::print("Eigensolve Failed.\n");
			throw_bad_result();
		}
		if (eigval.maxCoeff() >= 1.10000)
		{
			fmt::print("Absolute Eigen Value >= 1.\n");
			std::cout << "Matrix:\n" << mat_eigen << std::endl;
			std::cout << "Abs(Eigenval):\n" << eigval << std::endl;
			throw_bad_result();
		}

		return eigval.minCoeff();
	}

	double get_min_eigval(const Eigen::SparseMatrix<double>& mat, size_t l)
	{
		Eigen::SelfAdjointEigenSolver<Eigen::SparseMatrix<double>> eigensolver(mat /  (2 * l - 1));
		auto eigval = eigensolver.eigenvalues();
		eigval = eigval.cwiseAbs();
		if (eigensolver.info() != Eigen::Success)
		{
			fmt::print("Eigensolve Failed.\n");
			throw_bad_result();
		}
		if (eigval.maxCoeff() >= 1.10000)
		{
			fmt::print("Absolute Eigen Value >= 1.\n");
			// std::cout << "Matrix:\n" << mat_eigen << std::endl;
			std::cout << "Abs(Eigenval):\n" << eigval << std::endl;
			throw_bad_result();
		}

		return eigval.minCoeff();
	}


	SparseMatrix generate_simplest_sparse_matrix_unsigned_0()
	{
		std::vector<size_t> elements = { 18, 28, 13, 4 };
		std::vector<size_t> sparsity = { 0, 1, 2, 3 };

		return SparseMatrix(
			elements, sparsity,
			5 /* data_size : to represent 0~3 */,
			1 /* nnz_col : 2 elements per column */,
			4 /* n_row : 4 rows */
		);

	}

	SparseMatrix generate_simplest_sparse_matrix_unsigned_1()
	{
		/*
		*/


		std::vector<size_t> elements = { 1, 2, 2, 3, 2, 3, 0, 1 };
		std::vector<size_t> sparsity = { 0, 2, 1, 2, 0, 1, 0, 3 };

		return SparseMatrix(
			elements, sparsity,
			6 /* data_size : to represent 0~3 */,
			2 /* nnz_col : 2 elements per column */,
			4 /* n_row : 4 rows */
		);

	}

	SparseMatrix generate_simplest_sparse_matrix_unsigned_2()
	{
		/*
		*/


		std::vector<int64_t> elements = { 1, 3, 3, 12, 1, 3, 3, 1 };
		std::vector<size_t> sparsity = { 0, 1, 0, 1, 2, 3, 2, 3 };

		return SparseMatrix(
			elements, sparsity,
			8 /* data_size : to represent 0~3 */,
			2 /* nnz_col : 2 elements per column */,
			4 /* n_row : 4 rows */
		);

	}

	SparseMatrix generate_simplest_sparse_matrix_signed_0()
	{
		/*
		*/


		std::vector<int64_t> elements = { 1, -4, -4, 3, 7, -1, -1, 1 };
		std::vector<size_t> sparsity = { 0, 1, 0, 1, 2, 3, 2, 3 };

		return SparseMatrix(
			elements, sparsity,
			4 /* data_size : to represent -8~7 */,
			2 /* nnz_col : 2 elements per column */,
			4 /* n_row : 4 rows */
		);

	}

	SparseMatrix generate_simplest_sparse_matrix_signed_1()
	{
		/*
		1 0 2 0
		0 2 3 0
		2 3 0 0
		0 0 0 1

		elements = {1, 2, 2, 3, 2, 3, 0, 1}
		sparsity = {0, 2, 1, 2, 0, 1, 0, 3}
		*/


		std::vector<size_t> elements = { 1, 2, 2, 3, 2, 3, 0, 1 };
		std::vector<size_t> sparsity = { 0, 2, 1, 2, 0, 1, 0, 3 };

		return SparseMatrix(
			elements, sparsity,
			6 /* data_size  */,
			2 /* nnz_col : 2 elements per column */,
			4 /* n_row : 4 rows */
		);

	}

	//SparseMatrix random_generate_sparse_matrix(size_t n_row, size_t nnz, size_t data_range)
	//{
	//	/*
	//	 3  3  0  0
	//	 3  1  0  0
	//	 0  0  1  3
	//	 0  0  3  1

	//	elements = {1, 3, 3, 1, 1, 3, 3, 1}
	//	sparsity = {0, 1, 0, 1, 2, 3, 2, 3}
	//	*/


	//	std::vector<size_t> elements = { 1, 3, 3, 2, 6, 4, 4, 1 };
	//	std::vector<size_t> sparsity = { 0, 1, 0, 1, 2, 3, 2, 3 };

	//	return SparseMatrix(
	//		elements, sparsity,
	//		UnsignedInteger /* data_type */,
	//		4 /* elem_size : to represent 0~3 */,
	//		2 /* nnz_col : 2 elements per column */,
	//		4 /* n_row : 4 rows */
	//	);
	//}

	SparseMatrix get_test_matrix()
	{
		FILE* fp1 = fopen("../../../test/elements.txt", "r");
		FILE* fp2 = fopen("../../../test/sparsity.txt", "r");
		if (!fp1 || !fp2)
		{
			throw_invalid_input();
		}

		size_t nrow = 8192;
		size_t ncol = 128;
		size_t max_pos;
		double maxabs = std::numeric_limits<double>::lowest();
		std::vector<size_t> columns(nrow * ncol);
		std::vector<double> elements(nrow * ncol);
		std::vector<int64_t> elements_real(nrow * ncol);
		size_t k = 0;
		for (size_t i = 0; i < nrow; ++i)
		{
			for (size_t j = 0; j < ncol; ++j)
			{
				double vj;
				size_t colj;
				fscanf(fp1, "%lf", &vj);
				fscanf(fp2, "%llu", &colj);
				elements[k] = vj;
				columns[k] = colj;
				if (std::abs(vj) >= maxabs) {
					max_pos = k;
					maxabs = std::abs(vj);
				}
				k++;
			}
			if (!check_unique_sort(
				columns.begin() + i * ncol,
				columns.begin() + (i + 1) * ncol))
				throw_general_runtime_error();
		}

		fclose(fp1);
		fclose(fp2);

		size_t max_digit = 32;

		//for (size_t i = 0; i < nrow; ++i)
		//{
		//	for (size_t j = 0; j < ncol; ++j)
		//	{
		//		printf("(%d, %d) %lf\n", i, columns[i][j], elements[i][j]);
		//	}
		//	printf("\n");
		//}
		fmt::print("nrow = {}\n", nrow);
		fmt::print("nnz = {}\n", ncol);

		for (size_t i = 0; i < elements.size(); ++i)
		{
			int64_t eint = static_cast<int64_t>(elements[i] * (pow2(max_digit) / maxabs));
			elements_real[i] = make_complement(eint, max_digit);
		}

		return SparseMatrix(
			elements_real, columns,
			max_digit /* data_size : to represent sufficient large elements */,
			128 /* nnz_col */,
			8192 /* n_row */
		);
	}

	std::vector<complex_t> get_target_result()
	{
		FILE* fp = fopen("../../../test/target_result.txt", "r");
		if (!fp)
		{
			throw_invalid_input();
		}
		size_t nrow = 8192;
		std::vector<complex_t> elements(nrow);
		for (size_t i = 0; i < nrow; ++i)
		{
			double vj;
			fscanf(fp, "%lf", &vj);
			elements[i] = vj;
		}
		return elements;
		fclose(fp);
	}

	SparseMatrix random_band_sparse_signed(size_t row_size, size_t l, size_t elem_size)
	{
		auto mat = generate_band_mat(row_size, l);
		std::vector<int64_t> elements;
		std::vector<size_t> sparsity;
		size_t nnz_col = 2 * l - 1;
		if (l > 1)
			nnz_col = pow2(log2(nnz_col) + 1);

		if (nnz_col >= row_size)
			throw_invalid_input();

		for (size_t i = 0; i < l; ++i)
		{
			for (size_t j = 0; j < nnz_col; ++j)
			{
				elements.push_back(static_cast<int64_t>(std::floor(mat(i, j) * pow2(elem_size - 1))));
				sparsity.push_back(j);
			}
		}
		for (size_t i = l; i < row_size - l; ++i)
		{
			for (size_t j = i - l + 1; j < i - l + 1 + nnz_col; ++j)
			{
				elements.push_back(static_cast<int64_t>(std::floor(mat(i, j) * pow2(elem_size - 1))));
				sparsity.push_back(j);
			}
		}
		for (size_t i = row_size - l; i < row_size; ++i)
		{
			for (size_t j = row_size - nnz_col; j < row_size; ++j)
			{
				elements.push_back(static_cast<int64_t>(std::floor(mat(i, j) * pow2(elem_size - 1))));
				sparsity.push_back(j);
			}
		}

		return SparseMatrix(elements, sparsity, elem_size, nnz_col, mat.size);
	}

	SparseMatrix dense2sparse_band_unsigned(const DenseMatrix<double>& mat, size_t l, size_t elem_size)
	{
		std::vector<uint64_t> elements;
		std::vector<size_t> sparsity;
		size_t row_size = mat.size;
		size_t nnz_col = 2 * l - 1;
		elements.reserve(row_size * nnz_col);
		sparsity.reserve(row_size * nnz_col);

		if (l > 1)
			nnz_col = pow2(log2(nnz_col) + 1);

		if (nnz_col > row_size)
			throw_invalid_input();

		for (size_t i = 0; i < l; ++i)
		{
			for (size_t j = 0; j < nnz_col; ++j)
			{
				elements.push_back(static_cast<uint64_t>(std::floor(mat(i, j) * pow2(elem_size))));
				sparsity.push_back(j);
			}
		}
		for (size_t i = l; i < row_size - l; ++i)
		{
			for (size_t j = i - l + 1; j < i - l + 1 + nnz_col; ++j)
			{
				elements.push_back(static_cast<uint64_t>(std::floor(mat(i, j) * pow2(elem_size))));
				sparsity.push_back(j);
			}
		}
		for (size_t i = row_size - l; i < row_size; ++i)
		{
			for (size_t j = row_size - nnz_col; j < row_size; ++j)
			{
				elements.push_back(static_cast<uint64_t>(std::floor(mat(i, j) * pow2(elem_size))));
				sparsity.push_back(j);
			}
		}

		return SparseMatrix(
			elements, 
			sparsity, 
			elem_size,
			nnz_col, 
			mat.size
		);
	}

	double get_kappa(const DenseMatrix<double>& mat)
	{
		double min_eigval = get_min_eigval(mat, 1);
		double kappa = std::ceil(1.0 / min_eigval);
		return kappa;
	}

	double get_kappa_general(const DenseMatrix<double>& mat)
	{
		// fmt::print("mat_A:\n{}\n", mat.to_string());
		Eigen::MatrixXd mat_svd = mat.to_eigen();
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(mat_svd, Eigen::ComputeThinU | Eigen::ComputeThinV);
		size_t row_size = svd.singularValues().size();
		// fmt::print("singular values: {}\n", svd.singularValues());
		double maxSingular = svd.singularValues()(0);
		double minSingular = svd.singularValues()(row_size - 1);
		if (minSingular == 0) {
			fmt::print("Matrix is singular or nearly singular!\n");
			throw_bad_result();
		}
		return maxSingular / minSingular;
	}

	double get_kappa_Tridiagonal(double alpha, double beta, const size_t size)
	{
		double t = cos(pi / (size + 1));
		if (abs(alpha) < 2 * abs(beta) * t) {
			throw_general_runtime_error(
				fmt::format("This formula is not suitable for this matrix! (alpha={}, beta={})",
					alpha, beta)
			);
		}
		double kappa = (abs(alpha) + 2 * abs(beta) * t) / (abs(alpha) - 2 * abs(beta) * t);
		return kappa;
	}

	void normalize_column(DenseMatrix<complex_t>& mat, size_t col_id) {
		double norm = 0.0;
		for (size_t i = 0; i < mat.size; ++i) {
			norm += abs_sqr(mat(i, col_id));
		}
		norm = std::sqrt(norm);
		//fmt::print("norm of {} is {}\n", col_id, norm);
		for (size_t i = 0; i < mat.size; ++i) {
			mat(i, col_id) /= norm;
		}
	}

	void gram_schmidt_process(DenseMatrix<complex_t>& ret) {
		/* Apply the Gram-Schmidt process to the matrix ret

		Return: A matrix ret whose columns are orthogonal to each other.
		Note: This function modifies the input matrix ret.

		*/
		size_t N = ret.size;
		normalize_column(ret, 0);
		for (size_t j = 1; j < N; ++j) {
			for (size_t i = 0; i < j; ++i) { // Orthogonalize with respect to all previous columns
				complex_t proj_coeff = 0.0;
				for (size_t k = 0; k < N; ++k) {
					proj_coeff += std::conj(ret(k, j)) * ret(k, i);
				}
				for (size_t k = 0; k < N; ++k) {
					ret(k, j) -= proj_coeff * ret(k, i);
				}
			}
			normalize_column(ret, j);
		}
	}
}