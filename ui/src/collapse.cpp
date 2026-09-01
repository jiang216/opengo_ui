#include "ui/ui/collapse.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
constexpr float kTitleH = 42.f;   // layui 标题条高 42px
constexpr float kItemGap = 8.f;   // 面板项间距
constexpr float kContentPadY = 10.f;
constexpr float kContentPadX = 15.f;
}  // namespace

Collapse::Collapse(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-collapse");
}

void Collapse::add_item(const char* title) {
    mark_dirty();
        items_.push_back({title ? title : "", children_.size(), 0.f, 0.f});
    open_.push_back(false);
}

void Collapse::set_active(int index) {
    mark_dirty();
        if (index < 0 || index >= (int)items_.size()) return;
    active_ = index;
    if (accordion_) {
        // 手风琴：点开已展开项则收起，否则只开该项
        bool was_open = is_open_impl(index);
        std::fill(open_.begin(), open_.end(), false);
        if (!was_open) open_[index] = true;
    } else {
        // 非手风琴：toggle 该项
        open_[index] = !open_[index];
    }
    layout_items();
    if (on_change_) on_change_(active_);
}

int Collapse::active() const {
    if (accordion_) return active_;
    // 非手风琴：返回最后操作项；若该项已收起则回退到仍展开项
    if (active_ >= 0 && is_open_impl(active_)) return active_;
    for (size_t i = 0; i < open_.size(); ++i)
        if (open_[i]) return (int)i;
    return -1;
}

bool Collapse::is_open(int index) const {
    return is_open_impl(index);
}

bool Collapse::is_open_impl(int index) const {
    if (index < 0 || index >= (int)open_.size()) return false;
    if (accordion_) return open_[index];
    return open_[index];
}

int Collapse::open_count() const {
    int n = 0;
    for (bool b : open_) if (b) ++n;
    return n;
}

void Collapse::set_font(Font* font) { mark_dirty();
        font_ = font; }

void Collapse::set_theme(const Theme* theme) { mark_dirty();
        theme_ = theme; }

void Collapse::layout_items() {
    float y = bounds_.y;
    for (size_t i = 0; i < items_.size(); ++i) {
        Item& it = items_[i];
        it.y0 = y;
        it.y1 = y + kTitleH;
        y += kTitleH;

        const bool expanded = is_open_impl((int)i);
        const size_t begin = it.child_begin;
        const size_t end = (i + 1 < items_.size()) ? items_[i + 1].child_begin : children_.size();
        for (size_t c = begin; c < end; ++c) {
            Widget* child = children_[c];
            child->set_visible(expanded);  // UI-156：经 set_visible（含 mark_dirty）
            if (expanded) {
                float ch = child->preferred_height();
                if (ch <= 0) ch = 24.f;
                child->set_bounds(bounds_.x + kContentPadX, y, bounds_.w - kContentPadX * 2, ch);
                y += ch + kItemGap;
            }
        }
    }
}

void Collapse::on_layout() { layout_items(); }

float Collapse::preferred_height() const {
    float h = 0.f;
    for (size_t i = 0; i < items_.size(); ++i) {
        h += kTitleH;
        if (is_open_impl((int)i)) {
            const size_t begin = items_[i].child_begin;
            const size_t end = (i + 1 < items_.size()) ? items_[i + 1].child_begin : children_.size();
            for (size_t c = begin; c < end; ++c) {
                float ch = children_[c]->preferred_height();
                if (ch <= 0) ch = 24.f;
                h += ch + kItemGap;
            }
        }
    }
    return h;
}

bool Collapse::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (y >= items_[i].y0 && y <= items_[i].y1) {
            set_active((int)i);
            return true;
        }
    }
    return false;
}

void Collapse::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    // 外框 1px #eee 圆角 2
    renderer.draw_rect_outline(bounds_, 1.f, t->border);

    if (!font_) return;
    for (size_t i = 0; i < items_.size(); ++i) {
        const Item& it = items_[i];
        const bool expanded = is_open_impl((int)i);
        // 标题条 #fafafa 底 + 上边线（除首个）
        renderer.draw_quad(Rect{bounds_.x, it.y0, bounds_.w, kTitleH}, Vec4{0.98f, 0.98f, 0.98f, 1.0f});
        if (i > 0)
            renderer.draw_quad({bounds_.x, it.y0, bounds_.w, 1.f}, t->border);

        const float ty = it.y0 + (kTitleH - font_->line_height()) * 0.5f;
        // 展开箭头（旋转 90 度的小三角）——用 '>' 字符近似
        font_->draw_text(renderer, expanded ? "v" : ">", bounds_.x + 15.f, ty, t->text_secondary);
        font_->draw_text(renderer, it.title.c_str(), bounds_.x + 35.f, ty, t->text);

        if (i > 0 && !expanded)
            renderer.draw_quad({bounds_.x, it.y1, bounds_.w, 1.f}, t->border);
    }
}

}  // namespace ui
