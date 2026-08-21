#include "ui/ui/circle.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"

#include <algorithm>

namespace ui {

Circle::Circle(float size, const char* text)
    : Widget(0, 0, size, size), text_(text ? text : "") {}

void Circle::on_draw(Renderer& renderer) {
    const float r = std::min(bounds_.w, bounds_.h) * 0.5f;
    renderer.draw_rounded_rect(bounds_, r, color_);
    if (font_ && !text_.empty()) {
        const float tw = font_->measure_text(text_.c_str());
        const float lh = font_->line_height();
        font_->draw_text(renderer, text_.c_str(),
                         bounds_.x + (bounds_.w - tw) * 0.5f,
                         bounds_.y + (bounds_.h - lh) * 0.5f, Vec4{1, 1, 1, 1});
    }
}

}  // namespace ui
