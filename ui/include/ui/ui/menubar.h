#pragma once

#include "ui/ui/widget.h"

#include <functional>

namespace ui {
class Button;
class Font;
class Renderer;
class VBox;
struct Theme;

// 菜单栏（M14）：横向菜单按钮 + 下拉菜单（复用根节点弹出面板机制）。
// 每个菜单一个触发器按钮 + 一个 VBox 弹出面板；点击外部自动关闭。
class MenuBar : public Widget {
public:
    MenuBar(float x, float y, float w, float h);

    void set_font(Font* font);
    void set_theme(const Theme* theme);
    // 追加一个菜单（追加到末尾）；此后 add_item 都添加到该菜单
    void add_menu(const char* title);
    // 给最后一个菜单追加菜单项；点击执行 cb 并关闭弹出
    void add_item(const char* title, std::function<void()> cb);

    void on_layout() override;
    void on_draw(Renderer& renderer) override;

private:
    struct Menu {
        Button* trigger = nullptr;
        VBox* popup = nullptr;
    };
    void toggle_menu(Menu& m);
    void close_all_menus();

    std::vector<Menu> menus_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
