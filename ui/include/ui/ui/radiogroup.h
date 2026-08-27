#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <vector>

namespace ui {
class Font;
class Renderer;
class RadioButton;
struct Theme;

// 单选组（M9）：纵向排布若干 RadioButton，组内互斥单选
class RadioGroup : public Widget {
public:
    RadioGroup(float x, float y, float w, float h);
    ~RadioGroup() override;

    // 追加一个选项（行高自动布局）
    void add_option(const char* text);
    int selected() const { return selected_; }
    void select(int index);  // 选中并触发 on_change
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    void set_option_height(float h) {  option_h_ = h; mark_dirty(); }
    void set_spacing(float s) {  spacing_ = s; mark_dirty(); }
    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }

    // 禁用/启用单个选项（layui 单选框禁用）
    void set_option_enabled(int index, bool en);
    // 组级禁用：传播到所有选项
    void set_enabled(bool en) override;

    void on_layout() override;
    void on_draw(Renderer& renderer) override;
    float preferred_width() const;

private:
    std::vector<RadioButton*> options_;
    int selected_ = -1;
    float option_h_ = 24.f;
    float spacing_ = 6.f;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_change_;
};

}  // namespace ui
