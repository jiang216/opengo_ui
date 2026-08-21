#pragma once

#include "ui/ui/widget.h"

#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 卡片（M17a，layui-card 克隆）：白底 radius2 + 阴影 0 1px 2px 0 rgba(0,0,0,.05)。
// 结构：可选 header（layui-card-header，padding 10 15、下边框 #f8f8f8、色 #333）+
// 子控件纵向排布（layui-card-body，padding 10 15）。
class Card : public Widget {
public:
    Card(float x, float y, float w, float h);

    void set_header(const char* text);
    const std::string& header() const { return header_; }

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    // 子控件间距（body 内）
    void set_spacing(float spacing) { spacing_ = spacing; }
    // 可选描边（layui 图标列表等 1px 边框卡片）；a<=0 关闭
    void set_outline(const Vec4& color, float thickness = 1.f) {
        outline_ = color;
        outline_thickness_ = thickness;
    }

    void on_layout() override;
    float preferred_height() const override;
    void on_draw(Renderer& renderer) override;

private:
    std::string header_;
    float spacing_ = 10.f;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Vec4 outline_{0, 0, 0, -1.f};  // 默认无描边
    float outline_thickness_ = 1.f;
};

}  // namespace ui
