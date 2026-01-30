#include "PoiseuilleTestUtils.h"


void testBlockEncoding_U_plus_minus(size_t nqubit)
{
	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("overflow", UnsignedInteger, 1);
	
	fmt::print("Test Block Encoding U+:\n");
	{	
		bool is_full = false;
		bool is_dag = false;
		PlusOneAndOverflow block_enc("main_reg", "overflow");
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "overflow", 
			is_full, is_dag);
		fmt::print("{}\n", ret.to_string());
	}
	
	{
		bool is_full = true;
		bool is_dag = false;
		PlusOneAndOverflow block_enc("main_reg", "overflow");
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "overflow",
			is_full, is_dag);
		fmt::print("{}\n", ret.to_string());
	}

	fmt::print("Test Block Encoding U-:\n");
	{
		bool is_full = false;
		bool is_dag = true;
		PlusOneAndOverflow block_enc("main_reg", "overflow");
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "overflow",
			is_full, is_dag);
		fmt::print("{}\n", ret.to_string());
	}

	{
		bool is_full = true;
		bool is_dag = true;
		PlusOneAndOverflow block_enc("main_reg", "overflow");
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "overflow",
			is_full, is_dag);
		fmt::print("{}\n", ret.to_string());
	}

}

int main(int argc, const char** argv)
{
	
	size_t qubit_num = 3;
	size_t sz = pow2(qubit_num);
	//size_t steps = 742;
	double step_rate = 0.01;
	//double kappa = 50;
	double p = 1.3;
	//double alpha = 5.78;
	//double beta = -2.89;
	double alpha = 2.0;
	double beta = 1.0;
	
	
	double fs = 0.7788;
	
	testBlockEncodingA_via_Tridiagonal(qubit_num, alpha, beta);
	testBlockEncodingUs_via_Tridiagonal(qubit_num, alpha, beta, fs);
	testBlockEncodingUs_via_Tridiagonal(qubit_num, alpha, -beta, fs);

	
}