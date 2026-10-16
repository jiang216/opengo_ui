# opengl_ui - C++17 OpenGL UI Framework

## Overview

opengl_ui is a C++17 static library providing 50+ UI widgets styled after the layui v2.13.8 web component library, rendered via OpenGL 3.3 Core. It is a four-layer architecture with unidirectional dependencies:

```
apps  →  media (optional)  →  ui  →  gl  →  core
```

| Layer | Library | Description | Dependencies |
|-------|---------|-------------|-------------|
| core | `ui_core` | Pure C++: math, logging, keycodes, version | None |
| gl | `ui_gl` | GLFW window, 2D renderer, texture, font | core + GLFW + GLAD + stb + OpenGL |
| ui | `ui_ui` | Widget tree, layout, theming, 50+ components | gl + core |
| media/ffmpeg | `ui_media_ffmpeg` | Video/audio decode (optional) | ui + FFmpeg |
| media/opencv | `ui_media_opencv` | Image processing (optional) | ui + OpenCV |

## Requirements

- **Compiler**: C++17 (MSVC 14.x+, GCC 9+, Clang 10+)
- **Graphics**: OpenGL 3.3 Core capable GPU
- **Platform**: Windows (primary), Linux/macOS (future)
- **Build**: CMake 3.24+

## Installation

### Option 1: Copy install directory (recommended)

Copy the entire `install/` directory to your project. Then in your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.24)
project(my_app LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/third_party/install")
find_package(opengl_ui REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE opengl_ui::ui_ui)
```

### Option 2: add_subdirectory (source integration)

```cmake
add_subdirectory(path/to/opengl_ui)
target_link_libraries(my_app PRIVATE ui_ui)
```

## Minimal Example

The recommended entry point is `ui::WindowContainer`: it owns the window, renderer, self-drawn title bar (chromed), and `ui::Scene` (which drives the full lifecycle: layout + input dispatch + advance/`flush_pending_destroys` + draw). Using it avoids the lifecycle pitfalls listed after the example.

```cpp
#include "ui/all.h"

int main() {
    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    font.load_cjk_font_auto();
    font.load_icon_font("iconfont.ttf", 18.f);

    ui::Theme theme = ui::Theme::make_layui();

    // 一个对象收敛「窗口 + 渲染器 + 自绘标题栏(chrome) + Scene」；创建即居中主屏。
    ui::WindowContainer wc(1280, 800, "My App");
    if (!wc.ok()) return 1;
    wc.titlebar().set_font(&font);
    wc.titlebar().set_theme(&theme);
    wc.titlebar().set_icon(&font, ui::layui_icon::app);
    wc.scene().set_root_background(theme.window_bg);

    // 内容控件以 content_origin_y()（=标题栏高）为单一原点派生，不硬编码偏移。
    const float y = wc.content_origin_y();
    auto* btn = new ui::Button(0.f, y, 120.f, 40.f, "Click Me");
    btn->set_font(&font);
    btn->set_theme(&theme);
    btn->add_class("layui-btn layui-btn-primary");
    btn->set_on_click([] { ui::log_info("Button clicked!"); });
    wc.root().add_child(btn);

    auto* label = new ui::Label(160.f, y, 400.f, 30.f, "Hello, opengl_ui!");
    label->set_font(&font);
    label->set_theme(&theme);
    wc.root().add_child(label);

    // Scene::run() 内部完成 poll → frame（layout/输入/advance/绘制）→ swap 全流程，
    // 并自动 flush destroy() 的控件、处理弹窗自动关闭与 CJK 字体续帧烘焙。
    wc.run();
    return 0;
}
```

> **为什么不推荐手写 while 循环**：库的真实应用与 `WindowContainer` 都走 `Scene::run()/run_for()`。
> 若确实需要裸 `Window + Renderer + root` 自绘管线，必须自行履行以下契约，违反会在运行期静默复现历史 Bug：
> - 每帧调 `renderer.begin_frame()`（UI-169：内部清空/关闭 scissor，是跨帧裁剪状态干净的唯一保证）；
> - 帧末调 `root.advance(dt)` 或 `ui::Widget::flush_pending_destroys()`，否则 `destroy()` 的控件**永不释放（内存泄漏）**，且 `root.draw()` 的入口兜底 layout 需要 advance 驱动（UI-157/161/167）；
> - 自行处理 `dispatch_*`（鼠标/滚轮/字符/键盘）与滚动、输入法、弹窗自动关闭、CJK 续帧烘焙；
> - 注册 chrome（`window.container` 标题条）时确保 `content_origin_y` 对齐，且尺寸变化后重排；
> - 退出路径析构/收尾 `Scene`，否则 Cleanup/Shutdown 广播与销毁队列冲刷丢失（UI-167）。

## Widget Quick Reference

### Layout
- `VBox` - Vertical box layout
- `HBox` - Horizontal box layout
- `Grid` - 12-column responsive grid (layui grid)
- `ScrollBox` - Scrollable container
- `FlowBox` - Flow/wrap layout
- `Panel` - Basic container

### Basic Controls
- `Label` - Text display (multi-line, wrap)
- `Button` - Clickable button (layui variants: primary/danger/warning/etc.)
- `ButtonGroup` - Grouped buttons
- `Icon` - layui icon (190 icons)
- `Image` - Image display with lazy loading
- `Badge` - Badge/dot indicator
- `Progress` - Progress bar
- `Circle` - Circular progress
- `Switch` - Toggle switch
- `HLine` - Horizontal separator
- `Fieldset` / `FieldsetTitle` - Fieldset container
- `BlockQuote` - Quote block
- `Card` - Card container

### Form Controls
- `TextBox` - Single-line text input
- `TextArea` - Multi-line text input
- `CheckBox` - Checkbox
- `RadioButton` / `RadioGroup` - Radio buttons
- `ComboBox` - Dropdown select
- `Slider` - Range slider (single/range/vertical)
- `Rate` - Star rating
- `ColorPicker` - Color picker
- `Upload` - File upload
- `Laydate` / `LaydateBox` - Date/time picker

### Navigation
- `Tabs` - Tab panel (default/card/brief)
- `MenuBar` - Horizontal menu bar
- `NavBar` - Navigation bar
- `NavMenu` - Sidebar navigation
- `Menu` - Vertical menu
- `Dropdown` - Dropdown menu
- `Tree` - Tree view
- `Breadcrumb` - Breadcrumb navigation
- `Collapse` - Accordion/collapsible
- `Laypage` - Pagination

### Data Display
- `Table` - Data table (sortable, paginated, editable cells, checkbox/radio columns, summary row, action buttons)
- `Timeline` - Timeline
- `Transfer` - Shuttle/transfer
- `Carousel` - Image/content carousel
- `Code` - Syntax-highlighted code block
- `CountdownText` - Real-time countdown

### Containers
- `PanelWindow` - Draggable/resizable floating window (layer system, modals, shade, footer buttons)

### Media (optional, requires build flags)
- `VideoView` - Video player (FFmpeg decode, play/pause/seek/speed/volume)
- `ImageView` - Image viewer (OpenCV decode)
- `AudioOut` - Audio output (winmm)

## CSS-like Styling

Apply layui classes via `add_class()`:

```cpp
// Button variants
btn->add_class("layui-btn");              // default
btn->add_class("layui-btn layui-btn-primary");  // primary
btn->add_class("layui-btn layui-btn-danger");   // danger
btn->add_class("layui-btn layui-btn-lg");       // large
btn->add_class("layui-btn layui-btn-sm");       // small

// Badge
badge->add_class("layui-badge layui-bg-orange");

// Grid
col->add_class("layui-col-md4");          // 4/12 columns
col->add_class("layui-col-md-offset2");   // offset 2

// Progress
progress->add_class("layui-progress layui-progress-big");
```

## Theming

Three built-in themes:

```cpp
ui::Theme dark   = ui::Theme::make_dark();
ui::Theme light  = ui::Theme::make_light();
ui::Theme layui  = ui::Theme::make_layui();  // recommended

// Apply to all widgets via pointer sharing
btn->set_theme(&theme);
label->set_theme(&theme);
```

Runtime theme switching: change theme object contents, all widgets redraw instantly.

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `OPENGUI_MEDIA_FFMPEG` | OFF | Build FFmpeg media module |
| `OPENGUI_MEDIA_OPENCV` | OFF | Build OpenCV media module |
| `OPENGUI_BUILD_APPS` | ON | Build demo applications |

## Runtime Dependencies

| File | When | Description |
|------|------|-------------|
| `glfw3.dll` | Always | GLFW window library |
| `iconfont.ttf` | Demo apps | layui icon font |
| `avcodec-59.dll` etc. | FFmpeg ON | Video/audio codecs |
| `opencv_world452.dll` | OpenCV ON | Image processing |

All runtime DLLs are bundled in the `install/bin/` directory.

## File Structure

```
install/
├── bin/                    # DLLs (glfw3.dll, ffmpeg, opencv) + demo exes + runtime assets
│   ├── opengl_ui_demo.exe       # demo apps (built with OPENGUI_BUILD_APPS=ON)
│   ├── opengl_ui_image_viewer.exe / opengl_ui_video_player.exe ...
│   └── iconfont.ttf             # runtime assets (iconfont + media test assets)
├── include/
│   ├── ui/                 # opengl_ui headers
│   │   ├── all.h          # Umbrella header (includes everything)
│   │   ├── core/          # Math, logging, keycodes
│   │   ├── gl/            # Window, renderer, texture, font
│   │   ├── ui/            # 50+ widget headers
│   │   └── media/         # VideoView, ImageView, AudioOut
│   ├── GLFW/              # GLFW headers
│   ├── glad/ KHR/         # OpenGL loader headers
│   └── stb_image.h stb_truetype.h  # Image/font decoding
├── lib/
│   ├── *.lib              # Static libraries + import libs
│   └── cmake/opengl_ui/  # CMake config files
└── share/
    └── opengl_ui/         # iconfont.ttf + opengui MCP knowledge service (mcp/)
```

## License

See source repository for license details.
