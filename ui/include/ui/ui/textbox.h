#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 单行文本输入框（M4）：可聚焦，字符输入 / 退格 / Delete / 左右移 / Home / End，
// 聚焦时显示光标（acc2 竖线），文本超宽自动横向滚动保证光标可见
// UI-101：Shift+方向/Home/End 选择、鼠标拖选、Ctrl+A/C/X/V 全选/复制/剪切/粘贴，
// 选中区间高亮；Backspace/Delete/输入字符优先删除选中区间
class TextBox : public Widget {
public:
    TextBox(float x, float y, float w, float h);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    void set_text(const char* text);
    const std::string& text() const { return text_; }

    // 空文本时绘制灰色占位提示（不影响输入内容）
    void set_placeholder(const char* text) { placeholder_ = text ? text : ""; }
    // 密码模式：显示 '*' 掩码，光标宽度按掩码计算
    void set_password(bool on) { password_ = on; }

    // 文本变化（含程序 set_text）时回调
    void set_on_change(std::function<void(const char*)> cb) { on_change_ = std::move(cb); }
    // 按 Enter 时回调（M17c：Table 行内编辑提交）
    void set_on_enter(std::function<void()> cb) { on_enter_ = std::move(cb); }

    // 设置光标字节位置（Table::begin_edit 初始定位 + 点击定位）
    void set_caret_pos(size_t pos);

    // 文本起点 -> caret 的像素宽度（IME 锚定等外部用途）
    float caret_offset() const;

    // ---- 选择 / 剪贴板（UI-101）----
    bool has_selection() const { return sel_anchor_ != caret_; }
    // 选中区间文本（字节下标 [min(anchor,caret), max)；密码框返回原文）
    std::string selected_text() const;
    void select_all() { sel_anchor_ = 0; caret_ = text_.size(); }
    void clear_selection() { sel_anchor_ = caret_; }

    bool on_key_down(int key, int mods) override;
    bool on_char(unsigned int codepoint) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    // caret 为字节下标；前后移一个 UTF-8 字符
    void move_caret(int delta);
    // backspace=true 删光标前，false 删光标后
    void erase_cp(bool backspace);
    // 删除选中区间（caret 落区间起点并清除选择）
    void erase_selection();
    // 选中区间 -> 系统剪贴板
    void copy_selection() const;
    // 剪贴板 -> 插入点（替换选中；单行框换行折叠为空格）
    void paste_clipboard();
    // 屏幕点 -> 光标字节下标（点击定位 / 拖选共用）
    size_t hit_test_caret(float x) const;

    std::string text_;
    std::string placeholder_;
    bool password_ = false;
    size_t caret_ = 0;
    size_t sel_anchor_ = 0;   // 选择锚点（与 caret_ 构成选中区间）
    bool dragging_ = false;   // 鼠标拖选中
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(const char*)> on_change_;
    std::function<void()> on_enter_;
};

}  // namespace ui
