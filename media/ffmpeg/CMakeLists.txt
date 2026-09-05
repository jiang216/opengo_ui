# ---------- media/ffmpeg（可选）：视频解码 -> Texture ----------
# FFmpeg 目标由根 CMakeLists 的 find_package 提供（apps 也需要）
# UI-191：VideoView 实现迁到媒体管线（ui_media_core），此处提供 FFmpeg 节点（ffmpeg_nodes.cpp）
add_library(ui_media_ffmpeg STATIC src/video_view.cpp src/ffmpeg_nodes.cpp src/audio_out.cpp)
target_include_directories(ui_media_ffmpeg PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ui_media_ffmpeg PUBLIC ui_media_core ui_ui FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil FFmpeg::swscale FFmpeg::swresample)

# ---------- install ----------
include(GNUInstallDirs)

install(TARGETS ui_media_ffmpeg EXPORT opengl_uiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/media
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui
)
