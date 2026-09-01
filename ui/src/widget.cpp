#include "ui/ui/widget.h"

#include "ui/gl/font.h"

namespace ui {

namespace {
// 延迟销毁队列（UI-072）：destroy() 标记待销毁，帧末由根节点统一冲刷。
// 事件分发/遍历期间子树保持存活，避免悬空 this 与迭代器失效。
std::vector<Widget*> g_pending_destroy;

// 小缓冲快照：事件分发需在回调可改 children_ 的前提下安全遍历（UI-073），
// 但顶层/容器常用子节点数少，逐事件 new vector 造成每帧堆分配。
// 此处用栈上固定数组承载常见规模（≤32），超限才退化为堆分配，语义与 vector 快照完全一致。
// 返回指向快照首元素；返回 nullptr 表示无子节点（调用方直接跳过）。
Widget** snapshot_children(const std::vector<Widget*>& children, Widget** small, int small_cap,
                           std::vector<Widget*>* heap) {
    const size_t n = children.size();
    if (n == 0) return nullptr;
    if (n <= (size_t)small_cap) {
        for (size_t i = 0; i < n; ++i) small[i] = children[i];
        return small;
    }
    heap->assign(children.begin(), children.end());
    return heap->data();
}
}  // namespace

Widget::Widget(float x, float y, float w, float h)
    : bounds_{x, y, w, h} {}

Widget::~Widget() {
    // 子树由外部所有者管理；基类不负责 delete 子节点
}

void Widget::set_bounds(float x, float y, float w, float h) {
    if (bounds_.x == x && bounds_.y == y && bounds_.w == w && bounds_.h == h) return;
    bounds_ = {x, y, w, h};
    mark_dirty();
}

void Widget::set_visible(bool on) {
    if (visible == on) return;
    visible = on;
    mark_dirty();  // UI-156：重标——显隐变更向上传播，使父容器下一帧重排（见 widget.h 说明）
}

void Widget::set_enabled(bool en) {
    if (enabled == en) return;
    enabled = en;
    if (!en) blur();
    mark_dirty();
}

void Widget::mark_dirty() {
    dirty_ = true;
    Widget* p = parent_;
    while (p) {
        if (p->dirty_children_) break;
        p->dirty_children_ = true;
        p = p->parent_;
    }
}

// UI-136：全树递归置脏。字形逐帧烘焙后 xadvance 由近似变真实，仅根置脏不足以让
// 各 Label 重新换行/布局（子节点自己的 dirty_ 仍是 false，layout 会跳过 on_layout）。
// 由根节点 advance 在烘焙推进期调用，强制下一帧全树重排 + 重绘。
void Widget::mark_dirty_all() {
    mark_dirty();
    for (Widget* c : children_) c->mark_dirty_all();
}

// UI-156：帧末去标（收敛到唯一实现，取代原 clear_dirty 死代码）。self 本轮已绘制；
// 但若存在「隐藏且仍脏」的子节点（draw 跳过不可见子树故其 dirty_ 未清），dirty_children_
// 保持 true——否则父容器清后重标传播中断，隐藏脏子控件再显示时会停在旧/0,0 坐标成一帧闪。
void Widget::finish_draw_dirty() {
    dirty_ = false;
    for (Widget* c : children_) {
        if (!c->visible && (c->dirty_ || c->dirty_children_)) {
            dirty_children_ = true;
            return;
        }
    }
    dirty_children_ = false;
}

void Widget::add_class(const char* cls) {
    if (!cls) return;
    // 空格分隔拆成多个类名；空串/已存在跳过
    const char* p = cls;
    while (*p) {
        while (*p == ' ' || *p == '\t') ++p;
        const char* start = p;
        while (*p && *p != ' ' && *p != '\t') ++p;
        if (p > start) {
            const std::string token(start, (size_t)(p - start));
            bool found = false;
            for (const std::string& c : classes_) {
                if (c == token) {
                    found = true;
                    break;
                }
            }
            if (!found) classes_.push_back(token);
        }
    }
}

void Widget::remove_class(const char* cls) {
    if (!cls) return;
    const std::string target(cls);
    for (auto it = classes_.begin(); it != classes_.end();) {
        if (*it == target)
            it = classes_.erase(it);
        else
            ++it;
    }
}

bool Widget::has_class(const char* cls) const {
    if (!cls) return false;
    const std::string target(cls);
    for (const std::string& c : classes_) {
        if (c == target) return true;
    }
    return false;
}

void Widget::add_child(Widget* child) {
    if (!child || child->parent_ == this) return;
    if (child->parent_) child->parent_->remove_child(child);
    child->parent_ = this;
    children_.push_back(child);
    on_children_changed();
    // UI-154：子树变更即标脏本容器——layout() 的帧级门控（!dirty_ && !dirty_children_
    // 提前返回）下，若不标脏，长命容器（如 ScrollBox 内容面板）被 clear_children()
    // + 重建后本帧不重排、新子控件保持构造默认 (0,0,0,0) bounds 在窗口左上角闪一帧。
    // 标脏使下一 root.layout() 必重排，新子控件同帧归位，消除 0,0 闪帧。
    mark_dirty();
}

void Widget::remove_child(Widget* child) {
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (*it == child) {
            (*it)->parent_ = nullptr;
            children_.erase(it);
            on_children_changed();
            mark_dirty();
            return;
        }
    }
}

void Widget::clear_children() {
    std::vector<Widget*> kids = std::move(children_);
    children_.clear();
    for (Widget* k : kids) {
        k->parent_ = nullptr;
        k->clear_children();  // 递归释放 k 的整个子树
        delete k;
    }
    on_children_changed();
    mark_dirty();
}

void Widget::destroy() {
    // UI-072：延迟销毁。仅标记 + 立即隐藏，实际释放延迟到帧末 flush_pending_destroys()。
    // 避免在 dispatch_mouse_down / advance / draw 遍历期间销毁自身/兄弟导致的悬空 this 与迭代器失效。
    for (Widget* w : g_pending_destroy) {
        if (w == this) return;  // 幂等：已在待销毁队列，忽略重复调用
    }
    set_visible(false);  // UI-156：显隐变更经 set_visible（内含 mark_dirty）
    g_pending_destroy.push_back(this);
}
void Widget::flush_pending_destroys() {
    if (g_pending_destroy.empty()) return;
    std::vector<Widget*> pending;
    pending.swap(g_pending_destroy);
    for (Widget* w : pending) {
        // 摘除根节点 popups_ 中的自身（UI-071 语义）与模态/捕获引用，防止下一帧遍历到悬空指针
        Widget* r = w->root();
        if (r) {
            if (r->modal_ == w) r->modal_ = nullptr;
            if (r->capture_ == w) r->capture_ = nullptr;
            if (!r->popups_.empty()) {
                std::vector<Popup> kept;
                kept.reserve(r->popups_.size());
                for (const Popup& p : r->popups_) {
                    if (p.popup != w) kept.push_back(p);
                }
                if (kept.size() != r->popups_.size()) r->popups_.swap(kept);
            }
        }
        if (w->parent_) w->parent_->remove_child(w);
        w->clear_children();  // 递归释放 w 的整个子树
        delete w;
    }
}

void Widget::bring_to_front() {
    if (!parent_) return;
    for (auto it = parent_->children_.begin(); it != parent_->children_.end(); ++it) {
        if (*it == this) {
            parent_->children_.erase(it);
            parent_->children_.push_back(this);
            return;
        }
    }
}

bool Widget::contains(float x, float y) const {
    return x >= bounds_.x && x < bounds_.x + bounds_.w &&
           y >= bounds_.y && y < bounds_.y + bounds_.h;
}

bool Widget::on_mouse_move(float x, float y) {
    (void)x;
    (void)y;
    return false;
}

bool Widget::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    (void)button;
    return false;
}

bool Widget::on_mouse_scroll(float x, float y, float dx, float dy) {
    (void)x;
    (void)y;
    (void)dx;
    (void)dy;
    return false;
}

bool Widget::on_drop(float x, float y, const std::vector<std::string>& paths) {
    (void)x;
    (void)y;
    (void)paths;
    return false;
}

bool Widget::on_mouse_up(float x, float y, int button) {
    (void)x;
    (void)y;
    (void)button;
    return false;
}

void Widget::capture_mouse() {
    root()->capture_ = this;
}

void Widget::release_mouse_capture() {
    if (root()->capture_ == this) root()->capture_ = nullptr;
}

bool Widget::dispatch_mouse_move(float x, float y) {
    // 模态：事件仅路由给模态子树，模态外一律屏蔽，且清除模态外残留 hover 态
    if (!parent_ && modal_ && modal_->visible) {
        clear_hover_outside(modal_);
        return modal_->dispatch_mouse_move(x, y);
    }
    // 鼠标捕获优先：移动事件直接给捕获者（即使指针已移出其范围）
    Widget* cap = root()->capture_;
    if (cap && cap != this) {
        cap->hovered_ = true;
        cap->mark_dirty();  // 捕获命中即重绘（帧级门控需要显式标脏）
        return cap->on_mouse_move(x, y);
    }
    // 子节点在父之上，后添加的优先命中（禁用/隐藏的子节点不参与）
    // 快照遍历：子控件回调内可能 close_popups/remove_child 修改 children_，原地迭代会失效（UI-073）。
    // 常见规模 ≤32 走栈上小缓冲，避免每帧堆分配（性能优化）；超限退化为堆快照，语义不变。
    if (pass_hit_to_children(x, y)) {
        const int n = (int)children_.size();
        Widget* small[32];
        std::vector<Widget*> heap;
        Widget** kids = snapshot_children(children_, small, 32, &heap);
        if (kids) {
            for (int i = n - 1; i >= 0; --i) {
                Widget* child = kids[i];
                if (!child->visible || !child->enabled) continue;
                if (child->dispatch_mouse_move(x, y)) return true;
            }
        }
    }
    if (contains(x, y) && enabled) {
        const bool was_hovered = hovered_;
        hovered_ = true;
        if (!was_hovered) mark_dirty();  // hover 状态变更 → 触发帧级重绘
        if (on_mouse_move(x, y)) return true;
        return false;
    }
    if (hovered_) {
        hovered_ = false;
        mark_dirty();  // 移出 → 清除 hover 要高亮重绘
        on_mouse_move(x, y);
    }
    return false;
}

bool Widget::dispatch_mouse_down(float x, float y, int button) {
    // 模态：事件仅路由给模态子树，模态外一律屏蔽
    if (!parent_ && modal_ && modal_->visible) return modal_->dispatch_mouse_down(x, y, button);
    prune_popups(x, y);
    // 快照遍历：子控件回调内可能 close_popups/remove_child 修改 children_，原地迭代会失效（UI-073）
    if (pass_hit_to_children(x, y)) {
        const int n = (int)children_.size();
        Widget* small[32];
        std::vector<Widget*> heap;
        Widget** kids = snapshot_children(children_, small, 32, &heap);
        if (kids) {
            for (int i = n - 1; i >= 0; --i) {
                Widget* child = kids[i];
                if (!child->visible || !child->enabled) continue;
                if (child->dispatch_mouse_down(x, y, button)) return true;
            }
        }
    }
    if (contains(x, y) && enabled) {
        if (!pressed_) mark_dirty();  // 按下态变更 → 帧级重绘
        pressed_ = true;
        // 命中即获焦（仅 focusable 生效），子类无需自行 focus()
        if (focusable) focus();
        if (on_mouse_down(x, y, button)) {
            capture_mouse();  // 消费按下即捕获（拖拽控件）
            return true;
        }
        return false;
    }
    return false;
}

bool Widget::dispatch_mouse_up(float x, float y, int button) {
    // 模态：事件仅路由给模态子树，模态外一律屏蔽
    if (!parent_ && modal_ && modal_->visible) return modal_->dispatch_mouse_up(x, y, button);
    // 捕获者优先：鼠标抬起路由给捕获者并释放捕获
    Widget* cap = root()->capture_;
    if (cap && cap != this) {
        if (cap->pressed_) cap->mark_dirty();  // 抬起 → 释放按下态高亮
        cap->pressed_ = false;
        const bool handled = cap->on_mouse_up(x, y, button);
        cap->release_mouse_capture();
        return handled;
    }
    // 快照遍历：子控件回调内可能 close_popups/remove_child 修改 children_，原地迭代会失效（UI-073）
    if (pass_hit_to_children(x, y)) {
        const int n = (int)children_.size();
        Widget* small[32];
        std::vector<Widget*> heap;
        Widget** kids = snapshot_children(children_, small, 32, &heap);
        if (kids) {
            for (int i = n - 1; i >= 0; --i) {
                Widget* child = kids[i];
                if (!child->visible || !child->enabled) continue;
                if (child->dispatch_mouse_up(x, y, button)) return true;
            }
        }
    }
    if (contains(x, y)) {
        const bool was_pressed = pressed_;
        pressed_ = false;
        if (was_pressed) {
            if (on_mouse_up(x, y, button)) return true;
        }
        return false;
    }
    if (pressed_) {
        pressed_ = false;
        mark_dirty();  // 拖出释放 → 清除按下态高亮
    }
    return false;
}

bool Widget::dispatch_mouse_scroll(float x, float y, float dx, float dy) {
    // 模态：事件仅路由给模态子树，模态外一律屏蔽
    if (!parent_ && modal_ && modal_->visible) return modal_->dispatch_mouse_scroll(x, y, dx, dy);
    // 快照遍历：子控件回调内可能 close_popups/remove_child 修改 children_，原地迭代会失效（UI-073）
    if (pass_hit_to_children(x, y)) {
        const int n = (int)children_.size();
        Widget* small[32];
        std::vector<Widget*> heap;
        Widget** kids = snapshot_children(children_, small, 32, &heap);
        if (kids) {
            for (int i = n - 1; i >= 0; --i) {
                Widget* child = kids[i];
                if (!child->visible || !child->enabled) continue;
                if (child->dispatch_mouse_scroll(x, y, dx, dy)) return true;
            }
        }
    }
    if (contains(x, y)) {
        if (on_mouse_scroll(x, y, dx, dy)) return true;
        return false;
    }
    return false;
}

bool Widget::dispatch_drop(float x, float y, const std::vector<std::string>& paths) {
    if (!parent_ && modal_ && modal_->visible)
        return modal_->dispatch_drop(x, y, paths);
    if (pass_hit_to_children(x, y)) {
        const int n = (int)children_.size();
        Widget* small[32];
        std::vector<Widget*> heap;
        Widget** kids = snapshot_children(children_, small, 32, &heap);
        if (kids) {
            for (int i = n - 1; i >= 0; --i) {
                Widget* child = kids[i];
                if (!child->visible || !child->enabled) continue;
                if (child->dispatch_drop(x, y, paths)) return true;
            }
        }
    }
    if (contains(x, y)) {
        if (on_drop(x, y, paths)) return true;
    }
    return false;
}

void Widget::on_layout() {}

void Widget::layout() {
    if (!dirty_ && !dirty_children_) return;
    on_layout();
    for (Widget* child : children_) {
        if (child->visible) child->layout();
    }
}

Widget* Widget::root() {
    Widget* w = this;
    while (w->parent_) w = w->parent_;
    return w;
}

void Widget::set_focused(bool focused) {
    if (focused_ == focused) return;
    focused_ = focused;
    mark_dirty();  // 焦点态视觉（边框/光标）随帧级门控重绘
    on_focus_changed(focused);
}

void Widget::focus() {
    if (!focusable || !enabled) return;
    // 先从根清掉现有焦点（保证整棵树唯一焦点），再聚焦自身
    Widget* r = root();
    if (r != this) r->clear_focus_recursive();
    set_focused(true);
}

void Widget::blur() {
    if (!focused_) return;
    set_focused(false);
}

void Widget::clear_focus_recursive() {
    if (focused_) {
        set_focused(false);
        return;
    }
    for (Widget* child : children_) child->clear_focus_recursive();
}

void Widget::on_focus_changed(bool focused) {
    (void)focused;
}

bool Widget::on_key_down(int key, int mods) {
    (void)key;
    (void)mods;
    return false;
}

bool Widget::on_key_up(int key, int mods) {
    (void)key;
    (void)mods;
    return false;
}

bool Widget::on_char(unsigned int codepoint) {
    (void)codepoint;
    return false;
}

// UI-133：模态拦截仅在 modal 控件可见时生效。所有 dispatch_* 入口统一检查
// modal_->visible——控件被 destroy()（UI-072 仅隐藏待帧末释放）或手动隐藏后，
// 若仍持续拦截，宿主主循环未接 advance()/flush_pending_destroys() 时僵尸模态
// 将永久吞掉全部输入（audio_text CONF-002 全 UI 死锁根因）。隐藏即解除模态。
void Widget::set_modal(Widget* modal) {
    modal_ = modal;
    mark_dirty();
}

bool Widget::dispatch_key_down(int key, int mods) {
    // 模态：键盘仅路由给模态子树
    if (!parent_ && modal_ && modal_->visible) return modal_->dispatch_key_down(key, mods);
    // 优先让焦点控件消费（如 TextBox 光标移动/删除）；导航键未被消费才做焦点遍历
    Widget* f = focused_descendant();
    if (f) {
        if (f->on_key_down(key, mods)) return true;
        if (key == KeyTab || key == KeyUp || key == KeyDown || key == KeyLeft || key == KeyRight) {
            const bool reverse = (mods & ModShift) != 0 || key == KeyUp || key == KeyLeft;
            if (reverse)
                f->focus_prev();
            else
                f->focus_next();
            return true;
        }
        return false;
    }
    // 无焦点：Tab 落到第一个可聚焦控件
    if (key == KeyTab) {
        focus_next();
        return true;
    }
    return false;
}

bool Widget::dispatch_key_up(int key, int mods) {
    // 模态：键盘仅路由给模态子树
    if (!parent_ && modal_ && modal_->visible) return modal_->dispatch_key_up(key, mods);
    Widget* f = focused_descendant();
    if (f) return f->on_key_up(key, mods);
    return false;
}

bool Widget::dispatch_char(unsigned int codepoint) {
    // 模态：键盘仅路由给模态子树
    if (!parent_ && modal_ && modal_->visible) return modal_->dispatch_char(codepoint);
    Widget* f = focused_descendant();
    if (f) return f->on_char(codepoint);
    return false;
}

Widget* Widget::focused_descendant() {
    if (focused_) return this;
    for (Widget* child : children_) {
        Widget* f = child->focused_descendant();
        if (f) return f;
    }
    return nullptr;
}

void Widget::focus_next() {
    // 以根为起点先序收集可聚焦控件，找当前焦点位置的下一个
    Widget* r = root();
    std::vector<Widget*> list;
    r->collect_focusables(list);
    if (list.empty()) return;
    size_t idx = 0;
    bool found = false;
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i]->focused_) {
            idx = i;
            found = true;
            break;
        }
    }
    if (!found) {  // 无焦点：落到第一个
        list[0]->focus();
        return;
    }
    const size_t next = (idx + 1) % list.size();
    list[idx]->blur();
    list[next]->focus();
}

void Widget::focus_prev() {
    Widget* r = root();
    std::vector<Widget*> list;
    r->collect_focusables(list);
    if (list.empty()) return;
    size_t idx = 0;
    bool found = false;
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i]->focused_) {
            idx = i;
            found = true;
            break;
        }
    }
    if (!found) {  // 无焦点：落到最后一个
        list[list.size() - 1]->focus();
        return;
    }
    const size_t prev = (idx + list.size() - 1) % list.size();
    list[idx]->blur();
    list[prev]->focus();
}

void Widget::collect_focusables(std::vector<Widget*>& out) {
    if (focusable && visible && enabled) out.push_back(this);
    for (Widget* child : children_) child->collect_focusables(out);
}

void Widget::on_draw(Renderer&) {}

void Widget::on_advance(float) {}

void Widget::advance(float dt) {
    on_advance(dt);
    for (Widget* child : children_) {
        if (child->visible) child->advance(dt);
    }
    if (!parent_ && !popups_.empty()) {
        // 拷贝遍历：popup 的 on_advance（如自动关闭）可能 close_popups/销毁自身，原地迭代会失效（UI-071）
        const std::vector<Popup> snap = popups_;
        for (const Popup& p : snap) {
            if (p.popup->visible) p.popup->advance(dt);
        }
    }
    // 根节点帧末冲刷延迟销毁队列（UI-072）：事件分发/遍历期间被 destroy() 标记的控件在此统一释放
    if (!parent_) {
        // UI-136：CJK 字形逐帧烘焙推进。帧级门控宿主（is_dirty() 决定整帧渲染或跳过）中，
        // 首次渲染只烘 ≤12 个新码点，根 clean 后若停帧则其余字形永远停留在「空格占位」。
        // advance 在门控之前每帧无条件执行，此处据两个信号决定是否续帧：
        //   1) 仍有待烘焙字形（配额未烘）→ 继续渲染下一批；
        //   2) 烘焙 epoch 变化（有新字形上屏，xadvance 已更新）→ 全树重排。
        // 两者皆静即恢复空闲停帧，CPU 回落到门控下水平。
        static unsigned last_epoch = 0;
        const unsigned now = Font::bake_epoch_sum();
        if (Font::any_pending_bake() || now != last_epoch) mark_dirty_all();
        last_epoch = now;

        flush_pending_destroys();
    }
}

// 注意：draw() 不再按 dirty 跳过子树——begin_frame 每帧全屏清屏，
// 只重绘脏控件会擦掉干净控件的像素（DIRTY 回归根因）。脏标记只作
// 帧级门控用（root.is_dirty() 决定整帧渲染或跳过），渲染帧内必须全树绘制。
void Widget::draw(Renderer& renderer) {
    if (!visible) return;
    // UI-157：绘制前兜底布局——若自身或子树仍脏（事件阶段新建/变更后未被 Scene::frame()
    // 的补布局覆盖，比如绕过 Scene 裸调 Widget::draw 的第三方），先排位再画，杜绝 (0,0) 闪帧。
    // layout() 自带 `dirty_ || dirty_children_` 门控：无变更即时返回（零开销），且 layout 不
    // 调 draw（无递归）。Scene::frame() 已先布局，此处对标准路径是 no-op（双保险）。
    if (dirty_ || dirty_children_) layout();
    on_draw(renderer);
    for (Widget* child : children_) {
        if (child->visible) child->draw(renderer);
    }
    if (!parent_ && !popups_.empty()) {
        for (const Popup& p : popups_) {
            if (p.popup->visible) p.popup->draw(renderer);
        }
    }
    finish_draw_dirty();  // UI-156：收敛去标（保留隐藏脏子节点的 dirty_children_）
}

void Widget::open_popup(Widget* owner, Widget* popup) {
    if (!popup) return;
    Widget* r = root();
    for (const Popup& p : r->popups_) {
        if (p.popup == popup) return;  // 已在打开列表
    }
    r->popups_.push_back({owner, popup});
    popup->set_visible(true);  // UI-156：经 set_visible（含 mark_dirty）
    mark_dirty();
}

void Widget::close_popups() {
    Widget* r = root();
    for (const Popup& p : r->popups_) {
        p.popup->set_visible(false);  // UI-156
        // 弹出面板通常挂在根节点下（事件路由需要），关闭时从根分离，
        // 避免切页/销毁控件后根节点残留悬空子指针（UI-033 生命周期修复）
        if (p.popup->parent()) p.popup->parent()->remove_child(p.popup);
    }
    r->popups_.clear();
    mark_dirty();
}

bool Widget::has_open_popups() {
    return root()->popups_.size() > 0;
}

void Widget::close_topmost_popup() {
    Widget* r = root();
    if (r->popups_.empty()) return;
    const Popup& p = r->popups_.back();
    p.popup->set_visible(false);  // UI-156
    if (p.popup->parent()) p.popup->parent()->remove_child(p.popup);
    r->popups_.pop_back();
}

void Widget::prune_popups(float x, float y) {
    Widget* r = root();
    for (auto it = r->popups_.begin(); it != r->popups_.end();) {
        const bool inside_popup = it->popup->contains(x, y);
        const bool inside_owner = it->owner && it->owner->contains(x, y);
        if (!inside_popup && !inside_owner) {
            it->popup->set_visible(false);  // UI-156
            it = r->popups_.erase(it);
        } else {
            ++it;
        }
    }
}

namespace {
// keep 是否在某子树的祖先链上（含自身）
bool in_subtree(Widget* sub, Widget* node) {
    while (node) {
        if (node == sub) return true;
        node = node->parent();
    }
    return false;
}
}  // namespace

// 清除所有不在 keep 子树内的控件 hover 态（模态打开时残留高亮清理）
void Widget::clear_hover_outside(Widget* keep) {
    if (!in_subtree(keep, this)) {
        if (hovered_) {
            hovered_ = false;
            on_mouse_move(0.f, 0.f);  // 通知控件退出 hover（如按钮还原高亮）
        }
        return;
    }
    for (Widget* child : children_) child->clear_hover_outside(keep);
}

}  // namespace ui
