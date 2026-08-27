#include "ui/ui/code.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <cstdio>

namespace ui {

namespace {
constexpr float kHeaderH = 30.f;
constexpr float kPad = 10.f;
constexpr float kChildGap = 8.f;
}  // namespace

Code::Code(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void Code::set_code(const char* code) {
    mark_dirty();
        lines_.clear();
    if (!code) return;
    std::string line;
    for (const char* p = code; *p; ++p) {
        if (*p == '\n') {
            lines_.push_back(line);
            line.clear();
        } else {
            line += *p;
        }
    }
    if (!line.empty()) lines_.push_back(line);
}

float Code::preferred_height() const {
    const float line_h = font_ ? font_->line_height() + 4.f : line_h_;
    float h = kHeaderH + (float)lines_.size() * line_h;
    for (const Widget* c : children()) {
        if (!c->visible) continue;
        h += c->preferred_height() + kChildGap;
    }
    if (max_height_ > 0.f) h = std::max(kHeaderH, std::min(h, max_height_));
    return h;
}

void Code::on_layout() {
    const float line_h = font_ ? font_->line_height() + 4.f : line_h_;
    float y = bounds_.y + kHeaderH + (float)lines_.size() * line_h;
    const float child_w = bounds_.w - 2.f * kPad;
    for (Widget* c : children()) {
        if (!c->visible) continue;
        const float ch = c->preferred_height();
        c->set_bounds(bounds_.x + kPad, y, child_w, ch);
        y += ch + kChildGap;
    }
}

bool Code::on_mouse_scroll(float x, float y, float dx, float dy) {
    if (dy == 0.f) return false;
    const float max_scroll = preferred_height() - bounds_.h;
    if (max_scroll <= 0.f) return false;
    scroll_ -= dy * 24.f;
    if (scroll_ < 0.f) scroll_ = 0.f;
    if (scroll_ > max_scroll) scroll_ = max_scroll;
    mark_dirty();
    return true;
}

void Code::draw(Renderer& renderer) {
    if (!visible) return;
    on_draw(renderer);
    renderer.push_scissor(Rect{bounds_.x, bounds_.y + kHeaderH, bounds_.w, bounds_.h - kHeaderH});
    for (Widget* child : children_) {
        if (child->visible) child->draw(renderer);
    }
    renderer.pop_scissor();
    // 与基类 Widget::draw 末尾一致：清掉本节点脏标记，避免容器常脏短路 mark_dirty()。
    dirty_ = false;
    dirty_children_ = false;
}

void Code::on_draw(Renderer& renderer) {
    const bool notepad = (skin_ == Skin::kNotepad);
    const Vec4 bg = notepad ? Vec4{0.08f, 0.13f, 0.20f, 1.0f} : Vec4{0.16f, 0.17f, 0.19f, 1.0f};
    const Vec4 header_bg = notepad ? Vec4{0.06f, 0.10f, 0.16f, 1.0f} : Vec4{0.12f, 0.13f, 0.15f, 1.0f};
    renderer.draw_quad(bounds_, bg);

    if (!font_) return;
    const float line_h = font_->line_height() + 4.f;
    line_h_ = line_h;
    const float pad = kPad;

    // 标题条：左侧 title / 右侧 lang
    renderer.draw_quad(Rect{bounds_.x, bounds_.y, bounds_.w, kHeaderH}, header_bg);
    float hx = bounds_.x + pad;
    if (!title_.empty()) {
        const Vec4 tcol = notepad ? Vec4{0.55f, 0.80f, 0.60f, 1.0f} : Vec4{0.82f, 0.84f, 0.86f, 1.0f};
        font_->draw_text(renderer, title_.c_str(), hx, bounds_.y + 6.f, tcol);
        hx += font_->measure_text(title_.c_str()) + 12.f;
    }
    if (!lang_.empty()) {
        const Vec4 lcol = notepad ? Vec4{0.40f, 0.62f, 0.90f, 1.0f} : Vec4{0.5f, 0.8f, 0.55f, 1.0f};
        font_->draw_text(renderer, lang_.c_str(), hx, bounds_.y + 6.f, lcol);
    }

    // 行号列宽（最长行号 + 间距）
    char numbuf[16];
    std::snprintf(numbuf, sizeof(numbuf), "%d.", (int)lines_.size());
    const float num_w = font_->measure_text(numbuf) + 10.f;

    const float top = bounds_.y + kHeaderH;
    const float content_h = bounds_.h - kHeaderH;
    renderer.push_scissor(Rect{bounds_.x, top, bounds_.w, content_h});
    const int first = (int)(scroll_ / line_h);
    const int visible = (int)(content_h / line_h) + 1;
    const Vec4 num_col = notepad ? Vec4{0.40f, 0.55f, 0.70f, 1.0f} : Vec4{0.4f, 0.42f, 0.46f, 1.0f};
    const Vec4 code_col = notepad ? Vec4{0.70f, 0.86f, 1.0f, 1.0f} : Vec4{0.82f, 0.84f, 0.86f, 1.0f};
    const Vec4 comment_col = notepad ? Vec4{0.45f, 0.75f, 0.55f, 1.0f} : Vec4{0.45f, 0.68f, 0.48f, 1.0f};
    for (int i = 0; i < visible; ++i) {
        const int idx = first + i;
        if (idx < 0 || idx >= (int)lines_.size()) break;
        const float ly = top + idx * line_h - scroll_;
        std::snprintf(numbuf, sizeof(numbuf), "%d.", idx + 1);
        font_->draw_text(renderer, numbuf, bounds_.x + 6.f, ly + 1.f, num_col);
        const std::string& line = lines_[(size_t)idx];
        // 简单高亮：行首 # 注释变绿（layui-code 近似）
        if (!line.empty() && line[0] == '#') {
            font_->draw_text(renderer, line.c_str(), bounds_.x + num_w, ly + 1.f, comment_col);
        } else {
            font_->draw_text(renderer, line.c_str(), bounds_.x + num_w, ly + 1.f, code_col);
        }
    }
    renderer.pop_scissor();
}

}  // namespace ui
