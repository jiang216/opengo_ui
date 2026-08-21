# ---------- layui_test: 克隆 layui 官方 demo 网站示例的测试项目 ----------
# 独立工程：引用仓库 core/gl/ui 静态库 + glfw，仅需 glfw3.dll + iconfont.ttf + logo.png 依赖。
# VS 2026 打开 layui_test.slnx（由 CMake 4.2.3 "Visual Studio 18 2026" 生成器产出）即可 F5 调试。
#
# 两种消费模式：
#   1. add_subdirectory 模式（默认）：通过相对路径引用父仓库的 core/gl/ui 源码
#   2. find_package 模式：opengl_ui 已安装到 CMAKE_PREFIX_PATH，通过 find_package 获取
cmake_minimum_required(VERSION 3.29)
project(layui_test VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

# ---- 选择消费模式 ----
option(LAYUI_TEST_USE_FIND_PACKAGE "Use find_package(opengl_ui) instead of add_subdirectory" OFF)

if(LAYUI_TEST_USE_FIND_PACKAGE)
    # ---- find_package 模式：opengl_ui 已安装到系统或 CMAKE_PREFIX_PATH ----
    find_package(opengl_ui REQUIRED)
    set(_LAYUI_TEST_LIB opengl_ui::ui_ui)
    set(_LAYUI_TEST_DLL_DIR "")  # DLL 由 install prefix 管理
else()
    # ---- add_subdirectory 模式：复用父仓库源码（默认开发模式） ----
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(OPENGUI_ARCH "x64")
    else()
        set(OPENGUI_ARCH "x86")
    endif()
    set(OPENGUI_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/install/${OPENGUI_ARCH}")
    message(STATUS "layui_test: arch=${OPENGUI_ARCH}, 3rdparty=${OPENGUI_3RDPARTY}")

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core core)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gl gl)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ui ui)
    set(_LAYUI_TEST_LIB ui_ui)
    set(_LAYUI_TEST_DLL_DIR "${OPENGUI_3RDPARTY}/bin")
endif()

# ---- 测试工程可执行 ----
file(GLOB LAYUI_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/pages/*.cpp)
add_executable(layui_test ${LAYUI_TEST_SOURCES})
target_link_libraries(layui_test PRIVATE ${_LAYUI_TEST_LIB})

# ---- 依赖拷贝：glfw3.dll + iconfont.ttf + logo.png + avatar 到可执行目录（VS 调试/直接运行开箱可用） ----
if(_LAYUI_TEST_DLL_DIR)
    add_custom_command(TARGET layui_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_LAYUI_TEST_DLL_DIR}/glfw3.dll"
            $<TARGET_FILE_DIR:layui_test>
    )
endif()
add_custom_command(TARGET layui_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/iconfont.ttf"
        $<TARGET_FILE_DIR:layui_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/src/layui/static/images/layui/logo.png"
        $<TARGET_FILE_DIR:layui_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/src/layui/avatar/37326744.png"
        $<TARGET_FILE_DIR:layui_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/01.jpg"
        $<TARGET_FILE_DIR:layui_test>/a01.jpg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/05.png"
        $<TARGET_FILE_DIR:layui_test>/a02.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/08.png"
        $<TARGET_FILE_DIR:layui_test>/a03.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/A_01.jpg"
        $<TARGET_FILE_DIR:layui_test>/a04.jpg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/back.png"
        $<TARGET_FILE_DIR:layui_test>/a05.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/cs.png"
        $<TARGET_FILE_DIR:layui_test>/a06.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../file/img/log.jpg"
        $<TARGET_FILE_DIR:layui_test>/a07.jpg
)
