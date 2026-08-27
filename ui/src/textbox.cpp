#include "ui/ui/textbox.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/theme.h"

#include <GLFW/glfw3.h>

#include <algorithm>

namespace ui {
namespace {

// 下一个 UTF-8 字符起始下标（跳过后续续字节）
size_t next_cp(const std::string& s, size_t i) {
    if (i >= s.size()) return s.size();
    size_t j = i + 1;
    while (j < s.size() && ((unsigned char)s[j] & 0xC0) == 0x80) ++j;
    return j;
}

// 上一个 UTF-8 字符起始下标
size_t prev_cp(const std::string& s, size_t i) {
    if (i == 0) return 0;
    size_t j = i - 1;
    while (j > 0 && ((unsigned char)s[j] & 0xC0) == 0x80) --j;
    return j;
}

// Unicode 码点 -> UTF-8
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

// 密码掩码串：每个 UTF-8 码点一个 '*'
std::string mask_string(const std::string& s) {
    size_t n = 0;
    for (size_t i = 0; i < s.size(); i = next_cp(s, i)) ++n;
    return std::string(n, '*');
}

}  // namespace

TextBox::TextBox(float x, float y, float w, float h) : Widget(x, y, w, h) {
    focusable = true;
}

void TextBox::set_text(const char* text) {
    text_ = text ? text : "";
    if (caret_ > text_.size()) caret_ = text_.size();
    if (sel_anchor_ > text_.size()) sel_anchor_ = text_.size();
    if (on_change_) on_change_(text_.c_str());
}

void TextBox::move_caret(int delta) {
    if (delta < 0)
        caret_ = prev_cp(text_, caret_);
    else
        caret_ = next_cp(text_, caret_);
}

void TextBox::erase_cp(bool backspace) {
    if (backspace) {
        const size_t start = prev_cp(text_, caret_);
        if (start == caret_) return;
        text_.erase(start, caret_ - start);
        caret_ = start;
    } else {
        const size_t end = next_cp(text_, caret_);
        if (end == caret_) return;
        text_.erase(caret_, end - caret_);
    }
    if (on_change_) on_change_(text_.c_str());
}

float TextBox::caret_offset() const {
    if (!font_) return 0.f;
    const std::string prefix = text_.substr(0, caret_);
    return font_->measure_text(password_ ? mask_string(prefix).c_str() : prefix.c_str());
}

std::string TextBox::selected_text() const {
    if (!has_selection()) return "";
    const size_t s = std::min(sel_anchor_, caret_);
    const size_t e = std::max(sel_anchor_, caret_);
    return text_.substr(s, e - s);
}

void TextBox::erase_selection() {
    if (!has_selection()) return;
    const size_t s = std::min(sel_anchor_, caret_);
    const size_t e = std::max(sel_anchor_, caret_);
    text_.erase(s, e - s);
    caret_ = s;
    sel_anchor_ = s;
}

void TextBox::copy_selection() const {
    if (!has_selection()) return;
    GLFWwindow* w = Window::primary_handle();
    if (!w) return;
    glfwSetClipboardString(w, selected_text().c_str());
}

void TextBox::paste_clipboard() {
    GLFWwindow* w = Window::primary_handle();
    if (!w) return;
    const char* c = glfwGetClipboardString(w);
    if (!c) return;
    std::string s = c;
    // 单行框：换行折叠为空格
    std::replace_if(s.begin(), s.end(),
                    [](char ch) { return ch == '\r' || ch == '\n'; }, ' ');
    if (s.empty()) return;
    erase_selection();
    text_.insert(caret_, s);
    caret_ += s.size();
    sel_anchor_ = caret_;
    if (on_change_) on_change_(text_.c_str());
}

size_t TextBox::hit_test_caret(float x) const {
    if (!font_ || text_.empty()) return 0;
    const float pad = 8.f;
    const float avail = std::max(0.f, bounds_.w - pad * 2);
    const std::string disp = password_ ? mask_string(text_) : text_;
    const float text_w = font_->measure_text(disp.c_str());
    // 与 on_draw 相同的滚动量，保证点击/拖选位置与显示一致
    float scroll = 0.f;
    if (text_w > avail) {
        scroll = std::min(std::max(0.f, caret_offset() - (avail - 2.f)), text_w - avail);
    }
    const float local_x = x - bounds_.x - pad + scroll;
    if (local_x <= 0.f) return 0;
    size_t best = 0;
    float best_dist = std::abs(local_x);
    size_t i = 0;
    while (i < disp.size()) {
        const size_t next = next_cp(disp, i);
        if (next == i) break;
        const std::string prefix = disp.substr(0, next);
        const float w = font_->measure_text(prefix.c_str());
        const float dist = std::abs(local_x - w);
        if (dist < best_dist) { best_dist = dist; best = next; }
        i = next;
    }
    return best;
}

bool TextBox::on_key_down(int key, int mods) {
    const bool shift = (mods & ModShift) != 0;
    const bool ctrl = (mods & ModControl) != 0;

    // Ctrl 组合：全选 / 复制 / 剪切 / 粘贴
    if (ctrl) {
        switch (key) {
            case KeyC:
                copy_selection();
                return true;
            case KeyX:
                copy_selection();
                erase_selection();
                if (on_change_) on_change_(text_.c_str());
                return true;
            case KeyV:
                paste_clipboard();
                return true;
            case KeyA:
                sel_anchor_ = 0;
                caret_ = text_.size();
                return true;
            default:
                return false;
        }
    }

    switch (key) {
        case KeyLeft:
            // 无 Shift 且有选择：光标落到区间起点并清除选择
            if (!shift && has_selection()) {
                caret_ = std::min(sel_anchor_, caret_);
                sel_anchor_ = caret_;
                return true;
            }
            if (shift && !has_selection()) sel_anchor_ = caret_;
            move_caret(-1);
            if (!shift) sel_anchor_ = caret_;
            return true;
        case KeyRight:
            if (!shift && has_selection()) {
                caret_ = std::max(sel_anchor_, caret_);
                sel_anchor_ = caret_;
                return true;
            }
            if (shift && !has_selection()) sel_anchor_ = caret_;
            move_caret(1);
            if (!shift) sel_anchor_ = caret_;
            return true;
        case KeyHome:
            if (shift && !has_selection()) sel_anchor_ = caret_;
            caret_ = 0;
            if (!shift) sel_anchor_ = caret_;
            return true;
        case KeyEnd:
            if (shift && !has_selection()) sel_anchor_ = caret_;
            caret_ = text_.size();
            if (!shift) sel_anchor_ = caret_;
            return true;
        case KeyBackspace:
            if (has_selection()) {
                erase_selection();
                if (on_change_) on_change_(text_.c_str());
            } else {
                erase_cp(true);
            }
            return true;
        case KeyDelete:
            if (has_selection()) {
                erase_selection();
                if (on_change_) on_change_(text_.c_str());
            } else {
                erase_cp(false);
            }
            return true;
        case KeyEnter:
            if (on_enter_) on_enter_();
            return true;
        default:
            return false;
    }
}

bool TextBox::on_char(unsigned int codepoint) {
    // 过滤 C0 控制字符与 DEL（如 Ctrl+V 组合产生的 WM_CHAR 0x16），
    // 可见字符输入不受影响
    if (codepoint < 0x20 || codepoint == 0x7f) return true;
    erase_selection();
    const std::string s = encode_cp(codepoint);
    text_.insert(caret_, s);
    caret_ += s.size();
    sel_anchor_ = caret_;
    if (on_change_) on_change_(text_.c_str());
    return true;
}

void TextBox::set_caret_pos(size_t pos) {
    caret_ = std::min(pos, text_.size());
    sel_anchor_ = caret_;
}

bool TextBox::on_mouse_down(float x, float y, int button) {
    if (button != 0) return Widget::on_mouse_down(x, y, button);
    focus();
    dragging_ = true;
    caret_ = hit_test_caret(x);
    sel_anchor_ = caret_;  // 新按下：清除旧选择并从点击处重新锚定
    return true;
}

bool TextBox::on_mouse_move(float x, float y) {
    if (!dragging_) return Widget::on_mouse_move(x, y);
    caret_ = hit_test_caret(x);  // 拖选扩展（anchor 保持不动）
    return true;
}

bool TextBox::on_mouse_up(float x, float y, int button) {
    (void)x;
    (void)y;
    if (button == 0) dragging_ = false;
    return Widget::on_mouse_up(x, y, button);
}

void TextBox::insert_text(const std::string& s) {
    if (s.empty()) return;
    erase_selection();
    text_.insert(caret_, s);
    caret_ += s.size();
    sel_anchor_ = caret_;
    if (on_change_) on_change_(text_.c_str());
}

bool TextBox::on_drop(float x, float y, const std::vector<std::string>& paths) {
    (void)x;
    (void)y;
    if (!drop_enabled_ || paths.empty()) return false;
    drop_highlight_ = true;
    if (on_drop_cb_) {
        on_drop_cb_(paths);
    } else {
        // 默认：单行插入路径（空格分隔）
        std::string insert;
        for (size_t i = 0; i < paths.size(); ++i) {
            if (i > 0) insert += " ";
            insert += paths[i];
        }
        insert_text(insert);
    }
    return true;
}

void TextBox::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    // 边框 + 内容底（未聚焦灰边白底，hover accent 淡边，聚焦主题色边；禁用灰化）
    Vec4 border_color = t->input_border;
    if (drop_highlight_)
        border_color = t->accent;
    else if (enabled && focused())
        border_color = t->success;
    else if (enabled && hovered_)
        border_color = Vec4{t->accent.r, t->accent.g, t->accent.b, 0.5f};
    else if (!enabled)
        border_color = t->text_muted;
    drop_highlight_ = false;
    renderer.draw_rounded_rect(bounds_, t->radius, border_color);
    const Rect inner{bounds_.x + 1.f, bounds_.y + 1.f, bounds_.w - 2.f, bounds_.h - 2.f};
    renderer.draw_rounded_rect(inner, std::max(0.f, t->radius - 1.f),
                               enabled ? t->input_bg : t->border);

    if (!font_) return;

    const float pad = 8.f;
    const float avail = std::max(0.f, bounds_.w - pad * 2);
    const std::string disp = password_ ? mask_string(text_) : text_;
    const bool show_ph = text_.empty() && !placeholder_.empty();
    const std::string& draw = show_ph ? placeholder_ : disp;
    const Vec4& text_color = !enabled ? t->text_muted : (show_ph ? t->text_muted : t->text);

    const float caret_x = caret_offset();
    const float text_w = font_->measure_text(draw.c_str());

    // 文本超宽时滚动，保证光标可见
    float scroll = 0.f;
    if (text_w > avail) {
        scroll = std::min(std::max(0.f, caret_x - (avail - 2.f)), text_w - avail);
    }

    const float tx = bounds_.x + pad - scroll;
    const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;

    const Rect clip = {bounds_.x + pad, bounds_.y, avail, bounds_.h};
    renderer.push_scissor(clip);
    // 选中区间高亮（accent 半透明，文本下层）
    if (has_selection()) {
        const size_t s = std::min(sel_anchor_, caret_);
        const size_t e = std::max(sel_anchor_, caret_);
        const float x1 = font_->measure_text(disp.substr(0, s).c_str());
        const float x2 = font_->measure_text(disp.substr(0, e).c_str());
        const Rect sel_rect{tx + x1, bounds_.y + 4.f,
                            std::max(1.f, x2 - x1), bounds_.h - 8.f};
        renderer.draw_quad(sel_rect,
                           Vec4{t->accent.r, t->accent.g, t->accent.b, 0.30f});
    }
    if (!draw.empty()) font_->draw_text(renderer, draw.c_str(), tx, ty, text_color);
    // 聚焦光标（2px accent）
    if (focused() && enabled) {
        const float cx = bounds_.x + pad + caret_x - scroll;
        const Rect caret_rect = {cx, bounds_.y + 5, 2, bounds_.h - 10};
        renderer.draw_quad(caret_rect, t->accent);
    }
    renderer.pop_scissor();
}

}  // namespace ui
