# Thin pybind11 binding layer of QRAM-Simulator.
# Included by the root CMakeLists when QRAM_BUILD_PYTHON_BINDINGS=ON;
# pybind11 is provided either by scikit-build-core's build-isolation
# environment (pip) or by a system installation (find_package happens
# inside the option guard of the root CMakeLists, so embedded consumers
# are unaffected).

pybind11_add_module(_core core_binding.cpp)
target_compile_definitions(_core PRIVATE QRAM_Release)
target_link_libraries(_core PRIVATE SparQ_QRAMSimulator SparQ_Common)

# The install destination is the package directory of the wheel
# (pairs with wheel.packages/platlib in pyproject.toml)
install(TARGETS _core DESTINATION qram_simulator)
