project(Experiment_Grover)

file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${src})

target_link_libraries(${PROJECT_NAME} SparQ)