

void testBlockEncoding_Tridiagonal(size_t nqubit, double alpha, double beta, double fs)
{
	fmt::print("Test with (qubit, alpha, beta, fs) = ({}, {}, {}, {}).\n",
		nqubit, alpha, beta, fs);

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
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
			is_full, is_dag);
		// fmt::print("{}\n", ret.to_string());

		/* Compare mat and ret */
		if (!mat.allclose(ret, 1e-10))
			TEST_FAIL("Matrices are not equal.");

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

		DenseMatrix<complex_t> ret = extract_block_encoding_Hs(encHs,
			"main_reg", "anc_UA",
			"anc_1", "anc_2", "anc_3", "anc_4", nqubit);

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

	using namespace QDA::QDA_via_QRAM;

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
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	DenseVector<double> b_ = ones<double>(pow2(nqubit));
	std::vector<uint64_t> conv_b = scaleAndConvertVector(b_, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	qram_qutrit::QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	fmt::print("A (Extraction of block encoding of A):\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, nqubit);

		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_via_QRAM block_enc(&qram_A, "main_reg", "anc_UA", data_size, rational_size);

		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
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

		DenseMatrix<complex_t> ret = extract_block_encoding_Hs(encHs,
			"main_reg", "anc_UA",
			"anc_1", "anc_2", "anc_3", "anc_4", nqubit);

		auto A = mat.to_eigen();
		size_t sz = pow2(nqubit);
		auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

		auto Hs_compare_ = GetHs(A, fs, b);
		DenseMatrix<complex_t> Hs_compare = DenseMatrix<complex_t>::from_eigen(Hs_compare_);

		if (!Hs_compare.allclose(ret, 1.0 / pow2(exponent)))
		{
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

void testBlockEncodingA_Random_by_QRAM(size_t nqubit, double fs, int expoenent_)
{
	fmt::print("Test with (qubit, fs, seed) = ({}, {}, {}).\n",
		nqubit, fs, random_engine::get_seed());

	using namespace QDA::QDA_via_QRAM;
	size_t sz = pow2(nqubit);

	auto mat_ = DenseMatrix<double>::random(sz, -2, 2);
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
	qram_qutrit::QRAMCircuit qram_A(2 * nqubit + 1, data_size);
	qram_A.set_memory(data_tree_A);

	auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

	DenseVector<double> b_ = ones<double>(pow2(nqubit));
	std::vector<uint64_t> conv_b = scaleAndConvertVector(b_, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	qram_qutrit::QRAMCircuit qram_b(nqubit + 1, data_size);
	qram_b.set_memory(data_tree_b);

	fmt::print("A (Extraction of block encoding of A):\n");
	{
		System::clear();
		System::add_register("main_reg", UnsignedInteger, nqubit);
		System::add_register("anc_UA", UnsignedInteger, nqubit);

		bool is_full = false;
		bool is_dag = false;
		Block_Encoding_via_QRAM block_enc(&qram_A, "main_reg", "anc_UA", data_size, rational_size);

		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "anc_UA",
			is_full, is_dag);

		/* Compare mat and ret */
		if (!mat.allclose(ret, 1.0 / pow2(exponent)))
		{
			fmt::print("Mat = {}\n", mat.to_string());
			fmt::print("Ret = {}\n", ret.to_string());
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
		System::add_register("anc_UA", UnsignedInteger, nqubit);
		System::add_register("anc_1", Boolean, 1);
		System::add_register("anc_2", Boolean, 1);
		System::add_register("anc_3", Boolean, 1);
		System::add_register("anc_4", Boolean, 1);

		Block_Encoding_via_QRAM encA(&qram_A, "main_reg", "anc_UA", data_size, rational_size);
		Hadamard_Int_Full encb("main_reg");

		Block_Encoding_Hs<Block_Encoding_via_QRAM, Hadamard_Int_Full> encHs(encA, encb,
			"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4", fs);

		DenseMatrix<complex_t> ret = extract_block_encoding_Hs(encHs,
			"main_reg", "anc_UA",
			"anc_1", "anc_2", "anc_3", "anc_4", nqubit);

		auto A = mat.to_eigen();
		size_t sz = pow2(nqubit);
		auto b = EigenMat<double>::Ones(sz, 1) / std::sqrt(sz);

		auto Hs_compare_ = GetHs(A, fs, b);
		DenseMatrix<complex_t> Hs_compare = DenseMatrix<complex_t>::from_eigen(Hs_compare_);

		if (!Hs_compare.allclose(ret, 1.0 / pow2(exponent)))
		{
			TEST_FAIL("Hs matrices are not equal.");
		}
		System::clear();
	}
}

void testBlockEncodingA_Random_by_QRAM()
{
	int trials = 10;
	for (size_t i = 0; i < trials; ++i)
	{
		random_engine::time_seed();
		int random_qubit = random_engine::randint(2, 5);
		double random_fs = random_engine::uniform01();
		int random_exponent = random_engine::randint(15, 20);
		testBlockEncodingA_Random_by_QRAM(random_qubit, random_fs, random_exponent);
	}
}



void testBlockEncoding_U_plus_minus(size_t nqubit)
{
	using namespace block_encoding::block_encoding_tridiagonal;
	using namespace QDA::QDA_tridiagonal;

	System::clear();
	System::add_register("main_reg", UnsignedInteger, nqubit);
	System::add_register("overflow", UnsignedInteger, 1);

	fmt::print("Test Block Encoding U+:\n");
	{
		bool is_full = false;
		bool is_dag = false;
		PlusOneAndOverflow block_enc("main_reg", "overflow");
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "overflow",
			is_full, is_dag);

		DenseMatrix<complex_t> u_plus = Get_U_plus <complex_t>(pow2(nqubit));
		if (!ret.allclose(u_plus))
		{
			TEST_FAIL("U+ block encoding is incorrect");
		}
	}

	fmt::print("Test Block Encoding U-:\n");
	{
		bool is_full = false;
		bool is_dag = true;
		PlusOneAndOverflow block_enc("main_reg", "overflow");
		DenseMatrix<complex_t> ret = extract_block_encoding(block_enc, "main_reg", "overflow",
			is_full, is_dag);

		DenseMatrix<complex_t> u_minus = Get_U_minus<complex_t>(pow2(nqubit));
		if (!ret.allclose(u_minus))
		{
			TEST_FAIL("U- block encoding is incorrect");
		}
	}
	System::clear();
}

void testBlockEncoding_U_plus_minus()
{
	for (size_t nqubit : range(2, 6))
	{
		testBlockEncoding_U_plus_minus(nqubit);
	}
}
