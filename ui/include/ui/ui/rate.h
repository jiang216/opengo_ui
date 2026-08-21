#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Renderer;
class Font;
struct Theme;

// 评分（M17a，layui-rate 克隆）：五角星，星色 #ffb800，hover 预览 + 点击取值，
// 悬停星放大 scale(1.12)。支持 length 星数、half 半星、text 文本/setText 回调、
// theme 自定义色、readonly 只读（UI-088 补全 layui-rate 能力）。
class Rate : public Widget {
public:
    Rate(float x, float y, float w, float h);

    // 当前值 0..length（half 开启时可为 x.5）
    void set_value(float v);
    // 向后兼容：返回整数部分
    int value() const { return (int)value_; }
    float value_f() const { return value_; }

    // 星数（layui length，默认 5）
    void set_length(int n) { length_ = n > 0 ? n : 1; }
    int length() const { return length_; }

    // 半星（layui half）：允许 x.5 取值，绘制半颗星
    void set_half(bool half) { half_ = half; }
    bool half() const { return half_; }

    // 文本（layui text）：星后显示数值文本；text 回调自定义（layui setText）
    void set_text(bool text) { text_ = text; }
    bool text() const { return text_; }
    void set_text_callback(std::function<std::string(float)> cb) { text_cb_ = std::move(cb); }

    // 自定义主题色（layui theme）
    void set_theme_color(const Vec4& c) { theme_color_ = c; has_theme_color_ = true; }

    void set_readonly(bool readonly) { readonly_ = readonly; }
    bool readonly() const { return readonly_; }

    void set_theme(const Theme* theme) { theme_ = theme; }
    void set_font(Font* font) { font_ = font; }
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    bool on_mouse_move(float x, float y) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_up(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    // 第 i 颗星（0..length-1）的 x 范围
    float star_x(int i) const;
    int star_at(float x) const;
    // 半星：返回点击落到左半（返回 true 表示应取 x.5）
    bool star_half_at(float x, int idx) const;

    float value_ = 0.f;
    int length_ = 5;
    bool half_ = false;
    bool text_ = false;
    bool readonly_ = false;
    bool has_theme_color_ = false;
    Vec4 theme_color_{1.f, 0.72f, 0.f, 1.f};
    int hover_ = -1;  // -1 表示无悬停
    bool pressed_click_ = false;
    const Theme* theme_ = nullptr;
    Font* font_ = nullptr;
    std::function<void(int)> on_change_;
    std::function<std::string(float)> text_cb_;
};

}  // namespace ui
