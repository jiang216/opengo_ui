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
| 布局相关 constexpr 公开头（如 gl/font.h 字形数组尺寸）变更后，凡经历过编译失败/中断的构建树必须全量重建再验证；增量混合产物会触发 ODR 违例型堆崩溃（症状：仅个别页 0xC0000374/0xC0000005 且崩溃码漂移） | [UI-119](CHANGELOG.md#ui-119-表单选择框分页页崩溃排查陈旧增量构建产物-odr-混合布局--崩溃诊断钩子2026-08-22) |
| layui_test 崩溃时自动打印符号化调用栈（SetUnhandledExceptionFilter + dbghelp）；定位崩溃帧需 Release+/Z7 生成 PDB | [UI-119](CHANGELOG.md#ui-119-表单选择框分页页崩溃排查陈旧增量构建产物-odr-混合布局--崩溃诊断钩子2026-08-22) |
| MCP 知识服务随 `cmake --install` 发布：`tools/mcp/server.py` + `docs/mcp/*.json` + `docs/CHANGELOG.md` + `docs/ARCHITECTURE.md` → `share/opengl_ui/mcp/`，`scripts/publish/{register-mcp,OPENGUI_AGENT,publish}.ps1/md` → `share/opengl_ui/`；`docs/mcp/*.json` 由 `python tools/api_gen.py` 同源生成，`--check` 四产物全量校验 | [UI-208](CHANGELOG.md#ui-208-opengui-mcp-知识服务零依赖-stdlib-服务--apigen-同源目录--发布期一键注入-idecli-智能体toolsmcpserverpy--toolsapigenpy--toolsmcpdatapy--scriptspublishregister-mcpps12026-09-21) |

## MCP 知识服务资产生成（opengui）

生成与校验（不依赖第三方 Python 包；Windows 下先 `$env:PYTHONIOENCODING='utf-8'` 防中文乱码）：

```powershell
python tools/api_gen.py            # 生成 docs/UI_API.md + docs/mcp/{catalog,templates,constraints}.json
python tools/api_gen.py --check    # 四产物过期即 exit 1
python tools/mcp/server.py --selfcheck --data docs/mcp   # MCP 服务自检（发布注入前必经）
```

发布打包：根 `CMakeLists.txt` 的 install 规则把上述资产连同注入脚本打进 `install/`，
消费方见 `docs/README.md` 的「智能体 MCP 知识服务」小节。
