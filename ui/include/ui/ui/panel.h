#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Renderer;
struct Theme;

// 圆角面板：容器背景（可承载子控件）。layui-bg-gray 灰底容器用 set_fill(#fafafa)。
class Panel : public Widget {
public:
    Panel(float x, float y, float w, float h);

    void set_theme(const Theme* theme) { theme_ = theme; }
    // 覆盖背景填充色（默认 theme->button_normal）；传 a<0 表示不覆盖
    void set_fill(const Vec4& color) { fill_ = color; fill_.a = -1.f; }

    void on_draw(Renderer& renderer) override;

private:
    const Theme* theme_ = nullptr;
    Vec4 fill_{0, 0, 0, -1.f};
};

}  // namespace ui
