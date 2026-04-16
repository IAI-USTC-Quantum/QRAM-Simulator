# CMakeLists.txt for QRAM-Simulator Examples

# Hello QRAM - Basic QRAM usage example
add_executable(hello_qram hello_qram.cpp)
target_link_libraries(hello_qram SparQ)
set_target_properties(hello_qram PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Basic Gates - Quantum gates demonstration
add_executable(basic_gates basic_gates.cpp)
target_link_libraries(basic_gates SparQ)
set_target_properties(basic_gates PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Grover Search - Grover's algorithm with QRAM
add_executable(grover_search grover_search.cpp)
target_link_libraries(grover_search SparQ)
set_target_properties(grover_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable OpenMP for examples
if(OpenMP_CXX_FOUND)
    target_link_libraries(hello_qram OpenMP::OpenMP_CXX)
    target_link_libraries(basic_gates OpenMP::OpenMP_CXX)
    target_link_libraries(grover_search OpenMP::OpenMP_CXX)
endif()
