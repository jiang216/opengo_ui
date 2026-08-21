# ---------- core: 纯 C++，禁止 include GL/媒体头文件 ----------
add_library(ui_core STATIC
    src/log.cpp
)

target_include_directories(ui_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

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
