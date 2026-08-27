#include "ui/ui/grid.h"

#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <vector>

namespace ui {

namespace {

// 断点档位：0=xs 1=sm 2=md 3=lg
struct Span {
    int span = 0;
    int offset = 0;
};

// 从子控件类名解析 layui-col-{xs,sm,md,lg}{n} / layui-col-{x}-offset{n}，
// 返回各断点的 span/offset；span 为 0 表示该断点未定义。
// 未匹配任何 layui-col-* 类返回 has_any=false。
bool parse_breakpoints(const Widget* w, Span out[4]) {
    bool any = false;
    for (const std::string& cls : w->classes()) {
        const char* c = cls.c_str();
        int value = 0;
        if (std::sscanf(c, "layui-col-xs%d", &value) == 1 && value >= 0) {
            out[0].span = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-sm%d", &value) == 1 && value >= 0) {
            out[1].span = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-md%d", &value) == 1 && value >= 0) {
            out[2].span = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-lg%d", &value) == 1 && value >= 0) {
            out[3].span = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-xs-offset%d", &value) == 1 && value >= 0) {
            out[0].offset = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-sm-offset%d", &value) == 1 && value >= 0) {
            out[1].offset = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-md-offset%d", &value) == 1 && value >= 0) {
            out[2].offset = value;
            any = true;
        } else if (std::sscanf(c, "layui-col-lg-offset%d", &value) == 1 && value >= 0) {
            out[3].offset = value;
            any = true;
        }
    }
    return any;
}

// 按当前断点 bp 取「bp 及以下最高已定义档」；无则整行铺满（span=12）。
Span resolve_span(const Widget* w, int bp) {
    Span at[4] = {};
    if (!parse_breakpoints(w, at)) return {};  // 无栅格类
    for (int b = bp; b >= 0; --b) {
        if (at[b].span > 0) {
            Span s = at[b];
            s.span = std::min(s.span, 12);
            s.offset = std::max(0, std::min(s.offset, 12));
            return s;
        }
    }
    // 只在高断点定义了类（如仅 md6），当前断点更低 → 未启用，整行铺满
    return {12, 0};
}

// Grid 自身类 layui-col-space{n} → 列间距 n px；未设置返回 0
int col_space(const Widget* g) {
    for (const std::string& cls : g->classes()) {
        int v = 0;
        if (std::sscanf(cls.c_str(), "layui-col-space%d", &v) == 1 && v >= 0) return v;
    }
    return 0;
}

}  // namespace

Grid::Grid(float x, float y, float w, float h, int cols) : Widget(x, y, w, h) {
    set_cols(cols);
}

void Grid::set_cols(int cols) {
    mark_dirty();
        cols_ = std::max(1, cols);
}

int Grid::breakpoint() const {
    if (breakpoint_ >= 0) return std::min(3, breakpoint_);
    const float w = bounds_.w;
    if (w >= 1200.f) return 3;  // lg
    if (w >= 992.f) return 2;   // md
    if (w >= 768.f) return 1;   // sm
    return 0;                   // xs
}

bool Grid::span_mode() const {
    for (const Widget* child : children_) {
        for (const std::string& cls : child->classes()) {
            if (cls.rfind("layui-col-", 0) == 0) return true;
        }
    }
    return false;
}

// 行分组：返回每行 {起始下标, 结束下标, 行高}。嵌套 Grid 子控件行高取其 preferred_height。
std::vector<std::array<int, 3>> group_rows(const std::vector<Widget*>& children, bool span,
                                           int span_cols, int bp, float row_h) {
    std::vector<std::array<int, 3>> rows;
    int start = -1;
    int col_used = 0;
    float cur_h = row_h;
    for (int i = 0; i < (int)children.size(); ++i) {
        Widget* child = children[i];
        if (!child->visible) continue;
        int span_v = 1, offset = 0;
        if (span) {
            const Span s = resolve_span(child, bp);
            span_v = std::max(1, s.span);
            offset = s.offset;
        }
        if (col_used + span_v + offset > span_cols && col_used > 0) {
            rows.push_back({start, i, (int)cur_h});
            start = i;
            col_used = 0;
            cur_h = row_h;
        }
        if (start < 0) start = i;
        if (span && dynamic_cast<Grid*>(child) != nullptr)
            cur_h = std::max(cur_h, child->preferred_height());
        col_used += span_v + offset;
    }
    if (start >= 0) rows.push_back({start, (int)children.size(), (int)cur_h});
    return rows;
}

// 行主序放置子控件；返回行数。span 模式按 12 列解析断点/offset，其余按 cols_ 等宽。
int Grid::place_children() {
    if (children_.empty()) return 0;
    const bool span = span_mode();
    const int bp = breakpoint();
    const int space = col_space(this);
    const float avail = bounds_.w - 2.f * padding_;
    const float avail_ext = avail + (float)space;  // layui 负 margin 使行宽扩展

    if (span) {
        const auto rows = group_rows(children_, true, span_cols_, bp, row_h_);
        float y = bounds_.y + padding_;
        for (const auto& r : rows) {
            const int start = r[0], end = r[1];
            const float row_h = (float)r[2];
            int col_used = 0;
            for (int i = start; i < end; ++i) {
                Widget* child = children_[i];
                if (!child->visible) continue;
                const Span s = resolve_span(child, bp);
                const int span_v = std::max(1, s.span);
                const int offset = s.offset;
                if (col_used + span_v + offset > span_cols_ && col_used > 0) {
                    y += row_h + spacing_;
                    col_used = 0;
                }
                // 列宽/位置按扩展宽比例；col-space 时列宽扣 n、列间留 n px 间隙，首末列贴合行边
                // （layui 负 margin 双向撑宽语义，见 layui.css .layui-col-space*）
                const float cell_w = avail_ext * (float)span_v / (float)span_cols_;
                const float used_w = avail_ext * (float)col_used / (float)span_cols_;
                float cx = bounds_.x + padding_ + used_w;
                if (offset > 0) cx += avail_ext * (float)offset / (float)span_cols_;
                const float cy = y;
                if (space > 0) {
                    child->set_bounds(cx, cy, cell_w - (float)space, row_h);
                } else {
                    child->set_bounds(cx, cy, cell_w, row_h);
                }
                col_used += span_v + offset;
            }
            y += row_h + spacing_;
        }
        return (int)rows.size();
    }
    // 等宽模式：按 cols_ 换行
    const int n = (int)children_.size();
    const int rows = (n + cols_ - 1) / cols_;
    const float cell_w = (avail - (cols_ - 1) * spacing_) / (float)cols_;
    int i = 0;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const int col = i % cols_;
        const int row = i / cols_;
        const float x = bounds_.x + padding_ + col * (cell_w + spacing_);
        const float y = bounds_.y + padding_ + row * (row_h_ + spacing_);
        child->set_bounds(x, y, cell_w, row_h_);
        ++i;
    }
    return rows;
}

void Grid::on_layout() {
    place_children();
}

float Grid::preferred_height() const {
    if (children_.empty()) return 2.f * padding_;
    if (span_mode()) {
        const auto rows = group_rows(children_, true, span_cols_, breakpoint(), row_h_);
        float h = 2.f * padding_;
        for (size_t i = 0; i < rows.size(); ++i) {
            h += (float)rows[i][2];
            if (i + 1 < rows.size()) h += spacing_;
        }
        return h;
    }
    const int n = (int)children_.size();
    const int rows = (n + cols_ - 1) / cols_;
    return 2.f * padding_ + rows * row_h_ + std::max(0, rows - 1) * spacing_;
}

void Grid::on_draw(Renderer& renderer) {
    // layui 行容器无背景（layui-row 类，UI-039）；其余场景沿用等宽模式默认底色
    if (has_class("layui-row")) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    renderer.draw_rounded_rect(bounds_, t->radius, t->button_normal);
}

}  // namespace ui
