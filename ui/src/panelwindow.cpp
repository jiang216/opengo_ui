#include "ui/ui/panelwindow.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <cmath>

namespace ui {

PanelWindow::PanelWindow(float x, float y, float w, float h, const char* title)
    : Widget(x, y, w, h), title_(title ? title : "") {}

void PanelWindow::set_footer_buttons(const std::vector<std::string>& labels, int align,
                                     std::function<void(int)> cb) {
    footer_buttons_ = labels;
    footer_align_ = align;
    on_footer_ = std::move(cb);
}

float PanelWindow::title_height() const {
    if (!title_visible_) return 0.f;
    return has_class("layui-layer") ? 50.f : title_h_;
}

Rect PanelWindow::client_rect() const {
    return {bounds_.x, bounds_.y + title_height(), bounds_.w,
            std::max(0.f, bounds_.h - title_height() - footer_height())};
}

bool PanelWindow::in_title(float x, float y) const {
    const float th = title_height();
    if (th <= 0.f) return false;
    return x >= bounds_.x && x < bounds_.x + bounds_.w && y >= bounds_.y &&
           y < bounds_.y + th;
}

bool PanelWindow::in_close(float x, float y) const {
    if (!close_visible_ || !title_visible_) return false;
    const float cx = bounds_.x + bounds_.w - 30.f;
    return x >= cx && x < bounds_.x + bounds_.w && y >= bounds_.y &&
           y < bounds_.y + title_height();
}

bool PanelWindow::in_maxmin(float x, float y) const {
    if (!maxmin_ || !title_visible_) return false;
    const float cx = bounds_.x + bounds_.w - (close_visible_ ? 60.f : 30.f);
    return x >= cx && x < bounds_.x + bounds_.w - (close_visible_ ? 30.f : 0.f) &&
           y >= bounds_.y && y < bounds_.y + title_height();
}

bool PanelWindow::in_resize(float x, float y) const {
    if (maximized_) return false;
    return x >= bounds_.x + bounds_.w - corner_ && y >= bounds_.y + bounds_.h - corner_;
}

bool PanelWindow::in_footer(float x, float y, int& out_index) const {
    out_index = -1;
    if (footer_buttons_.empty()) return false;
    const float fy = bounds_.y + bounds_.h - footer_h_;
    if (y < fy || y >= bounds_.y + bounds_.h) return false;
    // 按钮条底部对齐：按钮高度 40，垂直居中于 footer_h_
    const float btn_h = 40.f;
    const float btn_top = bounds_.y + bounds_.h - (footer_h_ + btn_h) * 0.5f;
    if (y < btn_top || y >= btn_top + btn_h) return false;
    const float gap = 8.f;
    float total = 0.f;
    for (const auto& s : footer_buttons_) total += (font_ ? font_->measure_text(s.c_str()) : 60.f) + 2.f * 26.f + gap;
    total -= gap;
    const float pad = std::max(0.f, (bounds_.w - total) * (footer_align_ == 1 ? 0.5f : (footer_align_ == 2 ? 1.f : 0.f)));
    float bx = bounds_.x + pad;
    for (size_t i = 0; i < footer_buttons_.size(); ++i) {
        const float bw = (font_ ? font_->measure_text(footer_buttons_[i].c_str()) : 60.f) + 2.f * 26.f;
        if (x >= bx && x < bx + bw && y >= btn_top && y < btn_top + btn_h) {
            out_index = (int)i;
            return true;
        }
        bx += bw + gap;
    }
    return false;
}

void PanelWindow::on_layout() {
    const Rect c = client_rect();
    // 同步 child_layout_info_ 与 children_：移除已不在 children_ 中的条目
    for (auto it = child_layout_info_.begin(); it != child_layout_info_.end();) {
        bool found = false;
        for (Widget* child : children_) {
            if (child == it->widget) { found = true; break; }
        }
        it = found ? ++it : child_layout_info_.erase(it);
    }
    // 为新加入的子控件记录其相对客户区的偏移（首次见到时，子控件 bounds 即为创建时的相对坐标）
    for (Widget* child : children_) {
        if (!child->visible) continue;
        bool found = false;
        for (auto& info : child_layout_info_) {
            if (info.widget == child) { found = true; break; }
        }
        if (!found) {
            child_layout_info_.push_back({child, {child->bounds().x, child->bounds().y,
                                                 child->bounds().w, child->bounds().h}});
        }
    }
    // 基于存储的相对偏移重新定位所有子控件到客户区内
    for (auto& info : child_layout_info_) {
        if (!info.widget->visible) continue;
        info.widget->set_bounds(c.x + info.rel.x, c.y + info.rel.y,
                                info.widget->bounds().w, info.widget->bounds().h);
    }
}

bool PanelWindow::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    // 点击窗口任意区域置顶（layer.setTop）：把自己移到父节点子控件末尾（后添加者在上层）
    bring_to_front();
    int fi = -1;
    if (in_footer(x, y, fi)) {
        if (on_footer_ && fi >= 0) on_footer_(fi);
        return true;
    }
    if (in_close(x, y)) {
        if (on_close_) on_close_();
        return true;
    }
    if (in_maxmin(x, y)) {
        toggle_maximize();
        return true;
    }
    if (in_resize(x, y)) {
        resizing_ = true;
        drag_dx_ = bounds_.w - (x - bounds_.x);
        drag_dy_ = bounds_.h - (y - bounds_.y);
        capture_mouse();
        return true;
    }
    if (in_title(x, y)) {
        dragging_ = true;
        drag_dx_ = x - bounds_.x;
        drag_dy_ = y - bounds_.y;
        capture_mouse();
        return true;
    }
    return false;
}

bool PanelWindow::on_mouse_move(float x, float y) {
    int fi = -1;
    footer_hover_ = in_footer(x, y, fi) ? fi : -1;
    if (dragging_) {
        const float nx = x - drag_dx_;
        const float ny = y - drag_dy_;
        const float dx = nx - bounds_.x;
        const float dy = ny - bounds_.y;
        set_bounds(nx, ny, bounds_.w, bounds_.h);
        // 子控件跟随窗口平移
        for (Widget* child : children_) {
            if (child->visible)
                child->set_bounds(child->bounds().x + dx, child->bounds().y + dy,
                                  child->bounds().w, child->bounds().h);
        }
        return true;
    }
    if (resizing_) {
        const float w = std::max(160.f, x - bounds_.x + drag_dx_);
        const float h = std::max(120.f, y - bounds_.y + drag_dy_);
        set_bounds(bounds_.x, bounds_.y, w, h);
        return true;
    }
    return false;
}

bool PanelWindow::on_mouse_up(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    dragging_ = false;
    resizing_ = false;
    release_mouse_capture();
    return true;
}

void PanelWindow::on_advance(float dt) {
    if (auto_close_ > 0.f) {
        auto_close_ -= dt;
        if (auto_close_ <= 0.f) {
            if (on_close_) on_close_();
        }
    }
}

void PanelWindow::toggle_maximize() {
    Widget* r = root();
    const float vw = (float)(r ? r->bounds().w : bounds_.w);
    const float vh = (float)(r ? r->bounds().h : bounds_.h);
    if (!maximized_) {
        restore_rect_ = bounds_;
        set_bounds(0, 0, vw, vh);
        maximized_ = true;
    } else {
        set_bounds(restore_rect_.x, restore_rect_.y, restore_rect_.w, restore_rect_.h);
        maximized_ = false;
    }
}

void PanelWindow::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const bool layer = has_class("layui-layer");
    const float th = title_height();

    // 底部遮罩（layer shade）：先于窗口绘制，覆盖整个视口
    if (shade_ > 0.f) {
        const Rect full{0.f, 0.f, (float)renderer.viewport_width(), (float)renderer.viewport_height()};
        renderer.draw_quad(full, Vec4{0.f, 0.f, 0.f, shade_});
    }

    if (layer) {
        const bool notice = has_class("layui-layer-notice");
        const Vec4 win_bg = notice ? Vec4{0.224f, 0.239f, 0.286f, 1.0f}  // #393D49
                                   : Vec4{1, 1, 1, 1};
        // layui-layer 外观：白底圆角2 + 阴影
        const Rect sh{bounds_.x + 4.f, bounds_.y + 6.f, bounds_.w, bounds_.h};
        renderer.draw_rounded_rect(sh, t->radius, Vec4{0, 0, 0, 0.18f});
        renderer.draw_rounded_rect(bounds_, t->radius, win_bg);
        if (th > 0.f) {
            // 标题栏白底 + 下边框
            const Rect bar{bounds_.x, bounds_.y, bounds_.w, th};
            renderer.draw_quad(bar, win_bg);
            renderer.draw_quad({bar.x, bar.y + bar.h - 1.f, bar.w, 1.f}, t->border);
            if (font_ && !title_.empty()) {
                const float ty = bounds_.y + (th - font_->line_height()) * 0.5f;
                font_->draw_text(renderer, title_.c_str(), bounds_.x + 14.f, ty,
                                 notice ? Vec4{1, 1, 1, 1.0f} : Vec4{0.16f, 0.16f, 0.16f, 1.0f});
            }
            if (maxmin_ && font_) {
                // 最大化/还原按钮：置于关闭钮左侧
                const float mx = bounds_.x + bounds_.w - (close_visible_ ? 58.f : 28.f);
                const float my = bounds_.y + (th - font_->line_height()) * 0.5f;
                font_->draw_text(renderer, maximized_ ? "□" : "□", mx, my, Vec4{0.6f, 0.6f, 0.6f, 1.0f});
            }
            // 关闭按钮：右上角小方块 ×
            if (close_visible_ && font_) {
                const float cx = bounds_.x + bounds_.w - 24.f;
                const float cy = bounds_.y + (th - font_->line_height()) * 0.5f;
                font_->draw_text(renderer, "x", cx, cy, Vec4{0.6f, 0.6f, 0.6f, 1.0f});
            }
        }
    } else {
        // 窗口背景
        renderer.draw_rounded_rect(bounds_, t->radius, t->button_normal);
        // 标题栏
        const Rect bar{bounds_.x, bounds_.y, bounds_.w, th};
        renderer.draw_rounded_rect(bar, t->radius, t->accent);
        if (font_ && !title_.empty() && th > 0.f) {
            const float ty = bounds_.y + (th - (font_->ascent() - font_->descent())) * 0.5f;
            font_->draw_text(renderer, title_.c_str(), bounds_.x + 10.f, ty, t->text);
        }
        // 关闭按钮 'x'
        if (close_visible_ && font_ && th > 0.f) {
            font_->draw_text(renderer, "x", bounds_.x + bounds_.w - 20.f, bounds_.y + 6.f, t->text);
        }
        // 缩放柄
        const Rect grip{bounds_.x + bounds_.w - 10.f, bounds_.y + bounds_.h - 10.f, 6.f, 6.f};
        renderer.draw_rounded_rect(grip, 2.f, t->text_dim);
    }

    // 底部按钮条（layer btn）
    if (!footer_buttons_.empty()) {
        const float fy = bounds_.y + bounds_.h - footer_h_;
        renderer.draw_quad({bounds_.x, fy, bounds_.w, 1.f}, t->border);
        const float btn_h = 40.f;
        const float btn_top = bounds_.y + bounds_.h - (footer_h_ + btn_h) * 0.5f;
        const float gap = 8.f;
        float total = 0.f;
        for (const auto& s : footer_buttons_) total += (font_ ? font_->measure_text(s.c_str()) : 60.f) + 2.f * 26.f + gap;
        total -= gap;
        const float pad = std::max(0.f, (bounds_.w - total) * (footer_align_ == 1 ? 0.5f : (footer_align_ == 2 ? 1.f : 0.f)));
        float bx = bounds_.x + pad;
        for (size_t i = 0; i < footer_buttons_.size(); ++i) {
            const float bw = (font_ ? font_->measure_text(footer_buttons_[i].c_str()) : 60.f) + 2.f * 26.f;
            const Rect br{bx, btn_top, bw, btn_h};
            const Vec4 bg = (int)i == footer_hover_ ? Vec4{0.86f, 0.87f, 0.89f, 1.0f}
                                                    : Vec4{0.95f, 0.95f, 0.95f, 1.0f};
            renderer.draw_rounded_rect(br, t->radius, bg);
            if (font_) {
                const float tx = bx + (bw - font_->measure_text(footer_buttons_[i].c_str())) * 0.5f;
                const float ty = btn_top + (btn_h - font_->line_height()) * 0.5f;
                font_->draw_text(renderer, footer_buttons_[i].c_str(), tx, ty, Vec4{0.33f, 0.33f, 0.33f, 1.0f});
            }
            bx += bw + gap;
        }
    }
}

}  // namespace ui
