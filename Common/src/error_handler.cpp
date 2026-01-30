#include "error_handler.h"
#include "fmt/format.h"

namespace qram_simulator {

    void check_nan(const complex_t& m)
    {
#ifndef QRAM_Unchecked
        if (std::isnan(m.real()) || std::isnan(m.imag()))
        {
            throw std::runtime_error("[Fatal] The number is Nan!");
        }
#endif
    }

    void check_nan(const double& m)
    {
#ifndef QRAM_Release
        if (std::isnan(m))
        {
            throw std::runtime_error("[Fatal] The number is Nan!");
        }
#endif
    }

    void throw_not_implemented()
    {
#ifndef QRAM_Release
        throw std::runtime_error("[Fatal] This module is not implemented yet.");
#endif
    }

    void throw_not_implemented(const char* errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_not_implemented(const std::string& errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_not_implemented(std::string_view errinfo)
    {
        return throw_not_implemented(errinfo.data());
    }

    void throw_bad_switch_case()
    {
#ifndef QRAM_Release
        throw std::runtime_error("[Fatal] Impossible switch branch.");
#endif
    }

    void throw_bad_switch_case(const char* errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_bad_switch_case(const std::string& errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_bad_switch_case(std::string_view errinfo)
    {
        return throw_bad_switch_case(errinfo.data());
    }

    void throw_invalid_input()
    {
#ifndef QRAM_Release
        throw std::invalid_argument("Invalid input.");
#endif
    }

    void throw_invalid_input(const char* errinfo)
    {
#ifndef QRAM_Release
        throw std::invalid_argument(errinfo);
#endif
    }

    void throw_invalid_input(const std::string& errinfo)
    {
#ifndef QRAM_Release
        throw std::invalid_argument(errinfo);
#endif
    }

    void throw_invalid_input(std::string_view errinfo)
    {
        return throw_invalid_input(errinfo.data());
    }

    void throw_bad_result()
    {
#ifndef QRAM_Release
        throw std::runtime_error("Bad result.");
#endif
    }

    void throw_bad_result(const char* errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_bad_result(const std::string& errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_bad_result(std::string_view errinfo)
    {
        return throw_bad_result(errinfo.data());
    }

    void throw_general_runtime_error()
    {
#ifndef QRAM_Release
        throw std::runtime_error("[Fatal] General runtime error. Please check the code.");
#endif
    }

    void throw_general_runtime_error(const char* errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_general_runtime_error(const std::string& errinfo)
    {
#ifndef QRAM_Release
        throw std::runtime_error(errinfo);
#endif
    }

    void throw_general_runtime_error(std::string_view errinfo)
    {
        return throw_general_runtime_error(errinfo.data());
    }

    /* For testing purposes */
    QRAM_NoReturn void throw_test_fail()
    {
#ifndef QRAM_Release
        throw TestFailException("[Test] Test failed.");
#endif
    }
    QRAM_NoReturn void throw_test_fail(const char* errinfo, int lineno, const char* filename, const char* funcname)
    {
#ifndef QRAM_Release
        throw TestFailException(
            fmt::format(
                "[Test] Test Failed in function {} (File: {} Line: {})\n"
                "Error info: {}", funcname, filename, lineno, errinfo
        ));
#endif
    }
}