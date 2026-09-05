# ---------- media/core：媒体管线 + 合成源 + 视频容器（纯 core/gl/ui 依赖，无第三方） ----------
# 分层（UI-191）：media/core 定义 Source/Decoder/AudioSink 节点接口与组装、以及合成源；
# media/ffmpeg 等具体实现模块依赖本库并在运行期按名注册节点。luai_test / 纯 UI 工程
# 无需 FFmpeg 即可用合成源演示视频容器。
add_library(ui_media_core STATIC
    src/registry.cpp
    src/pipeline.cpp
    src/test_pattern_source.cpp
    src/video_container.cpp
)
target_include_directories(ui_media_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ui_media_core PUBLIC ui_ui)

include(GNUInstallDirs)

install(TARGETS ui_media_core EXPORT opengl_uiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/media
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui
)