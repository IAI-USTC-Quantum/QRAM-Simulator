# Shared CUDA target setup for QRAM-Simulator / SparQSim
# 用法：add_library(<name> ..._cuda ...) 之后调用 sparq_setup_cuda_target(<name>)
# 说明：所有 CUDA 目标共用此配置，避免旗标块在多处复制漂移。
include_guard(GLOBAL)

# VS 生成器环境预备：NVIDIA 的 CUDA <ver>.targets 通过 CUDA_PATH_V<ver> 或
# CudaToolkitDir 定位工具包目录，安装器漏写注册表/环境时 configure 直接失败
# （报 "The CUDA Toolkit directory '' does not exist"）。MSBuild 会把进程
# 环境变量当作全局属性，故在 enable_language 之前从 nvcc 实际位置推导并注入，
# 使用户无需手工配置环境变量。非 VS 生成器无此问题，直接跳过。
function(sparq_prepare_cuda_env)
    if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
        return()
    endif()
    if(DEFINED ENV{CudaToolkitDir} AND NOT "$ENV{CudaToolkitDir}" STREQUAL "")
        return()
    endif()

    # 注意：不能预置空变量再 find_program——变量已存在会令搜索被跳过。
    if(DEFINED ENV{CUDACXX} AND EXISTS "$ENV{CUDACXX}")
        set(_sparq_nvcc "$ENV{CUDACXX}")
    else()
        find_program(_sparq_nvcc nvcc)
    endif()

    if(_sparq_nvcc)
        get_filename_component(_sparq_cuda_root "${_sparq_nvcc}" DIRECTORY) # .../bin
        get_filename_component(_sparq_cuda_root "${_sparq_cuda_root}" DIRECTORY)
        if(EXISTS "${_sparq_cuda_root}/include/cuda_runtime.h")
            set(ENV{CudaToolkitDir} "${_sparq_cuda_root}")
            message(STATUS "CUDA (VS generator): CudaToolkitDir=${_sparq_cuda_root}")
        endif()
    endif()
endfunction()

function(sparq_setup_cuda_target target)
    set_target_properties(${target} PROPERTIES
        CUDA_STANDARD 17
        CUDA_STANDARD_REQUIRED ON
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
    )

    # VS 生成器：把工具包根目录以 Globals 属性写入 vcxproj（其求值先于
    # CUDA <ver>.props 的目录探测），使 build/IDE/CI 都无需环境变量。
    if(CMAKE_GENERATOR MATCHES "Visual Studio" AND CMAKE_CUDA_COMPILER)
        get_filename_component(_sparq_cuda_root "${CMAKE_CUDA_COMPILER}" DIRECTORY) # .../bin
        get_filename_component(_sparq_cuda_root "${_sparq_cuda_root}" DIRECTORY)
        if(EXISTS "${_sparq_cuda_root}/include/cuda_runtime.h")
            set_target_properties(${target} PROPERTIES
                VS_GLOBAL_CudaToolkitDir "${_sparq_cuda_root}")
        endif()
    endif()

    # 目标架构：默认继承 CMAKE_CUDA_ARCHITECTURES（父级或命令行设置）；
    # 完全未设置时回退 native（要求配置机有 GPU）。
    get_target_property(_sparq_arch ${target} CUDA_ARCHITECTURES)
    if(NOT _sparq_arch AND NOT CMAKE_CUDA_ARCHITECTURES)
        set_target_properties(${target} PROPERTIES CUDA_ARCHITECTURES "native")
    endif()

    # 注意：--extended-lambda 与 --expt-relaxed-constexpr 在 CUDA 12/13
    # 均非默认开启（实测），必须显式保留。
    set(_sparq_cuda_flags
        --generate-line-info
        --relocatable-device-code=true
        --extended-lambda
        --expt-relaxed-constexpr
    )
    if(MSVC)
        # CCCL 3.x（CUDA 13.x 捆绑版）要求 MSVC 宿主使用标准符合预处理器，
        # 否则 <cuda/std/*> 头文件直接 #error 拒绝编译（实测）。
        list(APPEND _sparq_cuda_flags -Xcompiler=/Zc:preprocessor)
    endif()
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:${_sparq_cuda_flags}>")

    # USE_CUDA 改变 System 等数据结构的内存布局（std::vector ↔ std::array），
    # 消费方编译时必须看到一致的开关，故为 PUBLIC。FMT_UNICODE=0 与核心仓库
    # 的全局定义一致——fmt 的 Unicode 路径要求 _UTF8 宏（cl 的 /utf-8 产物），
    # nvcc 前端解析 fmt 头文件时没有该宏，会触发 static_assert 与字符越界错误。
    target_compile_definitions(${target} PUBLIC USE_CUDA=1 EIGEN_NO_CUDA FMT_UNICODE=0)

    target_link_libraries(${target} PRIVATE cudart cudadevrt)
endfunction()
