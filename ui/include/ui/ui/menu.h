#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 基础垂直菜单（layui-menu 克隆）：白底 + 分组折叠 + 分割线 + 父级子面板 + 徽章 + 选中态。
class Menu : public Widget {
public:
    Menu(float x, float y, float w, float h);

    void add_item(const char* title, std::function<void()> cb = nullptr);
    // 带英文副标题的菜单项（新镜像 docs/2 侧栏：中文主标题 + 灰色英文副标题）
    void add_item_sub(const char* title, const char* subtitle, std::function<void()> cb = nullptr);
    void add_divider();
    void add_group(const char* title, bool allow_spread = true);
    void add_parent(const char* title);
    void end_group();
    // 清空全部项（第二列子菜单按板块重建时复用同一控件）
    void clear();

    void set_checked();
    // 选中指定 (组序号, 组内子项序号)：清除其余项选中态，高亮该项（layui `.layui-menu-item-checked` 绿字）
    void set_selected(int group_index, int item_index);
    void set_badge(const char* text);
    void set_dot_badge();

    void set_large(bool large);
    void set_font(Font*);
    void set_theme(const Theme*);
    // 自定义配色覆盖（bg/border/hover/文字），不设置则用主题默认
    void set_style_overrides(const Vec4& bg, const Vec4& border, const Vec4& hover, const Vec4& text);
    // 普通菜单项被点击时回调（title 文本），供宿主（如 Dropdown）做选中/收起
    void set_on_item_click(std::function<void(const char*)> cb);
    // 分组头被点击时回调（title 文本，spread 已切换后触发），供宿主联动收起/展开外部面板
    void set_on_group_click(std::function<void(const char*)> cb);

    float preferred_height() const override;

protected:
    void on_layout() override;
    void on_draw(Renderer&) override;
    void draw_overlay(Renderer&) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;

private:
    struct Item {
        std::string text;
        std::string subtitle;  // 英文副标题（新镜像 docs/2 侧栏），可空
        std::function<void()> cb;
        enum Type { Plain, Divider, Group, Parent };
        Type type = Plain;
        int level = 0;
        bool checked = false;
        bool spread = true;
        bool allow_spread = true;
        bool has_badge = false;
        bool has_dot = false;
        std::string badge_text;
        float y0 = 0.f, y1 = 0.f;
        mutable int group_index = -1;
        mutable bool visible = true;
    };

    void draw_item(Renderer&, Item&, int index);
    void draw_sub_panel(Renderer&, int parent_index);
    void layout_items();
    void compute_visibility() const;

    std::vector<Item> items_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    bool large_ = false;
    Item* hover_ = nullptr;
    Item* active_hover_ = nullptr;
    int current_level_ = 0;
    Rect sub_panel_rect_{};

    bool style_ov_ = false;
    Vec4 ov_bg_, ov_border_, ov_hover_, ov_text_;
    std::function<void(const char*)> on_item_click_;
    std::function<void(const char*)> on_group_click_;
};

}  // namespace ui
