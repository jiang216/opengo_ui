#include "ui/ui/hline.h"

#include "ui/gl/renderer.h"

namespace ui {

HLine::HLine(float w, float h, const Vec4& color)
    : Widget(0, 0, w, h), color_(color) {}

void HLine::on_draw(Renderer& renderer) {
    renderer.draw_quad({bounds_.x, bounds_.y + bounds_.h * 0.5f, bounds_.w, 1.f}, color_);
}

}  // namespace ui
