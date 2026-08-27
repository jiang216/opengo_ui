#include "ui/ui/slider.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
constexpr float kHandleR = 6.f;      // 圆钮半径
constexpr float kTrackH = 4.f;       // 轨道厚
constexpr float kInputW = 70.f;      // 输入框宽
constexpr float kInputH = 24.f;      // 输入框高

inline float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
inline bool in_rect(float x, float y, const Rect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}
}  // namespace

Slider::Slider(float x, float y, float w, float h) : Widget(x, y, w, h) {
    focusable = true;
    add_class("layui-slider");
}

void Slider::set_range(float min_value, float max_value) {
    mark_dirty();
        min_ = min_value;
    max_ = max_value;
    value_ = std::min(std::max(value_, min_), max_);
    value2_ = std::min(std::max(value2_, min_), max_);
}

void Slider::set_value_silent(float v) {
    mark_dirty();
        value_ = std::min(std::max(v, min_), max_);
}

void Slider::set_value(float v) {
    mark_dirty();
        set_value_silent(v);
    emit();
}

void Slider::set_range_value(float start, float end) {
    mark_dirty();
        value_ = std::min(std::max(start, min_), max_);
    value2_ = std::min(std::max(end, min_), max_);
    if (value_ > value2_) std::swap(value_, value2_);
    emit();
}

float Slider::step_round(float v) const {
    float s = step_ > 0.f ? step_ : 1.f;
    const float snapped = min_ + std::round((v - min_) / s) * s;
    return std::min(std::max(snapped, min_), max_);
}

Rect Slider::track_rect() const {
    if (vertical_) {
        const float cx = bounds_.x + 4.f + (input_ ? 6.f : 0.f);
        return {cx, bounds_.y + kHandleR, 4.f, bounds_.h - 2 * kHandleR};
    }
    float w = bounds_.w;
    if (input_) w = std::max(40.f, bounds_.w - (kInputW + 14.f));
    const float cy = bounds_.y + bounds_.h * 0.5f;
    return {bounds_.x, cy - kTrackH * 0.5f, w, kTrackH};
}

Rect Slider::input_rect() const {
    if (!input_) return {};
    if (vertical_) return {bounds_.x + 4.f + 10.f, bounds_.y + (bounds_.h - kInputH) * 0.5f,
                           kInputW, kInputH};
    return {bounds_.x + bounds_.w - kInputW, bounds_.y + (bounds_.h - kInputH) * 0.5f,
            kInputW, kInputH};
}

Vec4 Slider::accent() const {
    if (has_theme_color_) return theme_color_;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    return t->accent;
}

std::string Slider::tips_text(float v) const {
    if (tips_fn_) return tips_fn_(v);
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%.0f", v);
    return buf;
}

void Slider::emit() {
    mark_dirty();  // 状态写自报：值变更即标脏，覆盖键盘改值等无按下态路径
    if (range_ && on_range_change_) {
        on_range_change_(value_, value2_);
    } else if (on_change_) {
        on_change_(value_);
    }
}

float Slider::value_from_coord(float c) const {
    const Rect tr = track_rect();
    float t;
    if (vertical_) t = 1.f - (c - tr.y) / tr.h;
    else t = (c - tr.x) / tr.w;
    return step_round(min_ + (max_ - min_) * clamp01(t));
}

float Slider::coord_for(float v) const {
    const Rect tr = track_rect();
    const float t = (max_ == min_) ? 0.f : (v - min_) / (max_ - min_);
    if (vertical_) return tr.y + (1.f - clamp01(t)) * tr.h;
    return tr.x + clamp01(t) * tr.w;
}

bool Slider::on_mouse_down(float x, float y, int button) {
    if (button != 0 || disabled_) return false;
    // 输入框：上/下箭头改值
    const Rect ib = input_rect();
    if (input_ && in_rect(x, y, ib)) {
        const float up_x = ib.x + ib.w - 24.f;
        if (x >= up_x && x <= ib.x + ib.w) {  // 右侧加减区
            const bool up = y < ib.y + ib.h * 0.5f;
            if (range_) {
                value2_ = step_round(value2_ + (up ? step_ : -step_));
                if (value2_ < value_) value2_ = value_;
            } else {
                value_ = step_round(value_ + (up ? step_ : -step_));
            }
            emit();
            return true;
        }
        return true;  // 点击输入框不落到轨道
    }
    const float coord = vertical_ ? y : x;
    if (range_) {
        const float d0 = std::fabs(coord_for(value_) - coord);
        const float d1 = std::fabs(coord_for(value2_) - coord);
        active_range_ = (d1 < d0) ? 1 : 0;
        if (active_range_ == 0) value_ = step_round(std::min(value_from_coord(coord), value2_));
        else value2_ = step_round(std::max(value_from_coord(coord), value_));
        drag_ = (active_range_ == 0) ? Drag::Start : Drag::End;
    } else {
        value_ = value_from_coord(coord);
        drag_ = Drag::Single;
    }
    dragging_ = true;
    emit();
    return true;
}

bool Slider::on_mouse_move(float x, float y) {
    if (!dragging_) return false;
    const float coord = vertical_ ? y : x;
    if (drag_ == Drag::Single) {
        value_ = value_from_coord(coord);
    } else if (drag_ == Drag::Start) {
        value_ = step_round(std::min(value_from_coord(coord), value2_));
    } else if (drag_ == Drag::End) {
        value2_ = step_round(std::max(value_from_coord(coord), value_));
    }
    emit();
    return true;
}

bool Slider::on_mouse_up(float x, float y, int button) {
    (void)x; (void)y;
    if (button != 0) return false;
    dragging_ = false;
    drag_ = Drag::None;
    return true;
}

bool Slider::on_key_down(int key, int mods) {
    (void)mods;
    if (disabled_) return false;
    if (key == KeyLeft || key == KeyDown) {
        if (range_ && active_range_ == 1) {
            value2_ = step_round(std::max(value2_ - step_, value_));
            if (value2_ == value_) { /* 不穿过 */ }
        } else {
            if (range_) {
                value_ = step_round(std::min(value_ - step_, value2_));
            } else {
                value_ = step_round(value_ - step_);
            }
        }
        emit();
        return true;
    }
    if (key == KeyRight || key == KeyUp) {
        if (range_ && active_range_ == 1) {
            value2_ = step_round(value2_ + step_);
        } else {
            if (range_) {
                value_ = step_round(std::min(value_ + step_, value2_));
            } else {
                value_ = step_round(value_ + step_);
            }
        }
        emit();
        return true;
    }
    if (key == KeyHome) {
        if (range_) { value_ = min_; value2_ = std::max(value_, value2_); }
        else value_ = min_;
        emit();
        return true;
    }
    if (key == KeyEnd) {
        if (range_) { value2_ = max_; value_ = std::min(value_, value2_); }
        else value_ = max_;
        emit();
        return true;
    }
    return false;
}

void Slider::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const Rect tr = track_rect();
    const bool grey = disabled_;
    const Vec4 track_col = grey ? Vec4{0.85f, 0.85f, 0.85f, 1.f} : Vec4{0.93f, 0.93f, 0.93f, 1.f};
    const Vec4 fill_col = grey ? Vec4{0.82f, 0.82f, 0.82f, 1.f} : accent();

    // 轨道底
    renderer.draw_rounded_rect(tr, kTrackH * 0.5f, track_col);

    // 间断点（showstep）
    if (showstep_ && step_ > 0.f && !grey) {
        const int ticks = (int)((max_ - min_) / step_) + 1;
        if (ticks <= 60) {
            for (int i = 0; i < ticks; ++i) {
                const float v = min_ + i * step_;
                const float c = coord_for(v);
                if (vertical_) renderer.draw_quad({tr.x + tr.w + 3.f, c - 1.f, 3.f, 2.f}, Vec4{0.8f, 0.8f, 0.8f, 1.f});
                else renderer.draw_quad({c - 1.f, tr.y - 5.f, 2.f, 3.f}, Vec4{0.8f, 0.8f, 0.8f, 1.f});
            }
        }
    }

    // 填充
    if (range_) {
        const float c0 = coord_for(value_);
        const float c1 = coord_for(value2_);
        if (vertical_) {
            // vertical：值越大 y 越小，c0(起始值在下方) > c1(结束值在上方)，高度取正
            renderer.draw_rounded_rect({tr.x, c1, tr.w, c0 - c1}, kTrackH * 0.5f, fill_col);
        } else {
            const float x0 = std::min(c0, c1);
            renderer.draw_rounded_rect({x0, tr.y, std::fabs(c1 - c0), tr.h}, kTrackH * 0.5f, fill_col);
        }
    } else {
        const float c = coord_for(value_);
        if (vertical_) {
            const float h = (tr.y + tr.h) - c;
            if (h > 0.f) renderer.draw_rounded_rect({tr.x, c, tr.w, h}, kTrackH * 0.5f, fill_col);
        } else {
            const float w = c - tr.x;
            if (w > 0.f) renderer.draw_rounded_rect({tr.x, tr.y, w, tr.h}, kTrackH * 0.5f, fill_col);
        }
    }

    // 圆钮：白色圆点 + 统一圆环轮廓（美观与 bruga 一致）
    const float r = grey ? kHandleR : (hovered_ || pressed_ ? kHandleR * 1.2f : kHandleR);
    const Vec4 ring_col = grey ? Vec4{0.8f, 0.8f, 0.8f, 1.f}
                               : (has_theme_color_ ? theme_color_ : Vec4{0.80f, 0.80f, 0.80f, 1.f});
    const auto draw_handle = [&](float cc) {
        const Vec2 c = vertical_ ? Vec2{tr.x + tr.w * 0.5f, cc} : Vec2{cc, tr.y + tr.h * 0.5f};
        renderer.draw_rounded_rect({c.x - r, c.y - r, r * 2.f, r * 2.f}, r,
                                   Vec4{1.f, 1.f, 1.f, 1.f});
        renderer.draw_rounded_rect_outline({c.x - r, c.y - r, r * 2.f, r * 2.f}, r, 1.5f, ring_col);
    };
    if (range_) {
        draw_handle(coord_for(value_));
        draw_handle(coord_for(value2_));
    } else {
        draw_handle(coord_for(value_));
    }
    if (focus_ring_ && focused() && !grey) renderer.draw_rect_outline(bounds_, 1.5f, fill_col);

    // 提示文本（tips）：hover / 拖拽时显示
    if (tips_ && font_ && !grey && (hovered_ || dragging_)) {
        const float v = range_ ? (drag_ == Drag::Start ? value_ : (drag_ == Drag::End ? value2_ : value_)) : value_;
        const float hc = range_ ? coord_for((drag_ == Drag::Start) ? value_ : value2_) : coord_for(value_);
        const std::string txt = tips_text(v);
        float tw = font_->measure_text(txt.c_str());
        const float bw = tw + 12.f, bh = 18.f;
        float bx, by;
        if (vertical_) {
            // 垂直：气泡在轨道右侧、y 随手柄
            bx = std::max(bounds_.x, tr.x + tr.w + 6.f);
            by = std::min(std::max(hc - bh * 0.5f, bounds_.y), bounds_.y + bounds_.h - bh);
        } else {
            bx = std::min(std::max(hc - bw * 0.5f, bounds_.x), bounds_.x + bounds_.w - bw);
            by = std::max(bounds_.y, tr.y - 24.f);
        }
        renderer.draw_rounded_rect({bx, by, bw, bh}, 3.f, Vec4{0.1f, 0.1f, 0.1f, 0.94f});
        font_->draw_text(renderer, txt.c_str(), bx + 6.f, by + 3.f, Vec4{1.f, 1.f, 1.f, 1.f});
    }

    // 输入框（input）：数值 + 右侧上下箭头
    if (input_) {
        const Rect ib = input_rect();
        const float v = range_ ? value2_ : value_;
        std::string txt = tips_text(v);
        renderer.draw_rounded_rect(ib, 3.f, Vec4{1.f, 1.f, 1.f, 1.f});
        renderer.draw_rect_outline(ib, 1.f, Vec4{0.82f, 0.82f, 0.82f, 1.f});
        if (font_) {
            float tw = font_->measure_text(txt.c_str());
            font_->draw_text(renderer, txt.c_str(), ib.x + 6.f, ib.y + 4.f, Vec4{0.25f, 0.25f, 0.25f, 1.f});
            // 分隔线 + 三角
            renderer.draw_quad({ib.x + ib.w - 26.f, ib.y + 2.f, 1.f, ib.h - 4.f}, Vec4{0.85f, 0.85f, 0.85f, 1.f});
            const float ax = ib.x + ib.w - 13.f;
            const Vec4 arrow_col = Vec4{0.4f, 0.4f, 0.4f, 1.f};
            // 上箭头
            const Vec2 up[3] = {{ax, ib.y + 7.f}, {ax - 4.f, ib.y + 12.f}, {ax + 4.f, ib.y + 12.f}};
            // 下箭头
            const Vec2 dn[3] = {{ax, ib.y + ib.h - 7.f}, {ax - 4.f, ib.y + ib.h - 12.f}, {ax + 4.f, ib.y + ib.h - 12.f}};
            renderer.draw_polygon(up, 3, arrow_col);
            renderer.draw_polygon(dn, 3, arrow_col);
        }
    }
}

}  // namespace ui