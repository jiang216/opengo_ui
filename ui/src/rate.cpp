#include "ui/ui/rate.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace ui {
namespace {

// 生成五角星顶点（内接外径 outer、内径 inner，角度偏移 start 弧度），供 draw_polygon 三角扇
int star_points(float cx, float cy, float outer, float inner, float start, Vec2* out) {
    constexpr int kPoints = 5;
    for (int i = 0; i < kPoints; ++i) {
        const float a_o = start + i * (2.f * 3.14159265f / kPoints);
        const float a_i = start + (i + 0.5f) * (2.f * 3.14159265f / kPoints);
        out[i * 2] = {cx + std::cos(a_o) * outer, cy + std::sin(a_o) * outer};
        out[i * 2 + 1] = {cx + std::cos(a_i) * inner, cy + std::sin(a_i) * inner};
    }
    return kPoints * 2;
}

// 格式化数值文本：整数去小数，半星保留 x.5
std::string format_value(float v) {
    const float r = std::round(v * 2.f) / 2.f;
    const int whole = (int)r;
    if (r == (float)whole) return std::to_string(whole);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.1f", r);
    return buf;
}

}  // namespace

Rate::Rate(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-rate");
}

void Rate::set_value(float v) {
    mark_dirty();
        const float maxv = (float)length_;
    v = v < 0.f ? 0.f : (v > maxv ? maxv : v);
    if (half_) {
        v = std::round(v * 2.f) / 2.f;  // 吸附 0.5 步
    }
    value_ = v;
}

float Rate::star_x(int i) const {
    const float star_size = 20.f;
    const float step = star_size + 5.f;
    return bounds_.x + i * step + star_size * 0.5f;
}

int Rate::star_at(float x) const {
    for (int i = 0; i < length_; ++i) {
        if (x >= star_x(i) - 12.f && x <= star_x(i) + 12.f) return i;
    }
    return -1;
}

bool Rate::star_half_at(float x, int idx) const {
    // 落在该星左半（星心左侧）
    return half_ && x < star_x(idx);
}

bool Rate::on_mouse_move(float x, float y) {
    (void)y;
    if (!readonly_) hover_ = star_at(x);
    return hover_ >= 0;
}

bool Rate::on_mouse_down(float x, float y, int button) {
    (void)y;
    if (readonly_ || button != 0) return false;
    const int idx = star_at(x);
    if (idx >= 0) {
        pressed_click_ = true;
        return true;
    }
    return false;
}

bool Rate::on_mouse_up(float x, float y, int button) {
    (void)y;
    if (readonly_ || button != 0 || !pressed_click_) return false;
    pressed_click_ = false;
    const int idx = star_at(x);
    if (idx >= 0) {
        const bool half_val = star_half_at(x, idx);
        value_ = half_val ? (float)idx + 0.5f : (float)(idx + 1);
        if (on_change_) on_change_((int)value_);
        return true;
    }
    return false;
}

void Rate::on_draw(Renderer& renderer) {
    if (bounds_.w <= 0.f || bounds_.h <= 0.f) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const Vec4 star_col = has_theme_color_ ? theme_color_ : t->warning;

    constexpr float kOuter = 10.f;   // 外径（20px 星）
    constexpr float kInner = 4.2f;   // 内径
    constexpr float kHollow = 0.58f; // 空星内芯缩放（模拟空心轮廓，layui 空星为描边星）
    const float cy = bounds_.y + bounds_.h * 0.5f;

    Vec2 pts[10];
    for (int i = 0; i < length_; ++i) {
        const float cx = star_x(i);
        const float filled = i + 1 <= value_ ? 1.f : 0.f;
        const float frac = value_ - (float)i;  // 半星：0.5
        const bool full = filled >= 1.f;
        const bool half = half_ && !full && frac >= 0.5f && frac < 1.f;
        // hover 星放大 scale(1.12)；hover 时整星亮起
        const float scale = (i == hover_) ? 1.12f : 1.0f;
        const float hover_full = (!readonly_ && hover_ >= i);

        if (full || (hover_full && !readonly_)) {
            // 实心星
            star_points(cx, cy, kOuter * scale, kInner * scale, -3.14159265f / 2.f, pts);
            renderer.draw_polygon(pts, 10, star_col);
        } else if (half) {
            // 半星：先画空心星，再裁剪左半画实心
            star_points(cx, cy, kOuter * scale, kInner * scale, -3.14159265f / 2.f, pts);
            renderer.draw_polygon(pts, 10, star_col);
            star_points(cx, cy, kOuter * scale * kHollow, kInner * scale * kHollow,
                        -3.14159265f / 2.f, pts);
            renderer.draw_polygon(pts, 10, t->panel);
            // 左半实心（scissor 裁剪）
            const Rect sc = {cx - kOuter * scale - 1.f, cy - kOuter * scale - 1.f,
                             kOuter * scale + 1.f, kOuter * scale * 2.f + 2.f};
            renderer.push_scissor(sc);
            star_points(cx, cy, kOuter * scale, kInner * scale, -3.14159265f / 2.f, pts);
            renderer.draw_polygon(pts, 10, star_col);
            renderer.pop_scissor();
        } else {
            // 空星：warning 外轮廓 + panel 内芯挖空
            star_points(cx, cy, kOuter * scale, kInner * scale, -3.14159265f / 2.f, pts);
            renderer.draw_polygon(pts, 10, star_col);
            star_points(cx, cy, kOuter * scale * kHollow, kInner * scale * kHollow,
                        -3.14159265f / 2.f, pts);
            renderer.draw_polygon(pts, 10, t->panel);
        }
    }

    // 文本（layui text）：星后显示数值或自定义回调文本
    if (text_ && font_) {
        const float tw = star_x(length_ - 1) + 10.f - bounds_.x + 4.f;
        std::string txt = text_cb_ ? text_cb_(value_) : format_value(value_);
        const float tx = bounds_.x + tw;
        const float ty = cy - 7.f;
        font_->draw_text(renderer, txt.c_str(), tx, ty, star_col);
    }
}

}  // namespace ui
