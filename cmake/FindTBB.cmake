# 自定义的 FindTBB.cmake
# 根据您的 TBB 安装路径进行修改

# 尝试查找 TBB 头文件
find_path(TBB_INCLUDE_DIR
  NAMES tbb/tbb.h
  PATHS
    ${TBB_ROOT}/include
    $ENV{TBB_ROOT}/include
    /usr/include
    /usr/local/include
    $ENV{PROGRAMFILES}/tbb/include
)

# 尝试查找 TBB 库文件
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
# 处理标准参数并设置 TBB_FOUND
find_package_handle_standard_args(TBB DEFAULT_MSG TBB_LIBRARY TBB_INCLUDE_DIR)

if(TBB_FOUND)
  set(TBB_LIBRARIES ${TBB_LIBRARY})
  set(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR})
  
  # 创建导入目标
  if(NOT TARGET TBB::tbb)
    add_library(TBB::tbb UNKNOWN IMPORTED)
    set_target_properties(TBB::tbb PROPERTIES
      IMPORTED_LOCATION "${TBB_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${TBB_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(TBB_INCLUDE_DIR TBB_LIBRARY)

