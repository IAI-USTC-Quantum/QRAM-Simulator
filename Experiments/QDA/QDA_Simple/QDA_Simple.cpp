
#include "BlockEncoding/block_encoding_via_QRAM.h"
#include "BlockEncoding/make_qram.h"
#include "argparse.h"

using namespace qram_simulator;

struct QDATestArguments
{
	size_t steps;
	double eps;
	std::string filename;

	inline std::string to_string() const { return "Null"; }
};

inline QDATestArguments qda_argument_parse(int argc, const char** argv)
{
	QDATestArguments args;
	argparse::ArgumentParser parser("QDA_Test", "QDA Fidelity Test");

	parser.add_argument()
		.names({ "-e", "--eps" })
		.description("tolerable error")
		.required(false);

	parser.add_argument()
		.names({ "-s", "--steps" })
		.description("walk sequence length")
		.required(false);

	parser.add_argument()
		.name("--filename")
		.description("saved data name")
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

	if (parser.exists("eps"))
		args.eps = parser.get<double>("eps");
	else {
		args.eps = 1e-5;
	}

	if (parser.exists("steps"))
		args.steps = parser.get<size_t>("steps");
	else {
		args.steps = 2;
	}


	if (parser.exists("filename"))
		args.filename = parser.get<std::string>("filename");
	else {
		args.filename = "test.csv";
	}

	return args;
}


int main(int argc, const char** argv) {

	//QModule::set_dumpfile("QDAdump.txt");
	QDATestArguments args = qda_argument_parse(argc, argv);
	std::string savename = args.filename;
	fmt::print("{},{},{}", args.eps, args.steps, savename);

	return 0;
}