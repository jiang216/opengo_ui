#include "ui/ui/card.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

Card::Card(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-card");
}

void Card::set_header(const char* text) {
    mark_dirty();
        header_ = text ? text : "";
}

// 布局：header（约 40px）之下是 body，子控件纵向排布（padding 15，spacing 可调）
void Card::on_layout() {
    const float header_h = header_.empty() ? 0.f : 42.f;
    const float px = 15.f;
    const float py = header_h + 10.f;
    float y = py;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float h = child->preferred_height() > 0 ? child->preferred_height() : child->bounds().h;
        child->set_bounds(bounds_.x + px, bounds_.y + y, bounds_.w - px * 2, h);
        y += h + spacing_;
    }
}

float Card::preferred_height() const {
    const float header_h = header_.empty() ? 0.f : 42.f;
    float y = header_h + 10.f;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float h = child->preferred_height() > 0 ? child->preferred_height() : child->bounds().h;
        y += h + spacing_;
    }
    return y - spacing_ + 10.f;
}

void Card::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    // 阴影近似（box-shadow: 0 1px 2px 0 rgba(0,0,0,.05)）：两层衰减模拟模糊
    const Rect shadow1 = {bounds_.x, bounds_.y + 1, bounds_.w, bounds_.h};
    renderer.draw_rounded_rect(shadow1, t->radius, Vec4{0, 0, 0, 0.05f});
    const Rect shadow2 = {bounds_.x + 1, bounds_.y + 2, bounds_.w - 2, bounds_.h - 2};
    renderer.draw_rounded_rect(shadow2, t->radius, Vec4{0, 0, 0, 0.02f});

    // 卡片白底
    renderer.draw_rounded_rect(bounds_, t->radius, t->panel);

    // 可选描边（set_outline；a<=0 关闭）
    if (outline_.a > 0.f)
        renderer.draw_rounded_rect_outline(bounds_, t->radius, outline_thickness_, outline_);

    // header
    if (!header_.empty()) {
        const Rect hrect = {bounds_.x, bounds_.y, bounds_.w, 42.f};
        renderer.draw_rounded_rect(hrect, t->radius, t->panel);
        // 下边框 #f8f8f8
        const Rect sep = {bounds_.x + 1, bounds_.y + 41.f, bounds_.w - 2, 1.f};
        renderer.draw_quad(sep, Vec4{0.97f, 0.97f, 0.97f, 1.0f});
        if (font_) {
            const float hw = font_->measure_text(header_.c_str());
            font_->draw_text(renderer, header_.c_str(), bounds_.x + 15.f,
                             bounds_.y + (42.f - font_->line_height()) * 0.5f,
                             Vec4{0.2f, 0.2f, 0.2f, 1.0f});
        }
    }
}

}  // namespace ui
