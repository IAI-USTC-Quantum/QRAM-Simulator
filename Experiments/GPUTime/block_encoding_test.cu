#include "error_handler.h"
#include "sparse_state_simulator.h"
#include "cuda/sparse_state_simulator.cuh"
#include "cuda_utils.cuh"
#include "BlockEncoding/block_encoding_tridiagonal.h"
#include "DiscreteAdiabatic/cuda/qda_fundamental.cuh"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"

using namespace qram_simulator;

template<typename Ty>
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

	// select the QRAMCircuit/CuQRAMCircuit based on the type of Ty
	using QRAMCircuit = std::conditional_t<std::is_same_v<Ty, SparseState>, qram_qutrit::QRAMCircuit, qram_qutrit::CuQRAMCircuit>;

	QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	size_t sz = pow2(nqubit);
	auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

	DenseVector<double> b_ = ones<double>(pow2(nqubit));
	std::vector<uint64_t> conv_b = scaleAndConvertVector(b_, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	fmt::print("A (Extraction of block encoding of A):\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, nqubit);

		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_via_QRAM block_enc(&qram_A, "main_reg", "anc_UA", data_size, rational_size);
		DenseMatrix<complex_t> ret = std::is_same_v<Ty, CuSparseState>
			? cu_extract_block_encoding<Block_Encoding_via_QRAM>(block_enc, "main_reg", "anc_UA", is_full, is_dag) 
			: extract_block_encoding<Block_Encoding_via_QRAM>(block_enc, "main_reg", "anc_UA", is_full, is_dag);

		/* Compare mat and ret */
		if (!mat.allclose(ret, 1.0 / pow2(exponent)))
		{
			fmt::print("Mat = {}\n", mat.to_string());
			fmt::print("Ret = {}\n", ret.to_string());
			TEST_FAIL("Matrices are not equal.");
		}
	}

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
		DenseMatrix<complex_t> ret = std::is_same_v<Ty, CuSparseState>
			? cu_extract_block_encoding_Hs(encHs, "main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", nqubit)
			: extract_block_encoding_Hs(encHs, "main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", nqubit);

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
	{
		profiler _("GPU mode");
		int trials = 10;
		random_engine::set_seed(12341234);
		for (size_t i = 0; i < trials; ++i)
		{
			int random_qubit = random_engine::randint(2, 5);
			double random_alpha = random_engine::uniform(0.001, 10);
			double random_beta = random_engine::uniform(-10, 10);
			double random_fs = random_engine::uniform01();
			int random_exponent = random_engine::randint(15, 20);
			testBlockEncodingA_Tridiagonal_by_QRAM<CuSparseState>(random_qubit, random_alpha, random_beta, random_fs, random_exponent);
		}
		testBlockEncodingA_Tridiagonal_by_QRAM<CuSparseState>(3, 2, 0, 0.5, 15);
		fmt::print("{}", profiler::get_all_profiles_v2());
		profiler::init_profiler();
	}
	{
		profiler _("CPU mode");
		int trials = 10;
		random_engine::set_seed(12341234);
		for (size_t i = 0; i < trials; ++i)
		{
			int random_qubit = random_engine::randint(2, 5);
			double random_alpha = random_engine::uniform(0.001, 10);
			double random_beta = random_engine::uniform(-10, 10);
			double random_fs = random_engine::uniform01();
			int random_exponent = random_engine::randint(15, 20);
			testBlockEncodingA_Tridiagonal_by_QRAM<SparseState>(random_qubit, random_alpha, random_beta, random_fs, random_exponent);
		}
		testBlockEncodingA_Tridiagonal_by_QRAM<SparseState>(3, 2, 0, 0.5, 15);
		fmt::print("{}", profiler::get_all_profiles_v2());
		profiler::init_profiler();
	}
}

template<typename Ty>
void testBlockEncoding_Tridiagonal(size_t nqubit, double alpha, double beta, double fs)
{
	fmt::print("Test with (qubit, alpha, beta, fs) = ({}, {}, {}, {}).\n",
		nqubit, alpha, beta, fs);

	using namespace block_encoding;
	using namespace block_encoding::block_encoding_tridiagonal;
	using namespace QDA;
	using namespace QDA::QDA_tridiagonal;

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
		DenseMatrix<complex_t> ret = std::is_same_v<Ty, CuSparseState>
			? cu_extract_block_encoding(block_enc, "main_reg", "anc_UA", is_full, is_dag)
			: extract_block_encoding(block_enc, "main_reg", "anc_UA", is_full, is_dag);


		/* Compare mat and ret */
		if (!mat.allclose(ret, 1e-10))
			TEST_FAIL("Matrices are not equal.");

		System::clear();
	}

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

		DenseMatrix<complex_t> ret = std::is_same_v<Ty, CuSparseState>
			? cu_extract_block_encoding_Hs(encHs, "main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", nqubit)
			: extract_block_encoding_Hs(encHs, "main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", nqubit);

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
	{
		profiler _("GPU mode");
		int trials = 10;
		for (size_t i = 0; i < trials; ++i)
		{
			int random_qubit = random_engine::randint(2, 5);
			double random_alpha = random_engine::uniform(0.001, 10);
			double random_beta = random_engine::uniform(-10, 10);
			double random_fs = random_engine::uniform01();
			testBlockEncoding_Tridiagonal<CuSparseState>(random_qubit, random_alpha, random_beta, random_fs);
		}
		fmt::print("{}", profiler::get_all_profiles_v2());
		profiler::init_profiler();
	}
	{
		profiler _("CPU mode");
		int trials = 10;
		for (size_t i = 0; i < trials; ++i)
		{
			int random_qubit = random_engine::randint(2, 5);
			double random_alpha = random_engine::uniform(0.001, 10);
			double random_beta = random_engine::uniform(-10, 10);
			double random_fs = random_engine::uniform01();
			testBlockEncoding_Tridiagonal<SparseState>(random_qubit, random_alpha, random_beta, random_fs);
		}
		fmt::print("{}", profiler::get_all_profiles_v2());
		profiler::init_profiler();
	}
}

int main()
{
	// parallel_time_test();
	testBlockEncoding_Tridiagonal();
	testBlockEncodingA_Tridiagonal_by_QRAM();
	return 0;
}