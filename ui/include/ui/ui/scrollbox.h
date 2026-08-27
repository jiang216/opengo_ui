#pragma once

#include "ui/ui/scrollbar_style.h"
#include "ui/ui/widget.h"

#include <functional>

namespace ui {
class Renderer;
struct Theme;

// 视口锚定角（layui fixbar：钉在滚动容器可视区的哪个角）
enum class PinCorner { TopLeft, TopRight, BottomLeft, BottomRight };

// 纵向滚动容器（M8）：子控件按顺序纵向排列，超出可视区用 scissor 裁剪，
// 滚轮 / 拖动右侧滚动条滚动。内容自适应宽度，高度由外部指定。
class ScrollBox : public Widget {
public:
    ScrollBox(float x, float y, float w, float h);

    // 子控件之间的间距（像素）
    void set_spacing(float spacing) {  spacing_ = spacing; mark_dirty(); }
    // 内部边距（像素）
    void set_padding(float padding) {  padding_ = padding; mark_dirty(); }
    float padding() const { return padding_; }
    // 滚动步长（每滚一格滚轮）
    void set_scroll_step(float step) {  scroll_step_ = step; mark_dirty(); }
    float scroll_offset() const { return scroll_offset_; }
    // 滚动到指定偏移（自动 clamp 到 [0, scroll_max]；脚本化截图/程序化定位用）
    void set_scroll_offset(float offset) {  set_scroll(offset); mark_dirty(); }
    // 滚动回顶部（切页/内容重建后复位滚动位置，防止残留旧页滚动偏移）
    void reset_scroll();
    // 标记子树变化（clear_children/add_child 后调用），抑制滚动条绘制直到下一次 on_layout()
    void mark_content_dirty() { needs_layout_ = true; }

    // 滚到内容底部时回调一次（再次离开底部再滚回底部会重新触发）。
    // 传入 nullptr 清除回调（切页清理用，防悬空引用）。
    void set_on_scroll_to_bottom(std::function<void()> cb) { on_bottom_ = std::move(cb); }

    // 内容增长后由宿主调用：解除触底一次性武装并安排重检（UI-126 引入，UI-127 改分帧）。
    // 场景：流加载续灌批次未把内容底推出触发区（用户已在底部无法再滚），不清
    // bottom_fired_ 将永不再触发——补齐 layui flow.load「DOM 插入后再查一次 reachBottom」
    // 语义。UI-127 起不再同步重检（同步链会在 rebuild_rows 内递归级联多批、单帧工作量
    // 暴增），改为置挂起标记、下一次 on_layout() 开头消费 check_bottom——每帧至多灌一批，
    // 且回调重建的行控件在本次布局内即归位，无乱帧。
    void rearm_bottom() { pending_rearm_ = true; }

    // 通用滚动回调：每次 scroll_offset 变化时触发（用于关闭下拉面板等场景）。
    void set_on_scroll(std::function<void()> cb) { on_scroll_ = std::move(cb); }

    // 距底部触发距离 margin（镜像 layui flow mb，默认 50）：当 scroll_offset 距内容底部
    // 不足 mb 像素时即触发触底回调，而不必严格滚到底（UI-094）。仅影响 check_bottom，
    // 触发/重新武装语义（bottom_fired_）与 set_on_scroll_to_bottom 一致。
    void set_bottom_margin(float mb) {  bottom_margin_ = mb; mark_dirty(); }

    // —— 视口级锚定子控件（layui fixbar，UI-110）——
    // 控件照常 add_child（生命周期/命中/绘制顺序不变），但不参与纵向流式排布、
    // 不计入 content_height；每次布局钉在可视区 corner 内缩 (margin_x, margin_y) 处，
    // 不随 scroll_offset 移动。后加的锚定控件绘制在流式内容之上。
    void add_pinned(Widget* w, PinCorner corner = PinCorner::BottomRight,
                    float margin_x = 16.f, float margin_y = 16.f);
    int pinned_count() const { return (int)pinned_.size(); }

    void on_layout() override;
    void on_draw(Renderer& renderer) override;
    // 覆写 draw：背景 -> scissor(内容) -> 滚动条
    void draw(Renderer& renderer) override;
    // 子节点变化后抑制滚动条绘制，直到下一次 on_layout()
    void on_children_changed() override { needs_layout_ = true; }

    bool on_mouse_scroll(float x, float y, float dx, float dy) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;

    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }

private:
    // 内容高度（所有子控件 + 间距 + 边距）；溢出量 scroll_max = 内容 - 可视高度
    float content_height() const;
    float scroll_max() const;
    void clamp_scroll();
    void set_scroll(float value);
    // 由 y 坐标换算滚动值（拖滚动条用）
    float scroll_from_y(float y) const;
    // 滚动条轨道/滑块矩形（右缘内侧；宽度随悬停/拖拽态）
    Rect track_rect() const;
    Rect thumb_rect() const;
    bool in_scrollbar(float x, float y) const;
    // 鼠标是否悬停在右缘滚动条列（控件内坐标 mx_ 判定）
    bool over_bar() const;
    // 滚动触底时触发一次 on_bottom_（离开底部后重新武装）
    void check_bottom();
    // 是否为锚定子控件（不参与流式排布/内容高度）
    bool is_pinned(const Widget* w) const;

    float spacing_ = 4.f;
    float padding_ = 8.f;
    float scroll_step_ = 40.f;
    float scroll_offset_ = 0.f;
    bool needs_layout_ = false;  // 子树变化后抑制滚动条绘制，直到下一次 on_layout()
    const Theme* theme_ = nullptr;
    std::function<void()> on_bottom_;
    bool bottom_fired_ = false;
    bool pending_rearm_ = false;  // UI-127：rearm_bottom 挂起标记，on_layout 开头消费
    float bottom_margin_ = 0.f;  // 距底部触发距离（镜像 layui flow mb；0=严格滚到底）
    std::function<void()> on_scroll_;

    // ---- 垂直滚动条（共享样式/状态，见 ui/ui/scrollbar_style.h）----
    scrollbar::State sb_;
    float mx_ = -1.f, my_ = -1.f;  // 最近鼠标点（控件内坐标）

    // 锚定子控件（w 悬空——被 clear_children 释放——的条目在 on_layout 自动剔除）
    struct Pinned {
        Widget* w;
        PinCorner corner;
        float mx, my;
    };
    std::vector<Pinned> pinned_;
};

}  // namespace ui
