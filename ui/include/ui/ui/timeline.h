#pragma once

#include "ui/ui/widget.h"

#include <chrono>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 内容行类型（富内容块）
enum class LineKind { Text, Bullet, Em, Icon };

// 富内容行：Text 普通段落 / Bullet 无序列表项 / Em 强调（灰 #999）/ Icon 行内图标
struct ContentLine {
    LineKind kind = LineKind::Text;
    std::string text;
    uint32_t codepoint = 0;  // Icon 用
};

// 时间轴（M17a，layui-timeline 克隆）：左缘 #eee 竖线 + 20px 白底圆轴（内芯 icon）
// + 右侧内容（padding-left 25px）。条目纵向排布，item padding-bottom 20px。
// axis 轴点 20px 白圆内绘制图标（默认 circle 0xE63F 绿 #5FB878，可 set_axis_icon 换）。
// 标题 h3（large，18px #333）或普通标题；内容支持多行/列表/强调/图标富块。
class Timeline : public Widget {
public:
    Timeline(float x, float y, float w, float h);

    // 追加条目：title 为时间轴标题，content 为详情（可空，单行）
    void add_item(const char* title, const char* content);

    // 追加富内容条目：title 普通标题（false）或 h3 大标题（true，18px #333）
    void add_item_rich(const char* title, bool title_large, std::vector<ContentLine> lines);

    // 最后 add_item 条目的轴点图标（默认 layui-icon-circle 0xE63F）
    void set_axis_icon(uint32_t codepoint);

    // 最后 add_item 条目的轴点图标绕中心旋转（角度/秒，>0 开启；用于 loading 循环动画）
    void set_axis_rotate(float degrees_per_sec);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }

    // 内容高度 = 条目数 × (标题行 + 内容行数 × 行高 + 20px padding)
    float preferred_height() const override;
    bool on_mouse_move(float x, float y) override;
    void on_draw(Renderer& renderer) override;

private:
    struct Item {
        std::string title;
        bool title_large = false;
        std::vector<ContentLine> lines;  // 空 = 无详情
        uint32_t axis_icon = 0xE63F;     // layui-icon-circle
        float axis_rotate_dps = 0.f;     // 轴点图标旋转角速度（/秒；0 不旋转）
    };
    std::vector<Item> items_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    int hovered_index_ = -1;  // 鼠标悬停的轴点条目索引（-1 无；镜像 :hover 轴红 #FF5722）
    std::chrono::steady_clock::time_point anim_start_ = std::chrono::steady_clock::now();
};

}  // namespace ui
