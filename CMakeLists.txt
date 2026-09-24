cmake_minimum_required(VERSION 3.18)

project(SparQ)

include_directories(${PROJECT_SOURCE_DIR}/Common/include
                    ${PROJECT_SOURCE_DIR}/QRAM/include
                    ${PROJECT_SOURCE_DIR}/SparQ_Algorithm/include
                    ${PROJECT_SOURCE_DIR}/SparQ/include
                    ${PROJECT_SOURCE_DIR}/ThirdParty
                    ${PROJECT_SOURCE_DIR}/ThirdParty/argparse
                    ${PROJECT_SOURCE_DIR}/ThirdParty/fmt/include
                    ${PROJECT_SOURCE_DIR}/ThirdParty/eigen-3.4.0
                    )

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Enable testing (only when the test targets are built; consumers embedding
# this project via add_subdirectory, e.g. SparQSim, turn QRAM_BUILD_TESTS OFF)
option(QRAM_BUILD_TESTS "Build C++ test targets (SparQ/test, test/)" ON)
option(QRAM_BUILD_EXPERIMENTS "Build experiment programs (Experiments/)" ON)
option(QRAM_BUILD_PYTHON_BINDINGS "Build the thin qram_simulator pybind11 bindings" OFF)
if(QRAM_BUILD_TESTS)
    enable_testing()
endif()

option(QRAM_ENABLE_CUDA "Enable CUDA GPU support" OFF)

if(CACHED_REGISTER_SIZE)
    # CPU: initial std::vector reservation, not a hard register limit.
    # CUDA: fixed device layout and therefore still a hard limit (<= 64).
    add_compile_definitions(CACHED_REGISTER_SIZE=${CACHED_REGISTER_SIZE})
endif()

add_compile_definitions(FMT_UNICODE=0)

if(QRAM_ENABLE_CUDA)
    message(STATUS "CUDA support is temporarily disabled for the CondRot primitive refactor")
endif()
set(CUDA_FOUND FALSE)
message(STATUS "CUDA support disabled - building CPU-only")

option(CMAKE_BUILD_TYPE "Release")

message(STATUS "PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# TBB is optional - used for parallel algorithms if available
find_package(TBB QUIET)
if(TBB_FOUND)
    message(STATUS "Found TBB: ${TBB_VERSION}")
else()
    message(STATUS "TBB not found - using OpenMP for parallelization")
endif()

# Google Test - vendored under ThirdParty/googletest (v1.14.0)
# 不用 FetchContent 从 github.com 拉取:Gitea runner 无法访问 github.com
# (克隆失败会导致 configure/pip install 挂掉);vendored 源码树自包含,
# PyPI sdist 用户侧构建同样受益。
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if(QRAM_BUILD_TESTS)
    add_subdirectory(ThirdParty/googletest EXCLUDE_FROM_ALL)
    message(STATUS "GTest configured from ThirdParty (vendored v1.14.0)")
endif()

if (UNIX)
	add_compile_options(-fPIC)
endif(UNIX)

# MSVC: force UTF-8 source interpretation. Headers carry UTF-8 Chinese
# comments; on legacy code pages (e.g. CP936) MSVC mis-decodes them, which
# can merge an #ifdef line into a preceding comment and break the build.
if (MSVC)
	add_compile_options(/utf-8)
endif()

# if (MSVC)
#   add_compile_options(/bigobj)
#   add_compile_options(/FS)
# endif()


# pybind11 - find it early so ThirdParty/CMakeLists.txt can use the result
# When building wheels via scikit-build-core, pybind11 is installed via pyproject.toml
find_package(pybind11 CONFIG QUIET)
if(pybind11_FOUND)
    message(STATUS "Found pybind11: ${pybind11_VERSION}")
else()
    message(STATUS "pybind11 not found - Python bindings will not be built")
endif()

add_subdirectory(Common)
add_subdirectory(QRAM)
add_subdirectory(SparQ)
add_subdirectory(SparQ_Algorithm)
add_subdirectory(ThirdParty EXCLUDE_FROM_ALL)

if(QRAM_BUILD_TESTS)
    add_subdirectory(SparQ/test)
    add_subdirectory(test)
endif()

if(QRAM_BUILD_EXPERIMENTS)
    add_subdirectory(Experiments)
endif()

# Thin Python bindings for the qram_simulator PyPI package.
# The full pysparq bindings live in the SparQSim repository, which consumes
# this project as a git submodule and keeps QRAM_BUILD_PYTHON_BINDINGS=OFF
# to avoid building a second binding module.
if(QRAM_BUILD_PYTHON_BINDINGS)
    if(NOT pybind11_FOUND)
        message(FATAL_ERROR "QRAM_BUILD_PYTHON_BINDINGS=ON requires pybind11")
    endif()
    add_subdirectory(bindings/python)
endif()

# --- Install rules ---
include(GNUInstallDirs)

set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/QRAMSimulator")

install(TARGETS SparQ SparQ_Common SparQ_QRAMSimulator SparQ_Algorithm SparQ_Simulator
    EXPORT QRAMSimulatorTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY
    Common/include/
    QRAM/include/
    SparQ/include/
    SparQ_Algorithm/include/
    ThirdParty/eigen-3.4.0/
    ThirdParty/fmt/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT QRAMSimulatorTargets
    FILE QRAMSimulatorTargets.cmake
    NAMESPACE QRAMSimulator::
    DESTINATION ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QRAMSimulatorConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QRAMSimulatorConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QRAMSimulatorConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

# Examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
