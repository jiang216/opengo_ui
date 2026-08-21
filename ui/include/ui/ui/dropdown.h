#pragma once

#include "ui/ui/widget.h"

#include <chrono>
#include <functional>
#include <string>

namespace ui {
class Button;
class Font;
class Menu;
class Renderer;
struct Theme;

// 触发器方式
enum class DropdownTrigger { Click, Hover, MouseDown, DblClick, ContextMenu, Manual };
// 弹出对齐
enum class DropdownAlign { Left, Center, Right };
// 触发器外观：绿按钮 / 主色描边按钮 / 输入框样式 / 纯文本
enum class DropdownStyle { Button, PrimaryButton, Input, Text };

// 下拉菜单（layui-dropdown / layui-menu 克隆）：支持层级（父子面板）、分组、分隔线、
// 多种触发器（点击/悬停/按下/双击/右键）、对齐、自定义内容与自定义样式。
// 内部用 Menu 承载默认菜单内容。
class Dropdown : public Widget {
public:
    Dropdown(float x, float y, float w, float h, const char* title = "menu");
    ~Dropdown() override;

    // ---- 菜单内容（内部 Menu，默认白底弹板）----
    void add_item(const char* title, std::function<void()> cb = {});
    void add_separator();
    void add_group(const char* title, bool allow_spread = true);
    void add_parent(const char* title);
    void end_group();

    // ---- 触发器 / 外观 ----
    void set_trigger_style(DropdownStyle s);
    void set_trigger_event(DropdownTrigger t);
    void set_align(DropdownAlign a);
    void set_menu_width(float w);
    // 将触发器绘制为一块灰色区域并居中显示提示文字（常用于右键菜单占位区）
    void set_region(const char* placeholder);
    // 自定义弹出内容（非空则替代默认菜单）
    void set_custom_content(Widget* content);
    // 自定义弹出面板配色（bg / hover / 文字）
    void set_popup_style(const Vec4& bg, const Vec4& hover, const Vec4& text);

    // ---- 显隐（Manual / 外部触发 / 表格行）----
    void show();
    void hide();
    void show_at(float sx, float sy);
    bool opened() const;

    // 选中项回调（title 文本）
    void set_on_select(std::function<void(const char*)> cb) { on_select_ = std::move(cb); }

    void set_font(Font* font);
    void set_theme(const Theme* theme);

    void on_layout() override;
    void on_draw(Renderer& renderer) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;

private:
    void open();
    void place_popup();
    int menu_height() const;

    Button* trigger_ = nullptr;
    Menu* menu_ = nullptr;
    Widget* custom_ = nullptr;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;

    DropdownStyle style_ = DropdownStyle::PrimaryButton;
    DropdownTrigger event_ = DropdownTrigger::Click;
    DropdownAlign align_ = DropdownAlign::Left;
    float menu_w_ = 160.f;

    Vec4 popup_bg_{1.f, 1.f, 1.f, 1.f};
    Vec4 popup_hover_{0.97f, 0.97f, 0.97f, 1.f};
    Vec4 popup_text_{0.4f, 0.4f, 0.4f, 1.f};
    bool style_custom_ = false;

    std::string input_text_;
    std::string title_;
    std::string region_txt_;
    std::function<void(const char*)> on_select_;

    std::chrono::steady_clock::time_point last_down_;
    bool hover_open_ = false;
    float at_x_ = 0.f, at_y_ = 0.f;
    bool show_at_ = false;
};
}  // namespace ui