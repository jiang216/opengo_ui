#include "ui/ui/button.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/theme.h"

#include <algorithm>

namespace ui {

Button::Button(float x, float y, float w, float h, const char* text)
    : Widget(x, y, w, h), text_(text ? text : "") {
    focusable = true;
}

void Button::set_icon(const char* icon_name) {
    mark_dirty();
        icon_cp_ = icon_name && *icon_name ? layui_icon::from_class(icon_name) : 0;
}

void Button::set_badge(const char* text) {
    mark_dirty();
        has_badge_ = true;
    has_dot_ = false;
    badge_text_ = text ? text : "";
}

void Button::set_dot_badge() {
    mark_dirty();
        has_dot_ = true;
    has_badge_ = false;
    badge_text_.clear();
}

// 按下时消费事件并捕获鼠标，防止下拉面板选项与下层控件重叠时事件穿透
// （点击选项会落到下方 ComboBox 上、展开错误下拉；见 CHANGELOG 条目）。
bool Button::on_mouse_down(float x, float y, int button) {
    (void)x;
    (void)y;
    return button == 0;
}

bool Button::on_mouse_up(float x, float y, int button) {
    // 松开瞬间仍在按钮内才触发点击（按下-拖出-松开不触发）
    if (button == 0 && contains(x, y) && !has_class("layui-btn-disabled") && on_click_) on_click_();
    return true;
}

bool Button::on_key_down(int key, int mods) {
    (void)mods;
    if (has_class("layui-btn-disabled")) return true;
    if (key == KeyEnter || key == KeySpace) {
        if (on_click_) on_click_();
        return true;
    }
    return false;
}

float Button::preferred_height() const {
    if (has_class("layui-btn-lg")) return 44.f;
    if (has_class("layui-btn-sm")) return 30.f;
    if (has_class("layui-btn-xs")) return 22.f;
    return bounds_.h;
}

namespace {
// 在按钮内绘制「图标居左 + 文本居右」整体居中的内容；无图标时退化为纯文本居中；
// icon_right=true 时为「文本居左 + 图标居右」（镜像「按钮 <i layui-icon-down>」）
void draw_content(Font* font, Renderer& renderer, const std::string& text, unsigned icon_cp,
                  float icon_size, const Rect& bounds, const Vec4& color,
                  bool icon_right = false) {
    if (!font) return;
    const bool has_icon = icon_cp != 0 && font->icon_loaded();
    const float gap = has_icon ? 6.f : 0.f;  // layui i padding 0 2px + 图标与文本间距
    const float text_w = text.empty() ? 0.f : font->measure_text(text.c_str());
    const float content_w = text_w + (has_icon ? icon_size + gap : 0.f);
    const float left = bounds.x + (bounds.w - content_w) * 0.5f;
    // 文本块高度用 line_height（ascent-descent），descent 为负值；
    // 旧式 (ascent+descent) 少算 2*|descent|，文本整体下移 ~4px（UI-053）；
    // 文字视觉中心（baseline 与 cap 顶的中点）比文本框中心高 |descent|/2，
    // 向下平移该偏移使文字视觉中心与图标中心（行中线）共线（UI-206）。
    const float ty = bounds.y + (bounds.h - font->line_height()) * 0.5f
                     - font->descent() * 0.5f;
    const float cy = bounds.y + bounds.h * 0.5f;
    if (has_icon) {
        const float cx = icon_right ? left + text_w + gap + icon_size * 0.5f
                                    : left + icon_size * 0.5f;
        font->draw_icon(renderer, icon_cp, cx, cy, icon_size, color);
    }
    if (!text.empty()) {
        const float tx = icon_right ? left : left + (has_icon ? icon_size + gap : 0.f);
        font->draw_text(renderer, text.c_str(), tx, ty, color);
    }
}

// 按钮内图标字号（layui：默认 14px，sm/xs 12px）
float icon_size_for(const ui::Widget& w) {
    return (w.has_class("layui-btn-sm") || w.has_class("layui-btn-xs")) ? 12.f : 14.f;
}

// 返回控件上第一个 layui-bg-* 类名（如 "layui-bg-gray"），无则空串
std::string bg_class_name(const ui::Widget& w) {
    for (const std::string& c : w.classes()) {
        if (c.rfind("layui-bg-", 0) == 0) return c;
    }
    return std::string();
}

// layui-border 工具类：描边+文字同色（CSS 内 !important），返回 false 表示无该类
bool border_class_color(const ui::Widget& w, Vec4& out) {
    if (w.has_class("layui-border-green")) { out = Vec4{0.09f, 0.73f, 0.67f, 1.0f}; return true; }   // #16baaa
    if (w.has_class("layui-border-blue")) { out = Vec4{0.12f, 0.62f, 1.0f, 1.0f}; return true; }     // #1e9fff
    if (w.has_class("layui-border-orange")) { out = Vec4{1.0f, 0.72f, 0.0f, 1.0f}; return true; }    // #ffb800
    if (w.has_class("layui-border-red")) { out = Vec4{1.0f, 0.34f, 0.13f, 1.0f}; return true; }      // #ff5722
    if (w.has_class("layui-border-black")) { out = Vec4{0.18f, 0.21f, 0.24f, 1.0f}; return true; }   // #2f363c
    if (w.has_class("layui-border-cyan")) { out = Vec4{0.18f, 0.25f, 0.34f, 1.0f}; return true; }    // #2f4056
    if (w.has_class("layui-border-purple")) { out = Vec4{0.64f, 0.20f, 0.78f, 1.0f}; return true; }  // #a233c6
    return false;
}
}  // namespace

void Button::on_draw(Renderer& renderer) {
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;

    if (has_class("layui-form-select dl dd")) {
        // ---- layui 下拉选项（dd）：hover/选中 #f8f8f8 底，选中绿字 ----
        const bool is_this = has_class("layui-this");
        if (hovered_ || is_this)
            renderer.draw_quad(bounds_, Vec4{0.973f, 0.973f, 0.973f, 1.0f});
        if (font_ && !text_.empty()) {
            const float text_w = font_->measure_text(text_.c_str());
            const float tx = bounds_.x + 10.f;
            const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f
                             - font_->descent() * 0.5f;
            font_->draw_text(renderer, text_.c_str(), tx, ty,
                             is_this ? t->success : t->text_secondary);
        }
        return;
    }

    if (has_class("layui-menu divider")) {
        // 分隔线：1px #eee 横线，占位
        const float cy = bounds_.y + bounds_.h * 0.5f;
        renderer.draw_quad({bounds_.x + 8.f, cy, bounds_.w - 16.f, 1.f}, t->border);
        return;
    }

    if (has_class("layui-menu li")) {
        // layui 菜单项：hover #f8f8f8，文本左对齐 padding 5/15
        if (hovered_)
            renderer.draw_quad(bounds_, Vec4{0.973f, 0.973f, 0.973f, 1.0f});
        if (font_ && !text_.empty()) {
            const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f
                             - font_->descent() * 0.5f;
            font_->draw_text(renderer, text_.c_str(), bounds_.x + 15.f, ty, t->text);
        }
        return;
    }

    if (has_class("layui-btn")) {
        // ---- layui 按钮（M17b，克隆 layui.css）----
        const bool disabled = has_class("layui-btn-disabled");
        const bool primary = has_class("layui-btn-primary");
        const bool ghost = has_class("layui-btn-ghost");
        const bool radius = has_class("layui-btn-radius");

        // 背景色：layui-btn 默认 #16baaa（accent），normal/warm/danger 对应语义色
        Vec4 bg = t->accent;
        if (primary) {
            bg = Vec4{0, 0, 0, 0};  // 透明
        } else if (has_class("layui-btn-normal")) {
            bg = t->info;
        } else if (has_class("layui-btn-warm")) {
            bg = t->warning;
        } else if (has_class("layui-btn-danger")) {
            bg = t->danger;
        }

        // 圆角：layui-btn-radius -> 100px（clamp 到半高）；按钮组内中间零圆角、首/尾 2px
        float r = t->radius;
        if (group_role_ > 0)
            r = (group_role_ == 2) ? 0.f : t->radius;
        else if (radius)
            r = std::min(bounds_.w, bounds_.h) * 0.5f;

        const float icon_size = icon_size_for(*this);

        if (disabled) {
            // border #eee + 底 #fbfbfb + 字 #d2d2d2
            renderer.draw_rounded_rect(bounds_, r, Vec4{0.98f, 0.98f, 0.98f, 1.0f});
            renderer.draw_rounded_rect_outline(bounds_, r, 1.f, t->border);
            draw_content(font_, renderer, text_, icon_cp_, icon_size, bounds_,
                         Vec4{0.82f, 0.82f, 0.82f, 1.0f}, icon_right_);
            return;
        }

        // ghost：透明底、无边框，hover 时显示浅灰边框 + 白色文字/图标
        if (ghost) {
            Vec4 ghost_text{0.75f, 0.75f, 0.78f, 1.0f};
            if (hovered_) {
                ghost_text = Vec4{1.f, 1.f, 1.f, 1.f};
                renderer.draw_rounded_rect_outline(bounds_, r, 1.f,
                                                   Vec4{0.45f, 0.45f, 0.48f, 1.0f});
            }
            if (pressed_ && hovered_)
                renderer.draw_rounded_rect(bounds_, r, Vec4{0, 0, 0, 0.08f});
            draw_content(font_, renderer, text_, icon_cp_, icon_size, bounds_,
                         ghost_text, icon_right_);
            return;
        }

        // 状态：hover opacity .8、active opacity 1 -> pressed 再加深到 .65
        Vec4 fill = bg;
        if (!primary) {
            if (pressed_ && hovered_)
                fill = Vec4{bg.r * 0.65f, bg.g * 0.65f, bg.b * 0.65f, 1.0f};
            else if (hovered_)
                fill = Vec4{bg.r * 0.8f, bg.g * 0.8f, bg.b * 0.8f, 1.0f};
        }
        renderer.draw_rounded_rect(bounds_, r, fill);

        // primary pressed：内层半透明暗色叠层
        if (primary && pressed_ && hovered_) {
            renderer.draw_rounded_rect(bounds_, r, Vec4{0, 0, 0, 0.08f});
        }

        // 按钮组相邻分隔线：非首按钮、非 primary 时左侧 1px 半透明白（layui-btn-group .layui-btn border-left）
        if (group_role_ > 1 && !primary) {
            renderer.draw_quad({bounds_.x, bounds_.y + 1.f, 1.f, bounds_.h - 2.f},
                               Vec4{1, 1, 1, 0.5f});
        }

        Vec4 text_color = Vec4{1, 1, 1, 1};  // 非 primary 白字
        if (primary) {
            // 边框 #d2d2d2 + 灰字；hover 边框变 accent、字变 #333（按钮组内保持 #d2d2d2）；
            // layui-border-* 工具类以 !important 固定边框+文字同色，不受 hover 影响
            Vec4 border = Vec4{0.82f, 0.82f, 0.82f, 1.0f};
            Vec4 btext = t->text_secondary;
            Vec4 bcol;
            if (border_class_color(*this, bcol)) {
                border = bcol;
                btext = bcol;
            } else if (hovered_ && group_role_ == 0) {
                border = t->accent;
                btext = Vec4{0.2f, 0.2f, 0.2f, 1.0f};
            } else if (hovered_) {
                btext = t->accent;  // 按钮组内 primary hover 字变绿（#16baaa）
            }
            renderer.draw_rounded_rect_outline(bounds_, r, 1.f, border);
            text_color = btext;
        }

        // 聚焦态：外圈 2px accent 描边（圆角按钮随 r 圆角）
        if (focused()) {
            renderer.draw_rounded_rect_outline(bounds_, r, 2.f, t->accent);
        }

        draw_content(font_, renderer, text_, icon_cp_, icon_size, bounds_, text_color, icon_right_);

        // 内嵌徽章 / 小圆点：紧随文本右缘（layui-badge 默认红底白字，可叠加 layui-bg-* 换底）
        if ((has_badge_ || has_dot_) && font_) {
            const float line_h = font_->line_height();
            const float ty = bounds_.y + (bounds_.h - line_h) * 0.5f;
            // 计算文本右缘（与 draw_content 相同布局逻辑）
            const bool has_icon = icon_cp_ != 0 && font_->icon_loaded();
            const float gap = has_icon ? 6.f : 0.f;
            const float text_w = text_.empty() ? 0.f : font_->measure_text(text_.c_str());
            const float content_w = text_w + (has_icon ? icon_size + gap : 0.f);
            const float left = bounds_.x + (bounds_.w - content_w) * 0.5f;
            const float text_end = left + (has_icon ? icon_size + gap : 0.f) + text_w;

            // 徽章底色：layui-bg-* 优先，否则默认红（danger）
            Vec4 fill = t->danger;
            const std::string bgc = bg_class_name(*this);
            if (!bgc.empty()) fill = layui::bg_fill(bgc.c_str());

            if (has_badge_) {
                const float bw = font_->measure_text(badge_text_.c_str()) + 10.f;
                const float bx = text_end + 5.f;
                const float bh = line_h - 2.f;
                renderer.draw_rounded_rect({bx, ty + 1.f, bw, bh}, t->radius, fill);
                font_->draw_text(renderer, badge_text_.c_str(), bx + 5.f, ty, {1, 1, 1, 1});
            } else if (has_dot_) {
                const float d = 8.f;
                const float bx = text_end + 5.f;
                const float by = ty + (line_h - d) * 0.5f;
                renderer.draw_rounded_rect({bx, by, d, d}, d * 0.5f, fill);
            }
        }
        return;
    }

    // ---- 原扁平按钮 ----
    const Vec4* bg = &t->accent;
    if (pressed_ && hovered_)
        bg = &t->accent_pressed;
    else if (hovered_)
        bg = &t->accent_hover;
    renderer.draw_rounded_rect(bounds_, t->radius, *bg);

    // 聚焦态：外圈 2px accent 描边（先画外圈，再叠内层背景色）
    if (focused()) {
        renderer.draw_rounded_rect(bounds_, t->radius, t->accent);
        const Rect inner = {bounds_.x + 2, bounds_.y + 2, bounds_.w - 4, bounds_.h - 4};
        renderer.draw_rounded_rect(inner, t->radius - 1.f, *bg);
    }

    draw_content(font_, renderer, text_, icon_cp_, icon_size_for(*this), bounds_, t->text, icon_right_);
}

}  // namespace ui
