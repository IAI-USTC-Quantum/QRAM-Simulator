add_executable(QasmTest QasmTest.cpp)
target_link_libraries(QasmTest SparQ)

add_executable(displayTest displayTest.cpp)
target_link_libraries(displayTest SparQ)