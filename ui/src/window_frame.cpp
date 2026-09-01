#include "ui/ui/window_frame.h"

#include "ui/ui/titlebar.h"

namespace ui {

WindowFrame::WindowFrame(float w, float h) {
    bounds_.w = w;
    bounds_.h = h;
    // 默认内容区 + 默认头部（系统风格 TitleBar）。二者作为子控件挂到本框架上，
    // 从而随树绘制 / 命中 / 释放。
    content_ = new Widget();
    content_->set_bounds(bounds_.x, bounds_.y, w, h);
    add_child(content_);  // 先把内容区加进来，再叠头部（后加的在上层）

    TitleBar* tb = new TitleBar(w, head_h_, "", nullptr);
    tb->set_bounds(bounds_.x, bounds_.y, w, head_h_);
    head_ = tb;
    head_owned_ = true;
    add_child(head_);
}

void WindowFrame::set_head(Widget* head, float height) {
    if (head_ && head_owned_ && head_ != head) {
        // 释放内部默认标题栏
        Widget* old = head_;
        remove_child(old);
        delete old;
        head_owned_ = false;
    }
    if (head_ && !head_owned_) {
        remove_child(head_);  // 替换外部 head：仅摘除，不释放
    }
    head_ = head;
    head_owned_ = false;
    head_h_ = height;
    if (head_) {
        head_->set_bounds(bounds_.x, bounds_.y, bounds_.w, head_h_);
        add_child(head_);
    }
    mark_dirty();
}

void WindowFrame::set_titlebar_height(float h) {
    head_h_ = h;
    mark_dirty();
}

// 记录/更新 child 在 slots 中的相对矩形快照，并按 (ox, oy) 原点立即应用绝对坐标
void WindowFrame::set_slot(std::vector<Slot>& slots, Widget* child, float x, float y, float w, float h, float ox, float oy) {
    bool found = false;
    for (Slot& s : slots) {
        if (s.child == child) {
            s.rel = Rect{x, y, w, h};
            found = true;
            break;
        }
    }
    if (!found)
        slots.push_back(Slot{child, Rect{x, y, w, h}});
    child->set_bounds(ox + x, oy + y, w, h);
}

void WindowFrame::set_content_child(Widget* child, float x, float y, float w, float h) {
    set_slot(content_slots_, child, x, y, w, h,
             bounds_.x, bounds_.y + head_h_);  // 相对内容区顶部（标题栏下方）
    mark_dirty();
}

void WindowFrame::set_head_child(Widget* child, float x, float y, float w, float h) {
    set_slot(head_slots_, child, x, y, w, h, bounds_.x, bounds_.y);  // 相对头部左上角（框架原点）
    mark_dirty();
}

// 剔除已不在 owner 子节点中/已释放的旧快照，并按 (ox, oy) 原点重基准到绝对坐标
void WindowFrame::apply_slots(std::vector<Slot>& slots, Widget* owner, float ox, float oy) {
    if (!owner) return;
    for (size_t i = 0; i < slots.size();) {
        Widget* c = slots[i].child;
        bool alive = false;
        for (Widget* ch : owner->children())
            if (ch == c) { alive = true; break; }
        if (!alive) {
            slots.erase(slots.begin() + i);
            continue;
        }
        const Rect& r = slots[i].rel;
        c->set_bounds(ox + r.x, oy + r.y, r.w, r.h);
        ++i;
    }
}

void WindowFrame::on_layout() {
    const float x = bounds_.x;
    const float y = bounds_.y;
    const float w = bounds_.w;
    const float h = bounds_.h;
    const float hh = head_h_;

    // 头部：铺满顶部 (x, y, w, hh)，并应用头部相对坐标子项
    if (head_) {
        head_->set_bounds(x, y, w, hh);
        apply_slots(head_slots_, head_, x, y);
        head_->layout();
    }

    // 内容区：标题栏下方铺满剩余高度，并应用内容区相对坐标子项
    if (content_) {
        content_->set_bounds(x, y + hh, w, h - hh);
        apply_slots(content_slots_, content_, x, y + hh);
        content_->layout();
    }
}

// ---- 默认 TitleBar 透传 ----

void WindowFrame::set_font(Font* f) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_font(f);
}
void WindowFrame::set_theme(const Theme* t) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_theme(t);
}
void WindowFrame::set_title(const char* title) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_title(title);
}
void WindowFrame::set_icon(Font* font, uint32_t codepoint) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_icon(font, codepoint);
}
void WindowFrame::set_on_close(std::function<void()> cb) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_on_close(std::move(cb));
}
void WindowFrame::set_on_minimize(std::function<void()> cb) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_on_minimize(std::move(cb));
}
void WindowFrame::set_on_maximize(std::function<void()> cb) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_on_maximize(std::move(cb));
}
void WindowFrame::set_minimize_visible(bool v) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_minimize_visible(v);
}
void WindowFrame::set_maximize_visible(bool v) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_maximize_visible(v);
}
void WindowFrame::set_close_visible(bool v) {
    if (head_owned_)
        static_cast<TitleBar*>(head_)->set_close_visible(v);
}

}  // namespace ui
