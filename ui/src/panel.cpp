#include "ui/ui/panel.h"

#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

Panel::Panel(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void Panel::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const Vec4 color = (fill_.a < 0.f) ? t->button_normal : fill_;
    renderer.draw_rounded_rect(bounds_, t->radius, color);
}

}  // namespace ui
