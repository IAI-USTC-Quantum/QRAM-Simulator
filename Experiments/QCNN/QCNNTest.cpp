#if false

#include "hamiltonian_simulation.h"
#include "matrix.h"
#include <Eigen/Eigen>
#include <state_preparation.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <string>
#include "qcnn.h"

using namespace qram_simulator;
using namespace CNN;

struct VectorComparisonResult {
	double norm1;
	double norm2;
	double maxDifference;
	size_t maxDifferenceIndex;
};
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

class khan_Matrix {
private:
	std::vector<std::vector<double>> m;  
public:
	
	khan_Matrix(int rows, int cols) : m(rows, std::vector<double>(cols)) {}

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
	
	void setElement(int row, int col, double value) {
		m[row][col] = value;
	}
	
	double getElement(int row, int col) const {
		return m[row][col];
	}

	
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



	khan_Matrix scale(double factor) {
		int rows = m.size();
		int cols = m[0].size();
		khan_Matrix result(rows , cols);
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				result.setElement(i, j, m[i][j] * factor);
			}
		}
		return result;
	}

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
void transposeMatrix(std::vector<std::vector<double>>& matrix) {
	if (matrix.empty()) return;
	std::vector<std::vector<double>> transposed(matrix[0].size(), std::vector<double>(matrix.size()));
	for (size_t i = 0; i < matrix.size(); ++i) {
		for (size_t j = 0; j < matrix[0].size(); ++j) {
			transposed[j][i] = matrix[i][j];
		}
	}
	matrix = transposed;
}
std::vector<std::vector<double>> reshape_khan(const std::vector<double>& input, int rows, int cols) {
	std::vector<std::vector<double>> output(rows, std::vector<double>(cols));
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			output[i][j] = input[i + j * rows];
		}
	}
	return output;
}
void check_nan_khan(double value) {
	if (std::isnan(value)) {
		throw std::runtime_error("����ֵ��NaN");
	}
}
std::vector<double> vector_mul_double(std::vector<double> input, double m) {
	std::vector<double> res(input.size(),0.0);
	for (int i = 0; i < input.size(); i++) {
		res[i] = input[i] * m;
	}
	return res;
}
std::vector<double> one_hot_encode(int input) {
	std::vector<double> output(10, 0.0);
	if (input >= 0 && input < 10) {
		output[input] = 1.0;
	}
	return output;
}

double cross_entropy_loss(const std::vector<double>& predictions, const std::vector<double>& targets) {
	double loss = 0.0;
	for (size_t i = 0; i < predictions.size(); ++i) {
		loss -= targets[i] * std::log(predictions[i]);
	}
	return loss;
}
// Softmax
std::vector<double> softmax(const std::vector<double>& input) {
	std::vector<double> output(input.size());
	double max_val = *std::max_element(input.begin(), input.end());
	double sum = 0.0;

	for (size_t i = 0; i < input.size(); ++i) {
		output[i] = std::exp(input[i] - max_val);
		sum += output[i];
	}

	for (size_t i = 0; i < output.size(); ++i) {
		output[i] /= sum;
	}

	return output;
}
// Softmax
std::vector<double> softmax_derivative(const std::vector<double>& input,int lable) {
	std::vector<double> softmax_vals = softmax(input);
	softmax_vals[lable] = softmax_vals[lable]- 1;

	return softmax_vals;
}
void compareAndPrintVectors(const std::vector<double>& vec1, const std::vector<double>& vec2) {
	double norm1 = 0.0;
	double norm2 = 0.0;
	double maxDifference = 0.0;
	size_t maxDifferenceIndex = 0;

	for (size_t i = 0; i < vec1.size(); ++i) {
		double diff = std::abs(vec1[i] - vec2[i]);
		if (diff > maxDifference) {
			maxDifference = diff;
			maxDifferenceIndex = i;
		}
		norm1 += vec1[i] * vec1[i];
		norm2 += vec2[i] * vec2[i];
		check_nan_khan(norm1);
		check_nan_khan(norm2);

	}

	norm1 = std::sqrt(abs(norm1));
	norm2 = std::sqrt(abs(norm2));

	std::cout << "Norm of vector 1: " << norm1 << std::endl;
	std::cout << "Norm of vector 2: " << norm2 << std::endl;
	std::cout << "Max difference: " << maxDifference << " at index " << maxDifferenceIndex << std::endl;
}
VectorComparisonResult compareVectors(const std::vector<double>& vec1, const std::vector<double>& vec2) {
	VectorComparisonResult result;
	result.norm1 = 0.0;
	result.norm2 = 0.0;
	result.maxDifference = 0.0;
	result.maxDifferenceIndex = 0;

	for (size_t i = 0; i < vec1.size(); ++i) {
		double diff = std::abs(vec1[i] - vec2[i]);
		if (diff > result.maxDifference) {
			result.maxDifference = diff;
			result.maxDifferenceIndex = i;
		}
		result.norm1 += vec1[i] * vec1[i];
		result.norm2 += vec2[i] * vec2[i];
	}

	result.norm1 = std::sqrt(result.norm1);
	result.norm2 = std::sqrt(result.norm2);
	
	return result;
}
memory_t cover(std::vector<double>& image) {
	memory_t result;
	for (int i = 0; i < image.size(); i++) {
		result.push_back(reinterpret_cast<size_t&>(image[i]));
	}
	return result;
}

std::vector<double> vector_normalize(const std::vector<double>& input) {
	double sum = std::accumulate(input.begin(), input.end(), 0.0);
	double magnitude = std::sqrt(abs(sum));
	std::vector<double> output(input.size());
	

	if (magnitude == 0) {
		return output;
	}


	for (size_t i = 0; i < input.size(); ++i) {
		output[i] = input[i] / magnitude;
		check_nan_khan(output[i]);
	}

	return output;
}
void backward_propagation_test() {
	std::vector<System> state;
	state.emplace_back();
	auto i = System::add_register("i", UnsignedInteger, 2);
	auto j = System::add_register("j", UnsignedInteger, 2);
	(Hadamard_Int_Full("i"))(state);
	(Hadamard_Int_Full("j"))(state);
	auto index = System::add_register("index", UnsignedInteger, 10);//index=i*4+j
	indexCal("i", "j", "index", 4)(state);
	auto data1 = System::add_register("data1", UnsignedInteger, 5);
	auto data2 = System::add_register("data2", UnsignedInteger, 5);
	auto anc1 = System::add_register("anc1", Boolean, 1);
	auto anc2 = System::add_register("anc2", Boolean, 1);


	std::vector<size_t> vector1 = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	std::vector<size_t> vector2 = { 4,5,9,10 };
	qram_qutrit::QRAMCircuit qram1(4, 5);
	qram_qutrit::QRAMCircuit qram2(2, 5);
	qram1.set_memory(vector1);
	qram2.set_memory(vector2);


	StatePrint(StatePrintDisplay::Detail | 0)(state);

	QRAMLoad(&qram1, "index", "data1")(state);
	QRAMLoad(&qram2, "j", "data2")(state);
	//CondRot_P("data1", "anc1", conrotfunc_P, get_vector_F_form(vector1))(state);
	//CondRot_P("data2", "anc2", conrotfunc_P, get_vector_F_form(vector2))(state);
	QRAMLoad(&qram1, "index", "data1")(state);
	QRAMLoad(&qram2, "j", "data2")(state);
	killreg("index")(state);
	RemoveRegister("index")(state);
	RemoveRegister("data1")(state);
	RemoveRegister("data2")(state);

	StatePrint(StatePrintDisplay::Detail | 0)(state);
	PartialTraceSelect({ std::make_pair("anc1", 0) })(state);
	PartialTraceSelect({ std::make_pair("anc2", 0) })(state);
	StatePrint()(state);
	(Hadamard_Int_Full("j"))(state);
	StatePrint(StatePrintDisplay::Detail | 0)(state);
}
std::vector<double> compressToRange(const std::vector<double>& input) {
	double min_val = *std::min_element(input.begin(), input.end());
	double max_val = *std::max_element(input.begin(), input.end());

	std::vector<double> output(input.size());
	for (size_t i = 0; i < input.size(); i++) {
		output[i] = (input[i] - min_val) / (max_val - min_val) * 255.0;
	}
	return output;
}

double cal_gen(std::vector<double> vector_i_, std::vector<double> kernel_) {
	std::vector<double> vector_i = vectorappend(vector_i_);
	std::vector<double> kernel = vectorappend(kernel_);
	double norm = get_vector_F_form(vector_i);
	if (norm == 0 || get_vector_F_form(kernel) == 0) {
		return 0.0;
	}
	std::vector<System> state;
	state.emplace_back();

	qram_qutrit::QRAMCircuit qram1(qram_simulator::log2(vector_i.size()), 10);
	qram_qutrit::QRAMCircuit qram2(qram_simulator::log2(kernel.size()), 10);
	qram1.set_memory(cover(vector_i));
	qram2.set_memory(cover(kernel));

	auto r = System::add_register("q", UnsignedInteger, qram_simulator::log2(vector_i.size()));//pic�ڲ�����
	auto data = System::add_register("data", UnsignedInteger, 64);//���QRAM����
	auto anc = System::add_register("anc", Boolean, 1);//��QRAMLoadʱ����Ϊ���Ʊ���
	auto anc_cr = System::add_register("anc_cr", Boolean, 1);
	(Hadamard_Int_Full("q"))(state);
	(Hadamard_Int_Full("anc"))(state);
	QRAMLoad(&qram1, "q", "data").conditioned_by_nonzeros("anc")(state);
	Xgate_Bool("anc", 0)(state);
	QRAMLoad(&qram2, "q", "data").conditioned_by_nonzeros("anc")(state);
	Xgate_Bool("anc", 0)(state);

	CondRot_P("data", "anc_cr", conrotfunc_P, norm).conditioned_by_nonzeros("anc")(state);
	Xgate_Bool("anc", 0)(state);
	CondRot_P("data", "anc_cr", conrotfunc_P, get_vector_F_form(kernel)).conditioned_by_nonzeros("anc")(state);
	Xgate_Bool("anc", 0)(state);

	QRAMLoad(&qram1, "q", "data").conditioned_by_nonzeros("anc")(state);
	Xgate_Bool("anc", 0)(state);
	QRAMLoad(&qram2, "q", "data").conditioned_by_nonzeros("anc")(state);
	Xgate_Bool("anc", 0)(state);
	RemoveRegister("data")(state);

	(Hadamard_Int_Full("anc"))(state);

	double prob = 0;
	for (auto& s : state) {
		if (s.get(System::get("anc")).value == false && s.get(System::get("anc_cr")).value == false)
		{
			prob += abs_sqr(s.amplitude);
		}
	}
	double res = ((prob * vector_i.size()) * 2 - 1) * sqrt(norm) * sqrt(get_vector_F_form(kernel));

	//std::cout << res << std::endl;

	System::clear();
	return res;

}
double cal_class(std::vector<double> vector_i_, std::vector<double> kernel_) {
	int sz = vector_i_.size();
	if (vector_i_.size() != kernel_.size()) {
		std::cout<< "A = " << vector_i_.size() << " " << "B = " << kernel_.size() << std::endl;
		throw std::runtime_error("����֮��ά�Ȳ����");
	}
	double answer = 0.0;
	for (int i = 0; i < sz; i++) {
		answer += vector_i_[i] * kernel_[i];
	}
	return answer;
}
std::vector<double> back_cal(std::vector<double> AT, std::vector<double> y) {
	std::vector<System> state;
	state.emplace_back();
	std::vector<double> AT_a;
	AT_a = vectorappend(AT);
	auto i = System::add_register("i", UnsignedInteger, log2(AT_a.size()));
	(Hadamard_Int_Full("i"))(state);
	auto j = System::add_register("j", UnsignedInteger, 1);
	(Hadamard_Int_Full("j"))(state);
	auto data1 = System::add_register("data1", UnsignedInteger, 64);
	auto data2 = System::add_register("data2", UnsignedInteger, 64);
	auto anc1 = System::add_register("anc1", Boolean, 1);
	auto anc2 = System::add_register("anc2", Boolean, 1);
	StatePrint(StatePrintDisplay::Detail | 0)(state);
	qram_qutrit::QRAMCircuit qram1(log2(AT_a.size()), 10);
	qram_qutrit::QRAMCircuit qram2(1, 10);
	qram1.set_memory(cover(AT_a));
	qram2.set_memory(cover(y));
	QRAMLoad(&qram1, "i", "data1")(state);
	QRAMLoad(&qram2, "j", "data2")(state);
	CondRot_P("data1", "anc1", conrotfunc_P, get_vector_F_form(AT_a))(state);
	CondRot_P("data2", "anc2", conrotfunc_P, get_vector_F_form(y))(state);
	QRAMLoad(&qram1, "i", "data1")(state);
	QRAMLoad(&qram2, "j", "data2")(state);
	RemoveRegister("data1")(state);
	RemoveRegister("data2")(state);
	StatePrint(StatePrintDisplay::Detail | 0)(state);
	PartialTraceSelect({ std::make_pair("anc1", 0) })(state);
	PartialTraceSelect({ std::make_pair("anc2", 0) })(state);
	(Hadamard_Int_Full("j"))(state);
	StatePrint(StatePrintDisplay::Detail | 0)(state);
	std::vector<double> res;
	double unnorm = AT[0] * y[0];
	for (auto& s : state) {
		if (s.get(System::get("j")).value == 0) {
			res.push_back(sqrt(abs_sqr(s.amplitude)));
		}
	}
	unnorm = unnorm / res[0];
	for (int i = 0; i < res.size(); i++) {
		res[i] = res[i] * unnorm;
	}

	return res;


}
std::vector<double> back_cal_fake(std::vector<double> AT, std::vector<double> y) {
	std::vector<double> res(AT.size());
	for (int i = 0; i < AT.size(); i++) {
		res[i] = AT[i] * y[0];
	}
	return res;
}
std::vector<double> outer_product(const std::vector<double>& a, const std::vector<double>& b) {
	std::vector<double> result(a.size() * b.size());
	for (size_t i = 0; i < a.size(); ++i) {
		for (size_t j = 0; j < b.size(); ++j) {
			result[i * b.size() + j] = a[i] * b[j];
		}
	}
	return result;
}
struct MatrixToUnitary
{
	int addr_reg_i, addr_reg_j;
	int data_reg;
	int work, ancilla;

	MatrixToUnitary(int addr_reg_i_, int addr_reg_j_, int data_reg_, int work_, int ancilla_)
		: addr_reg_i(addr_reg_i_), addr_reg_j(addr_reg_j_), data_reg(data_reg_), work(work_), ancilla(ancilla_)
	{}

	MatrixToUnitary(std::string addr_reg_i_, std::string addr_reg_j_, std::string data_reg_, std::string work_, std::string ancilla_)
	{
		work = System::get(work_);
		addr_reg_i = System::get(addr_reg_i_);
		addr_reg_j = System::get(addr_reg_j_);
		data_reg = System::get(data_reg_);
		ancilla = System::get(ancilla_);
	}

	std::vector<std::vector<int>> operator()(std::vector<System>& state);

};
std::vector<double> getWback(std::vector<double> Y, std::vector<double> A) {
	std::vector<double> res;
	for (int i = 0; i < A.size(); i++) {
		std::vector<double> resi;
		std::vector<double> A_(2);
		A_[0] = A[i];
		//resi = back_cal(Y, A_);
		resi = back_cal_fake(Y, A_);
		for (int j = 0; j < resi.size(); j++) {
			res.push_back(resi[j]);
		}
	}
	return res;
}

//Multi-channel multi-scale wide convolution quantum convolutional neural network
std::vector<double> CapReLU(std::vector<double> input) {
	std::vector<double> res(input.size());
	for (int i = 0; i < input.size(); i++) {
		if (input[i] < -10) {
			res[i] = -10;
		}

		else if (input[i] < 10) {
			res[i] = input[i];
		}
		else {
			res[i] = 10;
		}
	}
	return res;
}
std::vector<double> CapReLU_d(std::vector<double> input) {
	std::vector<double> res(input.size());
	for (int i = 0; i < input.size(); i++) {
		if (input[i] < -10) {
			res[i] = 0;
		}
		else if (input[i] < 10) {
			res[i] = 1;
		}
		else {
			res[i] = 0;
		}
	}
	return res;
}
std::vector<int> classify(std::vector<double> outputs) {
	double max = 0;
	int loc = 0;
	for (int i = 0; i < outputs.size(); i++) {
		if (outputs[i] > max) {
			max = outputs[i];
			loc = i;
		}
	}
	std::vector<int> results = { 0,0,0,0,0,0,0,0,0,0 };
	results[loc] = 1;
	return results;
}
int classify_loc(std::vector<double> outputs) {
	double max = 0;
	int loc = 0;
	for (int i = 0; i < outputs.size(); i++) {
		if (outputs[i] > max) {
			max = outputs[i];
			loc = i;
		}
	}


	return loc;
}
std::vector<double> cal_loss(std::vector<int> results, int lable,double m) {
	std::vector<int> b = results;
	b[lable] = b[lable] - 1;
	std::vector<double> a(10);
	for (int i = 0; i < b.size(); i++) {
		a[i] = (double)b[i]*m;
	}
	return a;
}
std::vector<double> cal_gra(std::vector<double> loss, std::vector<double> W) {
	std::vector<double> output(W.size() / loss.size());
	for (int i = 0; i < output.size(); i++) {
		std::vector<double> auc(loss.size());
		for (int j = 0; j < loss.size(); j++) {
			auc[j] = W[i + j * W.size() / loss.size()];
		}
		output[i] = cal_gen(loss, auc);
	}
	return output;
}
std::vector<double> getWbac_conv(std::vector<double> loss, std::vector<double> pic) {
	std::vector<double> pic_2 = CNN::img2col(pic, 4);
	std::vector<double> output = getWback(loss, pic_2);
	return output;
}
int khan_check(int input) {
	size_t a = 2;
	while (a < input) {
		a = a * 2;
	}
	int b = (int)qram_simulator::log2(a);
	return a;
}
double check_correct(std::vector<double> a, std::vector<double> k) {
	double result = 0;
	for (int i = 0; i < a.size(); i++) {
		result += a[i] * k[i];
	}
	return result;
}
void printvector(std::vector<double> vec) {
	for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << ",";
	}
	std::cout << std::endl;
}
void printvector_(std::vector<int> vec) {
	for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << " ";
	}
	std::cout << std::endl;
}
std::vector<std::vector<double>> reshape_to_square(const std::vector<double>& input) {
	int size = input.size();
	int dim = std::sqrt(size); // ��������������
	std::vector<std::vector<double>> output(dim, std::vector<double>(dim));
	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			output[i][j] = input[i * dim + j];
			std::cout << output[i][j] << " ";
		}
		std::cout << std::endl;
	}
	return output;
}
void print2d(std::vector<double> pic) {
	for (int i = 0; i < sqrt(pic.size()); i++) {
		for (int j = 0; j < sqrt(pic.size()); j++) {
			std::cout << pic[i * sqrt(pic.size()) + j] << " ";
		}
		std::cout << std::endl;
	}
}

std::vector<double> pow2b(std::vector<double> pic) {
	int i = 2;
	while (i < pic.size()) {
		i = i * 2;
	}
	
	std::vector<double> pic_2(i);
	for (int j = 0; j < pic.size(); j++) {
		pic_2[j] = pic[j];
	}
	for (int k = pic.size(); k < i; k++) {
		pic_2[k] = 0;
	}
	return pic_2;
}
class QuantumConvolutionalLayer {
public:
	int input_width;
	int kernel_size;
	std::vector<double> kernel;
	std::vector<double> input_cache;
	QuantumConvolutionalLayer(int input_width,
		int kernel_size) :
		input_width(input_width),
		kernel_size(kernel_size){
		std::default_random_engine generator;
		std::normal_distribution<double> distribution(0.0, 1.0);
		kernel.resize(kernel_size * kernel_size);
		for (int i = 0; i < kernel.size(); i++) {
			double value = distribution(generator);
			// ��ֵ���ŵ�-1��1�ķ�Χ
			value = std::max(-1.0, std::min(1.0, value));
			kernel[i] = value;
		}
		//this->kernel = vector_normalize(kernel);
	};

	auto set_kernels(std::vector<double> kernels1) {
		this->kernel = kernels1;
	}
	std::vector<double> get_kernels() {
		return this->kernel;
	}
	auto print_kernels() {
		printvector(this->kernel);
	}
	std::vector<double> get_p() {
		return kernel;
	}
	// Forward pass through the layer
	std::vector<double> forward(std::vector<double> input) {
		this->input_cache = input;
		// Convert the input to a matrix using the img2col technique
		// (kernel_size*kernel_size)
		// ->(kernel_size*kernel_size*(input_width-kernel_size+1)*(input_width-kernel_size+1))
		std::vector<double> pic_col = CNN::img2col(input, kernel_size);

		//std::cout << "pic_col.size()=" << pic_col.size() << std::endl;
		std::vector<double> results(pic_col.size() / (kernel_size * kernel_size));

		for (int i = 0; i < pic_col.size() / (kernel_size * kernel_size); i++) {
			std::vector<double> vector_i_1(kernel_size * kernel_size);

			for (int j = 0; j < (kernel_size * kernel_size); j++) {
				vector_i_1[j] = pic_col[i * (kernel_size * kernel_size) + j];
			}
			double res = cal_class(vector_i_1, this->kernel);
			double class_res = check_correct(vector_i_1, this->kernel);
			if (class_res - res > 0.01) {
				std::cout << "compute incorrect!!!!!!!!!!!!!!!" << std::endl;
				std::cout << "the real answer is :" << class_res << "the quantum answer is :" << res<<std::endl;
			}
			results[i] = res;
		}

		return results;
	}


	// Backward pass through the layer
	std::vector<double> backward(std::vector<double> input,
		std::vector<double> grad_output,
		double learning_rate) {
		// grad_output:1*(input_width-kernel_size+1)*(input_width-kernel_size+1)
		// img2col(input):(input_width-kernel_size+1)*(input_width-kernel_size+1)*kernel_size*kernel_size
		// kernel update amount= grad_output*img2col(input)
		
		// Convert the input to A using the img2col technique
		std::vector<double> pic_col = CNN::img2col(input, kernel_size);
		auto flag = "classic";
		std::vector<double> grad;
		if (flag == "classic") {
			grad = outer_product(grad_output, pic_col);
		}
		else if(flag == "quantum") {
			grad = getWback(grad_output, pic_col);
		}
		else {
			std::cout << "flag error!" << std::endl;
		}
		

		// Update the kernels 
		for (int i = 0; i < kernel_size * kernel_size; i++) {
			kernel[i] = kernel[i] - learning_rate * grad[i];
			//std::cout << "update successfully!the value is:" << learning_rate * grad[i] << std::endl;
		}
	
		//calculate the loss backward
		std::vector<double> back_loss = forward_(paddingforback(grad_output, kernel_size));

		return back_loss;
	}
	std::vector<double> get_input() {
		return input_cache;
	}
private:
	std::vector<double> forward_(std::vector<double> input) {
		this->input_cache = input;
		// Convert the input to a matrix using the img2col technique
		// (kernel_size*kernel_size)
		// ->(kernel_size*kernel_size*(input_width-kernel_size+1)*(input_width-kernel_size+1))
		std::vector<double> pic_col = CNN::img2col(input, kernel_size);

		//std::cout << "pic_col.size()=" << pic_col.size() << std::endl;
		std::vector<double> results(pic_col.size() / (kernel_size * kernel_size));

		for (int i = 0; i < pic_col.size() / (kernel_size * kernel_size); i++) {
			std::vector<double> vector_i(kernel_size * kernel_size);

			for (int j = 0; j < (kernel_size * kernel_size); j++) {
				vector_i[j] = pic_col[i * (kernel_size * kernel_size) + j];
			}
			double res = cal_gen(vector_i, reversekernel(this->kernel));
			double class_res = check_correct(vector_i, reversekernel(this->kernel));
			if (class_res - res > 0.01) {
				std::cout << "compute incorrect!!!!!!!!!!!!!!!" << std::endl;
			}
			results[i] = res;

		}
		return results;
	}

};
class QuantumFullyConnectedLayer {
public:
	// Only designed for M*1 input and N*1 output
	int input_weight;
	int output_weight;
	//input   : M*1 vector
	std::vector<double> input;
	//weight  : M*N matrix
	std::vector<double> weight;
	//biase   : N*1 vector
	std::vector<double> biase;
	//output  : N*1 vector
	std::vector<double> output;
	std::vector<double> cached_input;
	std::vector<double> cached_grad_weights;

	QuantumFullyConnectedLayer(int input_weight, int output_weight) :
		input_weight(input_weight), output_weight(output_weight) {
		// Initialize weights and biases with 0,weight = (input_height*input_weight,std::vector<double>(output_height*output_weight))
		std::default_random_engine generator;
		std::normal_distribution<double> distribution(0.0, 1.0);
		weight.resize(input_weight * output_weight);
		for (int i = 0; i < weight.size(); i++) {
			double value = distribution(generator);
			// ��ֵ���ŵ�-1��1�ķ�Χ
			value = std::max(-1.0, std::min(1.0, value));
			weight[i] = value;
		}
		for (int j = 0; j < biase.size(); j++) {
			double value = distribution(generator);
			// ��ֵ���ŵ�-1��1�ķ�Χ
			value = std::max(-1.0, std::min(1.0, value));
			biase[j] = value;
		}
	}
	std::vector<double> get_p() {
		return weight;
	}
	void printweight() {
		for (int i = 0; i < input_weight; i++) {
			for (int j = 0; j < output_weight; j++) {
				std::cout << weight[i * output_weight + j] << " ";
			}
			std::cout << std::endl;
		}
	}
	auto get_weight() {
		return weight;
	}
	std::vector<double> forward(std::vector<double> input) {
		std::vector<double> results(output_weight);
		for (int i = 0; i < output_weight; i++) {
			std::vector<double> vector_i_1(input_weight);

			for (int j = 0; j < input_weight; j++) {
				vector_i_1[j] = weight[i * input_weight + j];
			}
			//quantum
			//double res = cal_gen(vector_i_1, input);
			//class
			double res = cal_class(vector_i_1, input);
			
			/*double class_res = check_correct(vector_i_1, input);
			
			if (class_res - res > 0.01) {
				std::cout << "compute incorrect!!!!!!!!!!!!!!!" << std::endl;
				std::cout << "the real answer is :" << class_res << "the quantum answer is :" << res << std::endl;
			}*/
			results[i] = res;
		}
		std::cout<<"------------------------------------------------"<<std::endl;
		std::cout<<"FCForward Layer Information:"<<std::endl;
		std::cout<<"Weight:"<<std::endl;
		//printweight();
		return results;
	}

	std::vector<double> backward(std::vector<double> input,
		std::vector<double> grad, double learning_rate) {

		std::vector<std::vector<double>> mat = reshape_khan(this->weight, input_weight, output_weight);
		std::vector<double> loss(input_weight);
		//transposeMatrix(mat);

		for (int i = 0; i < input_weight; i++) {

			double res = cal_class(grad, mat[i]);
			/*
			double res = cal_gen(v, grad);
			double class_res = check_correct(v, grad);
			if (class_res - res > 0.01) {
				std::cout << "compute incorrect!!!!!!!!!!!!!!!" << std::endl;
			}*/
			loss[i] = res;
		}

		auto flag = "classic";
		std::vector<double> gra;
		if (flag == "classic") {
			gra = outer_product(grad, input);
		}
		else if (flag == "quantum") {
			gra = getWback(grad, input);;
		}

		for (int i = 0; i < input_weight * output_weight; i++) {
			weight[i] = weight[i] - learning_rate * gra[i];
			//std::cout << "update successfully!the value is:" << learning_rate * gra[i];
		}

		//printvector(gra);
		std::cout << std::endl;
		//printvector(weight);

		return loss;
	}
};
class QuantumFullyConnectedLayer_m {
public:
	// Only designed for M*1 input and N*1 output
	int input_weight;
	int output_weight;
	//input   : M*1 vector
	khan_Matrix input;

	//weight  : M*N matrix
	khan_Matrix weight;
	//biase   : N*1 vector
	std::vector<double> biase;
	//output  : N*1 vector
	khan_Matrix output;

	QuantumFullyConnectedLayer_m(int input_weight, int output_weight) :
		input_weight(input_weight), output_weight(output_weight) 
		,input(1, input_weight),weight(input_weight,output_weight),
		output(1, output_weight)
	{

		// Initialize weights and biases with 0,weight = (input_height*input_weight,std::vector<double>(output_height*output_weight))
		std::default_random_engine generator;
		std::normal_distribution<double> distribution(0.0, 1.0);
		for(int i = 0;i<input_weight;i++)
			for (int j = 0; j < output_weight; j++) {
				double value = distribution(generator);
				// ��ֵ���ŵ�-1��1�ķ�Χ
				value = std::max(-1.0, std::min(1.0, value));
				weight.setElement(i,j,value);
			}
		
		for (int j = 0; j < biase.size(); j++) {
			double value = distribution(generator);
			// ��ֵ���ŵ�-1��1�ķ�Χ
			value = std::max(-1.0, std::min(1.0, value));
			biase[j] = value;
		}
	}

	auto get_weight() {
		return weight;
	}
	khan_Matrix forward(khan_Matrix input_) {
		this->input = input_;
		khan_Matrix res(1,output_weight);
		res = input_.multiply(weight);
		return res;
	}

	khan_Matrix backward(khan_Matrix input,
		khan_Matrix grad, double learning_rate) {
		// ���㷴�򴫲���ǰһ���Loss
		khan_Matrix loss(1,input_weight);
		loss = grad.multiply(weight.transpose());
		// ������һ���weight������
		khan_Matrix gra(input_weight,output_weight);
		gra = input.transpose().multiply(grad);
		this->weight = weight.subtract(gra.scale(learning_rate));
		return loss;
	}
};
class PoolingLayer {
public:
	int pooling_size;
};
auto TestQuantumConvLayer() {
	QuantumConvolutionalLayer c(4, 2);
	std::vector<double> input = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	std::vector<double> forward_output = c.forward(input);
	std::vector<double> grad = { 0.1,0.2,0.3,0.4,0.5,0.8,1.2,2.5,1.3 };
	std::vector<double> backward_output = c.backward(c.get_input(), grad, 0.01);
	printvector(forward_output);
	printvector(backward_output);
};

auto TestQuantumFullyConnectedLayer() {
	QuantumFullyConnectedLayer c(16, 8);
	std::vector<double> input = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	std::vector<double> forward_output = c.forward(input);
	std::vector<double> grad = { 1,2,3,4,5,6,7,8 };
	std::vector<double> backward_output = c.backward(input, grad, 0.01);
	printvector(forward_output);
	printvector(backward_output);
}

//convert std::vector<std::vector<double>> to std::vector<double>
std::vector<double> convert(std::vector<std::vector<double>> image) {
	std::vector<double> result;
	for (int i = 0; i < image.size(); i++) {
		for (int j = 0; j < image[i].size(); j++) {
			result.push_back(image[i][j]);
		}
	}
	return result;
}
struct QRAMLoad_Amptitude
{
	qram_qutrit::QRAMCircuit* qram;
	int addr_reg_i, addr_reg_j;
	int data_reg;
	int ancilla;

	QRAMLoad_Amptitude(qram_qutrit::QRAMCircuit* qram_, int addr_reg_i_, int addr_reg_j_,int data_reg_, int ancilla_)
		: qram(qram_), addr_reg_i(addr_reg_i_), addr_reg_j(addr_reg_j_), data_reg(data_reg_), ancilla(ancilla_)
	{}

	QRAMLoad_Amptitude(qram_qutrit::QRAMCircuit* qram_, std::string addr_reg_i_, std::string addr_reg_j_, std::string data_reg_, std::string ancilla_)
	{
		qram = qram_;
		addr_reg_i = System::get(addr_reg_i_);
		addr_reg_j = System::get(addr_reg_j_);
		data_reg = System::get(data_reg_);
		ancilla = System::get(ancilla_);
	}

	void operator()(std::vector<System>& state);

};
void QRAMLoad_Amptitude::operator()(std::vector<System>& state) {

	int M = pow2(System::size_of(addr_reg_j));
	double f_norm = 0.;
	Init_Unsafe("ancilla", reinterpret_cast<size_t&>(f_norm));
	for (auto& s : state) {
		double item = reinterpret_cast<double&>(qram->memory[s.get(addr_reg_i).value * M + s.get(addr_reg_j).value]);
		f_norm += item * item;
	}
	f_norm = sqrt(f_norm);

	for (auto& s : state) {
		double item = reinterpret_cast<double&>(qram->memory[s.get(addr_reg_i).value * M + s.get(addr_reg_j).value]);
		s.amplitude = item / f_norm;
	}
}
struct Image {
	char label;  // ͼ��ı�ǩ��0-9��
	std::vector<double> data;  // ͼ�����ݣ�784��������ÿ��������ֵ��0-255֮�䣩
};
std::vector<Image> get_pic_with_index() {
	int kSize = 28;
	std::vector<Image> images;
	std::string image_file = "D:\\CNN_C++\\t10k-images.idx3-ubyte";
	std::string label_file = "D:\\CNN_C++\\t10k-labels.idx1-ubyte";
	std::ifstream image_stream(image_file, std::ios::binary);
	std::ifstream label_stream(label_file, std::ios::binary);
	if (!image_stream || !label_stream) {
		throw std::invalid_argument("There is no dataset in the specified path");
	}
	// skip some gabage
	image_stream.seekg(16);
	label_stream.seekg(8);
	while (image_stream && label_stream) {
		Image image;
		image.data.resize(kSize * kSize);

		// read lable
		label_stream.read(reinterpret_cast<char*>(&image.label), 1);

		// read pic data
		for (int i = 0; i < kSize * kSize; ++i) {
			unsigned char pixel = 0;
			image_stream.read(reinterpret_cast<char*>(&pixel), 1);
			image.data[i] = (double)pixel;
		}
		images.push_back(image);
	}
	return images;
}


//print all the elements of std::vector<int>
void test_get_pic() {
	std::vector<Image> label = get_pic_with_index();
	std::vector<double> pic = label[45].data;
	printvector(pic);
	
}

int get_local(std::vector<int> res) {
	for (int i = 0; i < res.size(); i++) {
		if (res[i] != 0) {
			return i;
		}
	}
	return -1;
}
//get the max value
double max_value(std::vector<double> input) {
	double m = 0;
	for (int i = 0; i < input.size(); i++) {
		if (input[i] > m) {
			m = input[i];
		}
	}
	return m;
}
void ConvCorrectTest() {
	QuantumConvolutionalLayer layer(28, 4);
	std::vector<Image> img = get_pic_with_index();
	std::vector<double> pic = img[1].data;

	std::vector<double> test_kernel = { 1,3,5,0,3,0,4,0,12,4,3,7,6,2,9,0 };
	test_kernel = vector_normalize(test_kernel);

}
void check_back() {
	QuantumConvolutionalLayer layer0(28, 4);
	QuantumFullyConnectedLayer layer1(25*25, 50);
	QuantumFullyConnectedLayer layer2(50, 10);
	std::vector<Image> img = get_pic_with_index();

	for (int i = 0; i < 3000; i++) {
		std::vector<double> original_weight1 = layer1.get_weight();
		std::vector<double> original_weight2 = layer2.get_weight();

		std::vector<double> pic = vector_normalize(img[i].data);

		int lable = (int)img[i].label;
		auto layer0_output = layer0.forward(pic);
		auto layer1_output = layer1.forward(layer0_output);
		auto layer2_output = layer2.forward(layer1_output);

		std::vector<double> res = softmax(layer2_output);
		double loss = cross_entropy_loss(res, one_hot_encode(lable));
		std::vector<double> loss_b = softmax_derivative(layer2_output,lable);
		
		auto layer2_b_output = layer2.backward(layer1_output, loss_b, 0.01);
		auto layer1_b_output = layer1.backward(layer0_output, layer2_b_output, 0.01);
		auto layer0_b_output = layer0.backward(pic, layer1_b_output, 0.01);


		std::vector<double> operate_weight1 = layer1.get_weight();
		std::vector<double> operate_weight2 = layer2.get_weight();

		std::cout << "=========================================================" << std::endl;

		std::cout << "ȫ���Ӳ�ĸ��º����Ϊ��" << std::endl;
		compareAndPrintVectors(original_weight1, operate_weight1);
		compareAndPrintVectors(original_weight2, operate_weight2);
	}

	int counter = 0;
	for (int i = 5000; i < 5100; i++) {


		int lable = (int)img[i].label;
		std::vector<double> pic = vector_normalize(img[i].data);


		auto layer0_output = layer0.forward(pic);
		auto layer1_output = layer1.forward(layer0_output);
		auto layer2_output = layer2.forward(layer1_output);

		std::vector<double> res = softmax(layer2_output);

		int res_ = classify_loc(layer2_output);
		if (res_ == lable) {
			counter++;
		}
	}
	std::cout << "׼ȷ�ʣ�" << counter / 100.0 << std::endl;

}


void check_fclayer() {
	QuantumFullyConnectedLayer_m layer1(28*28, 50);
	QuantumFullyConnectedLayer_m layer2(50, 10);
	std::vector<Image> img = get_pic_with_index();
	for (int i = 0; i < 5000; i++) {
		std::vector<double> original_weight1 = layer1.get_weight().toVector();
		std::vector<double> original_weight2 = layer2.get_weight().toVector();

		std::vector<double> pic = vector_normalize(img[i].data);

		int lable = (int)img[i].label;
		khan_Matrix pic_m(1, 28*28, pic);
		khan_Matrix layer1_output = layer1.forward(pic_m);

		khan_Matrix layer2_output = layer2.forward(layer1_output);

		std::vector<double> res = softmax(layer2_output.toVector());
		double loss = cross_entropy_loss(res, one_hot_encode(lable));
		std::vector<double> loss_b = softmax_derivative(layer2_output.toVector(), lable);
		
		khan_Matrix loss_b_m(1, 10, loss_b);
		auto layer2_b_output = layer2.backward(layer1_output, loss_b_m, 0.01);

		auto layer1_b_output = layer1.backward(pic_m, layer2_b_output, 0.01);



		std::vector<double> operate_weight1 = layer1.get_weight().toVector();
		std::vector<double> operate_weight2 = layer2.get_weight().toVector();

		std::cout << "=========================================================" << std::endl;
		std::cout << i << "/1000" << std::endl;
		std::cout << "ȫ���Ӳ�ĸ��º����Ϊ��" << std::endl;
		std::cout << "��һ��ı仯����" << std::endl;
		compareAndPrintVectors(operate_weight1, original_weight1);
		std::cout << "�ڶ���ı仯����" << std::endl;
		compareAndPrintVectors(operate_weight2, original_weight2);
	}

	int counter = 0;
	for (int i = 5000; i < 5100; i++) {
		std::vector<double> pic = vector_normalize(img[i].data);
		int lable = (int)img[i].label;


		khan_Matrix pic_m(1, 28 * 28, pic);
		auto layer1_output = layer1.forward(pic_m);


		auto layer2_output = layer2.forward(layer1_output);

		std::vector<double> res = softmax(layer2_output.toVector());
		int res1 = classify_loc(res);
		if (res1 == lable) {
			counter++;
		}
	}
	std::cout << "׼ȷ�ʣ�" << counter / 100.0 << std::endl;

}

void onelayer_test() {
	QuantumFullyConnectedLayer_m layer1(28 * 28, 10);
	std::vector<Image> img = get_pic_with_index();
	for (int i = 0; i < 5000; i++) {
		std::vector<double> original_weight1 = layer1.get_weight().toVector();


		std::vector<double> pic = img[i].data;

		int lable = (int)img[i].label;
		khan_Matrix pic_m(1, 28 * 28, pic);
		khan_Matrix layer1_output = layer1.forward(pic_m);
		std::vector layer1_v = layer1_output.toVector();


		std::vector<double> res = softmax(layer1_v);
		double loss = cross_entropy_loss(res, one_hot_encode(lable));
		std::vector<double> loss_b = softmax_derivative(layer1_v, lable);

		khan_Matrix loss_b_m(1, 10, loss_b);
		auto layer2_b_output = layer1.backward(pic_m, loss_b_m, 0.01);




		std::vector<double> operate_weight1 = layer1.get_weight().toVector();

		std::cout << "=========================================================" << std::endl;
		std::cout << i << "/1000" << std::endl;
		std::cout << "ȫ���Ӳ�ĸ��º����Ϊ��" << std::endl;
		std::cout << "��һ��ı仯����" << std::endl;
		compareAndPrintVectors(operate_weight1, original_weight1);

	}

	int counter = 0;
	for (int i = 5000; i < 5100; i++) {
		std::vector<double> pic = img[i].data;
		int lable = (int)img[i].label;


		khan_Matrix pic_m(1, 28 * 28, pic);
		auto layer1_output = layer1.forward(pic_m);
		std::vector layer1_v = layer1_output.toVector();


		std::vector<double> res = softmax(layer1_v);
		int res1 = classify_loc(res);
		if (res1 == lable) {
			counter++;
		}
	}
	std::cout << "׼ȷ�ʣ�" << counter / 100.0 << std::endl;

}
void khan_CNN_new() {
	QuantumConvolutionalLayer layer1(28, 4);
	QuantumFullyConnectedLayer layer2(25 * 25, 100);
	QuantumFullyConnectedLayer layer3(100, 10);
	std::vector<Image> img = get_pic_with_index();
	for (int i = 1; i < 3; i++) {

		std::vector<double> pic = img[i].data;
		int lable = (int)img[i].label;
		//printvector(pic);
		std::cout << "------------------------" << i << "-------------------" << std::endl;
		auto layer1_output = layer1.forward(pic);
		auto layer2_output = layer2.forward(layer1_output);
		auto layer3_output = layer3.forward(layer2_output);
		//printvector(layer1_output);
		std::cout << "-------------------------------------------" << std::endl;
		//printvector(layer2_output);
		std::cout << "-------------------------------------------" << std::endl;
		//printvector(layer3_output);
		std::vector<int> res = classify(layer3_output);
		std::vector<double> loss = cal_loss(res, lable, 1);
		std::cout<< "loss:" << std::endl;
		printvector(loss);
		auto layer3_b_output = layer3.backward(layer2_output, loss, 0.01);
		auto layer2_b_output = layer2.backward(layer1_output, layer3_b_output, 0.01);
		layer1.backward(pic, layer2_b_output, 0.01);
		std::ofstream outfile1;
		std::ofstream outfile2;
		std::ofstream outfile3;
		std::string a = "conv";
		std::string b = "fc1";
		std::string c = "fc2";
		// ��a��������
		std::string in = std::to_string(i);
		a.append(in);
		b.append(in);
		c.append(in);
		a.append(".txt");
		b.append(".txt");
		c.append(".txt");
		outfile1.open(a);
		std::vector<double> ap = layer1.get_p();
		for (auto i : ap){
			outfile1 << i << ' ';
		}
		outfile1.close();

		outfile2.open(b);
		std::vector<double> bp = layer2.get_p();
		for (auto i : ap) {
			outfile2 << i << ' ';
		}
		outfile2.close();

		outfile3.open(c);
		std::vector<double> cp = layer3.get_p();
		for (auto i : ap) {
			outfile3 << i << ' ';
		}
		outfile3.close();

		std::cout << " prog:" << i << "/1000" << std::endl;
	}
	
	int counter = 0;
	for (int i = 1000; i < 1100; i++) {

		std::vector<double> pic = img[i].data;
		int lable = (int)img[i].label;
		//printvector(pic);
		//std::cout << "-------------------------------------------" << std::endl;
		auto layer1_output = compressToRange(layer1.forward(pic));
		auto layer2_output = compressToRange(layer2.forward(layer1_output));
		auto layer3_output = compressToRange(layer3.forward(layer2_output));
		//printvector(layer1_output);
		//std::cout << "-------------------------------------------" << std::endl;
		//printvector(layer2_output);
		//std::cout << "-------------------------------------------" << std::endl;
		//printvector(layer3_output);
		std::vector<int> res = classify(layer3_output);
		if (get_local(res) == lable) {
			counter++;
		}
		//double acc = (double)counter / 100;
		//std::cout << "acc = " << acc;
	}

}
auto test_double_CR() {
	std::vector<double> a = { 0.3,0.4,0.5,0.6 };

	std::vector<System> state;
	state.emplace_back();
	auto i = System::add_register("i", UnsignedInteger, 2);
	auto data = System::add_register("data", UnsignedInteger, 64);
	auto anc = System::add_register("anc", Boolean, 1);
	(Hadamard_Int_Full("i"))(state);
	qram_qutrit::QRAMCircuit qram1(2, 16);
	qram1.set_memory(cover(a));
	double x = reinterpret_cast<double&>(qram1.memory[0]);
	std::cout << qram1.memory[0] << "   " << x << "     "<< get_vector_F_form(a)<<std::endl;
	QRAMLoad(&qram1, "i", "data")(state);
	CondRot_P("data", "anc", conrotfunc_P, get_vector_F_form(a))(state);
	StatePrint()(state);
}
auto test_b() {
	std::vector<double> a = { 0.1,2,3,4,5,3,2,1,4,8 };
	std::vector<double> b = { 10,0 };
	std::vector<double> res;
	res = back_cal(a, b);
	printvector(res);
}
void testApload() {
	std::vector<System> state;
	state.emplace_back();

	auto p = System::add_register("p", UnsignedInteger, 2);
	auto r = System::add_register("r", UnsignedInteger, 2);
	auto i = System::add_register("i", UnsignedInteger, 4);
	(Hadamard_Int_Full("p"))(state);
	(Hadamard_Int_Full("r"))(state);
	indexCal("p", "r", "i", 4)(state);
	qram_qutrit::QRAMCircuit qram1(4, 10);
	std::vector<double> vec = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	std::vector<size_t> vector1 = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	qram1.set_memory(cover(vec));
	auto data = System::add_register("data", UnsignedInteger, 64);
	auto anc_cr = System::add_register("anc_cr", Boolean, 1);

	QRAMLoad(&qram1, "i", "data")(state);
	StatePrint(StatePrintDisplay::Detail | 0)(state);
	CondRot_P("data", "anc_cr", conrotfunc_P, get_vector_F_form(vec))(state);
	killreg("i")(state);
	RemoveRegister("i")(state);

	//AmplitudeLoad(p, r, data, 4)(state);

	StatePrint(StatePrintDisplay::Detail | 0)(state);
}

auto test_sparse() {
	size_t addr_size = 4;
	size_t data_size = 4;
	size_t rational_size = 6;
	std::vector<System> system_states;
	qram_qutrit::QRAMCircuit* qram;
	std::string qram_version;
	qram = new qram_qutrit::QRAMCircuit(addr_size, data_size);
	std::vector<double> vec = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	std::vector<size_t> veca = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	qram->set_memory(veca);
	auto div = System::add_register("addr_parent", UnsignedInteger, addr_size + 1);
	System::add_register("p", UnsignedInteger, 1);
	System::add_register("q", UnsignedInteger, 1);
	System::add_register("addr_child", UnsignedInteger, addr_size + 1);
	System::add_register("data_parent", SignedInteger, data_size);
	System::add_register("data_child", SignedInteger, data_size);
	System::add_register("temp_bit", Boolean, 1);
	System::add_register("div_result", Rational, rational_size);

	system_states.emplace_back();

	QRAMLoad::version = qram_version;

	Xgate_Bool("addr_parent", 0)(system_states);
	for (size_t k = 0; k < addr_size - 1; ++k) {
		std::cout<<"============================================================"<<k<<"========================================================="<<std::endl;
		StatePrint(StatePrintDisplay::Detail | 0)(system_states);
		Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(system_states);
		QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
		QRAMLoad(qram, "addr_child", "data_child")(system_states);
		StatePrint(StatePrintDisplay::Detail | 0)(system_states);
		Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(system_states);
		CondRot_Rational_Bool("div_result", "temp_bit")(system_states);
		Div_Sqrt_Arccos_Int_Int("data_child", "data_parent", "div_result")(system_states);
		QRAMLoad(qram, "addr_child", "data_child")(system_states);
		QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
		Mult_UInt_ConstUInt("addr_parent", 2, "addr_child")(system_states);
		ShiftLeft("addr_parent", 1)(system_states);
		Swap_Bool_Bool("temp_bit", 0, "addr_parent", 0)(system_states);
		StatePrint(StatePrintDisplay::Detail | 0)(system_states);
		std::cout << "===========================================================================================================================" << std::endl;
	}
	StatePrint(StatePrintDisplay::Detail | 0)(system_states);
	ShiftLeft("addr_parent", 1)(system_states);
	Assign("addr_parent", "addr_child")(system_states);
	Xgate_Bool("addr_child", 0)(system_states);
	QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
	QRAMLoad(qram, "addr_child", "data_child")(system_states);
	GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(system_states);
	CondRot_Rational_Bool("div_result", "temp_bit")(system_states);
	GetRotateAngle_Int_Int("data_parent", "data_child", "div_result")(system_states);
	QRAMLoad(qram, "addr_child", "data_child")(system_states);
	QRAMLoad(qram, "addr_parent", "data_parent")(system_states);
	Xgate_Bool("addr_child", 0)(system_states);
	Assign("addr_parent", "addr_child")(system_states);
	Swap_Bool_Bool("temp_bit", 0, "addr_parent", 0)(system_states);
	Xgate_Bool("addr_parent", addr_size)(system_states);

	SortByKey("addr_parent")(system_states);
}
int main() {

	//khan_CNN();
	//test_cal();
	//test_b();
	//TestQuantumConvLayer();
	//TestQuantumFullyConnectedLayer();
	//khan_CNN_new();
	//check_back();
	//check_fclayer();
	//onelayer_test();
	//testApload();
	//test_double_CR();
	test_sparse();
	return 0;
}

#else
int main() {
	return 0;
}

#endif




