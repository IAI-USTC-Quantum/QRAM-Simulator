add_subdirectory(fmt EXCLUDE_FROM_ALL)
# add_subdirectory(eigen-3.4.0 EXCLUDE_FROM_ALL)

# Try to use pybind11 from subdirectory first (for CI with submodules),
# fallback to find_package if not available (for local development)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pybind11/CMakeLists.txt")
    add_subdirectory(pybind11 EXCLUDE_FROM_ALL)
else()
    find_package(pybind11 REQUIRED)
endif()