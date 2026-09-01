#include "ui/ui/navbar.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/ui/theme.h"

#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace ui {

namespace {
constexpr float kItemH = 40.f;
constexpr float kItemPad = 20.f;
constexpr float kChildH = 36.f;
constexpr float kImgSize = 24.f;
constexpr float kDropdownMinW = 100.f;
constexpr float kArrowSize = 8.f;

std::string exe_directory() {
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, buf, (int)n, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string p((size_t)len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, buf, (int)n, &p[0], len, nullptr, nullptr);
    const size_t slash = p.find_last_of("/\\");
    if (slash == std::string::npos) return {};
    return p.substr(0, slash);
#else
    return {};
#endif
}

bool file_readable(const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (f) { std::fclose(f); return true; }
    return false;
}

bool is_bare_filename(const char* path) {
    for (const char* p = path; *p; ++p)
        if (*p == '/' || *p == '\\') return false;
    return true;
}

Texture* load_image_from_file(const char* path) {
    if (!path || !*path) return nullptr;
    std::string exe_cand;
    if (is_bare_filename(path)) {
        const std::string d = exe_directory();
        if (!d.empty()) exe_cand = d + "/" + path;
    }
    const char* targets[2] = {path, exe_cand.c_str()};
    for (int i = 0; i < 2; ++i) {
        const char* t = targets[i];
        if (!t || !*t) continue;
        if (!file_readable(t)) continue;
        Texture* tex = new Texture;
        if (tex->load_from_file(t)) return tex;
        delete tex;
    }
    return nullptr;
}
}  // namespace

NavBar::NavBar(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-nav");
}

NavBar::~NavBar() {
    for (auto& it : items_) delete it.image_tex;
}

void NavBar::add_item(const char* text, std::function<void()> cb) {
    mark_dirty();  // 数据变更自报：使新增项能触发 root 门控重绘，不因宿主停在 clean 帧而漏显
    items_.push_back({text ? text : "", std::move(cb), {}, false, "", false, false, "", nullptr, 0.f, 0.f});
}

void NavBar::add_child(const char* text, std::function<void()> cb) {
    if (items_.empty()) return;
    mark_dirty();  // 同上：子项数据变更自报标脏
    items_.back().children.push_back({text ? text : "", std::move(cb), 0.f, 0.f});
}

void NavBar::set_active(int index) {
    mark_dirty();
        if (index >= 0 && index < (int)items_.size()) active_ = index;
}

void NavBar::set_badge(const char* text) {
    mark_dirty();
        if (items_.empty()) return;
    items_.back().has_badge = true;
    items_.back().badge_text = text ? text : "";
}

void NavBar::set_dot_badge() {
    mark_dirty();
        if (items_.empty()) return;
    items_.back().has_dot = true;
}

void NavBar::set_image(const char* path) {
    mark_dirty();
        if (items_.empty()) return;
    auto& it = items_.back();
    it.has_image = true;
    it.image_path = path ? path : "";
    delete it.image_tex;
    it.image_tex = load_image_from_file(path);
}

void NavBar::set_bg_color(Vec4 color) { mark_dirty();
        bg_color_ = color; }

void NavBar::set_bar_disabled(bool disabled) { mark_dirty();
        bar_disabled_ = disabled; }

float NavBar::preferred_height() const { return bounds_.h > 0 ? bounds_.h : kItemH; }

void NavBar::on_layout() { layout_items(); }

void NavBar::layout_items() {
    float cx = bounds_.x;
    for (auto& it : items_) {
        const float tw = font_ ? font_->measure_text(it.text.c_str()) : 0.f;
        float item_w = kItemPad;
        if (it.has_image) {
            item_w += kImgSize + 8.f;
        }
        item_w += tw + kItemPad;
        if (it.has_badge) item_w += 20.f;
        if (it.has_dot) item_w += 12.f;
        if (!it.children.empty()) item_w += kArrowSize + 4.f;
        it.x0 = cx;
        it.x1 = cx + item_w;
        cx = it.x1;
    }
}

void NavBar::close_dropdown() { open_dropdown_ = -1; }

bool NavBar::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    layout_items();

    // 点击下拉子项
    if (open_dropdown_ >= 0 && open_dropdown_ < (int)items_.size()) {
        const auto& it = items_[open_dropdown_];
        const float dy = bounds_.y + kItemH;
        const float dh = (float)it.children.size() * kChildH;
        if (y >= dy && y < dy + dh && x >= it.x0 && x < it.x1 + 40.f) {
            int ci = (int)((y - dy) / kChildH);
            if (ci >= 0 && ci < (int)it.children.size()) {
                active_ = open_dropdown_;
                if (it.children[ci].cb) it.children[ci].cb();
                close_dropdown();
                return true;
            }
        }
        // 下拉面板外部点击：关闭面板，再由顶级项逻辑处理 toggle
        const int prev = open_dropdown_;
        close_dropdown();
        // 如果点击的不是之前打开的项，则 fall through 让顶级项处理器重新打开
        if (y >= bounds_.y && y < bounds_.y + kItemH) {
            for (int i = 0; i < (int)items_.size(); ++i) {
                if (x >= items_[i].x0 && x < items_[i].x1) {
                    if (!items_[i].children.empty()) {
                        // toggle: prev == i → 关闭; prev != i → 打开新项
                        open_dropdown_ = (prev == i) ? -1 : i;
                    } else {
                        active_ = i;
                        if (items_[i].cb) items_[i].cb();
                    }
                    return true;
                }
            }
        }
        return false;
    }

    // 点击顶级项
    if (y >= bounds_.y && y < bounds_.y + kItemH) {
        for (int i = 0; i < (int)items_.size(); ++i) {
            if (x >= items_[i].x0 && x < items_[i].x1) {
                if (!items_[i].children.empty()) {
                    open_dropdown_ = (open_dropdown_ == i) ? -1 : i;
                } else {
                    active_ = i;
                    close_dropdown();
                    if (items_[i].cb) items_[i].cb();
                }
                return true;
            }
        }
    }

    close_dropdown();
    return false;
}

bool NavBar::on_mouse_move(float x, float y) {
    layout_items();
    hover_ = -1;
    if (y >= bounds_.y && y < bounds_.y + kItemH) {
        for (int i = 0; i < (int)items_.size(); ++i) {
            if (x >= items_[i].x0 && x < items_[i].x1) {
                hover_ = i;
                return false;
            }
        }
    }
    return false;
}

void NavBar::draw_overlay(Renderer& renderer) {
    if (!font_) return;
    if (open_dropdown_ < 0 || open_dropdown_ >= (int)items_.size()) return;
    const auto& it = items_[open_dropdown_];
    if (it.children.empty()) return;

    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;
    const float line_h = font_->line_height();
    const float dy = bounds_.y + kItemH;
    const float dh = (float)it.children.size() * kChildH;
    float max_tw = 0.f;
    for (const auto& ch : it.children) {
        const float cw = font_->measure_text(ch.text.c_str());
        if (cw > max_tw) max_tw = cw;
    }
    const float dd_w = max_tw + 24.f > kDropdownMinW ? max_tw + 24.f : kDropdownMinW;
    const float dd_x = it.x0;
    renderer.draw_quad({dd_x, dy, dd_w, dh}, t->panel);
    const Vec4 bc = t->border;
    renderer.draw_quad({dd_x, dy, dd_w, 1.f}, bc);
    renderer.draw_quad({dd_x, dy + dh - 1.f, dd_w, 1.f}, bc);
    renderer.draw_quad({dd_x, dy, 1.f, dh}, bc);
    renderer.draw_quad({dd_x + dd_w - 1.f, dy, 1.f, dh}, bc);
    for (int ci = 0; ci < (int)it.children.size(); ++ci) {
        const float cy = dy + ci * kChildH;
        font_->draw_text(renderer, it.children[ci].text.c_str(),
                         dd_x + 12.f, cy + (kChildH - line_h) * 0.5f, t->text);
    }
}

void NavBar::on_draw(Renderer& renderer) {
    if (!font_) return;
    layout_items();
    static const Theme kDefault;
    const Theme* t = theme_ ? theme_ : &kDefault;

    // 背景：有颜色才画（默认不画，融合白色内容区）
    if (bg_color_.a > 0.f) {
        renderer.draw_quad(bounds_, bg_color_);
    }

    const float line_h = font_->line_height();
    const float ty = bounds_.y + (kItemH - line_h) * 0.5f;

    for (int i = 0; i < (int)items_.size(); ++i) {
        const auto& it = items_[i];
        float tx = it.x0 + kItemPad;
        const bool is_active = (i == active_);
        const bool is_hover = (i == hover_);
        const bool is_open = (i == open_dropdown_);

        // 选中态文字绿色
        const Vec4 text_color = is_active ? t->success : t->text;

        // hover / open 背景高亮
        if (is_hover || is_open) {
            renderer.draw_quad({it.x0, bounds_.y, it.x1 - it.x0, kItemH},
                               Vec4{0.96f, 0.96f, 0.96f, 1.f});
        }

        // 图片头像（圆形裁剪）
        if (it.has_image) {
            if (it.image_tex) {
                renderer.draw_textured_quad({tx, ty + 2.f, kImgSize, kImgSize},
                                           it.image_tex->id(),
                                           {0.f, 0.f, 1.f, 1.f}, {1.f, 1.f, 1.f, 1.f},
                                           kImgSize * 0.5f);
            } else {
                renderer.draw_rounded_rect({tx, ty + 2.f, kImgSize, kImgSize},
                                           kImgSize * 0.5f, t->text_muted);
            }
            tx += kImgSize + 8.f;
        }

        // 文字
        if (!it.text.empty()) {
            font_->draw_text(renderer, it.text.c_str(), tx, ty, text_color);
            tx += font_->measure_text(it.text.c_str());
        }

        // 徽章（红底白字，layui-badge 默认 #ff5722）
        if (it.has_badge) {
            tx += 4.f;
            const float bw = font_->measure_text(it.badge_text.c_str()) + 10.f;
            renderer.draw_rounded_rect({tx, ty + 2.f, bw, line_h - 2.f}, t->radius, t->danger);
            font_->draw_text(renderer, it.badge_text.c_str(), tx + 5.f, ty, {1, 1, 1, 1});
        }

        // 小圆点（8px 正圆，layui-badge-dot 默认红）
        if (it.has_dot) {
            tx += 6.f;
            const float d = 8.f;
            renderer.draw_rounded_rect({tx, ty + line_h * 0.5f - d * 0.5f, d, d}, d * 0.5f, t->danger);
        }

        // 折叠菜单基础样式标识（V 型向下三角）
        if (!it.children.empty()) {
            const float ax = it.x1 - kArrowSize - 4.f;
            const float ay = bounds_.y + (kItemH - line_h) * 0.5f;
            const float sz = 4.f;
            Vec2 tri[3] = {
                {ax - sz, ay + 1.f},
                {ax + sz, ay + 1.f},
                {ax, ay + sz + 1.f}
            };
            renderer.draw_polygon(tri, 3, text_color);
        }
    }
}

}  // namespace ui
