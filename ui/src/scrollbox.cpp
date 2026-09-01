#include "ui/ui/scrollbox.h"

#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

ScrollBox::ScrollBox(float x, float y, float w, float h) : Widget(x, y, w, h) {}

float ScrollBox::content_height() const {
    // 顶部内边距：启用固定头部时由 header_height_（固定、不随内容滚动）取代顶部 padding
    const float top_inset = header_height_ > 0.f ? header_height_ : padding_;
    float h = top_inset + padding_;
    for (const Widget* child : children_) {
        if (!child->visible) continue;
        if (is_pinned(child)) continue;  // 锚定控件不占内容流高度
        h += child->preferred_height() + spacing_;
    }
    if (!children_.empty()) h -= spacing_;  // 去掉末尾多算的间距
    return h;
}

bool ScrollBox::is_pinned(const Widget* w) const {
    for (const Pinned& p : pinned_)
        if (p.w == w) return true;
    return false;
}

void ScrollBox::add_pinned(Widget* w, PinCorner corner, float margin_x, float margin_y) {
    mark_dirty();
        if (!w) return;
    add_child(w);
    pinned_.push_back({w, corner, margin_x, margin_y});
}

float ScrollBox::scroll_max() const {
    // 可视滚动高度刨除固定头部（头部不参与内容滚动）
    const float view_h = bounds_.h - header_height_;
    return std::max(0.f, content_height() - view_h);
}

void ScrollBox::clamp_scroll() {
    scroll_offset_ = std::min(std::max(scroll_offset_, 0.f), scroll_max());
}

void ScrollBox::set_scroll(float value) {
    const float old = scroll_offset_;
    scroll_offset_ = value;
    clamp_scroll();
    // DIRTY 帧级门控：滚动偏移变化必须标脏，否则根保持 clean → 渲染被跳过 → 无法滚动
    //（滚轮 / 拖动滚动条 / set_scroll_offset 全部经此路径）。
    if (scroll_offset_ != old) {
        mark_dirty();
        if (on_scroll_) on_scroll_();
    }
    check_bottom();
}

void ScrollBox::reset_scroll() {
    mark_dirty();
        set_scroll(0.f);
}

void ScrollBox::check_bottom() {
    if (!on_bottom_) return;
    const float max_off = scroll_max();
    // 距底部不足 bottom_margin_ 即触发（镜像 layui flow mb 语义）；
    // bottom_margin_ 为 0 时退化为严格滚到底（UI-035 回归语义）。
    const float trigger = max_off - bottom_margin_;
    const bool at_bottom = scroll_offset_ >= trigger - 1.f;
    if (at_bottom && !bottom_fired_) {
        bottom_fired_ = true;
        on_bottom_();
    } else if (!at_bottom) {
        bottom_fired_ = false;
    }
}

void ScrollBox::on_layout() {
    needs_layout_ = false;
    // UI-127 分帧触底重检：rearm_bottom 只置挂起标记，此处消费 check_bottom。
    // 回调（流加载续灌）重建子控件后，本次 on_layout 后续流程即用新 children
    // 完成排布与 clamp_scroll，无乱帧；每帧至多消费一次 = 每帧至多灌一批，
    // 消除 rebuild_rows 内同步递归级联造成的单帧工作量暴增。
    if (pending_rearm_) {
        pending_rearm_ = false;
        check_bottom();
    }
    clamp_scroll();
    // 内容宽度让出右侧滚动条列
    const float content_w =
        bounds_.w - 2.f * padding_ - scrollbar::kBarW - scrollbar::kMargin;
    // 启用固定头部：内容在头部下方滚动，头部区恒为面板底色（留白固定不随滚动）
    const float top_inset = header_height_ > 0.f ? header_height_ : padding_;
    float cursor_y = bounds_.y + top_inset - scroll_offset_;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        if (is_pinned(child)) continue;  // 锚定控件不走流式排布
        const float h = child->preferred_height();
        child->set_bounds(bounds_.x + padding_, cursor_y, content_w, h);
        cursor_y += h + spacing_;
    }
    // 锚定子控件：剔除已被 clear_children 释放的悬空条目，再钉到可视区角落
    pinned_.erase(std::remove_if(pinned_.begin(), pinned_.end(),
                                 [this](const Pinned& p) {
                                     return std::find(children_.begin(), children_.end(), p.w) ==
                                            children_.end();
                                 }),
                  pinned_.end());
    for (const Pinned& p : pinned_) {
        Widget* w = p.w;
        if (!w->visible) continue;
        const float pw = w->bounds().w;
        const float ph = w->bounds().h > 0.f ? w->bounds().h : w->preferred_height();
        float px, py;
        if (p.corner == PinCorner::TopRight || p.corner == PinCorner::BottomRight)
            px = bounds_.x + bounds_.w - p.mx - pw;
        else
            px = bounds_.x + p.mx;
        if (p.corner == PinCorner::BottomLeft || p.corner == PinCorner::BottomRight)
            py = bounds_.y + bounds_.h - p.my - ph;
        else
            py = bounds_.y + p.my;
        w->set_bounds(px, py, pw, ph);
    }
}

bool ScrollBox::over_bar() const {
    return hovered() && mx_ >= bounds_.w - scrollbar::kMargin - scrollbar::kBarWHover;
}

Rect ScrollBox::track_rect() const {
    return scrollbar::track_rect(bounds_, scrollbar::bar_w(sb_, over_bar()));
}

Rect ScrollBox::thumb_rect() const {
    const float view_h = bounds_.h - header_height_;
    return scrollbar::thumb_rect(track_rect(), content_height(), view_h, scroll_offset_);
}

bool ScrollBox::in_scrollbar(float x, float y) const {
    return scrollbar::hit(track_rect(), x, y);
}

float ScrollBox::scroll_from_y(float y) const {
    return scrollbar::scroll_from_y(track_rect(), y, scroll_max());
}

// 命中裁剪守卫（UI-152）：坐标不在本容器可视区(bounds_)内则跳过子节点分发，
// 与 draw() 的 push_scissor(bounds_) 绘制裁剪对称。滚动越界到可视区上方（如标题栏
// 区域）的子控件绘制被 scissor 裁掉、命中也被此处挡住，避免点击上层透明区落到内容。
bool ScrollBox::pass_hit_to_children(float x, float y) {
    return contains(x, y);
}

bool ScrollBox::on_mouse_scroll(float x, float y, float dx, float dy) {
    (void)x;
    (void)y;
    (void)dx;
    if (scroll_max() <= 0.f) return false;
    set_scroll(scroll_offset_ - dy * scroll_step_);  // dy>0 向上滚
    scrollbar::touch(sb_);  // 滚动活动：滚动条浮现并保持 2s
    return true;
}

bool ScrollBox::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    mx_ = x - bounds_.x;
    my_ = y - bounds_.y;
    if (scroll_max() > 0.f && in_scrollbar(x, y)) {
        scrollbar::touch(sb_);
        const Rect th = thumb_rect();
        const bool on_thumb = x >= th.x && x <= th.x + th.w &&
                              y >= th.y && y <= th.y + th.h;
        if (on_thumb) {
            // 按住滑块：仅进入拖拽，不在按下瞬间 set_scroll（消除按下即跳）
            sb_.dragging = true;
        } else {
            // 点击轨道空白处（滑块上方/下方）：保持跳转到点击位置。
            set_scroll(scroll_from_y(y));
        }
        return true;  // 捕获鼠标，拖动滑块
    }
    // 下拉面板主体点击被消费，避免穿透到面板下方的其他控件（如重叠的 ComboBox）
    if (has_class("layui-form-select-panel")) return true;
    return false;
}

bool ScrollBox::on_mouse_move(float x, float y) {
    mx_ = x - bounds_.x;
    my_ = y - bounds_.y;
    if (!sb_.dragging) return false;
    set_scroll(scroll_from_y(y));
    return true;
}

bool ScrollBox::on_mouse_up(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    sb_.dragging = false;
    return true;
}

void ScrollBox::on_advance(float) {
    // overlay 滚动条活动态迁移（悬停/拖拽/2s 内滚过）时标脏：
    // 帧级门控下若无标脏，2s 后滚动条不会消失（残留上帧像素）。
    const bool show =
        !needs_layout_ && scroll_max() > 0.f && scrollbar::active(sb_, hovered());
    if (show != bar_visible_) mark_dirty();
    bar_visible_ = show;
}

void ScrollBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    if (has_class("layui-form-select-panel")) {
        // layui 下拉面板：白底 + #eee 边框 + 圆角 2 + 阴影
        renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1, 1, 1, 1});
        renderer.draw_rect_outline(bounds_, 1.f, t->border);
        return;
    }
    if (has_class("layui-body")) {
        // layui 内容区（layui-body）：白底
        renderer.draw_quad(bounds_, Vec4{1, 1, 1, 1});
        return;
    }
    if (has_class("ws-menu")) {
        // 新镜像 docs/2 侧栏（ws-menu）：白底（.ws-menu{background-color:#fff}）
        renderer.draw_quad(bounds_, Vec4{1, 1, 1, 1});
        return;
    }
    if (has_class("layui-side-scroll")) {
        // layui 侧栏滚动容器（layui-side-scroll）：深灰底 #2f363c
        renderer.draw_quad(bounds_, Vec4{0.18f, 0.21f, 0.24f, 1.0f});
        return;
    }
    // 默认背景：直角填充（UI-142，面板默认直角化，消除与左侧导航交界处的圆角空隙）。
    // 有特殊视觉效果需求的容器用 layui class（layui-form-select-panel/layui-body/ws-menu
    // /layui-side-scroll）。固定头部（见另 draw 分支）背景与下方内容面板连续。
    if (has_class("layui-round")) {
        renderer.draw_rounded_rect(bounds_, t->radius, t->button_normal);
        return;
    }
    renderer.draw_quad(bounds_, t->button_normal);
}

void ScrollBox::draw(Renderer& renderer) {
    if (!visible) return;
    // UI-157：与基类 Widget::draw() 一致的绘制前兜底布局（仅根触发，见 widget.cpp 说明）——
    // ScrollBox 可能作为独立根被绕过 Scene 裸调，此时先排位再画，杜绝其内容子控件 (0,0) 闪帧。
    if (!parent_ && (dirty_ || dirty_children_)) layout();
    on_draw(renderer);  // 背景
    // 内容裁剪区：启用固定头部时从头部下方开始，头部恒为面板底色（固定留白不滚动）
    const Rect content_clip = {bounds_.x, bounds_.y + header_height_,
                               bounds_.w, bounds_.h - header_height_};
    renderer.push_scissor(content_clip);
    for (Widget* child : children_) {
        if (child->visible) child->draw(renderer);
    }
    // 叠加层：下拉面板等溢出内容最后绘制（置于所有兄弟节点之上）
    for (Widget* child : children_) {
        if (child->visible) child->draw_overlay(renderer);
    }
    renderer.pop_scissor();
    // 滚动条（overlay 风格：悬停/拖拽/2s 内滚过才显示；子树变化后抑制一帧防闪烁）
    if (!needs_layout_ && scroll_max() > 0.f && scrollbar::active(sb_, hovered())) {
        static const Theme kDefaultTheme;
        const Theme* t = theme_ ? theme_ : &kDefaultTheme;
        scrollbar::draw(renderer, t, track_rect(), thumb_rect(), sb_, over_bar());
    }
    // 帧末去标：与基类一致（finish_draw_dirty 保留隐藏脏子节点的 dirty_children_，
    // 见 widget.cpp）。覆写 draw 若不清理 dirty_children_，该容器将永远被视为
    // “有脏子节点”，使 mark_dirty() 的短路优化在沿父链上抛时在此 break，子控件
    // 标脏永远到不了 root，帧级门控将跳过整帧渲染（设置页等 ScrollBox 内容点击/悬停不刷新）。
    finish_draw_dirty();
}

}  // namespace ui
