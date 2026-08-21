#include "ui/ui/navmenu.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
constexpr float kGroupH = 40.f;   // layui-nav-tree 一级项高 40
constexpr float kItemH = 32.f;    // 子项
constexpr float kSepH = 18.f;     // 分隔线（<hr> 分段）占高
constexpr float kIndent = 24.f;
constexpr float kBarH = 3.f;      // 当前项顶部指示条
}  // namespace

NavMenu::NavMenu(float x, float y, float w, float h) : Widget(x, y, w, h), menu_width_(w > 0.f ? w : 200.f) {
    add_class("layui-nav-tree");
}

void NavMenu::add_group(const char* title) {
    groups_.push_back({title ? title : "", {}, false, 0.f, 0.f, 0.f});
}

void NavMenu::clear_groups() {
    groups_.clear();
    active_ = -1;
    sel_group_ = -1;
    sel_item_ = -1;
    hover_group_ = -1;
    hover_item_ = -1;
}

void NavMenu::add_item(const char* title, std::function<void()> cb) {
    if (groups_.empty()) add_group("menu");
    Group& g = groups_.back();
    g.items.push_back({title ? title : "", std::move(cb), 0.f, 0.f, false});
}

void NavMenu::add_separator() {
    if (groups_.empty()) add_group("menu");
    Group& g = groups_.back();
    g.items.push_back({"", {}, 0.f, 0.f, true});
}

void NavMenu::set_font(Font* font) { font_ = font; }

void NavMenu::set_theme(const Theme* theme) { theme_ = theme; }

void NavMenu::set_active(int group) {
    if (group < 0 || group >= (int)groups_.size()) return;
    active_ = group;
    for (int i = 0; i < (int)groups_.size(); ++i) groups_[i].expanded = (i == group);
}

void NavMenu::set_selected(int group, int item) {
    if (group < 0 || group >= (int)groups_.size()) return;
    if (item < 0 || item >= (int)groups_[group].items.size()) return;
    sel_group_ = group;
    sel_item_ = item;
    active_ = group;
}

void NavMenu::expand_all() {
    for (Group& g : groups_) g.expanded = true;
}

void NavMenu::on_layout() {
    layout_groups();
}

void NavMenu::layout_groups() {
    float y = bounds_.y;
    for (Group& g : groups_) {
        g.y0 = y;
        g.y1 = y + kGroupH;
        y += kGroupH;
        g.body_y0 = y;
        if (!g.expanded) continue;  // 折叠组不占子项空间（UI-047 回归保证）
        for (Item& it : g.items) {
            const float h = it.separator ? kSepH : kItemH;
            it.y0 = y;
            it.y1 = y + h;
            y += h;
        }
    }
}

float NavMenu::preferred_height() const {
    float h = 0.f;
    for (const Group& g : groups_) {
        h += kGroupH;
        if (g.expanded) {
            for (const Item& it : g.items) h += it.separator ? kSepH : kItemH;
        }
    }
    return h;
}

bool NavMenu::on_mouse_down(float x, float y, int button) {
    if (button != 0 || !contains(x, y)) return false;
    layout_groups();
    for (int g = 0; g < (int)groups_.size(); ++g) {
        Group& gr = groups_[g];
        // 一级项：点击激活 + 展开/折叠
        if (y >= gr.y0 && y <= gr.y1) {
            active_ = g;
            gr.expanded = !gr.expanded;
            return true;
        }
        // 子项：展开时点击执行
        if (gr.expanded) {
            for (int i = 0; i < (int)gr.items.size(); ++i) {
                Item& it = gr.items[i];
                if (y >= it.y0 && y <= it.y1) {
                    active_ = g;
                    sel_group_ = g;  // 点击子项自动选中（layui-nav-tree .layui-this）
                    sel_item_ = i;
                    if (it.cb) it.cb();
                    return true;
                }
            }
        }
    }
    return false;
}

bool NavMenu::on_mouse_move(float x, float y) {
    (void)x;
    hover_group_ = -1;
    hover_item_ = -1;
    layout_groups();
    for (int g = 0; g < (int)groups_.size(); ++g) {
        const Group& gr = groups_[g];
        if (y >= gr.y0 && y <= gr.y1) {
            hover_group_ = g;
            return false;
        }
        if (gr.expanded) {
            for (int i = 0; i < (int)gr.items.size(); ++i) {
                const Item& it = gr.items[i];
                if (y >= it.y0 && y <= it.y1) {
                    hover_group_ = g;
                    hover_item_ = i;
                    return false;
                }
            }
        }
    }
    return false;
}

void NavMenu::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    if (!font_) return;
    layout_groups();

    // 深色底 #2f363c — 使用 menu_width_ 而非 bounds_.w（防 ScrollBox 拉伸）
    renderer.draw_quad({bounds_.x, bounds_.y, menu_width_, bounds_.h}, Vec4{0.18f, 0.21f, 0.24f, 1.0f});

    for (int g = 0; g < (int)groups_.size(); ++g) {
        const Group& gr = groups_[g];
        const float title_ty = gr.y0 + (kGroupH - font_->line_height()) * 0.5f;        // 激活组顶部 3px #16b777 指示条
        if (g == active_)
            renderer.draw_quad({bounds_.x, gr.y0, menu_width_, kBarH}, t->success);
        // 一级项 hover 浅底
        if (g == hover_group_ && hover_item_ < 0)
            renderer.draw_quad({bounds_.x, gr.y0, menu_width_, kGroupH}, Vec4{0.24f, 0.28f, 0.32f, 1.0f});
        // 箭头：展开 'v' / 折叠 '>'
        font_->draw_text(renderer, gr.expanded ? "v" : ">", bounds_.x + 12.f, title_ty,
                         Vec4{1, 1, 1, 1});
        font_->draw_text(renderer, gr.title.c_str(), bounds_.x + kIndent, title_ty,
                         Vec4{1, 1, 1, 1});

        if (gr.expanded) {
            for (int i = 0; i < (int)gr.items.size(); ++i) {
                const Item& it = gr.items[i];
                if (it.separator) {
                    // 分隔线：灰线居中一条，距两侧留白（镜像 <hr>）
                    const float cy = (it.y0 + it.y1) * 0.5f;
                    renderer.draw_quad({bounds_.x + kIndent, cy - 0.5f, menu_width_ - kIndent * 2.f, 1.f},
                                       Vec4{0.4f, 0.44f, 0.48f, 1.0f});
                    continue;
                }
                const float item_ty = it.y0 + (kItemH - font_->line_height()) * 0.5f;
                const bool selected = (g == sel_group_ && i == sel_item_);
                if (selected) {
                    // 选中项（layui `.layui-this`）：绿底 #16b777 + 白字
                    renderer.draw_quad({bounds_.x, it.y0, menu_width_, kItemH}, t->success);
                } else if (g == hover_group_ && i == hover_item_) {
                    // 子项 hover 浅底
                    renderer.draw_quad({bounds_.x, it.y0, menu_width_, kItemH},
                                       Vec4{0.24f, 0.28f, 0.32f, 1.0f});
                }
                font_->draw_text(renderer, it.text.c_str(), bounds_.x + kIndent + 8.f, item_ty,
                                 selected ? Vec4{1, 1, 1, 1} : Vec4{0.9f, 0.9f, 0.9f, 1.0f});
            }
        }
    }
}

}  // namespace ui
