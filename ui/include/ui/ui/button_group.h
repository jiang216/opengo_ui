#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Button;
class Renderer;
struct Theme;

// 按钮组（layui-btn-group）：水平紧排一组按钮，无 padding/spacing。
// 相邻按钮间 1px 半透明白分隔线（primary 按钮除外，保留自身边框）；中间按钮零圆角、
// 首按钮左圆角、末按钮右圆角。组角色由本控件 add_button 时自动维护到 Button。
// 背景透明（不绘制底），子按钮高度按自身 preferred_height()（lg/sm/xs）垂直居中。
class ButtonGroup : public Widget {
public:
    ButtonGroup(float x, float y, float w, float h);

    void add_button(Button* b);
    void set_theme(const Theme* theme) { theme_ = theme; }

    void on_layout() override;

private:
    void update_roles();
    const Theme* theme_ = nullptr;
};

}  // namespace ui
