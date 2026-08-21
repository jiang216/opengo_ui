#include "ui/ui/label.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {

// UTF-8 解码：返回本码点占用的字节数（0 表示无效首字节）
int utf8_decode(const char* s, unsigned int& out) {
    const unsigned char c = (unsigned char)s[0];
    if (c < 0x80) {
        out = c;
        return 1;
    }
    if ((c & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
        out = ((c & 0x1F) << 6) | ((unsigned char)s[1] & 0x3F);
        return 2;
    }
    if ((c & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
        out = ((c & 0x0F) << 12) | (((unsigned char)s[1] & 0x3F) << 6) |
              ((unsigned char)s[2] & 0x3F);
        return 3;
    }
    if ((c & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 &&
        (s[3] & 0xC0) == 0x80) {
        out = ((c & 0x07) << 18) | (((unsigned char)s[1] & 0x3F) << 12) |
              (((unsigned char)s[2] & 0x3F) << 6) | ((unsigned char)s[3] & 0x3F);
        return 4;
    }
    out = c;
    return 1;
}

}  // namespace

Label::Label(float x, float y, float w, float h, const char* text)
    : Widget(x, y, w, h), text_(text ? text : "") {}

void Label::set_text(const char* text) {
    text_ = text ? text : "";
    dirty_ = true;
}

void Label::set_wrap(bool wrap) {
    wrap_ = wrap;
    dirty_ = true;
}

float Label::preferred_height() const {
    if (!font_) return bounds_.h;
    if (!wrap_) return font_->line_height();
    if (dirty_) const_cast<Label*>(this)->recompute_lines();  // 宽度依赖当前 bounds_.w
    return std::max(1, (int)lines_.size()) * font_->line_height();
}

void Label::recompute_lines() {
    lines_.clear();
    dirty_ = false;
    if (!font_ || text_.empty()) {
        lines_.push_back("");
        return;
    }
    if (!wrap_ || bounds_.w <= 0.f) {
        lines_.push_back(text_);
        return;
    }
    const float max_w = bounds_.w;
    std::string line;
    float w = 0.f;
    size_t i = 0;
    const size_t len = text_.size();
    while (i < len) {
        unsigned int cp = 0;
        const int n = utf8_decode(text_.data() + i, cp);
        const float cw = font_->char_width(cp);
        if (w + cw > max_w && !line.empty()) {
            lines_.push_back(line);
            line.clear();
            w = 0.f;
            if (cp == ' ') {  // 行首空格丢弃
                i += n;
                continue;
            }
        }
        line.append(text_.data() + i, (size_t)n);
        w += cw;
        i += (size_t)n;
    }
    if (!line.empty() || lines_.empty()) lines_.push_back(line);
}

void Label::on_draw(Renderer& renderer) {
    if (!font_ || text_.empty()) return;
    if (dirty_) recompute_lines();
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const Vec4 color = color_set_ ? color_ : t->text;

    const float line_h = font_->line_height();
    const float total_h = (float)lines_.size() * line_h;
    float y = bounds_.y + (bounds_.h - total_h) * 0.5f;

    for (const std::string& line : lines_) {
        if (!line.empty()) {
            const float text_w = font_->measure_text(line.c_str());
            float tx = bounds_.x;
            if (align_ == 1)
                tx = bounds_.x + (bounds_.w - text_w) * 0.5f;
            else if (align_ == 2)
                tx = bounds_.x + bounds_.w - text_w;
            const float ty = y + (line_h - font_->line_height()) * 0.5f;
            font_->draw_text(renderer, line.c_str(), tx, ty, color);
        }
        y += line_h;
    }
}

}  // namespace ui
