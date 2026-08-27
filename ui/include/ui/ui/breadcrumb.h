#pragma once

#include "ui/ui/widget.h"

#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 面包屑（M17a，layui-breadcrumb 克隆）：链接 #999 hover #16b777，当前项 cite #5f5f5f，
// 分隔符 "lay-separator"（默认 " / "，margin 0 10px）。水平排布。
class Breadcrumb : public Widget {
public:
    Breadcrumb(float x, float y, float w, float h);

    // 追加一个链接项（灰色，hover 变 #16b777）
    void add_item(const char* text);
    // 追加当前页（cite，#5f5f5f）
    void add_current(const char* text);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    void set_separator(const char* sep) {  separator_ = sep ? sep : " / "; mark_dirty(); }

    float preferred_height() const override;
    void on_draw(Renderer& renderer) override;

private:
    struct Item {
        std::string text;
        bool current = false;
    };
    std::vector<Item> items_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::string separator_ = " / ";
};

}  // namespace ui
