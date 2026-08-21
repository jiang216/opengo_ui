#include "ui/ui/icon.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/theme.h"

namespace ui {

Icon::Icon(float x, float y, float w, float h, const char* icon_name)
    : Widget(x, y, w, h) {
    set_icon(icon_name);
}

void Icon::set_icon(const char* name) {
    icon_cp_ = name && *name ? layui_icon::from_class(name) : 0;
}

void Icon::on_draw(Renderer& renderer) {
    if (!font_ || icon_cp_ == 0 || !font_->icon_loaded()) return;
    const Vec4 color = has_color_ ? color_ : (theme_ ? theme_->text_secondary : Vec4{1, 1, 1, 1});
    // 图标以控件中心为基准绘制，字号取控件高度
    const float size = bounds_.h;
    const float cx = bounds_.x + bounds_.w / 2.f;
    const float cy = bounds_.y + bounds_.h / 2.f;
    font_->draw_icon(renderer, icon_cp_, cx, cy, size, color);
}

}  // namespace ui
