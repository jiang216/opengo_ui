# opengl_ui API Reference

Complete API reference for all public headers. All classes are in the `ui` namespace.

## Core Layer (`ui_core`)

### Math Types (`ui/core/math.h`)

```cpp
namespace ui {
struct Vec2 { float x = 0, y = 0; };
struct Vec4 { float r = 0, g = 0, b = 0, a = 1; };
struct Rect { float x = 0, y = 0, w = 0, h = 0; };
}
```

### Logging (`ui/core/log.h`)

```cpp
namespace ui {
void log_info(const char* fmt, ...);   // [ui] INFO: ...
void log_error(const char* fmt, ...);  // [ui] ERROR: ...
}
```

### Key Codes (`ui/core/keycodes.h`)

`ui::Key` enum: `KeyA`..`KeyZ`, `Key0`..`Key9`, `KeySpace`, `KeyEnter`, `KeyTab`, `KeyEscape`, `KeyBackspace`, `KeyDelete`, `KeyLeft`, `KeyRight`, `KeyUp`, `KeyDown`, `KeyHome`, `KeyEnd`, `KeyPageUp`, `KeyPageDown`, `KeyF1`..`KeyF12`

`ui::KeyMod` bitmask: `KeyModShift`, `KeyModControl`, `KeyModAlt`, `KeyModSuper`

### Version (`ui/core/version.h`)

```cpp
namespace ui {
constexpr int OPENGUI_VERSION_MAJOR = 0;
constexpr int OPENGUI_VERSION_MINOR = 1;
constexpr int OPENGUI_VERSION_PATCH = 0;
}
```

## GL Layer (`ui_gl`)

### Window (`ui/gl/window.h`)

```cpp
namespace ui {
class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool should_close() const;
    void poll_events();
    void swap_buffers();
    void set_vsync(int interval);

    int width() const;
    int height() const;
    GLFWwindow* handle() const;

    bool resized();          // edge-triggered, cleared after read

    float mouse_x() const;  // pixel coords, y down, (0,0) = top-left
    float mouse_y() const;
    bool mouse_down(int button) const;     // 0=left, 1=right, 2=middle
    bool mouse_pressed(int button);        // edge-triggered
    bool mouse_released(int button);       // edge-triggered
    float scroll_dx();                     // edge-triggered, cleared after read
    float scroll_dy();

    bool key_down(int key) const;          // held
    bool key_pressed(int key);             // edge-triggered
    bool key_released(int key);            // edge-triggered
    int char_pressed_count() const;        // Unicode codepoints this frame
    unsigned int char_pressed_at(int i) const;
    void clear_chars();
};
}
```

### Renderer (`ui/gl/renderer.h`)

```cpp
namespace ui {
class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool init();  // call once after GL context ready

    void begin_frame(const Vec4& clear_color);
    void set_viewport(int width, int height);

    // Drawing
    void draw_quad(const Rect& rect, const Vec4& color);
    void draw_rounded_rect(const Rect& rect, float radius, const Vec4& color);
    void draw_rect_outline(const Rect& rect, float thickness, const Vec4& color);
    void draw_rounded_rect_outline(const Rect& rect, float radius, float thickness, const Vec4& color);
    void draw_textured_quad(const Rect& rect, unsigned int texture_id, const Rect& uv,
                            const Vec4& color, float radius = 0.f, float rotation = 0.f);
    void draw_polygon(const Vec2* points, int count, const Vec4& color);

    // Clipping
    void push_scissor(const Rect& rect);  // nested, pixels y-down
    void pop_scissor();

    int viewport_width() const;
    int viewport_height() const;
    Rect current_scissor() const;  // for lazy-loading checks
};
}
```

### Texture (`ui/gl/texture.h`)

```cpp
namespace ui {
class Texture {
public:
    Texture() = default;
    ~Texture();

    bool load_from_file(const char* path);  // png/jpg/bmp/webp via stb_image
    void create(int width, int height, const void* rgba);
    void update(const void* rgba);                    // full update (video frames)
    void update_sub(int x, int y, int w, int h, const void* rgba);  // partial update
    void bind(unsigned int unit = 0) const;

    unsigned int id() const;   // GL texture id
    int width() const;
    int height() const;
};
}
```

### Font (`ui/gl/font.h`)

```cpp
namespace ui {
class Font {
public:
    Font() = default;
    ~Font();

    bool load_from_file(const char* path, float pixel_height = 32.f);
    bool load_cjk_font(const char* path, int font_index = 0);
    bool load_cjk_font_auto();  // auto-detect system CJK font
    bool load_icon_font(const char* path, float pixel_height);  // layui icons (U+E600..E7FF)

    void draw_text(Renderer& renderer, const char* text, float x, float y, const Vec4& color) const;
    float measure_text(const char* text) const;
    float char_width(unsigned int codepoint) const;

    bool icon_loaded() const;
    void draw_icon(Renderer& renderer, unsigned codepoint, float cx, float cy, float size,
                   const Vec4& color, float rotation = 0.f) const;

    float ascent() const;
    float descent() const;
    float line_height() const;
    float pixel_height() const;
    bool cjk_ready() const;
};
}
```

## UI Layer (`ui_ui`)

### Theme (`ui/ui/theme.h`)

```cpp
namespace ui {
struct Theme {
    Vec4 window_bg, button_normal, button_hover, button_pressed;
    Vec4 input_bg, input_border;
    Vec4 accent, accent_hover, accent_pressed;
    Vec4 text, text_dim;
    float radius = 6.f;
    Vec4 success, info, warning, danger, border, panel, text_secondary, text_muted;

    static Theme make_dark();
    static Theme make_light();
    static Theme make_layui();  // recommended
};
}
```

### Widget Base Class (`ui/ui/widget.h`)

```cpp
namespace ui {
class Widget {
public:
    explicit Widget(float x = 0, float y = 0, float w = 0, float h = 0);
    virtual ~Widget();

    void set_bounds(float x, float y, float w, float h);
    const Rect& bounds() const;

    bool visible = true;
    bool enabled = true;
    virtual void set_enabled(bool en);

    // CSS class names (layui syntax)
    void add_class(const char* cls);       // space-separated: "layui-btn layui-btn-primary"
    void remove_class(const char* cls);
    bool has_class(const char* cls) const;
    const std::vector<std::string>& classes() const;

    // Tree structure
    void add_child(Widget* child);
    void remove_child(Widget* child);
    void clear_children();   // recursive delete children subtree
    void destroy();          // deferred self-destruct (safe during events)
    static void flush_pending_destroys();  // call at frame end
    Widget* parent() const;
    const std::vector<Widget*>& children() const;
    void bring_to_front();

    bool contains(float x, float y) const;

    // Events (pixel coords, y down)
    virtual bool on_mouse_move(float x, float y);
    virtual bool on_mouse_down(float x, float y, int button);
    virtual bool on_mouse_up(float x, float y, int button);
    virtual bool on_mouse_scroll(float x, float y, float dx, float dy);
    void capture_mouse();
    void release_mouse_capture();

    // Dispatchers (hit-test from top child, bubble up)
    bool dispatch_mouse_move(float x, float y);
    bool dispatch_mouse_down(float x, float y, int button);
    bool dispatch_mouse_up(float x, float y, int button);
    bool dispatch_mouse_scroll(float x, float y, float dx, float dy);

    bool hovered() const;
    bool pressed() const;

    // Popups
    void open_popup(Widget* owner, Widget* popup);
    void close_popups();
    void close_topmost_popup();
    bool has_open_popups();

    // Modal
    void set_modal(Widget* modal);
    Widget* modal() const;

    // Tooltip
    void set_tooltip(const char* text);
    const std::string& tooltip() const;

    // Focus / Keyboard
    bool focusable = false;
    virtual void on_focus_changed(bool focused);
    virtual bool on_key_down(int key, int mods);
    virtual bool on_key_up(int key, int mods);
    virtual bool on_char(unsigned int codepoint);
    void focus();
    void blur();
    bool focused() const;
    bool dispatch_key_down(int key, int mods);
    bool dispatch_key_up(int key, int mods);
    bool dispatch_char(unsigned int codepoint);
    void focus_next();
    void focus_prev();
    Widget* root();

    // Layout
    virtual void on_layout();
    void layout();
    virtual float preferred_height() const;

    // Frame advance (animations, auto-close timers)
    virtual void on_advance(float dt);
    void advance(float dt);

    // Drawing
    virtual void on_draw(Renderer& renderer);
    virtual void draw(Renderer& renderer);
    virtual void draw_overlay(Renderer& renderer);
};
}
```

## Widget Reference

### Layout Widgets

#### VBox
Vertical box layout. Children stack top-to-bottom.

```cpp
class VBox : public Widget {
public:
    VBox(float x, float y, float w, float h);
    void set_spacing(float spacing);   // gap between children (pixels)
    void set_padding(float padding);   // inner padding (pixels)
    void set_theme(const Theme* theme);
    void on_layout() override;
    float preferred_height() const override;
};
```

#### HBox
Horizontal box layout. Children stack left-to-right.

```cpp
class HBox : public Widget {
public:
    HBox(float x, float y, float w, float h);
    void set_spacing(float spacing);
    void set_padding(float padding);
    void set_theme(const Theme* theme);
    void on_layout() override;
    float preferred_height() const override;
};
```

#### Grid
12-column responsive grid (layui grid system).

```cpp
class Grid : public Widget {
public:
    Grid(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
    // Use add_class("layui-col-md4") on children to set column spans
};
```

#### ScrollBox
Scrollable container with vertical scrollbar.

```cpp
class ScrollBox : public Widget {
public:
    ScrollBox(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
    void on_layout() override;
    float preferred_height() const override;
    float scroll_offset() const;
    void set_scroll_offset(float y);
};
```

#### FlowBox
Flow/wrap layout. Children wrap to next line when exceeding width.

```cpp
class FlowBox : public Widget {
public:
    FlowBox(float x, float y, float w, float h);
    void set_spacing(float spacing);
    void set_theme(const Theme* theme);
};
```

#### Panel
Basic container with optional background.

```cpp
class Panel : public Widget {
public:
    Panel(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
};
```

### Basic Controls

#### Label
Text display with optional word-wrap and alignment.

```cpp
class Label : public Widget {
public:
    Label(float x, float y, float w, float h, const char* text);
    void set_text(const char* text);
    const std::string& text() const;
    void set_wrap(bool wrap);            // auto-wrap at bounds width
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_color(const Vec4& color);   // override text color
    void set_align(int align);           // 0=left, 1=center, 2=right
    float preferred_height() const override;
};
```

#### Button
Clickable button with hover/pressed states and layui variants.

```cpp
class Button : public Widget {
public:
    Button(float x, float y, float w, float h, const char* text);
    void set_text(const char* text);
    const std::string& text() const;
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_icon(const char* icon_name);   // layui icon (e.g. "left", "search")
    void set_badge(const char* text);       // embedded badge (e.g. "12")
    void set_dot_badge();                   // 8px red dot
    void set_on_click(std::function<void()> cb);
    float preferred_height() const override;
};
```

**Layui button classes**: `layui-btn` (default), `layui-btn-primary`, `layui-btn-normal`, `layui-btn-warm`, `layui-btn-danger`, `layui-btn-disabled`, `layui-btn-lg`, `layui-btn-sm`, `layui-btn-xs`, `layui-btn-radius`, `layui-btn-fluid`.

#### ButtonGroup
Groups buttons with connected appearance.

```cpp
class ButtonGroup : public Widget {
public:
    ButtonGroup(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
    // ButtonGroup automatically manages group_role on child Buttons
};
```

#### Icon
Displays a layui icon (190 icons).

```cpp
class Icon : public Widget {
public:
    Icon(float x, float y, float w, float h, unsigned codepoint);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_size(float size);
};
```

#### Image
Image display with lazy loading support (stb_image decode).

```cpp
class Image : public Widget {
public:
    Image(float x, float y, float w, float h);
    bool load_from_file(const char* path);  // png/jpg/bmp/webp
    void set_lay_src(const char* path);     // lazy-load: decode only when visible
    bool loaded() const;
    float preferred_height() const override;
};
```

#### Badge
Badge/dot indicator.

```cpp
class Badge : public Widget {
public:
    Badge(float x, float y, float w, float h, const char* text);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_dot(bool dot);   // dot mode vs text mode
};
```

#### Progress
Horizontal progress bar.

```cpp
class Progress : public Widget {
public:
    Progress(float x, float y, float w, float h);
    void set_percent(float p);   // 0..100
    void set_theme(const Theme* theme);
    // add_class("layui-progress") for layui style
};
```

#### Circle
Circular progress indicator.

```cpp
class Circle : public Widget {
public:
    Circle(float x, float y, float w, float h);
    void set_percent(float p);   // 0..100
    void set_theme(const Theme* theme);
};
```

#### Switch
Toggle switch (on/off).

```cpp
class Switch : public Widget {
public:
    Switch(float x, float y, float w, float h);
    void set_checked(bool on);
    bool checked() const;
    void set_on_change(std::function<void(bool)> cb);
    void set_theme(const Theme* theme);
};
```

#### HLine
Horizontal separator line.

```cpp
class HLine : public Widget {
public:
    HLine(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
};
```

#### Fieldset / FieldsetTitle
Fieldset container with title.

```cpp
class Fieldset : public Widget {
public:
    Fieldset(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
};

class FieldsetTitle : public Widget {
public:
    FieldsetTitle(float x, float y, float w, float h, const char* text);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### BlockQuote
Quote block.

```cpp
class BlockQuote : public Widget {
public:
    BlockQuote(float x, float y, float w, float h);
    void set_text(const char* text);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Card
Card container.

```cpp
class Card : public Widget {
public:
    Card(float x, float y, float w, float h);
    void set_theme(const Theme* theme);
};
```

#### CountdownText
Real-time countdown display.

```cpp
class CountdownText : public Widget {
public:
    CountdownText(float x, float y, float w, float h);
    void set_target(std::chrono::steady_clock::time_point target);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

### Form Controls

#### TextBox
Single-line text input with cursor, scrolling, and password mode.

```cpp
class TextBox : public Widget {
public:
    TextBox(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_text(const char* text);
    const std::string& text() const;
    void set_placeholder(const char* text);  // gray hint when empty
    void set_password(bool on);              // mask with '*'
    void set_on_change(std::function<void(const char*)> cb);
    void set_on_enter(std::function<void()> cb);
    void set_caret_pos(size_t pos);
};
```

#### TextArea
Multi-line text input.

```cpp
class TextArea : public Widget {
public:
    TextArea(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_text(const char* text);
    const std::string& text() const;
    void set_placeholder(const char* text);
    void set_on_change(std::function<void(const char*)> cb);
};
```

#### CheckBox
Checkbox control.

```cpp
class CheckBox : public Widget {
public:
    CheckBox(float x, float y, float w, float h, const char* text);
    void set_checked(bool on);
    bool checked() const;
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(bool)> cb);
};
```

#### RadioButton
Single radio button.

```cpp
class RadioButton : public Widget {
public:
    RadioButton(float x, float y, float w, float h, const char* text);
    void set_checked(bool on);
    bool checked() const;
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_group(int group_id);
};
```

#### RadioGroup
Mutually exclusive radio button group.

```cpp
class RadioGroup : public Widget {
public:
    RadioGroup(float x, float y, float w, float h);
    void add_option(const char* text);
    int selected() const;
    void set_selected(int index);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(int)> cb);
};
```

#### ComboBox
Dropdown select.

```cpp
class ComboBox : public Widget {
public:
    ComboBox(float x, float y, float w, float h);
    void add_option(const char* text);
    int selected() const;
    void set_selected(int index);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(int)> cb);
};
```

#### Slider
Range slider (single handle, range, or vertical).

```cpp
class Slider : public Widget {
public:
    Slider(float x, float y, float w, float h);
    void set_range(float min_val, float max_val);
    void set_value(float val);
    float value() const;
    void set_vertical(bool on);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(float)> cb);
};
```

#### Rate
Star rating.

```cpp
class Rate : public Widget {
public:
    Rate(float x, float y, float w, float h);
    void set_count(int count);    // number of stars
    void set_value(float val);    // current rating
    float value() const;
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(float)> cb);
};
```

#### ColorPicker
Color picker.

```cpp
class ColorPicker : public Widget {
public:
    ColorPicker(float x, float y, float w, float h);
    Vec4 color() const;
    void set_color(const Vec4& c);
    void set_on_change(std::function<void(const Vec4&)> cb);
    void set_theme(const Theme* theme);
};
```

#### Upload
File upload button.

```cpp
class Upload : public Widget {
public:
    Upload(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_upload(std::function<void(const char* path)> cb);
};
```

#### Laydate / LaydateBox
Date/time picker.

```cpp
class Laydate : public Widget {
public:
    Laydate(float x, float y, float w, float h);
    void set_type(const char* type);  // "date", "time", "datetime"
    void set_value(const char* value);
    std::string value() const;
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(const char*)> cb);
};

class LaydateBox : public Widget {
public:
    LaydateBox(float x, float y, float w, float h);
    void set_type(const char* type);
    void set_value(const char* value);
    std::string value() const;
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_change(std::function<void(const char*)> cb);
};
```

### Navigation Widgets

#### Tabs
Tab panel with content switching.

```cpp
class Tabs : public Widget {
public:
    Tabs(float x, float y, float w, float h);
    void add_tab(const char* title, Widget* content);
    int selected() const;
    void set_selected(int index);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    // add_class("layui-tab layui-tab-brief") for layui styles
};
```

#### MenuBar
Horizontal menu bar.

```cpp
class MenuBar : public Widget {
public:
    MenuBar(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### NavBar
Navigation bar.

```cpp
class NavBar : public Widget {
public:
    NavBar(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### NavMenu
Sidebar navigation menu.

```cpp
class NavMenu : public Widget {
public:
    NavMenu(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Menu
Vertical menu.

```cpp
class Menu : public Widget {
public:
    Menu(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Dropdown
Dropdown menu.

```cpp
class Dropdown : public Widget {
public:
    Dropdown(float x, float y, float w, float h);
    void add_item(const char* text);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_select(std::function<void(int)> cb);
};
```

#### Tree
Tree view.

```cpp
class Tree : public Widget {
public:
    Tree(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Breadcrumb
Breadcrumb navigation.

```cpp
class Breadcrumb : public Widget {
public:
    Breadcrumb(float x, float y, float w, float h);
    void add_item(const char* text);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Collapse
Accordion/collapsible sections.

```cpp
class Collapse : public Widget {
public:
    Collapse(float x, float y, float w, float h);
    void add_panel(const char* title, Widget* content);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Laypage
Pagination.

```cpp
class Laypage : public Widget {
public:
    Laypage(float x, float y, float w, float h);
    void set_pages(int total_pages);
    void set_current(int page);  // 1-based
    void set_theme(const Theme* theme);
    void set_on_page(std::function<void(int)> cb);
};
```

### Data Display Widgets

#### Table
Data table with sorting, pagination, inline editing, checkbox/radio columns, summary row, and action buttons.

```cpp
class Table : public Widget {
public:
    Table(float x, float y, float w, float h);
    void add_column(const char* title, float width, bool sortable = false);
    void add_row(const std::vector<std::string>& cells);
    void clear_rows();

    void set_skin(TableSkin skin);   // Default, Line, Row, Nob
    void set_size(TableSize size);   // Normal, Lg, Sm
    void set_even(bool even);        // alternating row colors

    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_sort(std::function<void(int col, bool asc)> cb);
    void set_on_row_click(std::function<void(int row)> cb);

    // Pagination
    void set_paginated(bool on, int page_size = 10);
    void set_page(int page);  // 0-based

    // Selection columns
    void add_checkbox_column();
    void add_radio_column();
    void add_numbers_column();
    bool row_checked(int visible_row) const;
    int checked_radio() const;
    int checked_count() const;

    // Cell form columns
    void set_cell_switch(int col);
    void set_cell_checkbox(int col);

    // Summary row
    void set_total_row(bool on);
    void set_total_col(int col);

    // Action column (inline buttons)
    void add_button_column(const char* title, float width,
                           const std::vector<std::string>& labels,
                           const std::vector<std::string>& events = {});
    void set_on_action(std::function<void(int row, const std::string& event)> cb);

    void reload(const std::vector<std::vector<std::string>>& new_rows);
    void set_init_sort(int col, bool asc);
    void set_on_row_double_click(std::function<void(int visible_row)> cb);
};
```

#### Timeline
Timeline display.

```cpp
class Timeline : public Widget {
public:
    Timeline(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Transfer
Shuttle/transfer list.

```cpp
class Transfer : public Widget {
public:
    Transfer(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Carousel
Image/content carousel.

```cpp
class Carousel : public Widget {
public:
    Carousel(float x, float y, float w, float h);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

#### Code
Syntax-highlighted code block.

```cpp
class Code : public Widget {
public:
    Code(float x, float y, float w, float h);
    void set_text(const char* text);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
};
```

### Container Widgets

#### PanelWindow
Draggable/resizable floating window with title bar, close/maximize buttons, shade, footer buttons, and auto-close timer.

```cpp
class PanelWindow : public Widget {
public:
    PanelWindow(float x, float y, float w, float h, const char* title);
    void set_title(const char* title);
    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_close(std::function<void()> cb);

    void set_title_visible(bool v);
    void set_close_visible(bool v);
    void set_maxmin(bool v);                              // maximize/restore buttons
    void set_shade(float alpha);                          // 0..1 backdrop opacity
    void set_footer_buttons(const std::vector<std::string>& labels, int align,
                            std::function<void(int)> cb); // align: 0=left, 1=center, 2=right
    void set_auto_close(float seconds);                   // <=0 to disable

    Rect client_rect() const;  // area below title bar
    float footer_height() const;
};
```

### Media Widgets (optional, requires build flags)

#### VideoView
Video player (requires `OPENGUI_MEDIA_FFMPEG=ON`). Header: `ui/media/video_view.h`

#### ImageView
Image viewer (requires `OPENGUI_MEDIA_OPENCV=ON`). Header: `ui/media/image_view.h`

#### AudioOut
Audio output via winmm (requires `OPENGUI_MEDIA_FFMPEG=ON`). Header: `ui/media/audio_out.h`

## Umbrella Header

```cpp
#include "ui/all.h"  // includes everything except media modules
```

Media headers must be included separately:
```cpp
#include "ui/media/video_view.h"  // FFmpeg module
#include "ui/media/image_view.h"  // OpenCV module
```

## CMake Integration

```cmake
find_package(opengl_ui REQUIRED)

# Basic (core + gl + ui)
target_link_libraries(my_app PRIVATE opengl_ui::ui_ui)

# With FFmpeg (built with OPENGUI_MEDIA_FFMPEG=ON)
target_link_libraries(my_app PRIVATE opengl_ui::ui_media_ffmpeg)

# With OpenCV (built with OPENGUI_MEDIA_OPENCV=ON)
target_link_libraries(my_app PRIVATE opengl_ui::ui_media_opencv)
```

## Runtime Assets

| File | Location | Required By |
|------|----------|-------------|
| `glfw3.dll` | `install/bin/` | Always |
| `iconfont.ttf` | `install/share/opengl_ui/` | Demo apps using icons |
| `avcodec-59.dll` etc. | `install/bin/` | FFmpeg module |
| `opencv_world452.dll` | `install/bin/` | OpenCV module |

## Lifecycle & Precondition Contract

这些前置条件是**库的正确性依赖**。违反会在运行期**静默**复现历史 Bug，不报错（Debug 构建下部分有 `assert` 拦截，见文末）。推荐用 `ui::WindowContainer` + `Scene::run()/run_for()`——它们自动满足下表全部契约。

| 前置条件 | 强制 API | 违反的后果 |
|---|---|---|
| 每帧在绘制前调用 `Renderer::begin_frame()` | `Renderer::begin_frame` | scissor 状态不重置，跨帧脏裁剪会把内容画到错误区域 → **滚动内容穿透标题栏/Header（UI-169）**；清屏不执行 |
| `push_scissor`/`pop_scissor` 配对调用 | `Renderer::push_scissor` / `pop_scissor` | 未配平 pop 弹出父级裁剪区、过早关闭 `GL_SCISSOR_TEST` → 内容未裁剪绘制。退化矩形（w/h≤0）push 静默不入栈、pop 幂等，已实现自动防护 |
| 帧末调用根节点 `advance(dt)` 或 `Widget::flush_pending_destroys()` | `Widget::advance`/`flush_pending_destroys` | `destroy()` 的控件**永不释放（内存泄漏）**，仍以「隐藏但未删」状态保留（UI-161/167）。`Scene::frame` 自动调用 |
| `Scene::set_chrome()` 在**首帧 `frame()` 之前**调用（构造后 / Ready 阶段） | `Scene::set_chrome` | `content_origin_y()` 首帧后改值，已排布内容不会重排，顶部装帧层与内容区静默错位（UI-153/168） |
| 窗口/渲染器存活期 ≥ 绑定它的 `Scene` | `Scene` 构造 | Scene 持引用；提前析构 Window/Renderer 则 `~Scene` 访问悬空引用 |
| 同一 GL 上下文内**禁止**运行两个 `Renderer` | 所有 `Renderer` | 渲染器实例级缓存（active_program_/scissor_stack_）冲突（多窗口应每窗口独立 GL 上下文） |
| `Renderer::init()` 需在 GL 上下文就绪后调用一次 | `Renderer::init` | 未 init 即 draw → GL 对象未创建/空程序 |
| 拖拽须严格 `start→update→end` 序列 | `Window::start_drag/update_drag/end_drag` | 拖拽位移错误/跳变（UI-167: 非 Windows 分支 drag_off） |
| 尺寸变化后重排：`resized()` → `set_viewport` + `root.set_bounds` + relayout | `Window::resized`/`Renderer::set_viewport` | 视口/投影矩阵与窗口不同步 → 内容歪斜或裁切。`Scene::frame` 已处理 |

### Debug 断言（编译期仅在 Debug 断言开启，违反即 abort，用于尽早暴露下述契约被破坏）

- `Scene::set_chrome` 在首帧后调用 → `assert` 触发；
- `Renderer::begin_frame` 进入时 scissor 栈非空（上一帧未配平 pop，或本帧提前 push）→ `assert` 触发；
- `Widget::flush_pending_destroys` 被重入（flush 进行中再次 flush）→ `assert` 触发。

### 消费者侧链接/构建要求

- **C++17 起**：库头为标准库 C++17 用法。`ui_core` 已通过 `target_compile_features(PUBLIC cxx_std_17)` 导出，链接后 CMake 会自动启用（手写编译命令需自理）。
- **`GLFW_INCLUDE_NONE`**：已由 `ui_gl` 以 PUBLIC 编译定义导出，下游在自身 TU 同时 include `<glad/gl.h>` 与 `<GLFW/glfw3.h>` 时自动生效，避免系统 `<gl/gl.h>` 与 GLAD 冲突。
- **静态库**：`ui_*` 均为静态库，链接时下游的运行时 / `_ITERATOR_DEBUG_LEVEL` / toolset 必须与库一致，否则 MSVC 报 `LNK2038 mismatch` 类错误（库自身无法校验）。
