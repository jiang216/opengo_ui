#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
class Texture;
struct Theme;

// 图标+文本标签（UI-xxx）：左侧图标/图片 + 右侧文本，水平排列，垂直居中。
// 支持两种图标源：layui 图标字体（set_icon）或本地图片文件（set_image）。
// 无交互（默认）或可点击（set_on_click）；可配置图标尺寸、间距、对齐。
class IconLabel : public Widget {
public:
    IconLabel(float x, float y, float w, float h, const char* text = "");

    // ---- 文本 ----
    void set_text(const char* text) {
        std::string next = text ? text : "";
        if (text_ == next) return;
        text_ = std::move(next);
        mark_dirty();
    }
    const std::string& text() const { return text_; }

    // ---- layui 图标 ----
    void set_icon(const char* icon_name);
    void clear_icon() { if (icon_cp_ == 0) return; icon_cp_ = 0; mark_dirty(); }

    // ---- 本地图片（png/jpg/bmp/webp）----
    void set_image(const char* path);
    void clear_image();

    // ---- 布局参数 ----
    void set_icon_size(float s) { if (icon_size_ == s) return; icon_size_ = s; mark_dirty(); }
    void set_gap(float g) { if (gap_ == g) return; gap_ = g; mark_dirty(); }
    // 0=左对齐（默认） 1=水平居中 2=右对齐（文本相对整个控件）
    void set_align(int a) {  align_ = a; mark_dirty(); }

    // ---- 外观 ----
    void set_font(Font* f) {  font_ = f; mark_dirty(); }
    void set_theme(const Theme* t) {  theme_ = t; mark_dirty(); }
    void set_text_color(const Vec4& c) {  text_color_ = c; has_text_color_ = true; mark_dirty(); }
    void set_icon_color(const Vec4& c) {  icon_color_ = c; has_icon_color_ = true; mark_dirty(); }

    // ---- 交互（可选）----
    void set_on_click(std::function<void()> cb) { on_click_ = std::move(cb); }

    bool on_mouse_up(float x, float y, int button) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_key_down(int key, int mods) override;

    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    unsigned icon_cp_ = 0;       // layui 图标码点
    Texture* image_tex_ = nullptr; // 本地图片纹理
    bool image_loaded_ = false;

    float icon_size_ = 16.f;     // 图标显示尺寸
    float gap_ = 6.f;            // 图标与文本间距
    int align_ = 0;              // 0=左 1=中 2=右

    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Vec4 text_color_{1.f, 1.f, 1.f, 1.f};
    bool has_text_color_ = false;
    Vec4 icon_color_{1.f, 1.f, 1.f, 1.f};
    bool has_icon_color_ = false;

    std::function<void()> on_click_;
};

}  // namespace ui
