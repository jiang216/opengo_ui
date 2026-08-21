#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Font;
class Renderer;
struct Theme;

// 图标控件（M17d，layui-icon 克隆）：显示单个私用区图标码点。
// 用法：add_class("layui-icon layui-icon-star") 或 set_icon("star")，字体用 load_icon_font 加载。
class Icon : public Widget {
public:
    Icon(float x, float y, float w, float h, const char* icon_name = "");

    // 直接设图标名（不含 layui-icon- 前缀），找不到则清空
    void set_icon(const char* name);
    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    void set_color(const Vec4& c) { color_ = c; has_color_ = true; }

    void on_draw(Renderer& renderer) override;

private:
    unsigned icon_cp_ = 0;  // 私有区码点（0 表示无图标）
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Vec4 color_{1, 1, 1, 1};
    bool has_color_ = false;
};

}  // namespace ui
