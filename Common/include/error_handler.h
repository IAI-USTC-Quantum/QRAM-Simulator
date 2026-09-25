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
	/// Validate that a complex number is not NaN; throw if it is (no-op under QRAM_Release)
	void check_nan(const complex_t& m);
	/// Validate that a real number is not NaN; throw if it is (no-op under QRAM_Release)
	void check_nan(const double& m);
	/// Throw a "not implemented" exception
	QRAM_NoReturn void throw_not_implemented();
	/// @brief Throw a "not implemented" exception with a message.
	QRAM_NoReturn void throw_not_implemented(const char* errinfo);
	/// @copydoc throw_not_implemented(const char*)
	QRAM_NoReturn void throw_not_implemented(const std::string& errinfo);
	/// @copydoc throw_not_implemented(const char*)
	QRAM_NoReturn void throw_not_implemented(std::string_view errinfo);
	/// Throw an "invalid switch case" exception (fallback for missed enum dispatch)
	QRAM_NoReturn void throw_bad_switch_case();
	/// @brief Throw an "invalid switch case" exception with a message.
	QRAM_NoReturn void throw_bad_switch_case(const char* errinfo);
	/// @copydoc throw_bad_switch_case(const char*)
	QRAM_NoReturn void throw_bad_switch_case(const std::string& errinfo);
	/// @copydoc throw_bad_switch_case(const char*)
	QRAM_NoReturn void throw_bad_switch_case(std::string_view errinfo);
	/// Throw an "invalid input" exception
	QRAM_NoReturn void throw_invalid_input();
	/// @brief Throw an "invalid input" exception with a message.
	QRAM_NoReturn void throw_invalid_input(const char* errinfo);
	/// @copydoc throw_invalid_input(const char*)
	QRAM_NoReturn void throw_invalid_input(const std::string& errinfo);
	/// @copydoc throw_invalid_input(const char*)
	QRAM_NoReturn void throw_invalid_input(std::string_view errinfo);
	/// Throw a "bad result" exception (e.g. failed normalization checks)
	QRAM_NoReturn void throw_bad_result();
	/// @brief Throw a "bad result" exception with a message.
	QRAM_NoReturn void throw_bad_result(const char* errinfo);
	/// @copydoc throw_bad_result(const char*)
	QRAM_NoReturn void throw_bad_result(const std::string& errinfo);
	/// @copydoc throw_bad_result(const char*)
	QRAM_NoReturn void throw_bad_result(std::string_view errinfo);
	/// Throw a general runtime error
	QRAM_NoReturn void throw_general_runtime_error();
	/// @brief Throw a general runtime error with a message.
	QRAM_NoReturn void throw_general_runtime_error(const char* errinfo);
	/// @copydoc throw_general_runtime_error(const char*)
	QRAM_NoReturn void throw_general_runtime_error(const std::string& errinfo);
	/// @copydoc throw_general_runtime_error(const char*)
	QRAM_NoReturn void throw_general_runtime_error(std::string_view errinfo);

	/**
	 * @brief Failure exception of the in-house test framework.
	 *
	 * Used together with the TEST / TEST_FAIL macros: caught and reported
	 * in a try/catch inside main().
	 */
	class TestFailException : public std::runtime_error
	{
	public:
		explicit TestFailException(const std::string& _Message) : std::runtime_error(_Message.c_str()) {}
		explicit TestFailException(const char* _Message) : std::runtime_error(_Message) {}
	};

	/* For testing purposes */
	/// Throw a test-failure exception
	QRAM_NoReturn void throw_test_fail();
	/// @brief Throw a test-failure exception with location info (used internally by the TEST_FAIL macro).
	QRAM_NoReturn void throw_test_fail(const char* errinfo, int lineno, const char* filename, const char* funcname);
}

/// Run one test function: print the title -> invoke -> print a blank line
#define TEST(name) \
  fmt::print("===== Running test {} =====\n\n", #name);\
  name();\
  fmt::print("\n\n")

/// Report a test failure (with line number / file / function name)
#define TEST_FAIL(errmsg) throw_test_fail((errmsg), __LINE__, __FILE__, __FUNCTION__)
