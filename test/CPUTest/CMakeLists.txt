
add_executable(SparQ_Example "SparQ_Example.cpp")
target_link_libraries(SparQ_Example SparQ)

add_subdirectory(QASMTest)
add_subdirectory(CommonTest)