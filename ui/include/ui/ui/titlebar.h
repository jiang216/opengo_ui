#pragma once

#include "ui/ui/widget.h"
#include "ui/core/math.h"

#include <functional>
#include <string>

namespace ui {
class Renderer;
struct Theme;
class Font;
class Window;

// 系统标题栏风格自绘标题条（无边框窗口拖动）：深色底 + 标题文字 + 左上角应用图标 + 右侧
// 最小化 / 最大化(还原) / 关闭 三键。
//
// 归因（AGENTS）：无边框窗口拖动需要自绘标题栏机制，属 ui 库能力缺口，故做成
// 可复用控件（UI-148）。on_mouse_down 于条内空白区（非任一窗口键）→ window_->start_drag()；
// on_mouse_move（拖动中）→ window_->update_drag()；on_mouse_up → window_->end_drag()——
// Win32 下 Window::update_drag 用 GetCursorPos 计算增量，随窗口平移动量归位。
// 最小化最小 → window_->iconify()；最大化/还原 切换 window_->maximize()/restore()；
// 关闭钮命中 → on_close_ 回调（宿主注入 request_exit 退出主窗口 / 关闭子窗口）。
//
// window_ 可为空：此时仅绘制标题条、不做窗口操作（纯装饰用途）。
class TitleBar : public Widget {
public:
    TitleBar(float w, float h, const char* title = "", Window* win = nullptr);

    void set_font(Font* f) { font_ = f; }
    void set_theme(const Theme* t) { theme_ = t; }
    void set_title(const char* title);
    // 左上角应用图标：font 用 Font::draw_icon 画 codepoint（iconfont 已有码点，如 layui_icon::more）
    void set_icon(Font* font, uint32_t codepoint);
    void set_on_close(std::function<void()> cb) { on_close_ = std::move(cb); }
    void set_on_minimize(std::function<void()> cb) { on_minimize_ = std::move(cb); }
    void set_on_maximize(std::function<void()> cb) { on_maximize_ = std::move(cb); }
    // 右侧窗口键显隐（默认三键全显）
    void set_minimize_visible(bool v) { minimize_button_ = v; mark_dirty(); }
    void set_maximize_visible(bool v) { maximize_button_ = v; mark_dirty(); }
    void set_close_visible(bool v) { close_button_ = v; mark_dirty(); }
    bool close_visible() const { return close_button_; }

    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    enum class Btn { None, Minimize, Maximize, Close };
    Btn hit_button(float x, float y) const;
    void trigger(Btn b);

    std::string title_;
    Window* window_ = nullptr;
    Font* font_ = nullptr;
    Font* icon_font_ = nullptr;
    uint32_t icon_cp_ = 0;
    const Theme* theme_ = nullptr;
    bool dragging_ = false;
    Btn pressed_ = Btn::None;   // 当前按住的窗口键（拖拽与按键互斥）
    Btn hover_btn_ = Btn::None; // 当前悬停的窗口键（仅 hover 反馈）
    bool minimize_button_ = true;
    bool maximize_button_ = true;
    bool close_button_ = true;
    std::function<void()> on_close_;
    std::function<void()> on_minimize_;
    std::function<void()> on_maximize_;
};

}  // namespace ui
