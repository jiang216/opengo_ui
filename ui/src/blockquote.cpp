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
    mark_dirty();
        label_->set_font(font);
}

void BlockQuote::set_theme(const Theme* theme) {
    mark_dirty();
        theme_ = theme;
    label_->set_theme(theme);
    // UI-203：quote 底 #fafafa 为恒浅色（on_draw 内硬编码 layui.css .layui-elem-quote），
    // 文字须恒深色可读（layui 规范 #555）——不能用 theme->text_secondary：暗色主题下
    // text_secondary 为近白 → 白字白底（UI-201 同类缺陷）。灰卡片/蓝气泡底上均成立。
    label_->set_color(Vec4{0.333f, 0.333f, 0.333f, 1.0f});
}

void BlockQuote::set_text(const char* text) {
    mark_dirty();
        label_->set_text(text ? text : "");
}

float BlockQuote::preferred_height() const {
    return label_->preferred_height() + 30.f;
}

void BlockQuote::on_layout() {
    // UI-169：内部 Label 高度取实际行高（而非 0）——为 0 时 Label::on_draw 产生退化
    // clip 矩形（w/h<=0），与 push/pop 配对损坏父级 scissor，导致内容穿透标题栏/Header。
    label_->set_bounds(bounds_.x + 15.f, bounds_.y + 15.f, bounds_.w - 30.f,
                       label_->preferred_height());
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
