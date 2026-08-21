#pragma once

#include "ui/ui/widget.h"

#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 字段集（layui-elem-field 克隆）：Fieldset 上下边框 + legend；FieldsetTitle 带标题横线。
// 由 UI-039 从 layui_test 的 layui_parts.h 下沉为 ui 库组件（测试程序不得再自建承载）。
class FieldsetTitle : public Widget {
public:
    FieldsetTitle(float w, float h, const char* text);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

// 上下边框 + legend 的字段集，子控件纵向排布
class Fieldset : public Widget {
public:
    Fieldset(float w, float h, const char* legend);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    float preferred_height() const override;
    void on_layout() override;
    void on_draw(Renderer& renderer) override;

private:
    std::string legend_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
