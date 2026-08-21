#pragma once

#include "ui/ui/widget.h"

#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 徽标（M17a，layui-badge 克隆）：红底白字圆角小徽标。
// 类名：layui-badge（默认）、layui-badge-dot（8px 圆点）、layui-badge-rim（白底灰字描边）、
// 可叠加 layui-bg-red/orange/green/... 换底色。
class Badge : public Widget {
public:
    Badge(float x, float y, const char* text);

    void set_text(const char* text) { text_ = text ? text : ""; }
    const std::string& text() const { return text_; }

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    // 内容自适应宽度（文本宽 + 左右内边距；dot 为 8px）
    float preferred_width() const;
    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
