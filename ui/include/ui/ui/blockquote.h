#pragma once

#include "ui/ui/widget.h"

#include <string>

namespace ui {
class Font;
class Label;
class Renderer;
struct Theme;

// 引用区块（layui-elem-quote 克隆）：默认灰底 + 左侧 5px 绿条；nm 变体无底色。
// 由 UI-039 从 layui_test 的 layui_parts.h 下沉为 ui 库组件。
class BlockQuote : public Widget {
public:
    BlockQuote(float w, float h, const char* text, bool nm = false);

    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_text(const char* text);

    float preferred_height() const override;
    void on_layout() override;
    void on_draw(Renderer& renderer) override;

private:
    Label* label_ = nullptr;
    const Theme* theme_ = nullptr;
    bool nm_ = false;
};

}  // namespace ui
