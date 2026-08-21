# OpenGL UI

C++ OpenGL UI 框架（CMake 多模块工程）。里程碑计划见 [PLAN.md](PLAN.md)，四层架构见 [ARCHITECTURE.md](ARCHITECTURE.md)，依赖构建见 [BUILD.md](BUILD.md)。

## 必须保证的功能（回归清单）

| 功能 | 来源 |
|------|------|
| 文档图谱与 CHANGELOG 条目可被 `python tools\graphgen.py` 正常生成与校验 | UI-001 |
| `scripts/build/*.ps1` 双架构 x64/x86 一键构建全部 3rdparty 依赖并安装到 `3rdparty/install/{x64,x86}` | UI-002 |
| 顶层 CMake：C++17、media 开关默认 OFF、按架构推导 `OPENGUI_3RDPARTY` 前缀 | UI-003 |
| Demo（apps/opengl_ui_demo）开窗渲染可运行，media ON/OFF 双配置均可编译 | UI-003 |

## 示例应用（M16 收官）

两个可运行应用随对应 media 开关构建，POST_BUILD 自动拷贝依赖 DLL 与素材，开箱可运行：

- **image_viewer（`OPENGUI_MEDIA_OPENCV=ON`）**：图像查看器。按钮 / Ctrl+O 打开图片（Win32
  宽字符文件对话框，支持中文路径），滚轮缩放（以鼠标为锚点），左键拖拽平移，双击复位，
  状态栏显示文件名 / 尺寸 / 缩放。用法：`opengl_ui_image_viewer [图片路径] [--smoke <秒>]`。
- **video_player（`OPENGUI_MEDIA_FFMPEG=ON`）**：视频播放器。按钮 / Ctrl+O 打开视频，播放/暂停
  （按钮或空格键），进度条 seek，循环开关，倍速 0.5x..2x，音量 0..1。用法：
  `opengl_ui_video_player [视频路径] [--smoke <秒>]`。

构建与冒烟示例（x64，详见 [BUILD.md](BUILD.md)）：

```text
cmake -S . -B build-media -DCMAKE_BUILD_TYPE=Release -DOPENGUI_MEDIA_FFMPEG=ON -DOPENGUI_MEDIA_OPENCV=ON
cmake --build build-media --target opengl_ui_image_viewer opengl_ui_video_player
build-media/apps/opengl_ui_image_viewer.exe --smoke 2
build-media/apps/opengl_ui_video_player.exe --smoke 3
```
