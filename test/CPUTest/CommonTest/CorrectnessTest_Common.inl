
/* Test gram_schmidt_process */
void schmidt_test()
{
	double alpha = 1.0;
	double beta = 0.8;
	DenseMatrix<complex_t> mat(4);
	mat(0, 0) = alpha;
	mat(1, 0) = beta;
	mat(2, 0) = beta;
	mat(3, 0) = 0;
	mat(1, 1) = 1;
	mat(2, 2) = 1;
	mat(3, 3) = 1;

	// fmt::print("Original A = {}", mat.to_string());
	/* Original A =
	(1,0) (0,0) (0,0) (0,0)
	(0.8,0) (1,0) (0,0) (0,0)
	(0.8,0) (0,0) (1,0) (0,0)
	(0,0) (0,0) (0,0) (1,0)
	*/

	gram_schmidt_process(mat);

	// fmt::print("A = {}", mat.to_string());
	/* A =
	(0.662266178532522,0) (-0.413714401892061,0) (-0.624695047554424,0) (0,0)
	(0.529812942826018,0) (0.848114523878724,0) (-7.10889595793335e-17,0) (0,0)
	(0.529812942826018,0) (-0.330971521513649,0) (0.78086880944303,0) (0,0)
	(0,0) (0,0) (0,0) (1,0)
	*/

	DenseMatrix<complex_t> compare_A_1(4);
	compare_A_1(0, 0) = 0.662266178532522;
	compare_A_1(0, 1) = -0.413714401892061;
	compare_A_1(0, 2) = -0.624695047554424;
	compare_A_1(0, 3) = 0;
	compare_A_1(1, 0) = 0.529812942826018;
	compare_A_1(1, 1) = 0.848114523878724;
	compare_A_1(1, 2) = -7.10889595793335e-17;
	compare_A_1(1, 3) = 0;
	compare_A_1(2, 0) = 0.529812942826018;
	compare_A_1(2, 1) = -0.330971521513649;
	compare_A_1(2, 2) = 0.78086880944303;
	compare_A_1(2, 3) = 0;
	compare_A_1(3, 0) = 0;
	compare_A_1(3, 1) = 0;
	compare_A_1(3, 2) = 0;
	compare_A_1(3, 3) = 1;

	// fmt::print("Compare A = {}", compare_A_1.to_string());

	if (!mat.allclose(compare_A_1, 1e-7))
		TEST_FAIL("Schmidt test failed.");

	auto matdag = dagger(mat);

	// fmt::print("Adag = {}", matdag.to_string());
	/* Adag =
	(0.662266178532522,-0) (0.529812942826018,-0) (0.529812942826018,-0) (0,-0)
	(-0.413714401892061,-0) (0.848114523878724,-0) (-0.330971521513649,-0) (0,-0)
	(-0.624695047554424,-0) (-7.10889595793335e-17,-0) (0.78086880944303,-0) (0,-0)
	(0,-0) (0,-0) (0,-0) (1,-0)
	*/

	DenseMatrix<complex_t> compare_Adag_1(4);
	compare_Adag_1(0, 0) = 0.662266178532522;
	compare_Adag_1(0, 1) = 0.529812942826018;
	compare_Adag_1(0, 2) = 0.529812942826018;
	compare_Adag_1(0, 3) = 0;
	compare_Adag_1(1, 0) = -0.413714401892061;
	compare_Adag_1(1, 1) = 0.848114523878724;
	compare_Adag_1(1, 2) = -0.330971521513649;
	compare_Adag_1(1, 3) = 0;
	compare_Adag_1(2, 0) = -0.624695047554424;
	compare_Adag_1(2, 1) = -7.10889595793335e-17;
	compare_Adag_1(2, 2) = 0.78086880944303;
	compare_Adag_1(2, 3) = 0;
	compare_Adag_1(3, 0) = 0;
	compare_Adag_1(3, 1) = 0;
	compare_Adag_1(3, 2) = 0;
	compare_Adag_1(3, 3) = 1;

	// fmt::print("Compare Adag = {}", compare_Adag_1.to_string());

	if (!matdag.allclose(compare_Adag_1, 1e-7))
		TEST_FAIL("Schmidt test failed.");

	auto res = matdag * mat;

	// fmt::print("Expect Identity = {}", res.to_string());
	if (!res.is_identity(1e-7))
		TEST_FAIL("Schmidt test failed.");
}



auto complement_test()
{
	size_t digit = 3;
	int64_t m = pow2(digit - 1);
	// fmt::print("{} {}\n", -m, m);
	for (int64_t v = -m; v < m; ++v)
	{
		size_t data = make_complement(v, digit);
		int64_t comp = get_complement(data, digit);
		if (comp != v) {
			fmt::print("{} != {} (data = {})\n", v, comp, data);
			TEST_FAIL("Error: Complement test failed.");
		}
	}
	fmt::print("Test passed.\n");
}

auto integer_addition_complement_test()
{
	size_t digit = 3;
	int64_t m = pow2(digit - 1);
	for (int64_t v = -3 * m; v < 3 * m; ++v)
	{
		int64_t range = pow2(digit - 1);
		int64_t x = std::lldiv(v, range).rem;
		// int64_t x = v;
		size_t data = make_complement(x, digit);
		int64_t y = get_complement(data, digit);

		/* convert to reg1.size-signed integer again */
		// fmt::print("{}, {}, {}\n", v, x, y);

		if (x != y)
		{
			fmt::print("{} != {} (data = {})\n", x, y, data);
			TEST_FAIL("Error: Complement test failed.");
		}
	}
	fmt::print("Test passed.\n");
}

auto test_get_rational()
{
	for (size_t repeat = 0; repeat < 10; ++repeat) {
		for (size_t n = 3; n <= 32; ++n) {
			double f = 0.1231231;
			size_t data = get_rational(f, n);
			size_t dataieee = get_rational_IEEE754(f, n);
			double nf = data * 1.0 / pow2(n);
			double nfieee = data * 1.0 / pow2(n);

			double copyf = f;
			size_t* ptt = reinterpret_cast<size_t*>(&copyf);
			size_t t = *ptt;
			std::bitset<64> m(t);

			std::string fs;

			fs = m.to_string();
			// fmt::print("n = {}, f = {}, data = {}, {}, {}, {}, delta={}\n", n, fs, data, dataieee, nf, nfieee, std::abs(nf - f));

			if (std::abs(nf - f) > 1.0 / pow2(n))
				TEST_FAIL("Error in get_rational");
		}
	}
}

auto test_check_unique_sort()
{
	std::vector<int> va{ 1, 2, 3, 4, 5, 6, 7, 8 };
	std::vector<int> vb{ 1, 2, 3, 4, 5, 5, 6, 7 };
	std::vector<int> vc{ 1, 2, 3, 4, 3, 5 ,6 ,7 };

	std::array<int, 8> aa{ 1,2,3,4,5,6,7,8 };
	std::array<int, 8> ab{ 1,2,3,4,5,5,6,7 };
	std::array<int, 8> ac{ 1,2,3,4,3,5,6,7 };

	if (check_unique_sort(va) == false ||
		check_unique_sort(vb) == true ||
		check_unique_sort(vc) == true)
	{
		throw_general_runtime_error();
	}
	if (check_unique_sort(aa) == false ||
		check_unique_sort(ab) == true ||
		check_unique_sort(ac) == true)
	{
		throw_general_runtime_error();
	}

	if (check_unique_sort(va.begin(), va.end()) == false ||
		check_unique_sort(vb.begin(), vb.end()) == true ||
		check_unique_sort(vc.begin(), vc.end()) == true)
	{
		throw_general_runtime_error();
	}
}


auto random_matrix_test()
{
	for (size_t elem_sz = 3; elem_sz <= 35; elem_sz++)
	{
		auto mat = generate_band_mat_unsigned(8, 2);
		// fmt::print(mat.to_string());

		auto sparse_mat = dense2sparse_band_unsigned(mat, 2, elem_sz);
		// fmt::print("Element = {}\nSparsity = {}\n", sparse_mat.elements, sparse_mat.sparsity);
		auto densemat = sparse2dense(sparse_mat);

		densemat *= (1. / pow2(sparse_mat.data_size));

		// fmt::print(densemat.to_string());

		if (!mat.allclose(densemat, 1.0 / pow2(elem_sz)))
		{
			TEST_FAIL("Random matrix generation test failed.");
		}
	}
	fmt::print("Test passed.");
}


auto continuous_range_test()
{
	ContinuousRange cr;
	// fmt::print("{}\n", (std::string)cr);
	/* Expected output: (empty line) */
	if ((std::string)cr != "")
		TEST_FAIL("ContinuousRange test failed.");

	cr.merge(1, 7);
	// fmt::print("{}\n", (std::string)cr);
	/* Expected output: [1,7] */
	if ((std::string)cr != "[1,7]")
		TEST_FAIL("ContinuousRange test failed.");

	cr.merge(10, 15);
	// fmt::print("{}\n", (std::string)cr);
	/* Expected output: [1,7][10,15] */
	if ((std::string)cr != "[1,7][10,15]")
		TEST_FAIL("ContinuousRange test failed.");

	cr.merge(3, 6);
	// fmt::print("{}\n", (std::string)cr);
	/* Expected output: [1,7][10,15] */
	if ((std::string)cr != "[1,7][10,15]")
		TEST_FAIL("ContinuousRange test failed.");

	/* Expected:
	-10:false
	-9:false
	-8:false
	-7:false
	-6:false
	-5:false
	-4:false
	-3:false
	-2:false
	-1:false
	0:false
	1:true
	2:true
	3:true
	4:true
	5:true
	6:true
	7:true
	8:false
	9:false
	10:true
	11:true
	12:true
	13:true
	14:true
	15:true
	16:false
	17:false
	18:false
	19:false
	*/
	std::vector<bool> expected_results;
	expected_results.resize(20, false);
	expected_results[1] = true;
	expected_results[2] = true;
	expected_results[3] = true;
	expected_results[4] = true;
	expected_results[5] = true;
	expected_results[6] = true;
	expected_results[7] = true;
	expected_results[10] = true;
	expected_results[11] = true;
	expected_results[12] = true;
	expected_results[13] = true;
	expected_results[14] = true;
	expected_results[15] = true;

	for (int i = -10; i < 20; ++i)
	{
		// fmt::print("{}:{}\n", i, cr.accept(i));
		if (i <= 0 && cr.accept(i))
			TEST_FAIL("ContinuousRange test failed.");
		if (i > 0 && cr.accept(i) != expected_results[i])
			TEST_FAIL("ContinuousRange test failed.");
	}
	cr.merge(2, 12);
	// fmt::print("{}\n", (std::string)cr);
	/* Expected output: [1,15] */
	for (int i = -10; i < 20; ++i)
	{
		// fmt::print("{}:{}\n", i, cr.accept(i));
		if (i < 1 && cr.accept(i))
			TEST_FAIL("ContinuousRange test failed.");
		if (i >= 1 && i <= 15 && !cr.accept(i))
			TEST_FAIL("ContinuousRange test failed.");
		if (i > 15 && cr.accept(i))
			TEST_FAIL("ContinuousRange test failed.");
	}
	fmt::print("Test passed.");
}

auto test_time_step() {
	size_t n_trials = 1000;
	static std::uniform_real_distribution<double> ud(0, 1);
	double p = 1.0e-3;
	//double p = 1.0;
	double node_p = 2 * p - p * p;
	for (size_t n = 6; n < 14; ++n) {

		//size_t sum_count1 = 0;
		size_t sum_count2 = 0;

		for (size_t i = 0; i < n_trials; ++i) {

			size_t k = 1;
			TimeStep step(n, k);
			double sum = 0;
			size_t n_qubit = 2 * (pow2(n) - 1);
			//std::vector<int> bad1(pow2(n), 0);
			std::vector<int> bad2(pow2(n), 0);
			for (size_t i = 0; i < n_qubit; ++i)
			{
				if (ud(random_engine::get_engine()) < p) {
					//	{
					//		auto&& [l, r] = step.get_bad_range_qubit(i);
					//		//fmt::print("{},{},{}\n", i, l, r);
					//		std::fill(bad1.begin() + l, bad1.begin() + r + 1, 1);
					//	}
					{
						auto&& [l, r] = step.get_bad_range_qutrit(i);
						//fmt::print("{},{},{}\n", i, l, r);
						std::fill(bad2.begin() + l, bad2.begin() + r + 1, 1);
					}
				}
			}
			//auto ct1 = std::count(bad1.begin(), bad1.end(), 1);
			auto ct2 = std::count(bad2.begin(), bad2.end(), 1);
			//sum_count1 += ct1;
			sum_count2 += ct2;
		}

		double p_avg_test = sum_count2 * 1.0 / n_trials / pow2(n);
		double p_guess = 1 - std::pow(1 - node_p, n);

		fmt::print("n={}, avg(qutrit)={}, guess={}\n", n,
			//sum_count1 * 1.0 / n_trials / pow2(n),
			p_avg_test, p_guess
		);

		/* The two values should be close to each other. */
		if (std::abs(p_avg_test - p_guess) / p_guess > 0.1)
			TEST_FAIL("Time step test failed.");
	}
}
