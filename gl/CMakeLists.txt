# ---------- gl: GLFW + GLAD + stb，只做渲染，不认识 OpenCV/FFmpeg ----------
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED PATHS "${OPENGUI_3RDPARTY}/lib/cmake/glfw3" NO_DEFAULT_PATH)

add_library(ui_gl STATIC
    src/window.cpp
    src/renderer.cpp
    src/texture.cpp
    src/font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/glad/src/gl.c
)

target_include_directories(ui_gl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/glad/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/stb>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(ui_gl PUBLIC ui_core glfw OpenGL::GL)

# MSVC：GLAD 的 gl.c 是 C 文件，加 -TP 让 MSVC 按 C++ 编译（避免 C 代码警告当错误）
if(MSVC)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/glad/src/gl.c PROPERTIES LANGUAGE CXX)
endif()

# ---------- install ----------
include(GNUInstallDirs)

install(TARGETS ui_gl EXPORT opengl_uiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/gl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui
)

# GLAD + stb 头文件（被 ui_gl PUBLIC include 引用，下游也需要）
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/glad/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/stb/stb_image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/stb/stb_truetype.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
