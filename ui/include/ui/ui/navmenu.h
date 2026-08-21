#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 垂直导航菜单（M17c，layui-nav-tree 克隆）：深色条 + 可展开子菜单 + 3px #16b777 高亮条。
// 类名：layui-nav-tree。
class NavMenu : public Widget {
public:
    NavMenu(float x, float y, float w, float h);

    // 追加一级菜单；随后 add_item 的项挂到它下面
    void add_group(const char* title);
    // 清空全部菜单（第二列子菜单按板块重建时复用同一控件）
    void clear_groups();
    // 给最后一个 group 追加菜单项（点击执行 cb）
    void add_item(const char* title, std::function<void()> cb = {});
    // 给最后一个 group 追加分隔线（镜像 site-demo-table-nav 的 <hr> 分段，无回调不可点击）
    void add_separator();
    // 设置激活的一级组（顶部 3px 指示条；点击一级项也会自动激活）
    void set_active(int group);
    // 设置选中的子项（g 组 i 项，layui `.layui-this` 语义：绿底 #16b777 + 白字）。
    // 点击子项也会自动选中（layui-nav-tree 行为）。
    void set_selected(int group, int item);
    // 展开所有一级组（demo 侧栏多组全展开）；点击一级项仍可单独折叠
    void expand_all();

    void set_font(Font* font);
    void set_theme(const Theme* theme);

    void on_layout() override;
    float preferred_height() const override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    void on_draw(Renderer& renderer) override;

private:
    struct Item {
        std::string text;
        std::function<void()> cb;
        float y0 = 0.f, y1 = 0.f;
        bool separator = false;  // 分隔线（<hr> 分段）：无文字不可点击
    };
    struct Group {
        std::string title;
        std::vector<Item> items;
        bool expanded = false;
        float y0 = 0.f, y1 = 0.f;
        float body_y0 = 0.f;
    };

    void layout_groups();

    std::vector<Group> groups_;
    int active_ = -1;
    int sel_group_ = -1;  // 选中的子项（layui .layui-this）
    int sel_item_ = -1;
    int hover_group_ = -1;
    int hover_item_ = -1;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    float menu_width_ = 200.f;  // 用户指定的宽度，不受 ScrollBox 拉伸影响
};

}  // namespace ui
