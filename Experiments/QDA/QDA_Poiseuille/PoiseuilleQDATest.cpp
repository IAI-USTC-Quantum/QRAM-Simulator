#include "PoiseuilleTestUtils.h"



struct QDATestArguments
{
	double step_rate;
	double p;
	size_t nqubit;
	double alpha;
	double beta;
	bool use_qram;
};

inline QDATestArguments qda_argument_parse(int argc, const char** argv)
{
	QDATestArguments args;
	argparse::ArgumentParser parser("QDA_Test", "QDA Fidelity Test");

	parser.add_argument()
		.names({ "-s", "--steprate" })
		.description("walk sequence length")
		.required(false);

	parser.add_argument()
		.name("--p")
		.description("p")
		.required(false);

	parser.add_argument()
		.name("--nqubit")
		.description("num of qubit")
		.required(false);

	parser.add_argument()
		.name("--alpha")
		.description("alpha")
		.required(false);

	parser.add_argument()
		.name("--beta")
		.description("beta")
		.required(false);

	parser.add_argument()
		.name("--useqram")
		.description("use qram or not")
		.required(false);

	parser.enable_help();
	auto err = parser.parse(argc, argv);

	if (err) {
		std::cout << err << std::endl;
		std::exit(2);
		return args;
	}

	if (parser.exists("help")) {
		parser.print_help();
		// return args;
		exit(0);
	}

	if (parser.exists("steprate"))
	{
		args.step_rate = parser.get<double>("steprate");
	}
	else {
		args.step_rate = 0.01;
	}

	if (parser.exists("p"))
	{
		args.p = parser.get<double>("p");
	}
	else {
		args.p = 1.3;
	}

	if (parser.exists("nqubit"))
	{
		args.nqubit = parser.get<size_t>("nqubit");
	}
	else {
		args.nqubit = 3;
	}

	if (parser.exists("alpha"))
	{
		args.alpha = parser.get<double>("alpha");
	}
	else {
		args.alpha = 2.0;
	}

	if (parser.exists("beta"))
	{
		args.beta = parser.get<double>("beta");
	}
	else {
		args.beta = 1.0;
	}

	if (parser.exists("useqram"))
	{
		args.use_qram = parser.get<bool>("useqram");
	}
	else {
		args.use_qram = false;
	}

	return args;
}


int main(int argc, const char** argv)
{
	
	QDATestArguments args;

	if (argc == 1)
	{
		const char* test_argv[] = {
			".",
			"--steprate", "0.01",
			"--p", "1.3",
			"--nqubit", "3",
			"--alpha", "2.0",
			"--beta", "-1.0",
			"--useqram", "0"
		};
		argc = array_length<decltype(test_argv)>::value;
		argv = test_argv;
	}

	args = qda_argument_parse(argc, argv);

	if (args.use_qram) {
		PoiseuilleTest_via_QRAM_nonPD(args.nqubit, args.step_rate, args.p, args.alpha, args.beta);
	}
	else {
		PoiseuilleTest_via_Tridiagonal_nonPD(args.nqubit, args.step_rate, args.p, args.alpha, args.beta);
	}
} 