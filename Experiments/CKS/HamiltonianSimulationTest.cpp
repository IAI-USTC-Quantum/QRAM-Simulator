#include "hamiltonian_simulation.h"
#include "matrix.h"
#include <optional>

using namespace qram_simulator;
using namespace CKS;
auto test()
{	
	auto &&mat_obj = generate_simplest_sparse_matrix_unsigned_0();
	std::vector<size_t> &&data = mat_obj.get_data();
	size_t addr_size = log2(data.size());
	size_t data_size = addr_size;
	size_t offset = mat_obj.get_sparsity_offset(); 
	size_t n_row = mat_obj.n_row;
	size_t nnz_col = mat_obj.nnz_col;

	qram_qutrit::QRAMCircuit qram(addr_size, data_size, std::move(data));

	System::add_register("data_offset", UnsignedInteger, addr_size);
	System::add_register("sparse_offset", UnsignedInteger, addr_size);
	System::add_register("row_id", UnsignedInteger, addr_size);
	System::add_register("reg_b1", Boolean, 1);
	System::add_register("col_id", UnsignedInteger, addr_size);
	// System::add_register("data", UnsignedInteger, data_size);
	System::add_register("rot_target", Boolean, 1);
	System::add_register("k_comp", UnsignedInteger, addr_size);

	std::vector<System> system_states;
	system_states.emplace_back();

	Init_Unsafe("sparse_offset", offset)(system_states);
	Hadamard_Int("row_id", 2)(system_states);
	ClearZero()(system_states);

	(MoveBackRegister("col_id"))(system_states);
	(MoveBackRegister("row_id"))(system_states);
	(MoveBackRegister("reg_b1"))(system_states);
	(MoveBackRegister("rot_target"))(system_states);
	SortUnconditional()(system_states);

	(StatePrint(StatePrintDisplay::Detail))(system_states);

	T(&qram, "data_offset", "sparse_offset", "row_id", "reg_b1", "col_id", 
		"rot_target", "k_comp", 
		nnz_col, data_size, &mat_obj)
		.dag(system_states);
	/*Swap("row_id", "col_id")(system_states);
	Swap("reg_b1", "rot_target")(system_states);
	T(&qram, "data_offset", "sparse_offset", "row_id", "reg_b1", "col_id",
		"rot_target", "search_result", nnz_col, data_size).dag(system_states);*/

	CheckNormalization()(system_states);

	// fmt::print("T.dag\n");

	/*T(&qram, "data_offset", "sparse_offset", "row_id", "reg_b1", "col_id", 
		"rot_target", "search_result", nnz_col, data_size).dag(system_states);*/
	//

	ClearZero()(system_states);

	// (RemoveRegister("search_result"))(system_states);

	(StatePrint(StatePrintDisplay::Detail))(system_states);
}

auto test3()
{
	auto&& mat_obj = generate_simplest_sparse_matrix_signed_0();
	std::vector<size_t>&& data = mat_obj.get_data();
	size_t addr_size = log2(data.size());
	size_t data_size = addr_size;
	size_t offset = mat_obj.get_sparsity_offset();
	size_t n_row = mat_obj.n_row;
	size_t nnz_col = mat_obj.nnz_col;

	qram_qutrit::QRAMCircuit qram_obj(addr_size, data_size, std::move(data));
	auto qram = std::addressof(qram_obj);

	/* Register Init */
	std::string data_offset = "data_offset";
	System::add_register(data_offset, UnsignedInteger, addr_size);
	std::string sparse_offset = "sparse_offset";
	System::add_register(sparse_offset, UnsignedInteger, addr_size);
	std::string j = "row_id";
	System::add_register(j, UnsignedInteger, addr_size);
	std::string b1 = "reg_b1";
	System::add_register(b1, Boolean, 1);
	std::string k = "col_id";
	System::add_register(k, UnsignedInteger, addr_size);
	std::string b2 = "reg_b2";
	System::add_register(b2, Boolean, 1);
	std::string j_comp = "k_comp";
	System::add_register(j_comp, UnsignedInteger, addr_size);
	std::string k_comp = "k_comp";	
	System::add_register(k_comp, UnsignedInteger, addr_size);

	/* Begin */
	std::vector<System> system_states;
	system_states.emplace_back();

	Init_Unsafe(sparse_offset, offset)(system_states);
	// Hadamard_Int(j, 2)(system_states);
	Xgate_Bool(j, 0)(system_states);
	Xgate_Bool(j, 1)(system_states);
	ClearZero()(system_states);

	size_t sz = System::size_of(j);

	T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp,
		nnz_col, sz, &mat_obj)
		(system_states);

	for (int i = 0; i < 99; ++i)
	{
		QuantumWalk(qram, j, b1, k, b2, j_comp, k_comp, data_offset, sparse_offset, mat_obj)
			(system_states);
	}
	T(qram, data_offset, sparse_offset, j, b1, k, b2, k_comp, 
		nnz_col, sz, &mat_obj)
		.dag(system_states);

	CheckNormalization()(system_states);
	
	(StatePrint(StatePrintDisplay::Detail))(system_states);

	PartialTraceSelect({
		{b1, 0},
		{k, 0},
		{b2, 0},
		{k_comp, 0},
		{sparse_offset, offset},
		{data_offset, 0}
	})(system_states);

	(StatePrint(StatePrintDisplay::Detail))(system_states);
}

auto test_chebyshev_polynomial_coef()
{
	size_t b = 50000;
	ChebyshevPolynomialCoefficient chebyshev_obj(b);
	for (size_t j = 0; j <= 64; ++j)
	{
		fmt::print("j={}, coef={}\n", j, 
			chebyshev_obj.coef(j) * (chebyshev_obj.sign(j) ? -1 : 1)
		);
	}
}

auto test_quantum_walk_inversion_obj()
{
	std::string data_offset = "data_offset";
	std::string sparse_offset = "sparse_offset";
	std::string j = "row_id";
	std::string b1 = "reg_b1";
	std::string k = "col_id";
	std::string b2 = "reg_b2";
	std::string k_comp = "k_comp";
	QuantumWalkNSteps quantum_walk_obj(generate_simplest_sparse_matrix_unsigned_2());
	quantum_walk_obj.InitEnvironment();

	for (size_t i = 0; i < 5; ++i)
	{
		fmt::print("\nStep = {}, Press Enter to Continue.", i);
		getchar();
		auto ret = quantum_walk_obj.MakeNStepState(i);
		// (StatePrint(StatePrintDisplay::Detail))(ret);

		double prob = PartialTraceSelect({
			   {quantum_walk_obj.b1, 0},
			   {quantum_walk_obj.k, 0},
			   {quantum_walk_obj.b2, 0},
			   {quantum_walk_obj.k_comp, 0},
			   {quantum_walk_obj.sparse_offset, quantum_walk_obj.offset},
			   {quantum_walk_obj.data_offset, 0}
			})(ret);

		fmt::print("PartialTrace with prob = {}\n", 1 / prob / prob);

		(StatePrint(StatePrintDisplay::Detail))(ret);
	}

}

auto test_chebyshev_polynomial_coef_2()
{
	size_t b = 1000;
	ChebyshevPolynomialCoefficient chebyshev_obj(b);
	
	double x = 0.8;

	size_t PolynomialSize = 1000;
	size_t j_order = 300;
	std::vector<double> Ta(PolynomialSize, 0);
	Ta[0] = 1;
	Ta[1] = x;

	for (size_t j = 2; j < PolynomialSize; ++j)
	{
		Ta[j] = 2 * x * Ta[j - 1] - Ta[j - 2];
	}
	
	double sum = 0;
	for (size_t j = 0; j <= j_order; j ++)
	{
		double coef = chebyshev_obj.coef(j);
		if (chebyshev_obj.sign(j)) 
			coef *= -1;

		sum += Ta[2 * j + 1] * coef;
	}
	sum *= 4;
	fmt::print("x = {}, {{approx:1/x}} = {} (check = {})\n",
		x, sum, sum * x);

}

auto test_LCU_Container()
{
	double kappa_est = 3.0;
	double eps = 0.01;
	LCU_Container obj(generate_simplest_sparse_matrix_unsigned_2(), kappa_est, eps);
	auto& state = obj.current_state;

	obj.quantum_walk_obj.qram->set_noise_models(
		{
			{OperationType::Damping, 1e-5},
			{OperationType::Depolarizing, 1e-5}
		}
	);

	obj.iterate();

	std::string data_offset = "data_offset";
	std::string sparse_offset = "sparse_offset";
	std::string j = "row_id";
	std::string b1 = "reg_b1";
	std::string k = "col_id";
	std::string b2 = "reg_b2";
	std::string k_comp = "k_comp";

	(StatePrint(StatePrintDisplay::Detail))(state);

	double success_rate = PartialTraceSelect({
		{b1, 0},
		{k, 0},
		{b2, 0},
		{k_comp, 0},
		{sparse_offset, obj.quantum_walk_obj.offset},
		{data_offset, 0}
	})(state);
	success_rate = 1.0 / success_rate / success_rate;

	std::vector<complex_t> target_result =
	{
		0.223606797749979,
		0.223606797749979,
		0.670820393249937,
		0.670820393249937,
	};

	std::vector<complex_t> m(state.size(), 0);
	auto id = System::get(obj.GetInputVecReg());
	for (auto& s : state)
	{
		m[s.GetAs(id,uint64_t)] = s.amplitude;
	}
	double fidelity = get_fidelity(m, target_result);
	fmt::print("p_succ = {:6f}  |  F = {:6f} \n", success_rate, fidelity);

	(StatePrint(StatePrintDisplay::Detail))(state);
}

auto LinearSolverNoiseFree()
{
	double kappa_est = 30.0;
	double eps = 0.01;
	LCU_Container_NoiseFree obj(generate_simplest_sparse_matrix_unsigned_2(),
		kappa_est, eps);

	/*obj.quantum_walk_obj.qram->set_noise_models(
		{
			{OperationType::Damping, 1e-5},
			{OperationType::Depolarizing, 1e-5}
		}
	);*/

	// size_t addr_size = obj.quantum_walk_obj.get_init_size();
	size_t addr_size = 2;
	obj.ExternalInput<Hadamard_Int>(addr_size);

	std::vector<complex_t> target_result =
	{
		0.223606797749979,
		0.223606797749979,
		0.670820393249937,
		0.670820393249937,
	};

	size_t j = 0;
	FILE* fp = fopen("Result.txt", "w+");
	fmt::print("{:^3s}\t{:^7s}\t{:^7s}\n", "j", "  p_succ  ", "    F    ");
	fmt::print("b  = {}\nj0 = {}", obj.b, obj.j0);
	getchar();
	while (obj.Step()) {
		// StatePrint()(obj.step_state);
		auto [state, success_rate] = obj.PartialTrace_Nondestructive();

		std::vector<complex_t> m(target_result.size(), 0);
		auto id = System::get(obj.GetInputVecReg());
		for (auto &s : state)
		{
			m[s.GetAs(id,uint64_t)] = s.amplitude;
		}
		double fidelity = get_fidelity(m, target_result);
		fmt::print("{:^3d}\t{:^7f}\t{:^7f}\n", j, success_rate, fidelity);
		fmt::print(fp, "{:^3d}\t{:^7f}\t{:^7f}\n", j, success_rate, fidelity);

		j++;
	}
	fclose(fp);
	double ret = obj.PartialTrace();
	fmt::print("Successful Rate = {}\n", ret);
	StatePrint()(obj.current_state);
}

auto LinearSolverNoiseFree2()
{
	double kappa_est = 30;
	double eps = 0.01;
	LCU_Container_NoiseFree obj(
		generate_simplest_sparse_matrix_unsigned_2(),
		kappa_est, eps);

	fmt::print("{:^3s}\t{:^7s}\t{:^7s}\n", "j", "  p_succ  ", "    F    ");
	fmt::print("b  = {}\nj0 = {}", obj.b, obj.j0);
	getchar();
	/*obj.quantum_walk_obj.qram->set_noise_models(
		{
			{OperationType::Damping, 1e-5},
			{OperationType::Depolarizing, 1e-5}
		}
	);*/
	// size_t addr_size = obj.get_addr_size();
	size_t addr_size = obj.quantum_walk_obj.get_init_size();
	// size_t addr_size = 2;
	obj.ExternalInput<Hadamard_Int>(addr_size);
	std::vector<complex_t> target_result =
	{
		0.114875937160123,
		0.229751874320245,
		0.536087706747239,
		-0.804131560120858,
	};

	size_t j = 0;
	fmt::print("{:^3s}\t{:^7s}\t{:^7s}\n", "j", "  p_succ  ", "    F    ");
	while (obj.Step()) {
		auto &&[state, success_rate] = obj.PartialTrace_Nondestructive();
		
		std::vector<complex_t> m(target_result.size(), 0);
		auto id = System::get(obj.GetInputVecReg());
		for (auto &s : state)
		{
			m[s.GetAs(id,uint64_t)] = s.amplitude;
		}
		double fidelity = get_fidelity(m, target_result);
		fmt::print("{:^3d}\t{:^7f}\t{:^7f}\n", j++, success_rate, fidelity);
		
	}
	double ret = obj.PartialTrace();
	fmt::print("Successful Rate = {}\n", ret);
	StatePrint()(obj.current_state);
}

auto my_linear_solver_reference_v2(const SparseMatrix& mat)
{
	auto densemat = sparse2dense<double>(mat) ;
	densemat *= (1.0 / mat.nnz_col);
	densemat *= (1.0 / pow2(mat.data_size));

	if (!densemat.is_symmetric())
	{
		fmt::print("Not symmetric!");
		throw_invalid_input();
	}

	auto vec = ones<double>(mat.n_row) / std::sqrt(mat.n_row);

	auto result = my_linear_solver(densemat, vec);

	auto normalized_result = result / result.norm2();

	return std::make_pair(normalized_result.to_vec<complex_t>(), result.norm2());

}

auto matrix_test()
{
	auto mat = generate_simplest_sparse_matrix_unsigned_0();
	auto densemat = sparse2dense<double>(mat);

	if (!densemat.is_symmetric())
	{
		fmt::print("Not symmetric!");
		throw_invalid_input();
	}

	auto vec = ones<double>(mat.n_row);

	auto result = my_linear_solver(densemat, vec);

	auto normalized_result = result / result.norm2();

	fmt::print("mat = \n{}\n", densemat.to_string());
	fmt::print("vec = \n{}\n", vec.to_string());
	fmt::print("res = \n{}\n", result.to_string());
	fmt::print("RES = \n{}\n", normalized_result.to_string());
	fmt::print("RES = {}\n", normalized_result.to_vec());
}

auto automatic_Chebyshev_test(size_t step, SparseMatrix mat)
{
	auto densemat = sparse2dense<double>(mat);
	if (!densemat.is_symmetric())
	{
		fmt::print("Not symmetric!");
		throw_invalid_input();
	}
	densemat *= (1.0 / mat.nnz_col);
	if (mat.positive_only)
		densemat *= (1.0 / (pow2(mat.data_size) - 1));
	else
		densemat *= (1.0 / (pow2(mat.data_size - 1) - 1));
	
	// fmt::print("Converted: \n{}\n", densemat.to_string());

	auto vec = ones<double>(mat.n_row);

	QuantumWalkNSteps quantum_walk_obj(mat);

	quantum_walk_obj.InitEnvironment();
	quantum_walk_obj.qram->set_noise_models(
		{
			// {OperationType::Depolarizing, 1e-7 }
		}
	);

	auto state = quantum_walk_obj.MakeNStepState(step);

	auto target_vec = chebyshev_n(step, densemat, vec);
	target_vec = target_vec / target_vec.norm2();

	PartialTraceSelect({
			   {quantum_walk_obj.b1, 0},
			   {quantum_walk_obj.k, 0},
			   {quantum_walk_obj.b2, 0},
			   {quantum_walk_obj.j_comp, 0},
			   {quantum_walk_obj.k_comp, 0},
			   {quantum_walk_obj.sparse_offset, quantum_walk_obj.offset},
			   {quantum_walk_obj.data_offset, 0}
		})(state);
	
	CheckNormalization()(state);

	PartialTraceSelectRange(
		quantum_walk_obj.j,
		{ 0, quantum_walk_obj.n_row - 1 })(state);

	CheckNormalization()(state);

	std::vector<complex_t> m(mat.n_row, 0);
	auto id = System::get(quantum_walk_obj.j);
	for (auto& s : state)
	{
		m[s.GetAs(id,uint64_t)] = s.amplitude;
	}
	
	auto target_result = target_vec.to_vec<complex_t>();
	double fidelity = get_fidelity(m, target_result);

	if (quantum_walk_obj.qram->is_noise_free()
		&& (fidelity < 0.98 || fidelity > 1.02))
	{
		fmt::print("Step = {}, Fidelity = {}\n"
			//"Get = {}\n Want = {}\n"
			,step
			,fidelity
			//,complex2str(m) 
			//,complex2str(target_result)
		);
		// (StatePrint(StatePrintDisplay::Detail))(state_copy);
		getchar();
		// throw_bad_result();
	}
	else
	{
		fmt::print("Step = {}, Fidelity = {}, test passed.\n", step, fidelity);
	}
}

auto automatic_Chebyshev_test(size_t row, size_t strip_size, size_t data_size, std::optional<seed_t> seed)
{
	if (!seed.has_value())
		random_engine::time_seed();
	else
		random_engine::get_instance().set_seed(seed.value());

	auto save_seed = random_engine::get_instance().get_seed();
	fmt::print("Seed={}\n", save_seed);
	SparseMatrix mat;
	auto densemat = generate_band_mat_unsigned(row, strip_size);
	mat = dense2sparse_band_unsigned(densemat, strip_size, data_size);
	fmt::print("nrow = {}\n", mat.n_row);
	fmt::print("nnz  = {}\n", mat.nnz_col);
	for (size_t step = 1; step <= 5; step++)
	{
		automatic_Chebyshev_test(step, mat);
		fmt::print("  Max register count = {}\n", System::max_register_count);
		fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
		fmt::print("  Max state size  = {}\n", System::max_system_size);
		System::clear();
	}
}

auto automatic_linear_solver_test(
	size_t n_row, 
	size_t l, 
	size_t precision, 
	double eps,
	std::optional<seed_t> seed,
	std::optional<size_t> j0_test)
{
	if (!seed.has_value())
		random_engine::time_seed();
	else
		random_engine::get_instance().set_seed(seed.value());
	
	auto save_seed = random_engine::get_instance().get_seed();

	std::string hash = get_random_hash_str();

	auto densemat = generate_band_mat_unsigned(n_row, l);
	fmt::print("Finished generation.\n");
	auto mat = dense2sparse_band_unsigned(densemat, l, precision);
	fmt::print("Finished conversion.\n");
	// fmt::print("Mat = {}\n", densemat.to_string());
	double kappa_est = 0;
	if (n_row <= 1024)
	{
		double min_eigval = get_min_eigval(densemat, l);
		fmt::print("Minimum eigen value = {}\n", min_eigval);
		kappa_est = std::ceil(1.0 / min_eigval);
	}
	else
	{
		fmt::print("row > 1024. kappa is set to 0.\n");
		kappa_est = 0;
	}

	LCU_Container_NoiseFree obj(mat, kappa_est, eps);
	if (j0_test.has_value())
	{
		obj.j0 = j0_test.value();
		hash += fmt::format("-(j0={})", obj.j0);
	}

	fmt::print(
		"row = {}\n"
		"nnz_col = {}\n"
		"precision_size = {}\n", n_row, mat.nnz_col, mat.data_size);
	fmt::print(
		"kappa = {}\n"
		"eps = {}\n"
		"b  = {}\n"
		"j0 = {}\n", kappa_est, eps, obj.b, obj.j0);

	fmt::print("Please check the metadata. Press enter to continue.");

	// getchar();
	std::string metafile = "Metadata-" + hash + ".txt";
	FILE* fp2 = fopen(metafile.c_str(), "w");
	fmt::print(fp2,
		"seed = {}\n"
		"row = {}\n"
		"nnz_col = {}\n"
		"precision_size = {}\n", save_seed, n_row, mat.nnz_col, mat.data_size);
	fmt::print(fp2,
		"kappa = {}\n"
		"eps = {}\n"
		"b  = {}\n"
		"j0 = {}\n", kappa_est, eps, obj.b, obj.j0);

	//fmt::print("Writing matrix to file...\n");
	//fmt::print(fp2,
	//	"Elem = {}\n"
	//	"Sparsity= {}\n", mat.elements, mat.sparsity);
	//fmt::print(fp2,
	//	"Densemat = {}\n",
	//	densemat.to_string());

	fmt::print("OK.\n");

	/*obj.quantum_walk_obj.qram->set_noise_models(
		{
			{OperationType::Damping, 1e-5},
			{OperationType::Depolarizing, 1e-5}
		}
	);*/
	size_t addr_size = obj.quantum_walk_obj.get_init_size();
	obj.ExternalInput<Hadamard_Int>(addr_size);
	std::vector<complex_t> target_result = my_linear_solver_reference(mat);

	size_t j = 0;
	std::string filename = "Result-" + hash + ".txt";
	FILE* fp = fopen(filename.c_str(), "w");
	fmt::print("{:^3s}\t{:^7s}\t{:^7s}\n", "j", "  p_succ  ", "    F    ");
	while (obj.Step()) {
		auto&& [state, success_rate] = obj.PartialTrace_Nondestructive();

		std::vector<complex_t> m(target_result.size(), 0);
		auto id = System::get(obj.GetInputVecReg());
		for (auto& s : state)
		{
			m[s.GetAs(id,uint64_t)] = s.amplitude;
		}
		double fidelity = get_fidelity(m, target_result);
		j++;
		fmt::print("{:^3d}\t{:^15.12f}\t{:^15.12f}\t{:^8d}\n", j, success_rate, fidelity, System::max_system_size);
		fmt::print(fp, "{:^3d}\t{:^15.12f}\t{:^15.12f}\t{:^8d}\n", j, success_rate, fidelity, System::max_system_size);
	}
	double ret = obj.PartialTrace();
	fmt::print("Successful Rate = {}\n", ret);
	fmt::print("{}\n", profiler::get_all_profiles_v2());
	fmt::print("Max qubit count = {}\n", System::max_qubit_count);
	fmt::print("Max register count = {}\n", System::max_register_count);
	fmt::print("Max system size = {}\n", System::max_system_size);
	double nbytes = 1.0 * 8 * (System::max_register_count + 2) * System::max_system_size;

	fmt::print("Estimate memory = {} Bytes ({} KB, {} MB, {} GB)\n",
		nbytes, nbytes / pow2(10), nbytes / pow2(20), nbytes / pow2(30));

	fmt::print(fp2, "Successful Rate = {}\n", ret);
	fmt::print(fp2, "{}\n", profiler::get_all_profiles_v2());
	fmt::print(fp2, "Max qubit count = {}\n", System::max_qubit_count);
	fmt::print(fp2, "Max system size = {}\n", System::max_system_size);
	fmt::print(fp2, "Max register count = {}\n", System::max_qubit_count);
	/*StatePrint()(obj.current_state);
	fmt::print("target = {}", complex2str(target_result));*/
	fclose(fp);
	fclose(fp2);
}

auto automatic_linear_solver_theory_compare_test(
	size_t n_row,
	size_t l,
	size_t precision,
	double eps,
	std::optional<seed_t> seed,
	std::optional<size_t> j0_test)
{
	if (!seed.has_value())
		random_engine::time_seed();
	else
		random_engine::get_instance().set_seed(seed.value());

	auto save_seed = random_engine::get_instance().get_seed();

	std::string hash = get_random_hash_str();

	auto densemat = generate_band_mat_unsigned(n_row, l);
	auto mat = dense2sparse_band_unsigned(densemat, l, precision);
	// fmt::print("Mat = {}\n", densemat.to_string());
	double min_eigval = get_min_eigval(densemat, l);
	fmt::print("Minimum eigen value = {}\n", min_eigval);
	double kappa_est = std::ceil(1.0 / min_eigval);
	LCU_Container_Theory obj(mat, kappa_est, eps);

	if (j0_test.has_value())
	{
		obj.j0 = j0_test.value();
		hash += fmt::format("-(j0={})", obj.j0);
	}

	fmt::print(
		"row = {}\n"
		"nnz_col = {}\n"
		"precision_size = {}\n", n_row, mat.nnz_col, mat.data_size);
	fmt::print(
		"kappa = {}\n"
		"eps = {}\n"
		"b  = {}\n"
		"j0 = {}\n", kappa_est, eps, obj.b, obj.j0);

	fmt::print("Please check the metadata. Press enter to continue.\n");
	fmt::print("<Notice> This is for Theoretical Validation!! \n");
	fmt::print("<Notice> No metadata will be written \n");

	std::vector<complex_t> target_result = my_linear_solver_reference(mat);

	size_t j = 0;
	std::string filename = "Result-" + hash + ".txt";
	FILE* fp = fopen(filename.c_str(), "w");
	fmt::print("{:^3s}\t{:^7s}\t{:^7s}\n", "j", "  p_succ  ", "    F    ");
	while (obj.Step()) {
		auto&& [m, success_rate] = obj.GetOutput();

		double fidelity = get_fidelity(m.data, target_result);
		j++;
		fmt::print("{:^3d}\t{:^15.12f}\t{:^15.12f}\n", j, success_rate, fidelity);
		fmt::print(fp, "{:^3d}\t{:^15.12f}\t{:^15.12f}\n", j, success_rate, fidelity);
	}
	auto&& [m, success_rate] = obj.GetOutput();
	fmt::print("Successful Rate = {}\n", success_rate);
	fmt::print("{}\n", profiler::get_all_profiles_v2());
	fmt::print("Max qubit count = {}\n", System::max_qubit_count);

	fmt::print("target = {}", complex2str(target_result));
	fclose(fp);
}


auto automatic_Chebyshev_test_main()
{
	size_t row = 1024;
	size_t strip_size = 16;
	size_t data_size = 32;
	std::optional<seed_t> seed = 1668590069;
	automatic_Chebyshev_test(row, strip_size, data_size, seed);
	fmt::print("{}\n", profiler::get_all_profiles_v2());
}

auto automatic_linear_solver_test_main()
{
	size_t row = 1024;
	size_t strip_size = 8;
	size_t data_size = 32;
	double eps = 1e-3;	
	std::optional<seed_t> seed = std::nullopt;

	automatic_linear_solver_test(row, strip_size, data_size, eps, seed, std::nullopt);
}

auto automatic_linear_solver_theory_compare_test_main()
{
	size_t row = 1024;
	size_t strip_size = 16;
	size_t data_size = 8;
	double eps = 1e-3;
	std::optional<seed_t> seed = 1668754436;

	automatic_linear_solver_theory_compare_test(row, strip_size, data_size, eps, seed, std::nullopt);
}

auto automatic_linear_solver_test_main_2()
{
	size_t row = 1024;
	size_t strip_size = 16;
	size_t data_size = 8;
	double eps = 1e-3;
	std::optional<seed_t> seed = 1668754436;
	if (!seed.has_value())
		random_engine::time_seed();
	else
		random_engine::get_instance().set_seed(seed.value());

	auto save_seed = random_engine::get_instance().get_seed();

	std::string hash = get_random_hash_str();
	std::string metafile = "Metadata-" + hash + ".txt";
	auto densemat = generate_band_mat_unsigned(row, strip_size);
	auto mat = dense2sparse_band_unsigned(densemat, strip_size, data_size);
	auto &&[target_result, normcoef] = my_linear_solver_reference_v2(mat);

	ChebyshevPolynomialCoefficient chebyshev_obj(400742767);
	size_t j0 = 106122;
	double a = 0;
	for (size_t i = 0; i < j0; ++i)
	{
		a += chebyshev_obj.coef(i);
	}

	fmt::print(
		"norm2 = {}\n"
		"a = {}\n"
		"expect_prob = {}\n",
		normcoef,
		a,
		(normcoef * normcoef / a / a)
	);
}

auto automatic_linear_solver_test_main_capped_j0()
{
	size_t row = 1024;
	size_t strip_size = 4;
	size_t data_size = 32;
	size_t test_j0 = 10;
	double eps = 1e-3;
	std::optional<seed_t> seed = std::nullopt;

	automatic_linear_solver_test(row, strip_size, data_size, eps, seed, test_j0);
}

int main()
{
	omp_set_num_threads(8);
	automatic_Chebyshev_test_main();
	//automatic_linear_solver_test_main();
	// automatic_linear_solver_test_main_capped_j0();
	// automatic_linear_solver_theory_compare_test_main();
	getchar();
	return 0;
}