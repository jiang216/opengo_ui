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

```cpp
#include "ui/all.h"

int main() {
    ui::Window window(1280, 900, "My App");
    if (!window.handle()) return 1;

    ui::Renderer renderer;
    if (!renderer.init()) return 1;
    renderer.set_viewport(window.width(), window.height());

    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    font.load_cjk_font_auto();
    font.load_icon_font("iconfont.ttf", 18.f);

    ui::Theme theme = ui::Theme::make_layui();

    ui::Widget root;
    root.set_bounds(0, 0, (float)window.width(), (float)window.height());

    auto* btn = new ui::Button(0, 0, 120, 40, "Click Me");
    btn->set_font(&font);
    btn->set_theme(&theme);
    btn->add_class("layui-btn layui-btn-primary");
    btn->set_on_click([] { ui::log_info("Button clicked!"); });
    root.add_child(btn);

    auto* label = new ui::Label(0, 0, 400, 30, "Hello, opengl_ui!");
    label->set_font(&font);
    label->set_theme(&theme);
    root.add_child(label);

    while (!window.should_close()) {
        window.poll_events();
        if (window.resized()) {
            renderer.set_viewport(window.width(), window.height());
            root.set_bounds(0, 0, (float)window.width(), (float)window.height());
        }
        root.layout();
        root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());
        if (window.mouse_pressed(0))
            root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);
        if (window.mouse_released(0))
            root.dispatch_mouse_up(window.mouse_x(), window.mouse_y(), 0);
        if (window.key_pressed(ui::KeyTab))
            root.dispatch_key_down(ui::KeyTab, 0);

        renderer.begin_frame(theme.window_bg);
        root.draw(renderer);
        window.swap_buffers();
    }
    return 0;
}
```

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
├── bin/                    # Runtime DLLs (glfw3.dll, ffmpeg, opencv)
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
└── share/opengl_ui/       # Runtime assets (iconfont.ttf)
```

## License

See source repository for license details.
