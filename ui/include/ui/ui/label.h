#pragma once

#include "ui/ui/widget.h"

#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 文本标签：单行文本，可设置字体/颜色/对齐；M11 支持按宽度自动换行（多行）
class Label : public Widget {
public:
    Label(float x, float y, float w, float h, const char* text);

    void set_text(const char* text);
    const std::string& text() const { return text_; }

    // 开启按控件宽度自动换行（bounds_.w 为行宽）
    void set_wrap(bool wrap);
    bool wrap() const { return wrap_; }

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    void set_color(const Vec4& color) {  color_ = color; color_set_ = true; mark_dirty(); }

    // 0=左对齐 1=水平居中 2=右对齐
    void set_align(int align) {  align_ = align; mark_dirty(); }

    // 字号缩放：相对 Font 烘培像素号的倍率（1 = 原大），行高/换行/度量随缩放；
    // 图集 2x 过采样，≤2x 清晰，更大倍率发虚
    void set_text_scale(float s) { 
        if (s > 0.f && s != text_scale_) { text_scale_ = s; mark_dirty(); }
    }
    float text_scale() const { return text_scale_; }

    // 仿粗体：字形沿 x 错位叠加一次（偏移 ≈ 字号/16，随缩放同步）
    void set_bold(bool b) { 
        if (b != bold_) { bold_ = b; mark_dirty(); }
    }
    bool bold() const { return bold_; }

    // 文本行高（ascent - descent）；换行时为行数 × 行高
    float preferred_height() const override;

    void on_draw(Renderer& renderer) override;

private:
    // 按当前 bounds_.w 把 text_ 切分为多行（UTF-8 逐码点）
    void recompute_lines();

    std::string text_;
    std::vector<std::string> lines_;
    bool wrap_ = false;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Vec4 color_{1.f, 1.f, 1.f, 1.f};
    bool color_set_ = false;
    int align_ = 0;
    float text_scale_ = 1.f;
    bool bold_ = false;
};

}  // namespace ui
