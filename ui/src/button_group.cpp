#include "ui/ui/button_group.h"

#include "ui/ui/button.h"

namespace ui {

ButtonGroup::ButtonGroup(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void ButtonGroup::add_button(Button* b) {
    if (!b) return;
    b->set_theme(theme_);
    add_child(b);
    update_roles();
}

void ButtonGroup::update_roles() {
    const int n = static_cast<int>(children_.size());
    for (int i = 0; i < n; ++i) {
        Button* b = dynamic_cast<Button*>(children_[i]);
        if (!b) continue;
        b->set_group_role(n == 1 ? 1 : (i == 0 ? 1 : (i == n - 1 ? 3 : 2)));
    }
}

void ButtonGroup::on_layout() {
    float cursor_x = bounds_.x;
    for (Widget* child : children_) {
        if (!child->visible) continue;
        const float w = child->bounds().w;
        const float h = child->preferred_height();
        child->set_bounds(cursor_x, bounds_.y + (bounds_.h - h) * 0.5f, w, h);
        cursor_x += w;
    }
}

}  // namespace ui
