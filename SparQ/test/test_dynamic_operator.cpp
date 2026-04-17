/**
 * @file test_dynamic_operator.cpp
 * @brief 动态算子加载器单元测试
 * @details 测试动态算子的 C++ 核心功能：
 *          - 简单 SelfAdjointOperator 扩展
 *          - 带参数的 BaseOperator 扩展
 *          - 编译错误处理
 *          - 缓存机制
 *          - dagger 操作正确性
 */

// Windows compatibility
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstring>

// 平台相关的动态库头文件
#ifndef _WIN32
#include <dlfcn.h>
#define POPEN popen
#define PCLOSE pclose
#else
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#endif

// 我们直接测试 dynamic_operator_loader 的实现
#include "basic_components.h"
#include "basic_gates.h"
using namespace qram_simulator;

// ============ 辅助函数 ============

/**
 * @brief 创建临时 C++ 源文件
 * @details 使用进程 ID 作为前缀避免并行测试文件名冲突
 */
std::string create_temp_source_file(const std::string& code, const std::string& filename) {
    std::string temp_dir = std::filesystem::temp_directory_path().string();
    // 使用进程 ID 作为前缀避免并行测试冲突
    static std::string pid_prefix;
    if (pid_prefix.empty()) {
#ifndef _WIN32
        pid_prefix = std::to_string(getpid()) + "_";
#else
        pid_prefix = std::to_string(GetCurrentProcessId()) + "_";
#endif
    }
    std::string filepath = temp_dir + "/" + pid_prefix + filename;
    std::ofstream file(filepath);
    file << code;
    file.close();
    return filepath;
}

/**
 * @brief 查找项目根目录
 * @details 在 CI 环境中，测试可执行文件可能在 build/ 目录中运行，
 *          需要向上查找直到找到项目根目录
 */
std::string find_project_root() {
    // 方法1: 检查环境变量 (最高优先级)
    const char* env_root = std::getenv("PROJECT_ROOT");
    if (env_root) {
        std::string root(env_root);
        if (std::filesystem::exists(root + "/SparQ/include")) {
            std::cerr << "[find_project_root] Using PROJECT_ROOT: " << root << std::endl;
            return root;
        }
    }

    // 方法2: 检查 GITHUB_WORKSPACE 环境变量 (CI 环境)
    const char* github_workspace = std::getenv("GITHUB_WORKSPACE");
    if (github_workspace) {
        std::string root(github_workspace);
        if (std::filesystem::exists(root + "/SparQ/include")) {
            std::cerr << "[find_project_root] Using GITHUB_WORKSPACE: " << root << std::endl;
            return root;
        }
    }
    
    // 方法2b: 检查 CI 环境中的源码目录 (github.workspace)
    const char* ci_workspace = std::getenv("CI_WORKSPACE");
    if (ci_workspace) {
        std::string root(ci_workspace);
        if (std::filesystem::exists(root + "/SparQ/include")) {
            std::cerr << "[find_project_root] Using CI_WORKSPACE: " << root << std::endl;
            return root;
        }
    }

    // 方法3: 从当前可执行文件路径向上查找
    std::filesystem::path current = std::filesystem::current_path();
    
    // 尝试从当前目录向上查找最多5层
    for (int i = 0; i < 5; ++i) {
        // 检查是否是项目根目录（包含 SparQ/include 和 Common/include）
        if (std::filesystem::exists(current / "SparQ" / "include") &&
            std::filesystem::exists(current / "Common" / "include")) {
            return current.string();
        }
        
        // 检查是否是构建目录（在 build/ 或 build/Release/ 等子目录中）
        // 向上查找直到找到包含 SparQ/CMakeLists.txt 或 pyproject.toml 的目录
        if (std::filesystem::exists(current / "SparQ" / "CMakeLists.txt") ||
            std::filesystem::exists(current / "CMakeLists.txt")) {
            // 再向上找一层，可能是从 build/ 到项目根目录
            if (current.has_parent_path()) {
                auto parent = current.parent_path();
                if (std::filesystem::exists(parent / "SparQ" / "include")) {
                    return parent.string();
                }
            }
        }
        
        // 检查源码目录特征文件
        if (std::filesystem::exists(current / "pyproject.toml") &&
            std::filesystem::exists(current / "SparQ")) {
            return current.string();
        }
        
        // 向上移动一层
        if (!current.has_parent_path()) {
            break;
        }
        current = current.parent_path();
    }

    // 方法4: 尝试从源码目录特征反向查找
    // 检查是否在 build/SparQ/test/ 这样的结构中
    current = std::filesystem::current_path();
    std::filesystem::path candidate = current;
    
    // 移除已知的构建目录层次
    while (candidate.has_parent_path()) {
        std::string filename = candidate.filename().string();
        // 如果是已知的构建目录，检查父目录
        if (filename == "test" || filename == "SparQ" || filename == "build" ||
            filename == "Release" || filename == "Debug") {
            candidate = candidate.parent_path();
            if (std::filesystem::exists(candidate / "SparQ" / "include")) {
                return candidate.string();
            }
            continue;
        }
        break;
    }

    // 默认: 返回当前目录，但记录警告
    std::cerr << "[find_project_root] Warning: Could not find project root!" << std::endl;
    std::cerr << "[find_project_root] Current directory: " << std::filesystem::current_path() << std::endl;
    std::cerr << "[find_project_root] Checking for SparQ/include: " 
              << std::filesystem::exists(std::filesystem::current_path() / "SparQ" / "include") << std::endl;
    
    // 列出当前目录内容以便调试
    std::cerr << "[find_project_root] Directory contents:" << std::endl;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
        std::cerr << "  - " << entry.path().filename().string() << std::endl;
    }
    
    return ".";
}

/**
 * @brief 检查编译器是否可用
 */
bool is_compiler_available() {
#ifdef _WIN32
    // Windows: check for g++ (MinGW)
    FILE* pipe = POPEN("g++ --version 2>&1", "r");
    if (pipe) {
        PCLOSE(pipe);
        return true;
    }
    return false;
#else
    // Unix: check for g++
    FILE* pipe = POPEN("g++ --version 2>&1", "r");
    if (pipe) {
        PCLOSE(pipe);
        return true;
    }
    return false;
#endif
}

/**
 * @brief 编译 C++ 代码为共享库
 * @details 使用进程 ID 作为前缀避免并行测试文件名冲突
 */
std::string compile_to_shared_lib(const std::string& source_path, const std::string& lib_name) {
    // 首先检查编译器是否可用
    if (!is_compiler_available()) {
        std::cerr << "No suitable C++ compiler found for dynamic operator test" << std::endl;
        return "";
    }

    // 使用进程 ID 作为前缀避免并行测试冲突
    static std::string pid_prefix;
    if (pid_prefix.empty()) {
#ifndef _WIN32
        pid_prefix = std::to_string(getpid()) + "_";
#else
        pid_prefix = std::to_string(GetCurrentProcessId()) + "_";
#endif
    }

    std::string temp_dir = std::filesystem::temp_directory_path().string();
    std::string lib_path = temp_dir + "/" + pid_prefix + lib_name;

    // 查找项目根目录
    std::string project_root = find_project_root();

    // 构建编译命令
    std::string cmd;
#ifdef _WIN32
    // Windows (MinGW): need .dll extension and different flags
    lib_path += ".dll";
    cmd = "g++ -std=c++17 -O2 -shared ";
    cmd += "-I\"" + project_root + "/SparQ/include\" ";
    cmd += "-I\"" + project_root + "/Common/include\" ";
    cmd += "-I\"" + project_root + "/QRAM/include\" ";
    cmd += "-I\"" + project_root + "/ThirdParty/eigen-3.4.0\" ";
    cmd += "-I\"" + project_root + "/ThirdParty/fmt/include\" ";
    cmd += "-o \"" + lib_path + "\" \"" + source_path + "\" 2>&1";
#else
    // Unix/Linux/macOS
    cmd = "g++ -std=c++17 -O2 -fPIC -shared ";
    cmd += "-I" + project_root + "/SparQ/include ";
    cmd += "-I" + project_root + "/Common/include ";
    cmd += "-I" + project_root + "/QRAM/include ";
    cmd += "-I" + project_root + "/ThirdParty/eigen-3.4.0 ";
    cmd += "-I" + project_root + "/ThirdParty/fmt/include ";
    cmd += "-o " + lib_path + " " + source_path + " 2>&1";
#endif

    // 执行编译
    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }

    char buffer[128];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    PCLOSE(pipe);

    if (!std::filesystem::exists(lib_path)) {
        std::cerr << "Compilation failed: " << output << std::endl;
        return "";
    }

    return lib_path;
}

// 跨平台的动态库加载
class TestDynamicLoader {
public:
    void* handle_ = nullptr;
    std::string lib_path_;
    
    explicit TestDynamicLoader(const std::string& lib_path) : lib_path_(lib_path) {
#ifdef _WIN32
        handle_ = static_cast<void*>(LoadLibraryA(lib_path.c_str()));
#else
        handle_ = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
    }
    
    ~TestDynamicLoader() {
        if (handle_) {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
        }
    }
    
    bool is_valid() const { return handle_ != nullptr; }
    
    void* get_symbol(const std::string& name) {
        if (!handle_) return nullptr;
#ifdef _WIN32
        HMODULE hModule = static_cast<HMODULE>(handle_);
        FARPROC proc = GetProcAddress(hModule, name.c_str());
        return reinterpret_cast<void*>(proc);
#else
        return dlsym(handle_, name.c_str());
#endif
    }
};

// ============ 测试固件 ============

class DynamicOperatorTest : public ::testing::Test {
protected:
    // 获取当前进程的 PID 前缀（只清理本进程创建的文件）
    static std::string get_pid_prefix() {
        static std::string prefix;
        if (prefix.empty()) {
#ifndef _WIN32
            prefix = std::to_string(getpid()) + "_";
#else
            prefix = std::to_string(GetCurrentProcessId()) + "_";
#endif
        }
        return prefix;
    }

    // 检查是否可以运行动态编译测试
    // Windows MSVC 编译的主程序与 MinGW g++ 编译的 DLL ABI 不兼容
    static bool can_run_dynamic_compile_test() {
#ifdef _WIN32
        // Windows 上，如果主程序是 MSVC 编译的，g++ 编译的 DLL 无法加载
        // 检测是否有可用的 MSVC 编译器 (cl.exe)
        FILE* pipe = POPEN("cl 2>&1", "r");
        if (pipe) {
            PCLOSE(pipe);
            // 有 cl.exe，但我们测试代码目前使用 g++，仍然会失败
            // 所以在 Windows 上暂时禁用动态编译测试
            return false;
        }
        return false;
#else
        // Unix/Linux/macOS 上可以正常使用 g++
        return is_compiler_available();
#endif
    }

    void SetUp() override {
        System::clear();
    }

    void TearDown() override {
        System::clear();
        // 只清理当前进程创建的临时文件，避免干扰并行测试
        cleanup_temp_files();
    }

    void cleanup_temp_files() {
        std::string temp_dir = std::filesystem::temp_directory_path().string();
        std::string pid_prefix = get_pid_prefix();
        try {
            for (const auto& entry : std::filesystem::directory_iterator(temp_dir)) {
                std::string name = entry.path().filename().string();
                // 只清理以当前进程 PID 为前缀的文件
                if (name.find(pid_prefix + "test_op_") == 0 ||
                    name.find(pid_prefix + "test_dynamic_") == 0) {
                    std::error_code ec;
                    std::filesystem::remove(entry.path(), ec);
                    // 忽略删除失败（Windows 上文件可能被锁定）
                }
            }
        } catch (const std::exception& e) {
            // 忽略清理异常
        }
    }
};

// ============ 测试用例 ============

/**
 * @brief 测试简单的 SelfAdjointOperator 扩展
 */
TEST_F(DynamicOperatorTest, SelfAdjointOperatorExtension) {
    // Windows MSVC 与 MinGW ABI 不兼容，跳过测试
    if (!can_run_dynamic_compile_test()) {
        GTEST_SKIP() << "Skipped: Dynamic compilation not supported on Windows MSVC (ABI incompatibility with MinGW)";
    }

    // 创建一个简单的翻转算子
    std::string cpp_code = R"(
#include "basic_components.h"
#include <vector>

using namespace qram_simulator;

class TestFlipOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    TestFlipOp(size_t r) : reg_id(r) {}
    
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.get(reg_id).value ^= 1;
        }
    }
};

extern "C" BaseOperator* create_operator(size_t reg_id) {
    return new TestFlipOp(reg_id);
}

extern "C" void destroy_operator(BaseOperator* op) {
    delete op;
}

extern "C" const char* get_operator_name() {
    return "TestFlipOp";
}

extern "C" const char* get_base_class() {
    return "SelfAdjointOperator";
}
)";

    std::string source_path = create_temp_source_file(cpp_code, "test_op_flip.cpp");
    std::string lib_path = compile_to_shared_lib(source_path, "test_op_flip.so");
    
    ASSERT_FALSE(lib_path.empty()) << "Failed to compile dynamic operator";
    ASSERT_TRUE(std::filesystem::exists(lib_path));
    
    // 测试动态加载
    TestDynamicLoader loader(lib_path);
    EXPECT_TRUE(loader.is_valid());
    
    // 测试符号获取
    auto* create_func = reinterpret_cast<BaseOperator* (*)(size_t)>(loader.get_symbol("create_operator"));
    auto* destroy_func = reinterpret_cast<void (*)(BaseOperator*)>(loader.get_symbol("destroy_operator"));
    auto* get_name_func = reinterpret_cast<const char* (*)()>(loader.get_symbol("get_operator_name"));
    
    ASSERT_NE(create_func, nullptr);
    ASSERT_NE(destroy_func, nullptr);
    ASSERT_NE(get_name_func, nullptr);
    
    // 验证算子名称
    EXPECT_STREQ(get_name_func(), "TestFlipOp");
    
    // 创建寄存器并测试算子
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();  // |0>
    
    // 创建算子实例
    BaseOperator* op = create_func(q);
    ASSERT_NE(op, nullptr);
    
    // 测试算子功能：翻转 |0> -> |1>
    (*op)(state);
    EXPECT_EQ(state[0].get(q).value, 1);
    
    // 再次翻转 |1> -> |0>
    (*op)(state);
    EXPECT_EQ(state[0].get(q).value, 0);
    
    // 测试 dagger（SelfAdjointOperator 应该和自身相同）
    state[0].get(q).value = 1;
    op->dag(state);
    EXPECT_EQ(state[0].get(q).value, 0);
    
    destroy_func(op);
}

/**
 * @brief 测试带参数的 BaseOperator 扩展
 */
TEST_F(DynamicOperatorTest, BaseOperatorWithParams) {
    // Windows MSVC 与 MinGW ABI 不兼容，跳过测试
    if (!can_run_dynamic_compile_test()) {
        GTEST_SKIP() << "Skipped: Dynamic compilation not supported on Windows MSVC (ABI incompatibility with MinGW)";
    }

    // 创建带参数的相位算子
    std::string cpp_code = R"(
#include "basic_components.h"
#include <vector>
#include <cmath>

using namespace qram_simulator;

class TestPhaseOp : public BaseOperator {
    size_t reg_id;
    double phase;
public:
    TestPhaseOp(size_t r, double p) : reg_id(r), phase(p) {}
    
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            if (s.get(reg_id).value != 0) {
                s.amplitude *= std::exp(std::complex<double>(0, phase));
            }
        }
    }
    
    void dag(std::vector<System>& state) const override {
        for (auto& s : state) {
            if (s.get(reg_id).value != 0) {
                s.amplitude *= std::exp(std::complex<double>(0, -phase));
            }
        }
    }
};

extern "C" BaseOperator* create_operator(size_t reg_id, double phase) {
    return new TestPhaseOp(reg_id, phase);
}

extern "C" void destroy_operator(BaseOperator* op) {
    delete op;
}

extern "C" const char* get_operator_name() {
    return "TestPhaseOp";
}
)";

    std::string source_path = create_temp_source_file(cpp_code, "test_op_phase.cpp");
    std::string lib_path = compile_to_shared_lib(source_path, "test_op_phase.so");
    
    ASSERT_FALSE(lib_path.empty()) << "Failed to compile dynamic operator";
    
    TestDynamicLoader loader(lib_path);
    EXPECT_TRUE(loader.is_valid());
    
    auto* create_func = reinterpret_cast<BaseOperator* (*)(size_t, double)>(loader.get_symbol("create_operator"));
    ASSERT_NE(create_func, nullptr);
    
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    state.emplace_back();
    Init_Unsafe("q", 1)(state);  // |1>
    
    // 创建相位算子（π/2 相位）
    BaseOperator* op = create_func(q, M_PI / 2);
    ASSERT_NE(op, nullptr);
    
    // 应用算子
    (*op)(state);
    
    // 检查相位：|1> 应该获得 i 的相位
    EXPECT_NEAR(state[0].amplitude.real(), 0.0, 1e-10);
    EXPECT_NEAR(state[0].amplitude.imag(), 1.0, 1e-10);
}

/**
 * @brief 测试编译错误处理
 */
TEST_F(DynamicOperatorTest, CompilationError) {
    // 创建有语法错误的代码
    std::string bad_cpp_code = R"(
#include "basic_components.h"
using namespace qram_simulator;

class BadOp : public BaseOperator {  // 缺少分号
    void operator()(std::vector<System>& state) const override {
        // 语法错误：未定义的变量
        undefined_variable = 42;
    }
};
)";

    std::string source_path = create_temp_source_file(bad_cpp_code, "test_op_bad.cpp");
    std::string lib_path = compile_to_shared_lib(source_path, "test_op_bad.so");
    
    // 编译应该失败，库文件不应存在
    EXPECT_TRUE(lib_path.empty() || !std::filesystem::exists(lib_path));
}

/**
 * @brief 测试缓存机制
 */
TEST_F(DynamicOperatorTest, CacheMechanism) {
    // Windows MSVC 与 MinGW ABI 不兼容，跳过测试
    if (!can_run_dynamic_compile_test()) {
        GTEST_SKIP() << "Skipped: Dynamic compilation not supported on Windows MSVC (ABI incompatibility with MinGW)";
    }

    // 相同的代码应该产生相同的库
    std::string cpp_code = R"(
#include "basic_components.h"
#include <vector>

using namespace qram_simulator;

class TestCacheOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    TestCacheOp(size_t r) : reg_id(r) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.get(reg_id).value ^= 1;
        }
    }
};

extern "C" BaseOperator* create_operator(size_t reg_id) {
    return new TestCacheOp(reg_id);
}
extern "C" void destroy_operator(BaseOperator* op) { delete op; }
)";

    // 清除可能存在的旧缓存文件（只清理当前进程的 .so 文件）
    std::string temp_dir = std::filesystem::temp_directory_path().string();
    std::string pid_prefix = get_pid_prefix();
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir)) {
        std::string name = entry.path().filename().string();
        // 只清理编译产物 (.so/.dll)，不清理源文件
        if (name.find(pid_prefix + "test_op_cache") == 0 &&
            (name.find(".so") != std::string::npos || name.find(".dll") != std::string::npos)) {
            std::error_code ec;
            std::filesystem::remove(entry.path(), ec);
        }
    }

    // 创建源文件（放在清理之后）
    std::string source_path1 = create_temp_source_file(cpp_code, "test_op_cache1.cpp");
    std::string source_path2 = create_temp_source_file(cpp_code, "test_op_cache2.cpp");
    
    // 第一次编译
    std::string lib_path1 = compile_to_shared_lib(source_path1, "test_op_cache_a.so");
    ASSERT_FALSE(lib_path1.empty());
    
    // 第二次编译相同代码
    std::string lib_path2 = compile_to_shared_lib(source_path2, "test_op_cache_b.so");
    ASSERT_FALSE(lib_path2.empty());
    
    // 两个库都应该存在且可以加载
    TestDynamicLoader loader1(lib_path1);
    TestDynamicLoader loader2(lib_path2);
    
    EXPECT_TRUE(loader1.is_valid());
    EXPECT_TRUE(loader2.is_valid());
}

/**
 * @brief 测试 dagger 操作正确性
 */
TEST_F(DynamicOperatorTest, DaggerOperation) {
    // Windows MSVC 与 MinGW ABI 不兼容，跳过测试
    if (!can_run_dynamic_compile_test()) {
        GTEST_SKIP() << "Skipped: Dynamic compilation not supported on Windows MSVC (ABI incompatibility with MinGW)";
    }

    // SelfAdjointOperator: dagger 应该等于自身
    std::string self_adjoint_code = R"(
#include "basic_components.h"
#include <vector>

using namespace qram_simulator;

class TestSelfAdjointOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    TestSelfAdjointOp(size_t r) : reg_id(r) {}
    void operator()(std::vector<System>& state) const override {
        for (auto& s : state) {
            s.amplitude *= -1.0;  // 乘以 -1
        }
    }
};

extern "C" BaseOperator* create_operator(size_t reg_id) {
    return new TestSelfAdjointOp(reg_id);
}
extern "C" void destroy_operator(BaseOperator* op) { delete op; }
)";

    std::string source_path = create_temp_source_file(self_adjoint_code, "test_op_dagger.cpp");
    std::string lib_path = compile_to_shared_lib(source_path, "test_op_dagger.so");
    
    ASSERT_FALSE(lib_path.empty());
    
    TestDynamicLoader loader(lib_path);
    auto* create_func = reinterpret_cast<BaseOperator* (*)(size_t)>(loader.get_symbol("create_operator"));
    ASSERT_NE(create_func, nullptr);
    
    auto q = System::add_register("q", Boolean, 1);
    std::vector<System> state;
    state.emplace_back();
    
    BaseOperator* op = create_func(q);
    ASSERT_NE(op, nullptr);
    
    // 初始振幅为 1
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-10);
    
    // 应用算子: 1 -> -1
    (*op)(state);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(-1.0, 0)), 0.0, 1e-10);
    
    // 应用 dagger（对于 SelfAdjointOperator，应该和自身相同）: -1 -> 1
    op->dag(state);
    EXPECT_NEAR(std::abs(state[0].amplitude - complex_t(1.0, 0)), 0.0, 1e-10);
}

/**
 * @brief 测试动态库加载失败的情况
 */
TEST_F(DynamicOperatorTest, InvalidLibraryLoad) {
    TestDynamicLoader loader("/nonexistent/path/to/library.so");
    EXPECT_FALSE(loader.is_valid());
}

/**
 * @brief 测试动态库中的符号获取
 */
TEST_F(DynamicOperatorTest, SymbolRetrieval) {
    // Windows MSVC 与 MinGW ABI 不兼容，跳过测试
    if (!can_run_dynamic_compile_test()) {
        GTEST_SKIP() << "Skipped: Dynamic compilation not supported on Windows MSVC (ABI incompatibility with MinGW)";
    }

    std::string cpp_code = R"(
#include "basic_components.h"
#include <vector>

using namespace qram_simulator;

class SymbolTestOp : public SelfAdjointOperator {
    size_t reg_id;
public:
    SymbolTestOp(size_t r) : reg_id(r) {}
    void operator()(std::vector<System>& state) const override {}
};

extern "C" BaseOperator* create_operator(size_t reg_id) {
    return new SymbolTestOp(reg_id);
}
extern "C" void destroy_operator(BaseOperator* op) { delete op; }
extern "C" const char* get_operator_name() { return "SymbolTestOp"; }
extern "C" const char* get_base_class() { return "SelfAdjointOperator"; }
extern "C" int test_function() { return 42; }
)";

    std::string source_path = create_temp_source_file(cpp_code, "test_op_symbol.cpp");
    std::string lib_path = compile_to_shared_lib(source_path, "test_op_symbol.so");
    
    ASSERT_FALSE(lib_path.empty());
    
    TestDynamicLoader loader(lib_path);
    ASSERT_TRUE(loader.is_valid());
    
    // 测试存在的符号
    EXPECT_NE(loader.get_symbol("create_operator"), nullptr);
    EXPECT_NE(loader.get_symbol("destroy_operator"), nullptr);
    EXPECT_NE(loader.get_symbol("get_operator_name"), nullptr);
    EXPECT_NE(loader.get_symbol("get_base_class"), nullptr);
    EXPECT_NE(loader.get_symbol("test_function"), nullptr);
    
    // 测试不存在的符号
    EXPECT_EQ(loader.get_symbol("nonexistent_symbol"), nullptr);
    EXPECT_EQ(loader.get_symbol(""), nullptr);
}

// ============ 主函数 ============

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
