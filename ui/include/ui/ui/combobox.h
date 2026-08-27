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
    // 就地重载全部选项（UI-121）：清空并递归释放旧选项按钮后重建；
    // 调用方无需再「delete+new 销毁重建」整个下拉框——重建孤儿控件（可见性/坐标
    // 失管）是幽灵下拉类缺陷的根源。重载非用户选择事件：不触发 on_change，
    // 原选中项若仍在新列表范围内则保持，否则复位为未选中（显示回退到首项文本）。
    void set_options(const std::vector<std::string>& options);
    void select(int index);  // 选中并触发 on_change
    void close();
    int selected() const { return selected_; }
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }

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
