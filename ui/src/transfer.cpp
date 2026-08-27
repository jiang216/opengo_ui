#include "ui/ui/transfer.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/textbox.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <string>

namespace ui {

namespace {
constexpr float kHeaderH = 22.f;  // 标题条高度
constexpr float kRowH = 26.f;     // 列表行高
constexpr float kSearchH = 26.f;  // 搜索框区高度（showSearch 时列表下移）
constexpr float kMidGap = 40.f;   // 中间按钮区宽度（每侧各让出）
}

Transfer::Transfer(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void Transfer::add_item(const char* name, bool disabled) {
    mark_dirty();
        left_.items.push_back(name ? name : "");
    left_.disabled.push_back(disabled);
    left_.sel.push_back(false);
}

void Transfer::add_right_item(const char* name, bool disabled) {
    right_.items.push_back(name ? name : "");
    right_.disabled.push_back(disabled);
    right_.sel.push_back(false);
}

void Transfer::clear() {
    mark_dirty();
        left_ = Side{};
    right_ = Side{};
}

void Transfer::set_show_search(bool on) {
    mark_dirty();
        show_search_ = on;
    if (on) {
        if (!search_l_) {
            search_l_ = new TextBox(0, 0, 0, 0);
            search_l_->set_placeholder("搜索");
            search_l_->set_on_change([this](const char* t) { left_.filter = t ? t : ""; });
            add_child(search_l_);
        }
        if (!search_r_) {
            search_r_ = new TextBox(0, 0, 0, 0);
            search_r_->set_placeholder("搜索");
            search_r_->set_on_change([this](const char* t) { right_.filter = t ? t : ""; });
            add_child(search_r_);
        }
    }
}

std::vector<int> Transfer::visible_rows(const Side& s) const {
    std::vector<int> out;
    for (size_t i = 0; i < s.items.size(); ++i) {
        if (s.filter.empty() || s.items[i].find(s.filter) != std::string::npos) out.push_back((int)i);
    }
    return out;
}

float Transfer::list_view_h(const Rect& panel, bool search_on) const {
    return panel.h - kHeaderH - (search_on ? kSearchH : 0.f);
}

// 内容总高超过可视高度时，把 scroll 夹紧到有效范围
void Transfer::clamp_scroll(Side& s, const Rect& panel, bool search_on) {
    const float view_h = list_view_h(panel, search_on);
    const int vis = (int)visible_rows(s).size();
    const float content_h = (float)vis * kRowH;
    if (content_h <= view_h) s.scroll = 0.f;
    else {
        const float max_scroll = content_h - view_h;
        if (s.scroll > max_scroll) s.scroll = max_scroll;
    }
    if (s.scroll < 0.f) s.scroll = 0.f;
}

int Transfer::hit_row(const Side& s, const Rect& panel, float x, float y, bool search_on) const {
    if (x < panel.x || x > panel.x + panel.w || y < panel.y || y > panel.y + panel.h) return -1;
    const float top = panel.y + kHeaderH + (search_on ? kSearchH : 0.f);
    if (y < top || y > top + list_view_h(panel, search_on)) return -1;  // 仅可视区可命中
    const int vi = (int)((y - top + s.scroll) / kRowH);
    if (vi < 0) return -1;
    const std::vector<int> vis = visible_rows(s);
    return (vi < (int)vis.size()) ? vis[vi] : -1;
}

void Transfer::move(TransferDirection dir) {
    Side& from = (dir == TransferDirection::ToRight) ? left_ : right_;
    Side& to = (dir == TransferDirection::ToRight) ? right_ : left_;
    std::vector<std::string> moved;
    std::vector<std::string> keep_items;
    std::vector<bool> keep_disabled, keep_sel;
    for (size_t i = 0; i < from.items.size(); ++i) {
        if (i < from.sel.size() && from.sel[i] && !(i < from.disabled.size() && from.disabled[i])) {
            moved.push_back(from.items[i]);
        } else {
            keep_items.push_back(from.items[i]);
            keep_disabled.push_back(i < from.disabled.size() && from.disabled[i]);
            keep_sel.push_back(false);
        }
    }
    from.items = keep_items;
    from.disabled = keep_disabled;
    from.sel = keep_sel;
    for (const std::string& name : moved) {
        to.items.push_back(name);
        to.disabled.push_back(false);
        to.sel.push_back(false);
    }
    emit_change(dir, moved);
}

void Transfer::emit_change(TransferDirection dir, const std::vector<std::string>& moved) {
    if (on_change_) on_change_(dir, moved);
}

bool Transfer::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    const float mid = bounds_.x + bounds_.w / 2.f;
    const Rect panel_l{bounds_.x, bounds_.y, mid - bounds_.x - kMidGap, bounds_.h};
    const Rect panel_r{mid + kMidGap, bounds_.y, bounds_.x + bounds_.w - mid - kMidGap, bounds_.h};
    // 中间两个按钮（上下排列）
    const float btn_y = bounds_.y + bounds_.h / 2.f - 28.f;
    const Rect btn_r{mid - 16.f, btn_y, 32.f, 22.f};
    const Rect btn_l{mid - 16.f, btn_y + 30.f, 32.f, 22.f};
    if (x >= btn_r.x && x <= btn_r.x + btn_r.w && y >= btn_r.y && y <= btn_r.y + btn_r.w * 0.7f) {
        move(TransferDirection::ToRight);
        return true;
    }
    if (x >= btn_l.x && x <= btn_l.x + btn_l.w && y >= btn_l.y && y <= btn_l.y + btn_l.w * 0.7f) {
        move(TransferDirection::ToLeft);
        return true;
    }
    // 列表项选中（跳过禁用项）
    int idx = hit_row(left_, panel_l, x, y, show_search_);
    if (idx >= 0 && !(idx < (int)left_.disabled.size() && left_.disabled[idx])) {
        left_.sel[idx] = !left_.sel[idx];
        return true;
    }
    idx = hit_row(right_, panel_r, x, y, show_search_);
    if (idx >= 0 && !(idx < (int)right_.disabled.size() && right_.disabled[idx])) {
        right_.sel[idx] = !right_.sel[idx];
        return true;
    }
    return false;
}

bool Transfer::on_mouse_scroll(float x, float y, float dx, float dy) {
    const float mid = bounds_.x + bounds_.w / 2.f;
    const Rect panel_l{bounds_.x, bounds_.y, mid - bounds_.x - kMidGap, bounds_.h};
    const Rect panel_r{mid + kMidGap, bounds_.y, bounds_.x + bounds_.w - mid - kMidGap, bounds_.h};
    Side* side = nullptr;
    const Rect* panel = nullptr;
    if (x >= panel_l.x && x <= panel_l.x + panel_l.w && y >= panel_l.y && y <= panel_l.y + panel_l.h) {
        side = &left_; panel = &panel_l;
    } else if (x >= panel_r.x && x <= panel_r.x + panel_r.w && y >= panel_r.y && y <= panel_r.y + panel_r.h) {
        side = &right_; panel = &panel_r;
    }
    if (!side || !panel) return false;
    side->scroll -= dy * 26.f;  // 每格滚轮约一行
    clamp_scroll(*side, *panel, show_search_);
    mark_dirty();
    return true;
}

void Transfer::on_layout() {
    if (show_search_ && search_l_ && search_r_) {
        const float mid = bounds_.x + bounds_.w / 2.f;
        const Rect pl{bounds_.x, bounds_.y, mid - bounds_.x - kMidGap, bounds_.h};
        const Rect pr{mid + kMidGap, bounds_.y, bounds_.x + bounds_.w - mid - kMidGap, bounds_.h};
        search_l_->set_bounds(pl.x + 6.f, pl.y + kHeaderH + 2.f, pl.w - 12.f, kSearchH - 6.f);
        search_r_->set_bounds(pr.x + 6.f, pr.y + kHeaderH + 2.f, pr.w - 12.f, kSearchH - 6.f);
        // 保持字体/主题同步（set_show_search 可能在 set_font/set_theme 之前调用）
        search_l_->set_font(font_);
        search_l_->set_theme(theme_);
        search_r_->set_font(font_);
        search_r_->set_theme(theme_);
    }
}

void Transfer::on_draw(Renderer& renderer) {
    const float mid = bounds_.x + bounds_.w / 2.f;
    const Rect panel_l{bounds_.x, bounds_.y, mid - bounds_.x - kMidGap, bounds_.h};
    const Rect panel_r{mid + kMidGap, bounds_.y, bounds_.x + bounds_.w - mid - kMidGap, bounds_.h};
    const Vec4 border{0.85f, 0.85f, 0.85f, 1.0f};
    const Vec4 header{0.96f, 0.96f, 0.96f, 1.0f};
    // 两个面板
    for (const Rect& p : {panel_l, panel_r}) {
        renderer.draw_quad(p, Vec4{1, 1, 1, 1});
        renderer.draw_rect_outline(p, 1.f, border);
        renderer.draw_quad(Rect{p.x, p.y, p.w, kHeaderH}, header);
    }
    // 中间按钮
    const float btn_y = bounds_.y + bounds_.h / 2.f - 28.f;
    const Vec4 btn_col{0.12f, 0.62f, 1.0f, 1.0f};  // layui 蓝
    renderer.draw_rounded_rect(Rect{mid - 16.f, btn_y, 32.f, 22.f}, 3.f, btn_col);
    renderer.draw_rounded_rect(Rect{mid - 16.f, btn_y + 30.f, 32.f, 22.f}, 3.f, btn_col);
    if (!font_) return;
    font_->draw_text(renderer, ">", mid - 3.f, btn_y + 3.f, Vec4{1, 1, 1, 1});
    font_->draw_text(renderer, "<", mid - 3.f, btn_y + 33.f, Vec4{1, 1, 1, 1});
    // 标题 + 计数
    char title[64];
    std::snprintf(title, sizeof(title), "%s（%d）", left_label_.c_str(), (int)left_.items.size());
    font_->draw_text(renderer, title, panel_l.x + 6.f, panel_l.y + 2.f, Vec4{0.3f, 0.3f, 0.3f, 1.0f});
    std::snprintf(title, sizeof(title), "%s（%d）", right_label_.c_str(), (int)right_.items.size());
    font_->draw_text(renderer, title, panel_r.x + 6.f, panel_r.y + 2.f, Vec4{0.3f, 0.3f, 0.3f, 1.0f});
    // 列表项（含搜索过滤映射 + 滚动偏移 + 复选框），scissor 裁剪到面板可视区
    constexpr float kCBSize = 14.f;   // 列表行复选框尺寸（比标准 18px 紧凑）
    constexpr float kCBRadius = 3.f;
    const Vec4 scroll_col{0.82f, 0.82f, 0.82f, 1.0f};
    const Theme* t = theme_ ? theme_ : nullptr;
    const Vec4 accent_col = t ? t->accent : Vec4{0.09f, 0.73f, 0.67f, 1.f};
    const auto draw_list = [&](Side& s, const Rect& p) {
        clamp_scroll(s, p, show_search_);
        const float top = p.y + kHeaderH + (show_search_ ? kSearchH : 0.f);
        const float view_h = list_view_h(p, show_search_);
        renderer.push_scissor(Rect{p.x + 1.f, top, p.w - 2.f, view_h - 1.f});
        const std::vector<int> vis = visible_rows(s);
        for (size_t vi = 0; vi < vis.size(); ++vi) {
            const int i = vis[vi];
            const float row_y = top + vi * kRowH - s.scroll;
            if (row_y + kRowH < top || row_y > top + view_h) continue;
            const Rect row{p.x, row_y, p.w, kRowH - 2.f};
            const bool checked = i < (int)s.sel.size() && s.sel[i];
            const bool disabled = i < (int)s.disabled.size() && s.disabled[i];
            // 复选框：行内左侧居中
            const float cb_y = row.y + (row.h - kCBSize) * 0.5f;
            const Rect cb{row.x + 6.f, cb_y, kCBSize, kCBSize};
            if (checked) {
                renderer.draw_rounded_rect(cb, kCBRadius, accent_col);
                const Rect inner{cb.x + 2.f, cb.y + 2.f, cb.w - 4.f, cb.h - 4.f};
                renderer.draw_rounded_rect(inner, kCBRadius - 1.f, accent_col);
                if (font_) font_->draw_text(renderer, "\xe2\x9c\x93", cb.x + 2.f, cb.y + 1.f, Vec4{1, 1, 1, 1});  // ✓
            } else {
                renderer.draw_rounded_rect(cb, kCBRadius, Vec4{0.82f, 0.82f, 0.82f, 1.f});
                const Rect inner{cb.x + 2.f, cb.y + 2.f, cb.w - 4.f, cb.h - 4.f};
                renderer.draw_rounded_rect(inner, kCBRadius - 1.f, Vec4{1, 1, 1, 1});
            }
            // 文本（复选框右侧）
            if (font_) {
                const float tx = cb.x + kCBSize + 6.f;
                font_->draw_text(renderer, s.items[i].c_str(), tx, row.y + 4.f,
                                 disabled ? Vec4{0.72f, 0.72f, 0.72f, 1.f} : Vec4{0.35f, 0.35f, 0.35f, 1.f});
            }
        }
        renderer.pop_scissor();
        // 滚动条（内容超出可视区时显示）
        const int vis_n = (int)vis.size();
        const float content_h = (float)vis_n * kRowH;
        if (content_h > view_h) {
            const float track_x = p.x + p.w - 10.f;
            const float track_y = top;
            const float track_h = view_h - 2.f;
            renderer.draw_rounded_rect(Rect{track_x, track_y, 4.f, track_h}, 2.f, Vec4{0.93f, 0.93f, 0.93f, 1.f});
            const float thumb_h = track_h * (view_h / content_h);
            const float thumb_y = track_y + (track_h - thumb_h) * (s.scroll / (content_h - view_h));
            renderer.draw_rounded_rect(Rect{track_x, thumb_y, 4.f, thumb_h < 6.f ? 6.f : thumb_h}, 2.f, scroll_col);
        }
    };
    draw_list(left_, panel_l);
    draw_list(right_, panel_r);
}

}  // namespace ui
