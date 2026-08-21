# 构建与依赖管线

第三方依赖（GLFW/GLAD/stb/FFmpeg/OpenCV）全部本地构建，不依赖包管理器联网。
产物安装在 `3rdparty/install/{x64,x86}`，框架 CMake 按当前架构自动选择前缀。

## 目录

```
scripts/build/env.ps1          公共环境（CMAKE/VS/VCVARSALL/MSYS2 路径）
scripts/build/build-ffmpeg.ps1 FFmpeg 5.1.9 MSVC shared（configure+make+install）
scripts/build/build-opencv.ps1 OpenCV 4.5.2 world（CMake 配置+MSBuild+install）
```

## 必须保证的功能（回归清单）

| 功能 | 来源 |
|------|------|
| `build-ffmpeg.ps1 -Arch x64\|x86`：MSVC 环境经 vcvarsall+ProcessStartInfo 注入；`--toolchain=msvc --enable-shared --disable-programs --disable-doc`；产物 DLL+import lib 在 `install/{arch}/bin`，头文件在 `install/{arch}/include` | UI-002 |
| FFmpeg 环境注入依赖 `MSYS2_PATH_TYPE=inherit`（MSYS2 profile 默认 minimal 会丢弃继承的 Windows PATH，导致 cl 不可见） | UI-002 |
| `build-opencv.ps1 -Arch x64\|x86`：`BUILD_LIST=world,core,imgproc,highgui,imgcodecs`；`OPENCV_INSTALL_BINARIES_PREFIX=x64/vc16/`（**尾斜杠必需**，否则拼接出 `x64/vc16bin`） | UI-002 |
| OpenCV 安装为嵌套布局 `install/{arch}/x64/vc16/{bin,lib}`；CMake 直接 `find_package(OpenCV CONFIG PATHS .../x64/vc16/lib)` 绕开对 MSVC 194x 失效的 WinPack 版本检测 | UI-002 |
| GLFW 经 CMake 安装，`find_package(glfw3 CONFIG PATHS install/{arch}/lib/cmake/glfw3)`，目标名 `glfw` | UI-002 |
| `cmake/FindFFmpeg.cmake` 同时搜 `bin`/`lib` 两处 import lib；SHARED IMPORTED 目标需同时设 `IMPORTED_LOCATION`（DLL）与 `IMPORTED_IMPLIB`（.lib） | UI-002 |
| 顶层 CMake 对 MSVC 全局加 `/utf-8`（源码含 UTF-8 中文注释，否则 C2447/C4819） | UI-003 |
| MSVC 下 GLAD `gl.c` 以 `LANGUAGE CXX` 编译（-TP），避免 C 代码告警 | UI-003 |
| media 开启：`-DOPENGUI_MEDIA_FFMPEG=ON -DOPENGUI_MEDIA_OPENCV=ON`，且 **必须 `-DCMAKE_BUILD_TYPE=Release`**（OpenCV 仅发布库，Debug 下 `debug_build_guard` 命名空间错位 → LNK2019） | UI-010 |
| media ON 时 apps POST_BUILD 自动拷贝 glfw3.dll、FFmpeg `bin/*.dll`、OpenCV `*/vc16/bin/opencv_world*.dll` 及 `assets/test.{png,mp4}` 到可执行目录 | UI-010 |
