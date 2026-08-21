#pragma once

#include "ui/ui/widget.h"

#include <functional>

namespace ui {
class Renderer;
struct Theme;

// 纵向滚动容器（M8）：子控件按顺序纵向排列，超出可视区用 scissor 裁剪，
// 滚轮 / 拖动右侧滚动条滚动。内容自适应宽度，高度由外部指定。
class ScrollBox : public Widget {
public:
    ScrollBox(float x, float y, float w, float h);

    // 子控件之间的间距（像素）
    void set_spacing(float spacing) { spacing_ = spacing; }
    // 内部边距（像素）
    void set_padding(float padding) { padding_ = padding; }
    float padding() const { return padding_; }
    // 滚动步长（每滚一格滚轮）
    void set_scroll_step(float step) { scroll_step_ = step; }
    float scroll_offset() const { return scroll_offset_; }
    // 滚动到指定偏移（自动 clamp 到 [0, scroll_max]；脚本化截图/程序化定位用）
    void set_scroll_offset(float offset) { set_scroll(offset); }
    // 滚动回顶部（切页/内容重建后复位滚动位置，防止残留旧页滚动偏移）
    void reset_scroll();
    // 标记子树变化（clear_children/add_child 后调用），抑制滚动条绘制直到下一次 on_layout()
    void mark_content_dirty() { needs_layout_ = true; }

    // 滚到内容底部时回调一次（再次离开底部再滚回底部会重新触发）。
    // 传入 nullptr 清除回调（切页清理用，防悬空引用）。
    void set_on_scroll_to_bottom(std::function<void()> cb) { on_bottom_ = std::move(cb); }

    // 通用滚动回调：每次 scroll_offset 变化时触发（用于关闭下拉面板等场景）。
    void set_on_scroll(std::function<void()> cb) { on_scroll_ = std::move(cb); }

    // 距底部触发距离 margin（镜像 layui flow mb，默认 50）：当 scroll_offset 距内容底部
    // 不足 mb 像素时即触发触底回调，而不必严格滚到底（UI-094）。仅影响 check_bottom，
    // 触发/重新武装语义（bottom_fired_）与 set_on_scroll_to_bottom 一致。
    void set_bottom_margin(float mb) { bottom_margin_ = mb; }

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

    void set_theme(const Theme* theme) { theme_ = theme; }

private:
    // 内容高度（所有子控件 + 间距 + 边距）；溢出量 scroll_max = 内容 - 可视高度
    float content_height() const;
    float scroll_max() const;
    void clamp_scroll();
    void set_scroll(float value);
    // 由 y 坐标换算滚动值（拖滚动条用）
    float scroll_from_y(float y) const;
    // 滚动条轨道/滑块矩形（右缘内侧）
    Rect track_rect() const;
    Rect thumb_rect() const;
    bool in_scrollbar(float x, float y) const;
    // 滚动触底时触发一次 on_bottom_（离开底部后重新武装）
    void check_bottom();

    float spacing_ = 4.f;
    float padding_ = 8.f;
    float scroll_step_ = 40.f;
    float scroll_offset_ = 0.f;
    bool dragging_ = false;
    bool needs_layout_ = false;  // 子树变化后抑制滚动条绘制，直到下一次 on_layout()
    const Theme* theme_ = nullptr;
    std::function<void()> on_bottom_;
    bool bottom_fired_ = false;
    float bottom_margin_ = 0.f;  // 距底部触发距离（镜像 layui flow mb；0=严格滚到底）
    std::function<void()> on_scroll_;
};

}  // namespace ui
