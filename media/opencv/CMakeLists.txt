# ---------- media/opencv（可选）：图像处理 -> Texture ----------
# OpenCV 查找策略：优先系统安装（find_package 默认路径），回退到3rdparty本地构建
if(NOT OpenCV_FOUND)
    find_package(OpenCV CONFIG REQUIRED
        PATHS
            "${OPENGUI_3RDPARTY}/x64/vc16/lib"
            "${OPENGUI_3RDPARTY}/x86/vc16/lib"
        NO_DEFAULT_PATH)
endif()

add_library(ui_media_opencv STATIC src/image_view.cpp)
target_include_directories(ui_media_opencv PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ui_media_opencv PUBLIC ui_ui ${OpenCV_LIBS})

# ---------- install ----------
include(GNUInstallDirs)

install(TARGETS ui_media_opencv EXPORT opengl_uiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# media/opencv 头文件（可能和 ffmpeg 的 media/ 重叠，按文件安装）
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/media/image_view.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui/media
)
