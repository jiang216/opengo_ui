# vcpkg overlay port: opengl-ui
# 使用方法：将本项目目录作为 vcpkg overlay port
#   vcpkg install opengl-ui:x64-windows
#   或在 CMakePresets.json 中设置 VCPKG_OVERLAY_PORTS 指向本项目根目录

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO user/opengl_ui   # TODO: 替换为实际仓库地址
    HEAD_REF main
    # 如果是本地开发，改用：
    # vcpkg_from_local_path
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        ffmpeg  OPENGUI_MEDIA_FFMPEG
        opencv  OPENGUI_MEDIA_OPENCV
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DOPENGUI_BUILD_APPS=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
)

vcpkg_cmake_install()

# 移除示例程序和测试，只保留库
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin/opengl_ui_*")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/bin/opengl_ui_*")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/tools")

vcpkg_cmake_config_fixup(
    CONFIG_PATH lib/cmake/opengl_ui
    PACKAGE_NAME opengl_ui
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
