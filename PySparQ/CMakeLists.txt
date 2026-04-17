# Require CMake 3.15+ (matching scikit-build-core) Use new versions of all
# policies up to CMake 3.27
cmake_minimum_required(VERSION 3.15...3.27)

# Scikit-build-core sets these values for you, or you can just hard-code the
# name and version.
project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX)

# Find the module development requirements (requires FindPython from 3.17 or
# scikit-build-core's built-in backport)
# Use FindPython3 to match what pybind11 uses
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
# find_package(pybind11 CONFIG REQUIRED)

# Add a library using FindPython's tooling (pybind11 also provides a helper like
# this)
python_add_library(_core MODULE core.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers)
target_link_libraries(_core PRIVATE SparQ)

# Add dynamic_operator module
python_add_library(_dynamic_operator MODULE pysparq/dynamic_operator/src/dynamic_operator_loader.cpp WITH_SOABI)
target_link_libraries(_dynamic_operator PRIVATE pybind11::headers)
target_include_directories(_dynamic_operator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/pysparq/dynamic_operator/include
    ${CMAKE_SOURCE_DIR}/SparQ/include
    ${CMAKE_SOURCE_DIR}/Common/include
    ${CMAKE_SOURCE_DIR}/QRAM/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/eigen-3.4.0
    ${CMAKE_SOURCE_DIR}/ThirdParty/fmt/include
)

# This is passing in the version as a define just as an example
target_compile_definitions(_core PRIVATE VERSION_INFO=${PYQSPARSE_VERSION_INFO})

# The install directory is the output (wheel) directory
install(TARGETS _core DESTINATION pysparq)
install(TARGETS _dynamic_operator DESTINATION pysparq)
