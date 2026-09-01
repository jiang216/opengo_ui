# ---------- core: 纯 C++，禁止 include GL/媒体头文件 ----------
add_library(ui_core STATIC
    src/log.cpp
)

if(OPENGUI_USE_PCH)
    target_precompile_headers(ui_core PRIVATE
        <string>
        <vector>
        <cstdint>
    )
endif()

target_include_directories(ui_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 头文件为 C++17（标准库容器等），以 INTERFACE 需求导出——
# 顶层 CMAKE_CXX_STANDARD 17 仅限本仓库，不随 install/export 传给下游；
# 挂在依赖链底层的 ui_core（被 ui_gl/ui_ui/media PUBLIC 链接）上，经接口传播强制下游 C++17。
target_compile_features(ui_core PUBLIC cxx_std_17)

# 头文件加入 IDE
file(GLOB UI_CORE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/core/*.h)
target_sources(ui_core PRIVATE ${UI_CORE_HEADERS})

# ---------- install ----------
include(GNUInstallDirs)

install(TARGETS ui_core EXPORT opengl_uiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/core
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui
)
