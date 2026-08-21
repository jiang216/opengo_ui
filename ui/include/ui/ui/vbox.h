#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Renderer;
struct Theme;

// 纵向布局容器：子控件按添加顺序从上到下排列（M2 布局）
class VBox : public Widget {
public:
    VBox(float x, float y, float w, float h);

    // 子控件之间的间距（像素）
    void set_spacing(float spacing) { spacing_ = spacing; }
    // 内部边距（像素）
    void set_padding(float padding) { padding_ = padding; }

    // 布局：把子控件按顺序纵向排列；子控件高度用各自 bounds_.h（可通过 set_bounds 预设）
    void on_layout() override;
    // 内容高度 = 子控件高度之和 + 间距 + 边距（供外层容器如 ScrollBox 计算）
    float preferred_height() const override;

    void set_theme(const Theme* theme) { theme_ = theme; }
    void on_draw(Renderer& renderer) override;

private:
    float spacing_ = 8.f;
    float padding_ = 12.f;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
