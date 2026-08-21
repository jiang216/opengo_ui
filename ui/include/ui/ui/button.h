#pragma once

#include "ui/ui/widget.h"

#include <cstdint>
#include <functional>
#include <string>

namespace ui {
class Font;
class Renderer;
struct Theme;

// ��ƽ��ť��Բ�Ǳ��� + �����ı� + hover/pressed ״̬��
// layui ���壨add_class("layui-btn ...")����layui-btn-primary/normal/warm/danger/disabled +
// layui-btn-lg/sm/xs + layui-btn-radius + layui-btn-fluid��hover Ϊ opacity .8��active �ָ���
// 图标：set_icon("left") 等（layui_icons.h 类名，去 layui-icon- 前缀），图标居左、文本居右整体居中
// 按钮组（ButtonGroup 自动设置组角色）：中间圆角 0、首/尾圆角 2、非首彩色按钮左侧 1px 半透明白分隔线
class Button : public Widget {
public:
    Button(float x, float y, float w, float h, const char* text);

    void set_text(const char* text) { text_ = text ? text : ""; }
    const std::string& text() const { return text_; }

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    // layui 图标按钮：图标类名（如 "left"，即 layui-icon-left 去前缀）
    void set_icon(const char* icon_name);
    // 图标放文本右侧（镜像「按钮 <i class="layui-icon-down">」文字在前语义）；
    // 默认 false = 图标居左（既有行为不变）
    void set_icon_right(bool on) { icon_right_ = on; }
    bool icon_right() const { return icon_right_; }

    // 按钮内嵌徽章（layui-badge 红底白字，内嵌在文本右缘，如 `<button>查看消息<span layui-badge>1</span>`）
    // 叠加 layui-bg-* 可换底（如 layui-bg-gray / layui-bg-orange）；dot 为 8px 圆点
    void set_badge(const char* text);
    void set_dot_badge();
    // 按钮组角色（ButtonGroup 自动维护）：0=普通，1=首，2=中间，3=尾
    void set_group_role(int role) { group_role_ = role; }
    int group_role() const { return group_role_; }

    // 松开瞬间若仍在按钮内触发一次
    void set_on_click(std::function<void()> cb) { on_click_ = std::move(cb); }

    bool on_mouse_up(float x, float y, int button) override;
    bool on_mouse_down(float x, float y, int button) override;
    // Enter/Space 触发点击
    bool on_key_down(int key, int mods) override;
    // layui 尺寸类名（lg/sm/xs）返回对应高度，否则 bounds_.h
    float preferred_height() const override;
    void on_draw(Renderer& renderer) override;

private:
    std::string text_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void()> on_click_;
    unsigned icon_cp_ = 0;   // 图标码点（0 表示无图标）
    int group_role_ = 0;     // 按钮组角色（0 普通）
    bool has_badge_ = false; // 内嵌徽章
    std::string badge_text_;
    bool has_dot_ = false;   // 内嵌小圆点
    bool icon_right_ = false; // 图标置于文本右侧
};

}  // namespace ui
