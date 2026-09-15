#include "ui/ui/dropdown.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/menu.h"
#include "ui/ui/theme.h"

namespace ui {

Dropdown::Dropdown(float x, float y, float w, float h, const char* title)
    : Widget(x, y, w, h), title_(title ? title : "menu") {
    trigger_ = new Button(0, 0, 0, h, title ? title : "menu");
    trigger_->add_class("layui-btn layui-btn-primary layui-btn-sm");
    trigger_->set_on_click([this]() {
        if (event_ != DropdownTrigger::Click) return;
        if (opened())
            hide();
        else
            open();
    });
    add_child(trigger_);

    menu_ = new Menu(0, 0, menu_w_, 0);
    menu_->set_visible(false);  // UI-156
    menu_->add_class("layui-menu");
    menu_->set_on_item_click([this](const char* t) {
        input_text_ = t ? t : "";
        if (on_select_) on_select_(t);
        hide();
    });
}

Dropdown::~Dropdown() {
    // Fix: detach menu_ before custom_ — menu_ may be a child of custom_
    // (AG-777 custom-content template: sb->add_child(mn)). Deleting custom_
    // first then dereferencing menu_->parent() is a use-after-free.
    if (menu_ && menu_->parent()) menu_->parent()->remove_child(menu_);
    if (custom_ && custom_->parent()) custom_->parent()->remove_child(custom_);
    delete custom_;
    delete menu_;
}

void Dropdown::set_font(Font* font) {
    mark_dirty();
        font_ = font;
    trigger_->set_font(font);
    if (menu_) menu_->set_font(font);
}

void Dropdown::set_theme(const Theme* theme) {
    mark_dirty();
        theme_ = theme;
    trigger_->set_theme(theme);
    if (menu_) menu_->set_theme(theme);
}

// ---- 菜单内容 ----
void Dropdown::add_item(const char* title, std::function<void()> cb) {
    if (menu_) menu_->add_item(title, cb ? std::move(cb) : std::function<void()>([]() {}));
}

void Dropdown::add_separator() {
    mark_dirty();
        if (menu_) menu_->add_divider();
}

void Dropdown::add_group(const char* title, bool allow_spread) {
    mark_dirty();
        if (menu_) menu_->add_group(title, allow_spread);
}

void Dropdown::add_parent(const char* title) {
    if (menu_) menu_->add_parent(title);
}

void Dropdown::end_group() {
    if (menu_) menu_->end_group();
}

// ---- 触发器 / 外观 ----
void Dropdown::set_trigger_style(DropdownStyle s) {
    mark_dirty();
        style_ = s;
    if (style_ == DropdownStyle::PrimaryButton) {
        if (!trigger_->has_class("layui-btn-primary")) trigger_->add_class("layui-btn-primary");
    } else {
        if (trigger_->has_class("layui-btn-primary")) trigger_->remove_class("layui-btn-primary");
    }
    trigger_->set_visible(style_ == DropdownStyle::Button || style_ == DropdownStyle::PrimaryButton);  // UI-156
}

void Dropdown::set_trigger_event(DropdownTrigger t) { mark_dirty();
        event_ = t; }

void Dropdown::set_align(DropdownAlign a) { mark_dirty();
        align_ = a; }

void Dropdown::set_menu_width(float w) { mark_dirty();
        menu_w_ = w; }

void Dropdown::set_region(const char* placeholder) {
    mark_dirty();
        region_txt_ = placeholder ? placeholder : "";
    set_trigger_style(DropdownStyle::Text);
    set_trigger_event(DropdownTrigger::ContextMenu);
    trigger_->set_visible(false);  // UI-156
}

void Dropdown::set_custom_content(Widget* content) {
    mark_dirty();
        if (custom_ == content) return;
    if (custom_ && custom_->parent()) custom_->parent()->remove_child(custom_);
    delete custom_;
    custom_ = content;
    if (custom_) custom_->set_visible(false);  // UI-156
}

void Dropdown::set_popup_style(const Vec4& bg, const Vec4& hover, const Vec4& text) {
    mark_dirty();
        popup_bg_ = bg;
    popup_hover_ = hover;
    popup_text_ = text;
    style_custom_ = true;
    if (menu_) menu_->set_style_overrides(bg, bg, hover, text);
}

// UI-198：转发到内部 Menu —— 上下文菜单动态回显当前状态（文本/选中态），由宿主每帧刷新
int Dropdown::item_count() const {
    return menu_ ? menu_->item_count() : 0;
}

void Dropdown::set_item_text(int index, const char* title) {
    if (menu_) menu_->set_item_text(index, title);
}

void Dropdown::set_item_checked(int index, bool checked) {
    if (menu_) menu_->set_item_checked(index, checked);
}

// ---- 显隐 ----
void Dropdown::show() {
    mark_dirty();
        open();
}

void Dropdown::hide() {
    mark_dirty();
    // Fix: hide both custom_ and menu_ — opened() checks both visible flags.
    // If only custom_ is hidden, opened() still returns true (menu_ visible),
    // causing double-hide and triggering the destructor UAF.
    if (custom_) custom_->set_visible(false);
    if (menu_) menu_->set_visible(false);
    close_popups();
    hover_open_ = false;
    show_at_ = false;
}

void Dropdown::show_at(float sx, float sy) {
    mark_dirty();
        show_at_ = true;
    at_x_ = sx;
    at_y_ = sy;
    open();
}

void Dropdown::open() {
    Widget* box = custom_ ? static_cast<Widget*>(custom_) : menu_;
    if (!box) return;
    place_popup();
    if (!box->parent()) {
        if (!root()) return;
        root()->add_child(box);
    }
    open_popup(this, box);
    box->set_visible(true);  // UI-156
}

void Dropdown::place_popup() {
    Widget* box = custom_ ? static_cast<Widget*>(custom_) : menu_;
    if (!box) return;
    const float w = custom_ ? custom_->bounds().w : menu_w_;
    const float h = menu_height();
    float x, y;
    if (show_at_) {
        x = at_x_;
        y = at_y_;
    } else {
        const Rect& b = bounds_;
        x = b.x;
        y = b.y + b.h;
        if (align_ == DropdownAlign::Center) x = b.x + (b.w - w) * 0.5f;
        else if (align_ == DropdownAlign::Right) x = b.x + b.w - w;
        // UI-203：向下放不下时自动翻转向上（仿 ComboBox UI-110 sync_panel）
        const Widget* rt = root();
        if (rt) {
            const float vis_bot = rt->bounds().y + rt->bounds().h;
            if (y + h > vis_bot) {
                const float flipped_y = b.y - h;
                const float vis_top = rt->bounds().y;
                y = (flipped_y < vis_top) ? vis_top : flipped_y;
            }
        }
    }
    const float dx = x - box->bounds().x;
    const float dy = y - box->bounds().y;
    box->set_bounds(x, y, w, h);
    if (custom_ && (dx != 0.f || dy != 0.f)) {
        std::function<void(Widget*)> move_children = [&](Widget* p) {
            for (Widget* c : p->children()) {
                c->set_bounds(c->bounds().x + dx, c->bounds().y + dy, c->bounds().w, c->bounds().h);
                move_children(c);
            }
        };
        move_children(box);
    }
}

int Dropdown::menu_height() const {
    if (custom_) return (int)custom_->bounds().h;
    return (int)menu_->preferred_height();
}

bool Dropdown::opened() const {
    return (custom_ && custom_->visible) || (menu_ && menu_->visible);
}

void Dropdown::on_layout() {
    trigger_->set_bounds(bounds_.x, bounds_.y, bounds_.w, bounds_.h);
}

bool Dropdown::on_mouse_down(float x, float y, int button) {
    if (event_ == DropdownTrigger::ContextMenu) {
        if (button == 1) {
            show_at(x, y);
            return true;
        }
        return false;
    }

    const bool input = (style_ == DropdownStyle::Input || style_ == DropdownStyle::Text);
    if (button != 0) return false;
    if (input) {
        if (opened())
            hide();
        else
            open();
        return true;
    }

    // Button / PrimaryButton 风格：
    switch (event_) {
        case DropdownTrigger::MouseDown:
            open();
            return true;
        case DropdownTrigger::DblClick: {
            const auto now = std::chrono::steady_clock::now();
            const bool is_double = (now - last_down_) < std::chrono::milliseconds(400);
            last_down_ = now;
            if (is_double) open();
            return true;
        }
        default:
            return false;  // Click 由 trigger Button onClick 处理
    }
}

bool Dropdown::on_mouse_move(float x, float y) {
    if (event_ != DropdownTrigger::Hover) return false;
    const bool inside =
        x >= bounds_.x && x <= bounds_.x + bounds_.w && y >= bounds_.y && y <= bounds_.y + bounds_.h;
    if (inside) {
        if (!hover_open_) {
            hover_open_ = true;
            open();
        }
    } else {
        hover_open_ = false;
    }
    return false;
}

void Dropdown::on_draw(Renderer& renderer) {
static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;

    if (!region_txt_.empty() && font_) {
        renderer.draw_quad(bounds_, Vec4{0.94f, 0.94f, 0.94f, 1.f});
        const float tw = font_->measure_text(region_txt_.c_str());
        const float tx = bounds_.x + (bounds_.w - tw) * 0.5f;
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(renderer, region_txt_.c_str(), tx, ty, t->text_muted);
        return;
    }

    if (style_ == DropdownStyle::Input && font_) {
        const Vec4 border = opened() ? t->success : t->border;
        renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1.f, 1.f, 1.f, 1.f});
        renderer.draw_rect_outline(bounds_, 1.f, border);
        const char* txt = input_text_.empty() ? title_.c_str() : input_text_.c_str();
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(renderer, txt, bounds_.x + 10.f, ty, t->text_secondary);
        const float cy = bounds_.y + bounds_.h * 0.5f;
        const Vec2 tri[3] = {{bounds_.x + bounds_.w - 16.f, cy - 3.f},
                             {bounds_.x + bounds_.w - 6.f, cy - 3.f},
                             {bounds_.x + bounds_.w - 11.f, cy + 3.f}};
        renderer.draw_polygon(tri, 3, t->text_muted);
    }

    if (style_ == DropdownStyle::Text && font_ && !title_.empty()) {
        const Vec4 blue{0.12f, 0.62f, 1.0f, 1.0f};  // layui 链接蓝 #1e9fff
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(renderer, title_.c_str(), bounds_.x, ty, blue);
        const float tw = font_->measure_text(title_.c_str());
        const float cy = bounds_.y + bounds_.h * 0.5f;
        const Vec2 tri[3] = {{bounds_.x + tw + 4.f, cy - 2.f},
                             {bounds_.x + tw + 10.f, cy - 2.f},
                             {bounds_.x + tw + 7.f, cy + 2.f}};
        renderer.draw_polygon(tri, 3, blue);
    }

    if ((style_ == DropdownStyle::Button || style_ == DropdownStyle::PrimaryButton) && font_ &&
        trigger_->visible) {
        const float cy = bounds_.y + bounds_.h * 0.5f;
        const bool primary = (style_ == DropdownStyle::PrimaryButton);
        const Vec4 col = primary ? t->text_muted : Vec4{1.f, 1.f, 1.f, 0.95f};
        const Vec2 tri[3] = {{bounds_.x + bounds_.w - 17.f, cy - 3.f},
                             {bounds_.x + bounds_.w - 7.f, cy - 3.f},
                             {bounds_.x + bounds_.w - 12.f, cy + 3.f}};
        renderer.draw_polygon(tri, 3, col);
    }
}

}  // namespace ui



