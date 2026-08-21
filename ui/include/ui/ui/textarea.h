#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

struct LineSpan {
    size_t start;
    size_t end;
};

// 多行文本输入框（M4 扩展）：Enter 换行、↑↓/Home/End/退格/Delete、按宽度逐字符自动换行，
// 超宽纵向滚动保证光标可见；空文本时显示 placeholder 灰字提示。
//
// 选区扩展：支持鼠标拖拽选择、Ctrl+A 全选、Ctrl+C/X 剪贴板、Ctrl+V 粘贴、
// Shift+方向键扩展选区、选区绘制高亮、只读模式（readonly）。
class TextArea : public Widget {
public:
    TextArea(float x, float y, float w, float h);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    void set_placeholder(const char* text) { placeholder_ = text ? text : ""; }

    void set_text(const char* text);
    const std::string& text() const { return text_; }
    void set_caret(size_t pos) { caret_ = (pos <= text_.size()) ? pos : text_.size(); }
    // 文本变化（含程序 set_text）时回调
    void set_on_change(std::function<void(const char*)> cb) { on_change_ = std::move(cb); }

    // ---- 只读模式（日志显示用）----
    void set_readonly(bool v) { readonly_ = v; }
    bool readonly() const { return readonly_; }

    // ---- 剪贴板回调（可选覆盖；未注入时使用内置 GLFW 剪贴板，UI-103）----
    void set_on_copy(std::function<void(const std::string&)> cb) { on_copy_ = std::move(cb); }
    void set_on_paste(std::function<std::string()> cb) { on_paste_ = std::move(cb); }

    // ---- 选区查询 ----
    bool has_selection() const { return has_selection_; }
    std::string selected_text() const;
    // 选区范围（字节偏移，a <= b）
    void selection_range(size_t& a, size_t& b) const;

    bool on_key_down(int key, int mods) override;
    bool on_char(unsigned int codepoint) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    bool on_mouse_scroll(float x, float y, float dx, float dy) override;
    void on_draw(Renderer& renderer) override;

private:
    void erase_cp(bool backspace);
    void ensure_caret_visible(float line_h, size_t row);
    // 由点击点（控件内坐标）计算 caret 字节下标
    size_t caret_from_point(float local_y, float local_x) const;
    // 由 byte offset 计算 screen 坐标（控件内 y）
    float offset_to_y(size_t offset, float line_h,
                      const std::vector<LineSpan>& lines) const;
    // 选区删除（Backspace/Delete/输入时调用）
    void erase_selection();
    // 替换选区（粘贴或字符输入）
    void replace_selection(const std::string& repl);
    // 将 scroll_ clamp 到有效范围
    void clamp_scroll(float line_h);

    std::string text_;
    std::string placeholder_;
    size_t caret_ = 0;
    float scroll_ = 0.f;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(const char*)> on_change_;

    // ---- 选区 ----
    size_t sel_start_ = 0, sel_end_ = 0, sel_anchor_ = 0;
    bool selecting_ = false;      // 鼠标拖拽中
    bool has_selection_ = false;

    // ---- 只读 ----
    bool readonly_ = false;

    // ---- 拖拽自动滚屏 ----
    float scroll_margin_ = 24.f;

    // ---- 剪贴板回调 ----
    std::function<void(const std::string&)> on_copy_;
    std::function<std::string()> on_paste_;
};

}  // namespace ui
