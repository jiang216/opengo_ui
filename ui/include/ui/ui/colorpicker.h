#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 颜色展示/输入格式（layui colorpicker format：hex / rgb）
enum class ColorFormat { Hex, Rgb };   // rgb 开启 alpha 时自动转为 rgba

// 颜色框尺寸（layui colorpicker size：lg / sm / xs）
enum class ColorPickerSize { Lg, Sm, Xs };

// 颜色选择器（M17d，layui-colorpicker 克隆）：SV 色场 + 色相/透明度滑块 + 格式显示 +
// 预定义色 + 确认/清除。支持 hex / rgb / rgba 三类色彩模式（format + alpha），
// change 回调（取色中）与 done 回调（确认/清除）。
class ColorPicker : public Widget {
public:
    ColorPicker(float x, float y, float w, float h);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    // 默认色：任意颜色转为内部 RGBA + HSV
    void set_color(const Vec4& c);
    const Vec4& color() const { return color_; }

    // —— layui colorpicker 选项 ——
    void set_format(ColorFormat fmt) { format_ = fmt; }   // hex / rgb（alpha 时自动 rgba）
    void set_alpha(bool on) { alpha_ = on; }              // 开启透明度滑块
    void set_predefine(bool on) { predefine_ = on; }      // 开启预定义颜色
    void set_colors(const std::vector<Vec4>& cols);       // 自定义预定义色（需 predefine:true）
    void set_size(ColorPickerSize s) { size_ = s; }       // 颜色框尺寸 lg / sm / xs
    void set_clearable(bool on) { clearable_ = on; }      // 清除按钮（默认 true）

    // —— 回调 ——
    void set_on_change(std::function<void(const Vec4&)> cb) { on_change_ = std::move(cb); }  // 取色中
    void set_on_done(std::function<void(const Vec4&)> cb) { on_done_ = std::move(cb); }      // 确认/清除

    float preferred_height() const override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    enum class Drag { None, SV, Hue, Alpha };

    void apply_sv(float x, float y);
    void apply_hue(float x);
    void apply_alpha(float x);
    float sv_side() const;
    Rect sv_rect() const;       // SV 色场
    Rect hue_rect() const;      // 色相滑块
    Rect alpha_rect() const;    // 透明度滑块（alpha_ 开启时）
    Rect predefine_rect() const;
    Rect buttons_area() const;
    Rect confirm_rect() const;
    Rect clear_rect() const;
    float range_thick() const;
    std::string format_color() const;

    std::vector<Vec4> palette_;             // 预定义色
    Vec4 color_{0.09f, 0.73f, 0.67f, 1.0f}; // 默认 #16baaa
    float hue_ = 0.f, sat_ = 0.f, val_ = 0.7f;
    ColorFormat format_ = ColorFormat::Hex;
    ColorPickerSize size_ = ColorPickerSize::Sm;
    bool alpha_ = false;
    bool predefine_ = false;
    bool clearable_ = true;
    Drag drag_ = Drag::None;
    bool confirm_hover_ = false, clear_hover_ = false;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(const Vec4&)> on_change_;
    std::function<void(const Vec4&)> on_done_;
};

}  // namespace ui