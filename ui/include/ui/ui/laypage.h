#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Button;
class ComboBox;
class Font;
class Renderer;
class TextBox;
struct Theme;

// 分页条（M17c，layui-laypage 克隆）。
// 支持 layui laypage 的完整布局项：count / prev / page / next / limit / refresh / skip，
// 以及 first/last 首页尾页、自定义按钮文本、自定义主题色。
// 当前页主题色白字，其余白底 1px #eee 边框。
class Laypage : public Widget {
public:
    // 布局项（镜像 layui laypage layout 数组）
    enum class Item { Count, Prev, Page, Next, Limit, Refresh, Skip, First, Last };

    Laypage(float x, float y, float w, float h);
    ~Laypage() override;

    // ---- 数据（兼容旧接口：set_total = 页数；新增 set_count = 数据总数，页数=ceil(count/limit)）----
    void set_total(int total);            // 直接设置页数（Table 内部使用）
    void set_count(int count);            // 设置数据总数，页数 = ceil(count/limit)
    void set_limit(int limit);            // 每页条数（默认 10），设置后重算页数
    int limit() const { return limit_; }
    int count() const { return count_; }

    // ---- 布局 ----
    // 布局项顺序（不含 first/last；它们固定在首尾外侧）。默认 [Prev, Page, Next]
    void set_layout(std::vector<Item> layout) { 
        layout_ = std::move(layout);
        explicit_layout_ = true; mark_dirty(); }
    // 以字符串数组设置布局项（"count"/"prev"/"page"/"next"/"limit"/"refresh"/"skip"）
    void set_layout_str(const std::vector<std::string>& items);

    // ---- 首页/尾页 ----
    void set_show_first(bool on) {  show_first_ = on; mark_dirty(); }
    void set_show_last(bool on) {  show_last_ = on; mark_dirty(); }
    void set_first_text(const char* t) {  first_text_ = t ? t : ""; mark_dirty(); }
    void set_last_text(const char* t) {  last_text_ = t ? t : ""; mark_dirty(); }
    void set_prev_text(const char* t) {  prev_text_ = t ? t : ""; mark_dirty(); }
    void set_next_text(const char* t) {  next_text_ = t ? t : ""; mark_dirty(); }

    // ---- 主题 ----
    void set_theme_color(const Vec4& c) {  theme_color_ = c; mark_dirty(); }

    // ---- 每页条数选择项（limit 下拉）----
    void set_limits(std::vector<int> limits);

    // ---- 回调 ----
    void set_page(int page);  // 0 基
    int page() const { return page_; }
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }
    // layui jump：切页 / 切条数时回调 (curr, limit)，curr 为 1 基
    void set_on_jump(std::function<void(int, int)> cb) { on_jump_ = std::move(cb); }
    // refresh 按钮回调
    void set_on_refresh(std::function<void()> cb) { on_refresh_ = std::move(cb); }
    // limit 条数变化回调（1 基页参数为切后的页数）
    void set_on_limit_change(std::function<void(int)> cb) { on_limit_change_ = std::move(cb); }

    void set_font(Font* font);
    void set_theme(const Theme* theme);

    void on_layout() override;
    float preferred_height() const override { return bounds_.h; }
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    // 渲染当前页可见页码（0 基页码列表，-1 表示省略号）
    void visible_pages(std::vector<int>& out) const;
    // 跳转（含回调触发）；recalc 是否由外部重设页数后调用
    void jump_to(int page);
    // 重算总页数（count_ 有效时：ceil(count_/limit_)，否则保持 total_）
    void recompute_total();
    // 重建/同步 limit 下拉与 skip 输入子控件
    void sync_children();
    // 计算各布局项的水平偏移（返回下一项起点 x，同时 set 子控件 bounds）
    float place_items(float x);
    // 布局项是否启用
    bool has_item(Item it) const;

    struct ItemRect {
        Item type;
        float x, w;
    };
    // place_items 填充：按从左到右顺序记录每个可见项（含首页/尾页）的位置
    std::vector<ItemRect> item_rects_;
    // 当前布局 Page 项对应的页码（0 基，-1 为省略号，与 item_rects_ 中 Page 项一一对应）
    std::vector<int> page_nums_;
    // 上一页/下一页是否命中（由 on_mouse_down 用 item_rects_ 求交）
    bool hit_rect(Item type, float x) const;

    int total_ = 0;       // 总页数
    int page_ = 0;        // 当前页（0 基）
    int count_ = 0;       // 数据总数（0 = 未启用 set_count，直接用 total_）
    int limit_ = 10;      // 每页条数
    std::vector<int> limits_;         // limit 选择项（空则不显示下拉，直接用 limit_）
    std::vector<Item> layout_;        // 布局项顺序
    bool explicit_layout_ = false;    // 是否通过 set_layout/set_layout_str 显式指定（含 first/last 时按序放置）
    bool show_first_ = true;
    bool show_last_ = true;
    std::string first_text_ = "首页";
    std::string last_text_ = "尾页";
    std::string prev_text_ = "<";
    std::string next_text_ = ">";
    Vec4 theme_color_{0.09f, 0.73f, 0.67f, 1.0f};  // #16baaa layui 当前页主题色

    ComboBox* limit_box_ = nullptr;   // limit 下拉（layout 含 Limit 时创建）
    TextBox* skip_box_ = nullptr;     // skip 输入（layout 含 Skip 时创建）
    Button* skip_btn_ = nullptr;      // skip 确定按钮
    Button* refresh_btn_ = nullptr;   // refresh 图标按钮（layout 含 Refresh 时创建）

    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_change_;
    std::function<void(int, int)> on_jump_;
    std::function<void()> on_refresh_;
    std::function<void(int)> on_limit_change_;
};
}  // namespace ui
