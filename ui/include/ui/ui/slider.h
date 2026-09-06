#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 滑块（M5，layui-slider 克隆）：水平/垂直拖拽改值，点击/拖拽轨道、键盘方向键微调，
// 支持范围（range 双环）、步长（step）、间断点（showstep）、提示文本（tips/setTips）、
// 输入框（input）、禁用（disabled）、自定义主题色（theme）。
class Slider : public Widget {
public:
    Slider(float x, float y, float w, float h);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_range(float min_value, float max_value);
    // —— 单值（默认模式）——
    void set_value(float v);            // 改值并触发回调
    void set_value_silent(float v);     // 改值但不触发回调（程序同步）
    float value() const { return value_; }
    // —— 范围模式值（range 时两端）——
    void set_range_value(float start, float end);  // 触发回调
    float value2() const { return value2_; }
    bool dragging() const { return dragging_; }

    // —— layui slider 选项 ——
    void set_range_enabled(bool on) {  range_ = on; mark_dirty(); }   // 范围拖拽（双环）
    void set_step(float step) {  step_ = step > 0.f ? step : 1.f; has_step_ = true; mark_dirty(); }  // 显式步长：开启量化
    void set_showstep(bool on) {  showstep_ = on; mark_dirty(); }     // 间断点
    void set_tips(bool on) {  tips_ = on; mark_dirty(); }             // 提示文本
    void set_tips_callback(std::function<std::string(float)> fn) { tips_fn_ = std::move(fn); } // setTips
    void set_input(bool on) {  input_ = on; mark_dirty(); }           // 输入框（range 时无效）
    void set_disabled(bool on) {  disabled_ = on; mark_dirty(); }     // 禁用拖拽
    void set_theme_color(const Vec4& c) {  theme_color_ = c; has_theme_color_ = true; mark_dirty(); } // 主题色
    void set_vertical(bool on) {  vertical_ = on; mark_dirty(); }     // type:vertical
    void set_track_height(float h) {  track_len_ = h > 0.f ? h : 200.f; mark_dirty(); } // vertical 高度
    void set_focus_ring(bool on) {  focus_ring_ = on; mark_dirty(); } // 聚焦外框反馈（默认不显示）

    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    // 值变化回调（普通模式返回单值）
    void set_on_change(std::function<void(float)> cb) { on_change_ = std::move(cb); }
    // 范围模式回调整（start, end）
    void set_on_range_change(std::function<void(float, float)> cb) { on_range_change_ = std::move(cb); }

    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    bool on_key_down(int key, int mods) override;
    void on_draw(Renderer& renderer) override;

private:
    enum class Drag { None, Single, Start, End };

    float value_from_coord(float c) const;  // 像素坐标(横x/纵y)→值（含 step 取整）
    float coord_for(float v) const;         // 值→像素坐标
    float step_round(float v) const;        // 有显式步长才取整；无步长=原值（连续）
    float effective_step() const;           // 有显式步长用 step_；无则自动细步（min(step_, 区间/100)）
    Rect track_rect() const;
    Rect input_rect() const;
    Vec4 accent() const;
    std::string tips_text(float v) const;
    void emit();

    float value_ = 0.f, value2_ = 50.f;
    float min_ = 0.f, max_ = 100.f;
    float step_ = 1.f;      // 默认步长值（无 has_step_ 时仅作 effective_step 上限参考）
    bool has_step_ = false; // set_step 显式设置过：才按 step_round 量化（未设置 = 连续拖动）
    float track_len_ = 200.f;   // vertical 轨道长度
    bool range_ = false;
    bool showstep_ = false;
    bool tips_ = true;
    bool input_ = false;
    bool disabled_ = false;
    bool vertical_ = false;
    bool focus_ring_ = false;   // 聚焦时外框反馈，默认关
    bool has_theme_color_ = false;
    Vec4 theme_color_{0.0f, 0.59f, 0.53f, 1.f};  // #009688
    bool dragging_ = false;
    Drag drag_ = Drag::None;
    int active_range_ = 0;      // 范围模式下当前拖动的端点（0 开始 / 1 结束）
    const Theme* theme_ = nullptr;
    Font* font_ = nullptr;
    std::function<void(float)> on_change_;
    std::function<void(float, float)> on_range_change_;
    std::function<std::string(float)> tips_fn_;
};

}  // namespace ui