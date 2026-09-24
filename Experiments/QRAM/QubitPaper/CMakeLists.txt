project(Experiment_QRAM_QubitPaper)

add_executable(${PROJECT_NAME} QubitPaperScan.cpp)
target_link_libraries(${PROJECT_NAME} SparQ)

add_executable(${PROJECT_NAME}Verify QubitPaperVerify.cpp)
target_link_libraries(${PROJECT_NAME}Verify SparQ)
