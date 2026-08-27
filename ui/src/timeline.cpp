#include "ui/ui/timeline.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <chrono>

namespace ui {

Timeline::Timeline(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-timeline");
}

void Timeline::add_item(const char* title, const char* content) {
    mark_dirty();
        Item it;
    it.title = title ? title : "";
    if (content && *content) it.lines.push_back({LineKind::Text, content, 0});
    items_.push_back(std::move(it));
}

void Timeline::add_item_rich(const char* title, bool title_large, std::vector<ContentLine> lines) {
    Item it;
    it.title = title ? title : "";
    it.title_large = title_large;
    it.lines = std::move(lines);
    items_.push_back(std::move(it));
}

void Timeline::set_axis_icon(uint32_t codepoint) {
    mark_dirty();
        if (!items_.empty()) items_.back().axis_icon = codepoint;
}

void Timeline::set_axis_rotate(float degrees_per_sec) {
    mark_dirty();
        if (!items_.empty()) items_.back().axis_rotate_dps = degrees_per_sec;
}

float Timeline::preferred_height() const {
    if (!font_) return bounds_.h;
    const float line_h = font_->line_height();
    float total = 0.f;
    for (const Item& it : items_) {
        float h = line_h;                     // 标题行
        h += static_cast<float>(it.lines.size()) * line_h;  // 内容行
        total += h + 20.f;                    // item padding-bottom
    }
    return total;
}

bool Timeline::on_mouse_move(float x, float y) {
    if (!font_ || items_.empty() || !hovered()) {
        if (hovered_index_ != -1) hovered_index_ = -1;
        return false;
    }
    const float line_h = font_->line_height();
    const float axis_x = bounds_.x + 10.f;  // 轴线 x
    int found = -1;
    float yy = bounds_.y;
    for (size_t i = 0; i < items_.size(); ++i) {
        const Item& it = items_[i];
        const float body_h = static_cast<float>(it.lines.size()) * line_h;
        const float item_h = line_h + body_h + 20.f;
        const float cy = yy + line_h * 0.5f;
        // 命中 20px 白圆 ± 半径内（放大一点便于悬停）
        const float dx = x - axis_x, dy = y - cy;
        if (dx * dx + dy * dy <= 12.f * 12.f) { found = (int)i; break; }
        yy += item_h;
    }
    if (found != hovered_index_) hovered_index_ = found;
    return false;  // 不捕获，让兄弟控件也能响应
}

void Timeline::on_draw(Renderer& renderer) {
    if (!font_ || items_.empty()) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    const float line_h = font_->line_height();
    const float axis_x = bounds_.x + 10.f;  // 轴线 x（padding-left 5 + 轴心 5）
    const float content_x = bounds_.x + 25.f;
    const float line_half = 0.5f;

    float y = bounds_.y;
    for (size_t i = 0; i < items_.size(); ++i) {
        const Item& it = items_[i];
        const float body_h = static_cast<float>(it.lines.size()) * line_h;
        const float item_h = line_h + body_h + 20.f;
        const float cy = y + line_h * 0.5f;

        // 左缘竖线 #eee：从条目顶部到内容底部（最后一条到轴心为止）
        const float line_top = y;
        const float line_bot = (i == items_.size() - 1) ? cy : y + item_h;
        renderer.draw_quad({axis_x - line_half, line_top, 1.f, line_bot - line_top}, t->border);

        // 轴点：20px 白圆（layui-timeline-axis）+ 中心图标
        renderer.draw_rounded_rect({axis_x - 10.f, cy - 10.f, 20.f, 20.f}, 10.f, t->panel);
        const Vec4 axis_color = ((int)i == hovered_index_) ? t->danger : t->success;
        float rot = 0.f;
        if (it.axis_rotate_dps > 0.f) {
            const float elapsed =
                std::chrono::duration<float>(std::chrono::steady_clock::now() - anim_start_).count();
            rot = elapsed * it.axis_rotate_dps * 3.14159265f / 180.f;  // 弧度
        }
        font_->draw_icon(renderer, it.axis_icon, axis_x, cy, 14.f, axis_color, rot);
        // hover 描边（镜轴 :hover 变红 #FF5722）
        if ((int)i == hovered_index_) {
            renderer.draw_rounded_rect_outline({axis_x - 10.f, cy - 10.f, 20.f, 20.f}, 10.f, 1.5f, t->danger);
        }

        // 标题：h3 大标题 18px #333 或普通标题
        const float ty = y + (line_h - font_->line_height()) * 0.5f;
        if (it.title_large) {
            font_->draw_text(renderer, it.title.c_str(), content_x, ty, Vec4{0.20f, 0.20f, 0.20f, 1.0f});
        } else {
            font_->draw_text(renderer, it.title.c_str(), content_x, ty, t->text);
        }

        // 内容行
        float cy2 = y + line_h;
        for (const ContentLine& ln : it.lines) {
            switch (ln.kind) {
                case LineKind::Text:
                    font_->draw_text(renderer, ln.text.c_str(), content_x, cy2, t->text_muted);
                    break;
                case LineKind::Bullet:
                    font_->draw_text(renderer, "\u2022", content_x, cy2, t->text_muted);
                    font_->draw_text(renderer, ln.text.c_str(), content_x + 15.f, cy2, t->text_muted);
                    break;
                case LineKind::Em:
                    font_->draw_text(renderer, ln.text.c_str(), content_x, cy2, t->text_muted);
                    break;
                case LineKind::Icon:
                    font_->draw_icon(renderer, ln.codepoint, content_x + font_->measure_text(ln.text.c_str()), cy2 + line_h * 0.5f, 14.f, t->text_muted);
                    font_->draw_text(renderer, ln.text.c_str(), content_x, cy2, t->text_muted);
                    break;
            }
            cy2 += line_h;
        }

        y += item_h;
    }
}

}  // namespace ui
