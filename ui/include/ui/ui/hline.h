#pragma once

#include "ui/core/math.h"
#include "ui/ui/widget.h"

namespace ui {
class Renderer;

// 分割线（layui-hr 克隆）：1px 横线。由 UI-039 从 layui_test 的 layui_parts.h 下沉。
class HLine : public Widget {
public:
    HLine(float w, float h, const Vec4& color);

    void on_draw(Renderer& renderer) override;

private:
    Vec4 color_;
};

}  // namespace ui
