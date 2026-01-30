#if false

#include "hamiltonian_simulation.h"
#include "matrix.h"
#include <Eigen/Eigen>
#include <Eigen/Dense>
#include "state_preparation.h"
#include "qcnn.h"

namespace qram_simulator {
	namespace CNN {
		//4*4卷积
		std::vector<double> convolve4x4(const std::vector<double>& image, const std::vector<double>& kernel) {
			std::vector<double> result;

			// 检查输入图像和卷积核的大小是否正确
			if (image.size() != 784 || kernel.size() != 16) {
				std::cerr << "Invalid input size!" << std::endl;
				return result;
			}

			// 执行4x4卷积操作
			for (int i = 0; i <= 24; i += 1) {
				for (int j = 0; j <= 24; j += 1) {
					double sum = 0.0;
					for (int m = 0; m < 4; ++m) {
						for (int n = 0; n < 4; ++n) {
							sum += image[(i + m) * 28 + (j + n)] * kernel[m * 4 + n];
						}
					}
					result.push_back(sum);
				}
			}

			return result;
		}
		//找到大于input的最小power(2,n)
		int findpow2(int input) {
			for (int i = 0; i < 20; i++) {
				if (pow2(i) > input)
					return i;
			}
			return 0;
		}

		//将卷积转换为矩阵乘法
		std::vector<double> img2col(std::vector<double>& input, int kernel_size) {
			int pic_size = sqrt(input.size());
			std::vector<std::vector<double>> img(pic_size, std::vector<double>(pic_size));
			for (int i = 0; i < pic_size; i++) {
				for (int j = 0; j < pic_size; j++) {
					img[i][j] = input[i * pic_size + j];
				}
			}
			std::vector<double> output(0);
			for (int i = 0; i <= pic_size - kernel_size; i++) {
				for (int j = 0; j <= pic_size - kernel_size; j++) {
					for (int k = 0; k < kernel_size; k++) {
						for (int l = 0; l < kernel_size; l++) {
							output.push_back(img[i + k][j + l]);
						}
					}
				}
			}
			return output;
		}
		std::vector<double> col2img(std::vector<double>& input, int kernel_size, int pic_size) {
			int output_size = pic_size - kernel_size + 1;
			std::vector<std::vector<double>> img(pic_size, std::vector<double>(pic_size, 0));
			std::vector<std::vector<int>> count(pic_size, std::vector<int>(pic_size, 0));
			int index = 0;
			for (int i = 0; i < output_size; i++) {
				for (int j = 0; j < output_size; j++) {
					for (int k = 0; k < kernel_size; k++) {
						for (int l = 0; l < kernel_size; l++) {
							img[i + k][j + l] += input[index];
							count[i + k][j + l]++;
							index++;
						}
					}
				}
			}
			for (int i = 0; i < pic_size; i++) {
				for (int j = 0; j < pic_size; j++) {
					img[i][j] /= count[i][j];
				}
			}
			std::vector<double> output(pic_size * pic_size);
			for (int i = 0; i < pic_size; i++) {
				for (int j = 0; j < pic_size; j++) {
					output[i * pic_size + j] = img[i][j];
				}
			}
			return output;
		}

		std::vector<double> divideAndNormalize(std::vector<double>& inner_matrix, int p) {
			int size = inner_matrix.size();
			int chunkSize = size / p;
			std::vector<double> result(size);

			for (int i = 0; i < p; ++i) {
				double sum = 0;
				for (int j = 0; j < chunkSize; ++j) {
					sum += inner_matrix[i * chunkSize + j] * inner_matrix[i * chunkSize + j];
				}
				sum = sqrt(sum);
				for (int j = 0; j < chunkSize; ++j) {
					result[i * chunkSize + j] = inner_matrix[i * chunkSize + j] / sum;
				}
			}

			return result;
		}
		//长度不等于pow(2,size)的整数序列补0

		class khan_Matrix {
		private:
			std::vector<std::vector<double>> m;  // 存储矩阵的数据
		public:
			// 构造函数
			khan_Matrix(int rows, int cols) : m(rows, std::vector<double>(cols)) {}
			// 从std::vector<double>创建Matrix
			khan_Matrix(int rows, int cols, const std::vector<double>& vec) : m(rows, std::vector<double>(cols)) {
				if (rows * cols != vec.size()) {
					throw std::invalid_argument("The size of the vector does not match the size of the matrix");
				}
				for (int i = 0; i < rows; ++i) {
					for (int j = 0; j < cols; ++j) {
						m[i][j] = vec[i * cols + j];
					}
				}
			}
			// 设置矩阵元素的值
			void setElement(int row, int col, double value) {
				m[row][col] = value;
			}
			// 获取矩阵元素的值
			double getElement(int row, int col) const {
				return m[row][col];
			}

			// 矩阵加法
			khan_Matrix add(const khan_Matrix& other) const {
				int rows = m.size();
				int cols = m[0].size();
				khan_Matrix result(rows, cols);
				for (int i = 0; i < rows; ++i) {
					for (int j = 0; j < cols; ++j) {
						result.setElement(i, j, this->getElement(i, j) + other.getElement(i, j));
					}
				}
				return result;
			}

			// 矩阵减法
			khan_Matrix subtract(const khan_Matrix& other) const {
				int rows = m.size();
				int cols = m[0].size();
				khan_Matrix result(rows, cols);
				for (int i = 0; i < rows; ++i) {
					for (int j = 0; j < cols; ++j) {
						result.setElement(i, j, this->getElement(i, j) - other.getElement(i, j));
					}
				}
				return result;
			}

			// 矩阵乘法
			khan_Matrix multiply(const khan_Matrix& other) const {
				int rows = m.size();
				int cols = other.m[0].size();
				int inner_dim = m[0].size();
				khan_Matrix result(rows, cols);
				for (int i = 0; i < rows; ++i) {
					for (int j = 0; j < cols; ++j) {
						double value = 0;
						for (int k = 0; k < inner_dim; ++k) {
							value += this->getElement(i, k) * other.getElement(k, j);
						}
						result.setElement(i, j, value);
					}
				}
				return result;
			}

			// 矩阵转置
			khan_Matrix transpose() const {
				int rows = m.size();
				int cols = m[0].size();
				khan_Matrix result(cols, rows);
				for (int i = 0; i < rows; ++i) {
					for (int j = 0; j < cols; ++j) {
						result.setElement(j, i, this->getElement(i, j));
					}
				}
				return result;
			}

			// 矩阵放缩

			khan_Matrix scale(double factor) {
				int rows = m.size();
				int cols = m[0].size();
				khan_Matrix result(rows, cols);
				for (int i = 0; i < rows; ++i) {
					for (int j = 0; j < cols; ++j) {
						result.setElement(i, j, m[i][j] * factor);
					}
				}
				return result;
			}

			// 将矩阵转换为std::vector<double>
			std::vector<double> toVector() const {
				std::vector<double> result;
				for (const auto& row : m) {
					for (double value : row) {
						result.push_back(value);
					}
				}
				return result;
			}

		};

		memory_t padding(memory_t memory, size_t size) {
			int length = pow2(size);
			memory_t&& newMemory = std::move(memory);
			for (int index = memory.size(); index < length; index++) {
				newMemory.push_back(0);
			}
			return newMemory;
		}

		//作弊拿结果为(i,j)的概率
		double getProb(std::vector<System>& state, int i, int j) {
			double prob = 0;
			for (auto& s : state) {
				if (s.get(System::get("div")).value == false && s.get(System::get("anc_CR")).value == false
					&& s.get(System::get("p")).value == i && s.get(System::get("q")).value == j)
				{
					prob += abs_sqr(s.amplitude);
				}
			}
			return prob;
		}

		double khan_getProb(std::vector<System>& state, std::string anc, std::string anc_cr) {
			double prob = 0;
			for (auto& s : state) {
				if (s.get(System::get(anc)).value == false && s.get(System::get(anc_cr)).value == false)
				{
					prob += abs_sqr(s.amplitude);
				}
			}
			return prob;
		}

		//条件旋转矩阵
		u22_t conrotfunc_P(size_t x_, double norm) {
			std::array<std::complex<double>, 4> ret;
			double x = reinterpret_cast<double&>(x_);
			//std::cout<<x<<std::endl;
			ret[0] = x * 1.0 / sqrt(norm);
			ret[1] = -std::sqrt(1 - x * x * 1.0 / norm);
			ret[2] = -ret[1];
			ret[3] = ret[0];
			return ret;
		}

		//条件旋转逆矩阵
		u22_t conrotfunc_P_inv(size_t x, double norm) {
			std::array<std::complex<double>, 4> ret;
			ret[0] = x * 1.0 / sqrt(norm);
			ret[1] = std::sqrt(1 - x * x * 1.0 / norm);
			ret[2] = -ret[1];
			ret[3] = ret[0];
			return ret;
		}

		//khan
		CondRot_P::CondRot_P(std::string reg_in, std::string reg_out, angle_function_t t, double norm)
			: in_id(System::get(reg_in)), out_id(System::get(reg_out)), func(t), norm_c(norm)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(reg_out) != Boolean)
				throw_invalid_input();
#endif
		}

		CondRot_P::CondRot_P(int reg_in, int reg_out, angle_function_t t, double norm)
			: in_id(reg_in), out_id(reg_out), func(t), norm_c(norm)
		{
			/* Type check */
#ifndef QRAM_Release
			if (System::type_of(reg_out) != Boolean)
				throw_invalid_input();
#endif
		}

		void CondRot_P::operate(size_t l, size_t r, std::vector<System>& state) const
		{
			size_t n = r - l;
			constexpr size_t full_size = 2;
			size_t original_size = state.size();

			if (n == 0) return;
			size_t test = state[l].GetAs(in_id, uint64_t);
			// 1. get the rotation matrix
			size_t v = state[l].GetAs(in_id, uint64_t);

			u22_t mat = func(v, norm_c);

			if (_is_diagonal(mat))
			{
				_operate_diagonal(l, r, state, mat);
			}
			else if (_is_off_diagonal(mat))
			{
				_operate_off_diagonal(l, r, state, mat);
			}
			else
			{
				_operate_general(l, r, state, mat);
			}

		}

		bool CondRot_P::_is_diagonal(const u22_t& data)
		{
			if (abs_sqr(data[1]) < epsilon &&
				abs_sqr(data[2]) < epsilon)
			{
				return true;
			}
			return false;
		}

		void CondRot_P::_operate_diagonal(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const
		{
			// diagonal means that no new elements will be created
			// any operation can be handled in-place

			std::complex<double> a0 = mat[0];
			std::complex<double> a1 = mat[3];

			for (size_t i = l; i < r; ++i)
			{
				if (ConditionSatisfied(state[i])) {
					auto& s = state[i];
					if (s.get(out_id).as<bool>(1))
					{
						s.amplitude *= a1;
					}
					else
					{
						s.amplitude *= a0;
					}
				}
			}
		}

		bool CondRot_P::_is_off_diagonal(const u22_t& data)
		{
			if (abs_sqr(data[0]) < epsilon &&
				abs_sqr(data[3]) < epsilon)
			{
				return true;
			}
			return false;
		}

		void CondRot_P::_operate_off_diagonal(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const
		{
			// diagonal means that no new elements will be created
			// any operation can be handled in-place
			// with changing of storage (flipping)

			std::complex<double> a0 = mat[2];
			std::complex<double> a1 = mat[1];

			for (size_t i = l; i < r; ++i)
			{
				if (ConditionSatisfied(state[i])) {
					auto& s = state[i];
					auto& reg = s.get(out_id);
					if (reg.as<bool>(1))
					{
						s.amplitude *= a1;
						reg.value = 0; // flip
					}
					else
					{
						s.amplitude *= a0;
						reg.value = 1; // flip
					}
				}
			}
		}

		void CondRot_P::_operate_general(size_t l, size_t r,
			std::vector<System>& state, const u22_t& mat) const
		{
			size_t n = r - l;
			if (n == 1) // an extra entry should be added
			{
				size_t new_pos = state.size();
				//if(ConditionSatisfied(state[l]))
				state.push_back(state[l]);

				bool v = state[l].get(out_id).as<bool>(1);

				// if the original is 0
				if (!v)
				{
					state[new_pos].get(out_id).value = 1;

					state[l].amplitude *= mat[0];		// where |0>
					state[new_pos].amplitude *= mat[2]; // where |1>
				}
				// if the original is 1
				else
				{
					state[new_pos].get(out_id).value = 0;

					state[new_pos].amplitude *= mat[1]; // where |0>
					state[l].amplitude *= mat[3];		// where |1>
				}
			}
			else // everything can be computed in place
			{
				std::vector<std::complex<double>> new_amps(2, 0);
				complex_t a = state[l + 0].amplitude;
				complex_t b = state[l + 1].amplitude;
				state[l + 0].amplitude = a * mat[0] + b * mat[1];
				state[l + 1].amplitude = a * mat[2] + b * mat[3];
			}
		}
		void CondRot_P::operator()(std::vector<System>& state) const
		{
			//fmt::print("\nCondRot");
			/*(MoveBackRegister(out_name))(state);
			SortUnconditional()(state);*/

			(SortExceptKey(out_id))(state);
			size_t current_size = state.size();
			auto iter_l = 0;
			auto iter_r = 1;

			while (true)
			{
				if (iter_r == current_size)
				{
					if (ConditionSatisfied(state[iter_l]))
						operate(iter_l, iter_r, state);
					break;
				}
				if (!compare_equal(state[iter_l], state[iter_r], out_id))
				{
					if (ConditionSatisfied(state[iter_l]))
						operate(iter_l, iter_r, state);
					iter_l = iter_r;
					iter_r = iter_l + 1;
				}
				else
				{
					iter_r++;
				}
			}
		}

		//相位翻转
		void AllPhaseFlip::operator()(std::vector<System>& state) const
		{
			profiler _("AllPhaseFlip");
			for (auto& s : state)
			{
				if (ConditionSatisfied(s)) {
					s.amplitude *= -1;
				}
			}
		}

		//khan 
		isay::isay(std::string reg_to_change, size_t low_bounder, size_t high_bounder)
			: reg_to_change(reg_to_change), low_bounder(low_bounder), high_bounder(high_bounder) {}

		void isay::operator()(std::vector<System>& state) const {
			int counter = 0;
			for (auto& s : state) {
				if (counter < low_bounder) {
					continue;
				}
				s.get(System::get(reg_to_change)).value = 1;
				counter++;
				if (counter >= high_bounder) {
					break;
				}
			}
		}

		reindex::reindex(std::string reg_to_change, size_t div) :reg_to_change(reg_to_change), div(div) {};

		void reindex::operator()(std::vector<System>& state) const {
			int counter = 0;
			int index = 0;
			for (auto& s : state) {
				if (counter >= div) {
					s.get(System::get(reg_to_change)).value = index;
					index++;
				}
				counter++;

			}
		}

		int reverseInt(int i) {
			unsigned char c1, c2, c3, c4;
			c1 = i & 255;
			c2 = (i >> 8) & 255;
			c3 = (i >> 16) & 255;
			c4 = (i >> 24) & 255;
			return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
		}

		std::vector<int> read_mnist(std::string name) {
			std::ifstream file(name);
			if (file.is_open()) {
				int magic_number = 0;
				int number_of_images = 0;
				int n_rows = 0;
				int n_cols = 0;
				std::vector<int> image_data;
				file.read((char*)&magic_number, sizeof(magic_number));
				magic_number = reverseInt(magic_number);
				file.read((char*)&number_of_images, sizeof(number_of_images));
				number_of_images = reverseInt(number_of_images);
				file.read((char*)&n_rows, sizeof(n_rows));
				n_rows = reverseInt(n_rows);
				file.read((char*)&n_cols, sizeof(n_cols));
				n_cols = reverseInt(n_cols);

				for (int r = 0; r < n_rows; ++r) {
					for (int c = 0; c < n_cols; ++c) {
						unsigned char temp = 0;
						file.read((char*)&temp, sizeof(temp));
						image_data.push_back((int)temp);
					}

				}
				fmt::print("load_success");
				return image_data;
			}
			else {
				return std::vector<int>();
			}
		}

		void print_vector(const std::vector<int>& v) {
			std::cout << "[";
			for (size_t i = 0; i < v.size(); ++i) {
				std::cout << v[i];
				if (i != v.size() - 1) {
					std::cout << ", ";
				}
			}
			std::cout << "]" << std::endl;
		}

		void save_vector_to_file(const std::vector<int>& v, const std::string& filename) {
			std::ofstream file(filename);
			for (size_t i = 0; i < v.size(); ++i) {
				file << v[i] << " ";
			}
			file.close();
		}

		set_p::set_p(std::string reg_to_change, size_t col, size_t row, size_t start_index) :
			reg_to_change(reg_to_change), col(col), row(row), start_index(start_index) {};

		void set_p::operator()(std::vector<System>& state) const {
			int counter = 0;
			for (auto& s : state) {
				if (counter < start_index)
				{
					counter++;
					continue;
				}
				s.get(System::get(reg_to_change)).value = counter / col;
				counter++;
				if (counter >= start_index + col * row)
				{
					break;
				}
			}

		}

		//get Frobenius norm of a martix
		double get_martix_F_norm(const std::vector<std::vector<double>>& A) {
			double F_norm = 0;
			for (int i = 0; i < A.size(); i++) {
				for (int j = 0; j < A[0].size(); j++) {
					F_norm += A[i][j] * A[i][j];
				}
			}
			F_norm = sqrt(F_norm);
			return F_norm;
		};

		void test_get_martix_F_norm() {
			std::vector<std::vector<double>> A = { {1,2,3},{4,5,6},{7,8,9} };
			double A_F = get_martix_F_norm(A);
			std::cout << A_F << std::endl;
		}

		// Change Vector to Eigen  
		Eigen::MatrixXd convertVecToEigen(const std::vector<std::vector<double>>& vec) {
			int rows = vec.size();
			int cols = vec[0].size();

			Eigen::MatrixXd mat(rows, cols);

			for (int i = 0; i < rows; ++i) {
				for (int j = 0; j < cols; ++j) {
					mat(i, j) = vec[i][j];
				}
			}

			return mat;
		}

		//get Spectral norm of a martix
		double get_martix_Spectral_norm(std::vector<std::vector<double>> A) {
			Eigen::MatrixXd M = convertVecToEigen(A);
			Eigen::JacobiSVD<Eigen::MatrixXd> svd(M);
			Eigen::VectorXd singular_values = svd.singularValues();
			return singular_values.maxCoeff();
		}

		void test_get_martix_Spectral_norm() {
			std::vector<std::vector<double>> A = { {1,2,3},{4,5,6},{7,8,9} };
			double A_S = get_martix_Spectral_norm(A);
			std::cout << A_S << std::endl;
		}

		void killreg::operator()(std::vector<System>& state) {
			for (auto& s : state) {
				s.get(addr_reg).value = 0;
			}
		}

		indexCal::indexCal(int addr_reg_i_, int addr_reg_j_, int data_reg_as, int index_, int N_)
			: addr_reg_i(addr_reg_i_), addr_reg_j(addr_reg_j_), index(index_), N(N_)
		{}

		indexCal::indexCal(std::string addr_reg_i_, std::string addr_reg_j_, std::string index_, int N_)
		{
			addr_reg_i = System::get(addr_reg_i_);
			addr_reg_j = System::get(addr_reg_j_);
			index = System::get(index_);
			N = N_;
		}

		void indexCal::operator()(std::vector<System>& state) {
			for (auto& s : state) {
				s.get(index).value = s.get(addr_reg_i).value * N + s.get(addr_reg_j).value;
			}
		}

		AmplitudeLoad::AmplitudeLoad(int addr_reg_data_, int index_, int data_, int p_)
			: addr_reg_data(addr_reg_data_), index(index_), data(data_), p(p_)
		{}

		AmplitudeLoad::AmplitudeLoad(std::string addr_reg_data_, std::string index_, std::string data_, int p_)
		{
			addr_reg_data = System::get(addr_reg_data_);
			index = System::get(index_);
			data = System::get(data_);
			p = p_;
		}


		void AmplitudeLoad::operator()(std::vector<System>& state) {
			std::vector<double> inner_matrix;
			for (auto& s : state) {
				inner_matrix.push_back(s.get(data).value);
			}
			inner_matrix = divideAndNormalize(inner_matrix, p);
			for (int i = 0; i < state.size(); i++) {
				state[i].amplitude = state[i].amplitude * inner_matrix[i] * sqrt(p);
			}
		}

		std::vector<double> vectorappend(std::vector<double> pic) {
			int i = 2;
			while (i < pic.size()) {
				i *= 2;
			}
			std::vector<double> pic_2(i);
			for (int j = 0; j < pic.size(); j++) {
				pic_2[j] = pic[j];
			}
			return pic_2;
		}

		std::vector<double> reversekernel(std::vector<double> kernel) {
			std::vector<double> output(kernel.size());
			for (int i = 0; i < kernel.size(); i++) {
				output[i] = kernel[kernel.size() - i - 1];
			}
			return output;
		}

		std::vector<double> paddingforback(std::vector<double>& output, int kernel_size) {
			int pic_size = sqrt(output.size());
			std::vector<std::vector<double>> img(pic_size, std::vector<double>(pic_size));
			for (int i = 0; i < pic_size; i++) {
				for (int j = 0; j < pic_size; j++) {
					img[i][j] = output[i * pic_size + j];
				}
			}
			int padding = kernel_size / 2;
			int padded_pic_size = pic_size + 2 * padding;
			std::vector<std::vector<double>> padded_img(padded_pic_size, std::vector<double>(padded_pic_size, 0));
			for (int i = 0; i < pic_size; i++) {
				for (int j = 0; j < pic_size; j++) {
					padded_img[i + padding][j + padding] = img[i][j];
				}
			}
			std::vector<double> padded_output(padded_pic_size * padded_pic_size);
			for (int i = 0; i < padded_pic_size; i++) {
				for (int j = 0; j < padded_pic_size; j++) {
					padded_output[i * padded_pic_size + j] = padded_img[i][j];
				}
			}
			return padded_output;
		}

		double get_vector_F_form(std::vector<double> input) {
			double result = 0;
			for (int i = 0; i < input.size(); i++) {
				result += input[i] * input[i];
			}
			return result;
		}
	}
}

#endif