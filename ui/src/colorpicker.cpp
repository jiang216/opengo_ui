#include "ui/ui/colorpicker.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
inline float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

// HSV -> RGBA（h/s/v 均 0..1）
Vec4 hsv_to_rgba(float h, float s, float v, float a) {
    const float hi = std::fmod(h * 6.f, 6.f);
    const int i = (int)hi;
    const float f = hi - (float)i;
    const float p = v * (1.f - s);
    const float q = v * (1.f - s * f);
    const float t = v * (1.f - s * (1.f - f));
    float r = v, g = v, b = v;
    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return {r, g, b, a};
}

// RGBA -> HSV（h 0..1, s 0..1, v 0..1）
void rgba_to_hsv(const Vec4& c, float& h, float& s, float& v) {
    const float r = clamp01(c.r), g = clamp01(c.g), b = clamp01(c.b);
    const float mx = std::max(r, std::max(g, b));
    const float mn = std::min(r, std::min(g, b));
    const float d = mx - mn;
    v = mx;
    s = mx > 0.f ? d / mx : 0.f;
    if (d == 0.f) {
        h = 0.f;
    } else {
        if (mx == r) h = std::fmod((g - b) / d, 6.f);
        else if (mx == g) h = (b - r) / d + 2.f;
        else h = (r - g) / d + 4.f;
        h /= 6.f;
        if (h < 0.f) h += 1.f;
    }
}

// 默认预定义色（layui colorpicker 经典色板）
const Vec4 kDefaultColors[] = {
    {1.00f, 0.27f, 0.00f, 1}, {1.00f, 0.55f, 0.00f, 1}, {1.00f, 0.84f, 0.00f, 1},
    {0.56f, 0.93f, 0.56f, 1}, {0.00f, 0.81f, 0.82f, 1}, {0.12f, 0.56f, 1.00f, 1},
    {0.78f, 0.08f, 0.52f, 1}, {1.00f, 0.41f, 0.71f, 1}, {0.42f, 0.35f, 0.80f, 1},
    {0.00f, 0.00f, 0.00f, 1},
};

constexpr float kPad = 8.f;
constexpr float kRangeH = 16.f;   // 滑块厚
constexpr float kBtnH = 28.f;     // 按钮高
constexpr float kBtnW = 52.f;     // 按钮宽
constexpr float kSwatchH = 22.f;  // 预定义色块高

Vec4 premult(const Vec4& c) { return {c.r * c.a, c.g * c.a, c.b * c.a, c.a}; }

bool in_rect(float x, float y, const Rect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}
}  // namespace

ColorPicker::ColorPicker(float x, float y, float w, float h) : Widget(x, y, w, h) {
    palette_.assign(kDefaultColors, kDefaultColors + 10);
}

void ColorPicker::set_color(const Vec4& c) {
    mark_dirty();
        color_ = c;
    rgba_to_hsv(c, hue_, sat_, val_);
}

void ColorPicker::set_colors(const std::vector<Vec4>& cols) {
    mark_dirty();
        if (cols.empty()) return;
    palette_ = cols;
}

float ColorPicker::sv_side() const {
    switch (size_) {
        case ColorPickerSize::Lg: return 110.f;
        case ColorPickerSize::Xs: return 82.f;
        default: return 96.f;
    }
}

float ColorPicker::range_thick() const { return kRangeH; }

Rect ColorPicker::sv_rect() const {
    const float s = sv_side();
    return {bounds_.x + kPad, bounds_.y + kPad, s, s};
}

Rect ColorPicker::hue_rect() const {
    const float s = sv_side();
    return {bounds_.x + kPad, bounds_.y + kPad + s + kPad, bounds_.w - 2 * kPad, kRangeH};
}

Rect ColorPicker::alpha_rect() const {
    const Rect hue = hue_rect();
    return {hue.x, hue.y + hue.h + kPad, hue.w, kRangeH};
}

Rect ColorPicker::predefine_rect() const {
    const Rect hue = hue_rect();
    float y = hue.y + hue.h + kPad;
    if (alpha_) {
        const Rect al = alpha_rect();
        y = al.y + al.h + kPad;
    }
    return {bounds_.x + kPad, y, bounds_.w - 2 * kPad, kSwatchH};
}

Rect ColorPicker::buttons_area() const {
    const float s = sv_side();
    const Rect hue = hue_rect();
    float y = hue.y + hue.h + kPad;
    if (alpha_) y = alpha_rect().y + alpha_rect().h + kPad;
    if (predefine_) y = predefine_rect().y + predefine_rect().h + kPad;
    return {bounds_.x + kPad, y, bounds_.w - 2 * kPad, kBtnH};
}

Rect ColorPicker::confirm_rect() const {
    const Rect area = buttons_area();
    return {area.x + area.w - kBtnW, area.y, kBtnW, kBtnH};
}

Rect ColorPicker::clear_rect() const {
    const Rect area = buttons_area();
    if (!clearable_) return {};
    return {area.x + area.w - 2 * kBtnW - 8.f, area.y, kBtnW, kBtnH};
}

float ColorPicker::preferred_height() const {
    float h = kPad + sv_side() + kPad;      // SV 区
    h += kRangeH + kPad;                    // 色相滑块
    if (alpha_) h += kRangeH + kPad;        // 透明度滑块
    if (predefine_) h += kSwatchH + kPad;   // 预定义色
    h += kBtnH + kPad;                      // 按钮行
    return h;
}

std::string ColorPicker::format_color() const {
    const auto byte = [](float f) { return (int)(clamp01(f) * 255.f + 0.5f); };
    char buf[48];
    const int r = byte(color_.r), g = byte(color_.g), b = byte(color_.b), a = byte(color_.a);
    if (format_ == ColorFormat::Hex) {
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
    } else if (alpha_) {
        std::snprintf(buf, sizeof(buf), "rgba(%d,%d,%d,%.2f)", r, g, b, color_.a);
    } else {
        std::snprintf(buf, sizeof(buf), "rgb(%d,%d,%d)", r, g, b);
    }
    return buf;
}

void ColorPicker::apply_sv(float x, float y) {
    const Rect sv = sv_rect();
    sat_ = clamp01((x - sv.x) / sv.w);
    val_ = clamp01(1.f - (y - sv.y) / sv.h);
    color_ = hsv_to_rgba(hue_, sat_, val_, color_.a);
    if (on_change_) on_change_(color_);
}

void ColorPicker::apply_hue(float x) {
    const Rect hue = hue_rect();
    hue_ = clamp01((x - hue.x) / hue.w);
    color_ = hsv_to_rgba(hue_, sat_, val_, color_.a);
    if (on_change_) on_change_(color_);
}

void ColorPicker::apply_alpha(float x) {
    const Rect al = alpha_rect();
    color_.a = clamp01((x - al.x) / al.w);
    if (on_change_) on_change_(color_);
}

bool ColorPicker::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    if (in_rect(x, y, sv_rect())) {
        drag_ = Drag::SV;
        apply_sv(x, y);
        return true;
    }
    if (in_rect(x, y, hue_rect())) {
        drag_ = Drag::Hue;
        apply_hue(x);
        return true;
    }
    if (alpha_ && in_rect(x, y, alpha_rect())) {
        drag_ = Drag::Alpha;
        apply_alpha(x);
        return true;
    }
    if (predefine_) {
        const Rect pr = predefine_rect();
        if (y >= pr.y && y <= pr.y + pr.h) {
            const float sw = (pr.w - (float)palette_.size() * 4.f - 4.f) / (float)palette_.size();
            for (size_t i = 0; i < palette_.size(); ++i) {
                const float px = pr.x + i * (sw + 4.f);
                if (x >= px && x <= px + sw) {
                    color_ = palette_[i];
                    rgba_to_hsv(color_, hue_, sat_, val_);
                    if (on_change_) on_change_(color_);
                    return true;
                }
            }
        }
    }
    if (in_rect(x, y, confirm_rect())) {
        if (on_done_) on_done_(color_);
        return true;
    }
    if (clearable_ && in_rect(x, y, clear_rect())) {
        color_ = {0.f, 0.f, 0.f, 1.f};
        hue_ = 0.f; sat_ = 0.f; val_ = 0.f;
        if (on_done_) on_done_(color_);
        return true;
    }
    return false;
}

bool ColorPicker::on_mouse_move(float x, float y) {
    if (drag_ == Drag::None) {
        confirm_hover_ = in_rect(x, y, confirm_rect());
        clear_hover_ = clearable_ && in_rect(x, y, clear_rect());
    } else if (drag_ == Drag::SV) {
        apply_sv(x, y);
    } else if (drag_ == Drag::Hue) {
        apply_hue(x);
    } else if (drag_ == Drag::Alpha) {
        apply_alpha(x);
    }
    return drag_ != Drag::None;
}

bool ColorPicker::on_mouse_up(float x, float y, int button) {
    if (button != 0) return false;
    drag_ = Drag::None;
    return false;
}

void ColorPicker::on_draw(Renderer& renderer) {
    // 面板底 + 边框
    renderer.draw_rounded_rect(bounds_, 4.f, Vec4{1.0f, 1.0f, 1.0f, 1.0f});
    renderer.draw_rect_outline(bounds_, 1.f, Vec4{0.85f, 0.85f, 0.85f, 1.0f});

    // —— SV 色场（网格着色）——
    const Rect sv = sv_rect();
    const int kCells = 12;
    const float cw = sv.w / kCells, ch = sv.h / kCells;
    for (int r = 0; r < kCells; ++r) {
        for (int c = 0; c < kCells; ++c) {
            const float s = (float)c / (float)(kCells - 1);
            const float v = 1.f - (float)r / (float)(kCells - 1);
            renderer.draw_quad({sv.x + c * cw, sv.y + r * ch, cw + 0.5f, ch + 0.5f},
                               hsv_to_rgba(hue_, s, v, 1.f));
        }
    }
    renderer.draw_rect_outline(sv, 1.f, Vec4{0.8f, 0.8f, 0.8f, 1.0f});
    // 选中点标记
    {
        const float cx = sv.x + sat_ * sv.w;
        const float cy = sv.y + (1.f - val_) * sv.h;
        renderer.draw_rect_outline({cx - 5.f, cy - 5.f, 10.f, 10.f}, 1.5f, Vec4{1.f, 1.f, 1.f, 1.f});
        renderer.draw_rect_outline({cx - 5.f, cy - 5.f, 10.f, 10.f}, 0.6f, Vec4{0.f, 0.f, 0.f, 0.6f});
    }

    // —— 右侧：当前色显示框 + 格式文本 ——
    const float rx = sv.x + sv.w + kPad;
    const float rw = bounds_.x + bounds_.w - rx - kPad;
    const float disp_h = sv.h * 0.5f - kPad * 0.5f;
    {
        const Rect sw{rx, sv.y, rw, disp_h};
        renderer.draw_rounded_rect(sw, 3.f, premult(color_));
        renderer.draw_rect_outline(sw, 1.f, Vec4{0.8f, 0.8f, 0.8f, 1.0f});
        if (font_) {
            const std::string txt = format_color();
            float tw = font_->measure_text(txt.c_str());
            font_->draw_text(renderer, txt.c_str(),
                             rx + (rw - tw) * 0.5f, sw.y + sw.h + kPad * 0.5f,
                             Vec4{0.35f, 0.35f, 0.35f, 1.0f});
        }
    }

    // —— 色相滑块 ——
    {
        const Rect hue = hue_rect();
        const int kSeg = 40;
        const float segw = hue.w / kSeg;
        for (int i = 0; i < kSeg; ++i) {
            Vec4 col = hsv_to_rgba((float)i / (float)kSeg, 1.f, 0.6f, 1.f);
            renderer.draw_quad({hue.x + i * segw, hue.y, segw + 0.5f, hue.h}, col);
        }
        renderer.draw_rect_outline(hue, 1.f, Vec4{0.8f, 0.8f, 0.8f, 1.0f});
        const float hx = hue.x + hue_ * hue.w;
        renderer.draw_quad({hx - 1.f, hue.y - 2.f, 2.f, hue.h + 4.f}, Vec4{0.15f, 0.15f, 0.15f, 1.f});
    }

    // —— 透明度滑块 ——
    if (alpha_) {
        const Rect al = alpha_rect();
        renderer.draw_quad(al, Vec4{0.3f, 0.3f, 0.3f, 1.f});  // 底
        const int kSeg = 40;
        const float segw = al.w / kSeg;
        for (int i = 0; i < kSeg; ++i) {
            Vec4 c = color_;
            c.a = (float)i / (float)kSeg;
            renderer.draw_quad({al.x + i * segw, al.y, segw + 0.5f, al.h}, premult(c));
        }
        renderer.draw_rect_outline(al, 1.f, Vec4{0.8f, 0.8f, 0.8f, 1.0f});
        const float ax = al.x + color_.a * al.w;
        renderer.draw_quad({ax - 1.f, al.y - 2.f, 2.f, al.h + 4.f}, Vec4{0.15f, 0.15f, 0.15f, 1.f});
    }

    // —— 预定义色块 ——
    if (predefine_) {
        const Rect pr = predefine_rect();
        const float sw = (pr.w - (float)palette_.size() * 4.f - 4.f) / (float)palette_.size();
        for (size_t i = 0; i < palette_.size(); ++i) {
            const Rect cell{pr.x + i * (sw + 4.f), pr.y, sw, pr.h};
            renderer.draw_rounded_rect(cell, 3.f, palette_[i]);
            const float dr = std::fabs(palette_[i].r - color_.r) + std::fabs(palette_[i].g - color_.g) +
                             std::fabs(palette_[i].b - color_.b);
            renderer.draw_rect_outline(cell, 1.f,
                                       dr < 0.01f ? ui::Vec4{0.2f, 0.6f, 1.0f, 1.f}
                                                  : ui::Vec4{0.88f, 0.88f, 0.88f, 1.f});
        }
    }

    // —— 确认 / 清除 ——
    {
        const Rect cf = confirm_rect();
        const Vec4 cb = confirm_hover_ ? Vec4{0.14f, 0.62f, 0.96f, 1.f} : Vec4{0.16f, 0.70f, 0.98f, 1.f};
        renderer.draw_rounded_rect(cf, 3.f, cb);
        if (font_)
            font_->draw_text(renderer, "确认", cf.x + (cf.w - font_->measure_text("确认")) * 0.5f,
                             cf.y + 6.f, Vec4{1.f, 1.f, 1.f, 1.f});
        if (clearable_) {
            const Rect cl = clear_rect();
            renderer.draw_rounded_rect(cl, 3.f, clear_hover_ ? Vec4{0.90f, 0.90f, 0.90f, 1.f}
                                                             : Vec4{0.95f, 0.95f, 0.95f, 1.f});
            renderer.draw_rect_outline(cl, 1.f, Vec4{0.8f, 0.8f, 0.8f, 1.f});
            if (font_)
                font_->draw_text(renderer, "清除", cl.x + (cl.w - font_->measure_text("清除")) * 0.5f,
                                 cl.y + 6.f, Vec4{0.35f, 0.35f, 0.35f, 1.f});
        }
    }
}

}  // namespace ui