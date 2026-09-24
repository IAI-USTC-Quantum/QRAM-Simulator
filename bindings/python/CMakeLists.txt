# Thin Python bindings for the qram_simulator PyPI package.
# pybind11 is found by the root CMakeLists (find_package(pybind11 CONFIG) or
# provided by scikit-build-core's isolated build environment).

pybind11_add_module(_core core_binding.cpp)
target_compile_definitions(_core PRIVATE QRAM_Release)
target_link_libraries(_core PRIVATE SparQ)

# The install directory is the output (wheel) directory
install(TARGETS _core DESTINATION qram_simulator)
