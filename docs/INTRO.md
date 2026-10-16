# opengl_ui —— C++17 OpenGL 即时渲染 UI 框架

> 面向桌面产品的 C++17 即时渲染 UI 库：**一条主循环、一棵控件树、59 个 layui 风格控件**，
> 自带窗口 / 场景 / 主题 / 中文与图标字体，媒体能力（视频、音频、图片）可选启用。
> 许可 MIT，Windows 平台，vcpkg / conan / CMake config 三种方式接入。

## 一、总览

| 项目 | 说明 |
|------|------|
| 语言标准 | C++17（C11，MSVC `/utf-8`） |
| 当前版本 | 0.1.0（`project(opengl_ui VERSION 0.1.0)`） |
| 平台 | Windows（`vcpkg.json` `supports: windows`） |
| 渲染 | OpenGL 3.3 Core（GLFW 3.4 + GLAD + stb） |
| 控件数量 | **59 个**，覆盖 9 个类别；公开头文件 67 个 |
| 分层 | `core → gl → ui → media`（单向依赖，禁止反向 include） |
| 第三方 | GLFW / GLAD / stb 必需；FFmpeg 5.1、OpenCV 4.5 可选 |
| 依赖获取 | **全部本地构建，不依赖包管理器联网**（`scripts/build/*.ps1`） |
| 接入方式 | `find_package(opengl_ui CONFIG)`、vcpkg manifest、conan |
| 许可 | MIT（见 `vcpkg.json` / `conanfile.py`） |

## 二、核心特性

- **即时渲染的控件树**：每帧 `layout → dispatch 事件 → draw` 三步推进，无需在控件间传回调链；
  `#include "ui/all.h"` 一条头拉入 core / gl / ui 全部公开 API。
- **主题即时切换**：`ui::Theme::make_layui()` / `make_dark()` / `make_light()`，
  控件持有主题指针，换指针即换皮，无需重建控件树。
- **中文与图标开箱可用**：`Font::load_cjk_font_auto()` 懒烘培中文字形，
  `load_icon_font("iconfont.ttf")` 接入 layui 图标字体。
- **完整布局与数据组件**：`Grid` 12 列栅格、`VBox`/`HBox`、`ScrollBox` 滚动容器；
  `Table`（列排序 / 双击编辑 / 分页 / 流式加载 / 操作列）、`Transfer`、`Tree`、`Code` 语法高亮、`LayTpl` 模板引擎。
- **窗口与弹层能力**：`WindowFrame` 自绘标题栏、`PanelWindow` layer 弹层（拖拽 / 缩放 / 置顶 / 遮罩）、
  多窗口 `Scene` 生命周期（Ready → activated → StandBy）。
- **媒体层可选**：`OPENGUI_MEDIA_FFMPEG` / `OPENGUI_MEDIA_OPENCV` 默认 **OFF**，
  纯 UI 构建不引入任何媒体库；开启后提供 `VideoView` / `VideoContainer` / `ImageView` / `AudioOut`。
- **自包含分发**：`cmake --install` 产出 `install/`（`bin/ include/ lib/ share/`，含 GLFW 运行时与
  CMake config），`scripts/publish/package.ps1` 一步打成 `dist/opengl_ui-<arch>-all-release-<yyyyMMdd>.zip`。
- **智能体知识服务（opengui）**：随包发布零依赖 MCP 服务，编辑器 / CLI 智能体可直接查证
  「组件 / 模板 / 回归约束」，**禁止凭记忆猜 API**。
- **可追溯的工程纪律**：每处修复一条 `UI-###` CHANGELOG 条目，配文档图谱（`docs/GRAPH.md`）
  与自动生成的 API 手册，行为可回溯、功能有回归清单。

## 三、分层架构

依赖方向单向：`media → ui → gl → core`，上层只被依赖、不反向 include。

| 层 | 头文件根 | 职责 | 依赖 |
|----|----------|------|------|
| core | `ui/core/*` | 纯 C++：数学 / 日志 / 键码 / 渲染原语 / 纹理 / 字体图集 | 无 |
| gl | `ui/gl/*` | GL 3.3 渲染器 `Renderer`：`init` 一次 → 每帧 `begin_frame` → 画 quad / 圆角矩形 / 文本 | core + 3rdparty（GLFW/GLAD/stb） |
| ui | `ui/ui/*` | 控件库、布局、主题、窗口框架 | gl + core |
| media | `ui/media/*` | 管线 `MediaPipeline` + 合成源（无需第三方）；FFmpeg / OpenCV 节点可选 | ui + gl + core |

## 四、控件一览（59 个）

| 类别 | 数量 | 控件 |
|------|------|------|
| 框架入口 | 4 | Window · Scene · Widget · Theme |
| 基础层（core） | 3 | Renderer · Texture · Font |
| 布局容器 | 4 | Grid · HBox · ScrollBox · VBox |
| 基础控件 | 12 | Badge · BlockQuote · Button · Circle · Fieldset · FieldsetTitle · HLine · Icon · Image · Label · Progress · Switch |
| 表单 | 11 | CheckBox · ColorPicker · ComboBox · Laydate · LaydateBox · RadioButton · RadioGroup · Rate · Slider · TextBox · Upload |
| 导航 | 8 | Breadcrumb · Collapse · Dropdown · Menu · MenuBar · NavMenu · Tabs · Tree |
| 数据展示 | 8 | Carousel · Code · CountdownText · Laypage · LayTpl · Table · Timeline · Transfer |
| 容器 / 弹层 | 5 | Card · Panel · PanelWindow · TitleBar · WindowFrame |
| 媒体（可选构建） | 4 | AudioOut · ImageView · VideoContainer · VideoView |

逐个 API 签名与调用示例见 [UI_API.md](UI_API.md)（由 `tools/api_gen.py` 从公开头文件自动生成）。

## 五、快速开始

### 1. 构建

第三方依赖已在 `3rdparty/install/{x64,x86}` 就位时，两条命令即可出库：

```powershell
cmake --preset x64                 # 生成到 build/x64（generator = Visual Studio 18 2026）
cmake --build build/x64 --config Release
```

需要媒体能力时改用 `x64-all`（FFmpeg + OpenCV 同开），并**必须 Release**：

```powershell
# 依赖未就绪先本地构建（无需联网包管理器）
pwsh scripts/build/build-ffmpeg.ps1 -Arch x64
pwsh scripts/build/build-opencv.ps1 -Arch x64

cmake --preset x64-all
cmake --build build/x64-all --config Release
```

一次性产出自包含分发目录与发布包：

```powershell
cmake --preset install-all                     # install 前缀 = 仓库 install/
cmake --build build/install-all --config Release
cmake --install build/install-all --config Release
scripts\publish\package.ps1 -Rebuild           # -> dist/opengl_ui-x64-all-release-<yyyyMMdd>.zip
```

### 2. 最小应用骨架

```cpp
#include "ui/all.h"

ui::Window window(1280, 900, "demo");   // ① 窗口（GL 3.3 Core 上下文）
if (!window.handle()) return 1;
ui::Renderer renderer;                  // ② 渲染器（上下文就绪后 init）
if (!renderer.init()) return 1;
renderer.set_viewport(window.width(), window.height());

ui::Font font;                          // ③ 字体：ASCII + 中文懒烘培 + 图标
font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
font.load_cjk_font_auto();
font.load_icon_font("iconfont.ttf", 18.f);

ui::Theme theme = ui::Theme::make_layui();  // ④ 主题：make_layui / dark / light
ui::Widget root;                        // ⑤ 控件树根
root.set_bounds(0, 0, (float)window.width(), (float)window.height());
auto* btn = new ui::Button(0, 0, 120, 40, "OK");
btn->set_font(&font);
btn->set_theme(&theme);
root.add_child(btn);

while (!window.should_close()) {        // ⑥ 主循环：事件 → 布局 → 绘制
    window.poll_events();
    root.layout();
    root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());
    if (window.mouse_pressed(0)) root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);
    renderer.begin_frame(theme.window_bg);
    root.draw(renderer);
    window.swap_buffers();
}
```

### 3. 作为依赖接入你的工程

```cmake
find_package(opengl_ui CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE opengl_ui::ui_ui)   # 需要媒体再链 opengl_ui::ui_media_ffmpeg / ui_media_opencv
```

- `CMAKE_PREFIX_PATH` 指向发布包解压后的 `install/` 目录（其中已含 GLFW 头文件、import lib 与 cmake config）。
- 也提供 `vcpkg.json`（含 `ffmpeg` / `opencv` 可选 feature）与 `conanfile.py`
  （`-o with_ffmpeg` / `-o with_opencv`），库名 `opengl-ui`、CMake 包名 `opengl_ui`。

## 六、配套交付物

- **示例应用**（随 `install/bin/` 发布，可直接运行）：
  `opengl_ui_demo`（综合演示）、`opengl_ui_api_demo`（控件遍历）、`opengl_ui_edit_test`（输入 / 表单）、
  `opengl_ui_scene_multiwin`（多窗口场景）、`opengl_ui_image_viewer`（图片查看，OpenCV）、
  `opengl_ui_video_player`（视频播放，FFmpeg）。均支持 `--smoke <秒>` 冒烟自测。
- **MCP 知识服务 opengui**：解压发布包后
  `share\opengl_ui\register-mcp.ps1 -Mode register` 一键注入 opencode / Claude Code / `.mcp.json`
  （`-Mode status` 查看、`-Mode unregister` 卸载）；服务为纯 Python 标准库实现，无需 pip / venv / 外网。
- **打包脚本**：`scripts/publish/package.ps1`，默认只打包不改用户配置；`-Version` 可指定版本日期。

## 七、文档导航

| 文档 | 内容 |
|------|------|
| [README.md](README.md) | 回归清单（必须保证的功能）与 MCP 使用指引 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | 四层架构与模块边界 |
| [BUILD.md](BUILD.md) | 依赖管线、预设、打包与回归约束 |
| [UI_API.md](UI_API.md) | 自动生成的控件 API 手册（67 头文件） |
| [MEDIA_PIPELINE.md](MEDIA_PIPELINE.md) | 媒体管线与音视频数据流 |
| [CHANGELOG.md](CHANGELOG.md) | 逐条修复 / 功能记录（`UI-###`） |
| [distribution/README.md](distribution/README.md) | 发布包文件结构 |

## 八、版本与许可

- 版本：**0.1.0**
- 许可：**MIT**（声明见 `vcpkg.json` / `conanfile.py`）
- 仓库：`https://github.com/user/opengl_ui`
