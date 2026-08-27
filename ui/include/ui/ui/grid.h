#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Renderer;
struct Theme;

// 网格布局（M12）：子控件按行主序填入 cols 列的等宽网格，行高固定自动换行。
// layui 栅格扩展（M18/UI-029）：子控件带 layui-col-xs/sm/md/lg{n}（+offset）类时，
// 按 12 列基准解析 span/offset（克隆 layui.css .layui-row/.layui-col-*）。
// 断点语义：xs 为无条件基础档，sm(>=768)/md(>=992)/lg(>=1200) 依次覆盖；
// 生效档取「当前断点及以下最高已定义档」。Grid 自身带 layui-col-space{n} 类时列间留 n px 间隙。
class Grid : public Widget {
public:
    Grid(float x, float y, float w, float h, int cols);

    void set_cols(int cols);
    void set_row_height(float h) {  row_h_ = h; mark_dirty(); }
    void set_spacing(float s) {  spacing_ = s; mark_dirty(); }
    void set_padding(float p) {  padding_ = p; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    // layui 栅格：12 列基准下解析子控件 layui-col-* 类
    void set_span_cols(int cols) {  span_cols_ = std::max(1, cols); mark_dirty(); }
    // 断点：0=xs 1=sm 2=md 3=lg；<0 时按自身宽度自动推断（内容区宽度）
    void set_breakpoint(int bp) {  breakpoint_ = bp; mark_dirty(); }

    void on_layout() override;
    float preferred_height() const override;
    void on_draw(Renderer& renderer) override;

private:
    // 当前断点（自动推断或显式设置）
    int breakpoint() const;
    // 是否启用 layui span 模式（任一子控件带 layui-col-* 类）
    bool span_mode() const;
    // 行主序放置所有子控件，返回行数
    int place_children();

    int cols_ = 2;
    int span_cols_ = 12;
    float row_h_ = 40.f;
    float spacing_ = 8.f;
    float padding_ = 8.f;
    int breakpoint_ = -1;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
