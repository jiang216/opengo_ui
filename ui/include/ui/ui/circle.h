#pragma once

#include "ui/core/math.h"
#include "ui/ui/widget.h"

#include <string>

namespace ui {
class Font;
class Renderer;

// 圆形徽标容器（layui-circle 克隆）：直径 = size，绿底白字居中。
// 由 UI-039 从 layui_test 的 layui_parts.h 下沉为 ui 库组件。
class Circle : public Widget {
public:
    Circle(float size, const char* text);

    void set_font(Font* font) { font_ = font; }
    void set_color(const Vec4& c) { color_ = c; }

    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    Font* font_ = nullptr;
    Vec4 color_{0.09f, 0.72f, 0.47f, 1.0f};
};

}  // namespace ui
