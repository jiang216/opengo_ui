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

void CountdownText::on_advance(float) {
    // 帧级门控宿主（is_dirty() 决定跳帧）下，on_draw 不被调则状态/文本停在旧值：
    // 倒计时需每帧推进，故在 on_advance（不受门控，advance 每帧执行）据当前剩余秒判定
    // 显示（%.1f → 0.1s 桶）与 done_ 是否有变化，变化才 mark_dirty() 触发重绘；
    // 结束后桶不再变化即停止标脏、恢复空闲停帧。（见 Carousel::on_advance 的同类先例）
    if (!font_) return;
    const float elapsed =
        std::chrono::duration<float>(std::chrono::steady_clock::now() - start_).count();
    const float remain = total_ - elapsed;
    const bool ndone = remain <= 0.f;
    const int ntick = (int)((remain < 0.f ? 0.f : remain) * 10.f);
    if (ndone != done_ || ntick != tick_) {
        done_ = ndone;
        tick_ = ntick;
        mark_dirty();
    }
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
