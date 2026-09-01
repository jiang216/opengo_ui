#include "ui/ui/label.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <cstdint>
#include <cstring>

namespace ui {

namespace {
// 换行依赖输入的 FNV-1a 组合键：任一影响 lines_ 的输入变化即换键重算。
// 用 bits 混合（text 内容、focus 无关），避免依赖 is_dirty 时序导致 const 路径
// preferred_height 与 on_draw 按旧 boundsw 重复真算；输入未变则复用缓存。
size_t lines_dep_of(const std::string& text, bool wrap, float w, const Font* font, float scale) {
    auto fbits = [](float f) {
        uint32_t b; std::memcpy(&b, &f, sizeof b); return b;
    };
    const unsigned char* p = (const unsigned char*)text.data();
    const size_t n = text.size();
    size_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ull; }
    h ^= (size_t)(wrap ? 1 : 0) << 0;
    h ^= (size_t)fbits(w) << 1;
    h ^= (size_t)reinterpret_cast<uintptr_t>(font) << 2;
    h ^= (size_t)fbits(scale) << 3;
    // 字形烘焙推进会改变实际 xadvance（UI-136），即使字体指针未变也要重算换行
    h ^= (size_t)Font::bake_epoch_sum() << 4;
    return h ? h : 1;
}

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
    std::string next = text ? text : "";
    if (text_ == next) return;
    text_ = std::move(next);
    mark_dirty();
}

void Label::set_wrap(bool wrap) {
    if (wrap_ == wrap) return;
    wrap_ = wrap;
    mark_dirty();
}

float Label::preferred_height() const {
    if (!font_) return bounds_.h;
    if (!wrap_) return font_->line_height() * text_scale_;
    // const 路径按宽度换行：recompute_lines 自带依赖键去重（输入未变即时返回），
    // 不依赖 is_dirty 时序。契约：调用前需已确定 bounds_.w（preferred_height 由父容器在设宽后查询）。
    const_cast<Label*>(this)->recompute_lines();
    return std::max(1, (int)lines_.size()) * font_->line_height() * text_scale_;
}

void Label::recompute_lines() {
    const size_t dep = lines_dep_of(text_, wrap_, bounds_.w, font_, text_scale_);
    if (dep == lines_dep_) return;  // 换行依赖输入未变 → 复用缓存 lines_
    lines_dep_ = dep;
    lines_.clear();
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
        const float cw = font_->char_width(cp) * text_scale_;
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
    recompute_lines();  // 依赖键去重：输入未变即时返回复用 lines_，不再依赖 is_dirty
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const Vec4 color = color_set_ ? color_ : t->text;

    const float line_h = font_->line_height() * text_scale_;
    const float total_h = (float)lines_.size() * line_h;
    float y = bounds_.y + (bounds_.h - total_h) * 0.5f;

    // 裁剪到 label bounds，防止换行文本溢出到相邻控件（滚动条等）。
    // UI-169：仅当 clip 为非退化矩形时 push——push_scissor 对退化矩形(w/h<=0)不入栈，
    // 若此处无条件 pop 会误弹父级裁剪区（如 ScrollBox 的 content_clip）→ GL_SCISSOR_TEST
    // 过早关闭，后续子控件在未裁剪下绘制，滚动内容穿透到标题栏/Header 之上。
    const Rect clip{bounds_.x, bounds_.y, bounds_.w, bounds_.h};
    const bool clip_active = clip.w > 0.f && clip.h > 0.f;
    if (clip_active) renderer.push_scissor(clip);

    for (const std::string& line : lines_) {
        if (!line.empty()) {
            const float text_w = font_->measure_text(line.c_str(), text_scale_);
            float tx = bounds_.x;
            if (align_ == 1)
                tx = bounds_.x + (bounds_.w - text_w) * 0.5f;
            else if (align_ == 2)
                tx = bounds_.x + bounds_.w - text_w;
            // 缩放后的文本在行盒内垂直居中（未缩放时差值为 0，行为不变）
            const float scaled_line_h = font_->line_height() * text_scale_;
            const float ty = y + (line_h - scaled_line_h) * 0.5f;
            font_->draw_text(renderer, line.c_str(), tx, ty, color, text_scale_, bold_);
        }
        y += line_h;
    }

    if (clip_active) renderer.pop_scissor();
}

}  // namespace ui
