#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Renderer;
struct Theme;

// 流式换行容器（layui-btn-container 语义）：子控件从左到右排布，超出可用宽度自动换行；
// 行内高度取该行子控件 preferred_height 的最大值，子项间距 spacing_（layui 容器按钮
// 右/下 10px margin）。无背景，透明绘制。
class FlowBox : public Widget {
public:
    FlowBox(float x, float y, float w, float h);

    void set_spacing(float spacing) {  spacing_ = spacing; mark_dirty(); }
    void set_padding(float padding) {  padding_ = padding; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }

    void on_layout() override;
    float preferred_height() const override;
    void on_draw(Renderer& renderer) override;

private:
    float spacing_ = 10.f;
    float padding_ = 0.f;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
