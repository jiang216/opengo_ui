#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 可移动/缩放窗口（M14）：标题栏拖拽移动 + 右下角缩放柄 + 关闭按钮。
// 子控件放在客户区（标题栏下方），由 on_layout 约束在客户区左上角起排布。
// 配合根节点 set_modal 可做成模态对话框。
// UI-069 扩展（复刻 layui layer.html）：可隐藏标题栏/关闭钮、最大化/还原、
// 底部遮罩（shade）、自动关闭计时（msg 20s）、底部按钮条。
class PanelWindow : public Widget {
public:
    PanelWindow(float x, float y, float w, float h, const char* title);

    void set_title(const char* title) {  title_ = title ? title : ""; mark_dirty(); }
    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    // 关闭按钮回调（通常由使用者隐藏/销毁窗口）
    void set_on_close(std::function<void()> cb) { on_close_ = std::move(cb); }

    // ---- UI-069 ----
    // 是否显示标题栏（layer title:false）
    void set_title_visible(bool v) {  title_visible_ = v; mark_dirty(); }
    // 是否显示关闭按钮（layer closeBtn:false）
    void set_close_visible(bool v) {  close_visible_ = v; mark_dirty(); }
    // 最大化/还原按钮（layer maxmin:true），标题栏右侧追加 ─ / □
    void set_maxmin(bool v) {  maxmin_ = v; mark_dirty(); }
    // 底部遮罩 alpha（layer shade:0..1），0 表示无遮罩；遮罩覆盖整个视口绘制在窗口之后
    void set_shade(float alpha) {  shade_ = alpha; mark_dirty(); }
    // 底部按钮条：labels 按钮文本，align 0=左 1=中 2=右；点击回调（回调参数为按钮下标）
    void set_footer_buttons(const std::vector<std::string>& labels, int align,
                            std::function<void(int)> cb);
    // 自动关闭秒数（layer msg time），<=0 不自动关闭；需应用层每帧 root->advance(dt)
    void set_auto_close(float seconds) {  auto_close_ = seconds; mark_dirty(); }
    // 客户区高度中按钮条预留（footer_h），供子控件布局时避让
    float footer_height() const { return footer_buttons_.empty() ? 0.f : footer_h_; }

    // 客户区矩形（标题栏下方）
    Rect client_rect() const;

    void on_layout() override;
    void on_advance(float dt) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    bool in_title(float x, float y) const;
    bool in_close(float x, float y) const;
    bool in_maxmin(float x, float y) const;
    bool in_resize(float x, float y) const;
    bool in_footer(float x, float y, int& out_index) const;
    void toggle_maximize();
    // 标题栏高：layui-layer 外观下 50px（M17c）
    float title_height() const;

    std::string title_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void()> on_close_;
    bool dragging_ = false;  // 移动中
    bool resizing_ = false;  // 缩放中
    float drag_dx_ = 0, drag_dy_ = 0;
    const float title_h_ = 26.f;
    const float corner_ = 16.f;
    // UI-069
    bool title_visible_ = true;
    bool close_visible_ = true;
    bool maxmin_ = false;
    float shade_ = 0.f;
    float auto_close_ = 0.f;
    bool maximized_ = false;         // 当前是否最大化
    Rect restore_rect_{0, 0, 0, 0};  // 最大化前的原始矩形（还原用）
    const float footer_h_ = 56.f;    // 底部按钮条高
    std::vector<std::string> footer_buttons_;
    int footer_align_ = 1;           // 0 左 / 1 中 / 2 右
    std::function<void(int)> on_footer_;
    int footer_hover_ = -1;          // hover 按钮下标
    // 子控件客户区相对偏移（UI-073）：add_child 时记录子控件相对客户区的偏移，
    // on_layout 每帧基于此偏移重新定位，保证窗口移动/缩放后子控件跟随。
    struct ChildLayoutInfo {
        Widget* widget;
        Rect rel;  // 相对客户区左上角的偏移
    };
    std::vector<ChildLayoutInfo> child_layout_info_;
};

}  // namespace ui
