# QRAM-Simulator 的 pybind11 薄绑定层。
# 由根 CMakeLists 在 QRAM_BUILD_PYTHON_BINDINGS=ON 时引入；pybind11 由
# scikit-build-core 的构建隔离环境（pip）或系统安装提供（find_package 在
# 根 CMakeLists 的选项守卫内完成，嵌入消费方不受影响）。

pybind11_add_module(_core core_binding.cpp)
target_compile_definitions(_core PRIVATE QRAM_Release)
target_link_libraries(_core PRIVATE SparQ_QRAMSimulator SparQ_Common)

# 安装目录即 wheel 的包目录（配合 pyproject.toml 的 wheel.packages/platlib）
install(TARGETS _core DESTINATION qram_simulator)
