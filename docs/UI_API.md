# UI 库 API 手册

> 本文件为图谱节点 `DOC:UI_API`，AUTO-GENERATED（由 `python tools/api_gen.py` 生成，禁手改）。
> 功能注释 / 方法签名抽取自各公开头文件；调用示例 / 依赖说明在 `tools/api_gen.py` 的
> `EXAMPLES` / `NOTES` 字典中维护。新增 API 或改注释后：重新生成本文件并同步 CHANGELOG。
> 生成器结构化解析缺陷修复记录见 [UI-042](CHANGELOG.md#ui-042-api-文档生成器toolsapigenpy结构化解析修复2026-08-16)。

## 一、用法总览

一条 include 拉入 core / gl / ui/ui 全部公开头：

```cpp
#include "ui/all.h"
```

媒体模块（ui_media，可选、默认不编译）不在聚合头内，启用后按需 include：

```cpp
#include "ui/media/video_view.h"
#include "ui/media/audio_out.h"
#include "ui/media/image_view.h"
```

### 全局入口与主循环

最小应用骨架（完整参考 `layui_test/src/app.cpp`）：

```cpp
#include "ui/all.h"

ui::Window window(1280, 900, "demo");       // ① 窗口（提供 GL 3.3 Core 上下文）
if (!window.handle()) return 1;
ui::Renderer renderer;                       // ② 渲染器（须在 GL 上下文就绪后 init）
if (!renderer.init()) return 1;
renderer.set_viewport(window.width(), window.height());
ui::Font font;                               // ③ 字体（ASCII + 中文懒烘培 + 图标）
font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
font.load_cjk_font_auto();
font.load_icon_font("iconfont.ttf", 18.f);
ui::Theme theme = ui::Theme::make_layui();   // ④ 主题（make_dark/light/layui）
ui::Widget root;                             // ⑤ 控件树根节点
root.set_bounds(0, 0, (float)window.width(), (float)window.height());
ui::Button* btn = new ui::Button(0, 0, 120, 40, "OK");
btn->set_font(&font);
btn->set_theme(&theme);
root.add_child(btn);

while (!window.should_close()) {             // ⑥ 主循环：事件 → 布局 → 绘制
    window.poll_events();
    if (window.resized()) { renderer.set_viewport(window.width(), window.height());
        root.set_bounds(0, 0, (float)window.width(), (float)window.height()); }
    root.layout();
    root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());
    if (window.mouse_pressed(0)) root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);
    if (window.mouse_released(0)) root.dispatch_mouse_up(window.mouse_x(), window.mouse_y(), 0);
    if (window.key_pressed(ui::KeyTab)) root.dispatch_key_down(ui::KeyTab, 0);
    renderer.begin_frame(theme.window_bg);
    root.draw(renderer);
    window.swap_buffers();
}
```

## 二、分层依赖

依赖方向：`media → ui → gl → core`（单向，禁止反向 include）。

| 层 | 头文件根 | 依赖 |
| --- | --- | --- |
| core | `ui/core/*` | 无（纯 C++） |
| gl | `ui/gl/*` | core + 3rdparty（GLFW/GLAD/stb） |
| ui | `ui/ui/*` | gl + core |
| media（可选） | `ui/media/*` | ui + gl + core + FFmpeg/OpenCV |

## 3、ui_core（纯 C++ 基础）（`ui/core`）

### `ui/core/keycodes.h`

**枚举 `Key`**

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `KeySpace` | `32` |  |
| `KeyApostrophe` | `39` |  |
| `KeyComma` | `44` |  |
| `KeyMinus` | `45` |  |
| `KeyPeriod` | `46` |  |
| `KeySlash` | `47` |  |
| `Key0` | `48` |  |
| `Key1` | `49` |  |
| `Key2` | `50` |  |
| `Key3` | `51` |  |
| `Key4` | `52` |  |
| `Key5` | `53` |  |
| `Key6` | `54` |  |
| `Key7` | `55` |  |
| `Key8` | `56` |  |
| `Key9` | `57` |  |
| `KeySemicolon` | `59` |  |
| `KeyEqual` | `61` |  |
| `KeyA` | `65` |  |
| `KeyB` | `66` |  |
| `KeyC` | `67` |  |
| `KeyD` | `68` |  |
| `KeyE` | `69` |  |
| `KeyF` | `70` |  |
| `KeyG` | `71` |  |
| `KeyH` | `72` |  |
| `KeyI` | `73` |  |
| `KeyJ` | `74` |  |
| `KeyK` | `75` |  |
| `KeyL` | `76` |  |
| `KeyM` | `77` |  |
| `KeyN` | `78` |  |
| `KeyO` | `79` |  |
| `KeyP` | `80` |  |
| `KeyQ` | `81` |  |
| `KeyR` | `82` |  |
| `KeyS` | `83` |  |
| `KeyT` | `84` |  |
| `KeyU` | `85` |  |
| `KeyV` | `86` |  |
| `KeyW` | `87` |  |
| `KeyX` | `88` |  |
| `KeyY` | `89` |  |
| `KeyZ` | `90` |  |
| `KeyEscape` | `256` |  |
| `KeyEnter` | `257` |  |
| `KeyTab` | `258` |  |
| `KeyBackspace` | `259` |  |
| `KeyInsert` | `260` |  |
| `KeyDelete` | `261` |  |
| `KeyRight` | `262` |  |
| `KeyLeft` | `263` |  |
| `KeyDown` | `264` |  |
| `KeyUp` | `265` |  |
| `KeyPageUp` | `266` |  |
| `KeyPageDown` | `267` |  |
| `KeyHome` | `268` |  |
| `KeyEnd` | `269` |  |
| `KeyLeftShift` | `340` |  |
| `KeyLeftControl` | `341` |  |
| `KeyLeftAlt` | `342` |  |
| `KeyLeftSuper` | `343` |  |
| `KeyRightShift` | `344` |  |
| `KeyRightControl` | `345` |  |
| `KeyRightAlt` | `346` |  |
| `KeyRightSuper` | `347` |  |
| `KeyF1` | `290` |  |
| `KeyF2` | `291` |  |
| `KeyF3` | `292` |  |
| `KeyF4` | `293` |  |
| `KeyF5` | `294` |  |
| `KeyF6` | `295` |  |
| `KeyF7` | `296` |  |
| `KeyF8` | `297` |  |
| `KeyF9` | `298` |  |
| `KeyF10` | `299` |  |
| `KeyF11` | `300` |  |
| `KeyF12` | `301` |  |

**枚举 `KeyMod`**

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `ModShift` | `0x0001` |  |
| `ModControl` | `0x0002` |  |
| `ModAlt` | `0x0004` |  |
| `ModSuper` | `0x0008` |  |


### `ui/core/log.h`

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void log_info(const char* fmt, ...)` | 简洁日志：stderr 输出，带 [ui] 前缀。TODO(M1): 升级为可带 level/时间戳的 Logger |
| `void log_error(const char* fmt, ...)` |  |


### `ui/core/math.h`

**类 `Vec2`**

| 数据成员 | 说明 |
| --- | --- |
| `float x = 0, y = 0;` |  |

**类 `Vec4`**

| 数据成员 | 说明 |
| --- | --- |
| `float r = 0, g = 0, b = 0, a = 1;` |  |

**类 `Rect`**

| 数据成员 | 说明 |
| --- | --- |
| `float x = 0, y = 0, w = 0, h = 0;` |  |


### `ui/core/version.h`

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `inline const char* version_string()` |  |


## 4、ui_gl（渲染 / 窗口 / 字体）（`ui/gl`）

### `ui/gl/window.h`

**调用示例**：
```cpp
ui::Window window(1280, 900, "demo");
if (!window.handle()) return 1;
while (!window.should_close()) {
    window.poll_events();          // 更新鼠标/键盘/滚轮/字符队列（边沿触发）
    // ... root.layout(); root.dispatch_*(); renderer.begin_frame(...);
    window.swap_buffers();
}
```

**依赖说明**：前置：无。`Window` 需先于 `Renderer::init()` 创建（提供 GL 3.3 Core 上下文）。

**依赖 include**：`ui/gl/cursor_icon_manager.h`、`string`、`vector`

**类 `Window`**

| 方法 | 功能说明 |
| --- | --- |
| `static void init_glfw()` | GLFW 全局初始化/终止（整个进程只需调一次）； 多窗口架构下 ~Window() 不再调 glfwTerminate()，由应用层在退出时调 terminate_glfw() |
| `static void terminate_glfw()` |  |
| `Window(int width, int height, const char* title)` |  |
| `bool should_close() const` |  |
| `void poll_events()` |  |
| `void wait_events_timeout(double timeout)` |  |
| `void swap_buffers()` |  |
| `void set_vsync(int interval)` |  |
| `void make_current()` | 把本窗口的 GL 上下文设为当前（多窗口各自独立上下文：绘制前先 make_current） |
| `int width() const` |  |
| `int height() const` |  |
| `GLFWwindow* handle() const` |  |
| `static GLFWwindow* primary_handle()` | 最近创建窗口的句柄（剪贴板等系统级 API 用；无窗口时返回 nullptr） |
| `bool resized()` | 边沿触发：本轮 poll 内发生过窗口尺寸变化（读一次后清除） |
| `void set_title(const char* title)` | ---- 窗口管理 ---- |
| `void set_icon(int count, const GLFWimage* images)` |  |
| `void set_position(int xpos, int ypos)` |  |
| `void get_position(int& xpos, int& ypos) const` |  |
| `void center_on_primary_monitor()` | 在主显示器工作区内居中（避免无边框窗口默认落在屏幕角落 0,0） |
| `void set_size(int width, int height)` |  |
| `void get_size(int& width, int& height) const` |  |
| `void set_size_limits(int minw, int minh, int maxw, int maxh)` |  |
| `void set_aspect_ratio(int numer, int denom)` |  |
| `void get_framebuffer_size(int& width, int& height) const` |  |
| `void get_frame_size(int& left, int& top, int& right, int& bottom) const` |  |
| `float get_content_scale() const` |  |
| `float get_opacity() const` |  |
| `void set_opacity(float opacity)` |  |
| `void iconify()` |  |
| `void restore()` |  |
| `void maximize()` |  |
| `void show()` |  |
| `void hide()` |  |
| `void focus()` |  |
| `void request_attention()` |  |
| `bool is_visible() const` |  |
| `bool is_iconified() const` |  |
| `bool is_maximized() const` |  |
| `bool is_focused() const` |  |
| `bool is_resizable() const` |  |
| `bool is_decorated() const` |  |
| `void set_should_close(bool value)` |  |
| `void set_resizable(bool value)` |  |
| `void set_decorated(bool value)` |  |
| `void set_auto_iconify(bool value)` |  |
| `void set_floating(bool value)` |  |
| `void start_drag(float mouse_x, float mouse_y)` | ---- 拖拽（无边框窗口）---- |
| `void update_drag(float mouse_x, float mouse_y)` |  |
| `void end_drag()` |  |
| `bool is_dragging() const` |  |
| `void set_cursor(CursorType type)` | ---- 光标形状 ---- 统一光标接口 |
| `void set_cursor_arrow()` |  |
| `void set_cursor_hand()` |  |
| `void set_cursor_resize_ns()` |  |
| `void set_cursor_resize_ew()` |  |
| `void set_cursor_ibeam()` |  |
| `void set_clipboard_string(const char* string)` | ---- 剪贴板 ---- |
| `const char* get_clipboard_string() const` |  |
| `static double get_time()` | ---- 时间 ---- |
| `static void set_time(double time)` |  |
| `static GLFWmonitor* get_primary_monitor()` | ---- 显示器 ---- |
| `static GLFWmonitor** get_monitors(int& count)` |  |
| `static void get_monitor_position(GLFWmonitor* monitor, int& xpos, int& ypos)` |  |
| `static void get_monitor_workarea(GLFWmonitor* monitor, int& xpos, int& ypos, int& width, int& height)` |  |
| `static void get_monitor_content_scale(GLFWmonitor* monitor, float& xscale, float& yscale)` |  |
| `static const char* get_monitor_name(GLFWmonitor* monitor)` |  |
| `static void get_video_modes(GLFWmonitor* monitor, int& count, void*& modes)` |  |
| `static void get_video_mode(GLFWmonitor* monitor, void*& mode)` |  |
| `float mouse_x() const` | ---- 输入（像素坐标，y 向下，(0,0)=左上角）---- |
| `float mouse_y() const` |  |
| `bool mouse_down(int button) const` | button: 0=左键 1=右键 2=中键 |
| `bool mouse_pressed(int button)` | 边沿触发：本轮 poll 内新按下/新抬起（读一次后清除） |
| `bool mouse_released(int button)` |  |
| `float scroll_dx()` | 边沿触发：本轮 poll 内滚轮滚动量（dx,dy，读一次后清零） |
| `float scroll_dy()` |  |
| `bool key_down(int key) const` | 键盘（key 用 ui::Key 常量）：当前按住 / 本轮按下边沿 / 本轮抬起边沿 |
| `bool key_pressed(int key)` |  |
| `bool key_released(int key)` |  |
| `int key_pressed_mods(int key) const` | 本轮按下边沿发生时的修饰键快照（UI-102）：SendInput 等快速序列在主循环 轮询时可能已全部抬起，实时查询 key_down 拼 mods 会丢失；本方法返回 PRESS 回调瞬间的 mods，与 key_pressed 配合使用（先调 key_pressed 再取）。 |
| `int char_pressed_count() const` | 文本输入：本轮 poll 内收到的 Unicode 码点队列（读一次后清空），返回数量 |
| `unsigned int char_pressed_at(int i) const` |  |
| `void clear_chars()` |  |
| `int drop_count() const` | ---- 文件拖放 ---- |
| `const std::string& drop_path(int i) const` |  |
| `void clear_drops()` |  |


### `ui/gl/renderer.h`

**调用示例**：
```cpp
ui::Renderer renderer;
if (!renderer.init()) return 1;            // GL 上下文就绪后一次
renderer.begin_frame(theme.window_bg);     // 每帧清屏
renderer.draw_quad({10, 10, 200, 60}, ui::Vec4{1, 0, 0, 1});   // radius=0 别名
renderer.draw_rounded_rect({10, 80, 200, 60}, 8.f, theme.accent);
renderer.draw_rect_outline({10, 150, 200, 60}, 2.f, theme.border);
renderer.draw_textured_quad(rect, tex.id(), {0,0,1,1}, ui::Vec4{1,1,1,1});
renderer.draw_polygon(pts, n, color);
renderer.push_scissor({0, 0, 400, 300});   // 可嵌套
renderer.pop_scissor();
```

**依赖说明**：前置：GL 上下文就绪后调用 `init()` 一次；每次绘制前 `begin_frame()`；窗口缩放时 `set_viewport()`。

**依赖 include**：`ui/core/math.h`、`functional`、`vector`

**类 `Renderer`**

| 方法 | 功能说明 |
| --- | --- |
| `Renderer() = default` |  |
| `bool init()` | 必须在 GL 上下文就绪后调用一次 |
| `void begin_frame(const Vec4& clear_color)` |  |
| `void set_viewport(int width, int height)` |  |
| `void draw_quad(const Rect& rect, const Vec4& color)` | 纯色矩形（radius > 0 时为圆角，fwidth SDF 抗锯齿）。 契约（防 UI-037 回归）：radius=0（draw_quad）矩形内部必须全不透明； 颜色管线 SDF 若缺内部距离项，内部会以 50% alpha 混合（全界面灰蒙蒙）。 |
| `void draw_rounded_rect(const Rect& rect, float radius, const Vec4& color)` |  |
| `void draw_rect_outline(const Rect& rect, float thickness, const Vec4& color)` | 矩形描边（厚度为 t 的边框，4 条薄 quad 拼合，无圆角） |
| `void draw_rounded_rect_outline(const Rect& rect, float radius, float thickness, const Vec4& color)` | 圆角矩形描边：厚度为 thickness 的圆角环，半径/圆角随 radius 自动（UI-053） |
| `void draw_textured_quad(const Rect& rect, unsigned int texture_id, const Rect& uv, const Vec4& color, float radius = 0.f, float rotation = 0.f)` | 纹理四边形：uv = {u0, v0, u1, v1}（0..1，v 为纹理上边→下边）；radius>0 时 SDF 圆角裁剪； rotation 为绕矩形中心旋转角（弧度，0 不旋转），用于图标/旋转动画（如时间线 loading） |
| `void draw_polygon(const Vec2* points, int count, const Vec4& color)` | 填充多边形（凸多边形三角扇，首顶点为扇心；像素坐标 y 向下）。顶点按顺时针或逆时针均可 |
| `void push_scissor(const Rect& rect)` | 裁剪：此后绘制被限制在 rect 内（可嵌套，pop 恢复上一个）。像素坐标 y 向下 |
| `void pop_scissor()` |  |
| `int viewport_width() const` | 当前视口尺寸（由 set_viewport 记录，供 UI 层使用） |
| `int viewport_height() const` |  |
| `Rect current_scissor() const` | 当前裁剪区（scissor 栈顶）：即当前「可视屏」矩形（像素坐标 y 向下）。 未 push_scissor 时返回整个视口。供 LazyImage 判断「是否在当前屏内」以触发懒加载（lay-src）。 |


### `ui/gl/texture.h`

**调用示例**：
```cpp
ui::Texture tex;
tex.load_from_file("sprite.png");        // stb_image 解码
tex.create(320, 240, rgba);              // 或手工建纹理
tex.update(rgba);                        // 整图更新（尺寸须一致，视频逐帧用）
tex.update_sub(x, y, w, h, rgba);        // 局部更新（动态字体图集用）
renderer.draw_textured_quad({0,0,320,240}, tex.id(), {0,0,1,1}, ui::Vec4{1,1,1,1});
```

**依赖说明**：前置：GL 上下文就绪（Renderer init 之后）。

**类 `Texture`**

| 方法 | 功能说明 |
| --- | --- |
| `Texture() = default` |  |
| `bool load_from_file(const char* path)` |  |
| `void create(int width, int height, const void* rgba)` |  |
| `void update(const void* rgba)` | 更新已创建纹理的内容（尺寸须与 create 一致，视频逐帧上传用） |
| `void update_sub(int x, int y, int w, int h, const void* rgba)` | 局部更新纹理矩形区域（动态字体图集按需烘培用）；区域须在纹理边界内 |
| `void bind(unsigned int unit = 0) const` |  |
| `unsigned int id() const` |  |
| `int width() const` |  |
| `int height() const` |  |

| 数据成员 | 说明 |
| --- | --- |
| `Texture& operator=(const Texture&) = delete;` |  |


### `ui/gl/font.h`

**调用示例**：
```cpp
ui::Font font;
font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
font.load_cjk_font_auto();               // 懒烘培中文字体（环境变量 OPENGUI_CJK_FONT 可自定义）
font.load_icon_font("iconfont.ttf", 18.f);
font.draw_text(renderer, "你好 UI", 10, 10, theme.text);
float w = font.measure_text("宽度");
font.draw_icon(renderer, ui::layui_icon::ic_about, cx, cy, 18.f, theme.success);
```

**依赖说明**：前置：GL 上下文就绪；`draw_text/draw_icon/measure_*` 依赖 Renderer。中文字体需 `load_cjk_font*` 后才可绘制非 ASCII。

**依赖 include**：`ui/core/math.h`、`chrono`、`unordered_map`、`unordered_set`、`vector`

**类 `Font`**

| 方法 | 功能说明 |
| --- | --- |
| `Font()` |  |
| `bool load_from_file(const char* path, float pixel_height = 32.f)` | 从 .ttf 加载并烘培图集；pixel_height 为像素字号 |
| `bool load_cjk_font(const char* path, int font_index = 0)` | 加载中文字体（懒烘培源）：保留 TTF/TTC 字节，首次遇到图集外码点按需光栅化。 .ttc 集合用 font_index 选取子字体；失败返回 false（ASCII 仍可用）。 |
| `bool load_cjk_font_auto()` | 自动探测 CJK 字体：OPENGUI_CJK_FONT 环境变量（自定义字体）→ 系统候选 msyh.ttc / msyh.ttf / simhei.ttf / simsun.ttc / Deng.ttf；命中打日志。 |
| `bool load_emoji_font(const char* path)` | 加载 emoji 字体（UI-128）：中文字体缺字码点（增补平面 emoji U+1F300+ 等） 的回退懒烘培源。stbtt 仅支持轮廓字形——Windows seguiemj.ttf 的 glyf 单色 轮廓可正常烘焙（COLR 彩色分层忽略，形状可辨）；位图彩色 emoji 字体不可用。 |
| `bool load_emoji_font_auto()` | 自动探测 emoji 字体：OPENGUI_EMOJI_FONT 环境变量 → Windows seguiemj.ttf。 macOS/Linux 系统彩色 emoji 为位图格式（stbtt 无法光栅化），默认不探测， 需要时用环境变量指向单色轮廓字体（Symbola / NotoSansSymbols2 等）。 失败静默返回 false（无 emoji 字形时维持方块字宽近似，不报错刷屏）。 |
| `bool load_icon_font(const char* path, float pixel_height)` | 加载图标字体（M17d）：烘培 U+E600..E7FF 私用区 + U+1000..100F（layui 2.13.x close/ok-circle 等非私用区码点）到第二图集（layui iconfont.ttf） |
| `void draw_text(Renderer& renderer, const char* text, float x, float y, const Vec4& color, float scale = 1.f, bool bold = false) const` | (x, y) 为文本左上角（像素坐标，y 向下）；UTF-8 输入，私用区码点走图标图集。 scale 为相对烘培像素号的缩放倍率（图集 2x 过采样，≤2x 仍清晰）；bold 为仿粗体 （字形沿 x 错位叠加一次，偏移随字号缩放）。 |
| `void draw_text(Renderer& renderer, const char* text, float x, float y, const Vec4& color, int max_len, float scale = 1.f, bool bold = false) const` | 带长度的重载：绘制 text 的前 max_len 字节，避免 substr 堆分配 |
| `float measure_text(const char* text, float scale = 1.f) const` |  |
| `float char_width(unsigned int codepoint) const` | 单字符宽度：ASCII 用字形 advance；私用区图标用图标 advance；CJK 懒烘培后取真实 advance， 仍缺失时按方块字近似为字号 |
| `bool icon_loaded() const` |  |
| `void prebake_text(const char* utf8)` | 强制烘焙一段 UTF-8 文本的全部 CJK 字形（UI-125）：不受逐帧配额限制， 供启动预热等场景一次性消除首绘卡顿；重复码点/图集内字形自动跳过 |
| `static bool any_pending_bake()` | ---- 逐帧烘焙推进状态（UI-136）----    // 批次受 16ms 窗口限额，只有「渲染帧」才会烘焙下一批；帧级门控宿主 （root.is_dirty() 决定跳帧）必须在每帧无条件执行处查询这两个信号： 有待烘焙字形（配额耗尽未烘）→ 继续渲染； 烘焙 epoch 变化（有新字形上屏，xadvance 可能变化）→ 全树重排一次。 两者皆静 → 恢复空闲停帧。 |
| `static unsigned bake_epoch_sum()` |  |
| `void draw_icon(Renderer& renderer, unsigned codepoint, float cx, float cy, float size, const Vec4& color, float rotation = 0.f) const` | 以 (cx, cy) 为中心绘制单个图标，目标尺寸 size（像素）；rotation 绕中心旋转（弧度，0 不转） |
| `float ascent() const` |  |
| `float descent() const` |  |
| `float line_height() const` |  |
| `float pixel_height() const` |  |
| `bool cjk_ready() const` |  |

| 数据成员 | 说明 |
| --- | --- |
| `Font& operator=(const Font&) = delete;` |  |

**类 `Glyph`**

| 数据成员 | 说明 |
| --- | --- |
| `float x0 = 0, y0 = 0, x1 = 0, y1 = 0;` | 相对基线（像素，y 向下） |
| `float u0 = 0, v0 = 0, u1 = 0, v1 = 0;` | 图集 UV |
| `float xadvance = 0;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `static unsigned decode_utf8(const char*& in)` | UTF-8 解码下一个码点；*in 指向字节，返回解码后的码点，*in 前进到下一字符（返回 0 表示结束/非法） |
| `const Glyph* ensure_dynamic(unsigned int cp) const` | 懒烘培 CJK 字形：查 dyn_ → 命中返回；cjk 字体未载入返回 nullptr；否则光栅化打包进图集 |
| `void bake_cjk(unsigned int cp) const` |  |
| `const Glyph* icon_glyph(unsigned int cp) const` | 图标字形查询：按码点落在 E600..E7FF / 1000..100F 哪一段返回对应字形，段外返回 nullptr |


## 5、ui/ui 组件（`ui/ui`）

### `ui/ui/widget.h`

**调用示例**：
```cpp
ui::Widget root;                        // 根节点（应用持有栈对象）
root.set_bounds(0, 0, w, h);

ui::Button* b = new ui::Button(0, 0, 100, 36, "OK");
b->add_class("layui-btn layui-btn-primary");   // CSS 类名机制
root.add_child(b);                      // 裸指针，由树接管

// 每帧：先布局，再分发事件，最后绘制
root.layout();
root.dispatch_mouse_move(x, y);
root.dispatch_key_down(key, mods);
root.draw(renderer);

// 生命周期：销毁子树用 clear_children()（递归释放）；销毁单控件及其子树用 destroy()
root.clear_children();
b->destroy();
```

**依赖说明**：前置：无。根节点由应用持有，`add_child` 的控件为裸指针、由根节点或布局容器接管释放（见 `clear_children`/`destroy` 注释）。

**依赖 include**：`ui/core/keycodes.h`、`ui/core/math.h`、`string`、`vector`

**类 `Widget`**

| 方法 | 功能说明 |
| --- | --- |
| `explicit Widget(float x = 0, float y = 0, float w = 0, float h = 0)` |  |
| `void set_bounds(float x, float y, float w, float h)` |  |
| `const Rect& bounds() const` | 像素坐标，y 向下 |
| `virtual void set_enabled(bool en)` |  |
| `bool is_dirty() const` | ---- 脏标记机制：空闲时跳过无变子树绘制，降低 CPU/GPU 占用 ---- |
| `void mark_dirty()` |  |
| `void clear_dirty()` |  |
| `void mark_dirty_all()` | 全树递归置脏（UI-136）：字形逐帧烘焙导致 xadvance 变化后，由根节点 advance 调用，强制所有 Label 以真实宽度重新换行/布局（近似宽度 → 真实宽度） |
| `void add_class(const char* cls)` | ---- CSS 类名机制（M17a）：layui 语法入口，供样式层查询 ---- 支持空格分隔的多个类名（如 "layui-btn layui-btn-danger"） |
| `void remove_class(const char* cls)` |  |
| `bool has_class(const char* cls) const` |  |
| `const std::vector<std::string>& classes() const` |  |
| `void add_child(Widget* child)` | ---- 树结构 ---- |
| `void remove_child(Widget* child)` |  |
| `void clear_children()` | 递归释放全部子树：逐个 remove_child + delete（含深层子节点），调用后 children() 为空。 调用者持有子树所有权时使用（切页释放/列表重建）；不释放调用者自身。 注意：任何指向已删除子控件的指针立即失效（UI-039 回归保证）。 |
| `void destroy()` | 销毁自身（含整个子树）：从父节点摘除后递归释放子树并 delete 自身。 等价于「父节点 remove_child + 子树 clear_children + delete 自身」一条龙， 用于销毁单个控件及其子树（切页清理根级弹窗等）。 注意：调用后本控件及子树内任何指针立即失效，不得再访问（UI-040 回归保证）； 无父节点时直接销毁子树与自身。  延迟销毁（UI-072）：destroy() 只把自身标记为待销毁（立即 visible=false）， 实际的「摘除 popups/父引用 + 释放子树 + delete」统一延迟到根节点帧末 flush_pending_destroys() 执行。这样在事件分发（dispatch_mouse_*）或遍历 （advance/draw/children_ 迭代）期间销毁自身不会产生悬空 this 与失效迭代器 （如 footer 回调 cleanup_windows 在 dispatch_mouse_down 内自毁窗口）。 |
| `static void flush_pending_destroys()` | 帧末统一销毁被 destroy() 标记的控件（由根节点 advance 自动调用） |
| `Widget* parent() const` |  |
| `const std::vector<Widget*>& children() const` |  |
| `void bring_to_front()` | 置顶：把自身移到父节点子控件列表末尾（后添加者在绘制/命中上层），无父节点时无操作 |
| `bool contains(float x, float y) const` | 屏幕坐标命中测试 |
| `virtual bool on_mouse_move(float x, float y)` | ---- 事件（屏幕坐标，y 向下）---- 返回 true 表示事件被消费（阻止冒泡到兄弟/父节点） |
| `virtual bool on_mouse_down(float x, float y, int button)` |  |
| `virtual bool on_mouse_up(float x, float y, int button)` |  |
| `virtual bool on_mouse_scroll(float x, float y, float dx, float dy)` |  |
| `virtual bool on_drop(float x, float y, const std::vector<std::string>& paths)` | 文件拖放：paths 为拖入的文件/文件夹路径列表 |
| `void capture_mouse()` | 鼠标捕获（M5 拖拽）：on_mouse_down 返回 true 即自动捕获（如 Slider 拖动）， 此后鼠标 move/up 即使指针离开控件范围也路由给捕获者；up 后自动释放 |
| `void release_mouse_capture()` |  |
| `void open_popup(Widget* owner, Widget* popup)` | ---- 弹出层（M9）：根节点维护弹出面板（下拉/菜单），点击外部自动关闭 ---- 打开弹出面板（owner 内点击不关闭它；面板绘制在控件树最上层） |
| `void close_popups()` | 关闭所有弹出面板 |
| `void close_topmost_popup()` | 关闭最上层弹出面板（layer.escIndex 语义：Esc 关闭当前最上层窗口） |
| `bool has_open_popups()` |  |
| `void set_modal(Widget* modal)` | ---- 模态（M14）：根节点设置后，事件仅路由给模态子树，其余控件被屏蔽 ---- 用完后传 nullptr 解除模态 |
| `Widget* modal() const` |  |
| `void set_tooltip(const char* text)` | ---- Tooltip（M14）：悬停提示文本（由应用层在鼠标位置绘制）---- |
| `const std::string& tooltip() const` |  |
| `bool dispatch_mouse_move(float x, float y)` | 事件分发入口：从最上层子节点开始命中（后添加的子节点在上层） |
| `bool dispatch_mouse_down(float x, float y, int button)` |  |
| `bool dispatch_mouse_up(float x, float y, int button)` |  |
| `bool dispatch_mouse_scroll(float x, float y, float dx, float dy)` |  |
| `bool dispatch_drop(float x, float y, const std::vector<std::string>& paths)` |  |
| `bool hovered() const` |  |
| `bool pressed() const` |  |
| `virtual void on_focus_changed(bool focused)` | 获取/失去焦点回调（子类用于高亮） |
| `virtual bool on_key_down(int key, int mods)` | 键盘事件（key 用 ui::Key 常量，mods 用 ui::KeyMod） |
| `virtual bool on_key_up(int key, int mods)` |  |
| `virtual bool on_char(unsigned int codepoint)` | 文本输入事件（Unicode 码点；仅分发给当前焦点控件） |
| `void focus()` | 请求成为当前焦点（仅 focusable 生效）；blur 释放焦点 |
| `void blur()` |  |
| `bool focused() const` |  |
| `bool dispatch_key_down(int key, int mods)` | 键盘事件分发：先让焦点控件处理；导航键（Tab/方向键）未被消费才做焦点遍历 |
| `bool dispatch_key_up(int key, int mods)` |  |
| `bool dispatch_char(unsigned int codepoint)` | 字符分发：转发给当前焦点控件 |
| `void focus_next()` | Tab 顺序遍历可聚焦控件（供 dispatch_key_down 内部及外部调用） |
| `void focus_prev()` |  |
| `Widget* root()` | 树的根节点（向上追溯） |
| `virtual void on_layout()` | ---- 布局 ---- 布局容器覆写：计算子树位置/尺寸。由 draw 前调用者（root）触发一次 |
| `void layout()` | 递归：先布局自身，再遍历子树 |
| `virtual float preferred_height() const` | 内容自适应高度（像素）：布局容器调用；默认返回 bounds_.h |
| `virtual void on_advance(float dt)` | ---- 逐帧推进（UI-069）：应用层每帧调用 root->advance(dt)，供自动关闭/动画计时 ---- |
| `void advance(float dt)` |  |
| `virtual void on_draw(Renderer& renderer)` | ---- 绘制 ---- 按可见性遍历子树；父节点先画背景，子节点叠在其上 |
| `virtual void draw(Renderer& renderer)` |  |
| `virtual void draw_overlay(Renderer& renderer)` | 叠加层绘制（在所有兄弟节点之后调用，用于下拉面板等溢出内容） |

| 数据成员 | 说明 |
| --- | --- |
| `Widget& operator=(const Widget&) = delete;` |  |
| `bool visible = true;` |  |
| `bool enabled = true;` | 禁用态（M22）：禁用控件不接收鼠标/键盘/焦点，Tab 遍历跳过，绘制层灰化显示 |
| `bool focusable = false;` | ---- 焦点 / 键盘（M3 + M4 字符输入）---- |

**类 `Popup`**

| 数据成员 | 说明 |
| --- | --- |
| `Widget* owner;` |  |
| `Widget* popup;` |  |


### `ui/ui/scene.h`

**调用示例**：
```cpp
ui::Window window(1280, 900, "demo");
ui::Renderer renderer; renderer.init();
ui::Widget root; root.set_bounds(0, 0, 1280, 900);

ui::Scene scene(&window, &renderer, &root);   // 随窗口同步创建，构造广播 Ready
scene.run();                                  // poll + 分配(layout/dispatch/advance) + 消费(begin_frame+draw) + swap

// 或自定义主循环：run_for(秒) 承载 --smoke 通用语义
scene.run_for(2.0f);

// 全量订阅：阶段向量广播 + 逐事件钩子
scene.subscribe(ui::ScenePhase::Frame, [&]{ /* 每帧推进媒体/动画 */ });
scene.subscribe(ui::ScenePhase::PostDraw, [&]{ renderer.draw_text(...); }); // 应用直绘 HUD
scene.on(ui::SceneEvent::Key, [&](int key, int mods){ if (key==UI_KEY_ESCAPE) { scene.request_exit(); } return false; });

// 清理重置只清库内自管对象；子节点可为栈对象，严禁对子树 delete
```

**依赖说明**：前置：需要 `ui::Window`（已创建）+ `Renderer::init()`（GL 3.3 Core）。Scene 随窗口同步创建、构造即广播 Ready；每窗口一个 Scene 绑定同一 Window/Renderer/root。清理重置只清库内自管对象（flush_pending_destroys + close_popups），不裁子树——子节点可为栈对象。

**依赖 include**：`ui/core/math.h`、`ui/ui/widget.h`、`array`、`functional`

**类 `Scene`**

| 方法 | 功能说明 |
| --- | --- |
| `Scene(ui::Window& window, ui::Renderer& renderer)` | 生产：绑定窗口/渲染器。构造即广播 ScenePhase::Ready。 窗口、渲染器由调用方持有（非拥有关系），存活期必须 ≥ Scene。 |
| `ui::Window& window()` | ---- 资源访问 ---- |
| `ui::Renderer& renderer()` |  |
| `ui::Widget& root()` | 根控件树（Scene 拥有，构造时按窗口尺寸初始化 bounds；reset 清空后重建） |
| `void set_root_background(const ui::Vec4& clear_color)` |  |
| `void set_chrome(Widget* chrome, float top)` | ---- 顶部装帧（chrome）内容区原点 ---- 注册一个自绘顶部条（如无边框窗口的自绘 TitleBar）与它占用的顶部高度 top。 库由此掌握「内容区起始 y」，Rest/Draw/HUD 等直接绘制可经 content_origin_y() 统一对齐，避免各应用在 header/sidebar/content/HUD 里各自硬编码同一偏移量。 chrome 仍由应用作为 root 普通子控件加入（其自身以绝对坐标绘制于 y=0）； 本 API 仅登记其高度与指针（不拥有、不移动 chrome），供内容区消费。 |
| `Widget* chrome() const` |  |
| `float content_origin_y() const` | 内容区起始 y（= 顶部 chrome 占用高度，未注册时为 0） |
| `size_t subscribe(ScenePhase phase, PhaseFn fn)` | 返回订阅 token（非 0），供 unsubscribe 单独摘除（页面级局部订阅切页时摘除，防悬空）。 |
| `size_t on(ScenePhase phase, std::function<void(Scene&)> fn)` | 便捷重载：不关心 dt 的阶段/帧回调；同样返回 token |
| `void unsubscribe(size_t token)` | 按 token 摘除单个阶段观察者；token 为 0 或不存在则无操作。 与 clear_observers() 不同：仅移除这一个订阅，不清其他（App 级）订阅。 |
| `void set_event_hook(SceneEvent event, EventHook hook)` |  |
| `void clear_observers()` | 清空全部订阅与钩子（reset/shutdown 内部调用，防闭包悬空引用旧树/旧窗口） |
| `int event_key() const` | 最近一次 Key/Scroll/MouseDown 等事件的键码（event_key()）或滚轮量（event_scroll_dx/dy） |
| `float event_scroll_dx() const` |  |
| `float event_scroll_dy() const` |  |
| `void frame(float dt)` | ---- 消费者：运行 ---- 单帧：算 dt（由调用方传入）→ 分配 → 绘制。不进入 poll 循环。 |
| `void run()` | 直至窗口关闭：poll_events（生产者）→ frame |
| `void run_for(double seconds)` | 运行至超时自动退出（--smoke 通用语义）：内部按帧 poll 并累计运行时长 |
| `bool running() const` |  |
| `void request_exit()` | 请求退出（Run 循环下一帧即止；如 demo_exit 按钮） |
| `void reset()` | ---- 清理重置 ---- 关弹窗 + 排空延迟销毁队列 + 广播 Reset→Ready（重跑新一轮生命周期；先订阅 Reset/Ready 再调用） 不删子树（子节点可能为栈对象，见类注释） |
| `void shutdown()` | 广播 Cleanup/Shutdown + flush_pending_destroys（析构自动调用；可手动提前） |

| 数据成员 | 说明 |
| --- | --- |
| `Scene& operator=(const Scene&) = delete;` |  |
| `bool dispatch_mouse = true;` | ---- 分配者/消费者开关（保留各应用迁移前按键语义，默认全开）---- mouse_move + mouse_down/up |
| `bool dispatch_scroll = true;` | scroll_dx/dy -> dispatch_mouse_scroll |
| `bool dispatch_nav_keys = true;` | Tab/方向/Enter/Space/Backspace/Delete/Home/End/Escape |
| `bool dispatch_text = true;` | char 队列 -> dispatch_char |
| `bool dispatch_resize = true;` | window.resized() -> set_viewport + root.set_bounds |

**类 `Observer`**

| 数据成员 | 说明 |
| --- | --- |
| `size_t token;` |  |
| `PhaseFn fn;` |  |


### `ui/ui/theme.h`

**调用示例**：
```cpp
ui::Theme theme = ui::Theme::make_layui();   // 或 make_dark()/make_light()
// 分发同一地址：切换主题时重建该对象内容即可全局即时生效
button->set_theme(&theme);
label->set_theme(&theme);
label->set_color(theme.text_secondary);
```

**依赖说明**：前置：无。把 `Theme` 对象地址经 `set_theme(&theme)` 分发给控件，切换主题时重建同一地址即可即时生效。

**依赖 include**：`ui/core/math.h`

**类 `Theme`**

| 方法 | 功能说明 |
| --- | --- |
| `static Theme make_dark()` | 预设（M10）：新增主题只需在此加一个工厂函数，控件无需改动 |
| `static Theme make_light()` |  |
| `static Theme make_layui()` |  |

| 数据成员 | 说明 |
| --- | --- |
| `Vec4 window_bg{0.12f, 0.13f, 0.15f, 1.0f};` |  |
| `Vec4 button_normal{0.22f, 0.23f, 0.26f, 1.0f};` |  |
| `Vec4 button_hover{0.28f, 0.29f, 0.33f, 1.0f};` |  |
| `Vec4 button_pressed{0.16f, 0.17f, 0.20f, 1.0f};` |  |
| `Vec4 input_bg{0.16f, 0.17f, 0.19f, 1.0f};` | 输入类控件（TextBox/TextArea）：内容底 + 边框 |
| `Vec4 input_border{0.35f, 0.36f, 0.40f, 1.0f};` |  |
| `Vec4 accent{0.13f, 0.58f, 0.95f, 1.0f};` | 主色（Bootstrap primary） |
| `Vec4 accent_hover{0.10f, 0.49f, 0.84f, 1.0f};` |  |
| `Vec4 accent_pressed{0.08f, 0.40f, 0.72f, 1.0f};` |  |
| `Vec4 text{0.92f, 0.93f, 0.95f, 1.0f};` |  |
| `Vec4 text_dim{0.62f, 0.64f, 0.68f, 1.0f};` |  |
| `float radius = 6.f;` | 圆角半径（像素） |
| `Vec4 success{0.09f, 0.72f, 0.47f, 1.0f};` | ---- layui 克隆（M17a）：语义色 / 边框 / 面板 / 次级文字 ---- #16b777 表单焦点/成功 |
| `Vec4 info{0.12f, 0.62f, 1.0f, 1.0f};` | #1e9fff normal 蓝 |
| `Vec4 warning{1.0f, 0.72f, 0.0f, 1.0f};` | #ffb800 warm 黄 |
| `Vec4 danger{1.0f, 0.34f, 0.13f, 1.0f};` | #ff5722 danger 红 |
| `Vec4 border{0.93f, 0.93f, 0.93f, 1.0f};` | #eee 边框 |
| `Vec4 panel{1.0f, 1.0f, 1.0f, 1.0f};` | 面板/卡片白底 |
| `Vec4 text_secondary{0.37f, 0.37f, 0.37f, 1.0f};` | #5f5f5f 次级文字 |
| `Vec4 text_muted{0.6f, 0.6f, 0.6f, 1.0f};` | #999 弱化文字 |


### `ui/ui/layui_style.h`

**调用示例**：
```cpp
ui::Vec4 c = ui::layui::hex("#16baaa");          // 十六进制转色
ui::Vec4 bg = ui::layui::bg_fill("layui-bg-green");  // 工具色背景
ui::Vec4 fg = ui::layui::bg_color("layui-bg-red");    // 工具色前景（白字）
bool has = ui::layui::has_class(widget, "layui-btn");
```

**依赖说明**：前置：无。供绘制层查询 layui 工具色；`hex()` 十六进制转 Vec4。

**依赖 include**：`ui/core/math.h`

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `Vec4 bg_color(const char* cls)` | 工具色：bg-red/orange/green/cyan/blue/purple/black/gray -> 前景色（#fff 文字） |
| `Vec4 bg_fill(const char* cls)` | 工具色背景底（layui-bg-* 的 background-color 值） |
| `Vec4 font_color(const char* cls)` | 文字工具色（layui-font-* 的 color 值，2.13.x）：red/orange/green/cyan/blue/ purple/black/gray。色值与 layui-bg-* 不同源（green=#16baaa、blue=#01aaed、 black=#000、gray=#c2c2c2）；未知类回退黑色。 |
| `Vec4 hex(const char* hex)` | 十六进制颜色串 "#16baaa" / "16baaa" -> Vec4 |
| `bool has_class(const Widget* w, const char* cls)` | 组件便捷查询：widget 是否带某类 |


### `ui/ui/layui_icons.h`

**调用示例**：
```cpp
// 图标码点常量（190 项，layui-iconfont.ttf 私用区 U+E600..E7FF）
font.load_icon_font("iconfont.ttf", 18.f);
font.draw_icon(renderer, ui::layui_icon::ic_about, cx, cy, 18.f, ui::Vec4{1,1,1,1});
```

**依赖说明**：前置：配合 `Font::load_icon_font()`（iconfont.ttf）与 `Font::draw_icon()` 使用。

**依赖 include**：`cstdint`

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `uint32_t from_class(const char* name)` | 类名 -> 码点：入参为类名去掉 layui-icon- 前缀的部分（如 "star"），找不到返回 0。 |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `ic_404` | `0xE61C` | layui-icon-404 |
| `about` | `0xE60B` | layui-icon-about |
| `add_1` | `0xE654` | layui-icon-add-1 |
| `add_circle` | `0xE61F` | layui-icon-add-circle |
| `add_circle_fine` | `0xE608` | layui-icon-add-circle-fine |
| `addition` | `0xE624` | layui-icon-addition |
| `align_center` | `0xE647` | layui-icon-align-center |
| `align_left` | `0xE649` | layui-icon-align-left |
| `align_right` | `0xE648` | layui-icon-align-right |
| `android` | `0xE684` | layui-icon-android |
| `app` | `0xE653` | layui-icon-app |
| `at` | `0xE687` | layui-icon-at |
| `auz` | `0xE672` | layui-icon-auz |
| `backspace` | `0xE694` | layui-icon-backspace |
| `bluetooth` | `0xE689` | layui-icon-bluetooth |
| `bot` | `0xE7D6` | layui-icon-bot（2.13.9 新增） |
| `camera` | `0xE660` | layui-icon-camera |
| `camera_fill` | `0xE65D` | layui-icon-camera-fill |
| `carousel` | `0xE634` | layui-icon-carousel |
| `cart` | `0xE657` | layui-icon-cart |
| `cart_simple` | `0xE698` | layui-icon-cart-simple |
| `cellphone` | `0xE678` | layui-icon-cellphone |
| `cellphone_fine` | `0xE63B` | layui-icon-cellphone-fine |
| `chart` | `0xE62C` | layui-icon-chart |
| `chart_screen` | `0xE629` | layui-icon-chart-screen |
| `chat` | `0xE606` | layui-icon-chat |
| `chrome` | `0xE68A` | layui-icon-chrome |
| `circle` | `0xE63F` | layui-icon-circle |
| `circle_dot` | `0xE617` | layui-icon-circle-dot |
| `clear` | `0xE788` | layui-icon-clear |
| `close` | `0x1006` | layui-icon-close（2.13.x 新增，非私用区） |
| `close_fill` | `0x1007` | layui-icon-close-fill（2.13.x 新增，非私用区） |
| `code_circle` | `0xE635` | layui-icon-code-circle |
| `cols` | `0xE610` | layui-icon-cols |
| `component` | `0xE857` | layui-icon-component |
| `console` | `0xE665` | layui-icon-console |
| `date` | `0xE637` | layui-icon-date |
| `ic_delete` | `0xE640` | layui-icon-delete |
| `dialogue` | `0xE63A` | layui-icon-dialogue |
| `diamond` | `0xE735` | layui-icon-diamond |
| `disabled` | `0xE6CC` | layui-icon-disabled |
| `dollar` | `0xE659` | layui-icon-dollar |
| `down` | `0xE61A` | layui-icon-down |
| `download_circle` | `0xE601` | layui-icon-download-circle |
| `edge` | `0xE68B` | layui-icon-edge |
| `edit` | `0xE642` | layui-icon-edit |
| `email` | `0xE618` | layui-icon-email |
| `engine` | `0xE628` | layui-icon-engine |
| `error` | `0xE693` | layui-icon-error |
| `ic_export` | `0xE67D` | layui-icon-export |
| `eye` | `0xE695` | layui-icon-eye |
| `eye_invisible` | `0xE696` | layui-icon-eye-invisible |
| `face_cry` | `0xE69C` | layui-icon-face-cry |
| `face_smile` | `0xE6AF` | layui-icon-face-smile |
| `face_smile_b` | `0xE650` | layui-icon-face-smile-b |
| `face_smile_fine` | `0xE60C` | layui-icon-face-smile-fine |
| `face_surprised` | `0xE664` | layui-icon-face-surprised |
| `female` | `0xE661` | layui-icon-female |
| `file` | `0xE621` | layui-icon-file |
| `file_b` | `0xE655` | layui-icon-file-b |
| `find_fill` | `0xE670` | layui-icon-find-fill |
| `fire` | `0xE756` | layui-icon-fire |
| `firefox` | `0xE686` | layui-icon-firefox |
| `flag` | `0xE66C` | layui-icon-flag |
| `folder` | `0xEABE` | layui-icon-folder |
| `folder_open` | `0xEAC1` | layui-icon-folder-open |
| `fonts_clear` | `0xE639` | layui-icon-fonts-clear |
| `fonts_code` | `0xE64E` | layui-icon-fonts-code |
| `fonts_del` | `0xE64F` | layui-icon-fonts-del |
| `fonts_html` | `0xE64B` | layui-icon-fonts-html |
| `fonts_i` | `0xE644` | layui-icon-fonts-i |
| `fonts_strong` | `0xE62B` | layui-icon-fonts-strong |
| `fonts_u` | `0xE646` | layui-icon-fonts-u |
| `form` | `0xE63C` | layui-icon-form |
| `friends` | `0xE612` | layui-icon-friends |
| `gift` | `0xE627` | layui-icon-gift |
| `gitee` | `0xE69B` | layui-icon-gitee |
| `github` | `0xE6A7` | layui-icon-github |
| `group` | `0xE613` | layui-icon-group |
| `headset` | `0xE6FC` | layui-icon-headset |
| `heart` | `0xE68C` | layui-icon-heart |
| `heart_fill` | `0xE68F` | layui-icon-heart-fill |
| `help` | `0xE607` | layui-icon-help |
| `home` | `0xE68E` | layui-icon-home |
| `ie` | `0xE7BB` | layui-icon-ie |
| `ios` | `0xE680` | layui-icon-ios |
| `key` | `0xE683` | layui-icon-key |
| `layer` | `0xE638` | layui-icon-layer |
| `layim_download` | `0xE61E` | layui-icon-layim-download |
| `layim_theme` | `0xE61B` | layui-icon-layim-theme |
| `layim_uploadfile` | `0xE61D` | layui-icon-layim-uploadfile |
| `layouts` | `0xE632` | layui-icon-layouts |
| `leaf` | `0xE701` | layui-icon-leaf |
| `left` | `0xE603` | layui-icon-left |
| `light` | `0xE748` | layui-icon-light |
| `link` | `0xE64C` | layui-icon-link |
| `list` | `0xE60A` | layui-icon-list |
| `loading` | `0xE63D` | layui-icon-loading |
| `loading_1` | `0xE63E` | layui-icon-loading-1 |
| `location` | `0xE715` | layui-icon-location |
| `lock` | `0xE69A` | layui-icon-lock |
| `log` | `0xE60E` | layui-icon-log |
| `login_qq` | `0xE676` | layui-icon-login-qq |
| `login_wechat` | `0xE677` | layui-icon-login-wechat |
| `login_weibo` | `0xE675` | layui-icon-login-weibo |
| `logout` | `0xE682` | layui-icon-logout |
| `male` | `0xE662` | layui-icon-male |
| `menu_fill` | `0xE60F` | layui-icon-menu-fill |
| `mike` | `0xE6DC` | layui-icon-mike |
| `moon` | `0xE6C2` | layui-icon-moon |
| `more` | `0xE65F` | layui-icon-more |
| `more_vertical` | `0xE671` | layui-icon-more-vertical |
| `music` | `0xE690` | layui-icon-music |
| `mute` | `0xE685` | layui-icon-mute |
| `next` | `0xE65B` | layui-icon-next |
| `note` | `0xE66E` | layui-icon-note |
| `notice` | `0xE667` | layui-icon-notice |
| `ok` | `0xE605` | layui-icon-ok |
| `ok_circle` | `0x1005` | layui-icon-ok-circle（2.13.x 新增，非私用区） |
| `password` | `0xE673` | layui-icon-password |
| `pause` | `0xE651` | layui-icon-pause |
| `picture` | `0xE64A` | layui-icon-picture |
| `picture_fine` | `0xE60D` | layui-icon-picture-fine |
| `play` | `0xE652` | layui-icon-play |
| `praise` | `0xE6C6` | layui-icon-praise |
| `prev` | `0xE65A` | layui-icon-prev |
| `prev_circle` | `0xE633` | layui-icon-prev-circle |
| `print` | `0xE66D` | layui-icon-print |
| `question` | `0xE699` | layui-icon-question |
| `radio` | `0xE643` | layui-icon-radio |
| `rate` | `0xE67B` | layui-icon-rate |
| `rate_half` | `0xE6C9` | layui-icon-rate-half |
| `rate_solid` | `0xE67A` | layui-icon-rate-solid |
| `read` | `0xE705` | layui-icon-read |
| `reduce_circle` | `0xE616` | layui-icon-reduce-circle |
| `refresh` | `0xE669` | layui-icon-refresh |
| `refresh_1` | `0xE666` | layui-icon-refresh-1 |
| `refresh_3` | `0xE9AA` | layui-icon-refresh-3 |
| `release` | `0xE609` | layui-icon-release |
| `reply_fill` | `0xE611` | layui-icon-reply-fill |
| `ic_return` | `0xE65C` | layui-icon-return |
| `right` | `0xE602` | layui-icon-right |
| `rmb` | `0xE65E` | layui-icon-rmb |
| `rss` | `0xE808` | layui-icon-rss |
| `screen_full` | `0xE622` | layui-icon-screen-full |
| `screen_restore` | `0xE758` | layui-icon-screen-restore |
| `search` | `0xE615` | layui-icon-search |
| `senior` | `0xE674` | layui-icon-senior |
| `service` | `0xE626` | layui-icon-service |
| `set` | `0xE716` | layui-icon-set |
| `set_fill` | `0xE614` | layui-icon-set-fill |
| `set_sm` | `0xE620` | layui-icon-set-sm |
| `share` | `0xE641` | layui-icon-share |
| `shrink_right` | `0xE668` | layui-icon-shrink-right |
| `slider` | `0xE714` | layui-icon-slider |
| `snowflake` | `0xE6B1` | layui-icon-snowflake |
| `sound` | `0xE69D` | layui-icon-sound（2.13.9 新增） |
| `speaker` | `0xE645` | layui-icon-speaker |
| `spread_left` | `0xE66B` | layui-icon-spread-left |
| `star` | `0xE600` | layui-icon-star |
| `star_fill` | `0xE658` | layui-icon-star-fill |
| `subtraction` | `0xE67E` | layui-icon-subtraction |
| `success` | `0xE697` | layui-icon-success |
| `survey` | `0xE6B2` | layui-icon-survey |
| `table` | `0xE62D` | layui-icon-table |
| `tabs` | `0xE62A` | layui-icon-tabs |
| `ic_template` | `0xE663` | layui-icon-template |
| `template_1` | `0xE656` | layui-icon-template-1 |
| `templeate_1` | `0xE630` | layui-icon-templeate-1 |
| `test` | `0xE692` | layui-icon-test |
| `theme` | `0xE66A` | layui-icon-theme |
| `time` | `0xE68D` | layui-icon-time |
| `tips` | `0xE702` | layui-icon-tips |
| `tips_fill` | `0xEB2E` | layui-icon-tips-fill |
| `top` | `0xE604` | layui-icon-top |
| `transfer` | `0xE691` | layui-icon-transfer |
| `tread` | `0xE6C5` | layui-icon-tread |
| `tree` | `0xE62E` | layui-icon-tree |
| `triangle_d` | `0xE625` | layui-icon-triangle-d |
| `triangle_r` | `0xE623` | layui-icon-triangle-r |
| `unlink` | `0xE64D` | layui-icon-unlink |
| `up` | `0xE619` | layui-icon-up |
| `upload` | `0xE67C` | layui-icon-upload |
| `upload_circle` | `0xE62F` | layui-icon-upload-circle |
| `upload_drag` | `0xE681` | layui-icon-upload-drag |
| `user` | `0xE770` | layui-icon-user |
| `username` | `0xE66F` | layui-icon-username |
| `util` | `0xE631` | layui-icon-util |
| `vercode` | `0xE679` | layui-icon-vercode |
| `video` | `0xE6ED` | layui-icon-video |
| `voice` | `0xE688` | layui-icon-voice |
| `water` | `0xE636` | layui-icon-water |
| `website` | `0xE7AE` | layui-icon-website |
| `wifi` | `0xE7E0` | layui-icon-wifi |
| `windows` | `0xE67F` | layui-icon-windows |


### `ui/ui/badge.h`

**调用示例**：
```cpp
auto* b = new ui::Badge(0, 0, "新");      // 默认徽章
b->set_theme(&theme);
b->add_class("layui-badge layui-bg-orange");   // rim 描边用 layui-badge-rim，dot 用 layui-badge-dot
parent->add_child(b);
```

**依赖 include**：`ui/ui/widget.h`、`string`

**类 `Badge`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Badge(float x, float y, const char* text)` |  |
| `void set_text(const char* text)` |  |
| `const std::string& text() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `float preferred_width() const` | 内容自适应宽度（文本宽 + 左右内边距；dot 为 8px） |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/blockquote.h`

**调用示例**：
```cpp
auto* q = new ui::BlockQuote(0, 0, 600, 60, "注意：引用提示文字");
q->set_font(&font);
q->set_theme(&theme);
q->set_wrap(true);
parent->add_child(q);
```

**依赖 include**：`ui/ui/widget.h`、`string`

**类 `BlockQuote`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `BlockQuote(float w, float h, const char* text, bool nm = false)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_text(const char* text)` |  |
| `float preferred_height() const` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/breadcrumb.h`

**调用示例**：
```cpp
auto* bc = new ui::Breadcrumb(0, 0, 600, 30);
bc->set_font(&font);
bc->set_theme(&theme);
bc->add_item("首页");
bc->add_item("组件");
bc->add_item("当前", /*active=*/true);
parent->add_child(bc);
```

**依赖 include**：`ui/ui/widget.h`、`string`、`vector`

**类 `Breadcrumb`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Breadcrumb(float x, float y, float w, float h)` |  |
| `void add_item(const char* text)` | 追加一个链接项（灰色，hover 变 #16b777） |
| `void add_current(const char* text)` | 追加当前页（cite，#5f5f5f） |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_separator(const char* sep)` |  |
| `float preferred_height() const` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Item`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string text;` |  |
| `bool current = false;` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/button.h`

**调用示例**：
```cpp
auto* b = new ui::Button(0, 0, 120, 40, "提交");
b->set_font(&font);
b->set_theme(&theme);
b->add_class("layui-btn layui-btn-danger layui-btn-lg");  // 变体/尺寸类名
b->set_on_click([] { ui::log_info("clicked"); });
parent->add_child(b);
```

**依赖 include**：`ui/ui/widget.h`、`cstdint`、`functional`、`string`

**类 `Button`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Button(float x, float y, float w, float h, const char* text)` |  |
| `void set_text(const char* text)` |  |
| `const std::string& text() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_icon(const char* icon_name)` | layui 图标按钮：图标类名（如 "left"，即 layui-icon-left 去前缀） |
| `void set_icon_right(bool on)` | 图标放文本右侧（镜像「按钮 <i class="layui-icon-down">」文字在前语义）； 默认 false = 图标居左（既有行为不变） |
| `bool icon_right() const` |  |
| `void set_badge(const char* text)` | 按钮内嵌徽章（layui-badge 红底白字，内嵌在文本右缘，如 `<button>查看消息<span layui-badge>1</span>`） 叠加 layui-bg-* 可换底（如 layui-bg-gray / layui-bg-orange）；dot 为 8px 圆点 |
| `void set_dot_badge()` |  |
| `void set_group_role(int role)` | 按钮组角色（ButtonGroup 自动维护）：0=普通，1=首，2=中间，3=尾 |
| `int group_role() const` |  |
| `void set_on_click(std::function<void()> cb)` | 松开瞬间若仍在按钮内触发一次 |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_key_down(int key, int mods) override` | Enter/Space 触发点击 |
| `float preferred_height() const` | layui 尺寸类名（lg/sm/xs）返回对应高度，否则 bounds_.h |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/card.h`

**调用示例**：
```cpp
auto* c = new ui::Card(0, 0, 400, 200);
c->set_font(&font);
c->set_theme(&theme);
c->set_title("卡片标题");
ui::Label* body = c->body();            // body 为内容容器（布局/添加子控件）
body->add_child(new ui::Label(0, 0, 0, 0, "卡片内容"));
parent->add_child(c);
```

**依赖 include**：`ui/ui/widget.h`、`string`、`vector`

**类 `Card`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Card(float x, float y, float w, float h)` |  |
| `void set_header(const char* text)` |  |
| `const std::string& header() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_spacing(float spacing)` | 子控件间距（body 内） |
| `void set_outline(const Vec4& color, float thickness = 1.f)` | 可选描边（layui 图标列表等 1px 边框卡片）；a<=0 关闭 |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/carousel.h`

**调用示例**：
```cpp
auto* car = new ui::Carousel(0, 0, 800, 300);
car->set_font(&font);
car->set_theme(&theme);
car->add_item("图一");
car->add_item("图二");
car->add_item("图三");
parent->add_child(car);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Carousel`**
（继承 `: Widget`）

> 轮播动画类型（layui anim）

| 方法 | 功能说明 |
| --- | --- |
| `Carousel(float x, float y, float w, float h)` |  |
| `void add_slide(const Vec4& bg, const char* title)` | 添加一页：背景色 + 标题 |
| `bool add_image_slide(const char* path)` | 添加图片页（layui 填充图片）：加载路径内的图片作为该页内容 |
| `void clear()` |  |
| `int count() const` |  |
| `int current() const` |  |
| `void set_index(int idx)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `void set_anim(CarouselAnim anim)` | layui 配置项 |
| `CarouselAnim anim() const` |  |
| `void set_arrow(CarouselArrow arrow)` |  |
| `CarouselArrow arrow() const` |  |
| `void set_indicator(CarouselIndicator ind)` |  |
| `CarouselIndicator indicator() const` |  |
| `void set_autoplay(bool on)` |  |
| `bool autoplay() const` |  |
| `void set_interval(float ms)` | interval：毫秒（layui interval，默认 3000） |
| `float interval() const` | 毫秒 |
| `void set_size(float w, float h)` |  |
| `void set_auto_play(bool on, float seconds = 3.f)` | 旧 API 兼容（UI-089 前）：秒为单位开关自动轮播 |
| `void next()` |  |
| `void prev()` |  |
| `void advance(float dt)` | 旧 API 兼容：应用层每帧调用（自动轮播/动画计时） |
| `void on_advance(float dt) override` |  |
| `void on_layout() override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Slide`**

| 数据成员 | 说明 |
| --- | --- |
| `Vec4 bg{0.9f, 0.9f, 0.9f, 1.0f};` |  |
| `std::string title;` |  |
| `Texture* image = nullptr;` | 图片页（可选） |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void set_current_now(int idx)` |  |
| `void start_transition(int from, int to)` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/checkbox.h`

**调用示例**：
```cpp
auto* cb = new ui::CheckBox(0, 0, 120, 28, "阅读协议");
cb->set_font(&font);
cb->set_theme(&theme);
cb->set_checked(true);
cb->set_on_change([](bool on) { ui::log_info(on ? "勾选" : "取消"); });
parent->add_child(cb);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `CheckBox`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `CheckBox(float x, float y, float w, float h, const char* text)` |  |
| `void set_text(const char* text)` |  |
| `void set_checked(bool checked)` |  |
| `bool checked() const` |  |
| `void toggle()` |  |
| `void set_indeterminate(bool ind)` | 半选态（layui indeterminate）：只可动态设置，方框绿框白底 + 中间横线 |
| `bool indeterminate() const` |  |
| `void set_skin(Skin skin)` |  |
| `Skin skin() const` |  |
| `void set_on_change(std::function<void(bool)> cb)` | 选中态变化（点击/键盘触发） |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_custom_content(Widget* content)` | lay-checkbox 绑定：设置紧随复选框的自定义内容控件（layui <div lay-checkbox>）。 内容控件拥有权归 CheckBox（析构/clear_children 时释放）；点击内容区域同样切换选中态。 典型用途：自定义标题模板（链接/图标）、lay-skin=none 卡片风格多选组件。 注意：custom_ 不作为子节点（避免双重绘制/布局），由 CheckBox 在 on_layout/on_draw 中直接管理（Dropdown 同款模式）。 |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_key_down(int key, int mods) override` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/circle.h`

**调用示例**：
```cpp
auto* c = new ui::Circle(0, 0, 120, 120);
c->set_theme(&theme);
c->set_value(0.72f);                    // 0..1 填充比例
parent->add_child(c);
```

**依赖 include**：`ui/core/math.h`、`ui/ui/widget.h`、`string`

**类 `Circle`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Circle(float size, const char* text)` |  |
| `void set_font(Font* font)` |  |
| `void set_color(const Vec4& c)` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/code.h`

**调用示例**：
```cpp
auto* code = new ui::Code(0, 0, 600, 300);
code->set_font(&font);
code->set_theme(&theme);
code->set_lang("C++");
code->add_line("int main() { return 0; }");
parent->add_child(code);
code->clear();                          // 清空重填
```

**依赖 include**：`ui/ui/widget.h`、`string`、`vector`

**类 `Code`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Code(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_title(const char* title)` | 标题条左侧文本（lay-title；空则不画） |
| `void set_lang(const char* lang)` | 标题条右侧语言标签（lay-lang） |
| `void set_skin(Skin skin)` |  |
| `void set_max_height(float h)` | 固定最大高度（lay-height）：内容超高时内部滚动；0 表示自适应内容高度 |
| `void set_code(const char* code)` | 多行代码（\n 分隔） |
| `void clear()` |  |
| `void add_line(const char* line)` |  |
| `bool empty() const` |  |
| `bool on_mouse_scroll(float x, float y, float dx, float dy) override` |  |
| `float preferred_height() const` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |
| `void draw(Renderer& renderer) override` |  |


### `ui/ui/collapse.h`

**调用示例**：
```cpp
auto* col = new ui::Collapse(0, 0, 600, 40);
col->set_font(&font);
col->set_theme(&theme);
col->add_item("设计哲学", ui::Collapse::kUnfolded);
col->add_item("从心开始");
col->set_on_change([](int idx, bool open) {});
parent->add_child(col);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Collapse`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Collapse(float x, float y, float w, float h)` |  |
| `void add_item(const char* title)` |  |
| `void set_active(int index)` | 点按展开/收起该项 |
| `int active() const` | 手风琴：当前展开项；非手风琴：最后操作项 |
| `bool is_open(int index) const` | 该项当前是否展开 |
| `void set_accordion(bool on)` | 手风琴（默认 true） |
| `bool accordion() const` |  |
| `int open_count() const` | 当前展开项数 |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Item`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `size_t child_begin = 0;` | children_ 中本项内容起始下标 |
| `float y0 = 0.f, y1 = 0.f;` | 标题条 y 范围（点击命中） |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void layout_items()` |  |
| `bool is_open_impl(int index) const` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/colorpicker.h`

**调用示例**：
```cpp
auto* cp = new ui::ColorPicker(0, 0, 160, 36);
cp->set_theme(&theme);
cp->set_color(ui::Vec4{0.09f, 0.72f, 0.47f, 1.0f});
cp->set_on_change([](const ui::Vec4& c) { ui::log_info("选色"); });
parent->add_child(cp);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `ColorPicker`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `ColorPicker(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_color(const Vec4& c)` | 默认色：任意颜色转为内部 RGBA + HSV |
| `const Vec4& color() const` |  |
| `void set_format(ColorFormat fmt)` | —— layui colorpicker 选项 —— hex / rgb（alpha 时自动 rgba） |
| `void set_alpha(bool on)` | 开启透明度滑块 |
| `void set_predefine(bool on)` | 开启预定义颜色 |
| `void set_colors(const std::vector<Vec4>& cols)` | 自定义预定义色（需 predefine:true） |
| `void set_size(ColorPickerSize s)` | 颜色框尺寸 lg / sm / xs |
| `void set_clearable(bool on)` | 清除按钮（默认 true） |
| `void set_on_change(std::function<void(const Vec4&)> cb)` | —— 回调 —— 取色中 |
| `void set_on_done(std::function<void(const Vec4&)> cb)` | 确认/清除 |
| `float preferred_height() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/combobox.h`

**调用示例**：
```cpp
auto* cb = new ui::ComboBox(0, 0, 200, 36, "请选择");
cb->set_font(&font);
cb->set_theme(&theme);
cb->add_item("北京", [] { ui::log_info("选中北京"); });
cb->add_item("上海", nullptr);
parent->add_child(cb);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `ComboBox`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `ComboBox(float x, float y, float w, float h)` |  |
| `void add_option(const char* text)` |  |
| `void set_options(const std::vector<std::string>& options)` | 就地重载全部选项（UI-121）：清空并递归释放旧选项按钮后重建； 调用方无需再「delete+new 销毁重建」整个下拉框——重建孤儿控件（可见性/坐标 失管）是幽灵下拉类缺陷的根源。重载非用户选择事件：不触发 on_change， 原选中项若仍在新列表范围内则保持，否则复位为未选中（显示回退到首项文本）。 |
| `void select(int index)` | 选中并触发 on_change |
| `void close()` |  |
| `void open()` | 展开下拉面板（UI-143：Table 行内下拉等外部调用方需要显式展开） |
| `int selected() const` |  |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_layout() override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_key_down(int key, int mods) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/countdown_text.h`

**调用示例**：
```cpp
auto* cd = new ui::CountdownText(0, 0, 0, 0, "距离结束还有 10 秒");
cd->set_font(&font);
cd->set_theme(&theme);
cd->start();                            // 每帧自动走秒
parent->add_child(cd);
```

**依赖 include**：`ui/ui/widget.h`、`chrono`

**类 `CountdownText`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `CountdownText(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_total(float seconds)` |  |
| `void restart()` |  |
| `float preferred_height() const` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/dropdown.h`

**调用示例**：
```cpp
auto* dd = new ui::Dropdown(0, 0, 88, 38, "周边");
dd->set_font(&font);
dd->set_theme(&theme);
dd->add_item("官方网站", [] {});
dd->add_separator();
dd->add_item("开源地址", [] {});
parent->add_child(dd);
```

**依赖 include**：`ui/ui/widget.h`、`chrono`、`functional`、`string`

**类 `Dropdown`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Dropdown(float x, float y, float w, float h, const char* title = )` |  |
| `void add_item(const char* title, std::function<void()> cb = {})` | ---- 菜单内容（内部 Menu，默认白底弹板）---- |
| `void add_separator()` |  |
| `void add_group(const char* title, bool allow_spread = true)` |  |
| `void add_parent(const char* title)` |  |
| `void end_group()` |  |
| `void set_trigger_style(DropdownStyle s)` | ---- 触发器 / 外观 ---- |
| `void set_trigger_event(DropdownTrigger t)` |  |
| `void set_align(DropdownAlign a)` |  |
| `void set_menu_width(float w)` |  |
| `void set_region(const char* placeholder)` | 将触发器绘制为一块灰色区域并居中显示提示文字（常用于右键菜单占位区） |
| `void set_custom_content(Widget* content)` | 自定义弹出内容（非空则替代默认菜单） |
| `void set_popup_style(const Vec4& bg, const Vec4& hover, const Vec4& text)` | 自定义弹出面板配色（bg / hover / 文字） |
| `void show()` | ---- 显隐（Manual / 外部触发 / 表格行）---- |
| `void hide()` |  |
| `void show_at(float sx, float sy)` |  |
| `bool opened() const` |  |
| `void set_on_select(std::function<void(const char*)> cb)` | 选中项回调（title 文本） |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |


### `ui/ui/fieldset.h`

**调用示例**：
```cpp
auto* fs = new ui::FieldsetTitle(0, 0, 600, 40, "区块标题");
fs->set_font(&font);
fs->set_theme(&theme);
parent->add_child(fs);
```

**依赖 include**：`ui/ui/widget.h`、`string`

**类 `FieldsetTitle`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `FieldsetTitle(float w, float h, const char* text)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Fieldset`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Fieldset(float w, float h, const char* legend)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `float preferred_height() const` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/grid.h`

**调用示例**：
```cpp
ui::Grid* row = new ui::Grid(0, 0, 800, 40, 12);   // 12 列等宽基础
row->set_theme(&theme);
auto* cell = new ui::Panel(0, 0, 0, 0);
cell->add_class("layui-col-md4 layui-col-md-offset2");  // span 4 + 偏移 2
row->add_child(cell);
parent->add_child(row);
```

**依赖说明**：前置：`set_theme()`；子控件带 `layui-col-*` 类时自动进入 12 列栅格模式；跨断点演示见 layui_test grid_page。

**依赖 include**：`ui/ui/widget.h`

**类 `Grid`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Grid(float x, float y, float w, float h, int cols)` |  |
| `void set_cols(int cols)` |  |
| `void set_row_height(float h)` |  |
| `void set_spacing(float s)` |  |
| `void set_padding(float p)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_span_cols(int cols)` | layui 栅格：12 列基准下解析子控件 layui-col-* 类 |
| `void set_breakpoint(int bp)` | 断点：0=xs 1=sm 2=md 3=lg；<0 时按自身宽度自动推断（内容区宽度） |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/hbox.h`

**调用示例**：
```cpp
auto* box = new ui::HBox(0, 0, 800, 60);
box->set_spacing(10.f);
box->set_padding(8.f);
box->add_child(new ui::Button(0, 0, 100, 36, "A"));
box->add_child(new ui::Button(0, 0, 100, 36, "B"));
parent->add_child(box);
```

**依赖 include**：`ui/ui/widget.h`

**类 `HBox`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `HBox(float x, float y, float w, float h)` |  |
| `void set_spacing(float spacing)` |  |
| `void set_padding(float padding)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_valign(Valign valign)` |  |
| `void set_fill(const Vec4& color)` | 覆盖背景填充色（默认 theme->button_normal，如 layui-bg-gray 灰底 #fafafa）；传 a<0 不覆盖 |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |
| `float preferred_height() const` | h<=0（自适应）：取可见子控件 preferred_height 最大值 + 2*padding； h>0 时显式高度优先（返回 bounds_.h）。使 HBox 可直接投入 ScrollBox/VBox 流式排布。 |


### `ui/ui/hline.h`

**调用示例**：
```cpp
auto* line = new ui::HLine(0, 0, 800, 1);      // 细分隔线
line->set_color(theme.border);
parent->add_child(line);
```

**依赖 include**：`ui/core/math.h`、`ui/ui/widget.h`

**类 `HLine`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `HLine(float w, float h, const Vec4& color)` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/icon.h`

**调用示例**：
```cpp
auto* ic = new ui::Icon(0, 0, 18.f, ui::layui_icon::ic_about);
ic->set_font(&font);
ic->set_color(theme.success);
parent->add_child(ic);
```

**依赖 include**：`ui/ui/widget.h`

**类 `Icon`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Icon(float x, float y, float w, float h, const char* icon_name = )` |  |
| `void set_icon(const char* name)` | 直接设图标名（不含 layui-icon- 前缀），找不到则清空 |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_color(const Vec4& c)` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/image.h`

**依赖 include**：`ui/ui/widget.h`、`memory`、`string`

**类 `Image`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Image(float x, float y, float w, float h)` |  |
| `bool load_from_file(const char* path)` | 加载图片文件（png/jpg/bmp/webp 等），成功返回 true； 构造时未显式给显示尺寸（w/h <= 0）则用图片原始尺寸作为内容高度。 裸文件名（无路径分隔符）先按 CWD 解析，miss 时回退 exe 目录同名文件 （资产定位不依赖工作目录，UI-048）；全部 miss 返回 false 并 log 一次。 |
| `void set_lay_src(const char* path)` | 设置懒加载源（lay-src 语义）：仅在图片进入当前可视屏（Renderer::current_scissor） 时真正 load_from_file。便于信息流/图片懒加载按需取图；进入可视区即加载一次。 |
| `bool loaded() const` |  |
| `float preferred_height() const` | 内容自适应高度：返回显示高度（未加载且未显式指定显示尺寸返回 0； 懒加载图片构造时显式给 w/h，则加载前也预留该空间保证列表布局稳定——UI-093） |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/label.h`

**调用示例**：
```cpp
auto* label = new ui::Label(0, 0, 600, 0, "多行\n文本");
label->set_font(&font);
label->set_theme(&theme);
label->set_color(theme.text);
label->set_wrap(true);                   // 超宽自动换行
parent->add_child(label);
```

**依赖 include**：`ui/ui/widget.h`、`string`、`vector`

**类 `Label`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Label(float x, float y, float w, float h, const char* text)` |  |
| `void set_text(const char* text)` |  |
| `const std::string& text() const` |  |
| `void set_wrap(bool wrap)` | 开启按控件宽度自动换行（bounds_.w 为行宽） |
| `bool wrap() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_color(const Vec4& color)` |  |
| `void set_align(int align)` | 0=左对齐 1=水平居中 2=右对齐 |
| `void set_text_scale(float s)` | 字号缩放：相对 Font 烘培像素号的倍率（1 = 原大），行高/换行/度量随缩放； 图集 2x 过采样，≤2x 清晰，更大倍率发虚 |
| `float text_scale() const` |  |
| `void set_bold(bool b)` | 仿粗体：字形沿 x 错位叠加一次（偏移 ≈ 字号/16，随缩放同步） |
| `bool bold() const` |  |
| `float preferred_height() const` | 文本行高（ascent - descent）；换行时为行数 × 行高 |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/laydate.h`

**调用示例**：
```cpp
auto* date = new ui::Laydate(0, 0, 220, 36, "日期");
date->set_font(&font);
date->set_theme(&theme);
date->set_on_change([](const char* v) { ui::log_info("选中 %s", v); });
parent->add_child(date);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Laydate`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Laydate(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` | ---- 配置（参考 layui laydate.render({...}) 参数） ---- |
| `void set_theme(const Theme* theme)` |  |
| `void set_type(Type t)` | 类型：year / month / date(默认) / time / datetime |
| `void set_type(const char* t)` |  |
| `Type type() const` |  |
| `void set_date(int y, int m, int d)` | 初始日期/时间 |
| `void set_time(int h, int mi, int s)` |  |
| `void set_value(const char* v)` | 解析 "yyyy-MM-dd" / "HH:mm:ss" 等 |
| `void set_range(bool v)` | 范围选择 |
| `bool range() const` |  |
| `void set_range_separator(const char* s)` |  |
| `void set_format(const char* f)` | 自定义格式（默认根据 type 自动） |
| `void set_min(const char* v)` | 限定范围 |
| `void set_max(const char* v)` |  |
| `void set_min_offset(int days)` | 相对今天偏移（天）：等效 min/max 的 -7/+7 形式，与 set_min/set_max 互斥时后者优先 |
| `void set_max_offset(int days)` |  |
| `void set_calendar(bool v)` | 公历节日 |
| `void set_marks(const std::vector<Mark>& m)` |  |
| `void set_theme_style(const char* t)` | 主题：""(默认) / "molv"(墨绿) / "grid"(格子) / "#rrggbb"(自定义) |
| `void set_show_bottom(bool v)` | 底部栏 |
| `void set_btns(const std::vector<std::string>& b)` |  |
| `void set_lang(const char* l)` | 国际化 |
| `void set_on_change(std::function<void(int, int, int)> cb)` | ---- 回调 ---- |
| `void set_on_done(std::function<void(const char*, int, int, int, int, int, int)> cb)` |  |
| `void set_on_ready(std::function<void(int, int, int)> cb)` |  |
| `void next_month()` | ---- 便捷 API ---- |
| `void prev_month()` |  |
| `void next_year()` |  |
| `void prev_year()` |  |
| `void select_today()` |  |
| `std::string value() const` | 当前选中值的格式化字符串（用于写回输入框） |
| `bool on_mouse_down(float x, float y, int button) override` | ---- Widget 覆写 ---- |
| `void on_draw(Renderer& renderer) override` |  |

| 数据成员 | 说明 |
| --- | --- |
| `std::string text;` | 自定义标注（"MM-DD": "文字" / "YYYY-MM-DD": "文字"） |

**类 `Mark`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string key;` | 自定义标注（"MM-DD": "文字" / "YYYY-MM-DD": "文字"） |
| `std::string text;` | 自定义标注（"MM-DD": "文字" / "YYYY-MM-DD": "文字"） |

**类 `Colors`**

| 数据成员 | 说明 |
| --- | --- |
| `Vec4 bg, border, header_bg, header_text;` |  |
| `Vec4 weekday_text, weekend_text;` |  |
| `Vec4 day_text, day_hover_bg, day_selected_bg, day_selected_text;` |  |
| `Vec4 today_border, mark_bg, mark_text;` |  |
| `Vec4 footer_bg, footer_btn_text, footer_divider;` |  |
| `Vec4 range_bg;` |  |
| `Vec4 time_col_bg, time_item_text, time_item_selected_bg;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `Laydate(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` | ---- 配置（参考 layui laydate.render({...}) 参数） ---- |
| `void set_theme(const Theme* theme)` |  |
| `void set_type(Type t)` | 类型：year / month / date(默认) / time / datetime |
| `void set_type(const char* t)` |  |
| `Type type() const` |  |
| `void set_date(int y, int m, int d)` | 初始日期/时间 |
| `void set_time(int h, int mi, int s)` |  |
| `void set_value(const char* v)` | 解析 "yyyy-MM-dd" / "HH:mm:ss" 等 |
| `void set_range(bool v)` | 范围选择 |
| `bool range() const` |  |
| `void set_range_separator(const char* s)` |  |
| `void set_format(const char* f)` | 自定义格式（默认根据 type 自动） |
| `void set_min(const char* v)` | 限定范围 |
| `void set_max(const char* v)` |  |
| `void set_min_offset(int days)` | 相对今天偏移（天）：等效 min/max 的 -7/+7 形式，与 set_min/set_max 互斥时后者优先 |
| `void set_max_offset(int days)` |  |
| `void set_calendar(bool v)` | 公历节日 |
| `void set_marks(const std::vector<Mark>& m)` |  |
| `void set_theme_style(const char* t)` | 主题：""(默认) / "molv"(墨绿) / "grid"(格子) / "#rrggbb"(自定义) |
| `void set_show_bottom(bool v)` | 底部栏 |
| `void set_btns(const std::vector<std::string>& b)` |  |
| `void set_lang(const char* l)` | 国际化 |
| `void set_on_change(std::function<void(int, int, int)> cb)` | ---- 回调 ---- |
| `void set_on_done(std::function<void(const char*, int, int, int, int, int, int)> cb)` |  |
| `void set_on_ready(std::function<void(int, int, int)> cb)` |  |
| `void next_month()` | ---- 便捷 API ---- |
| `void prev_month()` |  |
| `void next_year()` |  |
| `void prev_year()` |  |
| `void select_today()` |  |
| `std::string value() const` | 当前选中值的格式化字符串（用于写回输入框） |
| `bool on_mouse_down(float x, float y, int button) override` | ---- Widget 覆写 ---- |
| `void on_draw(Renderer& renderer) override` |  |
| `private: void month_info(int* first_weekday, int* days) const` |  |
| `bool is_leap(int y) const` |  |
| `int days_in_month(int y, int m) const` |  |
| `void draw_header(Renderer& r)` | 当前视图绘制 |
| `void draw_weekday_row(Renderer& r)` |  |
| `void draw_date_grid(Renderer& r)` |  |
| `void draw_month_grid(Renderer& r)` |  |
| `void draw_year_grid(Renderer& r)` |  |
| `void draw_time_panel(Renderer& r)` |  |
| `void draw_footer(Renderer& r)` |  |
| `bool hit_header(float x, float y) const` | 命中测试 |
| `bool hit_grid(float x, float y, int& out_row, int& out_col) const` |  |
| `bool hit_footer(float x, float y, int& out_btn) const` |  |
| `bool hit_time(float x, float y, int& out_col, int& out_idx) const` |  |
| `Colors colors() const` |  |
| `bool date_selectable(int y, int m, int d) const` | 是否可选中：受 min/max（含相对今天偏移）限制 |
| `std::string format_value() const` | 格式化当前选中为字符串 |

**枚举 `Type`**

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `TypeDate` | `` |  |
| `TypeYear` | `` |  |
| `TypeMonth` | `` |  |
| `TypeTime` | `` |  |
| `TypeDateTime` | `` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/laypage.h`

**调用示例**：
```cpp
auto* page = new ui::Laypage(0, 0, 400, 36);
page->set_font(&font);
page->set_theme(&theme);
page->set_pages(20);
page->set_on_change([](int p) { ui::log_info("第 %d 页", p); });
parent->add_child(page);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Laypage`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Laypage(float x, float y, float w, float h)` |  |
| `void set_total(int total)` | ---- 数据（兼容旧接口：set_total = 页数；新增 set_count = 数据总数，页数=ceil(count/limit)）---- 直接设置页数（Table 内部使用） |
| `void set_count(int count)` | 设置数据总数，页数 = ceil(count/limit) |
| `void set_limit(int limit)` | 每页条数（默认 10），设置后重算页数 |
| `int limit() const` |  |
| `int count() const` |  |
| `void set_layout(std::vector<Item> layout)` | ---- 布局 ---- 布局项顺序（不含 first/last；它们固定在首尾外侧）。默认 [Prev, Page, Next] |
| `void set_layout_str(const std::vector<std::string>& items)` | 以字符串数组设置布局项（"count"/"prev"/"page"/"next"/"limit"/"refresh"/"skip"） |
| `void set_show_first(bool on)` | ---- 首页/尾页 ---- |
| `void set_show_last(bool on)` |  |
| `void set_first_text(const char* t)` |  |
| `void set_last_text(const char* t)` |  |
| `void set_prev_text(const char* t)` |  |
| `void set_next_text(const char* t)` |  |
| `void set_theme_color(const Vec4& c)` | ---- 主题 ---- |
| `void set_limits(std::vector<int> limits)` | ---- 每页条数选择项（limit 下拉）---- |
| `void set_page(int page)` | ---- 回调 ---- 0 基 |
| `int page() const` |  |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `void set_on_jump(std::function<void(int, int)> cb)` | layui jump：切页 / 切条数时回调 (curr, limit)，curr 为 1 基 |
| `void set_on_refresh(std::function<void()> cb)` | refresh 按钮回调 |
| `void set_on_limit_change(std::function<void(int)> cb)` | limit 条数变化回调（1 基页参数为切后的页数） |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `ItemRect`**

| 数据成员 | 说明 |
| --- | --- |
| `Item type;` |  |
| `float x, w;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `bool hit_rect(Item type, float x) const` | 上一页/下一页是否命中（由 on_mouse_down 用 item_rects_ 求交） |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/laytpl.h`

**调用示例**：
```cpp
ui::LayTplValue data;
ui::laytpl_parse_json("{\"title\":\"Layui\",\"list\":[{\"m\":1}]}", data);
ui::LayTpl tpl("{{ d.title }} / {{# layui.each(d.list, function(i,it){ }}{{ it.m }}{{# }); }}");
std::string html = tpl.render(data);
```

**依赖 include**：`functional`、`memory`、`string`、`vector`

**类 `LayTplValue`**

| 方法 | 功能说明 |
| --- | --- |
| `LayTplValue() = default` |  |
| `static LayTplValue make_null()` |  |
| `static LayTplValue make_bool(bool v)` |  |
| `static LayTplValue make_number(double v)` |  |
| `static LayTplValue make_string(const std::string& v)` |  |
| `static LayTplValue make_array()` |  |
| `static LayTplValue make_object()` |  |
| `Type type() const` |  |
| `bool is_object() const` |  |
| `bool is_array() const` |  |
| `bool is_string() const` |  |
| `const LayTplValue* get(const std::string& key) const` | 对象：按键取值 / 赋值 |
| `void set(const std::string& key, const LayTplValue& v)` |  |
| `bool has(const std::string& key) const` |  |
| `void push(const LayTplValue& v)` | 数组：追加 / 按序取值 / 长度 |
| `const LayTplValue* at(size_t i) const` |  |
| `size_t size() const` |  |
| `double number() const` | 标量读取 |
| `const std::string& str() const` |  |
| `bool boolean() const` |  |
| `std::string to_string() const` | 转为输出文本（{{ }} 非转义输出时的字符串化） |
| `bool truthy() const` | JS 真值判断（""、0、null、false、空数组/对象 为假） |

**类 `Obj`**

| 数据成员 | 说明 |
| --- | --- |
| `std::vector<std::pair<std::string, LayTplValue>> items;` |  |

**类 `LayTpl`**

| 方法 | 功能说明 |
| --- | --- |
| `LayTpl(const std::string& tpl, std::string* err = nullptr)` |  |
| `std::string render(const LayTplValue& data)` | 渲染：返回渲染后的字符串；失败返回空串（此时可查 last_error()）。 |
| `static void config(const char* open, const char* close)` | 设置自定义分隔符（open / close），例如 "<%" "%>"。影响后续新构造的模板。 |
| `const std::string& last_error() const` |  |
| `bool ok() const` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `bool laytpl_parse_json(const char* json, LayTplValue& out, std::string* err = nullptr)` | 从 JSON 文本解析出数据节点（laytpl.html 的 data 文本框即 JSON）。 支持对象 / 数组 / 字符串（含转义）/ 数字 / 布尔 / null。 成功返回 true，失败返回 false 且 *err 携带原因。 |


### `ui/ui/menubar.h`

**调用示例**：
```cpp
auto* mb = new ui::MenuBar(0, 0, 600, 36);
mb->set_font(&font);
mb->set_theme(&theme);
mb->add_menu("菜单", { {"选项A", [] {}}, {"选项B", nullptr} });
parent->add_child(mb);
```

**依赖 include**：`ui/ui/widget.h`、`functional`

**类 `MenuBar`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `MenuBar(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void add_menu(const char* title)` | 追加一个菜单（追加到末尾）；此后 add_item 都添加到该菜单 |
| `void add_item(const char* title, std::function<void()> cb)` | 给最后一个菜单追加菜单项；点击执行 cb 并关闭弹出 |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Menu`**

| 数据成员 | 说明 |
| --- | --- |
| `Button* trigger = nullptr;` |  |
| `VBox* popup = nullptr;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void toggle_menu(Menu& m)` |  |
| `void close_all_menus()` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/menu.h`

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Menu`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Menu(float x, float y, float w, float h)` |  |
| `void add_item(const char* title, std::function<void()> cb = nullptr)` |  |
| `void add_item_sub(const char* title, const char* subtitle, std::function<void()> cb = nullptr)` | 带英文副标题的菜单项（新镜像 docs/2 侧栏：中文主标题 + 灰色英文副标题） |
| `void add_divider()` |  |
| `void add_group(const char* title, bool allow_spread = true)` |  |
| `void add_parent(const char* title)` |  |
| `void end_group()` |  |
| `void clear()` | 清空全部项（第二列子菜单按板块重建时复用同一控件） |
| `void set_checked()` |  |
| `void set_selected(int group_index, int item_index)` | 选中指定 (组序号, 组内子项序号)：清除其余项选中态，高亮该项（layui `.layui-menu-item-checked` 绿字） |
| `void set_badge(const char* text)` |  |
| `void set_dot_badge()` |  |
| `void set_large(bool large)` |  |
| `void set_font(Font*)` |  |
| `void set_theme(const Theme*)` |  |
| `void set_style_overrides(const Vec4& bg, const Vec4& border, const Vec4& hover, const Vec4& text)` | 自定义配色覆盖（bg/border/hover/文字），不设置则用主题默认 |
| `void set_on_item_click(std::function<void(const char*)> cb)` | 普通菜单项被点击时回调（title 文本），供宿主（如 Dropdown）做选中/收起 |
| `void set_on_group_click(std::function<void(const char*)> cb)` | 分组头被点击时回调（title 文本，spread 已切换后触发），供宿主联动收起/展开外部面板 |
| `float preferred_height() const` |  |

**类 `Item`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string text;` |  |
| `std::string subtitle;` | 英文副标题（新镜像 docs/2 侧栏），可空 |
| `std::function<void()> cb;` |  |
| `Type type = Plain;` |  |
| `int level = 0;` |  |
| `bool checked = false;` |  |
| `bool spread = true;` |  |
| `bool allow_spread = true;` |  |
| `bool has_badge = false;` |  |
| `bool has_dot = false;` |  |
| `std::string badge_text;` |  |
| `float y0 = 0.f, y1 = 0.f;` |  |
| `mutable int group_index = -1;` |  |
| `mutable bool visible = true;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void draw_item(Renderer&, Item&, int index)` |  |
| `void draw_sub_panel(Renderer&, int parent_index)` |  |
| `void layout_items()` |  |
| `void compute_visibility() const` |  |

**枚举 `Type`**

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `Plain` | `` |  |
| `Divider` | `` |  |
| `Group` | `` |  |
| `Parent` | `` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/navmenu.h`

**调用示例**：
```cpp
auto* nav = new ui::NavMenu(0, 0, 200, 400);
nav->set_font(&font);
nav->set_theme(&theme);
nav->add_group("基础");
nav->add_item("按钮", [] {});
nav->set_active(0);
nav->expand_all();
parent->add_child(nav);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `NavMenu`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `NavMenu(float x, float y, float w, float h)` |  |
| `void add_group(const char* title)` | 追加一级菜单；随后 add_item 的项挂到它下面 |
| `void clear_groups()` | 清空全部菜单（第二列子菜单按板块重建时复用同一控件） |
| `void add_item(const char* title, std::function<void()> cb = {})` | 给最后一个 group 追加菜单项（点击执行 cb） |
| `void add_separator()` | 给最后一个 group 追加分隔线（镜像 site-demo-table-nav 的 <hr> 分段，无回调不可点击） |
| `void set_active(int group)` | 设置激活的一级组（顶部 3px 指示条；点击一级项也会自动激活） |
| `void set_selected(int group, int item)` | 设置选中的子项（g 组 i 项，layui `.layui-this` 语义：绿底 #16b777 + 白字）。 点击子项也会自动选中（layui-nav-tree 行为）。 |
| `void expand_all()` | 展开所有一级组（demo 侧栏多组全展开）；点击一级项仍可单独折叠 |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Item`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string text;` |  |
| `std::function<void()> cb;` |  |
| `float y0 = 0.f, y1 = 0.f;` |  |
| `bool separator = false;` | 分隔线（<hr> 分段）：无文字不可点击 |

**类 `Group`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `std::vector<Item> items;` |  |
| `bool expanded = false;` |  |
| `float y0 = 0.f, y1 = 0.f;` |  |
| `float body_y0 = 0.f;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void layout_groups()` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/panel.h`

**调用示例**：
```cpp
auto* p = new ui::Panel(0, 0, 300, 200);
p->set_theme(&theme);
p->add_child(new ui::Label(0, 0, 0, 0, "面板内容"));
parent->add_child(p);
```

**依赖 include**：`ui/ui/widget.h`

**类 `Panel`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Panel(float x, float y, float w, float h)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_fill(const Vec4& color)` | 覆盖背景填充色（默认 theme->button_normal）；传 a<0 表示不覆盖 |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/panelwindow.h`

**调用示例**：
```cpp
auto* w = new ui::PanelWindow(0, 0, 400, 260, "标题");
w->set_font(&font);
w->set_theme(&theme);
w->set_on_close([] { ui::log_info("窗口关闭"); });
w->add_child(new ui::Label(0, 0, 0, 0, "窗口内容"));
parent->add_child(w);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `PanelWindow`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `PanelWindow(float x, float y, float w, float h, const char* title)` |  |
| `void set_title(const char* title)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_on_close(std::function<void()> cb)` | 关闭按钮回调（通常由使用者隐藏/销毁窗口） |
| `void set_title_visible(bool v)` | ---- UI-069 ---- 是否显示标题栏（layer title:false） |
| `void set_close_visible(bool v)` | 是否显示关闭按钮（layer closeBtn:false） |
| `void set_maxmin(bool v)` | 最大化/还原按钮（layer maxmin:true），标题栏右侧追加 ─ / □ |
| `void set_shade(float alpha)` | 底部遮罩 alpha（layer shade:0..1），0 表示无遮罩；遮罩覆盖整个视口绘制在窗口之后 |
| `void set_footer_buttons(const std::vector<std::string>& labels, int align, std::function<void(int)> cb)` | 底部按钮条：labels 按钮文本，align 0=左 1=中 2=右；点击回调（回调参数为按钮下标） |
| `void set_auto_close(float seconds)` | 自动关闭秒数（layer msg time），<=0 不自动关闭；需应用层每帧 root->advance(dt) |
| `float footer_height() const` | 客户区高度中按钮条预留（footer_h），供子控件布局时避让 |
| `Rect client_rect() const` | 客户区矩形（标题栏下方） |
| `void on_layout() override` |  |
| `void on_advance(float dt) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `ChildLayoutInfo`**

| 数据成员 | 说明 |
| --- | --- |
| `Widget* widget;` |  |
| `Rect rel;` | 相对客户区左上角的偏移 |


### `ui/ui/progress.h`

**调用示例**：
```cpp
auto* pg = new ui::Progress(0, 0, 500, 30);
pg->set_theme(&theme);
pg->add_class("layui-progress layui-progress-big");  // 大号进度条
pg->set_value(0.6f);
parent->add_child(pg);
```

**依赖 include**：`ui/ui/widget.h`

**类 `Progress`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Progress(float x, float y, float w, float h)` |  |
| `void set_value(float v)` | 进度 0..1 |
| `float value() const` |  |
| `void set_show_text(bool show)` | 是否显示进度文本（layui-progress-text；默认显示百分比 "40%"） |
| `void set_text_override(const char* text)` | 自定义显示文本（如分数 "5 / 10"），为空则回退到百分比。清空后传 nullptr。 |
| `const std::string& text_override() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/radiobutton.h`

**调用示例**：
```cpp
auto* rb = new ui::RadioButton(0, 0, 80, 28, "男");
rb->set_font(&font);
rb->set_theme(&theme);
parent->add_child(rb);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `RadioButton`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `RadioButton(float x, float y, float w, float h, const char* text)` |  |
| `void set_text(const char* text)` |  |
| `const std::string& text() const` |  |
| `void set_checked(bool checked)` |  |
| `bool checked() const` |  |
| `void set_on_activate(std::function<void()> cb)` | 激活（点击/空格）回调：由 RadioGroup 统一做互斥选择 |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_key_down(int key, int mods) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/radiogroup.h`

**调用示例**：
```cpp
auto* rg = new ui::RadioGroup(0, 0, 400, 28);
rg->set_font(&font);
rg->set_theme(&theme);
rg->add_item("男");
rg->add_item("女");
rg->set_on_change([](const char* v, int idx) { ui::log_info("选中 %s", v); });
parent->add_child(rg);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`vector`

**类 `RadioGroup`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `RadioGroup(float x, float y, float w, float h)` |  |
| `void add_option(const char* text)` | 追加一个选项（行高自动布局） |
| `int selected() const` |  |
| `void select(int index)` | 选中并触发 on_change |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `void set_option_height(float h)` |  |
| `void set_spacing(float s)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_option_enabled(int index, bool en)` | 禁用/启用单个选项（layui 单选框禁用） |
| `void set_enabled(bool en) override` | 组级禁用：传播到所有选项 |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |
| `float preferred_width() const` |  |


### `ui/ui/rate.h`

**调用示例**：
```cpp
auto* r = new ui::Rate(0, 0, 300, 30);
r->set_font(&font);
r->set_theme(&theme);
r->set_value(3);
r->set_readonly(false);
r->set_on_change([](int v) {});
parent->add_child(r);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `Rate`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Rate(float x, float y, float w, float h)` |  |
| `void set_value(float v)` | 当前值 0..length（half 开启时可为 x.5） |
| `int value() const` | 向后兼容：返回整数部分 |
| `float value_f() const` |  |
| `void set_length(int n)` | 星数（layui length，默认 5） |
| `int length() const` |  |
| `void set_half(bool half)` | 半星（layui half）：允许 x.5 取值，绘制半颗星 |
| `bool half() const` |  |
| `void set_text(bool text)` | 文本（layui text）：星后显示数值文本；text 回调自定义（layui setText） |
| `bool text() const` |  |
| `void set_text_callback(std::function<std::string(float)> cb)` |  |
| `void set_theme_color(const Vec4& c)` | 自定义主题色（layui theme） |
| `void set_readonly(bool readonly)` |  |
| `bool readonly() const` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_font(Font* font)` |  |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/scrollbox.h`

**调用示例**：
```cpp
auto* sb = new ui::ScrollBox(0, 0, 300, 400);
sb->set_theme(&theme);
sb->set_padding(12.f);
sb->set_spacing(8.f);
sb->add_child(child_widget);                 // 内容超高自动滚动 + scissor 裁剪
sb->set_on_scroll_to_bottom([] { ui::log_info("滚到底"); });  // 传 nullptr 清除
parent->add_child(sb);
```

**依赖 include**：`ui/ui/scrollbar_style.h`、`ui/ui/widget.h`、`functional`

**类 `ScrollBox`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `ScrollBox(float x, float y, float w, float h)` |  |
| `void set_spacing(float spacing)` | 子控件之间的间距（像素） |
| `void set_padding(float padding)` | 内部边距（像素） |
| `float padding() const` |  |
| `void set_header_height(float h)` | 固定头部高度（像素，默认 0=关闭）。>0 时顶部留出高度 header_height_ 的固定 不可滚动横条（背景与内容面板连续），内容在其下方滚动——用于把「首个卡片上方 的留白」固定在顶部而不随滚动上移（UI-142）。既可用默认直角背景，也可 layui class。 |
| `float header_height() const` |  |
| `void set_scroll_step(float step)` | 滚动步长（每滚一格滚轮） |
| `float scroll_offset() const` |  |
| `void set_scroll_offset(float offset)` | 滚动到指定偏移（自动 clamp 到 [0, scroll_max]；脚本化截图/程序化定位用） |
| `void reset_scroll()` | 滚动回顶部（切页/内容重建后复位滚动位置，防止残留旧页滚动偏移） |
| `void mark_content_dirty()` | 标记子树变化（clear_children/add_child 后调用），抑制滚动条绘制直到下一次 on_layout() |
| `void set_on_scroll_to_bottom(std::function<void()> cb)` | 滚到内容底部时回调一次（再次离开底部再滚回底部会重新触发）。 传入 nullptr 清除回调（切页清理用，防悬空引用）。 |
| `void rearm_bottom()` | 内容增长后由宿主调用：解除触底一次性武装并安排重检（UI-126 引入，UI-127 改分帧）。 场景：流加载续灌批次未把内容底推出触发区（用户已在底部无法再滚），不清 bottom_fired_ 将永不再触发——补齐 layui flow.load「DOM 插入后再查一次 reachBottom」 语义。UI-127 起不再同步重检（同步链会在 rebuild_rows 内递归级联多批、单帧工作量 暴增），改为置挂起标记、下一次 on_layout() 开头消费 check_bottom——每帧至多灌一批， 且回调重建的行控件在本次布局内即归位，无乱帧。 |
| `void set_on_scroll(std::function<void()> cb)` | 通用滚动回调：每次 scroll_offset 变化时触发（用于关闭下拉面板等场景）。 |
| `void set_bottom_margin(float mb)` | 距底部触发距离 margin（镜像 layui flow mb，默认 50）：当 scroll_offset 距内容底部 不足 mb 像素时即触发触底回调，而不必严格滚到底（UI-094）。仅影响 check_bottom， 触发/重新武装语义（bottom_fired_）与 set_on_scroll_to_bottom 一致。 |
| `void add_pinned(Widget* w, PinCorner corner = PinCorner::BottomRight, float margin_x = 16.f, float margin_y = 16.f)` | —— 视口级锚定子控件（layui fixbar，UI-110）—— 控件照常 add_child（生命周期/命中/绘制顺序不变），但不参与纵向流式排布、 不计入 content_height；每次布局钉在可视区 corner 内缩 (margin_x, margin_y) 处， 不随 scroll_offset 移动。后加的锚定控件绘制在流式内容之上。 |
| `int pinned_count() const` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |
| `void draw(Renderer& renderer) override` | 覆写 draw：背景 -> scissor(内容) -> 滚动条 |
| `bool pass_hit_to_children(float x, float y) override` | 命中裁剪守卫（UI-152）：与绘制端 scissor(bounds_) 对称，滚动越界到可视区外的 子控件（如滚到标题栏区域的设置页内容）不可命中，消除「点击上层却落到越界内容」穿透。 |
| `void on_advance(float dt) override` | overlay 滚动条自动隐藏（2s 无活动）：状态迁移时标脏，触发帧级重绘收/放滚动条 |
| `void on_children_changed() override` | 子节点变化后抑制滚动条绘制，直到下一次 on_layout() |
| `bool on_mouse_scroll(float x, float y, float dx, float dy) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `void set_theme(const Theme* theme)` |  |

**类 `Pinned`**

| 数据成员 | 说明 |
| --- | --- |
| `Widget* w;` |  |
| `PinCorner corner;` |  |
| `float mx, my;` |  |


### `ui/ui/slider.h`

**调用示例**：
```cpp
auto* sl = new ui::Slider(0, 0, 400, 30);
sl->set_theme(&theme);
sl->set_range(0.f, 100.f);
sl->set_value(60.f);
sl->set_on_change([](float v) {});
sl->set_value_silent(50.f);                  // 程序同步不触发回调
parent->add_child(sl);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `Slider`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Slider(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_range(float min_value, float max_value)` |  |
| `void set_value(float v)` | —— 单值（默认模式）—— 改值并触发回调 |
| `void set_value_silent(float v)` | 改值但不触发回调（程序同步） |
| `float value() const` |  |
| `void set_range_value(float start, float end)` | —— 范围模式值（range 时两端）—— 触发回调 |
| `float value2() const` |  |
| `bool dragging() const` |  |
| `void set_range_enabled(bool on)` | —— layui slider 选项 —— 范围拖拽（双环） |
| `void set_step(float step)` |  |
| `void set_showstep(bool on)` | 间断点 |
| `void set_tips(bool on)` | 提示文本 |
| `void set_tips_callback(std::function<std::string(float)> fn)` | setTips |
| `void set_input(bool on)` | 输入框（range 时无效） |
| `void set_disabled(bool on)` | 禁用拖拽 |
| `void set_theme_color(const Vec4& c)` | 主题色 |
| `void set_vertical(bool on)` | type:vertical |
| `void set_track_height(float h)` | vertical 高度 |
| `void set_focus_ring(bool on)` | 聚焦外框反馈（默认不显示） |
| `void set_theme(const Theme* theme)` |  |
| `void set_on_change(std::function<void(float)> cb)` | 值变化回调（普通模式返回单值） |
| `void set_on_range_change(std::function<void(float, float)> cb)` | 范围模式回调整（start, end） |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `bool on_key_down(int key, int mods) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/switch.h`

**调用示例**：
```cpp
auto* sw = new ui::Switch(0, 0, 56, 28);
sw->set_theme(&theme);
sw->set_checked(true);
sw->set_on_change([](bool on) { ui::log_info(on ? "开" : "关"); });
parent->add_child(sw);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `Switch`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Switch(float x, float y, float w, float h)` |  |
| `void set_text(const char* text)` | 设置两态文本，用 "\|" 分隔（如 "ON\|OFF"、"开启\|关闭"）；无 "\|" 时整体作为开态文本 |
| `const std::string& text() const` |  |
| `void set_checked(bool on)` |  |
| `bool checked() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_on_change(std::function<void(bool)> cb)` | 值变化回调（参数为最新值） |
| `void set_custom_content(Widget* content)` | lay-checkbox 绑定（开关风格）：设置紧随开关的自定义标题内容控件。 拥有权归 Switch；内容绘制在滑块右侧（开关关态显示）。典型用途：图标双态标题 （layui <i moon> \| <i light>），配合 set_on_change 在测试端控制显隐。 |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_key_down(int key, int mods) override` |  |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `void on_draw(Renderer& renderer) override` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void draw_switch_skin(Renderer& renderer, const Rect& rect, const Theme* theme, bool checked, bool enabled, bool hovered = false, bool pressed = false)` | 统一圆角开关皮肤（UI-130）：项目内一切开关必须用本函数渲染（ui::Switch 本体、 Table set_cell_switch 行内开关等），严禁另造矩形/方块皮肤——开=绿底白滑块靠右， 关=白底灰边灰滑块靠左，禁用=全灰。取色走传入 Theme（空指针回退默认主题）； hover/pressed 仅控件本体传入。 |


### `ui/ui/table.h`

**调用示例**：
```cpp
auto* tb = new ui::Table(0, 0, 800, 400);
tb->set_font(&font);
tb->set_theme(&theme);
tb->add_column("ID");
tb->add_column("姓名");
tb->add_row({"1", "张三"});                  // 行 = 与列等长的字符串列表
tb->add_class("layui-table");                // layui 外观类
parent->add_child(tb);
```

**依赖 include**：`ui/ui/widget.h`、`chrono`、`functional`、`string`、`vector`

**类 `Table`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Table(float x, float y, float w, float h)` |  |
| `void add_column(const char* title, float width, bool sortable = false)` | 添加列：title 表头文本、width 列宽（像素）、sortable 是否可排序 |
| `void add_row(const std::vector<std::string>& cells)` |  |
| `void clear_rows()` |  |
| `void set_skin(TableSkin skin)` |  |
| `void set_size(TableSize size)` |  |
| `void set_even(bool even)` |  |
| `void set_cell_min_width(float w)` |  |
| `void set_truncate_cols(const std::vector<int>& cols)` | 指定哪些列启用自动截断：文字超出列宽时以 "..." 截断 |
| `bool is_truncate_col(int col) const` |  |
| `void set_measure_text_cols(const std::vector<int>& cols)` | 指定哪些列用 measure_text 精确截断（渲染时逐行测量，替代纯字符估算） |
| `bool is_measure_text_col(int col) const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_on_sort(std::function<void(int col, bool asc)> cb)` |  |
| `void set_on_row_click(std::function<void(int row)> cb)` |  |
| `void set_paginated(bool on, int page_size = 10)` | —— 分页（layui page:true / resetPage）—— 分页时表底显示 Laypage，仅渲染当前页 |
| `bool paginated() const` |  |
| `void set_page(int page)` | 0 基，立即跳页 |
| `int page() const` |  |
| `int page_count() const` |  |
| `int page_size() const` |  |
| `int auto_page_size() const` | 按当前 bounds 高度自适应每页行数（UI-123）：已扣除表头与内部分页条高度 （28px）。调用方此前按 (h-表头)/行高 自行估算，末行被挤进分页条下方盲区。 |
| `int visible_row_count() const` | 当前页可见数据行数（未分页 = 全量行数） |
| `int data_row_of(int visible) const` | 可视行号 → 数据行下标（rows_ 下标；跨分页/树形过滤的稳定行标识） |
| `void set_on_load_more(std::function<void()> cb)` | —— 流加载（layui flow 信息流滚动自动加载，UI-126）—— 仅非分页模式生效： body_ 滚动近底部（mb 临界距离，UI-035/UI-094 语义）时回调；配合 add_row/reload 续灌数据。rebuild_rows 尾部自动 rearm 重检：追加批次后若内容底仍未推出触发区， 将继续回调直至填满一屏或数据耗尽（耗尽时回调方停止 append 即可，链自然断）。 |
| `void set_end_note(const char* text)` | 流加载尾注（如「没有更多了」）：非分页且非空时在最后一行之后渲染只读提示行 （灰字居中跨列，随内容滚动）；不参与 data_row_of/选中/编辑/命中回调。 文本变化才触发重建，空串清除。 |
| `void reset_scroll()` | 滚动回顶部（过滤条件变化重灌数据后调用） |
| `void prebake_visible_text()` | 预热表头 + 当前可见行全部单元格 + 尾注的 CJK 字形（UI-127）：一次性烘焙消除 UI-125 逐帧配额限流的方块宽渐进补齐闪烁（流加载续灌瞬间/表格首次显示瞬间）。 prebake_text 不受配额限制且自动跳过已烘焙码点，重复调用开销可忽略。 |
| `void add_checkbox_column()` | —— 复选列 / 单选列（layui type:'checkbox' / type:'radio'）—— 表头全选 + 各行复选位 |
| `void add_radio_column()` | 表头占位 + 各行单选位 |
| `void add_numbers_column()` | 序号列（从 1 起，跨页连续） |
| `bool has_selection() const` |  |
| `int selection_col() const` |  |
| `bool is_radio_column() const` |  |
| `bool is_cell_form(int col) const` |  |
| `bool is_cell_switch(int col) const` |  |
| `bool is_numbers(int col) const` |  |
| `bool is_button(int col) const` |  |
| `bool row_checked(int visible_row) const` | 复选位 |
| `int checked_radio() const` | 单选选中行（可见行，-1 无） |
| `int checked_count() const` | 当前选中行数（复选） |
| `int first_checked() const` | 第一个复选行（可见行，-1 无） |
| `void set_on_selection(std::function<void(int visible_row, bool checked)> cb)` | 选中 / 单选变化回调（参数：可见行号, 最新选中态） |
| `void set_cell_switch(int col)` | —— 单元格表单列（layui templet 内嵌 switch/checkbox，form.html）—— 该列每格渲染 Switch（基于该行第 col 列文本非空与否预置开） |
| `void set_cell_checkbox(int col)` | 该列每格渲染 CheckBox（默认勾选：文本为 "1"/"true"） |
| `void set_on_cell_form(std::function<void(int visible_row, int col, bool checked)> cb)` | 单元格表单切换回调（UI-129）：点击 Switch/CheckBox 格翻转后触发 （参数：可见行号, 列号, 切换后的选中态）；此前仅翻转单元格文本不通知，调用方无法感知 |
| `void set_on_cell_edited(std::function<void(int data_row, int col, const std::string& text)> cb)` | —— 单元格文本编辑提交回调（UI-134）—— 双击文本格进入行内编辑，Enter/失焦 commit_edit 写回 rows_ 后触发； 参数：数据行号（rows_ 下标，跨分页/排序稳定）, 列号, 新文本。 此前仅写控件私有副本不通知调用方，应用层无法感知编辑结果 （audio_text 模式映射编辑丢失缺陷根源）。 |
| `void set_cell_editable(int col, bool editable)` | 列级可编辑开关（UI-134）：false 则该列双击不进入行内编辑（默认 true 可编辑） |
| `void set_cell_combobox(int col)` | —— 下拉单元格列（UI-143：表格行内下拉选择，复用 ComboBox layui-form-select 皮肤）—— 该列每格渲染文本 + 右侧下拉三角，单击单元格展开行内 ComboBox； 选项选中后写回该格文本并触发选中回调。 |
| `bool is_cell_combobox(int col) const` |  |
| `void set_combobox_options(int col, const std::vector<std::string>& options)` | 指定下拉列的可选项（与列下标平行；未设置视为空选项集） |
| `void set_on_cell_combobox(std::function<void(int visible_row, int col, int index, const std::string& text)> cb)` | 下拉选中回调（参数：可见行号, 列号, 选项下标, 选中文本） |
| `void set_total_row(bool on)` | —— 合计行（layui totalRow，totalRow.html / parseData.html）—— |
| `bool total_row() const` |  |
| `void set_total_row_text(const char* s)` |  |
| `void set_total_col(int col)` | 标记某列为合计列（表尾对当前页该列数值求和）；total-row-text 仅放首合计列 |
| `int total_col() const` |  |
| `void set_cell_event_col(int col)` | —— 列点击事件（layui 单元格事件 cellEvent.html，如签名列 event:'setSign'）—— |
| `int cell_event_col() const` |  |
| `void set_on_cell_event(std::function<void(int visible_row, int col)> cb)` |  |
| `void set_cell_tooltip(std::function<std::string(int data_row, int col)> cb)` | —— 单元格 tooltip（UI-177）—— 注册悬停回调：行控件悬停某列时按（数据行号, 列号）查询完整文本；返回空串不显示 tooltip。 |
| `std::string cell_tooltip_at(int visible_row, float x) const` | 行控件内部调用：根空间横坐标命中的单元格 tooltip（未注册回调返回空） |
| `void set_init_sort(int col, bool asc)` | —— 初始排序（layui initSort）—— |
| `void reload(const std::vector<std::vector<std::string>>& new_rows)` | —— 数据重载（layui reload）—— |
| `void set_on_row_double_click(std::function<void(int visible_row)> cb)` | —— 行双击回调（layui onrow）—— |
| `void add_button_column(const char* title, float width, const std::vector<std::string>& labels, const std::vector<std::string>& events = {})` | —— 操作列（layui toolbar 列：行内按钮 查看/编辑/删除，toolbar/operate/totalRow 等）—— |
| `int button_col() const` |  |
| `int button_count() const` |  |
| `const std::string& button_label(int i) const` |  |
| `const std::string& button_event(int i) const` |  |
| `void set_on_action(std::function<void(int visible_row, const std::string& event)> cb)` | 行内工具按钮点击回调（参数：可视行号, 事件名） |
| `void add_tree_row(int depth, const std::vector<std::string>& cells)` | —— 树形层级（layui treeTable）—— 追加带层级的行：depth 0=根，>0 为上一 depth-1 行的子级（须按先序次序追加）。 有子级的行首列渲染展开三角（▼ 展开 / ▶ 折叠），点击切换；折叠时整棵子树隐藏。 |
| `bool is_tree() const` | 存在 depth>0 的行 |
| `int row_depth(int data_row) const` | 层级深度（add_row 的行恒为 0） |
| `bool row_has_children(int data_row) const` | 下一行深度更深即有子级 |
| `bool row_expanded(int data_row) const` | 默认展开 |
| `void set_row_expanded(int data_row, bool on)` |  |
| `void toggle_row(int visible_row)` | 点击展开图标切换并重建 |
| `float tree_indent() const` | 每级缩进像素 |
| `void set_on_expand(std::function<void(int data_row, bool expanded)> cb)` |  |
| `int col_count() const` | 供内部行控件查询列/数据（TableRow） |
| `float col_width(int c) const` |  |
| `const std::vector<std::string>& row(int r) const` |  |
| `int selected_row() const` |  |
| `void set_selected_row(int visible_row)` | 行选中态（UI-124）：单击数据行高亮定位；参数为可见行号，-1 清除。 与复选/单选列无关，纯视觉反馈；reload/set_page/排序时自动清除。 |
| `bool even() const` |  |
| `bool has_row_line() const` |  |
| `bool has_col_line() const` |  |
| `float header_h() const` |  |
| `float row_h() const` |  |
| `float cell_pad_x() const` |  |
| `void on_layout() override` |  |
| `float preferred_height() const` | 高度语义（UI-131）：默认内容跟随——每次调用按表头 + 可见行数×行高 [+ 分页条] 重算， 不再被首次分配的 bounds_.h 冻结（旧「h>0 即冻结」导致流式容器内行数增长永不反映， 行区被裁剪成 0，见 audio_text SPK-005）。 set_fixed_height(h>0)：显式锁定高度（固定视口 + 内部纵向滚动的摆放），h<=0 解除回内容跟随； set_max_content_height(h>0)：内容跟随模式的上限钳制（流式容器防溢出），超出部分由内部滚动区承载。 |
| `void set_fixed_height(float h)` |  |
| `void set_max_content_height(float h)` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Column`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `float width = 0.f;` |  |
| `bool sortable = false;` |  |
| `bool ascending = true;` |  |
| `bool editable = true;` | UI-134：false 则双击不进入行内编辑 |
| `ColKind kind = ColKind::Data;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void layout_columns()` | 表头/行共用列起始 x（缓存） |
| `void rebuild_rows()` | 重建 body 内行控件 |
| `void truncate_rows()` | 按列宽截断 truncate_cols_ 中的文本 |
| `void sort_rows(int col)` |  |
| `void begin_edit(int row, int col, float click_x = -1.f)` |  |
| `void commit_edit()` |  |
| `void begin_combo(int row, int col)` | 下拉单元格列：单击展开行内 ComboBox（UI-143） |
| `void close_combo()` |  |
| `bool in_header(float x, float y) const` |  |
| `int header_col_at(float x) const` |  |
| `int col_hit(float x) const` |  |
| `float col_x(int c) const` | 第 c 列起始 x（布局后有效） |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/tabs.h`

**调用示例**：
```cpp
auto* tabs = new ui::Tabs(0, 0, 600, 40);
tabs->set_font(&font);
tabs->set_theme(&theme);
tabs->add_tab("预览");
tabs->add_tab("代码");
tabs->set_on_change([](int idx) { ui::log_info("切到 %d", idx); });
parent->add_child(tabs);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Tabs`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Tabs(float x, float y, float w, float h)` |  |
| `int add_tab(const char* title, const char* content = nullptr)` | 追加并返回页下标 |
| `void set_content(int index, const char* content)` | 设置指定页内容文本 |
| `void remove_tab(int index)` | 删除指定页（含内容区联动） |
| `int count() const` |  |
| `void set_badge(const char* text)` | 徽章 / 小圆点（作用在最后 add_tab 的标签页标题右缘，layui-tab 标题内嵌） |
| `void set_dot_badge()` |  |
| `void set_active(int index)` | 激活第 index 个标签页并触发 on_change |
| `int active() const` |  |
| `void set_on_change(std::function<void(int)> cb)` |  |
| `void set_allow_close(bool allow)` | lay-allowclose：true 时每页标题右缘显示删除 ×，点击删除该页 |
| `bool allow_close() const` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Tab`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `std::string content;` |  |
| `bool has_badge = false;` | 标题右缘徽章（layui-badge 红底白字） |
| `std::string badge_text;` |  |
| `bool has_dot = false;` | 标题右缘小圆点（8px layui-badge-dot） |
| `float x0 = 0.f, x1 = 0.f;` | 标题条内 x 范围（点击命中） |
| `bool visible = true;` | 超宽时被收纳的页标为不可见 |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `int hit_tab(float x) const` | 命中可见标题下标，未命中 -1 |
| `bool hit_close(float x, float y) const` | 点击允许关闭页的 × 区 |
| `void apply_active()` | 激活页内容写入内容区 Label |
| `void layout_titles()` | 计算标题布局 + 超宽收纳 |
| `float title_width(const Tab& tb) const` | 单个标题宽（含徽章/圆点余量） |
| `void open_overflow()` | 展开超宽收纳页下拉面板 |
| `void show_content(const char* text)` | 内容区显示指定文本 |
| `std::vector<Button*> overflow_buttons()` | 下拉面板中的按钮列表 |
| `float content_h() const` | 内容区高度 |
| `float overflow_x() const` | 超宽展开三角区起点 x |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/textbox.h`

**调用示例**：
```cpp
auto* tb = new ui::TextBox(0, 0, 300, 36);
tb->set_font(&font);
tb->set_theme(&theme);
tb->set_placeholder("请输入");
tb->set_on_change([](const char* s) {});
parent->add_child(tb);
tb->focus();                                 // focusable，可聚焦输入
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`

**类 `TextBox`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `TextBox(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_text(const char* text)` |  |
| `const std::string& text() const` |  |
| `void set_placeholder(const char* text)` | 空文本时绘制灰色占位提示（不影响输入内容） |
| `void set_password(bool on)` | 密码模式：显示 '*' 掩码，光标宽度按掩码计算 |
| `void set_on_change(std::function<void(const char*)> cb)` | 文本变化（含程序 set_text）时回调 |
| `void set_on_enter(std::function<void()> cb)` | 按 Enter 时回调（M17c：Table 行内编辑提交） |
| `void set_caret_pos(size_t pos)` | 设置光标字节位置（Table::begin_edit 初始定位 + 点击定位） |
| `float caret_offset() const` | 文本起点 -> caret 的像素宽度（IME 锚定等外部用途） |
| `bool has_selection() const` | ---- 选择 / 剪贴板（UI-101）---- |
| `std::string selected_text() const` | 选中区间文本（字节下标 [min(anchor,caret), max)；密码框返回原文） |
| `void select_all()` |  |
| `void clear_selection()` |  |
| `void set_drop_enabled(bool v)` | ---- 文件拖放 ---- |
| `bool drop_enabled() const` |  |
| `void set_on_drop(std::function<void(const std::vector<std::string>&)> cb)` | 自定义拖放处理：覆盖默认路径插入行为 |
| `void insert_text(const std::string& s)` | 向外部暴露插入能力（供 on_drop_cb_ 回调使用） |
| `bool on_key_down(int key, int mods) override` |  |
| `bool on_char(unsigned int codepoint) override` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `bool on_drop(float x, float y, const std::vector<std::string>& paths) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/timeline.h`

**调用示例**：
```cpp
auto* tl = new ui::Timeline(0, 0, 600, 200);
tl->set_theme(&theme);
tl->add_item("第一步", "说明文字");
tl->add_item("第二步");
parent->add_child(tl);
```

**依赖 include**：`ui/ui/widget.h`、`chrono`、`string`、`vector`

**类 `ContentLine`**

| 数据成员 | 说明 |
| --- | --- |
| `LineKind kind = LineKind::Text;` |  |
| `std::string text;` |  |
| `uint32_t codepoint = 0;` | Icon 用 |

**类 `Timeline`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Timeline(float x, float y, float w, float h)` |  |
| `void add_item(const char* title, const char* content)` | 追加条目：title 为时间轴标题，content 为详情（可空，单行） |
| `void add_item_rich(const char* title, bool title_large, std::vector<ContentLine> lines)` | 追加富内容条目：title 普通标题（false）或 h3 大标题（true，18px #333） |
| `void set_axis_icon(uint32_t codepoint)` | 最后 add_item 条目的轴点图标（默认 layui-icon-circle 0xE63F） |
| `void set_axis_rotate(float degrees_per_sec)` | 最后 add_item 条目的轴点图标绕中心旋转（角度/秒，>0 开启；用于 loading 循环动画） |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `float preferred_height() const` | 内容高度 = 条目数 × (标题行 + 内容行数 × 行高 + 20px padding) |
| `bool on_mouse_move(float x, float y) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Item`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `bool title_large = false;` |  |
| `std::vector<ContentLine> lines;` | 空 = 无详情 |
| `uint32_t axis_icon = 0xE63F;` | layui-icon-circle |
| `float axis_rotate_dps = 0.f;` | 轴点图标旋转角速度（/秒；0 不旋转） |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `std::chrono::steady_clock::time_point anim_start_ = std::chrono::steady_clock::now()` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/titlebar.h`

**依赖 include**：`ui/ui/widget.h`、`ui/core/math.h`、`functional`、`string`

**类 `TitleBar`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `TitleBar(float w, float h, const char* title = , Window* win = nullptr)` |  |
| `void set_font(Font* f)` |  |
| `void set_theme(const Theme* t)` |  |
| `void set_title(const char* title)` |  |
| `void set_icon(Font* font, uint32_t codepoint)` | 左上角应用图标：font 用 Font::draw_icon 画 codepoint（iconfont 已有码点，如 layui_icon::more） |
| `void set_on_close(std::function<void()> cb)` |  |
| `void set_on_minimize(std::function<void()> cb)` |  |
| `void set_on_maximize(std::function<void()> cb)` |  |
| `void set_minimize_visible(bool v)` | 右侧窗口键显隐（默认三键全显） |
| `void set_maximize_visible(bool v)` |  |
| `void set_close_visible(bool v)` |  |
| `bool close_visible() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `bool on_mouse_up(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/transfer.h`

**调用示例**：
```cpp
auto* tr = new ui::Transfer(0, 0, 700, 260);
tr->set_font(&font);
tr->set_theme(&theme);
tr->add_item("左项1");
tr->add_item("左项2");
parent->add_child(tr);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Transfer`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Transfer(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_left_label(const char* s)` |  |
| `void set_right_label(const char* s)` |  |
| `void add_item(const char* name, bool disabled = false)` | 加入左侧列表项；disabled 项不可选中/搬移（layui data.disabled） |
| `void add_right_item(const char* name, bool disabled = false)` | 预置右侧列表项（layui value：初始右侧数据集合） |
| `void clear()` | 清空两侧列表（实例 reload 重建用） |
| `void set_show_search(bool on)` | 显示搜索框（layui showSearch）：各面板头部输入实时过滤 |
| `bool show_search() const` |  |
| `void set_on_change(std::function<void(TransferDirection, const std::vector<std::string>&)> cb)` | 穿梭回调：direction 为本次搬移的目标面板；items 为被搬移的数据标题 |
| `const std::vector<std::string>& right_items() const` | getData：右侧当前数据集合 |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_scroll(float x, float y, float dx, float dy) override` |  |
| `void on_layout() override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Side`**

| 数据成员 | 说明 |
| --- | --- |
| `std::vector<std::string> items;` |  |
| `std::vector<bool> disabled;` |  |
| `std::vector<bool> sel;` |  |
| `std::string filter;` | 搜索过滤关键字（showSearch） |
| `float scroll = 0.f;` | 列表滚动偏移（large 数据集超出面板高度时滚轮可达） |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `int hit_row(const Side& s, const Rect& panel, float x, float y, bool search_on) const` | 命中某面板某行 → 返回该项在 items 中的真实下标（含过滤映射 + 滚动偏移）；未命中返回 -1 |
| `std::vector<int> visible_rows(const Side& s) const` | 过滤后可见项的真实下标集合（顺序保持；无过滤时返回全量） |
| `float list_view_h(const Rect& panel, bool search_on) const` | 某面板列表可视区高度（标题条 + 搜索区之下） |
| `void clamp_scroll(Side& s, const Rect& panel, bool search_on)` | 内容总高超过可视高度时，把 scroll 夹紧到有效范围 |
| `void move(TransferDirection dir)` |  |
| `void emit_change(TransferDirection dir, const std::vector<std::string>& moved)` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/tree.h`

**调用示例**：
```cpp
auto* tree = new ui::Tree(0, 0, 300, 400);
tree->set_font(&font);
tree->set_theme(&theme);
tree->add_item("父节点", { "子1", "子2" });
tree->set_on_select([](const char* p, const char* name) {});
parent->add_child(tree);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `TreeNodeSpec`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `int id = 0;` |  |
| `std::string field;` |  |
| `std::string href;` | isJump 时点击跳转目标 |
| `bool disabled = false;` |  |
| `bool fixed = false;` | 固定节点（不允许删除/移动） |
| `bool checked = false;` |  |
| `bool spread = false;` | 默认展开 |

**类 `Tree`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Tree(float x, float y, float w, float h)` |  |
| `int add_node(const char* text, int parent = -1)` | 追加节点：parent 为父节点下标（-1 表示根级）。返回新节点下标。 |
| `int add_node(const TreeNodeSpec& spec, int parent = -1)` |  |
| `void set_node_text(int index, const char* text)` | 节点属性 |
| `void set_node_id(int index, int id)` |  |
| `void set_node_href(int index, const char* href)` |  |
| `void set_node_disabled(int index, bool disabled)` |  |
| `void set_node_checked(int index, bool checked)` |  |
| `void set_expanded(int index, bool expanded)` | 展开/折叠指定节点（含子孙显隐）；accordion 时自动收起同级兄弟 |
| `bool is_expanded(int index) const` |  |
| `void expand_all(bool expanded)` |  |
| `void set_show_checkbox(bool on)` | —— 组件外观/行为开关（layui tree 选项）—— |
| `void set_show_line(bool on)` |  |
| `void set_accordion(bool on)` |  |
| `void set_only_icon_control(bool on)` |  |
| `void set_is_jump(bool on)` |  |
| `void set_editable(const std::vector<TreeEdit>& actions)` | 开启节点操作图标（add/update/del） |
| `std::vector<int> get_checked() const` | —— 复选框查询/勾选（layui getChecked/setChecked）—— 返回当前勾选的节点下标（含半选父级？否——仅完全勾选的叶子/分支） |
| `void set_checked(int index, bool checked)` | 含子孙传播 + 祖先重算 |
| `void check_all(bool on)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void set_on_click(std::function<void(int)> cb)` | 点击节点回调（index 为节点下标） |
| `void set_on_edit(std::function<void(int, TreeEdit)> cb)` | 操作图标回调（index 节点下标，act 操作类型） |
| `void on_layout() override` |  |
| `float preferred_height() const` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `bool on_mouse_move(float x, float y) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

**类 `Node`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string title;` |  |
| `int id = 0;` |  |
| `std::string field;` |  |
| `std::string href;` |  |
| `bool disabled = false;` |  |
| `bool fixed = false;` |  |
| `bool checked = false;` |  |
| `bool expanded = false;` |  |
| `int parent = -1;` |  |
| `std::vector<int> children;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void layout_nodes() const` | 计算可见节点顺序与 y 位置 |
| `int hit_node(float y) const` | 命中 y 所在可见序号（-1 无） |
| `bool node_has_children(int idx) const` |  |
| `void set_checked_recursive(int idx, bool checked)` |  |
| `void update_ancestors(int idx)` | 由子节点勾选态重算祖先行（半选） |
| `bool is_indeterminate(int idx) const` | 节点是否半选（有子勾选但未全选）——动态计算 |
| `std::vector<int> depth_of_nodes() const` | 节点深度表（绘制缩进用） |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/upload.h`

**调用示例**：
```cpp
auto* up = new ui::Upload(0, 0, 300, 60);
up->set_font(&font);
up->set_theme(&theme);
up->add_class("layui-upload layui-upload-drag");  // 拖拽上传样式
up->set_on_select([](const char* path) { ui::log_info("选择 %s", path); });
parent->add_child(up);
```

**依赖 include**：`ui/ui/widget.h`、`functional`、`string`、`vector`

**类 `Upload`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `Upload(float x, float y, float w, float h)` |  |
| `void set_font(Font* font)` |  |
| `void set_theme(const Theme* theme)` |  |
| `void add_file(const char* name, int size_bytes)` | 添加一个模拟文件（文件名 + 大小字节），开始上传 |
| `void advance(float dt)` |  |
| `bool on_mouse_down(float x, float y, int button) override` |  |
| `void on_draw(Renderer& renderer) override` |  |

| 数据成员 | 说明 |
| --- | --- |
| `std::function<void()> on_pick;` | 应用层可覆盖"选择文件"行为 |

**类 `File`**

| 数据成员 | 说明 |
| --- | --- |
| `std::string name;` |  |
| `int size = 0;` |  |
| `float progress = 0.f;` |  |
| `bool done = false;` |  |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void remove_done()` |  |

**常量**：

| 常量 | 值 | 说明 |
| --- | --- | --- |
| `theme_` | `nullptr` |  |


### `ui/ui/vbox.h`

**调用示例**：
```cpp
auto* vb = new ui::VBox(0, 0, 400, 200);
vb->set_padding(10.f);
vb->set_spacing(8.f);
vb->add_child(new ui::Button(0, 0, 0, 36, "上"));   // 子控件等宽填满
vb->add_child(new ui::Button(0, 0, 0, 36, "下"));
parent->add_child(vb);
```

**依赖 include**：`ui/ui/widget.h`

**类 `VBox`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `VBox(float x, float y, float w, float h)` |  |
| `void set_spacing(float spacing)` | 子控件之间的间距（像素） |
| `void set_padding(float padding)` | 内部边距（像素） |
| `void on_layout() override` | 布局：把子控件按顺序纵向排列；子控件高度用各自 bounds_.h（可通过 set_bounds 预设） |
| `float preferred_height() const` | 内容高度 = 子控件高度之和 + 间距 + 边距（供外层容器如 ScrollBox 计算） |
| `void set_theme(const Theme* theme)` |  |
| `void on_draw(Renderer& renderer) override` |  |


### `ui/ui/window_frame.h`

**调用示例**：
```cpp
// 窗口容器：内置自绘标题栏 + 下方内容区，内容子控件用容器相对坐标
auto* wf = new ui::WindowFrame(0, 0, 500, 320);
wf->set_font(&font);
wf->set_theme(&theme);
wf->set_title("窗口标题");
wf->set_titlebar_height(30.f);                    // 标题栏高度（默认 30）
wf->set_content_child(new ui::Panel(0, 0, 0, 0), 10.f, 10.f, 0.f, 0.f);  // 相对 titlebar 底缘
wf->set_on_close([] { ui::log_info("关闭"); });
parent->add_child(wf);
```

**依赖 include**：`ui/ui/widget.h`、`ui/core/math.h`、`functional`、`string`、`vector`

**类 `WindowFrame`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `WindowFrame(float w, float h)` |  |
| `void set_head(Widget* head, float height)` | ---- 头部槽 ---- 通用头部：替换默认 TitleBar（任意外部构造的 Widget 作 head）；width 随本控件矩形自动铺满。 传 nullptr 表示无头部（内容区从 y=0 起）。 |
| `void set_titlebar_height(float h)` | 标题栏高度（默认 30）：仅当 head 为内部默认 TitleBar 时同步内容区原点；自定义 head 请用 set_head 指定 |
| `float titlebar_height() const` |  |
| `float content_origin_y() const` | 内容区顶部（标题栏下方）的 y：等价 titlebar_height()，便于外部以之为单一来源派生子坐标 |
| `void set_font(Font* f)` | ---- 默认 TitleBar 透传（仅当 head 为内部默认标题栏时生效）---- |
| `void set_theme(const Theme* t)` |  |
| `void set_title(const char* title)` |  |
| `void set_icon(Font* font, uint32_t codepoint)` |  |
| `void set_on_close(std::function<void()> cb)` |  |
| `void set_on_minimize(std::function<void()> cb)` |  |
| `void set_on_maximize(std::function<void()> cb)` |  |
| `void set_minimize_visible(bool v)` |  |
| `void set_maximize_visible(bool v)` |  |
| `void set_close_visible(bool v)` |  |
| `Widget* content() const` | ---- 内容区 ---- 返回内容容器：把内容子控件 add_child 到它上面 |
| `void set_content_child(Widget* child, float x, float y, float w, float h)` | 以「容器相对坐标」定位内容子控件：(x,y) 相对内容区顶部（标题栏下方）。 记录相对矩形快照并于 on_layout 重基准为绝对坐标（相对 = 绝对 - 内容区原点）， 不会随拖动/缩放二次叠加。 |
| `void set_head_child(Widget* child, float x, float y, float w, float h)` | 以「通用头部相对坐标」定位头部子控件：(x,y) 相对头部左上角（即本框架原点）。 仅对非默认 TitleBar 的通用 head 有意义（默认 TitleBar 自绘无子控件）； 记录相对矩形快照并于 on_layout 重基准为绝对坐标，与 set_content_child 对称。 |
| `void on_layout() override` |  |

**类 `Slot`**

| 数据成员 | 说明 |
| --- | --- |
| `Widget* child;` |  |
| `Rect rel;` | 相对所在区域（head 或 content）的矩形（x,y,w,h） |

**自由函数**（命名空间 `ui` / `ui::layui`）：

| 函数 | 功能说明 |
| --- | --- |
| `void set_slot(std::vector<Slot>& slots, Widget* child, float x, float y, float w, float h, float ox, float oy)` |  |
| `void apply_slots(std::vector<Slot>& slots, Widget* owner, float ox, float oy)` |  |


## 6、ui_media（可选模块）（`ui/media`）

### `ui/media/audio_out.h`

**调用示例**：
```cpp
// winmm 系统播放，无死锁（回调线程不做 waveOutUnprepareHeader）
auto* ao = new ui::media::AudioOut;
if (!ao->open(44100, 2)) return;          // 打开默认输出设备，失败视为静音
ao->write(s16_stereo_samples, count);
ao->flush();                              // seek/循环回卷时清缓冲
// 结束：ao->close();
```

**依赖 include**：`cstddef`、`cstdint`、`deque`、`memory`、`mutex`、`vector`

**类 `AudioOut`**

| 方法 | 功能说明 |
| --- | --- |
| `AudioOut()` |  |
| `bool open(int sample_rate, int channels)` | 打开默认输出设备；失败返回 false（调用方应视为静音） |
| `void close()` |  |
| `bool is_open() const` |  |
| `void write(const std::int16_t* samples, int count)` | 追加一段 16bit 交错 PCM；内部复制，异步播放 |
| `void pause()` | 暂停 / 恢复 / 清空（seek 时调用 flush） |
| `void resume()` |  |
| `void flush()` |  |

| 数据成员 | 说明 |
| --- | --- |
| `AudioOut& operator=(const AudioOut&) = delete;` |  |


### `ui/media/video_view.h`

**调用示例**：
```cpp
// 仅当 media 启用（OPENGUI_MEDIA_FFMPEG=ON）时可用
auto* vv = new ui::media::VideoView;
vv->set_bounds(0, 0, 800, 450);
if (!vv->open_source("movie.mp4")) return;
vv->play();
vv->set_loop(true);
vv->set_speed(2.f);                       // 0.1..4 倍速
vv->seek(30.0);
vv->set_volume(0.8f);                     // 0..1 音量
// 主线程每帧：vv->advance(dt)（swap 出帧后内部上传纹理）
// 退出：vv->close_source();
parent->add_child(vv);
```

**依赖 include**：`ui/ui/widget.h`、`atomic`、`condition_variable`、`cstdint`、`mutex`、`thread`、`vector`

**类 `VideoView`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `VideoView()` |  |
| `bool open_source(const char* path)` | 打开视频文件（mp4 等 FFmpeg 支持格式），失败返回 false |
| `void close_source()` |  |
| `bool has_video() const` |  |
| `void play()` | ---- 播放控制（M7）---- |
| `void pause()` |  |
| `void set_playing(bool playing)` |  |
| `bool playing() const` |  |
| `void set_loop(bool loop)` |  |
| `bool loop() const` |  |
| `void set_speed(float speed)` | 播放倍速（0.1..4，M13）；影响播放节奏与解码节流 |
| `float speed() const` |  |
| `void step_frame()` | 暂停并解码下一帧（M13 逐帧步进） |
| `void seek(float seconds)` | 跳到指定秒（解码线程处理，主线程不阻塞；关键帧后前进解码到精确位置） |
| `double duration() const` |  |
| `float position() const` |  |
| `bool has_audio() const` | ---- 音频（M14 遗留修复：winmm 输出 + 音量）---- 文件是否含可解码音频流 |
| `void set_volume(float volume)` | 音量 0..1（默认 1），实时作用于解码出的 PCM |
| `float volume() const` |  |
| `bool advance(float dt)` | 每帧调用：推进播放并上传最新解码帧；返回是否有新帧上传（GL 调用必须在主线程） |
| `void on_draw(Renderer& renderer) override` |  |


## 7、ui_media（可选模块）（`ui/media`）

### `ui/media/image_view.h`

**调用示例**：
```cpp
// 仅当 media 启用（OPENGUI_MEDIA_OPENCV=ON）时可用
auto* iv = new ui::media::ImageView;
iv->set_bounds(0, 0, 400, 300);
if (!iv->load_from_file("photo.jpg")) return;   // 中文路径 OK
parent->add_child(iv);
```

**依赖 include**：`ui/ui/widget.h`

**类 `ImageView`**
（继承 `: Widget`）

| 方法 | 功能说明 |
| --- | --- |
| `ImageView()` |  |
| `bool load_from_file(const char* path)` | 载入图像（jpg/png/bmp 等 OpenCV 支持的格式），失败返回 false |
| `bool has_image() const` |  |
| `int image_width() const` |  |
| `int image_height() const` |  |
| `void on_draw(Renderer& renderer) override` |  |

