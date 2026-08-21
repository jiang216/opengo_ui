#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
class Texture;
struct Theme;

// 横向导航菜单（layui-nav 克隆）：水平排列项 + 子菜单下拉 + 徽章 + 彩色主题。
// 类名：layui-nav（水平）。
class NavBar : public Widget {
public:
    NavBar(float x, float y, float w, float h);
    ~NavBar() override;

    // 追加一个顶级导航项
    void add_item(const char* text, std::function<void()> cb = {});
    // 给最后一个顶级项追加子菜单项（触发下拉）
    void add_child(const char* text, std::function<void()> cb = {});

    // 设置选中项（layui-this：文字 #16b777，下拉项同理）
    void set_active(int index);

    // 徽章（在最后追加的项上）
    void set_badge(const char* text);
    void set_dot_badge();

    // 图片头像（在最后追加的项上）
    void set_image(const char* path);

    // 主题背景色（默认透明；可设 cyan/green/blue 等）
    void set_bg_color(Vec4 color);

    // 禁用滑块跟随（lay-bar="disabled"）
    void set_bar_disabled(bool disabled);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    float preferred_height() const override;
    void on_layout() override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    void on_draw(Renderer& renderer) override;
    void draw_overlay(Renderer& renderer) override;

private:
    struct Child {
        std::string text;
        std::function<void()> cb;
        float x0 = 0.f, x1 = 0.f;
    };
    struct Item {
        std::string text;
        std::function<void()> cb;
        std::vector<Child> children;
        bool has_badge = false;
        std::string badge_text;
        bool has_dot = false;
        bool has_image = false;
        std::string image_path;
        Texture* image_tex = nullptr;
        float x0 = 0.f, x1 = 0.f;
    };

    void layout_items();
    void close_dropdown();

    std::vector<Item> items_;
    int active_ = -1;
    int hover_ = -1;
    int open_dropdown_ = -1;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Vec4 bg_color_ = {0.f, 0.f, 0.f, 0.f};  // 透明 = 默认色
    bool bar_disabled_ = false;
};

}  // namespace ui
