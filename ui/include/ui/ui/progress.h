#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Font;
class Renderer;
struct Theme;

// 进度条（M17a，layui-progress 克隆）：轨道 #eee h6 圆角 20 + 填充 #16b777。
// 类名：layui-progress（默认）、layui-progress-big（高 18px）、layui-progress-bar（无文本时也绘制）
class Progress : public Widget {
public:
    Progress(float x, float y, float w, float h);

    // 进度 0..1
    void set_value(float v) { value_ = v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
    float value() const { return value_; }

    // 是否显示进度文本（layui-progress-text；默认显示百分比 "40%"）
    void set_show_text(bool show) { show_text_ = show; }

    // 自定义显示文本（如分数 "5 / 10"），为空则回退到百分比。清空后传 nullptr。
    void set_text_override(const char* text) { text_ = text ? text : ""; }
    const std::string& text_override() const { return text_; }

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    void on_draw(Renderer& renderer) override;

private:
    float value_ = 0.f;
    bool show_text_ = true;
    std::string text_;  // 自定义显示文本（空则显示百分比）
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
