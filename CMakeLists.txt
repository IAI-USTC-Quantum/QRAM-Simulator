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

# Enable testing
enable_testing()

option(QRAM_ENABLE_CUDA "Enable CUDA GPU support" OFF)

if(CACHED_REGISTER_SIZE)
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

# Google Test - Use FetchContent for consistent cross-platform builds
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
message(STATUS "GTest configured via FetchContent")

if (UNIX)
	add_compile_options(-fPIC)
endif(UNIX)

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
add_subdirectory(SparQ/test)
add_subdirectory(SparQ_Algorithm)
add_subdirectory(ThirdParty EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_subdirectory(Experiments)

# PySparQ requires pybind11
if(pybind11_FOUND)
    add_subdirectory(PySparQ)
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
