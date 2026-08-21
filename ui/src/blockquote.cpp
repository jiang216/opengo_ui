#include "ui/ui/blockquote.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/label.h"
#include "ui/ui/theme.h"

namespace ui {

BlockQuote::BlockQuote(float w, float h, const char* text, bool nm)
    : Widget(0, 0, w, h), nm_(nm) {
    label_ = new Label(0, 0, w - 30.f, 0, text ? text : "");
    label_->set_wrap(true);
    add_child(label_);
}

void BlockQuote::set_font(Font* font) {
    label_->set_font(font);
}

void BlockQuote::set_theme(const Theme* theme) {
    theme_ = theme;
    label_->set_theme(theme);
    label_->set_color(theme ? theme->text_secondary : Vec4{0.4f, 0.4f, 0.4f, 1.0f});
}

void BlockQuote::set_text(const char* text) {
    label_->set_text(text ? text : "");
}

float BlockQuote::preferred_height() const {
    return label_->preferred_height() + 30.f;
}

void BlockQuote::on_layout() {
    label_->set_bounds(bounds_.x + 15.f, bounds_.y + 15.f, bounds_.w - 30.f, 0.f);
}

void BlockQuote::on_draw(Renderer& renderer) {
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    // layui.css .layui-elem-quote：背景 #fafafa，左侧 5px 绿条 #5FB878（非 theme.success）
    if (!nm_) renderer.draw_quad(bounds_, Vec4{0.98f, 0.98f, 0.98f, 1.0f});
    renderer.draw_quad({bounds_.x, bounds_.y, 5.f, bounds_.h},
                       Vec4{0.373f, 0.722f, 0.471f, 1.0f});
}

}  // namespace ui
