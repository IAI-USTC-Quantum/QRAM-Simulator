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
	/// 校验复数非 NaN，是则抛出异常（QRAM_Release 下为空操作）
	void check_nan(const complex_t& m);
	/// 校验实数非 NaN，是则抛出异常（QRAM_Release 下为空操作）
	void check_nan(const double& m);
	/// 抛出"未实现"异常
	QRAM_NoReturn void throw_not_implemented();
	/// @brief 抛出"未实现"异常并附带信息。
	QRAM_NoReturn void throw_not_implemented(const char* errinfo);
	/// @copydoc throw_not_implemented(const char*)
	QRAM_NoReturn void throw_not_implemented(const std::string& errinfo);
	/// @copydoc throw_not_implemented(const char*)
	QRAM_NoReturn void throw_not_implemented(std::string_view errinfo);
	/// 抛出"非法 switch 分支"异常（枚举分发遗漏时的兜底）
	QRAM_NoReturn void throw_bad_switch_case();
	/// @brief 抛出"非法 switch 分支"异常并附带信息。
	QRAM_NoReturn void throw_bad_switch_case(const char* errinfo);
	/// @copydoc throw_bad_switch_case(const char*)
	QRAM_NoReturn void throw_bad_switch_case(const std::string& errinfo);
	/// @copydoc throw_bad_switch_case(const char*)
	QRAM_NoReturn void throw_bad_switch_case(std::string_view errinfo);
	/// 抛出"无效输入"异常
	QRAM_NoReturn void throw_invalid_input();
	/// @brief 抛出"无效输入"异常并附带信息。
	QRAM_NoReturn void throw_invalid_input(const char* errinfo);
	/// @copydoc throw_invalid_input(const char*)
	QRAM_NoReturn void throw_invalid_input(const std::string& errinfo);
	/// @copydoc throw_invalid_input(const char*)
	QRAM_NoReturn void throw_invalid_input(std::string_view errinfo);
	/// 抛出"结果异常"异常（归一化校验失败等）
	QRAM_NoReturn void throw_bad_result();
	/// @brief 抛出"结果异常"异常并附带信息。
	QRAM_NoReturn void throw_bad_result(const char* errinfo);
	/// @copydoc throw_bad_result(const char*)
	QRAM_NoReturn void throw_bad_result(const std::string& errinfo);
	/// @copydoc throw_bad_result(const char*)
	QRAM_NoReturn void throw_bad_result(std::string_view errinfo);
	/// 抛出一般运行时异常
	QRAM_NoReturn void throw_general_runtime_error();
	/// @brief 抛出一般运行时异常并附带信息。
	QRAM_NoReturn void throw_general_runtime_error(const char* errinfo);
	/// @copydoc throw_general_runtime_error(const char*)
	QRAM_NoReturn void throw_general_runtime_error(const std::string& errinfo);
	/// @copydoc throw_general_runtime_error(const char*)
	QRAM_NoReturn void throw_general_runtime_error(std::string_view errinfo);

	/**
	 * @brief 自制测试框架的失败异常。
	 *
	 * 搭配 TEST / TEST_FAIL 宏使用：main() 中以 try/catch 捕获并报告。
	 */
	class TestFailException : public std::runtime_error
	{
	public:
		explicit TestFailException(const std::string& _Message) : std::runtime_error(_Message.c_str()) {}
		explicit TestFailException(const char* _Message) : std::runtime_error(_Message) {}
	};

	/* For testing purposes */
	/// 抛出测试失败异常
	QRAM_NoReturn void throw_test_fail();
	/// @brief 抛出测试失败异常并附带位置信息（TEST_FAIL 宏内部使用）。
	QRAM_NoReturn void throw_test_fail(const char* errinfo, int lineno, const char* filename, const char* funcname);
}

/// 运行一个测试函数：打印标题 → 调用 → 打印空行
#define TEST(name) \
  fmt::print("===== Running test {} =====\n\n", #name);\
  name();\
  fmt::print("\n\n")

/// 报告测试失败（附带行号 / 文件 / 函数名）
#define TEST_FAIL(errmsg) throw_test_fail((errmsg), __LINE__, __FILE__, __FUNCTION__)
