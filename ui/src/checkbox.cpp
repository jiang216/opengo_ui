#include "ui/ui/checkbox.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
constexpr float kBoxSize = 18.f;
constexpr float kBoxRadius = 4.f;
}  // namespace

CheckBox::CheckBox(float x, float y, float w, float h, const char* text)
    : Widget(x, y, w, h), text_(text ? text : "") {
    focusable = true;
    add_class("layui-form-checkbox");
}

CheckBox::~CheckBox() {
    if (custom_) {
        if (custom_->parent()) custom_->parent()->remove_child(custom_);
        delete custom_;
    }
}

void CheckBox::set_checked(bool checked) {
    mark_dirty();
        if (checked_ == checked) return;
    checked_ = checked;
    if (checked_) indeterminate_ = false;  // 选中后清除半选（layui 行为）
    if (on_change_) on_change_(checked_);
}

void CheckBox::toggle() {
    mark_dirty();
        set_checked(!checked_);
}

void CheckBox::set_custom_content(Widget* content) {
    mark_dirty();
        if (custom_ == content) return;
    if (custom_) {
        if (custom_->parent()) custom_->parent()->remove_child(custom_);
        delete custom_;
    }
    custom_ = content;
}

void CheckBox::on_layout() {
    if (custom_) {
        // lay-checkbox 内容按皮肤摆放：
        //   None：铺满整个控件；Primary：紧随左侧方框；Tag：填充左侧标题块（方框在右）
        if (skin_ == Skin::None) {
            custom_->set_bounds(bounds_.x, bounds_.y, bounds_.w, bounds_.h);
        } else if (skin_ == Skin::Tag) {
            const float cw = bounds_.w - 30.f - 4.f;  // 标题块宽度（右侧 30px 方框）
            if (cw > 0.f) {
                custom_->set_bounds(bounds_.x + 4.f, bounds_.y, cw, bounds_.h);
            }
        } else {
            const float cw = bounds_.w - kBoxSize - 8.f;
            if (cw > 0.f) {
                custom_->set_bounds(bounds_.x + kBoxSize + 8.f, bounds_.y, cw, bounds_.h);
            }
        }
        custom_->layout();
    }
}

bool CheckBox::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    toggle();
    return true;
}

bool CheckBox::on_key_down(int key, int mods) {
    (void)mods;
    if (key == KeyEnter || key == KeySpace) {
        toggle();
        return true;
    }
    return false;
}

void CheckBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const float cy = bounds_.y + bounds_.h * 0.5f;
    const float bx = bounds_.x;
    const float by = cy - kBoxSize * 0.5f;
    const Rect box{bx, by, kBoxSize, kBoxSize};

    const Vec4 text_color = enabled ? t->text : t->text_muted;

    if (skin_ == Skin::None) {
        // 无默认样式：仅交互 + 自定义内容（layui lay-skin=none 卡片/标签风格多选组件）
        if (custom_) {
            custom_->draw(renderer);
        } else if (font_ && !text_.empty()) {
            const float tx = bx;
            const float ty = cy - font_->line_height() * 0.5f;
            font_->draw_text(renderer, text_.c_str(), tx, ty, text_color);
        }
        return;
    }

    if (skin_ == Skin::Tag) {
        // 标签风格（layui lay-skin=tag，继承基础 .layui-form-checkbox 结构）：
        // 左侧标题块（#d2d2d2 灰底白字，左圆角）+ 右侧 30px 对勾方框；
        // 选中：标题块变绿底白字 + 方框绿边绿勾；禁用：标题块 #eee 灰字。
        const float box_w = 30.f;
        const Rect tag_box{bounds_.x + bounds_.w - box_w, bounds_.y, box_w, bounds_.h};
        const Rect title_rect{bounds_.x, bounds_.y, bounds_.w - box_w, bounds_.h};

        // 右侧方框：未选中白底灰边，选中绿边绿勾，禁用灰边
        Vec4 box_fill = !enabled ? Vec4{0.96f, 0.96f, 0.96f, 1.0f}
                                 : (checked_ ? t->success : t->panel);
        Vec4 box_border = !enabled ? t->border
                                   : (checked_ ? t->success : Vec4{0.82f, 0.82f, 0.82f, 1.0f});
        renderer.draw_rounded_rect(tag_box, kBoxRadius, box_border);
        renderer.draw_rounded_rect(Rect{tag_box.x + 1, tag_box.y + 1, tag_box.w - 2, tag_box.h - 2},
                                   kBoxRadius - 1.f, box_fill);
        // 对勾图标：选中时绿色显示在方框内
        if (checked_ && font_ && font_->icon_loaded() && !enabled) {
            font_->draw_icon(renderer, layui_icon::ok, tag_box.x + tag_box.w * 0.5f,
                             tag_box.y + tag_box.h * 0.5f, 20.f, Vec4{0.76f, 0.76f, 0.76f, 1.0f});
        } else if (checked_ && font_ && font_->icon_loaded()) {
            font_->draw_icon(renderer, layui_icon::ok, tag_box.x + tag_box.w * 0.5f,
                             tag_box.y + tag_box.h * 0.5f, 20.f, t->success);
        }

        // 左侧标题块：未选中灰底白字，选中绿底白字，禁用 #eee 灰字
        Vec4 title_bg = !enabled ? Vec4{0.93f, 0.93f, 0.93f, 1.0f}
                                 : (checked_ ? t->success : Vec4{0.82f, 0.82f, 0.82f, 1.0f});
        if (enabled && hovered_ && !checked_) title_bg = Vec4{0.76f, 0.76f, 0.76f, 1.0f};
        renderer.draw_rounded_rect(title_rect, kBoxRadius, title_bg);
        if (custom_) {
            custom_->draw(renderer);
        } else if (font_ && !text_.empty()) {
            const float tw = font_->measure_text(text_.c_str());
            const float tx = title_rect.x + (title_rect.w - tw) * 0.5f;
            const float ty = bounds_.y + bounds_.h * 0.5f - font_->line_height() * 0.5f;
            const Vec4 col = !enabled ? t->text_muted : Vec4{1, 1, 1, 1};
            font_->draw_text(renderer, text_.c_str(), tx, ty, col);
        }
        return;
    }

    // ---- Primary 风格：方框 + 对勾/横线 + 文本 ----
    Vec4 accent = enabled ? t->accent : t->text_muted;
    Vec4 inner = !enabled ? t->button_normal : (checked_ ? t->accent : t->button_normal);
    if (enabled && hovered_ && !checked_) {
        inner = Vec4{0.93f, 0.98f, 0.95f, 1.0f};  // 淡绿底
    }
    if (enabled && pressed_) {
        if (checked_) {
            inner = Vec4{t->accent.r * 0.85f, t->accent.g * 0.85f, t->accent.b * 0.85f, 1.f};
        } else {
            accent = Vec4{t->accent.r * 0.85f, t->accent.g * 0.85f, t->accent.b * 0.85f, 1.f};
        }
    }

    // 边框（accent 2px）：先画外圈再叠内层
    renderer.draw_rounded_rect(box, kBoxRadius, accent);
    const Rect inner_box{box.x + 2, box.y + 2, box.w - 4, box.h - 4};
    renderer.draw_rounded_rect(inner_box, kBoxRadius - 1.f, inner);

    // 半选态：绿框白底 + 中间横线（layui-icon-subtraction）
    if (enabled && indeterminate_) {
        renderer.draw_rounded_rect(box, kBoxRadius, t->accent);
        renderer.draw_rounded_rect(inner_box, kBoxRadius - 1.f, Vec4{1, 1, 1, 1});
        renderer.draw_rounded_rect(Rect{box.x + 3, cy - 1.f, box.w - 6, 2.f}, 1.f, t->accent);
    }

    // 选中态：方框内绘制白色对勾（layui-icon-ok）
    if (enabled && checked_ && font_ && font_->icon_loaded()) {
        font_->draw_icon(renderer, layui_icon::ok, box.x + box.w * 0.5f,
                         box.y + box.h * 0.5f, 14.f, Vec4{1, 1, 1, 1});
    }

    // 文本 / lay-checkbox 自定义内容
    if (custom_) {
        custom_->draw(renderer);
    } else if (font_ && !text_.empty()) {
        const float tx = bx + kBoxSize + 8.f;
        const float ty = cy - font_->line_height() * 0.5f;
        font_->draw_text(renderer, text_.c_str(), tx, ty, text_color);
    }
}

}  // namespace ui