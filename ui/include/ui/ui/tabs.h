#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Button;
class Font;
class Label;
class Renderer;
class ScrollBox;
struct Theme;

// 标签页（M17b + 增强，layui-tab 克隆）：标题行 + 当前页内容区。
// 类名：layui-tab（默认）/ layui-tab-card（卡片式）/ layui-tab-brief（简约下划线式）。
//
// 增强能力（对应 layui tab.html / element 模块）：
//   - 内容区：每页可带 content 文本，切换时联动显示/隐藏（add_tab / set_content）
//   - 动态操作：count()/remove_tab()/set_active() 实现「新增/删除/切换到指定页」，
//     remove 支持 id 索引（lay-id 语义，外部定位凭据）
//   - 允许关闭：set_allow_close(true) 后每页标题右缘绘制 ×，点击删除该页
//   - 超宽响应式：标题总宽超出可用宽时，右侧绘制展开三角，点击弹出下拉面板列出全部页
class Tabs : public Widget {
public:
    Tabs(float x, float y, float w, float h);
    ~Tabs() override;

    int add_tab(const char* title, const char* content = nullptr);  // 追加并返回页下标
    void set_content(int index, const char* content);               // 设置指定页内容文本
    void remove_tab(int index);                                     // 删除指定页（含内容区联动）
    int count() const { return (int)tabs_.size(); }

    // 徽章 / 小圆点（作用在最后 add_tab 的标签页标题右缘，layui-tab 标题内嵌）
    void set_badge(const char* text);
    void set_dot_badge();

    // 激活第 index 个标签页并触发 on_change
    void set_active(int index);
    int active() const { return active_; }
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    // lay-allowclose：true 时每页标题右缘显示删除 ×，点击删除该页
    void set_allow_close(bool allow) { allow_close_ = allow; }
    bool allow_close() const { return allow_close_; }

    void set_font(Font* font);
    void set_theme(const Theme* theme);

    void on_layout() override;
    float preferred_height() const override;
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    struct Tab {
        std::string title;
        std::string content;
        bool has_badge = false;      // 标题右缘徽章（layui-badge 红底白字）
        std::string badge_text;
        bool has_dot = false;        // 标题右缘小圆点（8px layui-badge-dot）
        float x0 = 0.f, x1 = 0.f;    // 标题条内 x 范围（点击命中）
        bool visible = true;         // 超宽时被收纳的页标为不可见
    };

    int hit_tab(float x) const;                       // 命中可见标题下标，未命中 -1
    bool hit_close(float x, float y) const;           // 点击允许关闭页的 × 区
    void apply_active();                              // 激活页内容写入内容区 Label
    void layout_titles();                             // 计算标题布局 + 超宽收纳
    float title_width(const Tab& tb) const;           // 单个标题宽（含徽章/圆点余量）
    void open_overflow();                             // 展开超宽收纳页下拉面板
    void show_content(const char* text);              // 内容区显示指定文本
    std::vector<Button*> overflow_buttons();          // 下拉面板中的按钮列表
    float content_h() const;                          // 内容区高度
    float overflow_x() const;                         // 超宽展开三角区起点 x

    std::vector<Tab> tabs_;
    int active_ = 0;
    bool allow_close_ = false;
    bool overflow_ = false;          // 标题是否超宽（触发展开三角）
    float kTitleH = 40.f;            // layui-tab-title 高
    Label* content_label_ = nullptr; // 内容区文本（内部子控件的单个 Label）
    ScrollBox* overflow_panel_ = nullptr;  // 超宽展开下拉面板（根级弹出）
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_change_;
};

}  // namespace ui