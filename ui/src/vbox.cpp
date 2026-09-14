#include "ui/ui/vbox.h"

#include "ui/gl/renderer.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/theme.h"

namespace ui {

// 返回控件上第一个 layui-bg-* 类名（如 "layui-bg-gray"），无则空串（镜像 button.cpp 同逻辑）
static std::string bg_class_name(const Widget& w) {
    for (const std::string& c : w.classes()) {
        if (c.rfind("layui-bg-", 0) == 0) return c;
    }
    return std::string();
}

VBox::VBox(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void VBox::on_layout() {
    float cursor_y = bounds_.y + padding_;
    const float content_w = bounds_.w - 2.f * padding_;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float h = child->preferred_height();
        child->set_bounds(bounds_.x + padding_, cursor_y, content_w, h);
        cursor_y += h + spacing_;
    }
}

float VBox::preferred_height() const {
    float h = 2.f * padding_;
    for (const Widget* child : children_) {
        if (!child->visible) continue;
        h += child->preferred_height() + spacing_;
    }
    if (!children_.empty()) h -= spacing_;
    return h;
}

void VBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    if (has_class("layui-menu")) {
        // layui 菜单面板：白底 + #eee 边框 + 圆角 2 + 阴影
        renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1, 1, 1, 1});
        renderer.draw_rect_outline(bounds_, 1.f, t->border);
        return;
    }
    // layui-bg-* 工具色卡片（UI-203）：气泡/色块底（如 add_class("layui-bg-blue")
    // → 蓝底 #1e9fff；layui-bg-gray → 灰卡片底），颜色取自 layui::bg_fill 表。
    const std::string bgc = bg_class_name(*this);
    if (!bgc.empty()) {
        renderer.draw_rounded_rect(bounds_, t->radius, layui::bg_fill(bgc.c_str()));
        return;
    }
    renderer.draw_rounded_rect(bounds_, t->radius, t->button_normal);
}

}  // namespace ui
