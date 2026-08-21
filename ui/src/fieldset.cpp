#include "ui/ui/fieldset.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

FieldsetTitle::FieldsetTitle(float w, float h, const char* text)
    : Widget(0, 0, w, h), text_(text ? text : "") {}

void FieldsetTitle::on_draw(Renderer& renderer) {
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    const float cy = bounds_.y + bounds_.h * 0.5f;
    renderer.draw_quad({bounds_.x, cy, bounds_.w, 1.f}, t->border);
    if (font_ && !text_.empty()) {
        const float tw = font_->measure_text(text_.c_str());
        const float lh = font_->line_height();
        // 文字底下垫白色块，压住横线（layui legend 效果）
        renderer.draw_quad({bounds_.x, bounds_.y, tw + 16.f, bounds_.h}, t->panel);
        font_->draw_text(renderer, text_.c_str(), bounds_.x, cy - lh * 0.5f, t->text_secondary);
    }
}

Fieldset::Fieldset(float w, float h, const char* legend)
    : Widget(0, 0, w, h), legend_(legend ? legend : "") {}

float Fieldset::preferred_height() const {
    float h = 34.f;  // legend 行
    for (const Widget* c : children()) h += c->preferred_height() + 6.f;
    return h;
}

void Fieldset::on_layout() {
    const float pad = 12.f;
    float y = bounds_.y + 34.f;
    for (Widget* c : children()) {
        const float ch = c->preferred_height();
        c->set_bounds(bounds_.x + pad, y, bounds_.w - 2.f * pad, ch);
        y += ch + 6.f;
    }
}

void Fieldset::on_draw(Renderer& renderer) {
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    renderer.draw_quad({bounds_.x, bounds_.y + 22.f, bounds_.w, 1.f}, t->border);
    renderer.draw_quad({bounds_.x, bounds_.y + bounds_.h - 1.f, bounds_.w, 1.f}, t->border);
    if (font_ && !legend_.empty()) {
        const float lh = font_->line_height();
        const float tw = font_->measure_text(legend_.c_str());
        const float ly = bounds_.y + 22.f - lh * 0.5f;
        renderer.draw_quad({bounds_.x + 16.f, ly, tw + 16.f, lh}, t->panel);
        font_->draw_text(renderer, legend_.c_str(), bounds_.x + 24.f, ly, t->text_secondary);
    }
}

}  // namespace ui
