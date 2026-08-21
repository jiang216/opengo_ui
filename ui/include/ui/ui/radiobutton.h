#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 单选按钮（M9）：圆环 + 选中圆点 + 文本；选中操作交给外部（RadioGroup 独占互斥）
class RadioButton : public Widget {
public:
    RadioButton(float x, float y, float w, float h, const char* text);

    void set_text(const char* text) { text_ = text ? text : ""; }
    const std::string& text() const { return text_; }
    void set_checked(bool checked);
    bool checked() const { return checked_; }

    // 激活（点击/空格）回调：由 RadioGroup 统一做互斥选择
    void set_on_activate(std::function<void()> cb) { on_activate_ = std::move(cb); }
    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    bool on_mouse_down(float x, float y, int button) override;
    bool on_key_down(int key, int mods) override;
    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    bool checked_ = false;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void()> on_activate_;
};

}  // namespace ui
