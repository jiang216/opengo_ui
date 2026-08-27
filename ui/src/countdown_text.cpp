#include "ui/ui/countdown_text.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <cstdio>

namespace ui {

CountdownText::CountdownText(float x, float y, float w, float h)
    : Widget(x, y, w, h) {
    restart();
}

void CountdownText::set_total(float seconds) {
    mark_dirty();
        total_ = seconds;
    restart();
}

void CountdownText::restart() {
    mark_dirty();
        start_ = std::chrono::steady_clock::now();
    done_ = false;
}

float CountdownText::preferred_height() const {
    return font_ ? font_->line_height() : bounds_.h;
}

void CountdownText::on_draw(Renderer& renderer) {
    if (!font_) return;
    const float elapsed =
        std::chrono::duration<float>(std::chrono::steady_clock::now() - start_).count();
    const float remain = total_ - elapsed;
    if (remain <= 0.f) done_ = true;

    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    char buf[80];
    if (done_) {
        std::snprintf(buf, sizeof(buf), "时间到！点击按钮重新开始");
    } else {
        std::snprintf(buf, sizeof(buf), "剩余 %.1f 秒（实时刷新，%.0fs 后结束）", remain, total_);
    }
    font_->draw_text(renderer, buf, bounds_.x, bounds_.y, t->success);
}

}  // namespace ui
