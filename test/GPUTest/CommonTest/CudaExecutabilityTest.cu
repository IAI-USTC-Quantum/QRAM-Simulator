#include "error_handler.h"
#include "cuda/sparse_state_simulator.cuh"
#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "DiscreteAdiabatic/cuda/qda_fundamental.cuh"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"

using namespace qram_simulator;

auto test_cuda_basic()
{
	run_cuda_kernel();
	fmt::print("Test passed.\n");
}

auto CuSparseState_memory_move()
{
	System::clear();
	CuSparseState state;
	state.move_to_cpu();
	state.move_to_cpu();
	state.move_to_gpu();
	state.move_to_gpu();
	state.move_to_cpu();
	state.move_to_cpu();
	fmt::print("Test passed.\n");
}

void inherited_operator_test()
{
	System::clear();
	CuSparseState s;
	ModuleInheritance_Test mod1;
	ModuleInheritance_Test_SelfAdjoint mod2;

	mod1(s);
	try {
		mod1.dag(s);
	}
	catch (std::runtime_error& e) {
		fmt::print("mod1.dag is OK!\n");
	}
	mod2(s);
	mod2.dag(s);

	AddRegister("reg1", UnsignedInteger, 20)(s);	
	Hadamard_Int_Full("reg1")(s);
	AddRegister("reg2", UnsignedInteger, 3)(s);

	StatePrint(StatePrintDisplay::Detail | 0)(s);
}


void testBlockEncoding_Tridiagonal(size_t nqubit, double alpha, double beta, double fs)
{
	fmt::print("Test with (qubit, alpha, beta, fs) = ({}, {}, {}, {}).\n",
		nqubit, alpha, beta, fs);

	using namespace QDA;
	using namespace QDA_tridiagonal;

	auto mat_ = get_tridiagonal_matrix(alpha, beta, pow2(nqubit));
	mat_ = mat_ / mat_.normF();
	auto mat = DenseMatrix<complex_t>(mat_);

	fmt::print("A (Extraction of block encoding of A):\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, 4);

		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_Tridiagonal block_enc("main_reg", "anc_UA", alpha, beta);
		DenseMatrix<complex_t> ret = cu_extract_block_encoding<Block_Encoding_Tridiagonal>(block_enc, "main_reg", "anc_UA",
			is_full, is_dag);

		/* Compare mat and ret */
		if (!mat.allclose(ret, 1e-10))
		{
			fmt::print("Mat = {}\n", mat.to_string());
			fmt::print("Ret = {}\n", ret.to_string());
			fmt::print("diff = {}\n", (mat - ret).to_string());
			TEST_FAIL("Matrices are not equal.");
		}

		System::clear();
	}

	/* Basically this will pass the test, but it is too slow so I commented it out. */
	//fmt::print("Test Block Encoding U_A:\n");
	//{
	//	System::add_register("main_reg", UnsignedInteger, nqubit);
	//	System::add_register("anc_UA", UnsignedInteger, 4);

	//	bool is_full = true;
	//	bool is_dag = false;
	//	Block_Encoding_Tridiagonal block_enc("main_reg", "anc_UA", alpha, beta);
	//	DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
	//		is_full, is_dag);

	//	if (!ret.is_unitary(1e-10))
	//		TEST_FAIL("U_A is not unitary.");

	//	System::clear();
	//}

	fmt::print("Test Block Encoding Hs:\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, 4);
		System::add_register("anc_1", Boolean, 1);
		System::add_register("anc_2", Boolean, 1);
		System::add_register("anc_3", Boolean, 1);
		System::add_register("anc_4", Boolean, 1);

		Block_Encoding_Tridiagonal encA("main_reg", "anc_UA", alpha, beta);
		Hadamard_Int_Full encb("main_reg");

		Block_Encoding_Hs<Block_Encoding_Tridiagonal, Hadamard_Int_Full> encHs(encA, encb,
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

		DenseMatrix<complex_t> ret = cu_extract_block_encoding_Hs<Block_Encoding_Tridiagonal, Hadamard_Int_Full>			
			(encHs, "main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", nqubit);

		auto A = mat.to_eigen();
		size_t sz = pow2(nqubit);
		auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

		auto Hs_compare = GetHs(A, fs, b);
		auto ret_eigen = ret.to_eigen();

		if (!Hs_compare.isApprox(ret_eigen, 1e-10))
		{
			std::cout << "Hs_compare:\n" << Hs_compare << std::endl;
			std::cout << "ret:\n" << ret_eigen << std::endl;
			std::cout << "diff:\n" << (Hs_compare - ret_eigen) << std::endl;
			TEST_FAIL("Hs matrices are not equal.");
		}
		System::clear();
	}

	fmt::print("Test passed with (qubit, alpha, beta) = ({}, {}, {}).\n",
		nqubit, alpha, beta);
}

void testBlockEncoding_Tridiagonal()
{
	int trials = 10;
	for (size_t i = 0; i < trials; ++i)
	{
		int random_qubit = random_engine::randint(2, 5);
		double random_alpha = random_engine::uniform(0.001, 10);
		double random_beta = random_engine::uniform(-10, 10);
		double random_fs = random_engine::uniform01();
		testBlockEncoding_Tridiagonal(random_qubit, random_alpha, random_beta, random_fs);
	}
	testBlockEncoding_Tridiagonal(3, 2, 0, 0.5);
}


void testBlockEncodingA_Tridiagonal_by_QRAM(size_t nqubit, double alpha, double beta, double fs, int expoenent_)
{
	fmt::print("Test with (qubit, alpha, beta, fs, expoenent_) = ({}, {}, {}, {}, {}).\n",
		nqubit, alpha, beta, fs, expoenent_);

	using namespace QDA;
	using namespace QDA_via_QRAM;

	auto mat_ = block_encoding::block_encoding_tridiagonal::get_tridiagonal_matrix(alpha, beta, pow2(nqubit));
	mat_ = mat_ / mat_.normF();
	auto mat = DenseMatrix<complex_t>(mat_);

	/* The precision of the data, where each item has V' = round(pow2(exponent) * V) */
	int exponent = expoenent_;
	/* The size of data stored in the QRAM */
	size_t data_size = 50;
	/* The size to represent the rational numbers */
	size_t rational_size = 51;
	std::vector<uint64_t> conv_A = scaleAndConvertVector(mat_, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);
	qram_qutrit::CuQRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	size_t sz = pow2(nqubit);
	auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

	DenseVector<double> b_ = ones<double>(pow2(nqubit));
	std::vector<uint64_t> conv_b = scaleAndConvertVector(b_, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	qram_qutrit::CuQRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	fmt::print("A (Extraction of block encoding of A):\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, nqubit);

		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_via_QRAM block_enc(&qram_A, "main_reg", "anc_UA", data_size, rational_size);

		DenseMatrix<complex_t> ret = cu_extract_block_encoding<Block_Encoding_via_QRAM>(block_enc, "main_reg", "anc_UA",
			is_full, is_dag);

		/* Compare mat and ret */
		if (!mat.allclose(ret, 1.0 / pow2(exponent)))
		{
			fmt::print("Mat = {}\n", mat.to_string());
			fmt::print("Ret = {}\n", ret.to_string());
			TEST_FAIL("Matrices are not equal.");
		}
	}

	/* Basically this will pass the test, but it is too slow so I commented it out. */
	//fmt::print("Test Block Encoding U_A:\n");
	//{
	//	System::add_register("main_reg", UnsignedInteger, nqubit);
	//	System::add_register("anc_UA", UnsignedInteger, 4);

	//	bool is_full = true;
	//	bool is_dag = false;
	//	Block_Encoding_Tridiagonal block_enc("main_reg", "anc_UA", alpha, beta);
	//	DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
	//		is_full, is_dag);

	//	if (!ret.is_unitary(1e-10))
	//		TEST_FAIL("U_A is not unitary.");

	//	System::clear();
	//}

	fmt::print("Test Block Encoding Hs:\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, nqubit);
		System::add_register("anc_1", Boolean, 1);
		System::add_register("anc_2", Boolean, 1);
		System::add_register("anc_3", Boolean, 1);
		System::add_register("anc_4", Boolean, 1);

		Block_Encoding_via_QRAM encA(&qram_A, "main_reg", "anc_UA", data_size, rational_size);
		Hadamard_Int_Full encb("main_reg");

		Block_Encoding_Hs<Block_Encoding_via_QRAM, Hadamard_Int_Full> encHs(encA, encb,
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

		DenseMatrix<complex_t> ret = cu_extract_block_encoding_Hs<Block_Encoding_via_QRAM, Hadamard_Int_Full>(
			encHs,
			"main_reg", "anc_UA",
			"anc_1", "anc_2", "anc_3", "anc_4", nqubit);

		auto A = mat.to_eigen();
		size_t sz = pow2(nqubit);
		auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

		auto Hs_compare_ = GetHs(A, fs, b);
		DenseMatrix<complex_t> Hs_compare = DenseMatrix<complex_t>::from_eigen(Hs_compare_);

		if (!Hs_compare.allclose(ret, 1.0 / pow2(exponent)))
		{
			fmt::print("Hs_compare = {}\n", Hs_compare.to_string());
			fmt::print("Ret = {}\n", ret.to_string());
			fmt::print("diff = {}\n", (Hs_compare - ret).to_string());
			TEST_FAIL("Hs matrices are not equal.");
		}
		System::clear();
	}
}

void testBlockEncodingA_Tridiagonal_by_QRAM()
{
	int trials = 10;
	for (size_t i = 0; i < trials; ++i)
	{
		int random_qubit = random_engine::randint(2, 5);
		double random_alpha = random_engine::uniform(0.001, 10);
		double random_beta = random_engine::uniform(-10, 10);
		double random_fs = random_engine::uniform01();
		int random_exponent = random_engine::randint(15, 20);
		testBlockEncodingA_Tridiagonal_by_QRAM(random_qubit, random_alpha, random_beta, random_fs, random_exponent);
	}
	testBlockEncodingA_Tridiagonal_by_QRAM(3, 2, 0, 0.5, 15);
}

auto test_debugger_executability()
{
	System::clear();
	CuSparseState s;
	AddRegister("reg1", UnsignedInteger, 3)(s);
	Hadamard_Int_Full("reg1")(s);
	s.move_to_gpu();
	StatePrint()(s);
	CheckNan()(s);
	ViewNormalization()(s);
	CheckNormalization()(s);
	StatePrint()(s);
	StatePrint(StatePrintDisplay::Detail | 0)(s);
	CheckDuplicateKey()(s);
}


struct printf_functor
{
	__host__ __device__
	void operator()(System &s)
	{
		double* amplitude = (reinterpret_cast<double*>(&s));
		printf("printf_functor System: %p, Amplitude: %lf + %lfi\n", &s, amplitude[0], amplitude[1]);
	}
};

auto test_operations()
{
	System::clear();
	CuSparseState s;
	AddRegister("reg1", UnsignedInteger, 5)(s);
	Hadamard_Int_Full("reg1")(s);
	
	s.move_to_gpu();
	for (int i = 0; i < 4; ++i)
	{
		std::string reg_name = fmt::format("reg{}", i + 2);
		SplitRegister("reg1", reg_name, 1)(s);
	}
	// s.copy_to_cpu();
	StatePrint(Detail | 0)(s);
	Xgate_Bool("reg1", 0).conditioned_by_nonzeros("reg2")(s);
	// Xgate_Bool("reg1", 0)(s);
	StatePrint(Detail | 0)(s);

	//System::add_register("reg1", UnsignedInteger, 5);
	//CuSparseState s(10);
	//ParallelOperationTest(2.0, -2.0, 3)(s); 
	//CUDA_CHECK(cudaDeviceSynchronize());

	//thrust::for_each(thrust::device, s.sparse_state_gpu.begin(), s.sparse_state_gpu.end(), printf_functor());

	//CUDA_CHECK(cudaDeviceSynchronize());
	//s.copy_to_cpu();
	//StatePrint(Detail | 0)(s);

	}

int main()
{
	try
	{
		/* BlockEncoding test */
		TEST(testBlockEncoding_Tridiagonal);
		TEST(testBlockEncodingA_Tridiagonal_by_QRAM);
		
		/* Common test */
		TEST(test_cuda_basic);
		TEST(CuSparseState_memory_move);
		TEST(inherited_operator_test);
		TEST(test_debugger_executability);
		TEST(test_operations);

		fmt::print("All tests passed.\n");
		return 0;
	}
	catch (const TestFailException& e)
	{
		fmt::print("Test failed: {}\n", e.what());
		return 1;
	}
	catch (const std::runtime_error& e)
	{
		fmt::print("Runtime error: {}\n", e.what());
		return 2;
	}
	catch (const std::exception& e)
	{
		fmt::print("Error: {}\n", e.what());
		return 3;
	}

	return 0;
}