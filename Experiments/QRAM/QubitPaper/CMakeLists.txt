project(Experiment_QRAM_QubitPaper)

add_executable(${PROJECT_NAME} QubitPaperScan.cpp)
target_link_libraries(${PROJECT_NAME} SparQ_QRAMSimulator SparQ_Common)

add_executable(${PROJECT_NAME}Verify QubitPaperVerify.cpp)
target_link_libraries(${PROJECT_NAME}Verify SparQ_QRAMSimulator SparQ_Common)

add_executable(${PROJECT_NAME}ExactTest QubitPaperExactTest.cpp)
target_link_libraries(${PROJECT_NAME}ExactTest SparQ_QRAMSimulator SparQ_Common)
add_test(NAME QubitPaperExactness COMMAND ${PROJECT_NAME}ExactTest)
add_test(NAME QubitPaperExactnessChannels COMMAND ${PROJECT_NAME}ExactTest --channels)
