#include "ui/ui/hbox.h"

#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

HBox::HBox(float x, float y, float w, float h) : Widget(x, y, w, h) {}

float HBox::preferred_height() const {
    if (bounds_.h > 0.f) return bounds_.h;  // 显式高度优先
    float max_h = 0.f;
    for (const Widget* child : children_) {
        if (!child->visible) continue;
        max_h = std::max(max_h, child->preferred_height());
    }
    return 2.f * padding_ + max_h;
}

void HBox::on_layout() {
    const float content_h = bounds_.h - 2.f * padding_;
    const float y_base = bounds_.y + padding_;
    float cursor_x = bounds_.x + padding_;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float w = child->bounds().w;
        if (valign_ == Valign::Center) {
            const float ch = child->preferred_height();
            child->set_bounds(cursor_x, y_base + (content_h - ch) * 0.5f, w, ch);
        } else {
            child->set_bounds(cursor_x, y_base, w, content_h);
        }
        cursor_x += w + spacing_;
    }
}

void HBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const Vec4 color = (fill_.a < 0.f) ? t->button_normal : fill_;
    renderer.draw_rounded_rect(bounds_, t->radius, color);
}

}  // namespace ui
