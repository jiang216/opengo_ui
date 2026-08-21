#include "ui/ui/flowbox.h"

#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

FlowBox::FlowBox(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void FlowBox::on_layout() {
    const float left = bounds_.x + padding_;
    const float avail_w = bounds_.w - 2.f * padding_;
    float cursor_x = left;
    float cursor_y = bounds_.y + padding_;
    float line_h = 0.f;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float w = child->bounds().w;
        const float h = child->preferred_height();
        // 放不下且本行已有内容 → 换行（首项超宽不换行，与 Label::recompute_lines 同规则）
        if (cursor_x + w > left + avail_w && cursor_x > left) {
            cursor_x = left;
            cursor_y += line_h + spacing_;
            line_h = 0.f;
        }
        child->set_bounds(cursor_x, cursor_y, w, h);
        cursor_x += w + spacing_;
        line_h = std::max(line_h, h);
    }
}

float FlowBox::preferred_height() const {
    float avail_w = bounds_.w - 2.f * padding_;
    if (avail_w <= 0.f) avail_w = 1.f;
    float x = 0.f;
    float line_h = 0.f;
    float total = 0.f;
    for (const Widget* child : children_) {
        if (!child->visible) continue;
        const float w = child->bounds().w;
        const float h = child->preferred_height();
        if (x + w > avail_w && x > 0.f) {
            total += line_h + spacing_;
            x = 0.f;
            line_h = 0.f;
        }
        x += w + spacing_;
        line_h = std::max(line_h, h);
    }
    return 2.f * padding_ + total + line_h;
}

void FlowBox::on_draw(Renderer& renderer) {
    (void)renderer;  // layui-btn-container 无背景，透明
}

}  // namespace ui
