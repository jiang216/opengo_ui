#include "ui/ui/laydatebox.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/laydate.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
constexpr float kPanelW = 280.f;
constexpr float kTextH = 38.f;
constexpr float kTextPad = 10.f;
constexpr float kCaret = 14.f;  // 右侧下拉指示宽度
}  // namespace

LaydateBox::LaydateBox(float x, float y, float w, float h)
    : Widget(x, y, w, h) {
    if (h <= 0.f) bounds_.h = kTextH;
    focusable = true;
    panel_ = new Laydate(0, 0, kPanelW, 0);  // 尺寸由 sync_panel 按 type 决定
    panel_->visible = false;
    panel_->set_on_change([this](int, int, int) { apply_value_from_panel(); });
    panel_->set_on_done([this](const char*, int, int, int, int, int, int) { apply_value_from_panel(); });
}

LaydateBox::~LaydateBox() {
    // 面板展开时挂 root，析构前摘除，否则 root 悬空引用
    if (panel_->parent()) panel_->parent()->remove_child(panel_);
    delete panel_;
}

void LaydateBox::set_type(const char* t) {
    type_ = t ? t : "date";
    panel_->set_type(t);
}
void LaydateBox::set_range(bool v) { panel_->set_range(v); }
void LaydateBox::set_range_separator(const char* s) { panel_->set_range_separator(s); }
void LaydateBox::set_format(const char* f) { panel_->set_format(f); }
void LaydateBox::set_min(const char* v) { panel_->set_min(v); }
void LaydateBox::set_max(const char* v) { panel_->set_max(v); }
void LaydateBox::set_min_offset(int days) { panel_->set_min_offset(days); }
void LaydateBox::set_max_offset(int days) { panel_->set_max_offset(days); }
void LaydateBox::set_calendar(bool v) { panel_->set_calendar(v); }
void LaydateBox::set_marks(const std::vector<Laydate::Mark>& m) { panel_->set_marks(m); }
void LaydateBox::set_theme_style(const char* t) { panel_->set_theme_style(t); }
void LaydateBox::set_show_bottom(bool v) { panel_->set_show_bottom(v); }
void LaydateBox::set_btns(const std::vector<std::string>& b) { panel_->set_btns(b); }
void LaydateBox::set_lang(const char* l) { panel_->set_lang(l); }

void LaydateBox::set_text(const char* t) { text_ = t ? t : ""; }
void LaydateBox::set_value(const char* v) {
    panel_->set_value(v);
    text_ = panel_->value();
}
void LaydateBox::set_on_done(std::function<void(const char*, int, int, int, int, int, int)> cb) {
    panel_->set_on_done([this, cb](const char* val, int y, int m, int d, int h, int mi, int s) {
        apply_value_from_panel();
        if (cb) cb(val, y, m, d, h, mi, s);
    });
}

void LaydateBox::set_on_change(std::function<void(const char*)> cb) {
    on_change_ = std::move(cb);
}

void LaydateBox::apply_value_from_panel() {
    std::string v = panel_->value();
    if (v != text_) {
        text_ = v;
        if (on_change_) on_change_(v.c_str());
    }
}

void LaydateBox::sync_panel() {
    if (theme_) {
        panel_->set_theme(theme_);
        panel_->set_font(font_);
    }
    float ph = 300.f;
    switch (panel_->type()) {
        case Laydate::TypeTime: ph = 220.f; break;
        case Laydate::TypeDateTime: ph = 400.f; break;
        default: ph = 300.f; break;
    }
    panel_->set_bounds(bounds_.x, bounds_.y + bounds_.h, kPanelW, ph);
}

void LaydateBox::open() {
    if (readonly_) return;
    sync_panel();
    if (!panel_->parent()) root()->add_child(panel_);
    open_popup(this, panel_);
    open_ = true;
}

void LaydateBox::close() {
    panel_->visible = false;
    close_popups();
    open_ = false;
}

bool LaydateBox::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button != 0) return false;
    if (readonly_) return true;
    if (open_)
        close();
    else
        open();
    return true;
}

void LaydateBox::on_layout() {
    if (open_) sync_panel();
}

void LaydateBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    // 输入框外观：白底 + #eee 边框 + 聚焦 accent
    renderer.draw_rounded_rect(bounds_, 2.f, Vec4{1, 1, 1, 1});
    Vec4 border_color = t->border;
    if (hovered_ && !focused()) border_color = Vec4{t->accent.r, t->accent.g, t->accent.b, 0.6f};
    renderer.draw_rect_outline(bounds_, 1.f, border_color);
    if (focused()) {
        renderer.draw_rect_outline(bounds_, 1.f, t->success);
        const Rect shadow = {bounds_.x - 3, bounds_.y - 3, bounds_.w + 6, bounds_.h + 6};
        renderer.draw_rect_outline(shadow, 3.f, Vec4{0.09f, 0.72f, 0.47f, 0.08f});
    }
    if (font_) {
        const bool has_text = !text_.empty();
        const char* txt = has_text ? text_.c_str() : placeholder_.c_str();
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(renderer, txt, bounds_.x + kTextPad, ty,
                         has_text ? t->text : t->text_muted);
        // 下拉指示：细三角（仅在可交互时）
        if (!readonly_) {
            const float cx = bounds_.x + bounds_.w - kTextPad - 5.f;
            const float cy = bounds_.y + bounds_.h * 0.5f;
            const Vec2 tri[3] = {{cx - 4, cy - 2}, {cx + 4, cy - 2}, {cx, cy + 3}};
            renderer.draw_polygon(tri, 3, Vec4{0.76f, 0.76f, 0.76f, 1.0f});
        }
    }
}

}  // namespace ui