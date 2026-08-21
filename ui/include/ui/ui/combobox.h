#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Button;
class Font;
class Renderer;
class ScrollBox;
struct Theme;

// 下拉选择框（M9）：点击展开选项列表（复用 ScrollBox + 根级弹出层），点击外部自动关闭
class ComboBox : public Widget {
public:
    ComboBox(float x, float y, float w, float h);
    ~ComboBox() override;

    void add_option(const char* text);
    void select(int index);  // 选中并触发 on_change
    void close();
    int selected() const { return selected_; }
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    void on_layout() override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_key_down(int key, int mods) override;
    void on_draw(Renderer& renderer) override;

private:
    void open();
    void sync_panel();

    // 全实例注册表：保证「打开一个下拉，关闭其他所有下拉」（open 时遍历关闭）
    static std::vector<ComboBox*> instances_;

    std::vector<std::string> options_;
    int selected_ = -1;
    ScrollBox* panel_ = nullptr;
    std::vector<Button*> option_buttons_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_change_;
};

}  // namespace ui
