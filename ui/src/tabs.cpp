#include "ui/ui/tabs.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

namespace {
constexpr float kPadX = 15.f;     // 标题左右内边距
constexpr float kCloseW = 20.f;   // 删除 × 区宽
constexpr float kOverflowW = 30.f;  // 超宽右端展开三角区宽
constexpr float kContentPad = 10.f; // 内容区内边距
}  // namespace

Tabs::Tabs(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-tab");
    content_label_ = new Label(0, 0, 1, 1, "");
    content_label_->set_wrap(true);
    add_child(content_label_);
    overflow_panel_ = new ScrollBox(0, 0, 1, 1);
    overflow_panel_->set_visible(false);  // UI-156
    overflow_panel_->set_padding(2.f);
    overflow_panel_->set_spacing(1.f);
    overflow_panel_->add_class("layui-form-select-panel");
}

Tabs::~Tabs() {
    // content_label_ 是 Tabs 的子节点（add_child），由 clear_children() 递归释放，析构不得再删（防二次释放）。
    // overflow_panel_ 不是子节点（展开时挂 root），析构前必须摘除，否则 root 悬空引用（同 ComboBox）。
    if (overflow_panel_->parent()) overflow_panel_->parent()->remove_child(overflow_panel_);
    delete overflow_panel_;
}

int Tabs::add_tab(const char* title, const char* content) {
    mark_dirty();
        Tab tb;
    tb.title = title ? title : "";
    tb.content = content ? content : "";
    tabs_.push_back(std::move(tb));
    layout_titles();
    return (int)tabs_.size() - 1;
}

void Tabs::set_content(int index, const char* content) {
    mark_dirty();
        if (index < 0 || index >= (int)tabs_.size()) return;
    tabs_[index].content = content ? content : "";
    if (index == active_) apply_active();
}

void Tabs::set_badge(const char* text) {
    mark_dirty();
        if (tabs_.empty()) return;
    tabs_.back().has_badge = true;
    tabs_.back().badge_text = text ? text : "";
    layout_titles();
}

void Tabs::set_dot_badge() {
    mark_dirty();
        if (tabs_.empty()) return;
    tabs_.back().has_dot = true;
    layout_titles();
}

void Tabs::remove_tab(int index) {
    mark_dirty();
        if (index < 0 || index >= (int)tabs_.size()) return;
    tabs_.erase(tabs_.begin() + index);
    if (tabs_.empty()) {
        active_ = 0;
        show_content("");
    } else {
        if (active_ >= (int)tabs_.size()) active_ = (int)tabs_.size() - 1;
        else if (index < active_) --active_;
        apply_active();
    }
    layout_titles();
}

void Tabs::set_active(int index) {
    mark_dirty();
        if (index < 0 || index >= (int)tabs_.size() || index == active_) return;
    active_ = index;
    apply_active();
    if (on_change_) on_change_(active_);
}

int Tabs::hit_tab(float x) const {
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (!tabs_[i].visible) continue;
        if (x >= tabs_[i].x0 && x <= tabs_[i].x1) return (int)i;
    }
    return -1;
}

bool Tabs::hit_close(float x, float y) const {
    if (!allow_close_) return false;
    if (y < bounds_.y || y > bounds_.y + kTitleH) return false;
    const int idx = hit_tab(x);
    if (idx < 0 || !tabs_[idx].visible) return false;
    return x >= tabs_[idx].x1 - kCloseW;
}

void Tabs::set_font(Font* font) {
    mark_dirty();
        font_ = font;
    if (content_label_) content_label_->set_font(font);
    for (Button* b : overflow_buttons()) b->set_font(font);
}

void Tabs::set_theme(const Theme* theme) {
    mark_dirty();
        theme_ = theme;
    content_label_->set_theme(theme);
    if (overflow_panel_) overflow_panel_->set_theme(theme);
}

float Tabs::content_h() const { return std::max(0.f, bounds_.h - kTitleH); }

float Tabs::overflow_x() const { return bounds_.x + bounds_.w - kOverflowW; }

void Tabs::apply_active() {
    if (active_ < 0 || active_ >= (int)tabs_.size()) return;
    show_content(tabs_[active_].content.c_str());
}

void Tabs::show_content(const char* text) {
    content_label_->set_text(text ? text : "");
    content_label_->set_theme(theme_);
}

float Tabs::title_width(const Tab& tb) const {
    // 单个标题宽 = 文本 + 内边距 + 徽章/圆点余量
    float w = font_ ? font_->measure_text(tb.title.c_str()) : 0.f;
    w += kPadX * 2;
    if (tb.has_badge) w += 6.f + (font_ ? font_->measure_text(tb.badge_text.c_str()) : 0.f) + 10.f;
    else if (tb.has_dot) w += 8.f + 8.f;
    return w;
}

void Tabs::layout_titles() {
    if (!font_) return;
    const float avail = std::max(0.f, bounds_.w - (overflow_ ? kOverflowW : 0.f));
    float x = bounds_.x;
    overflow_ = false;
    for (size_t i = 0; i < tabs_.size(); ++i) {
        Tab& tb = tabs_[i];
        const float w = title_width(tb);
        tb.x0 = x;
        tb.x1 = x + w;
        x += w;
        tb.visible = true;
    }
    // 超宽检测：标题总宽超出可用宽（不含右端展开区）
    const float total_w = (tabs_.empty() ? 0.f : tabs_.back().x1) - bounds_.x;
    if (total_w > avail && avail > 0.f) {
        overflow_ = true;
        // 保留右端展开区，收纳放不下的页（从前往后，至少保留一项）
        const float limit = bounds_.x + bounds_.w - kOverflowW;
        x = bounds_.x;
        bool first = true;
        for (size_t i = 0; i < tabs_.size(); ++i) {
            Tab& tb = tabs_[i];
            tb.x0 = x;
            tb.x1 = x + title_width(tb);
            if (tb.x1 > limit && !first) {
                tb.visible = false;
                // 不可见页不参与后续 x 累加，但保持 x 指针不动（被收纳）
            } else {
                tb.visible = true;
                first = false;
                x = tb.x1;
            }
        }
    }
}

void Tabs::open_overflow() {
    layout_titles();
    if (!overflow_ || tabs_.empty()) return;
    Widget* r = root();
    if (!r) return;
    if (!overflow_panel_->parent()) r->add_child(overflow_panel_);
    // 重建下拉项：列出全部被收纳页
    overflow_panel_->clear_children();
    const float panel_w = bounds_.w * 0.6f;
    const float panel_h = std::min((float)(tabs_.size() * 26 + 4), 150.f);
    overflow_panel_->set_bounds(bounds_.x + bounds_.w - panel_w, bounds_.y + kTitleH, panel_w, panel_h);
    for (size_t i = 0; i < tabs_.size(); ++i) {
        Button* b = new Button(0, 0, 0, 26.f, tabs_[i].title.c_str());
        b->add_class("layui-form-select dd");
        if ((int)i == active_) b->add_class("layui-this");
        b->set_font(font_);
        b->set_theme(theme_);
        b->set_on_click([this, i]() {
            set_active((int)i);
            close_popups();
        });
        overflow_panel_->add_child(b);
    }
    open_popup(this, overflow_panel_);
}

void Tabs::on_layout() {
    layout_titles();
    apply_active();
    const float ch = content_h();
    if (ch > 0.f) {
        content_label_->set_bounds(bounds_.x + kContentPad, bounds_.y + kTitleH + 2.f,
                                   std::max(1.f, bounds_.w - kContentPad * 2), std::max(1.f, ch - 2.f));
    }
}

float Tabs::preferred_height() const {
    return bounds_.h;
}

bool Tabs::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    if (y >= bounds_.y && y <= bounds_.y + kTitleH) {
        // 允许关闭：优先命中 ×（删除该页）
        if (hit_close(x, y)) {
            const int idx = hit_tab(x);
            if (idx >= 0) remove_tab(idx);
            return true;
        }
        // 超宽三角区
        if (overflow_ && x >= overflow_x()) {
            open_overflow();
            return true;
        }
        const int idx = hit_tab(x);
        if (idx >= 0) {
            set_active(idx);
            return true;
        }
    }
    return false;
}

void Tabs::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const bool card = has_class("layui-tab-card");
    const bool brief = has_class("layui-tab-brief");

    if (card) {
        // 卡片式：内容区白底 + 标题条 #fafafa + 整体 #eee 边框
        renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1, 1, 1, 1});
        renderer.draw_rect_outline(bounds_, 1.f, t->border);
        const Rect title_bar{bounds_.x, bounds_.y, bounds_.w, kTitleH};
        renderer.draw_quad(title_bar, Vec4{0.98f, 0.98f, 0.98f, 1.0f});
    } else {
        // 默认/brief：标题条底部 1px #eee 线
        renderer.draw_quad({bounds_.x, bounds_.y + kTitleH, bounds_.w, 1.f}, t->border);
    }
    if (card && brief) return;

    // 标题
    if (!font_) return;
    const float cy = bounds_.y + kTitleH * 0.5f;
    const float ty = cy - font_->line_height() * 0.5f;
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (!tabs_[i].visible) continue;
        const Tab& tb = tabs_[i];
        const bool active = (int)i == active_;
        const float tw = font_->measure_text(tb.title.c_str());
        const float tx = tb.x0 + (tb.x1 - tb.x0 - tw) * 0.5f;

        Vec4 color = active ? Vec4{0, 0, 0, 1} : t->text_muted;
        if (brief && active) color = t->accent;
        font_->draw_text(renderer, tb.title.c_str(), tx, ty, color);

        // 标题右缘徽章（layui-badge 红底白字）或小圆点（layui-badge-dot）
        float bx = tx + tw;
        if (tb.has_badge) {
            bx += 6.f;
            const float bw = font_->measure_text(tb.badge_text.c_str()) + 10.f;
            renderer.draw_rounded_rect({bx, ty + 1.f, bw, font_->line_height() - 2.f}, t->radius, t->danger);
            font_->draw_text(renderer, tb.badge_text.c_str(), bx + 5.f, ty, {1, 1, 1, 1});
            bx += bw;
        } else if (tb.has_dot) {
            bx += 8.f;
            const float r = 4.f;
            renderer.draw_rounded_rect({bx, ty + (font_->line_height() - 2.f * r) * 0.5f, 2.f * r, 2.f * r}, r, t->danger);
            bx += 2.f * r;
        }

        if (card && active) {
            renderer.draw_rounded_rect(Rect{tb.x0, bounds_.y, tb.x1 - tb.x0, kTitleH}, 2.f,
                                       Vec4{1, 1, 1, 1});
            font_->draw_text(renderer, tb.title.c_str(), tx, ty, color);
            // card 激活态文字后需重绘徽章（白底覆盖标题条后徽章也被盖）
            if (tb.has_badge) {
                const float bw = font_->measure_text(tb.badge_text.c_str()) + 10.f;
                renderer.draw_rounded_rect({bx - bw, ty + 1.f, bw, font_->line_height() - 2.f}, t->radius, t->danger);
                font_->draw_text(renderer, tb.badge_text.c_str(), bx - bw + 5.f, ty, {1, 1, 1, 1});
            } else if (tb.has_dot) {
                const float r = 4.f;
                renderer.draw_rounded_rect({bx - 2.f * r, ty + (font_->line_height() - 2.f * r) * 0.5f, 2.f * r, 2.f * r}, r, t->danger);
            }
        }
        if (brief && active) {
            const float lw = std::min(tw + 12.f, tb.x1 - tb.x0);
            renderer.draw_quad({tb.x0 + (tb.x1 - tb.x0 - lw) * 0.5f, bounds_.y + kTitleH - 2.f, lw, 2.f},
                               t->success);
        }
        // 允许关闭：标题右缘 ×（#999 → hover 不处理，点击命中 hit_close）
        if (allow_close_) {
            const float cx = tb.x1 - kCloseW + 10.f;
            const float dx = kCloseW * 0.5f - 5.f;
            font_->draw_text(renderer, "\xc3\x97", cx - 3.f, ty, Vec4{0.6f, 0.6f, 0.6f, 1.0f});
            (void)dx;
        }
    }

    // 超宽展开三角
    if (overflow_) {
        const float cx = overflow_x() + kOverflowW * 0.5f;
        const float cyy = bounds_.y + kTitleH * 0.5f;
        const Vec2 tri[3] = {{cx - 5.f, cyy - 3.f}, {cx + 5.f, cyy - 3.f}, {cx, cyy + 3.f}};
        renderer.draw_polygon(tri, 3, Vec4{0.76f, 0.76f, 0.76f, 1.0f});
    }
}

// 供 set_font 遍历下拉项（私有，内部用）
std::vector<Button*> Tabs::overflow_buttons() {
    std::vector<Button*> out;
    if (!overflow_panel_) return out;
    for (Widget* w : overflow_panel_->children()) {
        if (Button* b = dynamic_cast<Button*>(w)) out.push_back(b);
    }
    return out;
}

}  // namespace ui