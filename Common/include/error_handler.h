#pragma once

#include "typedefs.h"

namespace qram_simulator {

	///* QRAM_Release = true will skip many tests, checks and logs.
	//   Use this if the performance is vital and all programs are 
	//   tested.
	//*/
	
#ifdef QRAM_Release
#define QRAM_NoReturn
#else
#define QRAM_NoReturn [[noreturn]]
#undef QRAM_NoReturn
#define QRAM_NoReturn 
#endif

	/* throw declarations */
	void check_nan(const complex_t& m);
	void check_nan(const double& m);
	QRAM_NoReturn void throw_not_implemented();
	QRAM_NoReturn void throw_not_implemented(const char* errinfo);
	QRAM_NoReturn void throw_not_implemented(const std::string& errinfo);
	QRAM_NoReturn void throw_not_implemented(std::string_view errinfo);
	QRAM_NoReturn void throw_bad_switch_case();
	QRAM_NoReturn void throw_bad_switch_case(const char* errinfo);
	QRAM_NoReturn void throw_bad_switch_case(const std::string& errinfo);
	QRAM_NoReturn void throw_bad_switch_case(std::string_view errinfo);
	QRAM_NoReturn void throw_invalid_input();
	QRAM_NoReturn void throw_invalid_input(const char* errinfo);
	QRAM_NoReturn void throw_invalid_input(const std::string& errinfo);
	QRAM_NoReturn void throw_invalid_input(std::string_view errinfo);
	QRAM_NoReturn void throw_bad_result();
	QRAM_NoReturn void throw_bad_result(const char* errinfo);
	QRAM_NoReturn void throw_bad_result(const std::string& errinfo);
	QRAM_NoReturn void throw_bad_result(std::string_view errinfo);
	QRAM_NoReturn void throw_general_runtime_error();
	QRAM_NoReturn void throw_general_runtime_error(const char* errinfo);
	QRAM_NoReturn void throw_general_runtime_error(const std::string& errinfo);
	QRAM_NoReturn void throw_general_runtime_error(std::string_view errinfo);

	class TestFailException : public std::runtime_error 
	{
	public:
		explicit TestFailException(const std::string& _Message) : std::runtime_error(_Message.c_str()) {}
		explicit TestFailException(const char* _Message) : std::runtime_error(_Message) {}
	};

	/* For testing purposes */
	QRAM_NoReturn void throw_test_fail();
	QRAM_NoReturn void throw_test_fail(const char* errinfo, int lineno, const char* filename, const char* funcname);
}

#define TEST(name) \
  fmt::print("===== Running test {} =====\n\n", #name);\
  name();\
  fmt::print("\n\n")

#define TEST_FAIL(errmsg) throw_test_fail((errmsg), __LINE__, __FILE__, __FUNCTION__)

