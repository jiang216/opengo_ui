#include "ui/ui/titlebar.h"

#include <algorithm>

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/theme.h"

namespace ui {

TitleBar::TitleBar(float w, float h, const char* title, Window* win)
    : Widget(0, 0, w, h), title_(title ? title : ""), window_(win) {}

void TitleBar::set_title(const char* title) {
    title_ = title ? title : "";
    mark_dirty();
}

void TitleBar::set_icon(Font* font, uint32_t codepoint) {
    icon_font_ = font;
    icon_cp_ = codepoint;
    mark_dirty();
}

TitleBar::Btn TitleBar::hit_button(float x, float y) const {
    if (y < bounds_.y || y >= bounds_.y + bounds_.h) return Btn::None;
    const float right = bounds_.x + bounds_.w;
    const float bw = 28.f;
    if (close_button_ && x >= right - bw) return Btn::Close;
    if (maximize_button_ && x >= right - 2 * bw) return Btn::Maximize;
    if (minimize_button_ && x >= right - 3 * bw) return Btn::Minimize;
    return Btn::None;
}

void TitleBar::trigger(Btn b) {
    switch (b) {
        case Btn::Minimize:
            if (on_minimize_) on_minimize_();
            else if (window_) window_->iconify();
            break;
        case Btn::Maximize:
            if (on_maximize_) on_maximize_();
            else if (window_) {
                // 视窗口当前态切换 最大化/还原
                if (window_->is_maximized()) window_->restore();
                else window_->maximize();
            }
            break;
        case Btn::Close:
            if (on_close_) on_close_();
            break;
        default:
            break;
    }
}

bool TitleBar::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    const Btn b = hit_button(x, y);
    if (b != Btn::None) {
        pressed_ = b;
        dragging_ = false;
        mark_dirty();
        return true;  // 捕获；mouse_up 落在同键才触发
    }
    if (window_) {
        dragging_ = true;
        pressed_ = Btn::None;
        window_->start_drag(x, y);
        mark_dirty();
        return true;  // 捕获，拖动窗口期间 move 持续路由到本控件
    }
    return false;
}

bool TitleBar::on_mouse_move(float x, float y) {
    if (dragging_) {
        if (window_) window_->update_drag(x, y);
        return true;
    }
    // 按键 hover 反馈（非拖动时）
    const Btn b = hit_button(x, y);
    const bool changed = (b != hover_btn_);
    hover_btn_ = b;
    if (changed) mark_dirty();
    return pressed_ != Btn::None;
}

bool TitleBar::on_mouse_up(float x, float y, int button) {
    if (button != 0) return false;
    if (pressed_ != Btn::None) {
        const Btn fired = hit_button(x, y);  // 松键时同键才触发
        const Btn was = pressed_;
        pressed_ = Btn::None;
        mark_dirty();
        if (fired == was) trigger(was);
        return true;
    }
    if (dragging_) {
        dragging_ = false;
        if (window_) window_->end_drag();
        return true;
    }
    return false;
}

void TitleBar::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    (void)t;

    // 深色底：独立标题条深色（#262a2f），与下方 layui Header(#2f363c) 区分，避免视觉上无缝融合成一块
    renderer.draw_quad(bounds_, Vec4{0.149f, 0.165f, 0.184f, 1.0f});
    // 底部 1px 分隔线
    renderer.draw_quad({bounds_.x, bounds_.y + bounds_.h - 1.f, bounds_.w, 1.f},
                       Vec4{0.13f, 0.15f, 0.17f, 1.0f});

    const float right = bounds_.x + bounds_.w;
    const float bw = 28.f;
    const float line_h = font_ ? font_->line_height() : 14.f;

    // 右侧三键：最小化 / 最大化(还原) / 关闭（从右往左排）。
    // 图标优先用 iconfont 码点（减号/全屏/退出全屏/关闭）绘制，视觉统一；icon_font_ 未烘培
    // iconfont 或无 iconfont 时优雅回退到文本字形（—/□/⧉/×），不中断（UI-171）。
    const bool use_icon = icon_font_ && icon_font_->icon_loaded();
    auto draw_button = [&](bool visible, float bx01, Btn id, uint32_t icon_cp,
                           const char* text_glyph) {
        if (!visible || !font_) return;
        const float bx = right - bx01 * bw;
        if (pressed_ == id || hover_btn_ == id) {
            renderer.draw_quad({bx, bounds_.y, bw, bounds_.h},
                               Vec4{0.80f, 0.25f, 0.22f, 1.0f});  // 关闭 hover/按下红底
        }
        const Vec4 c = (pressed_ == id || hover_btn_ == id) ? Vec4{1, 1, 1, 1}
                                                            : Vec4{0.68f, 0.70f, 0.73f, 1.0f};
        if (use_icon && icon_cp) {
            const float isz = std::min(16.f, bounds_.h * 0.6f);
            icon_font_->draw_icon(renderer, icon_cp, bx + bw * 0.5f,
                                  bounds_.y + bounds_.h * 0.5f, isz, c);
        } else {
            const float ty = bounds_.y + (bounds_.h - line_h) * 0.5f;
            font_->draw_text(renderer, text_glyph, bx + (bw - font_->measure_text(text_glyph)) * 0.5f,
                             ty, c);
        }
    };
    // close 在最右
    draw_button(close_button_, 1.f, Btn::Close, layui_icon::close, "×");
    // 还原/最大化图标随窗口态切换（全屏 ↔ 退出全屏）
    const uint32_t max_cp = (window_ && window_->is_maximized()) ? layui_icon::screen_restore
                                                                 : layui_icon::screen_full;
    const char* max_glyph = (window_ && window_->is_maximized()) ? "⧉" : "□";
    draw_button(maximize_button_, 2.f, Btn::Maximize, max_cp, max_glyph);
    // 最小化=减号
    draw_button(minimize_button_, 3.f, Btn::Minimize, layui_icon::subtraction, "—");

    // 左上角应用图标（需外部 set_icon 提供 iconfont 码点）+ 标题文字
    float tx = bounds_.x + 10.f;
    if (icon_font_ && icon_cp_) {
        const float isz = bounds_.h > 20.f ? 20.f : bounds_.h;
        const float icx = bounds_.x + 8.f + isz * 0.5f;
        const float icy = bounds_.y + bounds_.h * 0.5f;
        icon_font_->draw_icon(renderer, icon_cp_, icx, icy, isz,
                              Vec4{0.55f, 0.85f, 0.95f, 1.0f});
        tx = bounds_.x + 8.f + isz + 8.f;
    }
    if (font_ && !title_.empty()) {
        const float ty = bounds_.y + (bounds_.h - line_h) * 0.5f;
        // 标题文字裁剪在最大键左侧，避免压到右侧窗口键
        const float max_text_w = (right - 3 * bw) - tx - 8.f;
        const std::string t = title_.size() > 64 ? title_.substr(0, 64) : title_;
        if (max_text_w > 0.f)
            font_->draw_text(renderer, t.c_str(), tx, ty,
                             Vec4{0.88f, 0.89f, 0.90f, 1.0f});
    }
}

}  // namespace ui
