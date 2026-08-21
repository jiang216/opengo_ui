# ---------- apps: 演示 + 冒烟测试 ----------
add_executable(opengl_ui_demo ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(opengl_ui_demo PRIVATE ui_ui)

# API 总调用示例 / 冒烟（docs/UI_API.md 的可执行伴随程序，走聚合头 ui/all.h）
add_executable(opengl_ui_api_demo ${CMAKE_CURRENT_SOURCE_DIR}/api_demo.cpp)
target_link_libraries(opengl_ui_api_demo PRIVATE ui_ui)

# 素材加载测试工具（真实图片/音频/视频逐个加载验证；中文路径经 wmain 宽入口转 UTF-8）
# 依赖 FFmpeg + OpenCV 双库，故仅在两者都开启时构建
if(OPENGUI_MEDIA_FFMPEG AND OPENGUI_MEDIA_OPENCV)
    add_executable(opengl_ui_media_load_test ${CMAKE_CURRENT_SOURCE_DIR}/media_load_test.cpp)
    target_link_libraries(opengl_ui_media_load_test PRIVATE ui_ui ui_media_ffmpeg ui_media_opencv)
    target_compile_definitions(opengl_ui_media_load_test PRIVATE OPENGUI_MEDIA_FFMPEG=1
        OPENGUI_MEDIA_OPENCV=1)
endif()

if(OPENGUI_MEDIA_FFMPEG)
    target_link_libraries(opengl_ui_demo PRIVATE ui_media_ffmpeg)
    target_compile_definitions(opengl_ui_demo PRIVATE OPENGUI_MEDIA_FFMPEG=1)
    target_link_libraries(opengl_ui_api_demo PRIVATE ui_media_ffmpeg)
    target_compile_definitions(opengl_ui_api_demo PRIVATE OPENGUI_MEDIA_FFMPEG=1)
    # M16：视频播放器示例应用
    add_executable(opengl_ui_video_player ${CMAKE_CURRENT_SOURCE_DIR}/video_player.cpp)
    target_link_libraries(opengl_ui_video_player PRIVATE ui_media_ffmpeg)
    target_compile_definitions(opengl_ui_video_player PRIVATE OPENGUI_MEDIA_FFMPEG=1)
    # 开发工具：生成带音频测试视频（M14 遗留修复验证素材）
    add_executable(opengl_ui_gen_av ${CMAKE_SOURCE_DIR}/tools/gen_media/gen_av.cpp)
    target_link_libraries(opengl_ui_gen_av PRIVATE FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil
        FFmpeg::swscale FFmpeg::swresample)
endif()
if(OPENGUI_MEDIA_OPENCV)
    target_link_libraries(opengl_ui_demo PRIVATE ui_media_opencv)
    target_compile_definitions(opengl_ui_demo PRIVATE OPENGUI_MEDIA_OPENCV=1)
    target_link_libraries(opengl_ui_api_demo PRIVATE ui_media_opencv)
    target_compile_definitions(opengl_ui_api_demo PRIVATE OPENGUI_MEDIA_OPENCV=1)
    # M16：图像查看器示例应用
    add_executable(opengl_ui_image_viewer ${CMAKE_CURRENT_SOURCE_DIR}/image_viewer.cpp)
    target_link_libraries(opengl_ui_image_viewer PRIVATE ui_media_opencv)
    target_compile_definitions(opengl_ui_image_viewer PRIVATE OPENGUI_MEDIA_OPENCV=1)
endif()

# 拷贝依赖 DLL 便于 VS 调试/直接运行（glfw3.dll；media ON 时含 FFmpeg/OpenCV DLL）
add_executable(opengl_ui_edit_test ${CMAKE_CURRENT_SOURCE_DIR}/edit_test.cpp)
target_link_libraries(opengl_ui_edit_test PRIVATE ui_ui)
set(_APP_TARGETS opengl_ui_demo opengl_ui_api_demo opengl_ui_edit_test)
if(OPENGUI_MEDIA_FFMPEG)
    list(APPEND _APP_TARGETS opengl_ui_video_player)
endif()
if(OPENGUI_MEDIA_OPENCV)
    list(APPEND _APP_TARGETS opengl_ui_image_viewer)
endif()
if(OPENGUI_MEDIA_FFMPEG AND OPENGUI_MEDIA_OPENCV)
    list(APPEND _APP_TARGETS opengl_ui_media_load_test)
endif()
foreach(_TGT ${_APP_TARGETS})
    add_custom_command(TARGET ${_TGT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENGUI_3RDPARTY}/bin/glfw3.dll"
            $<TARGET_FILE_DIR:${_TGT}>
    )
    if(OPENGUI_MEDIA_FFMPEG)
        file(GLOB _FFMPEG_DLLS "${OPENGUI_3RDPARTY}/bin/*.dll")
        add_custom_command(TARGET ${_TGT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_FFMPEG_DLLS}
                $<TARGET_FILE_DIR:${_TGT}>)
    endif()
    if(OPENGUI_MEDIA_OPENCV)
        # OpenCV 安装树在架构子目录下（x86 安装也可能落在 x64/vc16），用通配匹配
        file(GLOB _OPENCV_DLLS "${OPENGUI_3RDPARTY}/*/vc16/bin/opencv_world*.dll")
        add_custom_command(TARGET ${_TGT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_OPENCV_DLLS}
                $<TARGET_FILE_DIR:${_TGT}>)
    endif()
endforeach()

# 拷贝演示素材（测试图片/视频/图标字体）到可执行目录
# 图标字体（M17d iconfont，Demo 始终加载）：任何配置都拷贝
file(GLOB _ICON_FONT "${CMAKE_SOURCE_DIR}/assets/iconfont.ttf")
foreach(_TGT opengl_ui_demo opengl_ui_api_demo)
    add_custom_command(TARGET ${_TGT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_ICON_FONT}
            $<TARGET_FILE_DIR:${_TGT}>)
endforeach()
if(OPENGUI_MEDIA_OPENCV)
    file(GLOB _TEST_IMG "${CMAKE_SOURCE_DIR}/assets/test.png")
    foreach(_TGT opengl_ui_demo opengl_ui_image_viewer opengl_ui_api_demo)
        add_custom_command(TARGET ${_TGT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_TEST_IMG}
                $<TARGET_FILE_DIR:${_TGT}>)
    endforeach()
endif()
if(OPENGUI_MEDIA_FFMPEG)
    file(GLOB _TEST_VID "${CMAKE_SOURCE_DIR}/assets/test.mp4")
    file(GLOB _TEST_AV "${CMAKE_SOURCE_DIR}/assets/test_av.mp4")
    file(GLOB _TEST_H264 "${CMAKE_SOURCE_DIR}/assets/test_h264.mp4")
    foreach(_TGT opengl_ui_demo opengl_ui_video_player opengl_ui_api_demo)
        add_custom_command(TARGET ${_TGT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_TEST_VID} ${_TEST_AV} ${_TEST_H264}
                $<TARGET_FILE_DIR:${_TGT}>)
    endforeach()
endif()
