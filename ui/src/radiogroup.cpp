#include "ui/ui/radiogroup.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/radiobutton.h"

namespace ui {

RadioGroup::RadioGroup(float x, float y, float w, float h) : Widget(x, y, w, h) {}

// 选项为自身子控件，由所有者 release_tree 递归释放；析构不重复删除
RadioGroup::~RadioGroup() = default;

void RadioGroup::add_option(const char* text) {
    mark_dirty();
        const size_t index = options_.size();
    RadioButton* rb = new RadioButton(0, 0, 0, option_h_, text);
    rb->set_font(font_);
    rb->set_theme(theme_);
    rb->set_on_activate([this, index]() { select((int)index); });
    add_child(rb);
    options_.push_back(rb);
}

void RadioGroup::select(int index) {
    mark_dirty();
        if (index < 0 || index >= (int)options_.size()) return;
    if (selected_ == index) return;
    selected_ = index;
    for (size_t i = 0; i < options_.size(); ++i) options_[i]->set_checked((int)i == selected_);
    if (on_change_) on_change_(selected_);
}

void RadioGroup::set_option_enabled(int index, bool en) {
    mark_dirty();
        if (index < 0 || index >= (int)options_.size()) return;
    options_[(size_t)index]->set_enabled(en);
}

void RadioGroup::set_enabled(bool en) {
    Widget::set_enabled(en);
    for (RadioButton* rb : options_) rb->set_enabled(en);
}

void RadioGroup::on_layout() {
    float cursor_x = bounds_.x;
    for (RadioButton* rb : options_) {
        const float tw = font_ ? font_->measure_text(rb->text().c_str()) : 60.f;
        const float w = 9.f * 2.f + 8.f + tw;
        rb->set_bounds(cursor_x, bounds_.y, w, bounds_.h);
        cursor_x += w + spacing_;
    }
}

float RadioGroup::preferred_width() const {
    float w = 0.f;
    for (size_t i = 0; i < options_.size(); ++i) {
        const float tw = font_ ? font_->measure_text(options_[i]->text().c_str()) : 60.f;
        w += 9.f * 2.f + 8.f + tw;
        if (i > 0) w += spacing_;
    }
    return w;
}

void RadioGroup::on_draw(Renderer&) {}

}  // namespace ui
