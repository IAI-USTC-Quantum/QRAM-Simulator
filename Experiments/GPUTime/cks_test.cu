#include "cuda/sparse_state_simulator.cuh"
#include "hamiltonian_simulation.h"

using namespace qram_simulator;
using namespace CKS;

template<typename Ty = SparseState>
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

	QuantumWalkNSteps<Ty> quantum_walk_obj(mat);
	if constexpr (std::is_same_v<Ty, CuSparseState>)
	{
		// uplift QRAMCircuit to CuQRAMCircuit
		qram_qutrit::CuQRAMCircuit *qram_circuit = new qram_qutrit::CuQRAMCircuit(*quantum_walk_obj.qram);
		delete quantum_walk_obj.qram;
		quantum_walk_obj.qram = qram_circuit;
	}

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
	int id = System::get(quantum_walk_obj.j);
	for (System& s : state)
	{
		m[s.GetAs(id, uint64_t)] = s.amplitude;
	}

	auto target_result = target_vec.to_vec<complex_t>();
	double fidelity = get_fidelity(m, target_result);

	if (quantum_walk_obj.qram->is_noise_free() && (fidelity < 0.98 || fidelity > 1.02))
	{
		fmt::print("Step = {}, Fidelity = {}\n"
			//"Get = {}\n Want = {}\n"
			, step
			, fidelity
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

template<typename Ty = SparseState>
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
		automatic_Chebyshev_test<Ty>(step, mat);
		fmt::print("  Max register count = {}\n", System::max_register_count);
		fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
		fmt::print("  Max state size  = {}\n", System::max_system_size);
		System::clear();
	}
}

auto automatic_Chebyshev_test_main()
{
	{
		profiler _("GPU Mode");
		size_t row = 1024;
		size_t strip_size = 16;
		size_t data_size = 32;
		std::optional<seed_t> seed = 1668590069;
		automatic_Chebyshev_test<CuSparseState>(row, strip_size, data_size, seed);
		profiler::print_profiler();
		profiler::init_profiler();
	}
	{
		profiler _("CPU Mode");
		size_t row = 1024;
		size_t strip_size = 16;
		size_t data_size = 32;
		std::optional<seed_t> seed = 1668590069;
		automatic_Chebyshev_test<SparseState>(row, strip_size, data_size, seed);
		profiler::print_profiler();
		profiler::init_profiler();
	}
}

int main()
{
	automatic_Chebyshev_test_main();
	return 0;
}