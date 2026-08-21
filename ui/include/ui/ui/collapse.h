#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 折叠面板（M17b，layui-collapse 克隆）：多个面板项，点击标题展开/收起内容。
// 用法：add_item("title") 后 add_child(...) 填充该面板内容，再 add_item 下一个。
// 手风琴：默认同一时刻仅一个展开（lay-accordion）；set_accordion(false) 可多开（layui-collapse 默认）。
// 类名：layui-collapse。
class Collapse : public Widget {
public:
    Collapse(float x, float y, float w, float h);

    void add_item(const char* title);
    void set_active(int index);       // 点按展开/收起该项
    int active() const;               // 手风琴：当前展开项；非手风琴：最后操作项
    bool is_open(int index) const;    // 该项当前是否展开
    void set_accordion(bool on) { accordion_ = on; }  // 手风琴（默认 true）
    bool accordion() const { return accordion_; }
    int open_count() const;           // 当前展开项数
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    void set_font(Font* font);
    void set_theme(const Theme* theme);

    void on_layout() override;
    float preferred_height() const override;
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    struct Item {
        std::string title;
        size_t child_begin = 0;  // children_ 中本项内容起始下标
        float y0 = 0.f, y1 = 0.f;  // 标题条 y 范围（点击命中）
    };

    void layout_items();
    bool is_open_impl(int index) const;

    std::vector<Item> items_;
    std::vector<bool> open_;
    int active_ = -1;               // 手风琴：当前展开项 / 非手风琴：最后操作项
    bool accordion_ = true;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_change_;
};

}  // namespace ui
