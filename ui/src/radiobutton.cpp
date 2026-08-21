#include "ui/ui/radiobutton.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
constexpr float kRadius = 9.f;  // 圆环半径（像素）
}  // namespace

RadioButton::RadioButton(float x, float y, float w, float h, const char* text)
    : Widget(x, y, w, h), text_(text ? text : "") {
    focusable = true;
}

void RadioButton::set_checked(bool checked) {
    checked_ = checked;
}

bool RadioButton::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    if (on_activate_) on_activate_();
    return true;
}

bool RadioButton::on_key_down(int key, int mods) {
    (void)mods;
    if (key == KeyEnter || key == KeySpace) {
        if (on_activate_) on_activate_();
        return true;
    }
    return false;
}

void RadioButton::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const float cx = bounds_.x + kRadius;
    const float cy = bounds_.y + bounds_.h * 0.5f;
    const Rect ring{cx - kRadius, cy - kRadius, kRadius * 2.f, kRadius * 2.f};
    const float radius = kRadius;  // 半径=宽半 -> 圆

    const Vec4 accent = enabled ? t->accent : t->text_muted;
    const Vec4 text_color = enabled ? t->text : t->text_muted;

    // hover：外环加粗感（颜色加深）；pressed：内底压暗
    Vec4 ring_color = accent;
    Vec4 bg = t->button_normal;
    if (enabled && hovered_ && !checked_) {
        ring_color = Vec4{accent.r * 0.85f, accent.g * 0.85f, accent.b * 0.85f, 1.f};
    }
    if (enabled && pressed_) {
        bg = Vec4{0.93f, 0.93f, 0.93f, 1.f};
    }

    // 内底填充 + 外环描边（accent 2px）
    renderer.draw_rounded_rect(ring, radius, bg);
    renderer.draw_rounded_rect_outline(ring, radius, 2.f, ring_color);

    // 选中圆点
    if (checked_) {
        const float d = kRadius * 2.f - 8.f;
        const Rect dot{cx - d * 0.5f, cy - d * 0.5f, d, d};
        renderer.draw_rounded_rect(dot, d * 0.5f, accent);
    }

    if (font_ && !text_.empty()) {
        const float tx = bounds_.x + kRadius * 2.f + 8.f;
        const float ty = cy - font_->line_height() * 0.5f;
        font_->draw_text(renderer, text_.c_str(), tx, ty, text_color);
    }
}

}  // namespace ui
