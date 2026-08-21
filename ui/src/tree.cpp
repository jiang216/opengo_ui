#include "ui/ui/tree.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace ui {

namespace {
constexpr float kRowH = 26.f;
constexpr float kIndent = 20.f;
constexpr float kFoldW = 20.f;   // 折叠箭头区宽度
constexpr float kCBSize = 14.f;  // 复选框尺寸
constexpr float kEditW = 18.f;   // 操作图标宽度/间距
}

Tree::Tree(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-tree");
}

int Tree::add_node(const char* text, int parent) {
    TreeNodeSpec spec;
    spec.title = text ? text : "";
    return add_node(spec, parent);
}

int Tree::add_node(const TreeNodeSpec& spec, int parent) {
    const int index = (int)nodes_.size();
    Node n;
    n.title = spec.title;
    n.id = spec.id;
    n.field = spec.field;
    n.href = spec.href;
    n.disabled = spec.disabled;
    n.fixed = spec.fixed;
    n.checked = spec.checked;
    n.expanded = spec.spread;
    n.parent = parent;
    if (parent >= 0 && parent < (int)nodes_.size())
        nodes_[parent].children.push_back(index);
    nodes_.push_back(std::move(n));
    return index;
}

void Tree::set_node_text(int index, const char* text) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    nodes_[index].title = text ? text : "";
}

void Tree::set_node_id(int index, int id) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    nodes_[index].id = id;
}

void Tree::set_node_href(int index, const char* href) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    nodes_[index].href = href ? href : "";
}

void Tree::set_node_disabled(int index, bool disabled) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    nodes_[index].disabled = disabled;
}

void Tree::set_node_checked(int index, bool checked) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    nodes_[index].checked = checked;
}

void Tree::set_expanded(int index, bool expanded) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    if (expanded && accordion_) {
        // 手风琴：收起同父级的其他子节点
        const int p = nodes_[index].parent;
        for (Node& n : nodes_) n.expanded = (n.parent == p) ? false : n.expanded;
    }
    nodes_[index].expanded = expanded;
}

bool Tree::is_expanded(int index) const {
    if (index < 0 || index >= (int)nodes_.size()) return false;
    return nodes_[index].expanded;
}

void Tree::expand_all(bool expanded) {
    for (Node& n : nodes_) n.expanded = expanded;
}

void Tree::set_font(Font* font) { font_ = font; }

void Tree::set_theme(const Theme* theme) { theme_ = theme; }

void Tree::set_editable(const std::vector<TreeEdit>& actions) {
    edit_actions_ = actions;
    editable_ = !edit_actions_.empty();
}

bool Tree::node_has_children(int idx) const {
    if (idx < 0 || idx >= (int)nodes_.size()) return false;
    return !nodes_[idx].children.empty();
}

void Tree::set_checked_recursive(int idx, bool checked) {
    nodes_[idx].checked = checked;
    for (int c : nodes_[idx].children) set_checked_recursive(c, checked);
}

// 由子节点勾选态重算祖先行：全选=✓，部分=半选，全不选=✗
void Tree::update_ancestors(int idx) {
    if (idx < 0) return;
    const int p = nodes_[idx].parent;
    if (p < 0) return;
    int checked_n = 0;
    for (int c : nodes_[p].children) if (nodes_[c].checked) ++checked_n;
    nodes_[p].checked = (checked_n == (int)nodes_[p].children.size());
    update_ancestors(p);  // 继续向上
}

// 节点半选：有子勾选但未全选
bool Tree::is_indeterminate(int idx) const {
    if (idx < 0 || idx >= (int)nodes_.size()) return false;
    const Node& n = nodes_[idx];
    int checked_n = 0;
    for (int c : n.children) if (nodes_[c].checked) ++checked_n;
    return checked_n > 0 && checked_n < (int)n.children.size();
}

std::vector<int> Tree::get_checked() const {
    std::vector<int> out;
    for (size_t i = 0; i < nodes_.size(); ++i)
        if (nodes_[i].checked && !is_indeterminate((int)i)) out.push_back((int)i);
    return out;
}

void Tree::set_checked(int index, bool checked) {
    if (index < 0 || index >= (int)nodes_.size()) return;
    set_checked_recursive(index, checked);
    update_ancestors(index);
}

void Tree::check_all(bool on) {
    std::function<void(int)> rec = [&](int idx) {
        nodes_[idx].checked = on;
        for (int c : nodes_[idx].children) rec(c);
    };
    for (size_t i = 0; i < nodes_.size(); ++i)
        if (nodes_[i].parent < 0) rec((int)i);
}

// 重算可见节点：vis_ 存节点下标，node_y_ 存对应 y 中心，depth_of_ 存深度
void Tree::layout_nodes() const {
    vis_.clear();
    node_y_.clear();
    depth_of_.assign(nodes_.size(), 0);
    float y = bounds_.y + kRowH * 0.5f;
    std::function<void(int, int)> visit = [&](int idx, int depth) {
        vis_.push_back(idx);
        depth_of_[idx] = depth;
        node_y_.push_back(y);
        y += kRowH;
        if (!nodes_[idx].expanded) return;
        for (int c : nodes_[idx].children) visit(c, depth + 1);
    };
    for (int i = 0; i < (int)nodes_.size(); ++i) {
        if (nodes_[i].parent < 0) visit(i, 0);
    }
}

void Tree::on_layout() {
    layout_nodes();
}

float Tree::preferred_height() const {
    layout_nodes();
    return std::max(bounds_.h, (float)vis_.size() * kRowH);
}

int Tree::hit_node(float y) const {
    for (int i = 0; i < (int)node_y_.size(); ++i) {
        if (y >= node_y_[i] - kRowH * 0.5f && y <= node_y_[i] + kRowH * 0.5f)
            return i;
    }
    return -1;
}

bool Tree::on_mouse_down(float x, float y, int button) {
    if (button != 0 || !contains(x, y)) return false;
    layout_nodes();
    const int hit = hit_node(y);
    if (hit < 0 || hit >= (int)vis_.size()) return false;
    const int idx = vis_[hit];
    if (nodes_[idx].disabled) return false;

    const float indent = bounds_.x + depth_of_[idx] * kIndent;
    // 复选框区：切勾选（含子孙传播 + 祖先重算）
    float cb_x = indent;
    if (show_checkbox_) {
        const Rect cb{cb_x + 4.f, y - kCBSize * 0.5f, kCBSize, kCBSize};
        // 半选状态下点击 = 全选
        const bool to_checked = !(nodes_[idx].checked || is_indeterminate(idx));
        if (x >= cb.x && x <= cb.x + cb.w && y >= cb.y && y <= cb.y + cb.h) {
            set_checked_recursive(idx, to_checked);
            update_ancestors(idx);
            return true;
        }
    }
    // 折叠箭头区：切换展开/折叠（onlyIconControl 仅此处可收缩）
    const float fold_x = cb_x + (show_checkbox_ ? kCBSize + 4.f : 0.f);
    if (x >= fold_x && x < fold_x + kFoldW) {
        if (node_has_children(idx)) {
            set_expanded(idx, !nodes_[idx].expanded);
            return true;
        }
    }
    // 默认点击行可收缩（非 onlyIconControl 且含子节点）
    if (!only_icon_ && node_has_children(idx)) {
        if (x >= fold_x + kFoldW) {
            set_expanded(idx, !nodes_[idx].expanded);
            return true;
        }
    }
    // 操作图标区（test9）
    if (editable_) {
        float ex = bounds_.x + bounds_.w - (float)edit_actions_.size() * kEditW - 6.f;
        for (size_t i = 0; i < edit_actions_.size(); ++i) {
            const Rect er{ex + i * kEditW, y - kRowH * 0.5f, kEditW, kRowH};
            if (x >= er.x && x <= er.x + er.w) {
                if (on_edit_) on_edit_(idx, edit_actions_[i]);
                return true;
            }
        }
    }
    // 点击节点：回调（isJump 时 href 非空给跳转语义）
    if (on_click_) on_click_(idx);
    return true;
}

bool Tree::on_mouse_move(float x, float y) {
    (void)x;
    layout_nodes();
    const int hit = hit_node(y);
    if (hit >= 0 && hit < (int)vis_.size())
        hover_node_ = hit;
    else
        hover_node_ = -1;
    return false;
}

void Tree::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    if (!font_) return;

    const int n = (int)vis_.size();
    for (int v = 0; v < n; ++v) {
        const int idx = vis_[v];
        const Node& node = nodes_[idx];
        const float y = node_y_[v];
        const float ty = y - font_->line_height() * 0.5f;
        const float row_y0 = y - kRowH * 0.5f;
        const float indent = bounds_.x + depth_of_[idx] * kIndent;

        // hover 背景 #eee（layui-tree-entry:hover）
        if (v == hover_node_) renderer.draw_quad({bounds_.x, row_y0, bounds_.w, kRowH}, Vec4{0.93f, 0.93f, 0.93f, 1.0f});

        // 复选框（showCheckbox）
        float cx = indent;
        if (show_checkbox_) {
            const Rect cb{cx + 4.f, y - kCBSize * 0.5f, kCBSize, kCBSize};
            const bool checked = node.checked;
            const bool half = is_indeterminate(idx);
            if (checked || half) {
                renderer.draw_rounded_rect(cb, 3.f, t->accent);
                const Rect inner{cb.x + 2.f, cb.y + 2.f, cb.w - 4.f, cb.h - 4.f};
                renderer.draw_rounded_rect(inner, 2.f, half ? Vec4{0.09f, 0.73f, 0.67f, 1.f} : t->accent);
                if (font_) font_->draw_text(renderer, half ? "-" : "\xe2\x9c\x93", cb.x + 2.f, cb.y + 1.f, Vec4{1, 1, 1, 1});
            } else {
                renderer.draw_rounded_rect(cb, 3.f, Vec4{0.82f, 0.82f, 0.82f, 1.f});
                const Rect inner{cb.x + 2.f, cb.y + 2.f, cb.w - 4.f, cb.h - 4.f};
                renderer.draw_rounded_rect(inner, 2.f, Vec4{1, 1, 1, 1});
            }
            cx += kCBSize + 4.f;
        }

        // 折叠箭头：无子节点画空心三角（#c0c4cc）；有子节点展开朝下、折叠朝右
        if (!node.children.empty()) {
            const float ax = cx + kFoldW * 0.5f;
            const float ay = y;
            const Vec2 right[3] = {{ax - 3, ay - 4}, {ax - 3, ay + 4}, {ax + 3, ay}};
            const Vec2 down[3] = {{ax - 4, ay - 3}, {ax + 4, ay - 3}, {ax, ay + 3}};
            renderer.draw_polygon(node.expanded ? down : right, 3, Vec4{0.75f, 0.77f, 0.8f, 1.0f});
        }

        // 连接线（showLine，layui-tree-line）
        if (show_line_ && depth_of_[idx] > 0) {
            const float lx = indent - 9.f;
            renderer.draw_quad({lx, row_y0, 1.f, kRowH}, Vec4{0.75f, 0.77f, 0.8f, 0.6f});
            renderer.draw_quad({lx, y, 17.f, 1.f}, Vec4{0.75f, 0.77f, 0.8f, 0.6f});
        }

        // 节点文本 #555（禁用置灰）
        const float tx = cx + kFoldW;
        font_->draw_text(renderer, node.title.c_str(), tx, ty,
                         node.disabled ? Vec4{0.75f, 0.75f, 0.75f, 1.f} : Vec4{0.33f, 0.33f, 0.33f, 1.f});

        // 操作图标（editable_：add ⊕ / update ✎ / del ✕）
        if (editable_) {
            float ex = bounds_.x + bounds_.w - (float)edit_actions_.size() * kEditW - 6.f;
            for (size_t i = 0; i < edit_actions_.size(); ++i) {
                const Vec2 c{ex + i * kEditW + kEditW * 0.5f, y};
                const Vec4 icon_col{0.65f, 0.65f, 0.65f, 1.f};
                switch (edit_actions_[i]) {
                    case TreeEdit::Add:
                        renderer.draw_quad({c.x - 4.f, c.y - 1.f, 8.f, 2.f}, icon_col);
                        renderer.draw_quad({c.x - 1.f, c.y - 4.f, 2.f, 8.f}, icon_col);
                        break;
                    case TreeEdit::Update:
                        renderer.draw_quad({c.x - 3.f, c.y - 3.f, 6.f, 6.f}, icon_col);
                        renderer.draw_quad({c.x - 4.f, c.y + 2.f, 8.f, 2.f}, icon_col);
                        break;
                    case TreeEdit::Del:
                        renderer.draw_quad({c.x - 4.f, c.y + 1.f, 8.f, 2.f}, Vec4{0.9f, 0.5f, 0.5f, 1.f});
                        break;
                }
            }
        }
    }
}

}  // namespace ui