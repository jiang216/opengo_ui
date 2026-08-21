#include "ui/ui/switch.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <cstring>

namespace ui {

namespace {
constexpr float kRadius = 20.f;  // 容器大圆角
constexpr float kKnobSize = 16.f;
constexpr float kKnobInset = 4.f;  // 滑块距容器边缘（容器 padding 0 5px + 上 3px）
}  // namespace

Switch::Switch(float x, float y, float w, float h) : Widget(x, y, w, h) {
    focusable = true;
    add_class("layui-form-switch");
}

Switch::~Switch() {
    if (custom_) {
        if (custom_->parent()) custom_->parent()->remove_child(custom_);
        delete custom_;
    }
}

void Switch::set_text(const char* text) {
    text_ = text ? text : "";
    // 用 "|" 分隔两态文本：开态=左段，关态=右段（layui title="ON|OFF" 语义）
    const char* sep = std::strchr(text_.c_str(), '|');
    if (sep) {
        on_text_ = text_.substr(0, static_cast<size_t>(sep - text_.c_str()));
        off_text_ = sep + 1;
    } else {
        on_text_ = text_;
        off_text_.clear();
    }
}

void Switch::set_custom_content(Widget* content) {
    if (custom_ == content) return;
    if (custom_) {
        if (custom_->parent()) custom_->parent()->remove_child(custom_);
        delete custom_;
    }
    custom_ = content;
}

void Switch::on_layout() {
    if (custom_) {
        // lay-checkbox 自定义标题：铺在滑块右侧区域
        custom_->set_bounds(bounds_.x + kKnobInset + kKnobSize + 6.f, bounds_.y,
                            bounds_.w - kKnobInset - kKnobSize - 6.f, bounds_.h);
        custom_->layout();
    }
}

bool Switch::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    checked_ = !checked_;
    if (on_change_) on_change_(checked_);
    return true;
}

bool Switch::on_key_down(int key, int mods) {
    (void)mods;
    if (key == KeyEnter || key == KeySpace) {
        checked_ = !checked_;
        if (on_change_) on_change_(checked_);
        return true;
    }
    return false;
}

void Switch::on_draw(Renderer& renderer) {
    if (bounds_.w <= 0.f || bounds_.h <= 0.f) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const float w = bounds_.w;
    const float h = bounds_.h;
    const float cy = bounds_.y + h * 0.5f;

    if (!enabled) {
        // 禁用：灰底灰边 + 灰滑块靠左
        renderer.draw_rounded_rect(bounds_, kRadius, Vec4{0.88f, 0.88f, 0.88f, 1.0f});
        renderer.draw_rounded_rect_outline(bounds_, kRadius, 1.f, t->text_muted);
        const float kx = bounds_.x + kKnobInset;
        renderer.draw_rounded_rect(Rect{kx, cy - kKnobSize * 0.5f, kKnobSize, kKnobSize},
                                   kRadius, t->text_muted);
    } else if (checked_) {
        // 开：绿底 #16b777 + 白滑块靠右
        Vec4 track = t->success;
        if (hovered_) track = Vec4{track.r * 0.92f, track.g * 0.92f, track.b * 0.92f, 1.0f};
        if (pressed_) track = Vec4{track.r * 0.85f, track.g * 0.85f, track.b * 0.85f, 1.0f};
        renderer.draw_rounded_rect(bounds_, kRadius, track);
        const float kx = bounds_.x + w - kKnobSize - kKnobInset;
        renderer.draw_rounded_rect(Rect{kx, cy - kKnobSize * 0.5f, kKnobSize, kKnobSize},
                                   kRadius, Vec4{1, 1, 1, 1});
    } else {
        // 关：白底 + #d2d2d2 边框 + 灰滑块靠左
        Vec4 track = Vec4{1, 1, 1, 1};
        if (hovered_) track = Vec4{0.97f, 0.97f, 0.97f, 1.0f};
        renderer.draw_rounded_rect(bounds_, kRadius, track);
        Vec4 border_color = Vec4{0.82f, 0.82f, 0.82f, 1.0f};
        if (hovered_) border_color = Vec4{0.70f, 0.70f, 0.70f, 1.0f};
        renderer.draw_rounded_rect_outline(bounds_, kRadius, 1.f, border_color);
        Vec4 knob = Vec4{0.82f, 0.82f, 0.82f, 1.0f};
        if (hovered_) knob = Vec4{0.70f, 0.70f, 0.70f, 1.0f};
        const float kx = bounds_.x + kKnobInset;
        renderer.draw_rounded_rect(Rect{kx, cy - kKnobSize * 0.5f, kKnobSize, kKnobSize},
                                   kRadius, knob);
    }

    // 两态文本 / lay-checkbox 自定义标题：开时显示 on（左段），关时显示 off（右段）
    if (custom_) {
        custom_->draw(renderer);
    } else if (font_) {
        const char* cur = checked_ ? on_text_.c_str() : off_text_.c_str();
        if (cur && *cur) {
            const float text_w = font_->measure_text(cur);
            const float ty = cy - font_->line_height() * 0.5f;
            const Vec4 color = (!enabled || !checked_) ? t->text_muted : Vec4{1, 1, 1, 1};
            if (enabled && checked_) {
                font_->draw_text(renderer, cur, bounds_.x + kKnobInset + 2.f, ty, color);
            } else {
                font_->draw_text(renderer, cur, bounds_.x + w - text_w - kKnobInset - 2.f,
                                 ty, color);
            }
        }
    }

    // 聚焦态：外圈 2px accent 描边
    if (focused()) renderer.draw_rounded_rect_outline(bounds_, kRadius, 2.f, t->accent);
}

}  // namespace ui