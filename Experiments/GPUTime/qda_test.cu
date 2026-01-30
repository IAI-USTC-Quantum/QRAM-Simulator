
#include "DiscreteAdiabatic/qda_fundamental.h"
#include "DiscreteAdiabatic/qda_tridiagonal.h"
#include "DiscreteAdiabatic/qda_via_QRAM.h"
#include "DiscreteAdiabatic/cuda/qda_fundamental.cuh"

using namespace qram_simulator;

void QDA_Poiseuille_via_QRAM_test(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	using namespace QDA;
	using namespace QDA_via_QRAM;

	int exponent = 20;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	double kappa = get_kappa_general(mat);
	fmt::print("kappa = {}\n", kappa);

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);

	DenseVector<double> b = DenseVector<double>::ones(pow2(nqubit));
	b = b / b.norm2();

	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);
	
	{
		using QRAMCircuit = qram_qutrit::CuQRAMCircuit;
		QRAMCircuit qram_A(2 * nqubit + 1, data_size, data_tree_A);
		QRAMCircuit qram_b(nqubit + 1, data_size, data_tree_b);

		System::clear();
		{
			profiler _("GPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			CuSparseState state;

			// Walk_s_via_QRAM::Encb enc_b(&qram_b, "main_reg", data_size, rational_size);
			Hadamard_Int_Full enc_b("main_reg");
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0)
				{
					fmt::print("step {}/{}\n", n, steps);
					fmt::print("  Max register count = {}\n", System::max_register_count);
					fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
					fmt::print("  Max state size  = {}\n", System::max_system_size);
				}
				double s = double(n) / steps;
				auto walk = Walk_s_via_QRAM_A<Hadamard_Int_Full>(&qram_A, enc_b,
					"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, data_size, rational_size);
				walk(state);
				ClearZero()(state);
				CheckNormalization_Renormalize()(state);
				
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();
		
		System::clear();
	}
	{
		using QRAMCircuit = qram_qutrit::QRAMCircuit;
		QRAMCircuit qram_A(2 * nqubit + 1, data_size, data_tree_A);
		QRAMCircuit qram_b(nqubit + 1, data_size, data_tree_b);

		System::clear();
		{
			profiler _("CPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			SparseState state;

			Walk_s_via_QRAM::Encb enc_b(&qram_b, "main_reg", data_size, rational_size);
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0) fmt::print("step {}/{}\n", n, steps);
				double s = double(n) / steps;
				auto walk = Walk_s_via_QRAM(&qram_A, &qram_b,
					"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, data_size, rational_size);
				walk(state);
				ClearZero()(state);
				CheckNormalization_Renormalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
}


void QDA_Poiseuille_via_QRAM_test(size_t nqubit, double step_rate, double p, double alpha, double beta, bool GPU)
{
	using namespace QDA;
	using namespace QDA_via_QRAM;

	int exponent = 20;
	size_t data_size = 50;
	size_t rational_size = 51;

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	double kappa = get_kappa_Tridiagonal(alpha, beta, pow2(nqubit));
	fmt::print("kappa = {}\n", kappa);

	std::vector<size_t> conv_A = scaleAndConvertVector(mat, exponent, data_size);
	memory_t data_tree_A = make_vector_tree(conv_A, data_size);

	DenseVector<double> b = DenseVector<double>::ones(pow2(nqubit));
	b = b / b.norm2();

	std::vector<size_t> conv_b = scaleAndConvertVector(b, exponent, data_size);
	memory_t data_tree_b = make_vector_tree(conv_b, data_size);

	if (GPU)
	{
		using QRAMCircuit = qram_qutrit::CuQRAMCircuit;
		QRAMCircuit qram_A(2 * nqubit + 1, data_size, data_tree_A);
		QRAMCircuit qram_b(nqubit + 1, data_size, data_tree_b);

		System::clear();
		{
			profiler _("GPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			CuSparseState state;

			//Walk_s_via_QRAM::Encb enc_b(&qram_b, "main_reg", data_size, rational_size);
			//enc_b(state);
			Hadamard_Int_Full enc_b("main_reg");
			enc_b(state);
			
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 10)
				{
					fmt::print("step {}/{}\n", n, steps);
					fmt::print("  Max register count = {}\n", System::max_register_count);
					fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
					fmt::print("  Max state size  = {}\n", System::max_system_size);
				}
				double s = double(n) / steps;
				auto walk = Walk_s_via_QRAM_A<Hadamard_Int_Full>(&qram_A, enc_b,
					"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, data_size, rational_size);
				walk(state);
				ClearZero()(state);
				//fmt::print("  Max register count = {}\n", System::max_register_count);
				//fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
				//fmt::print("  Max state size  = {}\n", System::max_system_size);
				//CheckNormalization_Renormalize()(state);
				Normalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
	else
	{
		using QRAMCircuit = qram_qutrit::QRAMCircuit;
		QRAMCircuit qram_A(2 * nqubit + 1, data_size, data_tree_A);
		QRAMCircuit qram_b(nqubit + 1, data_size, data_tree_b);

		System::clear();
		{
			profiler _("CPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, nqubit);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			SparseState state;

			Walk_s_via_QRAM::Encb enc_b(&qram_b, "main_reg", data_size, rational_size);
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0) fmt::print("step {}/{}\n", n, steps);
				double s = double(n) / steps;
				auto walk = Walk_s_via_QRAM(&qram_A, &qram_b,
					"main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, data_size, rational_size);
				walk(state);
				ClearZero()(state);
				CheckNormalization_Renormalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
}


void QDA_Poiseuille_Tridiagonal_test(size_t nqubit, double step_rate, double p, double alpha, double beta)
{
	using namespace QDA;
	using namespace QDA_tridiagonal;
	System::clear();

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	double kappa = get_kappa_general(mat);
	fmt::print("kappa = {}\n", kappa);

	DenseVector<double> b = DenseVector<double>::ones(pow2(nqubit));
	b = b / b.norm2();

	//fmt::print("mat_A:\n{}\n", mat.to_string());
	//fmt::print("Vec_b:\n{}\n", b.to_string());
	//auto x = my_linear_solver(mat, b);
	//fmt::print("Vec_x:\n{}\n", x.to_string());
	{
		System::clear();
		{
			profiler _("GPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, 4);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			CuSparseState state;

			Walk_s_Tridiagonal::Encb enc_b("main_reg");
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0)
				{
					fmt::print("step {}/{}\n", n, steps);
					fmt::print("  Max register count = {}\n", System::max_register_count);
					fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
					fmt::print("  Max state size  = {}\n", System::max_system_size);
				}
				double s = double(n) / steps;
				auto walk = Walk_s_Tridiagonal("main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, alpha, beta);
				walk(state);
				ClearZero()(state);
				//CheckNormalization_Renormalize()(state);
				Normalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
	{
		System::clear();
		{
			profiler _("CPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, 4);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			SparseState state;

			Walk_s_Tridiagonal::Encb enc_b("main_reg");
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0) fmt::print("step {}/{}\n", n, steps);
				double s = double(n) / steps;
				auto walk = Walk_s_Tridiagonal("main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, alpha, beta);
				walk(state);
				ClearZero()(state);
				CheckNormalization_Renormalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
}

void QDA_Poiseuille_Tridiagonal_test(size_t nqubit, double step_rate, double p, double alpha, double beta, bool GPU)
{
	using namespace QDA;
	using namespace QDA_tridiagonal;
	System::clear();

	DenseMatrix mat = generate_Poiseuille_mat<double>(pow2(nqubit), alpha, beta);
	mat = mat / mat.normF();

	double kappa = get_kappa_general(mat);
	fmt::print("kappa = {}\n", kappa);

	DenseVector<double> b = DenseVector<double>::ones(pow2(nqubit));
	b = b / b.norm2();

	//fmt::print("mat_A:\n{}\n", mat.to_string());
	//fmt::print("Vec_b:\n{}\n", b.to_string());
	//auto x = my_linear_solver(mat, b);
	//fmt::print("Vec_x:\n{}\n", x.to_string());
	if (GPU)
	{
		System::clear();
		{
			profiler _("GPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, 4);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			CuSparseState state;

			Walk_s_Tridiagonal::Encb enc_b("main_reg");
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0)
				{
					fmt::print("step {}/{}\n", n, steps);
					fmt::print("  Max register count = {}\n", System::max_register_count);
					fmt::print("  Max qubit count = {}\n", System::max_qubit_count);
					fmt::print("  Max state size  = {}\n", System::max_system_size);
				}
				double s = double(n) / steps;
				auto walk = Walk_s_Tridiagonal("main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, alpha, beta);
				walk(state);
				ClearZero()(state);
				//CheckNormalization_Renormalize()(state);
				Normalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
	else
	{
		System::clear();
		{
			profiler _("CPU Mode");
			auto main_reg = System::add_register("main_reg", UnsignedInteger, nqubit);
			auto anc_UA = System::add_register("anc_UA", UnsignedInteger, 4);
			auto anc_1 = System::add_register("anc_1", Boolean, 1);
			auto anc_2 = System::add_register("anc_2", Boolean, 1);
			auto anc_3 = System::add_register("anc_3", Boolean, 1);
			auto anc_4 = System::add_register("anc_4", Boolean, 1);
			// do walk sequence or directly prepare the eigenstate of H_1.
			// To set initial state as |1>_{anc_1}|b>
			SparseState state;

			Walk_s_Tridiagonal::Encb enc_b("main_reg");
			enc_b(state);
			//Xgate_Bool(anc_1, 0)(state);

			constexpr int StepConstant = 2305;
			size_t steps = size_t(step_rate * StepConstant * kappa);
			if (steps % 2 != 0) {
				steps += 1;
			}

			for (size_t n : range(steps))
			{
				if (n % 100 == 0) fmt::print("step {}/{}\n", n, steps);
				double s = double(n) / steps;
				auto walk = Walk_s_Tridiagonal("main_reg", "anc_UA", "anc_1", "anc_2", "anc_3", "anc_4",
					s, kappa, p, alpha, beta);
				walk(state);
				ClearZero()(state);
				CheckNormalization_Renormalize()(state);
			}
		}
		fmt::print("{}\n", profiler::get_all_profiles_v2());
		profiler::init_profiler();

		System::clear();
	}
}

int main(int argc, const char* argv[])
{
	//size_t nqubit = 3;
	//double step_rate = 0.01;
	//double p = 1.3;
	//double alpha = 1;
	//double beta = 0.5;
	////QDA_Poiseuille_Tridiagonal_test(nqubit, step_rate, p, alpha, beta);
	//QDA_Poiseuille_via_QRAM_test(3, step_rate, p, alpha, beta);
	//QDA_Poiseuille_via_QRAM_test(4, step_rate, p, alpha, beta);
	//QDA_Poiseuille_via_QRAM_test(5, step_rate, p, alpha, beta);

	// use argc and argv to get input parameters
	if (argc != 3)
	{
		size_t nqubit = 10;
		double step_rate = 0.01;
		double p = 1.3;
		double alpha = 1;
		double beta = 0.5;
		QDA_Poiseuille_via_QRAM_test(nqubit, step_rate, p, alpha, beta, true);
		// fmt::print("Usage: ./Experiment_GPUTime_qda_test {nqubit}\n");
		// return 1;
		return 0;
	}
	if (argc == 3)
	{
		size_t nqubit = std::stoul(argv[1]);
		bool GPU;
		if (std::string(argv[2]) == "GPU")
		{
			GPU = true;
		}
		else if (std::string(argv[2]) == "CPU")
		{
			GPU = false;
		}
		else if (std::string(argv[2]) == "Both")
		{
			double step_rate = 0.01;
			double p = 1.3;
			double alpha = 1;
			double beta = 0.5;
			QDA_Poiseuille_via_QRAM_test(nqubit, step_rate, p, alpha, beta);
			return 0;
		}
		else
		{
			fmt::print("Usage: ./Experiment_GPUTime_qda_test {nqubit} {GPU/CPU/Both}\n");
			return 1;
		}
		double step_rate = 0.01;
		double p = 1.3;
		double alpha = 1;
		double beta = 0.5;
		QDA_Poiseuille_via_QRAM_test(nqubit, step_rate, p, alpha, beta, GPU);
		return 0;
	}

}
