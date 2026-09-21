cmake_minimum_required(VERSION 3.24)
project(opengl_ui VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- 发布构建优化（阶段4）：LTO + 高优化等级 ----
# GCC/Clang 用 -O3 + 显式 LTO；MSVC 用 /O2（已含 LTCG 由 CMAKE_INTERPROCEDURAL_OPTIMIZATION 控制）。
# 统一收敛各类静态库/可执行目标，reduce 迭代产出体积并提升运行性能。
# 注意：多配置生成器（Visual Studio）下 CMAKE_BUILD_TYPE 仅 configure 时求值，
# 必须用生成器表达式按实际构建配置分发优化标志，否则 /O2 会泄漏到 Debug 配置
# 与 MSVC 自动追加的 /RTC1 冲突（error D8016）。
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    if(MSVC)
        add_compile_options("$<$<CONFIG:Release>:/O2>")
    else()
        add_compile_options("$<$<CONFIG:Release>:-O3>")
    endif()
endif()

# PCH（可选项，默认开；编译慢/兼容问题时 -DOPENGUI_USE_PCH=OFF 关闭）
option(OPENGUI_USE_PCH "Use precompiled headers to speed up builds" ON)

# 源码（头文件注释等）为 UTF-8，MSVC 默认按 GBK 读会报 C2447/C4819
if(MSVC)
    add_compile_options(/utf-8)
endif()

# GLFW 默认在未定义 GLFW_INCLUDE_NONE 时 include 系统 <GL/gl.h>（仅 Windows 分支），
# 与 GLAD 的 <glad/gl.h> 在同一 TU 冲突（glad 会 #error "already included"）。
# 统一定义 GLFW_INCLUDE_NONE，由各 TU 显式 include <glad/gl.h> 提供 GL API。
add_compile_definitions(GLFW_INCLUDE_NONE)

# ---- 可选模块开关（默认 OFF，纯 UI 不依赖媒体库） ----
option(OPENGUI_MEDIA_CORE "Build media core module (pipeline + synthetic source)" OFF)
option(OPENGUI_MEDIA_FFMPEG "Build FFmpeg media module (video decode)" OFF)
option(OPENGUI_MEDIA_OPENCV "Build OpenCV media module (image processing)" OFF)
option(OPENGUI_BUILD_APPS "Build demo applications" ON)

# ---- 架构 -> 3rdparty 安装前缀 ----
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OPENGUI_ARCH "x64")
else()
    set(OPENGUI_ARCH "x86")
endif()
set(OPENGUI_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/install/${OPENGUI_ARCH}")
message(STATUS "opengl_ui: arch=${OPENGUI_ARCH}, 3rdparty=${OPENGUI_3RDPARTY}")

add_subdirectory(core)
add_subdirectory(gl)
add_subdirectory(ui)
if(OPENGUI_MEDIA_CORE OR OPENGUI_MEDIA_FFMPEG OR OPENGUI_MEDIA_OPENCV)
    if(OPENGUI_MEDIA_FFMPEG)
        list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
        find_package(FFmpeg REQUIRED)
    endif()
    add_subdirectory(media)
endif()
if(OPENGUI_BUILD_APPS)
    add_subdirectory(apps)
endif()

# ===================================================================
#                     install / export / package config
# ===================================================================
include(GNUInstallDirs)

# ---- 安装 FindFFmpeg.cmake（下游 find_package(FFmpeg) 需要） ----
if(OPENGUI_MEDIA_FFMPEG)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindFFmpeg.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opengl_ui
    )
endif()

# ---- 导出 targets ----
install(EXPORT opengl_uiTargets
    FILE opengl_uiTargets.cmake
    NAMESPACE opengl_ui::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opengl_ui
)

# ---- 生成 Config.cmake（调用 find_dependency 查找 GLFW/OpenGL/FFmpeg/OpenCV） ----
include(CMakePackageConfigHelpers)

set(_OPENGUI_HAS_FFMPEG $<BOOL:${OPENGUI_MEDIA_FFMPEG}>)
set(_OPENGUI_HAS_OPENCV $<BOOL:${OPENGUI_MEDIA_OPENCV}>)
set(_OPENGUI_PROJECT_VERSION ${PROJECT_VERSION})

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/opengl_uiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/opengl_uiConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opengl_ui
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/opengl_uiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/opengl_uiConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/opengl_uiConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opengl_ui
)

# ---- 安装运行时资产（图标字体等） ----
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/iconfont.ttf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/opengl_ui
)

# ===================================================================
#          打包第三方依赖到 install 目录（自包含分发）
# ===================================================================
# 目标：install 目录即完整运行包，下游只需 CMAKE_PREFIX_PATH 指向它即可

# ---- GLFW：DLL + import lib + headers + cmake config ----
install(FILES "${OPENGUI_3RDPARTY}/bin/glfw3.dll"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES "${OPENGUI_3RDPARTY}/lib/glfw3dll.lib"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY "${OPENGUI_3RDPARTY}/include/GLFW"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY "${OPENGUI_3RDPARTY}/lib/cmake/glfw3"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
)

# ---- FFmpeg：DLLs + import libs + headers ----
if(OPENGUI_MEDIA_FFMPEG)
    file(GLOB _FFMPEG_DLLS "${OPENGUI_3RDPARTY}/bin/*.dll")
    install(FILES ${_FFMPEG_DLLS} DESTINATION ${CMAKE_INSTALL_BINDIR})
    # import lib 在 bin/ 下（FFmpeg --toolchain=msvc 安装布局）
    file(GLOB _FFMPEG_IMPLIBS "${OPENGUI_3RDPARTY}/bin/*.lib")
    install(FILES ${_FFMPEG_IMPLIBS} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY "${OPENGUI_3RDPARTY}/include/libavcodec"
        "${OPENGUI_3RDPARTY}/include/libavformat"
        "${OPENGUI_3RDPARTY}/include/libavutil"
        "${OPENGUI_3RDPARTY}/include/libswscale"
        "${OPENGUI_3RDPARTY}/include/libswresample"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# ---- OpenCV：DLL + import lib + headers + cmake config ----
if(OPENGUI_MEDIA_OPENCV)
    # 维持 OpenCV 原始嵌套布局（x64/vc16/），其 cmake config 依赖此结构
    file(GLOB _OPENCV_DLLS "${OPENGUI_3RDPARTY}/*/vc16/bin/opencv_world*.dll")
    install(FILES ${_OPENCV_DLLS} DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY "${OPENGUI_3RDPARTY}/${OPENGUI_ARCH}/vc16/lib"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opencv
        FILES_MATCHING PATTERN "*.lib" PATTERN "*.cmake"
    )
    install(DIRECTORY "${OPENGUI_3RDPARTY}/include/opencv2"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# ---- Documentation ----
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/distribution/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/distribution/API_REFERENCE.md"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# ---- MCP 知识服务（opengui）：随包发布，供 IDE / CLI 智能体查证 API 与模板 ----
# server.py + 数据 JSON 同目录部署（--data 免配置）；CHANGELOG/ARCHITECTURE 供检索溯源。
# 消费入口：share/opengl_ui/OPENGUI_AGENT.md；注入命令：share/opengl_ui/register-mcp.ps1。
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mcp/server.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/mcp/catalog.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/mcp/templates.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/mcp/constraints.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/CHANGELOG.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/ARCHITECTURE.md"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/opengl_ui/mcp
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/publish/register-mcp.ps1"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/publish/OPENGUI_AGENT.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/publish/publish.ps1"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/opengl_ui
)
