#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 节点操作图标类型（layui tree edit: ['add','update','del']）
enum class TreeEdit { Add, Update, Del };

// 节点规格（layui tree 数据节点：title/id/field/href/disabled/fixed/checked/spread）
struct TreeNodeSpec {
    std::string title;
    int id = 0;
    std::string field;
    std::string href;    // isJump 时点击跳转目标
    bool disabled = false;
    bool fixed = false;  // 固定节点（不允许删除/移动）
    bool checked = false;
    bool spread = false; // 默认展开
};

// 树形控件（M17c，layui-tree 克隆）：节点含子节点，可展开/折叠、复选框、手风琴、
// 仅图标控制、跳转、操作图标。类名：layui-tree（默认）。
class Tree : public Widget {
public:
    Tree(float x, float y, float w, float h);

    // 追加节点：parent 为父节点下标（-1 表示根级）。返回新节点下标。
    int add_node(const char* text, int parent = -1);
    int add_node(const TreeNodeSpec& spec, int parent = -1);

    // 节点属性
    void set_node_text(int index, const char* text);
    void set_node_id(int index, int id);
    void set_node_href(int index, const char* href);
    void set_node_disabled(int index, bool disabled);
    void set_node_checked(int index, bool checked);
    // 展开/折叠指定节点（含子孙显隐）；accordion 时自动收起同级兄弟
    void set_expanded(int index, bool expanded);
    bool is_expanded(int index) const;
    void expand_all(bool expanded);

    // —— 组件外观/行为开关（layui tree 选项）——
    void set_show_checkbox(bool on) { show_checkbox_ = on; }
    void set_show_line(bool on) { show_line_ = on; }
    void set_accordion(bool on) { accordion_ = on; }
    void set_only_icon_control(bool on) { only_icon_ = on; }
    void set_is_jump(bool on) { is_jump_ = on; }
    // 开启节点操作图标（add/update/del）
    void set_editable(const std::vector<TreeEdit>& actions);

    // —— 复选框查询/勾选（layui getChecked/setChecked）——
    // 返回当前勾选的节点下标（含半选父级？否——仅完全勾选的叶子/分支）
    std::vector<int> get_checked() const;
    void set_checked(int index, bool checked);  // 含子孙传播 + 祖先重算
    void check_all(bool on);

    void set_font(Font* font);
    void set_theme(const Theme* theme);
    // 点击节点回调（index 为节点下标）
    void set_on_click(std::function<void(int)> cb) { on_click_ = std::move(cb); }
    // 操作图标回调（index 节点下标，act 操作类型）
    void set_on_edit(std::function<void(int, TreeEdit)> cb) { on_edit_ = std::move(cb); }

    void on_layout() override;
    float preferred_height() const override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    void on_draw(Renderer& renderer) override;

private:
    struct Node {
        std::string title;
        int id = 0;
        std::string field;
        std::string href;
        bool disabled = false;
        bool fixed = false;
        bool checked = false;
        bool expanded = false;
        int parent = -1;
        std::vector<int> children;
    };

    void layout_nodes() const;      // 计算可见节点顺序与 y 位置
    int hit_node(float y) const;    // 命中 y 所在可见序号（-1 无）
    bool node_has_children(int idx) const;
    void set_checked_recursive(int idx, bool checked);
    void update_ancestors(int idx);  // 由子节点勾选态重算祖先行（半选）
    // 节点是否半选（有子勾选但未全选）——动态计算
    bool is_indeterminate(int idx) const;
    // 节点深度表（绘制缩进用）
    std::vector<int> depth_of_nodes() const;

    mutable std::vector<Node> nodes_;
    mutable std::vector<int> vis_;        // 可见节点下标（DFS 顺序）
    mutable std::vector<float> node_y_;   // 可见节点 y 中心（与 vis_ 一一对应）
    mutable std::vector<int> depth_of_;   // 节点深度（与 nodes_ 一一对应）
    mutable int hover_node_ = -1;

    bool show_checkbox_ = false;
    bool show_line_ = false;   // 连接线（layui 默认 on，test13 关）
    bool accordion_ = false;
    bool only_icon_ = false;
    bool is_jump_ = false;
    bool editable_ = false;
    std::vector<TreeEdit> edit_actions_;

    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_click_;
    std::function<void(int, TreeEdit)> on_edit_;
};
}
