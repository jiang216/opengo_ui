#include "ui/ui/textarea.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/theme.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <vector>

namespace ui {
namespace {

constexpr float kPad = 8.f;
constexpr float kSelAlpha = 0.30f;  // 选区高亮透明度

// ---- 默认剪贴板（UI-103）：未注入 on_copy_/on_paste_ 回调时的内置实现 ----

void default_copy(const std::string& text) {
    GLFWwindow* w = Window::primary_handle();
    if (w) glfwSetClipboardString(w, text.c_str());
}

std::string default_paste() {
    GLFWwindow* w = Window::primary_handle();
    if (!w) return {};
    const char* c = glfwGetClipboardString(w);
    return c ? std::string(c) : std::string();
}

// ---- UTF-8 工具 ----

size_t next_cp(const std::string& s, size_t i) {
    if (i >= s.size()) return s.size();
    size_t j = i + 1;
    while (j < s.size() && ((unsigned char)s[j] & 0xC0) == 0x80) ++j;
    return j;
}

size_t prev_cp(const std::string& s, size_t i) {
    if (i == 0) return 0;
    size_t j = i - 1;
    while (j > 0 && ((unsigned char)s[j] & 0xC0) == 0x80) --j;
    return j;
}

unsigned int decode_cp(const std::string& s, size_t i) {
    const unsigned char c = (unsigned char)s[i];
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) return ((c & 0x1F) << 6) | ((unsigned char)s[i + 1] & 0x3F);
    if ((c & 0xF0) == 0xE0)
        return ((c & 0x0F) << 12) | (((unsigned char)s[i + 1] & 0x3F) << 6) |
               ((unsigned char)s[i + 2] & 0x3F);
    return ((c & 0x07) << 18) | (((unsigned char)s[i + 1] & 0x3F) << 12) |
           (((unsigned char)s[i + 2] & 0x3F) << 6) | ((unsigned char)s[i + 3] & 0x3F);
}

std::string encode_cp(unsigned int cp) {
    std::string s;
    if (cp < 0x80) {
        s += (char)cp;
    } else if (cp < 0x800) {
        s += (char)(0xC0 | (cp >> 6));
        s += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        s += (char)(0xE0 | (cp >> 12));
        s += (char)(0x80 | ((cp >> 6) & 0x3F));
        s += (char)(0x80 | (cp & 0x3F));
    } else {
        s += (char)(0xF0 | (cp >> 18));
        s += (char)(0x80 | ((cp >> 12) & 0x3F));
        s += (char)(0x80 | ((cp >> 6) & 0x3F));
        s += (char)(0x80 | (cp & 0x3F));
    }
    return s;
}

std::vector<LineSpan> wrap_lines(const std::string& text, const Font* font, float avail,
                                 float* line_h) {
    const float lh = font ? font->line_height() + 2.f : 16.f;
    if (line_h) *line_h = lh;
    std::vector<LineSpan> out;
    const size_t n = text.size();
    size_t i = 0;
    while (i < n) {
        if (text[i] == '\n') {
            out.push_back({i, i});
            ++i;
            continue;
        }
        const size_t start = i;
        float w = 0.f;
        bool wrapped = false;
        while (i < n && text[i] != '\n') {
            const size_t k = next_cp(text, i);
            const float cw = font ? font->char_width(decode_cp(text, i)) : 8.f;
            if (w > 0.f && w + cw > avail) {
                wrapped = true;
                break;
            }
            w += cw;
            i = k;
        }
        out.push_back({start, i});
        if (!wrapped && i < n) ++i;
    }
    if (out.empty()) out.push_back({0, 0});
    return out;
}

// 带缓存的 wrap_lines：命中缓存时跳过全部逐字符宽度计算
std::vector<LineSpan> cached_wrap_lines(TextArea* self, const std::string& text,
                                         const Font* font, float avail, float* line_h) {
    if (self && self->lines_cache_valid()) {
        if (line_h) *line_h = self->cached_line_h();
        return self->cached_lines();
    }
    auto result = wrap_lines(text, font, avail, line_h);
    if (self) {
        self->update_lines_cache(result, line_h ? *line_h : (font ? font->line_height() + 2.f : 16.f));
    }
    return result;
}

}  // namespace

TextArea::TextArea(float x, float y, float w, float h) : Widget(x, y, w, h) {
    focusable = true;
}

// ---- Selection helpers ----

std::string TextArea::selected_text() const {
    if (!has_selection_ || sel_start_ == sel_end_) return {};
    size_t a = std::min(sel_start_, sel_end_);
    size_t b = std::max(sel_start_, sel_end_);
    if (a >= text_.size()) return {};
    if (b > text_.size()) b = text_.size();
    return text_.substr(a, b - a);
}

void TextArea::selection_range(size_t& a, size_t& b) const {
    a = std::min(sel_start_, sel_end_);
    b = std::max(sel_start_, sel_end_);
}

void TextArea::erase_selection() {
    mark_dirty();
    if (!has_selection_ || sel_start_ == sel_end_) return;
    size_t a = std::min(sel_start_, sel_end_);
    size_t b = std::max(sel_start_, sel_end_);
    if (b > text_.size()) b = text_.size();
    text_.erase(a, b - a);
    caret_ = a;
    sel_start_ = sel_end_ = sel_anchor_ = caret_;
    has_selection_ = false;
    invalidate_lines();
}

void TextArea::replace_selection(const std::string& repl) {
    mark_dirty();
    if (has_selection_ && sel_start_ != sel_end_) {
        size_t a = std::min(sel_start_, sel_end_);
        size_t b = std::max(sel_start_, sel_end_);
        if (b > text_.size()) b = text_.size();
        text_.replace(a, b - a, repl);
        caret_ = a + repl.size();
    } else {
        text_.insert(caret_, repl);
        caret_ += repl.size();
    }
    sel_start_ = sel_end_ = sel_anchor_ = caret_;
    has_selection_ = false;
    invalidate_lines();
    if (on_change_) on_change_(text_.c_str());
}

void TextArea::clamp_scroll(float line_h) {
    float content_h = 0.f;
    {
        const float avail = avail_width();
        auto lines = cached_wrap_lines(this, text_, font_, avail, nullptr);
        content_h = (float)lines.size() * line_h + 2.f * kPad;
    }
    const float vis = bounds_.h;
    const float max_scroll = std::max(0.f, content_h - vis);
    scroll_ = std::max(0.f, std::min(scroll_, max_scroll));
}

// ---- 垂直滚动条（共享样式见 ui/ui/scrollbar_style.h）----

float TextArea::avail_width() const {
    return std::max(1.f, bounds_.w - 2.f * kPad - scrollbar::kBarW - scrollbar::kMargin);
}

float TextArea::content_height(float line_h) const {
    const auto lines = cached_wrap_lines(const_cast<TextArea*>(this), text_, font_, avail_width(), nullptr);
    return (float)lines.size() * line_h + 2.f * kPad;
}

// ---- Text API ----

void TextArea::set_text(const char* text) {
    std::string next = text ? text : "";
    if (text_ == next) return;
    text_ = std::move(next);
    if (caret_ > text_.size()) caret_ = text_.size();
    sel_start_ = sel_end_ = sel_anchor_ = caret_;
    has_selection_ = false;
    invalidate_lines();
    mark_dirty();
    if (on_change_) on_change_(text_.c_str());
}

void TextArea::scroll_to_end() {
    if (!font_) return;
    mark_dirty();
    float line_h = 0.f;
    auto lines = cached_wrap_lines(this, text_, font_, avail_width(), &line_h);
    const float content_h = (float)lines.size() * line_h + 2.f * kPad;
    const float vis = bounds_.h;
    scroll_ = std::max(0.f, content_h - vis);
}

void TextArea::erase_cp(bool backspace) {
    mark_dirty();
    if (has_selection_ && sel_start_ != sel_end_) {
        erase_selection();
        if (on_change_) on_change_(text_.c_str());
        return;
    }
    if (backspace) {
        if (caret_ == 0) return;
        const size_t start = prev_cp(text_, caret_);
        text_.erase(start, caret_ - start);
        caret_ = start;
    } else {
        if (caret_ >= text_.size()) return;
        const size_t end = next_cp(text_, caret_);
        text_.erase(caret_, end - caret_);
    }
    sel_start_ = sel_end_ = sel_anchor_ = caret_;
    has_selection_ = false;
    invalidate_lines();
    if (on_change_) on_change_(text_.c_str());
}

void TextArea::ensure_caret_visible(float line_h, size_t row) {
    const float content_h = bounds_.h - 2.f * kPad;
    const float caret_y = (float)row * line_h;
    if (caret_y < scroll_) scroll_ = caret_y;
    if (caret_y + line_h > scroll_ + content_h) scroll_ = caret_y + line_h - content_h;
    if (scroll_ < 0.f) scroll_ = 0.f;
}

size_t TextArea::caret_from_point(float local_y, float local_x) const {
    if (!font_) return text_.size();
    float line_h = 0.f;
    const auto lines = cached_wrap_lines(const_cast<TextArea*>(this), text_, font_, avail_width(), &line_h);
    int row = (int)((local_y - kPad + scroll_) / line_h);
    if (row < 0) row = 0;
    if (row >= (int)lines.size()) row = (int)lines.size() - 1;
    const LineSpan& ls = lines[(size_t)row];
    float x = kPad;
    size_t last = ls.start;
    size_t j = ls.start;
    while (j < ls.end) {
        const size_t k = next_cp(text_, j);
        const float cw = font_->char_width(decode_cp(text_, j));
        if (x + cw * 0.5f > local_x) break;
        x += cw;
        last = k;
        j = k;
    }
    return last;
}

float TextArea::offset_to_y(size_t offset, float line_h,
                            const std::vector<LineSpan>& lines) const {
    for (size_t i = 0; i < lines.size(); ++i) {
        if (offset <= lines[i].end) return (float)i * line_h;
    }
    return (float)lines.size() * line_h;
}

// ---- Mouse events ----

bool TextArea::on_mouse_down(float x, float y, int button) {
    mark_dirty();
    if (button != 0) return false;
    mx_ = x - bounds_.x;
    my_ = y - bounds_.y;
    // 滚动条命中优先（不抢焦点、不移动光标）
    {
        float line_h = font_ ? font_->line_height() + 2.f : 16.f;
        const float ch = content_height(line_h);
        if (ch > bounds_.h) {
            const Rect track = scrollbar::track_rect(bounds_, scrollbar::kBarWHover);
            if (scrollbar::hit(track, x, y)) {
                scrollbar::touch(sb_);
                sb_.dragging = true;
                capture_mouse();
                scroll_ = scrollbar::scroll_from_y(track, y, ch - bounds_.h);
                clamp_scroll(line_h);
                return true;
            }
        }
    }
    focus();
    float local_x = x - bounds_.x;
    float local_y = y - bounds_.y;
    caret_ = caret_from_point(local_y, local_x);
    sel_anchor_ = caret_;
    sel_start_ = sel_end_ = caret_;
    has_selection_ = false;
    selecting_ = true;
    capture_mouse();

    float line_h = 0.f;
    const auto lines = cached_wrap_lines(this, text_, font_, avail_width(), &line_h);
    for (size_t r = 0; r < lines.size(); ++r) {
        if (caret_ >= lines[r].start && caret_ <= lines[r].end) {
            ensure_caret_visible(line_h, r);
            break;
        }
    }
    return true;
}

bool TextArea::on_mouse_move(float x, float y) {
    mx_ = x - bounds_.x;
    my_ = y - bounds_.y;
    if (sb_.dragging) {
        mark_dirty();
        float line_h = font_ ? font_->line_height() + 2.f : 16.f;
        const Rect track = scrollbar::track_rect(bounds_, scrollbar::kBarWHover);
        scroll_ = scrollbar::scroll_from_y(track, y, content_height(line_h) - bounds_.h);
        clamp_scroll(line_h);
        return true;
    }
    if (!selecting_) return false;
    mark_dirty();
    float local_x = x - bounds_.x;
    float local_y = y - bounds_.y;
    float line_h = font_ ? font_->line_height() + 2.f : 16.f;

    // 边缘自动滚屏（直接操作 scroll_，即时生效）
    if (local_y < scroll_margin_ && scroll_ > 0.f) {
        scroll_ -= 3.f;
        if (scroll_ < 0.f) scroll_ = 0.f;
    } else if (local_y > bounds_.h - scroll_margin_) {
        const float avail = avail_width();
        auto lines = cached_wrap_lines(this, text_, font_, avail, nullptr);
        float content_h = (float)lines.size() * line_h + 2.f * kPad;
        float max_scroll = std::max(0.f, content_h - bounds_.h);
        scroll_ += 3.f;
        if (scroll_ > max_scroll) scroll_ = max_scroll;
    }

    // clamp local_y 到可见范围
    if (local_y < 0.f) local_y = 0.f;
    if (local_y > bounds_.h) local_y = bounds_.h;

    caret_ = caret_from_point(local_y, local_x);
    sel_end_ = caret_;
    has_selection_ = (sel_start_ != sel_end_);
    return true;
}

bool TextArea::on_mouse_up(float x, float y, int button) {
    (void)x; (void)y;
    if (button != 0) return false;
    if (sb_.dragging) {
        sb_.dragging = false;
        release_mouse_capture();
        return true;
    }
    if (!selecting_) return false;
    mark_dirty();
    selecting_ = false;
    release_mouse_capture();
    has_selection_ = (sel_start_ != sel_end_);
    return true;
}

bool TextArea::on_mouse_scroll(float x, float y, float dx, float dy) {
    (void)x; (void)y; (void)dx;
    if (dy == 0.f) return false;
    float line_h = 0.f;
    const auto lines = cached_wrap_lines(this, text_, font_, avail_width(), &line_h);
    const float content_h = (float)lines.size() * line_h + 2.f * kPad;
    const float max_scroll = std::max(0.f, content_h - bounds_.h);
    if (max_scroll <= 0.f) return false;
    mark_dirty();
    scroll_ -= dy * 24.f;
    scroll_ = std::max(0.f, std::min(scroll_, max_scroll));
    scrollbar::touch(sb_);  // 滚动活动：滚动条浮现并保持 2s
    return true;
}

void TextArea::insert_text(const std::string& s) {
    mark_dirty();
        if (s.empty()) return;
    replace_selection(s);
    if (on_change_) on_change_(text_.c_str());
}

bool TextArea::on_drop(float x, float y, const std::vector<std::string>& paths) {
    (void)x;
    (void)y;
    if (!drop_enabled_ || paths.empty()) return false;
    mark_dirty();
    drop_highlight_ = true;
    if (on_drop_cb_) {
        on_drop_cb_(paths);
    } else {
        // 默认：每行一个路径
        std::string insert;
        for (size_t i = 0; i < paths.size(); ++i) {
            if (i > 0) insert += '\n';
            insert += paths[i];
        }
        insert_text(insert);
    }
    return true;
}

// ---- Keyboard ----

bool TextArea::on_key_down(int key, int mods) {
    if (!font_) return false;
    mark_dirty();
    bool ctrl = (mods & ModControl) != 0;
    bool shift = (mods & ModShift) != 0;
    const float avail = avail_width();
    const float line_h = font_->line_height() + 2.f;
    const auto lines = cached_wrap_lines(this, text_, font_, avail, nullptr);
    size_t row = 0;
    for (size_t r = 0; r < lines.size(); ++r) {
        if (caret_ >= lines[r].start && caret_ <= lines[r].end) {
            row = r;
            break;
        }
    }

    // ---- Ctrl 快捷键 ----
    if (ctrl && key == 'A') {
        sel_start_ = 0;
        sel_end_ = text_.size();
        sel_anchor_ = 0;
        caret_ = text_.size();
        has_selection_ = !text_.empty();
        return true;
    }
    if (ctrl && key == 'C') {
        if (has_selection_ && sel_start_ != sel_end_) {
            if (on_copy_)
                on_copy_(selected_text());
            else
                default_copy(selected_text());  // UI-103：内置剪贴板兜底
        }
        return true;
    }
    if (ctrl && key == 'X') {
        if (has_selection_ && sel_start_ != sel_end_) {
            if (readonly_) return true;
            if (on_copy_)
                on_copy_(selected_text());
            else
                default_copy(selected_text());
            erase_selection();
            if (on_change_) on_change_(text_.c_str());
        }
        return true;
    }
    if (ctrl && key == 'V') {
        if (readonly_) return true;
        std::string clip = on_paste_ ? on_paste_() : default_paste();  // UI-103 兜底
        if (!clip.empty()) replace_selection(clip);
        return true;
    }

    // ---- Shift+方向键：扩展选区 ----
    auto extend_selection = [&](size_t new_pos) {
        if (shift) {
            if (!has_selection_) sel_anchor_ = caret_;
            caret_ = new_pos;
            sel_end_ = caret_;
            has_selection_ = (sel_start_ != sel_end_);
        } else {
            if (has_selection_) {
                // collapse: 移到选区较远端
                size_t a = std::min(sel_start_, sel_end_);
                size_t b = std::max(sel_start_, sel_end_);
                caret_ = (new_pos <= a) ? a : b;
            } else {
                caret_ = new_pos;
            }
            sel_start_ = sel_end_ = sel_anchor_ = caret_;
            has_selection_ = false;
        }
    };

    switch (key) {
        case KeyLeft: {
            size_t np = (caret_ > 0) ? prev_cp(text_, caret_) : 0;
            extend_selection(np);
            break;
        }
        case KeyRight: {
            size_t np = (caret_ < text_.size()) ? next_cp(text_, caret_) : text_.size();
            extend_selection(np);
            break;
        }
        case KeyHome:
            extend_selection(lines[row].start);
            break;
        case KeyEnd:
            extend_selection(lines[row].end);
            break;
        case KeyUp:
        case KeyDown: {
            float col = 0.f;
            for (size_t j = lines[row].start; j < caret_; j = next_cp(text_, j))
                col += font_->char_width(decode_cp(text_, j));
            const int dir = (key == KeyUp) ? -1 : 1;
            const int target = (int)row + dir;
            if (target >= 0 && target < (int)lines.size()) {
                size_t new_caret = lines[(size_t)target].start;
                float acc = 0.f;
                size_t j = lines[(size_t)target].start;
                while (j < lines[(size_t)target].end) {
                    const size_t k = next_cp(text_, j);
                    const float cw = font_->char_width(decode_cp(text_, j));
                    if (acc + cw * 0.5f > col) break;
                    acc += cw;
                    new_caret = k;
                    j = k;
                }
                extend_selection(new_caret);
            }
            break;
        }
        case KeyBackspace:
            if (readonly_ && has_selection_) return true;
            erase_cp(true);
            break;
        case KeyDelete:
            if (readonly_ && has_selection_) return true;
            erase_cp(false);
            break;
        case KeyEnter: {
            if (readonly_) return true;
            replace_selection("\n");
            break;
        }
        default:
            return false;
    }
    ensure_caret_visible(line_h, row);
    return true;
}

bool TextArea::on_char(unsigned int codepoint) {
    if (codepoint < 0x20) return false;
    if (readonly_) return true;  // 吞掉输入但不修改
    mark_dirty();
    const std::string s = encode_cp(codepoint);
    replace_selection(s);
    float line_h = font_ ? font_->line_height() + 2.f : 16.f;
    const auto lines = cached_wrap_lines(this, text_, font_, avail_width(), nullptr);
    size_t row = 0;
    for (size_t r = 0; r < lines.size(); ++r) {
        if (caret_ >= lines[r].start && caret_ <= lines[r].end) {
            row = r;
            break;
        }
    }
    ensure_caret_visible(line_h, row);
    return true;
}

// ---- Drawing ----

void TextArea::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    // 边框 + 内容底
    Vec4 border_color = t->input_border;
    if (drop_highlight_)
        border_color = t->accent;
    else if (enabled && focused())
        border_color = t->success;
    else if (!enabled)
        border_color = t->text_muted;
    drop_highlight_ = false;
    renderer.draw_rounded_rect(bounds_, t->radius, border_color);
    const Rect inner{bounds_.x + 1.f, bounds_.y + 1.f, bounds_.w - 2.f, bounds_.h - 2.f};
    renderer.draw_rounded_rect(inner, std::max(0.f, t->radius - 1.f),
                               enabled ? t->input_bg : t->border);

    if (!font_) return;

    const float avail = avail_width();
    float line_h = 0.f;
    const auto lines = cached_wrap_lines(this, text_, font_, avail, &line_h);

    const Rect clip{bounds_.x + 1.f, bounds_.y + 1.f, std::max(1.f, bounds_.w - 2.f),
                    std::max(1.f, bounds_.h - 2.f)};
    renderer.push_scissor(clip);

    const Vec4 text_color = enabled ? t->text : t->text_muted;
    const float top = bounds_.y + kPad - scroll_;
    const float text_x = bounds_.x + kPad;

    // ---- 选区高亮 ----
    if (has_selection_ && sel_start_ != sel_end_) {
        size_t a = std::min(sel_start_, sel_end_);
        size_t b = std::max(sel_start_, sel_end_);
        if (b > text_.size()) b = text_.size();

        // 找到 a、b 所在行
        int a_row = 0, b_row = 0;
        for (size_t r = 0; r < lines.size(); ++r) {
            if (a >= lines[r].start && a <= lines[r].end) a_row = (int)r;
            if (b >= lines[r].start && b <= lines[r].end) b_row = (int)r;
        }

        Vec4 sel_color{t->accent.r, t->accent.g, t->accent.b, kSelAlpha};
        for (int li = a_row; li <= b_row; ++li) {
            if (li < 0 || li >= (int)lines.size()) continue;
            const LineSpan& ls = lines[(size_t)li];
            float sx = kPad;
            float ex = kPad;
            // 计算 seg_start 的 x
            if (a > ls.start || li > a_row) {
                sx = kPad;
                for (size_t j = ls.start; j < a && j < ls.end; j = next_cp(text_, j))
                    sx += font_->char_width(decode_cp(text_, j));
                if (li > a_row) {
                    // 整行从头开始
                    sx = kPad;
                }
            }
            // 计算 seg_end 的 x
            if (b < ls.end || li < b_row) {
                ex = kPad;
                size_t limit = (li == b_row) ? b : ls.end;
                for (size_t j = ls.start; j < limit && j < ls.end; j = next_cp(text_, j))
                    ex += font_->char_width(decode_cp(text_, j));
            } else {
                // 整行到尾
                ex = kPad;
                for (size_t j = ls.start; j < ls.end; j = next_cp(text_, j))
                    ex += font_->char_width(decode_cp(text_, j));
            }
            float sy = top + (float)li * line_h;
            float sw = ex - sx;
            if (sw < 1.f) sw = 1.f;
            renderer.draw_quad(Rect{text_x + sx - kPad, sy, sw, line_h}, sel_color);
        }
    }

    // ---- 文字（只渲染可视行，避免 5000 行全量遍历）----
    if (text_.empty() && !placeholder_.empty()) {
        font_->draw_text(renderer, placeholder_.c_str(), text_x, top, t->text_muted);
    } else {
        const float vis_top = scroll_;
        const float vis_bot = scroll_ + bounds_.h;
        const int first_visible = std::max(0, (int)(vis_top / line_h) - 1);
        const int last_visible = std::min((int)lines.size() - 1,
                                          (int)(vis_bot / line_h) + 1);
        for (int r = first_visible; r <= last_visible; ++r) {
            const LineSpan& ls = lines[(size_t)r];
            const float ly = top + (float)r * line_h;
            if (ls.end > ls.start) {
                font_->draw_text(renderer, text_.c_str() + ls.start, text_x, ly, text_color,
                                 (int)(ls.end - ls.start));
            }
        }
    }

    // ---- 光标（聚焦 + 无选区时）----
    if (focused() && enabled && !has_selection_) {
        size_t cr = 0;
        for (size_t r = 0; r < lines.size(); ++r) {
            if (caret_ >= lines[r].start && caret_ <= lines[r].end) {
                cr = r;
                break;
            }
        }
        float col = 0.f;
        for (size_t j = lines[cr].start; j < caret_; j = next_cp(text_, j))
            col += font_->char_width(decode_cp(text_, j));
        const float caret_y = top + (float)cr * line_h;
        renderer.draw_quad(Rect{text_x + col, caret_y, 1.6f, line_h}, t->accent);
    }

    renderer.pop_scissor();

    // ---- 垂直滚动条（overlay 风格：悬停/拖拽/2s 内滚过才显示；三态色见 scrollbar_style.h）----
    {
        const float ch = content_height(line_h);
        if (ch > bounds_.h && scrollbar::active(sb_, hovered())) {
            const bool over_bar =
                hovered() && mx_ >= bounds_.w - scrollbar::kMargin - scrollbar::kBarWHover;
            const Rect track = scrollbar::track_rect(bounds_, scrollbar::bar_w(sb_, over_bar));
            const Rect thumb = scrollbar::thumb_rect(track, ch, bounds_.h, scroll_);
            scrollbar::draw(renderer, t, track, thumb, sb_, over_bar);
        }
    }
}

}  // namespace ui
