# Require CMake 3.15+ (matching scikit-build-core) Use new versions of all
# policies up to CMake 3.27
cmake_minimum_required(VERSION 3.15...3.27)

# Scikit-build-core sets these values for you, or you can just hard-code the
# name and version.
project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX)


file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(QDAAlgo qda_algo.cpp)
target_link_libraries(QDAAlgo PRIVATE SparQ)

# Find the module development requirements (requires FindPython from 3.17 or
# scikit-build-core's built-in backport)
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
# For pybind11 installed by pip
# find_package(pybind11 CONFIG REQUIRED)

# Add a library using FindPython's tooling (pybind11 also provides a helper like
# this)
python_add_library(_core MODULE ${src} WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers)
target_link_libraries(_core PRIVATE SparQ)

# This is passing in the version as a define just as an example
target_compile_definitions(_core PRIVATE VERSION_INFO=${PYQSPARSE_VERSION_INFO})

# set /bigobj for MSVC to handle large object files
if(MSVC)
  target_compile_options(_core PRIVATE /bigobj)
  # target_compile_options(_core PRIVATE /FS)
endif()

# The install directory is the output (wheel) directory
install(TARGETS _core DESTINATION pysparq)
