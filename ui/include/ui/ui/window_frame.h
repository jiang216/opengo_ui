#pragma once

#include "ui/ui/widget.h"
#include "ui/core/math.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
class TitleBar;
class Window;
struct Theme;

// 窗口容器 / 框架控件（window_frame，UI-154）：自带顶部自绘标题栏的整窗容器。
//
// 结构：head（默认系统风格 TitleBar，标题栏高度 head_h 固定参数化，默认 30）
// 置于顶部 (0,0,w,hh)；content 内容区置于其下方 (0,hh,w,h-hh)。
//
// 归因（AGENTS）：本库子控件 bounds_ 为绝对坐标、Widget::draw/dispatch 无父级坐标变换，
// 容器靠 on_layout 手动叠加父偏移（惯例见 card/blockquote/collapse/checkbox）。
// UI-153 已为 Scene 提供内容原点（content_origin_y）；WindowFrame 把「顶部装帧 +
// 内容区原点」下沉为可复用库控件：内容子控件用「容器相对坐标」定位，由本控件
// 在 on_layout 内按快照重基准（y 相对内容区顶部，即标题栏之下）为绝对坐标，
// 避免绝对叠加累积误差。
//
// 用法：
//   WindowFrame* f = new WindowFrame(760, 300);
//   f->set_title("我的窗口");
//   scene.add_child(f);
//   ui::Widget* body = f->content();          // 内容区（标题栏下方）
//   body->add_child(btn);
//   f->set_content_child(btn, 20, 20, 140, 36);  // (x,y) 相对内容区顶部，非窗口原点
//
// 通用头部：set_head(Widget*, height) 可把任意 Widget 当作 head（TitleBar 为默认）；
// 换用自定义头部后，set_title/set_icon/set_on_* 等 TitleBar 透传方法仅对默认头部生效。
class WindowFrame : public Widget {
public:
    WindowFrame(float w, float h);

    // ---- 头部槽 ----
    // 通用头部：替换默认 TitleBar（任意外部构造的 Widget 作 head）；width 随本控件矩形自动铺满。
    // 传 nullptr 表示无头部（内容区从 y=0 起）。
    void set_head(Widget* head, float height);
    // 标题栏高度（默认 30）：仅当 head 为内部默认 TitleBar 时同步内容区原点；自定义 head 请用 set_head 指定
    void set_titlebar_height(float h);
    float titlebar_height() const { return head_h_; }
    // 内容区顶部（标题栏下方）的 y：等价 titlebar_height()，便于外部以之为单一来源派生子坐标
    float content_origin_y() const { return head_h_; }

    // ---- 默认 TitleBar 透传（仅当 head 为内部默认标题栏时生效）----
    void set_font(Font* f);
    void set_theme(const Theme* t);
    void set_title(const char* title);
    void set_icon(Font* font, uint32_t codepoint);
    void set_on_close(std::function<void()> cb);
    void set_on_minimize(std::function<void()> cb);
    void set_on_maximize(std::function<void()> cb);
    void set_minimize_visible(bool v);
    void set_maximize_visible(bool v);
    void set_close_visible(bool v);

    // ---- 内容区 ----
    // 返回内容容器：把内容子控件 add_child 到它上面
    Widget* content() const { return content_; }
    // 以「容器相对坐标」定位内容子控件：(x,y) 相对内容区顶部（标题栏下方）。
    // 记录相对矩形快照并于 on_layout 重基准为绝对坐标（相对 = 绝对 - 内容区原点），
    // 不会随拖动/缩放二次叠加。
    void set_content_child(Widget* child, float x, float y, float w, float h);

    // 以「通用头部相对坐标」定位头部子控件：(x,y) 相对头部左上角（即本框架原点）。
    // 仅对非默认 TitleBar 的通用 head 有意义（默认 TitleBar 自绘无子控件）；
    // 记录相对矩形快照并于 on_layout 重基准为绝对坐标，与 set_content_child 对称。
    void set_head_child(Widget* child, float x, float y, float w, float h);

    void on_layout() override;

private:
    struct Slot {
        Widget* child;
        Rect rel;  // 相对所在区域（head 或 content）的矩形（x,y,w,h）
    };

    void set_slot(std::vector<Slot>& slots, Widget* child, float x, float y, float w, float h, float ox, float oy);
    void apply_slots(std::vector<Slot>& slots, Widget* owner, float ox, float oy);

    Widget* head_ = nullptr;
    Widget* content_ = nullptr;
    bool head_owned_ = false;  // head_ 是否为默认内部 TitleBar（重建/替换时需释放）
    float head_h_ = 30.f;

    std::vector<Slot> head_slots_;
    std::vector<Slot> content_slots_;
};

}  // namespace ui
