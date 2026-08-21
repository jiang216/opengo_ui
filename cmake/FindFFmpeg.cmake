# 自定义 FindFFmpeg：定位自编译安装的 FFmpeg（3rdparty/install/{arch}）
# 用法：find_package(FFmpeg REQUIRED) → FFmpeg::avcodec/avformat/avutil/swscale/swresample

set(_FFMPEG_ROOT_HINT "${OPENGUI_3RDPARTY}")

find_path(FFmpeg_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    PATHS ${_FFMPEG_ROOT_HINT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH
)

set(_FFMPEG_LIBS avcodec avformat avutil swscale swresample)
foreach(_c ${_FFMPEG_LIBS})
    string(TOUPPER ${_c} _CU)
    # FFmpeg(--toolchain=msvc) 安装时 import lib 放在 bin/（与 DLL 同目录），老式安装放在 lib/
    find_library(FFmpeg_${_c}_LIBRARY
        NAMES ${_c}
        PATHS ${_FFMPEG_ROOT_HINT}
        PATH_SUFFIXES bin lib
        NO_DEFAULT_PATH
    )
    if(NOT FFmpeg_${_c}_LIBRARY)
        message(FATAL_ERROR "FFmpeg 库 ${_c}.lib 未找到（${_FFMPEG_ROOT_HINT}/bin|lib）")
    endif()
endforeach()

if(NOT FFmpeg_INCLUDE_DIR)
    message(FATAL_ERROR "FFmpeg 头文件未找到（${_FFMPEG_ROOT_HINT}/include）")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg REQUIRED_VARS FFmpeg_INCLUDE_DIR
    FFmpeg_avcodec_LIBRARY FFmpeg_avformat_LIBRARY FFmpeg_avutil_LIBRARY
    FFmpeg_swscale_LIBRARY FFmpeg_swresample_LIBRARY)

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avcodec)
    foreach(_c ${_FFMPEG_LIBS})
        string(TOUPPER ${_c} _CU)
        # 定位同名版本 DLL（avcodec-59.dll 等），import lib 作为 IMPORTED_IMPLIB
        file(GLOB FFmpeg_${_c}_DLL "${_FFMPEG_ROOT_HINT}/bin/${_c}-*.dll")
        if(NOT FFmpeg_${_c}_DLL)
            message(FATAL_ERROR "FFmpeg DLL ${_c}-*.dll 未找到（${_FFMPEG_ROOT_HINT}/bin）")
        endif()
        add_library(FFmpeg::${_c} SHARED IMPORTED)
        set_target_properties(FFmpeg::${_c} PROPERTIES
            IMPORTED_LOCATION "${FFmpeg_${_c}_DLL}"
            IMPORTED_IMPLIB "${FFmpeg_${_c}_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIR}"
        )
    endforeach()
endif()
