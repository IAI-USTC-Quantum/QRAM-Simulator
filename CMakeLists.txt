cmake_minimum_required(VERSION 3.18)

project(QRAMSimulator)

include_directories(${PROJECT_SOURCE_DIR}/Common/include
                    ${PROJECT_SOURCE_DIR}/QRAM/include
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
option(QRAM_BUILD_TESTS "Build C++ test targets (test/)" ON)
option(QRAM_BUILD_EXPERIMENTS "Build QRAM experiment programs (Experiments/)" ON)
option(QRAM_BUILD_PYTHON_BINDINGS "Build pybind11 Python bindings (bindings/python/)" OFF)
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

if (UNIX)
	add_compile_options(-fPIC)
endif(UNIX)

# MSVC: force UTF-8 source interpretation. Headers carry UTF-8 Chinese
# comments; on legacy code pages (e.g. CP936) MSVC mis-decodes them, which
# can merge an #ifdef line into a preceding comment and break the build.
if (MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(Common)
add_subdirectory(QRAM)

# Expose the flat header layout to consumers that link these libraries from
# OUTSIDE this subtree (e.g. SparQSim, which embeds the repo as a git
# submodule and builds its own SparQ umbrella target on top).
# The include_directories() call at the top of this file only reaches targets
# inside this directory tree. BUILD_INTERFACE serves add_subdirectory
# consumers; INSTALL_INTERFACE matches the flat include/ layout installed by
# the install(DIRECTORY ...) rules at the bottom of this file.
include(GNUInstallDirs)
set(QRAM_CORE_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/Common/include
    ${PROJECT_SOURCE_DIR}/QRAM/include
    ${PROJECT_SOURCE_DIR}/ThirdParty
    ${PROJECT_SOURCE_DIR}/ThirdParty/argparse
    ${PROJECT_SOURCE_DIR}/ThirdParty/fmt/include
    ${PROJECT_SOURCE_DIR}/ThirdParty/eigen-3.4.0
)
target_include_directories(SparQ_QRAMSimulator INTERFACE
    "$<BUILD_INTERFACE:${QRAM_CORE_INCLUDE_DIRS}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_include_directories(SparQ_Common INTERFACE
    "$<BUILD_INTERFACE:${QRAM_CORE_INCLUDE_DIRS}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_subdirectory(ThirdParty EXCLUDE_FROM_ALL)

# Python 绑定（pybind11）：默认 OFF，嵌入消费方（SparQSim 的
# add_subdirectory）与纯 C++ CI 不受影响。开启时 pybind11 由
# scikit-build-core 的构建隔离环境（pip）或系统安装提供。
# 注意：pybind11 的 imported target 是目录作用域，find_package 必须
# 在本目录内完成，不能依赖上层消费者代为查找。
if(QRAM_BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)
    add_subdirectory(bindings/python)
endif()

if(QRAM_BUILD_TESTS)
    add_subdirectory(test)
endif()

if(QRAM_BUILD_EXPERIMENTS)
    add_subdirectory(Experiments)
endif()

# --- Install rules ---
# (GNUInstallDirs is included above, before the INTERFACE includes)
#
# C++ 开发包安装规则（静态库 + 平铺头文件 + CMake package config）。
# wheel 构建（QRAM_BUILD_PYTHON_BINDINGS=ON，scikit-build-core 把安装前缀
# 指向 wheel 的 platlib）时跳过，避免把 include/、lib/ 灌进 site-packages；
# 此时仅有 bindings/python 里 _core 扩展的 install 规则生效。
if(NOT QRAM_BUILD_PYTHON_BINDINGS)

set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/QRAMSimulator")

install(TARGETS SparQ_Common SparQ_QRAMSimulator
    EXPORT QRAMSimulatorTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY
    Common/include/
    QRAM/include/
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

endif() # NOT QRAM_BUILD_PYTHON_BINDINGS（跳过 C++ 开发包安装）
