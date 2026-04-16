# SparQ Unit Tests
# Uses Google Test for unit testing SparQ core operations

# Create test executables
add_executable(hadamard_test hadamard.cpp)
target_link_libraries(hadamard_test SparQ GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(hadamard_test)

add_executable(qft_test qft.cpp)
target_link_libraries(qft_test SparQ GTest::gtest_main)
gtest_discover_tests(qft_test)

add_executable(basic_gates_test basic_gates.cpp)
target_link_libraries(basic_gates_test SparQ GTest::gtest_main)
gtest_discover_tests(basic_gates_test)

add_executable(condrot_test condrot.cpp)
target_link_libraries(condrot_test SparQ GTest::gtest_main)
gtest_discover_tests(condrot_test)

add_executable(quantum_arithmetic_test quantum_arithmetic.cpp)
target_link_libraries(quantum_arithmetic_test SparQ GTest::gtest_main)
gtest_discover_tests(quantum_arithmetic_test)
