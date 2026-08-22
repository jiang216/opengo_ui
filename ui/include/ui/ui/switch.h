#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 统一圆角开关皮肤（UI-130）：项目内一切开关必须用本函数渲染（ui::Switch 本体、
// Table set_cell_switch 行内开关等），严禁另造矩形/方块皮肤——开=绿底白滑块靠右，
// 关=白底灰边灰滑块靠左，禁用=全灰。取色走传入 Theme（空指针回退默认主题）；
// hover/pressed 仅控件本体传入。
void draw_switch_skin(Renderer& renderer, const Rect& rect, const Theme* theme, bool checked,
                      bool enabled, bool hovered = false, bool pressed = false);

// 开关（M17b，layui-form-switch 克隆）：24px 高圆角滑块，可选开/关文本。
// 文本用 "|" 分隔两态：开时显示左段、关时显示右段（layui title="ON|OFF"）。
// 类名：layui-form-switch（默认）+ layui-form-onswitch（选中，由内部维护）。
class Switch : public Widget {
public:
    Switch(float x, float y, float w, float h);
    ~Switch() override;

    // 设置两态文本，用 "|" 分隔（如 "ON|OFF"、"开启|关闭"）；无 "|" 时整体作为开态文本
    void set_text(const char* text);
    const std::string& text() const { return text_; }

    void set_checked(bool on) { checked_ = on; }
    bool checked() const { return checked_; }

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    // 值变化回调（参数为最新值）
    void set_on_change(std::function<void(bool)> cb) { on_change_ = std::move(cb); }

    // lay-checkbox 绑定（开关风格）：设置紧随开关的自定义标题内容控件。
    // 拥有权归 Switch；内容绘制在滑块右侧（开关关态显示）。典型用途：图标双态标题
    // （layui <i moon> | <i light>），配合 set_on_change 在测试端控制显隐。
    void set_custom_content(Widget* content);

    bool on_mouse_down(float x, float y, int button) override;
    bool on_key_down(int key, int mods) override;
    void on_layout() override;
    float preferred_height() const override { return 24.f; }
    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    std::string on_text_;   // 开态文本（| 左段）
    std::string off_text_;  // 关态文本（| 右段）
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Widget* custom_ = nullptr;  // lay-checkbox 自定义标题（owned）
    bool checked_ = false;
    std::function<void(bool)> on_change_;
};

}  // namespace ui