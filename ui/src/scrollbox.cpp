#include "ui/ui/scrollbox.h"

#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

namespace {
constexpr float kScrollbarW = 6.f;
constexpr float kScrollbarMargin = 4.f;
}  // namespace

ScrollBox::ScrollBox(float x, float y, float w, float h) : Widget(x, y, w, h) {}

float ScrollBox::content_height() const {
    float h = 2.f * padding_;
    for (const Widget* child : children_) {
        if (!child->visible) continue;
        h += child->preferred_height() + spacing_;
    }
    if (!children_.empty()) h -= spacing_;  // 去掉末尾多算的间距
    return h;
}

float ScrollBox::scroll_max() const {
    return std::max(0.f, content_height() - bounds_.h);
}

void ScrollBox::clamp_scroll() {
    scroll_offset_ = std::min(std::max(scroll_offset_, 0.f), scroll_max());
}

void ScrollBox::set_scroll(float value) {
    const float old = scroll_offset_;
    scroll_offset_ = value;
    clamp_scroll();
    if (scroll_offset_ != old && on_scroll_) on_scroll_();
    check_bottom();
}

void ScrollBox::reset_scroll() {
    set_scroll(0.f);
}

void ScrollBox::check_bottom() {
    if (!on_bottom_) return;
    const float max_off = scroll_max();
    // 距底部不足 bottom_margin_ 即触发（镜像 layui flow mb 语义）；
    // bottom_margin_ 为 0 时退化为严格滚到底（UI-035 回归语义）。
    const float trigger = max_off - bottom_margin_;
    const bool at_bottom = scroll_offset_ >= trigger - 1.f;
    if (at_bottom && !bottom_fired_) {
        bottom_fired_ = true;
        on_bottom_();
    } else if (!at_bottom) {
        bottom_fired_ = false;
    }
}

void ScrollBox::on_layout() {
    needs_layout_ = false;
    clamp_scroll();
    // 内容宽度让出右侧滚动条列
    const float content_w = bounds_.w - 2.f * padding_ - kScrollbarW - kScrollbarMargin;
    float cursor_y = bounds_.y + padding_ - scroll_offset_;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float h = child->preferred_height();
        child->set_bounds(bounds_.x + padding_, cursor_y, content_w, h);
        cursor_y += h + spacing_;
    }
}

Rect ScrollBox::track_rect() const {
    const float x = bounds_.x + bounds_.w - kScrollbarMargin - kScrollbarW;
    return {x, bounds_.y + 4.f, kScrollbarW, std::max(1.f, bounds_.h - 8.f)};
}

Rect ScrollBox::thumb_rect() const {
    const Rect track = track_rect();
    const float content = content_height();
    if (content <= 0.f) return {track.x, track.y, track.w, track.h};
    const float frac = bounds_.h / content;
    const float thumb_h = std::max(20.f, track.h * frac);
    const float max_off = scroll_max();
    const float t = max_off > 0.f ? scroll_offset_ / max_off : 0.f;
    const float thumb_y = track.y + (track.h - thumb_h) * t;
    return {track.x, thumb_y, track.w, thumb_h};
}

bool ScrollBox::in_scrollbar(float x, float y) const {
    const Rect track = track_rect();
    return x >= track.x - 2.f && x <= track.x + track.w + 2.f && y >= track.y && y <= track.y + track.h;
}

float ScrollBox::scroll_from_y(float y) const {
    const Rect track = track_rect();
    if (track.h <= 0.f) return 0.f;
    const float t = std::min(std::max((y - track.y) / track.h, 0.f), 1.f);
    return t * scroll_max();
}

bool ScrollBox::on_mouse_scroll(float x, float y, float dx, float dy) {
    (void)x;
    (void)y;
    (void)dx;
    if (scroll_max() <= 0.f) return false;
    set_scroll(scroll_offset_ - dy * scroll_step_);  // dy>0 向上滚
    return true;
}

bool ScrollBox::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    if (scroll_max() > 0.f && in_scrollbar(x, y)) {
        dragging_ = true;
        set_scroll(scroll_from_y(y));
        return true;  // 捕获鼠标，拖动滑块
    }
    // 下拉面板主体点击被消费，避免穿透到面板下方的其他控件（如重叠的 ComboBox）
    if (has_class("layui-form-select-panel")) return true;
    return false;
}

bool ScrollBox::on_mouse_move(float x, float y) {
    if (!dragging_) return false;
    set_scroll(scroll_from_y(y));
    return true;
}

bool ScrollBox::on_mouse_up(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    dragging_ = false;
    return true;
}

void ScrollBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    if (has_class("layui-form-select-panel")) {
        // layui 下拉面板：白底 + #eee 边框 + 圆角 2 + 阴影
        renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1, 1, 1, 1});
        renderer.draw_rect_outline(bounds_, 1.f, t->border);
        return;
    }
    if (has_class("layui-body")) {
        // layui 内容区（layui-body）：白底
        renderer.draw_quad(bounds_, Vec4{1, 1, 1, 1});
        return;
    }
    if (has_class("ws-menu")) {
        // 新镜像 docs/2 侧栏（ws-menu）：白底（.ws-menu{background-color:#fff}）
        renderer.draw_quad(bounds_, Vec4{1, 1, 1, 1});
        return;
    }
    if (has_class("layui-side-scroll")) {
        // layui 侧栏滚动容器（layui-side-scroll）：深灰底 #2f363c
        renderer.draw_quad(bounds_, Vec4{0.18f, 0.21f, 0.24f, 1.0f});
        return;
    }
    renderer.draw_rounded_rect(bounds_, t->radius, t->button_normal);
}

void ScrollBox::draw(Renderer& renderer) {
    if (!visible) return;
    on_draw(renderer);  // 背景
    renderer.push_scissor(bounds_);
    for (Widget* child : children_) {
        if (child->visible) child->draw(renderer);
    }
    // 叠加层：下拉面板等溢出内容最后绘制（置于所有兄弟节点之上）
    for (Widget* child : children_) {
        if (child->visible) child->draw_overlay(renderer);
    }
    renderer.pop_scissor();
    // 滚动条（仅溢出时显示；子树变化后抑制一帧防闪烁）
    if (!needs_layout_ && scroll_max() > 0.f) {
        static const Theme kDefaultTheme;
        const Theme* t = theme_ ? theme_ : &kDefaultTheme;
        renderer.draw_rounded_rect(track_rect(), kScrollbarW * 0.5f, t->button_pressed);
        renderer.draw_rounded_rect(thumb_rect(), kScrollbarW * 0.5f, t->text_dim);
    }
}

}  // namespace ui
