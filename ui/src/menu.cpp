#include "ui/ui/menu.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

namespace {
constexpr float kItemH = 40.f;
constexpr float kItemHLg = 48.f;
constexpr float kItemPad = 12.f;
constexpr float kGroupPad = 10.f;
constexpr float kIndent = 16.f;
constexpr float kDividerH = 1.f;
constexpr float kBadgePad = 4.f;
constexpr float kBadgeTextPad = 8.f;
constexpr float kBadgeDotSize = 6.f;
constexpr float kArrowSize = 5.f;
constexpr float kSubPanelPad = 8.f;
constexpr float kSubItemH = 36.f;
}  // namespace

Menu::Menu(float x, float y, float w, float h) : Widget(x, y, w, h) {
    hitbox_ = new HitBox(this);
    hitbox_->set_visible(false);
    add_child(hitbox_);
}

void Menu::add_item(const char* title, std::function<void()> cb) {
    items_.push_back({title ? title : "", "", std::move(cb), Item::Plain, current_level_,
                      false, true, false, false, false, {}, 0.f, 0.f, -1, true});
}

void Menu::add_item_sub(const char* title, const char* subtitle, std::function<void()> cb) {
    items_.push_back({title ? title : "", subtitle ? subtitle : "", std::move(cb), Item::Plain,
                      current_level_, false, true, false, false, false, {}, 0.f, 0.f, -1, true});
}

void Menu::add_divider() {
    items_.push_back({"", "", nullptr, Item::Divider, current_level_,
                      false, false, false, false, false, {}, 0.f, 0.f, -1, true});
}

void Menu::add_group(const char* title, bool allow_spread) {
    mark_dirty();
        // Group header is at current_level_; increment for items inside
    const int group_level = current_level_;
    items_.push_back({title ? title : "", "", nullptr, Item::Group, group_level,
                      false, true, allow_spread, false, false, {}, 0.f, 0.f, -1, true});
    ++current_level_;
}

void Menu::add_parent(const char* title) {
    // Parent item is at current_level_; increment for sub-panel items
    const int parent_level = current_level_;
    items_.push_back({title ? title : "", "", nullptr, Item::Parent, parent_level,
                      false, true, false, false, false, {}, 0.f, 0.f, -1, true});
    ++current_level_;
}

void Menu::end_group() {
    if (current_level_ > 0) --current_level_;
}

void Menu::clear() {
    mark_dirty();
        items_.clear();
    current_level_ = 0;
    hover_ = nullptr;
    active_hover_ = nullptr;
    open_parent_ = -1;
    sub_panel_rect_ = {};
}

void Menu::set_checked() {
    mark_dirty();
        if (!items_.empty()) items_.back().checked = true;
}

// UI-198：按插入序改项文本（含分隔线/父级项），无变化不标脏（refresh_controls 每帧调用零开销）
void Menu::set_item_text(int index, const char* title) {
    if (index < 0 || index >= (int)items_.size()) return;
    const std::string t = title ? title : "";
    if (items_[index].text == t) return;
    items_[index].text = t;
    mark_dirty();
}

// UI-198：按插入序改项选中态（绿字回显当前值，如倍速/音量/亮度当前档）
void Menu::set_item_checked(int index, bool checked) {
    if (index < 0 || index >= (int)items_.size()) return;
    if (items_[index].checked == checked) return;
    items_[index].checked = checked;
    mark_dirty();
}

void Menu::set_selected(int group_index, int item_index) {
    mark_dirty();
        // Clear all checked
    for (auto& it : items_) it.checked = false;
    // Find group_index-th Group item, then item_index-th Plain inside it
    int gi = 0;
    bool in_target = false;
    int ii = 0;
    for (auto& it : items_) {
        if (it.type == Item::Group) {
            in_target = (gi == group_index);
            ++gi;
            ii = 0;
            continue;
        }
        if (!in_target || it.type != Item::Plain) continue;
        if (ii == item_index) {
            it.checked = true;
            return;
        }
        ++ii;
    }
}

void Menu::set_badge(const char* text) {
    mark_dirty();
        if (items_.empty()) return;
    items_.back().has_badge = true;
    items_.back().badge_text = text ? text : "";
}

void Menu::set_dot_badge() {
    mark_dirty();
        if (!items_.empty()) items_.back().has_dot = true;
}

void Menu::set_large(bool large) { mark_dirty();
        large_ = large; }

void Menu::set_font(Font* f) { mark_dirty();
        font_ = f; }

void Menu::set_theme(const Theme* t) { mark_dirty();
        theme_ = t; }

void Menu::set_style_overrides(const Vec4& bg, const Vec4& border, const Vec4& hover,
                               const Vec4& text) {
    mark_dirty();
        style_ov_ = true;
    ov_bg_ = bg;
    ov_border_ = border;
    ov_hover_ = hover;
    ov_text_ = text;
}

void Menu::set_on_item_click(std::function<void(const char*)> cb) {
    on_item_click_ = std::move(cb);
}

void Menu::set_on_group_click(std::function<void(const char*)> cb) {
    on_group_click_ = std::move(cb);
}

float Menu::preferred_height() const {
    compute_visibility();
    float h = 0.f;
    for (const auto& it : items_) {
        if (!it.visible) continue;
        h += (it.type == Item::Divider) ? kDividerH : (large_ ? kItemHLg : kItemH);
    }
    return h;
}

void Menu::on_layout() { layout_items(); }

void Menu::layout_items() {
    compute_visibility();
    float y = 0.f;
    const float ih = large_ ? kItemHLg : kItemH;
    for (auto& it : items_) {
        if (!it.visible) {
            it.y0 = it.y1 = bounds_.y + y;
            continue;
        }
        it.y0 = bounds_.y + y;
        if (it.type == Item::Divider) {
            y += kDividerH;
        } else {
            y += ih;
        }
        it.y1 = bounds_.y + y;
    }
}

void Menu::compute_visibility() const {
    // Build a map: group_index → Group Item*
    std::vector<const Item*> group_ptrs;
    for (auto& it : items_) {
        if (it.type == Item::Group) {
            it.group_index = (int)group_ptrs.size();
            group_ptrs.push_back(&it);
        }
    }
    // Assign group_index to non-group items: they belong to the most recently opened
    // (and not yet closed) group at their level
    int open_group_at_level[64] = {};
    for (int i = 0; i < 64; ++i) open_group_at_level[i] = -1;
    for (auto& it : items_) {
        if (it.type == Item::Group) {
            open_group_at_level[it.level] = it.group_index;
            it.visible = true;
        } else if (it.type == Item::Divider) {
            it.visible = true;
        } else {
            if (it.level > 0) {
                it.group_index = open_group_at_level[it.level - 1];
            }
            it.visible = true;
        }
    }
    // Hide items inside collapsed groups
    for (const Item* grp : group_ptrs) {
        if (grp->spread) continue;
        bool inside = false;
        for (auto& it : items_) {
            if (&it == grp) { inside = true; continue; }
            if (!inside) continue;
            if (it.type == Item::Group && it.level <= grp->level) break;
            if (it.type == Item::Divider) continue;
            it.visible = false;
        }
    }
    // UI-198：Parent 子区折叠——父项（add_parent）的子项不在地一层平铺，仅随
    // 「悬停自动展开」的右侧子面板（draw_sub_panel）显示。边界条件与 draw_sub_panel
    // 的收集一致：从父项后起到「分隔线 / 层级 ≤ 父项」处为止，其余全部置不可见。
    for (size_t pi = 0; pi < items_.size(); ++pi) {
        if (items_[pi].type != Item::Parent) continue;
        for (size_t j = pi + 1; j < items_.size(); ++j) {
            const Item& it = items_[j];
            if (it.type == Item::Divider) break;
            if (it.level <= items_[pi].level) break;
            items_[j].visible = false;
        }
    }
}

void Menu::draw_item(Renderer& renderer, Item& it, int /*index*/) {
    if (!font_ || !it.visible) return;
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    const Vec4 panel = style_ov_ ? ov_bg_ : t->panel;
    const Vec4 border = style_ov_ ? ov_border_ : t->border;
    const Vec4 text = style_ov_ ? ov_text_ : t->text;
    const Vec4 text2 = style_ov_ ? ov_text_ : t->text_secondary;
    const Vec4 muted = style_ov_ ? ov_text_ : t->text_muted;
    const Vec4 hover = style_ov_ ? ov_hover_ : Vec4{0.96f, 0.96f, 0.96f, 1.f};
    const float ih = large_ ? kItemHLg : kItemH;
    const float line_h = font_->line_height();
    const float indent = it.level * kIndent;

    if (it.type == Item::Divider) {
        const Rect line = {bounds_.x + kItemPad, it.y0, bounds_.w - kItemPad * 2, kDividerH};
        renderer.draw_quad(line, border);
        return;
    }

    const Rect row = {bounds_.x, it.y0, bounds_.w, it.y1 - it.y0};

    if (it.type == Item::Group) {
        const bool is_spread = it.spread;
        const Rect header_rect = {bounds_.x, it.y0, bounds_.w, ih};
        if (hover_ == &it) renderer.draw_quad(header_rect, hover);

        const float ty = it.y0 + (ih - line_h) * 0.5f;
        const float tx = bounds_.x + kItemPad + indent;
        font_->draw_text(renderer, it.text.c_str(), tx, ty, text2);

        const float ax = bounds_.x + bounds_.w - kArrowSize - kItemPad;
        const float ay = it.y0 + ih * 0.5f;
        // 折叠箭头用 iconfont 码点（原先裸 draw_polygon 三角、图标库缺失：UI-148）
        const uint32_t arrow_cp = is_spread ? layui_icon::down : layui_icon::right;
        font_->draw_icon(renderer, arrow_cp, ax, ay, ih * 0.45f, muted);
        return;
    }

    if (hover_ == &it || active_hover_ == &it) renderer.draw_quad(row, hover);

    const float ty = it.y0 + (ih - line_h) * 0.5f;
    const float tx = bounds_.x + kItemPad + indent;
    const Vec4 text_color = it.checked ? t->success : text;

    font_->draw_text(renderer, it.text.c_str(), tx, ty, text_color);

    // 英文副标题（新镜像 docs/2 侧栏）：紧跟主标题，灰色小字
    if (!it.subtitle.empty()) {
        const float sub_x = tx + font_->measure_text(it.text.c_str()) + kBadgePad;
        const Vec4 sub_color = it.checked ? t->success : t->text_muted;
        font_->draw_text(renderer, it.subtitle.c_str(), sub_x, ty + 2.f, sub_color);
    }

    if (it.has_badge && !it.badge_text.empty()) {
        const float tw = font_->measure_text(it.text.c_str());
        const float bw = font_->measure_text(it.badge_text.c_str()) + kBadgeTextPad;
        const float bh = line_h - 2.f;
        const float bx = tx + tw + kBadgePad;
        const float by = ty + 1.f;
        renderer.draw_rounded_rect({bx, by, bw, bh}, bh * 0.5f, t->danger);
        font_->draw_text(renderer, it.badge_text.c_str(), bx + kBadgeTextPad * 0.5f, ty, {1.f, 1.f, 1.f, 1.f});
    }

    if (it.has_dot) {
        const float tw = font_->measure_text(it.text.c_str());
        const float dx = tx + tw + kBadgePad;
        const float dy = ty + line_h * 0.5f - kBadgeDotSize * 0.5f;
        renderer.draw_quad({dx, dy, kBadgeDotSize, kBadgeDotSize}, t->success);
    }

    if (it.type == Item::Parent) {
        const float ax = bounds_.x + bounds_.w - kArrowSize - kItemPad;
        const float ay = it.y0 + ih * 0.5f;
        Vec2 tri[3] = {{ax - kArrowSize, ay - kArrowSize}, {ax, ay}, {ax - kArrowSize, ay + kArrowSize}};
        renderer.draw_polygon(tri, 3, muted);
    }
}

// (x,y) 命中当前打开父项的 flyout 子面板内子项 → 返回其 items_ 下标；未展开/未命中 → -1
int Menu::hit_sub_item(float x, float y) const {
    if (open_parent_ < 0 || open_parent_ >= (int)items_.size() ||
        items_[open_parent_].type != Item::Parent || !items_[open_parent_].visible)
        return -1;
    if (sub_panel_rect_.w <= 0.f || sub_panel_rect_.h <= 0.f) return -1;
    if (x < sub_panel_rect_.x || x >= sub_panel_rect_.x + sub_panel_rect_.w ||
        y < sub_panel_rect_.y || y >= sub_panel_rect_.y + sub_panel_rect_.h)
        return -1;
    const float sub_ly = y - sub_panel_rect_.y;
    if (sub_ly < kSubPanelPad) return -1;
    const int idx = (int)((sub_ly - kSubPanelPad) / kSubItemH);
    std::vector<int> sub;
    for (int j = open_parent_ + 1; j < (int)items_.size(); ++j) {
        if (items_[j].type == Item::Divider) break;
        if (items_[j].level <= items_[open_parent_].level) break;
        if (items_[j].level == items_[open_parent_].level + 1) sub.push_back(j);
    }
    if (idx < 0 || idx >= (int)sub.size()) return -1;
    return sub[idx];
}

void Menu::draw_sub_panel(Renderer& renderer, int parent_index) {
    if (!font_ || parent_index < 0 || parent_index >= (int)items_.size()) return;
    if (!items_[parent_index].visible) return;
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    const Vec4 panel = style_ov_ ? ov_bg_ : t->panel;
    const Vec4 border = style_ov_ ? ov_border_ : t->border;
    const Vec4 text = style_ov_ ? ov_text_ : t->text;
    const Vec4 muted = style_ov_ ? ov_text_ : t->text_muted;
    const Vec4 hover = style_ov_ ? ov_hover_ : Vec4{0.96f, 0.96f, 0.96f, 1.f};
    const float line_h = font_->line_height();

    // Collect visible items for this sub-panel
    std::vector<int> sub;
    for (int i = parent_index + 1; i < (int)items_.size(); ++i) {
        if (items_[i].type == Item::Divider) break;
        if (items_[i].level <= items_[parent_index].level) break;
        if (items_[i].level == items_[parent_index].level + 1) sub.push_back(i);
    }
    if (sub.empty()) return;

    // Measure panel width
    float max_tw = 0.f;
    for (int si : sub) {
        const float tw = font_->measure_text(items_[si].text.c_str());
        if (tw > max_tw) max_tw = tw;
    }
    const float pw = max_tw + kItemPad * 2 + kSubPanelPad * 2;
    const float ph = (float)sub.size() * kSubItemH + kSubPanelPad * 2;

    // Panel position: to the right of the menu, vertically aligned with parent item
    const float panel_x = bounds_.x + bounds_.w + 4.f;
    const float panel_y = items_[parent_index].y0;

    // Background + border
    renderer.draw_quad({panel_x, panel_y, pw, ph}, panel);
    renderer.draw_quad({panel_x, panel_y, pw, 1.f}, border);
    renderer.draw_quad({panel_x, panel_y + ph - 1.f, pw, 1.f}, border);
    renderer.draw_quad({panel_x, panel_y, 1.f, ph}, border);
    renderer.draw_quad({panel_x + pw - 1.f, panel_y, 1.f, ph}, border);

    // Store sub-panel rect for mouse hit testing (screen coords) + sync hitbox child
    const_cast<Menu*>(this)->sub_panel_rect_ = {panel_x, panel_y, pw, ph};
    if (hitbox_) {
        hitbox_->set_bounds(panel_x, panel_y, pw, ph);
        hitbox_->set_visible(true);
    }

    // Draw sub-items
    for (size_t si = 0; si < sub.size(); ++si) {
        Item& sub_it = items_[sub[si]];
        const float iy = panel_y + kSubPanelPad + (float)si * kSubItemH;
        const float ih = kSubItemH;
        const float ty = iy + (ih - line_h) * 0.5f;
        const float tx = panel_x + kSubPanelPad;

        if (active_hover_ == &sub_it)
            renderer.draw_quad({panel_x, iy, pw, ih}, hover);

        const Vec4 text_color = sub_it.checked ? t->success : text;
        font_->draw_text(renderer, sub_it.text.c_str(), tx, ty, text_color);

        if (sub_it.has_badge && !sub_it.badge_text.empty()) {
            const float stw = font_->measure_text(sub_it.text.c_str());
            const float bw = font_->measure_text(sub_it.badge_text.c_str()) + kBadgeTextPad;
            const float bh = line_h - 2.f;
            const float bx = tx + stw + kBadgePad;
            const float by = ty + 1.f;
            renderer.draw_rounded_rect({bx, by, bw, bh}, bh * 0.5f, t->danger);
            font_->draw_text(renderer, sub_it.badge_text.c_str(), bx + kBadgeTextPad * 0.5f, ty,
                             {1.f, 1.f, 1.f, 1.f});
        }

        if (sub_it.has_dot) {
            const float stw = font_->measure_text(sub_it.text.c_str());
            const float dx = tx + stw + kBadgePad;
            const float dy = ty + line_h * 0.5f - kBadgeDotSize * 0.5f;
            renderer.draw_quad({dx, dy, kBadgeDotSize, kBadgeDotSize}, t->success);
        }

        if (sub_it.type == Item::Parent) {
            const float ax = panel_x + pw - kArrowSize - kSubPanelPad;
            const float ay = iy + ih * 0.5f;
            Vec2 tri[3] = {{ax - kArrowSize, ay - kArrowSize}, {ax, ay}, {ax - kArrowSize, ay + kArrowSize}};
            renderer.draw_polygon(tri, 3, muted);
        }
    }
}

void Menu::draw_overlay(Renderer& renderer) {
    // UI-198: draw flyout if open_parent_ is set (persists when hover is on sub-item)
    if (open_parent_ >= 0 && open_parent_ < (int)items_.size() &&
        items_[open_parent_].type == Item::Parent && items_[open_parent_].visible) {
        draw_sub_panel(renderer, open_parent_);
    } else if (hitbox_) {
        hitbox_->set_visible(false);  // 无 flyout / 已收起 → 同步隐藏命中框
    }
}

void Menu::on_draw(Renderer& renderer) {
    if (!font_) return;
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    const Vec4 panel = style_ov_ ? ov_bg_ : t->panel;
    const Vec4 border = style_ov_ ? ov_border_ : t->border;

    // Menu background (white)
    renderer.draw_quad(bounds_, panel);

    // Border
    renderer.draw_quad({bounds_.x, bounds_.y, bounds_.w, 1.f}, border);
    renderer.draw_quad({bounds_.x, bounds_.y + bounds_.h - 1.f, bounds_.w, 1.f}, border);
    renderer.draw_quad({bounds_.x, bounds_.y, 1.f, bounds_.h}, border);
    renderer.draw_quad({bounds_.x + bounds_.w - 1.f, bounds_.y, 1.f, bounds_.h}, border);

    for (int i = 0; i < (int)items_.size(); ++i) {
        draw_item(renderer, items_[i], i);
    }
    // UI-198: flyout sub-panel must be drawn after items (sub-panel is outside bounds_)
    draw_overlay(renderer);
}

bool Menu::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    if (!font_) return false;
    const float lx = x - bounds_.x;

    // If a sub-panel is open, check if click is inside it first
    if (const int si = hit_sub_item(x, y); si >= 0) {
        // UI-198：flyout 子项行命中 → 触发回调 + on_item_click（Dropdown 据此选中即关）
        Item& sit = items_[si];
        if (sit.cb) sit.cb();
        if (on_item_click_) on_item_click_(sit.text.c_str());
        open_parent_ = -1;
        sub_panel_rect_ = {};
        if (hitbox_) hitbox_->set_visible(false);
        return true;
    }
    if (open_parent_ >= 0) {
        open_parent_ = -1;
        sub_panel_rect_ = {};
        if (hitbox_) hitbox_->set_visible(false);
    }

    if (lx < 0.f || lx >= bounds_.w || y < bounds_.y || y >= bounds_.y + bounds_.h)
        return false;

    for (int i = 0; i < (int)items_.size(); ++i) {
        Item& it = items_[i];
        if (!it.visible) continue;
        if (y >= it.y0 && y < it.y1) {
            if (it.type == Item::Group && it.allow_spread) {
                it.spread = !it.spread;
                if (on_group_click_) on_group_click_(it.text.c_str());
                return true;
            }
            if (it.type == Item::Parent) {
                // UI-198：点击父项 → 展开右侧 flyout（不收起菜单）
                active_hover_ = &it;
                open_parent_ = i;
                sub_panel_rect_ = {};  // 几何由本帧 draw_sub_panel 重算
                return true;
            }
            if (it.type == Item::Plain && it.cb) {
                it.cb();
                if (on_item_click_) on_item_click_(it.text.c_str());
                return true;
            }
        }
    }
    return true;
}

bool Menu::on_mouse_move(float x, float y) {
    if (!font_) return false;

    const float lx = x - bounds_.x;

    // 帧级门控（UI-135 / IDLE-002）：hover 目标切换（跟随框在高亮行间移动）必须显式标脏，
    // 否则 root is_dirty() 不翻 → 渲染被跳过 → 高亮停留在首个命中行，直至其他事件触发重绘。
    Item* const prev_hover = hover_;
    Item* const prev_active = active_hover_;
    const int prev_open = open_parent_;
    auto notify_hover_changed = [&]() {
        if (hover_ != prev_hover || active_hover_ != prev_active || open_parent_ != prev_open)
            mark_dirty();
    };

    hover_ = nullptr;

    // UI-198：悬停在已展开 flyout（右侧子面板）内 → 高亮子项、保持面板开启
    if (const int si = hit_sub_item(x, y); si >= 0) {
        active_hover_ = &items_[si];
        notify_hover_changed();
        return false;
    }

    // Check menu items
    if (lx >= 0.f && lx < bounds_.w && y >= bounds_.y && y < bounds_.y + bounds_.h) {
        for (int i = 0; i < (int)items_.size(); ++i) {
            Item& it = items_[i];
            if (!it.visible) continue;
            if (y >= it.y0 && y < it.y1) {
                if (it.type != Item::Divider) hover_ = &it;
                // UI-198：悬停父项自动展开右侧子面板（需先命中本面板再进 flyout）
                if (it.type == Item::Parent) {
                    if (open_parent_ != i) {
                        open_parent_ = i;
                        sub_panel_rect_ = {};  // 新父项 flyout 几何由本帧 draw_sub_panel 重算
                        active_hover_ = &it;
                    }
                } else if (open_parent_ >= 0) {
                    open_parent_ = -1;
                    sub_panel_rect_ = {};
                    if (active_hover_ && active_hover_->type == Item::Parent) active_hover_ = nullptr;
                }
                notify_hover_changed();
                return false;
            }
        }
    }

    // Mouse outside menu — keep flyout if mouse is over it; else close
    if (open_parent_ >= 0 && sub_panel_rect_.w > 0.f && sub_panel_rect_.h > 0.f) {
        if (x < sub_panel_rect_.x || x >= sub_panel_rect_.x + sub_panel_rect_.w ||
            y < sub_panel_rect_.y || y >= sub_panel_rect_.y + sub_panel_rect_.h) {
            open_parent_ = -1;
            sub_panel_rect_ = {};
            active_hover_ = nullptr;
        }
    } else {
        open_parent_ = -1;
        active_hover_ = nullptr;
    }
    notify_hover_changed();
    return false;
}

}  // namespace ui
