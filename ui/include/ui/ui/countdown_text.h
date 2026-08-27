#pragma once

#include "ui/ui/widget.h"

#include <chrono>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 实时倒计时文本（layui.util.countDown 克隆）：on_draw 每帧按 steady_clock 计算剩余秒数，
// 无需 advance(dt) 驱动即实时刷新。由 UI-039 从 layui_test 的 util_page.cpp 下沉为 ui 库组件。
class CountdownText : public Widget {
public:
    CountdownText(float x, float y, float w, float h);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    void set_total(float seconds);

    void restart();

    float preferred_height() const override;
    void on_draw(Renderer& renderer) override;

private:
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    float total_ = 60.f;
    std::chrono::steady_clock::time_point start_{};
    bool done_ = false;
};

}  // namespace ui
