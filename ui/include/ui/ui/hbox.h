#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Renderer;
struct Theme;

// 子控件纵向对齐方式：Stretch 拉伸填满容器（M12 默认）；Center 按各自 preferred_height
// 垂直居中（layui 行内按钮 vertical-align:middle 语义，尺寸类按钮展示实际高度差）
enum class Valign {
    Stretch,
    Center,
};

// 横向布局容器（M12）：子控件按添加顺序从左到右排列（宽度用各自预设值，高度按对齐方式）
class HBox : public Widget {
public:
    HBox(float x, float y, float w, float h);

    void set_spacing(float spacing) { spacing_ = spacing; }
    void set_padding(float padding) { padding_ = padding; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    void set_valign(Valign valign) { valign_ = valign; }
    // 覆盖背景填充色（默认 theme->button_normal，如 layui-bg-gray 灰底 #fafafa）；传 a<0 不覆盖
    void set_fill(const Vec4& color) { fill_ = color; fill_.a = -1.f; }

    void on_layout() override;
    void on_draw(Renderer& renderer) override;
    // h<=0（自适应）：取可见子控件 preferred_height 最大值 + 2*padding；
    // h>0 时显式高度优先（返回 bounds_.h）。使 HBox 可直接投入 ScrollBox/VBox 流式排布。
    float preferred_height() const override;

private:
    float spacing_ = 8.f;
    float padding_ = 8.f;
    Valign valign_ = Valign::Stretch;
    const Theme* theme_ = nullptr;
    Vec4 fill_{0, 0, 0, -1.f};
};

}  // namespace ui
