
add_executable(ExecutabilityTest ExecutabilityTest.cpp)
target_link_libraries(ExecutabilityTest SparQ)

add_executable(CorrectnessTest CorrectnessTest.cpp)
target_link_libraries(CorrectnessTest SparQ)

add_test(NAME ExecutabilityTest COMMAND ExecutabilityTest)
add_test(NAME CorrectnessTest COMMAND CorrectnessTest)
