#include "ui/ui/iconlabel.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/theme.h"

namespace ui {

IconLabel::IconLabel(float x, float y, float w, float h, const char* text)
    : Widget(x, y, w, h), text_(text ? text : "") {
    focusable = true;
}

void IconLabel::set_icon(const char* icon_name) {
    unsigned next = icon_name && *icon_name ? layui_icon::from_class(icon_name) : 0;
    if (icon_cp_ == next) return;
    clear_image();
    icon_cp_ = next;
    mark_dirty();
}

void IconLabel::set_image(const char* path) {
    if (!path || !*path) { clear_image(); return; }
    icon_cp_ = 0;
    delete image_tex_;
    image_tex_ = new Texture;
    if (!image_tex_->load_from_file(path)) {
        delete image_tex_;
        image_tex_ = nullptr;
        image_loaded_ = false;
    } else {
        image_loaded_ = true;
    }
    mark_dirty();
}

void IconLabel::clear_image() {
    if (!image_tex_) return;
    delete image_tex_;
    image_tex_ = nullptr;
    image_loaded_ = false;
    mark_dirty();
}

bool IconLabel::on_mouse_down(float x, float y, int button) {
    (void)x; (void)y;
    return button == 0 && on_click_;
}

bool IconLabel::on_mouse_up(float x, float y, int button) {
    if (button == 0 && contains(x, y) && on_click_) on_click_();
    return true;
}

bool IconLabel::on_key_down(int key, int mods) {
    (void)mods;
    if ((key == KeyEnter || key == KeySpace) && on_click_) {
        on_click_();
        return true;
    }
    return false;
}

void IconLabel::on_draw(Renderer& renderer) {
    if (!font_) return;

    const bool has_icon = icon_cp_ != 0 && font_->icon_loaded();
    const bool has_img  = image_loaded_ && image_tex_ != nullptr;
    const bool has_any_icon = has_icon || has_img;

    const float text_w = text_.empty() ? 0.f : font_->measure_text(text_.c_str());
    const float content_w = text_w + (has_any_icon ? icon_size_ + gap_ : 0.f);

    float left;
    if (align_ == 1)       left = bounds_.x + (bounds_.w - content_w) * 0.5f;
    else if (align_ == 2)  left = bounds_.x + bounds_.w - content_w - 4.f;
    else                   left = bounds_.x + 4.f;  // 左对齐留 4px padding

    const float cy = bounds_.y + bounds_.h * 0.5f;

    // 绘制图标（layui 字体图标）
    if (has_icon) {
        const Vec4 col = has_icon_color_ ? icon_color_
                        : (theme_ ? theme_->text_secondary : Vec4{1, 1, 1, 1});
        const float cx = left + icon_size_ * 0.5f;
        font_->draw_icon(renderer, icon_cp_, cx, cy, icon_size_, col);
    }

    // 绘制图片
    if (has_img) {
        const float img_x = left;
        const float img_y = cy - icon_size_ * 0.5f;
        renderer.draw_textured_quad(
            {img_x, img_y, icon_size_, icon_size_},
            image_tex_->id(),
            {0.f, 0.f, 1.f, 1.f},
            {1.f, 1.f, 1.f, 1.f});
    }

    // 绘制文本
    if (!text_.empty()) {
        const float tx = left + (has_any_icon ? icon_size_ + gap_ : 0.f);
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        const Vec4 col = has_text_color_ ? text_color_
                        : (theme_ ? theme_->text : Vec4{1, 1, 1, 1});
        font_->draw_text(renderer, text_.c_str(), tx, ty, col);
    }
}

}  // namespace ui
