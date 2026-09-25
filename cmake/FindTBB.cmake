# Custom FindTBB.cmake
# Modify according to your TBB installation path

# Try to find the TBB headers
find_path(TBB_INCLUDE_DIR
  NAMES tbb/tbb.h
  PATHS
    ${TBB_ROOT}/include
    $ENV{TBB_ROOT}/include
    /usr/include
    /usr/local/include
    $ENV{PROGRAMFILES}/tbb/include
)

# Try to find the TBB library
find_library(TBB_LIBRARY
  NAMES tbb
  PATHS
    ${TBB_ROOT}/lib
    ${TBB_ROOT}/lib64
    $ENV{TBB_ROOT}/lib
    $ENV{TBB_ROOT}/lib64
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64
    $ENV{PROGRAMFILES}/tbb/lib
)

include(FindPackageHandleStandardArgs)
# Handle the standard arguments and set TBB_FOUND
find_package_handle_standard_args(TBB DEFAULT_MSG TBB_LIBRARY TBB_INCLUDE_DIR)

if(TBB_FOUND)
  set(TBB_LIBRARIES ${TBB_LIBRARY})
  set(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR})

  # Create the imported target
  if(NOT TARGET TBB::tbb)
    add_library(TBB::tbb UNKNOWN IMPORTED)
    set_target_properties(TBB::tbb PROPERTIES
      IMPORTED_LOCATION "${TBB_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${TBB_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(TBB_INCLUDE_DIR TBB_LIBRARY)
