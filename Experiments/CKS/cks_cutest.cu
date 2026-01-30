#include "cuda/sparse_state_simulator.cuh"
#include "hamiltonian_simulation.h"

using namespace qram_simulator;
using namespace CKS;

template<typename Ty>
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

	constexpr size_t SKIP_COMPUTE_SIZE = 2048;
	if (n_row <= SKIP_COMPUTE_SIZE)
	{
		kappa_est = mat.get_kappa();
	}
	else
	{
		kappa_est = -1;
	}

	//Eigen::JacobiSVD<Eigen::MatrixXd> svd(densemat.to_eigen());
	//Eigen::VectorXd singular_values = svd.singularValues();
	//double cond_svd = singular_values(0) / singular_values(singular_values.size() - 1);
	//if (kappa_est < 0)
	//{
	//	kappa_est = cond_svd;
	//	fmt::print("row > 1024. kappa is estimated from SVD. kappa = {}.\n", kappa_est);
	//}
	//else
	//{
	//	fmt::print("kappa(min_eigval) = {} (used), kappa(svd) = {} (reference)\n", kappa_est, cond_svd);
	//}

	LCU_Container_NoiseFree<Ty> obj(mat, kappa_est, eps);
	if constexpr (std::is_same_v<Ty, CuSparseState>)
	{
		// uplift QRAMCircuit to CuQRAMCircuit
		qram_qutrit::CuQRAMCircuit* qram_circuit = new qram_qutrit::CuQRAMCircuit(*obj.quantum_walk_obj.qram);
		delete obj.quantum_walk_obj.qram;
		obj.quantum_walk_obj.qram = qram_circuit;
	}

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

	//getchar();
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
	// obj.ExternalInput<Hadamard_Int>(addr_size);
	obj.ExternalInput_V2(Hadamard_Int(obj.GetInputVecReg(), addr_size));
	std::vector<complex_t> target_result = n_row <= SKIP_COMPUTE_SIZE
		? my_linear_solver_reference(mat)
		: std::vector<complex_t>(n_row);

	size_t j = 0;
	std::string filename = "Result-" + hash + ".txt";
	FILE* fp = fopen(filename.c_str(), "w");
	fmt::print("{:^3s}\t{:^7s}\t{:^7s}\n", "j", "  p_succ  ", "    F    ");
	while (obj.Step()) {
		auto&& [state, success_rate] = obj.PartialTrace_Nondestructive();

		std::vector<complex_t> m(target_result.size(), 0);
		int id = System::get(obj.GetInputVecReg());
		for (System& s : state)
		{
			m[s.GetAs(id,uint64_t)] = s.amplitude;
		}
		double fidelity = get_fidelity(m, target_result);
		j++;
		fmt::print("{:^3d}/{:^3d}\t{:^15.12f}\t{:^15.12f}\t{:^8d}\n", j, obj.j0, success_rate, fidelity, System::max_system_size);
		fmt::print(fp, "{:^3d}\t{:^15.12f}\t{:^15.12f}\t{:^8d}\n", j, success_rate, fidelity, System::max_system_size);
	}
	double ret = obj.PartialTrace();
	fmt::print("Successful Rate = {}\n", ret);
	fmt::print("Max qubit count = {}\n", System::max_qubit_count);
	fmt::print("Max register count = {}\n", System::max_register_count);
	fmt::print("Max system size = {}\n", System::max_system_size);

	profiler::print_profiler();
	double nbytes = 8 * (System::max_register_count + 2) * System::max_system_size;
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

auto automatic_linear_solver_test_main()
{
	/* Config A */
	size_t row = 1024;
	size_t band_size = 16;
	size_t data_size = 8;
	double eps = 1e-3;	
	std::optional<seed_t> seed = 1668754436;
	// (CachedRegisterSize = 37)

	/* Config B */
	//size_t row = 2048;
	//size_t band_size = 32;
	//size_t data_size = 8;
	//double eps = 1e-3;
	//std::optional<seed_t> seed = std::nullopt;

	automatic_linear_solver_test<SparseState>(row, band_size, data_size, eps, seed, std::nullopt);
}

auto automatic_linear_solver_test_main_capped_j0()
{
	size_t row = 16384;
	size_t band_size = 16;
	size_t data_size = 8;
	double eps = 1e-3;	
	std::optional<seed_t> seed = 1668754436;
	//std::optional<seed_t> seed = std::nullopt;
	size_t j0_test = 10;

	automatic_linear_solver_test<CuSparseState>(row, band_size, data_size, eps, seed, j0_test);
}

int main(int argc, const char* argv[])
{
	if (argc == 1)
	{
		automatic_linear_solver_test_main();
		//automatic_linear_solver_test_main_capped_j0();
	}

	const char* mode = argv[1];
	if (std::string(mode) == "step")
	{
		if (argc != 8) {
			fmt::print("Usage: {} step row band_size data_size eps seed j0_test\n", argv[0]);
			return 1;
		}
		size_t row = std::stoll(argv[2]);
		size_t band_size = std::stoll(argv[3]);
		size_t data_size = std::stoll(argv[4]);
		double eps = std::stod(argv[5]);
		seed_t seed = std::stod(argv[6]);
		size_t j0_test = std::stoll(argv[7]);

		automatic_linear_solver_test<CuSparseState>(row, band_size, data_size, eps, seed, j0_test);
	}
	else if (std::string(mode) == "full")
	{
		if (argc != 7) {
			fmt::print("Usage: {} full row band_size data_size eps seed\n", argv[0]);
			return 1;
		}
		size_t row = std::stoll(argv[2]);
		size_t band_size = std::stoll(argv[3]);
		size_t data_size = std::stoll(argv[4]);
		double eps = std::stod(argv[5]);
		seed_t seed = std::stod(argv[6]);

		automatic_linear_solver_test<CuSparseState>(row, band_size, data_size, eps, seed, std::nullopt);
	}

	return 0;
}