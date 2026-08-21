#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 复选框（M9）：方框 + 文本标签，点击/空格切换选中态。
// layui form/checkbox 克隆增强（UI-097）：
//   - 选中态方框内绘制对勾（layui-icon-ok）
//   - set_indeterminate() 半选态（绿框白底 + 中间横线 layui-icon-subtraction）
//   - set_skin() 皮肤：Primary(默认) / Tag(标签) / None(无默认样式，配合自定义内容)
// 类名：layui-form-checkbox（默认）+ layui-form-checked（选中，由内部维护）。
class CheckBox : public Widget {
public:
    enum class Skin {
        Primary,  // 默认：方框 + 对勾 + 文本
        Tag,      // 标签：选中时整标签绿底白字（layui lay-skin=tag）
        None,     // 无默认样式（layui lay-skin=none），配合自定义内容绑定
    };

    CheckBox(float x, float y, float w, float h, const char* text);
    ~CheckBox() override;

    void set_text(const char* text) { text_ = text ? text : ""; }
    void set_checked(bool checked);
    bool checked() const { return checked_; }
    void toggle();

    // 半选态（layui indeterminate）：只可动态设置，方框绿框白底 + 中间横线
    void set_indeterminate(bool ind) { indeterminate_ = ind; }
    bool indeterminate() const { return indeterminate_; }

    void set_skin(Skin skin) { skin_ = skin; }
    Skin skin() const { return skin_; }

    // 选中态变化（点击/键盘触发）
    void set_on_change(std::function<void(bool)> cb) { on_change_ = std::move(cb); }
    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    // lay-checkbox 绑定：设置紧随复选框的自定义内容控件（layui <div lay-checkbox>）。
    // 内容控件拥有权归 CheckBox（析构/clear_children 时释放）；点击内容区域同样切换选中态。
    // 典型用途：自定义标题模板（链接/图标）、lay-skin=none 卡片风格多选组件。
    // 注意：custom_ 不作为子节点（避免双重绘制/布局），由 CheckBox 在 on_layout/on_draw
    // 中直接管理（Dropdown 同款模式）。
    void set_custom_content(Widget* content);

    bool on_mouse_down(float x, float y, int button) override;
    bool on_key_down(int key, int mods) override;
    void on_layout() override;
    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    bool checked_ = false;
    bool indeterminate_ = false;
    Skin skin_ = Skin::Primary;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    Widget* custom_ = nullptr;  // lay-checkbox 自定义内容（owned）
    std::function<void(bool)> on_change_;
};

}  // namespace ui