#include "ui/ui/combobox.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

namespace {
constexpr float kOptionH = 26.f;
constexpr float kMaxPanelH = 150.f;
constexpr float kTextPad = 10.f;
}  // namespace

std::vector<ComboBox*> ComboBox::instances_;

ComboBox::ComboBox(float x, float y, float w, float h) : Widget(x, y, w, h) {
    focusable = true;
    add_class("layui-form-select");
    panel_ = new ScrollBox(0, 0, 1, 1);
    panel_->visible = false;
    panel_->set_padding(2.f);
    panel_->set_spacing(1.f);
    panel_->add_class("layui-form-select-panel");
    instances_.push_back(this);
}

ComboBox::~ComboBox() {
    auto it = std::find(instances_.begin(), instances_.end(), this);
    if (it != instances_.end()) instances_.erase(it);
    for (Button* b : option_buttons_) delete b;
    // panel_ 展开时挂到 root 下，析构前必须摘除，否则 root 悬空引用
    if (panel_->parent()) panel_->parent()->remove_child(panel_);
    delete panel_;
}

void ComboBox::add_option(const char* text) {
    const size_t index = options_.size();
    options_.emplace_back(text ? text : "");
    Button* opt = new Button(0, 0, 0, kOptionH, text ? text : "");
    opt->add_class("layui-form-select dd");
    if (index == 0) opt->add_class("layui-this");
    opt->set_on_click([this, index]() {
        select((int)index);
        focus();  // 选中后焦点回到下拉框
    });
    panel_->add_child(opt);
    option_buttons_.push_back(opt);
    sync_panel();
}

void ComboBox::select(int index) {
    if (index < 0 || index >= (int)options_.size()) return;
    if (selected_ != index) {
        selected_ = index;
        if (on_change_) on_change_(selected_);
    }
    close();
}

void ComboBox::open() {
    // 保证同一时刻只有一个下拉面板展开：打开当前下拉前，关闭所有其他 ComboBox 面板
    for (ComboBox* other : instances_) {
        if (other != this) other->close();
    }
    sync_panel();
    close_popups();  // 兜底清 root 弹出列表残留
    if (!panel_->parent()) root()->add_child(panel_);  // 根级子节点：点击命中优先
    open_popup(this, panel_);
}

void ComboBox::close() {
    panel_->visible = false;
    close_popups();
}

void ComboBox::sync_panel() {
    if (theme_) panel_->set_theme(theme_);
    const float max_h = std::min((float)(option_buttons_.size() * kOptionH + 4), kMaxPanelH);
    const float panel_x = bounds_.x;
    float panel_y = bounds_.y + bounds_.h;
    const float panel_w = bounds_.w;

    // 查找最近的 ScrollBox 祖先，确定可视区域
    Widget* vis_parent = parent_;
    ScrollBox* scroll_anc = nullptr;
    while (vis_parent) {
        scroll_anc = dynamic_cast<ScrollBox*>(vis_parent);
        if (scroll_anc) break;
        vis_parent = vis_parent->parent();
    }

    if (scroll_anc) {
        // 将 ComboBox 底部与可视区底部均转换为内容坐标（去除 ScrollBox 位置与滚动偏移），
        // 保证同一坐标空间内比较，避免面板翻转判断因坐标错位失效。
        const float sy = scroll_anc->bounds().y;
        const float sh = scroll_anc->bounds().h;
        const float sp = scroll_anc->padding();
        const float so = scroll_anc->scroll_offset();
        const float panel_y_c   = bounds_.y + bounds_.h - sy - sp + so;   // ComboBox 底部（内容坐标）
        const float vis_bot_c   = sh - sp + so;                           // 可视区底部（内容坐标）
        const float vis_top_c   = so;                                     // 可视区顶部（内容坐标）
        if (panel_y_c + max_h > vis_bot_c) {
            // 翻转到 ComboBox 上方；若超出可视区顶部则钉在顶部
            const float flipped_c = panel_y_c - bounds_.h - max_h;        // 翻转后 ComboBox 顶部的内容坐标
            if (flipped_c < vis_top_c)
                panel_y = sy + sp + so - max_h;                           // 钉在可视区顶部
            else
                panel_y = bounds_.y - max_h;
        }
    }

    panel_->set_bounds(panel_x, panel_y, panel_w, max_h);
    for (size_t i = 0; i < option_buttons_.size(); ++i) {
        Button* b = option_buttons_[i];
        b->set_font(font_);
        b->set_theme(theme_);
        if ((int)i == selected_)
            b->add_class("layui-this");
        else
            b->remove_class("layui-this");
    }
}

void ComboBox::on_layout() {
    sync_panel();
}

bool ComboBox::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    if (panel_->visible)
        close();
    else
        open();
    return true;
}

bool ComboBox::on_key_down(int key, int mods) {
    (void)mods;
    if (key == KeyUp || key == KeyDown) {
        if (options_.empty()) return true;
        const int n = (int)options_.size();
        const int dir = (key == KeyDown) ? 1 : -1;
        const int next = ((selected_ + dir) % n + n) % n;
        selected_ = next;
        if (on_change_) on_change_(selected_);
        return true;
    }
    if (key == KeyEnter || key == KeySpace) {
        if (panel_->visible)
            close();
        else
            open();
        return true;
    }
    if (key == KeyEscape && panel_->visible) {
        close();
        return true;
    }
    return false;
}

void ComboBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    if (has_class("layui-form-select")) {
        // ---- layui select 外观（M17b）：白底 + #eee 边框 + 右箭头三角形 ----
        renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1, 1, 1, 1});
        // hover：边框变 accent 浅色；聚焦：边框 accent + shadow
        Vec4 border_color = t->border;
        if (hovered_ && !focused()) {
            border_color = Vec4{t->accent.r, t->accent.g, t->accent.b, 0.6f};
        }
        renderer.draw_rect_outline(bounds_, 1.f, border_color);
        // 聚焦：边框 #16b777 + shadow 0 0 0 3px rgba(22,183,119,.08)
        if (focused()) {
            renderer.draw_rect_outline(bounds_, 1.f, t->success);
            const Rect shadow = {bounds_.x - 3, bounds_.y - 3, bounds_.w + 6, bounds_.h + 6};
            renderer.draw_rect_outline(shadow, 3.f, Vec4{0.09f, 0.72f, 0.47f, 0.08f});
        }
        if (font_) {
            const char* txt = selected_ >= 0 ? options_[selected_].c_str()
                                             : (options_.empty() ? "" : options_[0].c_str());
            const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
            font_->draw_text(renderer, txt, bounds_.x + kTextPad, ty, t->text_secondary);
            // 下拉箭头：三角（#c2c2c2，6px）
            const float cx = bounds_.x + bounds_.w - 14.f;
            const float cy = bounds_.y + bounds_.h * 0.5f;
            const Vec2 tri[3] = {{cx - 5, cy - 3}, {cx + 5, cy - 3}, {cx, cy + 3}};
            renderer.draw_polygon(tri, 3, Vec4{0.76f, 0.76f, 0.76f, 1.0f});
        }
        return;
    }

    // 主体（按钮风）：accent 底 + 聚焦外圈描边
    renderer.draw_rounded_rect(bounds_, t->radius, t->accent);
    if (focused()) {
        renderer.draw_rounded_rect(bounds_, t->radius, t->accent);
        const Rect inner = {bounds_.x + 2, bounds_.y + 2, bounds_.w - 4, bounds_.h - 4};
        renderer.draw_rounded_rect(inner, t->radius - 1.f, t->accent);
    }

    if (font_) {
        const char* txt = selected_ >= 0 ? options_[selected_].c_str()
                                         : (options_.empty() ? "" : options_[0].c_str());
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(renderer, txt, bounds_.x + kTextPad, ty, t->text);
        // 下拉指示（'v'）
        font_->draw_text(renderer, "v", bounds_.x + bounds_.w - kTextPad - 8.f, ty, t->text);
    }
}

}  // namespace ui
