#include "ui/ui/breadcrumb.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

Breadcrumb::Breadcrumb(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-breadcrumb");
}

void Breadcrumb::add_item(const char* text) {
    items_.push_back({text ? text : "", false});
}

void Breadcrumb::add_current(const char* text) {
    items_.push_back({text ? text : "", true});
}

float Breadcrumb::preferred_height() const {
    return font_ ? font_->line_height() : bounds_.h;
}

void Breadcrumb::on_draw(Renderer& renderer) {
    if (!font_) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const float line_h = font_->line_height();
    const float ty = bounds_.y + (bounds_.h - line_h) * 0.5f;
    float tx = bounds_.x;
    const Vec4 link = hovered() ? t->success : t->text_muted;  // #999 -> hover #16b777
    const Vec4 cur = t->text_secondary;                        // #5f5f5f

    for (size_t i = 0; i < items_.size(); ++i) {
        if (i > 0) {
            const float sw = font_->measure_text(separator_.c_str());
            font_->draw_text(renderer, separator_.c_str(), tx, ty, t->text_muted);
            tx += sw;
        }
        const Item& it = items_[i];
        const Vec4& color = it.current ? cur : link;
        const float tw = font_->measure_text(it.text.c_str());
        font_->draw_text(renderer, it.text.c_str(), tx, ty, color);
        tx += tw;
    }
}

}  // namespace ui
