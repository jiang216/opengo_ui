#include "ui/ui/badge.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/theme.h"

namespace ui {
namespace {

// 找到控件上的 layui-bg-* 类名（无则返回空串）
std::string bg_class(const Widget* w) {
    for (const std::string& c : w->classes()) {
        if (c.rfind("layui-bg-", 0) == 0) return c;
    }
    return std::string();
}

}  // namespace

Badge::Badge(float x, float y, const char* text)
    : Widget(x, y, 0, 18), text_(text ? text : "") {
    add_class("layui-badge");
}

float Badge::preferred_width() const {
    if (has_class("layui-badge-dot")) return 8.f;
    float w = 12.f;  // padding 0 6px
    if (font_ && !text_.empty()) w += font_->measure_text(text_.c_str());
    return w;
}

void Badge::on_draw(Renderer& renderer) {
    if (bounds_.w <= 0.f || bounds_.h <= 0.f) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const std::string bg = bg_class(this);

    if (has_class("layui-badge-dot")) {
        // 8px Բ�㣺�̶�取边长 8 的正圆（layui-badge-dot 宽高 8、border-radius 50%）
        const float d = 8.f;
        const Vec4 fill = bg.empty() ? t->danger : layui::bg_fill(bg.c_str());
        renderer.draw_rounded_rect({bounds_.x, bounds_.y + (bounds_.h - d) * 0.5f, d, d}, d * 0.5f, fill);
        return;
    }

    if (has_class("layui-badge-rim")) {
        // 白底 + 1px #eee 描边（外圈画 #eee 再内缩 1px 画白底）+ 灰字
        renderer.draw_rounded_rect(bounds_, t->radius, t->border);
        const Rect inner = {bounds_.x + 1, bounds_.y + 1, bounds_.w - 2, bounds_.h - 2};
        renderer.draw_rounded_rect(inner, t->radius, t->panel);
        if (font_ && !text_.empty()) {
            const float tw = font_->measure_text(text_.c_str());
            const float tx = bounds_.x + (bounds_.w - tw) * 0.5f;
            const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
            font_->draw_text(renderer, text_.c_str(), tx, ty, t->text_secondary);
        }
        return;
    }

    // 默认：红底白字（可叠加 layui-bg-* 换底色）
    const Vec4 fill = bg.empty() ? t->danger : layui::bg_fill(bg.c_str());
    renderer.draw_rounded_rect(bounds_, t->radius, fill);
    if (font_ && !text_.empty()) {
        const float tw = font_->measure_text(text_.c_str());
        const float tx = bounds_.x + (bounds_.w - tw) * 0.5f;
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(renderer, text_.c_str(), tx, ty, Vec4{1, 1, 1, 1});
    }
}

}  // namespace ui
