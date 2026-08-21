#include "ui/ui/progress.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/theme.h"

#include <cstdio>

namespace ui {

Progress::Progress(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-progress");
}

void Progress::on_draw(Renderer& renderer) {
    if (bounds_.w <= 0.f || bounds_.h <= 0.f) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const bool big = has_class("layui-progress-big");
    const float radius = bounds_.h * 0.5f;  // 圆角 20px（big 时轨道圆角取半高）

    // 轨道
    renderer.draw_rounded_rect(bounds_, radius, t->border);

    // 填充色：layui-bg-* 色类覆盖；无则默认 #5fb878（layui-progress-bar 默认）
    Vec4 fill = layui::hex("#5fb878");
    for (const std::string& c : classes()) {
        if (c.rfind("layui-bg-", 0) == 0) {
            fill = layui::bg_fill(c.c_str());
            break;
        }
    }

    // 填充（宽度按 value）
    if (value_ > 0.f) {
        const Rect bar = {bounds_.x, bounds_.y, bounds_.w * value_, bounds_.h};
        renderer.draw_rounded_rect(bar, radius, fill);
    }

    // 文本：layui-progress-text 位于轨道右上（big 时内嵌白字）
    std::string shown = text_;
    if (show_text_ && font_) {
        if (shown.empty()) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%.0f%%", value_ * 100.f);
            shown = buf;
        }
        if (!big) {
            const float tw = font_->measure_text(shown.c_str());
            const float tx = bounds_.x + bounds_.w - tw;  // 轨道右缘
            const float ty = bounds_.y - 2.f - font_->line_height();
            font_->draw_text(renderer, shown.c_str(), tx, ty, t->text_secondary);
        } else {
            const float tw = font_->measure_text(shown.c_str());
            const float tx = bounds_.x + (bounds_.w - tw) * 0.5f;
            const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
            font_->draw_text(renderer, shown.c_str(), tx, ty, Vec4{1, 1, 1, 1});
        }
    }
}

}  // namespace ui
