#pragma once

#include "basic.h"
#include "Eigen/Eigen"
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "Eigen/SVD"

namespace qram_simulator
{	
	template<typename Ty = complex_t> using EigenMat = Eigen::MatrixX<Ty>;
	template<typename Ty = complex_t> using EigenVec = Eigen::VectorX<Ty>;

	// SparseMatrix data storage
	struct SparseMatrix
	{
		// data storage
		std::vector<size_t> elements;
		std::vector<size_t> sparsity;

		/* data[n_row * nnz_col], sparsity[n_row * nnz_col] */
		std::vector<size_t> get_data() const;

		size_t get_sparsity_offset() const;

		// metadata
		// StateStorageType data_type; // type of data (seems unnecessary)

		bool positive_only = false; // whether the matrix only contains positive values
		size_t data_size = 0; // size of each element (word length)
		size_t nnz_col = 0;   // number of nonzeros in a column
		size_t n_row = 0;     // number of rows / columns

		SparseMatrix() {}

		SparseMatrix(const std::vector<uint64_t>& elements_, const std::vector<size_t>& sparsity_,
			size_t data_size, size_t nnz_col_, size_t n_row_)
			: sparsity(sparsity_), positive_only(true),
			data_size(data_size), nnz_col(nnz_col_), n_row(n_row_)
		{
			// check range
			elements.resize(elements_.size());

			size_t max_value = pow2(data_size) - 1;

			for (size_t i = 0; i < elements_.size(); ++i)
			{
				if (elements_[i] > max_value)
					throw_invalid_input("SparseMatrix element must be less than 2^data_size");

				elements[i] = elements_[i];
			}
		}

		SparseMatrix(const std::vector<int64_t>& elements_, const std::vector<size_t>& sparsity_,
			size_t data_size, size_t nnz_col_, size_t n_row_)
			: sparsity(sparsity_), positive_only(false),
			data_size(data_size), nnz_col(nnz_col_), n_row(n_row_)
		{
			elements.resize(elements_.size());

			int64_t max_value = pow2(data_size - 1) - 1;
			int64_t min_value = -static_cast<int64_t>(pow2(data_size - 1));

			for (size_t i = 0; i < elements_.size(); ++i)
			{
				if (elements_[i] < min_value || elements_[i] > max_value)
					throw_invalid_input("SparseMatrix element must be between -2^(data_size-1) and 2^(data_size-1)-1");

				elements[i] = make_complement(elements_[i], data_size);
			}
		}

		SparseMatrix(const std::vector<double>& elements_, const std::vector<size_t>& sparsity_,
			size_t data_size, size_t nnz_col_, size_t n_row_, bool positive_only)
			: sparsity(sparsity_), positive_only(positive_only),
			data_size(data_size), nnz_col(nnz_col_), n_row(n_row_)
		{

			elements.resize(elements_.size());
			for (size_t i = 0; i < elements_.size(); ++i)
			{
				if (std::abs(elements_[i]) > 1.0)
					throw_invalid_input("SparseMatrix element must be between -1 and 1");
				if (positive_only)
				{
					if (elements_[i] < 0)
						throw_invalid_input("SparseMatrix element must be positive");

					elements[i] = static_cast<size_t>(elements_[i] * pow2(data_size));
				}
				else
				{
					int64_t value = static_cast<int64_t>(elements_[i] * pow2(data_size - 1));
					elements[i] = make_complement(value, data_size);
				}
			}
		}

		double get_kappa() const;
		double get_j0(double eps) const;

		Eigen::SparseMatrix<double> to_eigen() const
		{
			Eigen::SparseMatrix<double> ret(n_row, n_row);

			if (positive_only) {
				for (size_t row_id = 0; row_id < n_row; ++row_id)
				{
					for (size_t j = 0; j < nnz_col; ++j)
					{
						size_t col_id = sparsity[row_id * nnz_col + j];
						size_t col_value = elements[row_id * nnz_col + j];

						ret.insert(row_id, col_id) = (col_value * 1.0) / (pow2(data_size));
					}
				}
			}
			else
			{
				for (size_t row_id = 0; row_id < n_row; ++row_id)
				{
					for (size_t j = 0; j < nnz_col; ++j)
					{
						size_t col_id = sparsity[row_id * nnz_col + j];
						size_t col_value = elements[row_id * nnz_col + j];

						int64_t real_value = get_complement(col_value, data_size);

						ret.insert(row_id, col_id) = (real_value * 1.0) / (pow2(data_size - 1));
					}
				}
			}
			return ret;
		}

	};

	template<typename Ty> struct DenseMatrix;
	template<typename Ty> struct DenseVector;

	template<typename Ty>
	struct DenseMatrix {
		size_t size;
		/*Ty* data;*/
		std::vector<Ty> data;

		explicit DenseMatrix()
			: size(0)
		{
		}

		explicit DenseMatrix(size_t size_)
			: size(size_), data(size_* size_, 0)
		{
		}

		explicit DenseMatrix(size_t size_, const std::vector<Ty>& initial_data)
			: size(size_), data(initial_data)
		{
		}

		template<typename Ty2>
		explicit DenseMatrix(const DenseMatrix<Ty2>& other)
			: size(other.size), data(other.size * other.size)
		{
			for (size_t i = 0; i < size * size; ++i)
			{
				data[i] = Ty(other.data[i]);
			}
		}

		DenseMatrix(const Eigen::MatrixXd& matrix)
			: size(matrix.rows()), data(matrix.size())
		{
			assert(matrix.rows() == matrix.cols());  // Ensure it's a square matrix

			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					data[i * size + j] = matrix(i, j);
				}
			}
		}

		DenseMatrix(const Eigen::MatrixXcd& matrix)
			: size(matrix.rows()), data(matrix.size())
		{
			assert(matrix.rows() == matrix.cols());  // Ensure it's a square matrix

			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					data[i * size + j] = matrix(i, j);
				}
			}
		}

		void assign(size_t size_, const std::vector<Ty>& initial_data)
		{
			size = size_;
			data = initial_data;
		}

		size_t n_row() const {
			return size;
		}

		Ty& get(size_t x, size_t y) {
			return data[x * size + y];
		}

		const Ty& get(size_t x, size_t y) const {
			return data[x * size + y];
		}

		Ty& operator()(size_t x, size_t y) {
			return get(x, y);
		}

		const Ty& operator()(size_t x, size_t y) const {
			return data[x * size + y];
		}

		template<typename Ty2>
		DenseMatrix<Ty>& operator+=(const DenseMatrix<Ty2>& m) 
		{
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					(*this)(i, j) += m(i, j);
				}
			}
			return *this;
		}

		template<typename Ty2>
		DenseMatrix<Ty>& operator-=(const DenseMatrix<Ty2>& m) 
		{
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					(*this)(i, j) -= m(i, j);
				}
			}
			return *this;
		}

		template<typename Ty2>
		DenseMatrix<Ty> operator+(const DenseMatrix<Ty2>& m) const 
		{
			DenseMatrix<Ty> ret(*this);
			ret += m;
			return ret;
		}

		template<typename Ty2>
		DenseMatrix<Ty> operator-(const DenseMatrix<Ty2>& m) const 
		{
			DenseMatrix<Ty> ret(*this);
			ret -= m;
			return ret;
		}
	
		template<typename Ty2>
		DenseMatrix<Ty>& operator*=(Ty2 m) {
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					(*this)(i, j) *= m;
				}
			}
			return *this;
		}

#if defined(_MSC_VER)
#pragma warning(disable: 4244) // 从“Ty2”转换到“const std::complex<double>::_Ty”，可能丢失数据
#endif
		template<typename Ty2>
		DenseMatrix<Ty> operator/(Ty2 m) {
			DenseMatrix<Ty> ret(*this);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					ret(i, j) /= m;
				}
			}
			return ret;
		}

#if defined(_MSC_VER)
#pragma warning(default: 4244)
#endif

		DenseMatrix<Ty> operator*(const DenseMatrix<Ty>& m) const {
			DenseMatrix<Ty> newm(size);

			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					for (size_t k = 0; k < size; ++k) {
						newm(i, j) += (*this)(i, k) * m(k, j);
					}
				}
			}
			return newm;
		}

		double normF() const {
			double sum = 0;
			for (size_t j = 0; j < size; ++j)
			{
				for (size_t i = 0; i < size; ++i)
				{
					double val = std::abs(data[size * j + i]);
					sum += val * val;
				}
			}
			return std::sqrt(sum);
		}

		std::string to_string() const {
			std::stringstream ss;
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					ss << std::setprecision(15) << get(i, j) << " ";
				}
				ss << std::endl;
			}
			return ss.str();
		}

		void write_matlab_file(std::string filename) {
			std::ofstream out(filename, std::ios::out);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					out << get(i, j) << " ";
				}
				out << std::endl;
			}
		}

		bool is_symmetric(double tol = epsilon) const
		{
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = i + 1; j < size; ++j)
				{
					if (std::abs(get(i, j) - get(j, i)) > tol)
						return false;
				}
			}
			return true;
		}

		bool is_Hermitian(double tol = epsilon) const
		{
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = i + 1; j < size; ++j)
				{
					if (std::abs(get(i, j) - std::conj(get(j, i))) > tol)
						return false;
				}
			}
			return true;
		}

		Eigen::MatrixX<Ty> to_eigen() const
		{
			Eigen::MatrixX<Ty> m(size, size);
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = 0; j < size; ++j)
				{
					m(i, j) = get(i, j);
				}
			}
			return m;
		}

		static DenseMatrix<Ty> from_eigen(const Eigen::MatrixX<Ty>& mat)
		{
			if (mat.rows() != mat.cols())
				throw_invalid_input("Matrix must be square!");

			DenseMatrix<Ty> ret(mat.rows());
			for (size_t i = 0; i < mat.rows(); ++i) {
				for (size_t j = 0; j < mat.cols(); ++j) {
					ret(i, j) = mat(i, j);
				}
			}
			return ret;
		}

		bool allclose(const DenseMatrix<Ty>& other, double tol = epsilon) const
		{
			// Check if sizes are equal
			if (size != other.size) return false;

			// Check if all elements are close
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					double diff = std::abs(get(i, j) - other.get(i, j));
					if (diff > tol) {
						return false;
					}
				}
			}
			return true;
		}

		bool is_zero(double tol = epsilon) const
		{
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					if (std::abs(get(i, j)) > tol) {
						return false;
					}
				}
			}
			return true;
		}

		bool is_identity(double tol = epsilon) const
		{
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					if (i == j) {
						if (std::abs(get(i, j) - 1.0) > tol) {
							return false;
						}
					}
					else {
						if (std::abs(get(i, j)) > tol) {
							return false;
						}
					}
				}
			}
			return true;
		}

		bool is_unitary(double tol = epsilon) const
		{
			return (dagger() * *this).is_identity(tol)
				&& (*this * dagger()).is_identity(tol);
		}

		DenseMatrix<Ty> transpose() const
		{
			DenseMatrix<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					ret(j, i) = get(i, j);
				}
			}
			return ret;
		}

		DenseMatrix<Ty> conjugate() const 
		{
			DenseMatrix<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					ret(i, j) = std::conj(get(i, j));
				}
			}
			return ret;
		}

		DenseMatrix<Ty> dagger() const {
			DenseMatrix<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					ret(i, j) = std::conj(get(j, i));
				}
			}
			return ret;
		}

		static DenseMatrix<Ty> identity(size_t size) {
			DenseMatrix<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					if (i == j) {
						ret(i, j) = 1;
					}
					else {
						ret(i, j) = 0;
					}
				}
			}
			return ret;
		}

		static DenseMatrix<Ty> random(size_t size, double minval = -1, double maxval = 1) {
			DenseMatrix<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				for (size_t j = 0; j < size; ++j) {
					ret(i, j) = Ty(random_engine::uniform(minval, maxval));
				}
			}
			return ret;
		}


		~DenseMatrix() {}
	};

	DenseMatrix<complex_t> dagger(const DenseMatrix<complex_t>& mat);

	template<typename Ty>
	struct DenseVector {
		size_t size = 0;
		/*Ty* data;*/
		std::vector<Ty> data;

		DenseVector() {
		}

		DenseVector(size_t size_)
			: size(size_), data(size, 0)
		{
		}

		DenseVector(size_t size_, std::vector<Ty> initial_data)
			: size(size_), data(initial_data) {	}

		DenseVector<Ty> operator+(const DenseVector<Ty>& v) const {
			DenseVector<Ty> vout(*this);

			for (size_t i = 0; i < size; ++i)
				vout.data[i] += v.data[i];
			return vout;
		}

		DenseVector<Ty> operator-(const DenseVector<Ty>& v) const {
			DenseVector<Ty> vout(*this);

			for (size_t i = 0; i < size; ++i)
				vout.data[i] -= v.data[i];
			return vout;
		}

		DenseVector<Ty> operator*(const Ty m) const {
			DenseVector<Ty> vout(*this);

			for (size_t i = 0; i < size; ++i)
				vout.data[i] = data[i] *m;
			return vout;
		}

		DenseVector<Ty> operator/(const Ty m) const {
			DenseVector<Ty> vout(*this);

			for (size_t i = 0; i < size; ++i)
				vout.data[i] = data[i] / m;
			return vout;
		}

		Ty& get(size_t x) {
			return data[x];
		}

		const Ty& get(size_t x) const {
			return data[x];
		}

		Ty& operator()(size_t x) {
			return get(x);
		}

		const Ty& operator()(size_t x) const {
			return get(x);
		}

		Ty& operator[](size_t x) {
			return get(x);
		}

		const Ty& operator[](size_t x) const {
			return get(x);
		}

		double norm2() const {
			double sum = 0;
			for (size_t i = 0; i < size; ++i) sum += (std::abs(data[i]) * std::abs(data[i]));
			return std::sqrt(sum);
		}

		double norm2(size_t s, size_t n) const {
			double sum = 0;
			for (size_t i = s; i < size; i += n) sum += (std::abs(data[i]) * std::abs(data[i]));
			return std::sqrt(sum);
		}

		double normInf() const {
			double ninf = 0;
			for (size_t i = 0; i < size; ++i) {
				if (std::abs(data[i]) > ninf) {
					ninf = std::abs(data[i]);
				}
			}
			return ninf;
		}

		std::string to_string() const {
			std::stringstream ss;
			for (size_t i = 0; i < size; ++i) {
				ss << std::setprecision(15) << data[i] << std::endl;
			}
			return ss.str();
		}

		void write_matlab_file(std::string filename) const {
			std::ofstream out(filename, std::ios::out);
			for (size_t i = 0; i < size - 1; ++i) {
				out << get(i) << ";";
			}
			out << get(size - 1);
		}

		size_t max(const Ty& maxvalue) const {
			size_t idx = 0;
			maxvalue = get(0);
			for (size_t i = 1; i < size; ++i) {
				if (get(i) > maxvalue) {
					maxvalue = get(i);
					idx = i;
				}
			}
			return idx;
		}

		size_t maxabs(const Ty& maxvalue) const {
			size_t idx = 0;
			maxvalue = abs(get(0));
			for (size_t i = 1; i < size; ++i) {
				if (std::abs(get(i)) > maxvalue) {
					maxvalue = std::abs(get(i));
					idx = i;
				}
			}
			return idx;
		}

		template<typename OutType = Ty>
		std::vector<OutType> to_vec() const {
			std::vector<OutType> values;
			values.resize(size);

			for (size_t i = 0; i < size; ++i) {
				values[i] = static_cast<OutType>(data[i]);
			}
			return values;
		}

		std::vector<double> square() const {
			std::vector<double> values;
			values.resize(size);

			for (size_t i = 0; i < size; ++i) {
				values[i] = data[i] * data[i];
			}
			return values;
		}

		std::vector<size_t> get_sgn() const {
			std::vector<size_t> sgns;
			sgns.resize(size);
			for (size_t i = 0; i < size; ++i) {
				sgns[i] = data[i] > 0 ? 1 : -1;
			}
			return sgns;
		}

		EigenVec<Ty> to_eigen() const
		{
			Eigen::VectorX<Ty> m(size);
			for (size_t i = 0; i < size; ++i)
			{
				m(i) = get(i);
			}
			return m;
		}

		static DenseVector<Ty> from_eigen(const EigenVec<Ty>& vec)
		{
			DenseVector<Ty> ret(vec.size());
			for (size_t i = 0; i < vec.size(); ++i) {
				ret(i) = vec(i);
			}
			return ret;
		}

		static DenseVector<Ty> random(size_t size, double minval = -1, double maxval = 1) 
		{
			DenseVector<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				ret(i) = Ty(random_engine::uniform(minval, maxval));
			}
			return ret;
		}

		static DenseVector<Ty> ones(size_t size)
		{
			DenseVector<Ty> ret(size);
			for (size_t i = 0; i < size; ++i) {
				ret(i) = 1;
			}
			return ret;
		}

		bool allclose(const DenseVector<Ty>& other, double tol = epsilon) const
		{
			// Check if sizes are equal
			if (size != other.size) return false;

			// Check if all elements are close
			for (size_t i = 0; i < size; ++i) {
				double diff = std::abs(get(i) - other.get(i));
				if (diff > tol) {
					return false;
				}
			}
			return true;
		}

		bool is_zero(double tol = epsilon) const
		{
			for (size_t i = 0; i < size; ++i) {
				if (std::abs(get(i)) > tol) {
					return false;
				}
			}
			return true;
		}

		~DenseVector() {
			// delete[] data;
		}
	};

	template<typename Ty>
	DenseVector<Ty> operator*(const DenseMatrix<Ty> &m, const DenseVector<Ty> &v) {
		if (m.size != v.size) throw std::runtime_error("Bad size");
		DenseVector<Ty> v2(m.size);

		for (size_t i = 0; i < m.size; ++i) {
			for (size_t j = 0; j < m.size; ++j) {
				v2(i) += m(i, j) * v(j);
			}
		}
		return v2;
	}

	template<typename Ty, typename Ty2>
	DenseVector<Ty> operator*(const DenseVector<Ty> &v, Ty2 a) {
		DenseVector<Ty> v2(v.size);

		for (size_t i = 0; i < v.size; ++i) {
			v2(i) = v(i) * a;
		}
		return v2;
	}

	template<typename Ty, typename Ty2>
	DenseVector<Ty> operator/(const DenseVector<Ty> &v, Ty2 a) {
		return v * Ty(1.0 / a);
	}

	template<typename Ty>
	DenseVector<Ty> get_column(const DenseMatrix<Ty> &A, size_t col) {
		DenseVector<Ty> v = DenseVector<Ty>(A.size);
		for (size_t i = 0; i < A.size; ++i) {
			v(i) = A(i, col);
		}
		return v;
	}

	template<typename Ty>
	bool operator==(const DenseVector<Ty> &v1, const DenseVector<Ty> &v2) {
		Ty tolerance = 1e-3;
		if (v1.size != v2.size) throw std::runtime_error("Bad size");
		if ((v1 - v2).norm2() < tolerance) return true;
		return false;
	}

	template<typename Ty>
	void check_vec(const DenseVector<Ty> &v1, const DenseVector<Ty> &v2) {
		Ty tolerance = 1e-4;
		if (v1.size != v2.size) throw std::runtime_error("Bad size");
		for (size_t i = 0; i < v1.size; ++i) {
			if (std::abs(v1(i) - v2(i)) > tolerance) {
				std::cout << "Bad: " << i << "\t" << "v1: " << v1(i) << "\tv2:" << v2(i) << std::endl;
			}
		}
	}

	template<typename Ty>
	void check_vec(const DenseVector<Ty> &v1, const DenseVector<Ty> &v2, Ty tolerance) {
		if (v1.size != v2.size) throw std::runtime_error("Bad size");
		for (size_t i = 0; i < v1.size; ++i) {
			if (std::abs(v1(i) - v2(i)) > tolerance) {
				std::stringstream ss;
				ss << "Bad: " << i << "\t" << "v1: " << v1(i) << "\tv2:" << v2(i) << std::endl;
				throw std::runtime_error(ss.str());
			}
		}
	}

	template<typename Ty>
	void swap_two_rows(DenseMatrix<Ty>& A, DenseVector<Ty>& b, size_t row1, size_t row2) {
		std::swap(b(row1), b(row2));

		for (size_t i = 0; i < A.size; ++i) {
			std::swap(A(row1, i), A(row2, i));
		}
	}

	template<typename Ty>
	void row_elimination(DenseMatrix<Ty>& A, DenseVector<Ty>& b, size_t row, size_t row2) {
		size_t size = A.size;
		if (A(row, row) == 0 || A.size != b.size) {
			throw std::runtime_error("Bad Gaussian Solver.");
		}
		if (A(row2, row) == 0) return;
		Ty coef = A(row2, row) / A(row, row);
		for (size_t j = row; j < size; ++j) {
			A(row2, j) -= (A(row, j) * coef);
		}
		b(row2) -= (b(row) * coef);
	}

	template<typename Ty>
	void column_elimination(DenseMatrix<Ty>& A, DenseVector<Ty>& b, DenseVector<Ty>& x) {
		size_t size = A.size;
		x(size - 1) = b(size - 1) / A(size - 1, size - 1);
		for (size_t i = size - 1; i --> 0;) {
			Ty b0 = b(i);
			for (size_t j = i + 1; j < size; ++j) {
				b0 -= (A(i, j) * x(j));
			}
			x(i) = b0 / A(i, i);
		}
	}

	template<typename Ty>
	DenseVector<Ty> my_linear_solver(DenseMatrix<Ty> A, DenseVector<Ty> b) {
		profiler _("my_linear_solver");
		if (A.size != b.size) 
			throw std::runtime_error("Bad size");

		size_t n = A.size;
		DenseVector<Ty> x(n);

		for (size_t i = 0; i < n; ++i) {
			// find maximum row
			Ty maximum = 0.0;
			size_t j_ = i;
			for (size_t j = i + 1; j < n; ++j) {
				if (std::abs(A(j, i)) > maximum) {
					j_ = j;
					maximum = std::abs(A(j, i));
				}
			}
			if (A(j_, i) == 0) {
				DenseVector<Ty> vi = get_column(A, i);
				std::cout << vi.norm2();
				throw std::runtime_error("No solution.");
			}
			// swap_two_rows(A, b, j_, i);

			for (size_t j = i + 1; j < n; ++j) {
				row_elimination(A, b, i, j);
			}
		}
		//cout << A.to_string();
		//cout << b.to_string();
		column_elimination(A, b, x);
		return x;
	}

	template<typename Ty>
	DenseVector<Ty> eigen_linear_solver(const DenseMatrix<Ty> &A, const DenseVector<Ty> &b) {
		/* convert it to eigen mat */
		EigenMat<Ty> A_eig = A.to_eigen();
		EigenMat<Ty> b_eig = b.to_eigen();

		Eigen::FullPivLU<Eigen::MatrixX<Ty>> lu(A_eig);
    
		// check invertible
		if (!lu.isInvertible()) {
			throw std::runtime_error("No solution.");
		}
		
		// get solution
		EigenVec<Ty> x = lu.solve(b_eig);
		return DenseVector<Ty>::from_eigen(x);
	}

	template<typename Ty>
	DenseVector<Ty> eigen_linear_solver(const SparseMatrix& A, const DenseVector<Ty>& b) {
		/* convert it to eigen mat */
		Eigen::SparseMatrix<double> A_eig = A.to_eigen();
		EigenMat<Ty> b_eig = b.to_eigen();

		Eigen::SparseLU<Eigen::SparseMatrix<Ty>> lu;
		lu.compute(A_eig);

		// check invertible
		if (lu.info() != Eigen::Success) {
			throw std::runtime_error("No solution.");
		}

		// get solution
		EigenVec<Ty> x = lu.solve(b_eig);
		return DenseVector<Ty>::from_eigen(x);
	}

	template<typename Ty>
	DenseMatrix<Ty> randmat(size_t size) {
		
		DenseMatrix<Ty> m(size);
		for (size_t i = 0; i < size; ++i) {
			for (size_t j = 0; j < size; ++j) {
				m(i, j) = random_engine::rng();
			}
		}
		return m;
	}

	template<typename Ty>
	DenseVector<Ty> randvec(size_t size) {
		DenseVector<Ty> v(size);
		for (size_t i = 0; i < size; ++i) {
			v(i) = random_engine::rng();
		}
		return v;
	}

	template<typename Ty>
	DenseVector<Ty> pick_threshold(DenseVector<Ty>& v, Ty threshold, size_t& counter) {
		DenseVector<Ty> v2(v.size);
		counter = 0;

		for (size_t i = 0; i < v.size; ++i) {
			double elem = v(i);
			if (std::abs(elem) > threshold) {
				counter++;
				v2(i) = elem;
			}
		}
		return v2;
	}

	template<typename Ty = double>
	DenseVector<Ty> ones(size_t nrow)
	{
		DenseVector<Ty> vec(nrow);
		for (size_t i = 0; i < nrow; ++i)
			vec[i] = static_cast<Ty>(1);
		return vec;
	}

	template<typename Ty>
	DenseVector<Ty> chebyshev_n(size_t step, const DenseMatrix<Ty>& mat, const DenseVector<Ty>& vec)
	{
		DenseVector<Ty> vec0 = vec;
		if (step == 0) 
			return vec0;
		
		DenseVector<Ty> vec1 = mat * vec;
		
		if (step == 1) 
			return vec1;
		
		DenseVector<Ty> vec2;

		for (size_t n = 2; n <= step; ++n)
		{
			// Tn = 2xTn-1(x)-Tn-2(x)
			vec2 = (mat * vec1) * 2.0 - vec0;
			vec0 = vec1;
			vec1 = vec2;
		}
		return vec2;
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_band_mat(size_t row_size, size_t l)
	{
		DenseMatrix<Ty> m(row_size);
		// diagonal
		for (size_t i = 0; i < row_size; ++i)
		{
			m(i, i) = random_engine::rng() * 0.5 + 0.5;
		}
		
		for (size_t k = 1; k < l; ++k)
		{
			for (size_t i = 0; i + k < row_size; ++i)
			{
				m(i + k, i) = random_engine::rng() - 0.5;
				m(i, i + k) = m(i + k, i);
			}
		}
		return m;
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_band_mat_unsigned(size_t row_size, size_t l)
	{
		DenseMatrix<Ty> m(row_size);
		// diagonal
		for (size_t i = 0; i < row_size; ++i)
		{
			m(i, i) = random_engine::rng() * 0.5 + 0.5;
		}

		for (size_t k = 1; k < l; ++k)
		{
			for (size_t i = 0; i + k < row_size; ++i)
			{
				m(i + k, i) = random_engine::rng();
				m(i, i + k) = m(i + k, i);
			}
		}
		return m;
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_band_mat_signed(size_t row_size, size_t l)
	{
		DenseMatrix<Ty> m(row_size);
		// diagonal
		for (size_t i = 0; i < row_size; ++i)
		{
			m(i, i) = random_engine::rng() * 0.5 + 0.5;
		}

		for (size_t k = 1; k < l; ++k)
		{
			for (size_t i = 0; i + k < row_size; ++i)
			{
				m(i + k, i) = random_engine::rng() * 2 - 1.0;
				m(i, i + k) = m(i + k, i);
			}
		}
		return m;
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_Poiseuille_mat(size_t row_size, double alpha, double beta)
	{
		DenseMatrix<Ty> m(row_size);
		// diagonal
		for (size_t i = 0; i < row_size; ++i)
		{
			m(i, i) = alpha;
			if (i > 0) {
				m(i, i - 1) = beta;
				m(i - 1, i) = beta;
			}
		}

		return m;
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_band_mat_asymmetric(size_t row_size, size_t l)
	{
		DenseMatrix<Ty> m(row_size);
		// diagonal
		for (size_t i = 0; i < row_size; ++i)
		{
			m(i, i) = random_engine::rng() * 0.5 + 0.5;
		}

		for (size_t k = 1; k < l; ++k)
		{
			for (size_t i = 0; i + k < row_size; ++i)
			{
				m(i + k, i) = random_engine::rng() * 2 - 1.0;
				m(i, i + k) = random_engine::rng() * 2 - 1.0;
			}
		}
		return m;
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_specified_kappa_mat_symmetric(size_t row_size, double kappa)
	{
		Eigen::VectorXd eigenvalues(row_size);
		eigenvalues[0] = 1;
		eigenvalues[row_size - 1] = kappa;
		for (size_t i = 1; i < row_size - 1; i++) {
			eigenvalues[i] = random_engine::rng() * (kappa - 1) + 1.0;
			// Random values between 1 and kappa
		}
		Eigen::MatrixXd mat = generate_band_mat_unsigned(row_size, 10).to_eigen();
		Eigen::HouseholderQR<Eigen::MatrixXd> qr(mat);
		Eigen::MatrixXd Q = qr.householderQ();

		// Construct the diagonal matrix of eigenvalues
		Eigen::MatrixXd Lambda = eigenvalues.asDiagonal();
		Eigen::MatrixXd QQ = Q * Q.transpose();
		std::cout << "Matrix Q * Q^T:\n" << QQ << "\n" << std::endl;
		// Form the Hermitian matrix A = Q * Lambda * Q.transpose()
		Eigen::MatrixXd A = Q * Lambda * Q.transpose();
		return DenseMatrix<Ty>(A);
	}

	template<typename Ty = double>
	DenseMatrix<Ty> generate_specified_kappa_mat_asymmetric(size_t row_size, double kappa)
	{
		
		Eigen::MatrixXd mat_U = generate_band_mat_unsigned(row_size, 20).to_eigen();
		Eigen::MatrixXd mat_V = generate_band_mat_asymmetric(row_size, 10).to_eigen();
		Eigen::HouseholderQR<Eigen::MatrixXd> qrU(mat_U);
		Eigen::HouseholderQR<Eigen::MatrixXd> qrV(mat_V);
		Eigen::MatrixXd U = qrU.householderQ();
		Eigen::MatrixXd V = qrV.householderQ();

		Eigen::VectorXd singular_values(row_size);
		singular_values[0] = 1;
		singular_values[row_size - 1] = kappa;
		for (size_t i = 1; i < row_size - 1; i++) {
			singular_values[i] = random_engine::rng() * (kappa - 1) + 1.0;
		}
		Eigen::MatrixXd Sigma = singular_values.asDiagonal();

		Eigen::MatrixXd A = U * Sigma * V.transpose();
		return DenseMatrix<Ty>(A);
	}

	template<typename Ty = double>
	DenseMatrix<Ty> sparse2dense(const SparseMatrix& mat)
	{
		size_t nrow = mat.n_row;
		size_t nnz_col = mat.nnz_col;
		DenseMatrix<Ty> ret(nrow);

		if (mat.positive_only) {
			for (size_t row_id = 0; row_id < mat.n_row; ++row_id)
			{
				for (size_t j = 0; j < mat.nnz_col; ++j)
				{
					size_t col_id = mat.sparsity[row_id * nnz_col + j];
					size_t col_value = mat.elements[row_id * nnz_col + j];

					ret(row_id, col_id) = static_cast<Ty>(col_value);
				}
			}
		}
		else
		{
			for (size_t row_id = 0; row_id < mat.n_row; ++row_id)
			{
				for (size_t j = 0; j < mat.nnz_col; ++j)
				{
					size_t col_id = mat.sparsity[row_id * nnz_col + j];
					size_t col_value = mat.elements[row_id * nnz_col + j];

					int64_t real_value = get_complement(col_value, mat.data_size);

					ret(row_id, col_id) = static_cast<Ty>(real_value);
				}
			}
		}
		return ret;
	}

	double get_min_eigval(const SparseMatrix& mat);
	double get_min_eigval(const DenseMatrix<double>& mat, size_t l);
	double get_min_eigval_from_sparsity(const DenseMatrix<double>& mat, size_t l);
	double get_min_eigval(const Eigen::SparseMatrix<double>& mat, size_t l);

	SparseMatrix generate_simplest_sparse_matrix_unsigned_0();
	SparseMatrix generate_simplest_sparse_matrix_unsigned_1();
	SparseMatrix generate_simplest_sparse_matrix_unsigned_2();
	SparseMatrix generate_simplest_sparse_matrix_signed_0();
	SparseMatrix generate_simplest_sparse_matrix_signed_1();
	// SparseMatrix generate_simplest_sparse_matrix_signed_2();

	SparseMatrix get_test_matrix();
	std::vector<complex_t> get_target_result();

	SparseMatrix random_band_sparse_signed(size_t row_size, size_t l, size_t elem_size);
	SparseMatrix dense2sparse_band_unsigned(const DenseMatrix<double>& mat, size_t l, size_t elem_size);
	double get_kappa(const DenseMatrix<double>& mat);
	double get_kappa_general(const DenseMatrix<double>& mat);
	double get_kappa_Tridiagonal(double alpha, double beta, const size_t size);
	void normalize_column(DenseMatrix<complex_t>& mat, size_t col_id);
	void gram_schmidt_process(DenseMatrix<complex_t>& ret);

} // namespace qram_simulator

/* Eigen Library Support */
namespace qram_simulator {

	template<typename DerivedA, typename DerivedB>
	auto kroneckerProduct(const Eigen::MatrixBase<DerivedA>& A, const Eigen::MatrixBase<DerivedB>& B) {
		using Scalar = typename DerivedA::Scalar;
		using ScalarB = typename DerivedB::Scalar;
		// static_assert(std::is_same_v<Scalar, ScalarB>, "Two types must be the same.");
		using ResultScalar = decltype(std::declval<Scalar>() * std::declval<ScalarB>());
		using ResultType = Eigen::MatrixX<ResultScalar>;

		ResultType result(A.rows() * B.rows(), A.cols() * B.cols());
		for (int i = 0; i < A.rows(); ++i) {
			for (int j = 0; j < A.cols(); ++j) {
				result.block(i * B.rows(), j * B.cols(), B.rows(), B.cols()) = A(i, j) * B;
			}
		}
		return result;
	}

	template<typename Derived>
	auto zeros_like(const Eigen::MatrixBase<Derived>& mat) {
		using EigenTy = typename Derived::PlainObject;
		return EigenTy::Zero(mat.rows(), mat.cols());
	}

	template<typename Derived>
	auto ones_like(const Eigen::MatrixBase<Derived>& mat) {
		using EigenTy = typename Derived::PlainObject;
		return EigenTy::Ones(mat.rows(), mat.cols());
	}

	template<typename Derived>
	auto eyes_like(const Eigen::MatrixBase<Derived>& mat) {
		using EigenTy = typename Derived::PlainObject;
		if (mat.rows() != mat.cols())
			throw_invalid_input("Matrix must be square");

		return EigenTy::Identity(mat.rows(), mat.cols());
	}


	template <typename Derived>
	std::string eigenmat2str(const Eigen::MatrixBase<Derived>& mat) {
		std::ostringstream oss;
		oss << mat;
		return oss.str();
	}

	template<typename Ty = complex_t>
	auto GetVec0() -> EigenVec<Ty>
	{
		EigenMat<Ty> mat(2, 1);
		mat << 1, 0;
		return mat;
	}

	template<typename Ty = complex_t>
	auto GetVec1() -> EigenVec<Ty>
	{
		EigenMat<Ty> mat(2, 1);
		mat << 0, 1;
		return mat;
	}

	template<typename Ty = complex_t>
	auto GetMat01() -> EigenMat<Ty>
	{
		EigenMat<Ty> mat(2, 2);
		mat <<
			0, 1,
			0, 0;
		return mat;
	}

	template<typename Ty = complex_t>
	auto GetMat10() -> EigenMat<Ty>
	{
		EigenMat<Ty> mat(2, 2);
		mat <<
			0, 0,
			1, 0;
		return mat;
	}

	inline auto GetSigmaZ() -> EigenMat<complex_t>
	{
		EigenMat<complex_t> mat(2, 2);
		mat <<
			1, 0,
			0, -1;
		return mat;
	}

	template<typename Ty>
	auto HermitianA(const Eigen::MatrixBase<Ty>& A) -> EigenMat<complex_t>
	{
		auto A01 = kroneckerProduct(GetMat01<complex_t>(), A);
		auto A10 = kroneckerProduct(GetMat10<complex_t>(), A.adjoint());
		return A01 + A10;
	}
}