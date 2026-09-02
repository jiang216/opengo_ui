#pragma once

#include "ui/core/keycodes.h"
#include "ui/core/math.h"

#include <string>
#include <vector>

namespace ui {
class Renderer;

// UI 控件基类：对象树节点 + 事件分发 + 布局 + 绘制遍历 + 焦点（M3）
// 坐标系统：绝对像素坐标，y 向下，(0,0) 为窗口左上角
class Widget {
public:
    explicit Widget(float x = 0, float y = 0, float w = 0, float h = 0);
    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    void set_bounds(float x, float y, float w, float h);
    // 像素坐标，y 向下
    const Rect& bounds() const { return bounds_; }

    // 显隐态：直接写可见性会破坏脏标记「重标」闭环——端帧 draw 已把本容器
    // dirty_children_ 清 false，隐藏脏子控件再被显示时父容器会跳过 layout()，
    // 新现身子控件停在旧/0,0 坐标成一帧闪（UI-156）。一律经 set_visible() 变更，
    // 变化时 mark_dirty() 向上传播，保证下一帧重排。
    bool visible = true;
    virtual void set_visible(bool on);
    // 禁用态（M22）：禁用控件不接收鼠标/键盘/焦点，Tab 遍历跳过，绘制层灰化显示
    bool enabled = true;
    virtual void set_enabled(bool en);

    // ---- 脏标记机制：空闲时跳过无变子树绘制，降低 CPU/GPU 占用 ----
    bool is_dirty() const { return dirty_ || dirty_children_; }
    void mark_dirty();
    // 帧末清脏（去标）：self 本轮已绘制，清 dirty_ 与 dirty_children_（UI-175 全清）。
    // 隐藏且仍脏的子控件不画不清，但不再驻留本容器 dirty_children_——mark_dirty() 已
    // 无条件上溯 root，隐藏子重显经 set_visible(true) 传播 mark_dirty 触发父容器重排，
    // 无 (0,0) 闪帧（UI-156 语义保持，见 widget.cpp finish_draw_dirty 注释）。
    void finish_draw_dirty();
    // 全树递归置脏（UI-136）：字形逐帧烘焙导致 xadvance 变化后，由根节点 advance
    // 调用，强制所有 Label 以真实宽度重新换行/布局（近似宽度 → 真实宽度）
    void mark_dirty_all();

    // ---- CSS 类名机制（M17a）：layui 语法入口，供样式层查询 ----
    // 支持空格分隔的多个类名（如 "layui-btn layui-btn-danger"）
    void add_class(const char* cls);
    void remove_class(const char* cls);
    bool has_class(const char* cls) const;
    const std::vector<std::string>& classes() const { return classes_; }

    // ---- 树结构 ----
    void add_child(Widget* child);
    void remove_child(Widget* child);
    // 递归释放全部子树：逐个 remove_child + delete（含深层子节点），调用后 children() 为空。
    // 调用者持有子树所有权时使用（切页释放/列表重建）；不释放调用者自身。
    // 注意：任何指向已删除子控件的指针立即失效（UI-039 回归保证）。
    void clear_children();
    // 销毁自身（含整个子树）：从父节点摘除后递归释放子树并 delete 自身。
    // 等价于「父节点 remove_child + 子树 clear_children + delete 自身」一条龙，
    // 用于销毁单个控件及其子树（切页清理根级弹窗等）。
    // 注意：调用后本控件及子树内任何指针立即失效，不得再访问（UI-040 回归保证）；
    // 无父节点时直接销毁子树与自身。
    //
    // 延迟销毁（UI-072）：destroy() 只把自身标记为待销毁（立即 visible=false），
    // 实际的「摘除 popups/父引用 + 释放子树 + delete」统一延迟到根节点帧末
    // flush_pending_destroys() 执行。这样在事件分发（dispatch_mouse_*）或遍历
    // （advance/draw/children_ 迭代）期间销毁自身不会产生悬空 this 与失效迭代器
    // （如 footer 回调 cleanup_windows 在 dispatch_mouse_down 内自毁窗口）。
    //
    // 宿主契约：延迟销毁只在 flush_pending_destroys() 被调用时生效——它由根节点
    // advance()（Scene::frame() 每帧自动走）及 Scene::reset()/shutdown() 统一触发。
    // 若自定义渲染循环绕过 Scene 且不调 root_.advance()，则 destroy() 永不冲刷 →
    // 内存泄漏 + 隐藏控件仍可能被模态引用造成「僵尸模态」永久拦截输入。此类宿主
    // 必须每帧调用 advance()（或 Scene::frame），或主动调用静态 flush_pending_destroys()。
    void destroy();
    // 帧末统一销毁被 destroy() 标记的控件（由根节点 advance 自动调用；自定义主循环
    // 宿主可主动显式调用冲刷）。全局单一队列，多 Scene 共存时共享（UI-146 已收敛单 Scene）。
    static void flush_pending_destroys();
    Widget* parent() const { return parent_; }
    const std::vector<Widget*>& children() const { return children_; }

    // 置顶：把自身移到父节点子控件列表末尾（后添加者在绘制/命中上层），无父节点时无操作
    void bring_to_front();

    // 屏幕坐标命中测试
    bool contains(float x, float y) const;

    // ---- 事件（屏幕坐标，y 向下）----
    // 返回 true 表示事件被消费（阻止冒泡到兄弟/父节点）
    virtual bool on_mouse_move(float x, float y);
    virtual bool on_mouse_down(float x, float y, int button);
    virtual bool on_mouse_up(float x, float y, int button);
    virtual bool on_mouse_scroll(float x, float y, float dx, float dy);
    // 文件拖放：paths 为拖入的文件/文件夹路径列表
    virtual bool on_drop(float x, float y, const std::vector<std::string>& paths);

    // 鼠标捕获（M5 拖拽）：on_mouse_down 返回 true 即自动捕获（如 Slider 拖动），
    // 此后鼠标 move/up 即使指针离开控件范围也路由给捕获者；up 后自动释放
    void capture_mouse();
    void release_mouse_capture();

    // ---- 弹出层（M9）：根节点维护弹出面板（下拉/菜单），点击外部自动关闭 ----
    // 打开弹出面板（owner 内点击不关闭它；面板绘制在控件树最上层）
    void open_popup(Widget* owner, Widget* popup);
    // 关闭所有弹出面板
    void close_popups();
    // 关闭最上层弹出面板（layer.escIndex 语义：Esc 关闭当前最上层窗口）
    void close_topmost_popup();
    bool has_open_popups();

    // ---- 模态（M14）：根节点设置后，事件仅路由给模态子树，其余控件被屏蔽 ----
    // 用完后传 nullptr 解除模态
    void set_modal(Widget* modal);
    Widget* modal() const { return modal_; }

    // ---- Tooltip（M14）：悬停提示文本（由应用层在鼠标位置绘制）----
    void set_tooltip(const char* text) { tooltip_ = text ? text : ""; }
    const std::string& tooltip() const { return tooltip_; }

    // 事件分发入口：从最上层子节点开始命中（后添加的子节点在上层）
    bool dispatch_mouse_move(float x, float y);
    bool dispatch_mouse_down(float x, float y, int button);
    bool dispatch_mouse_up(float x, float y, int button);
    bool dispatch_mouse_scroll(float x, float y, float dx, float dy);
    bool dispatch_drop(float x, float y, const std::vector<std::string>& paths);

    bool hovered() const { return hovered_; }
    bool pressed() const { return pressed_; }

    // ---- 焦点 / 键盘（M3 + M4 字符输入）----
    bool focusable = false;
    // 获取/失去焦点回调（子类用于高亮）
    virtual void on_focus_changed(bool focused);
    // 键盘事件（key 用 ui::Key 常量，mods 用 ui::KeyMod）
    virtual bool on_key_down(int key, int mods);
    virtual bool on_key_up(int key, int mods);
    // 文本输入事件（Unicode 码点；仅分发给当前焦点控件）
    virtual bool on_char(unsigned int codepoint);
    // 请求成为当前焦点（仅 focusable 生效）；blur 释放焦点
    void focus();
    void blur();
    bool focused() const { return focused_; }
    // 键盘事件分发：先让焦点控件处理；导航键（Tab/方向键）未被消费才做焦点遍历
    bool dispatch_key_down(int key, int mods);
    bool dispatch_key_up(int key, int mods);
    // 字符分发：转发给当前焦点控件
    bool dispatch_char(unsigned int codepoint);
    // Tab 顺序遍历可聚焦控件（供 dispatch_key_down 内部及外部调用）
    void focus_next();
    void focus_prev();
    // 树的根节点（向上追溯）
    Widget* root();

    // ---- 布局 ----
    // 布局容器覆写：计算子树位置/尺寸。由 draw 前调用者（root）触发一次
    virtual void on_layout();
    // 递归：先布局自身，再遍历子树
    void layout();
    // 内容自适应高度（像素）：布局容器调用；默认返回 bounds_.h
    virtual float preferred_height() const { return bounds_.h; }

    // ---- 逐帧推进（UI-069）：应用层每帧调用 root->advance(dt)，供自动关闭/动画计时 ----
    virtual void on_advance(float dt);
    void advance(float dt);

    // ---- 绘制 ----
    // 按可见性遍历子树；父节点先画背景，子节点叠在其上
    virtual void on_draw(Renderer& renderer);
    virtual void draw(Renderer& renderer);
    // 叠加层绘制（在所有兄弟节点之后调用，用于下拉面板等溢出内容）
    virtual void draw_overlay(Renderer& renderer) { (void)renderer; }

protected:
    // 命中裁剪守卫（UI-152）：返回 false 时跳过本控件的子节点事件分发。
    // 与绘制端 scissor 对称——ScrollBox 绘制用 push_scissor(bounds_) 裁剪可视区，
    // 但基类 dispatch 命中无裁剪：滚动越界到可视区（如标题栏区域）的子控件仍可被
    // contains 命中，造成「点击上层透明区却落到越界内容」穿透。默认放行不影响其它控件。
    virtual bool pass_hit_to_children(float x, float y) { (void)x; (void)y; return true; }

    // 焦点请求内部的设置/清除：通知旧焦点 on_focus_changed(false)、新焦点 on_focus_changed(true)
    virtual void set_focused(bool focused);
    // 子节点变化回调：add_child/remove_child/clear_children 后调用，供子类标记布局脏
    virtual void on_children_changed() {}

    Rect bounds_;
    Widget* parent_ = nullptr;
    bool dirty_ = true;
    bool dirty_children_ = true;
    std::vector<Widget*> children_;
    bool hovered_ = false;
    bool pressed_ = false;
    bool focused_ = false;
    // 鼠标捕获者（仅根节点持有，指向当前捕获的控件）
    Widget* capture_ = nullptr;

private:
    // 子树中当前持有焦点的控件（自身聚焦则返回自身；无则 nullptr）
    Widget* focused_descendant();
    // 清除子树内所有焦点（focus() 调用前）
    void clear_focus_recursive();
    // 先序收集可聚焦可见控件（供 Tab 遍历）
    void collect_focusables(std::vector<Widget*>& out);
    // 关闭未命中 x,y 的弹出面板（owner 内点击不关闭）
    void prune_popups(float x, float y);
    // 清除所有不在 keep 子树内的控件 hover 态（模态屏蔽时残留高亮清理）
    void clear_hover_outside(Widget* keep);

    struct Popup {
        Widget* owner;
        Widget* popup;
    };
    std::vector<Popup> popups_;
    Widget* modal_ = nullptr;  // 根节点持有：模态控件（事件仅路由给它）
    std::string tooltip_;      // 悬停提示文本
    std::vector<std::string> classes_;  // CSS 类名列表
};

}  // namespace ui
