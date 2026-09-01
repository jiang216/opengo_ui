#include "ui/ui/menubar.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace ui {

namespace {
constexpr float kTriggerH = 24.f;
constexpr float kItemH = 26.f;
constexpr float kItemW = 180.f;
constexpr float kGap = 4.f;
}  // namespace

MenuBar::MenuBar(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void MenuBar::set_font(Font* font) {
    font_ = font;
    for (Menu& m : menus_) m.trigger->set_font(font);
}

void MenuBar::set_theme(const Theme* theme) {
    theme_ = theme;
    for (Menu& m : menus_) {
        m.trigger->set_theme(theme);
        m.popup->set_theme(theme);
    }
}

void MenuBar::add_menu(const char* title) {
    Button* trig = new Button(0, 0, 0, kTriggerH, title ? title : "");
    trig->set_font(font_);
    trig->set_theme(theme_);
    add_child(trig);

    VBox* popup = new VBox(0, 0, 0, 0);
    popup->set_visible(false);  // UI-156
    popup->set_padding(4.f);
    popup->set_spacing(2.f);
    popup->set_theme(theme_);

    const int idx = (int)menus_.size();
    menus_.push_back({trig, popup});
    trig->set_on_click([this, idx]() { toggle_menu(menus_[idx]); });
}

void MenuBar::add_item(const char* title, std::function<void()> cb) {
    if (menus_.empty()) add_menu("menu");
    Menu& m = menus_.back();
    Button* item = new Button(0, 0, kItemW, kItemH, title ? title : "");
    item->set_font(font_);
    item->set_theme(theme_);
    item->set_on_click([this, cb = std::move(cb)]() {
        close_all_menus();
        if (cb) cb();
    });
    m.popup->add_child(item);
}

void MenuBar::toggle_menu(Menu& m) {
    if (m.popup->visible) {
        m.popup->set_visible(false);  // UI-156
        close_popups();
        return;
    }
    // 打开本菜单：先收起其它菜单
    close_all_menus();
    const Rect& b = m.trigger->bounds();
    m.popup->set_bounds(b.x, b.y + b.h, kItemW, m.popup->preferred_height());
    if (!m.popup->parent()) root()->add_child(m.popup);
    open_popup(m.trigger, m.popup);
}

void MenuBar::close_all_menus() {
    for (Menu& m : menus_) m.popup->set_visible(false);  // UI-156
    close_popups();
}

void MenuBar::on_layout() {
    float x = bounds_.x + 4.f;
    for (Menu& m : menus_) {
        float w = 80.f;
        if (font_) w = font_->measure_text(m.trigger->text().c_str()) + 24.f;
        m.trigger->set_bounds(x, bounds_.y + (bounds_.h - kTriggerH) * 0.5f, w, kTriggerH);
        x += w + kGap;
    }
}

void MenuBar::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    renderer.draw_rounded_rect(bounds_, t->radius, t->button_normal);
}

}  // namespace ui
