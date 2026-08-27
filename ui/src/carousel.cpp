#include "ui/ui/carousel.h"

#include "ui/core/log.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <cmath>
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

constexpr float kTransDur = 0.45f;  // 切换过渡时长（秒）

bool file_readable(const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (f) { std::fclose(f); return true; }
    return false;
}

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

bool is_bare_filename(const char* path) {
    for (const char* p = path; *p; ++p)
        if (*p == '/' || *p == '\\') return false;
    return true;
}

}  // namespace

Carousel::Carousel(float x, float y, float w, float h) : Widget(x, y, w, h) {}

Carousel::~Carousel() {
    for (auto& s : slides_) delete s.image;
}

void Carousel::add_slide(const Vec4& bg, const char* title) {
    slides_.push_back(Slide{bg, title ? title : "", nullptr});
}
bool Carousel::add_image_slide(const char* path) {
    if (!path || !*path) return false;
    // 资产定位（UI-048）：裸文件名先试 CWD，再回退 exe 目录（与 Image::load_from_file 一致）。
    const char* targets[2] = {path, ""};
    std::string exe_cand;
    if (is_bare_filename(path)) {
        const std::string d = exe_directory();
        if (!d.empty()) { exe_cand = d + "/" + path; targets[1] = exe_cand.c_str(); }
    }
    Texture* tex = nullptr;
    for (int i = 0; i < 2; ++i) {
        const char* t = targets[i];
        if (!t || !*t || !file_readable(t)) continue;
        tex = new Texture;
        if (tex->load_from_file(t)) break;
        delete tex; tex = nullptr;
    }
    if (!tex) { log_error("carousel: cannot load image %s (cwd / exe dir)", path); return false; }
    slides_.push_back(Slide{Vec4{0.85f, 0.85f, 0.85f, 1.f}, "", tex});
    return true;
}
void Carousel::clear() {
    mark_dirty();
        for (auto& s : slides_) delete s.image;
    slides_.clear();
    current_ = 0;
    trans_t_ = -1.f;
    autoplay_ = false;
}

void Carousel::set_current_now(int idx) {
    if (slides_.empty()) return;
    const int n = (int)slides_.size();
    current_ = ((idx % n) + n) % n;
    if (on_change_) on_change_(current_);
}

void Carousel::set_index(int idx) {
    mark_dirty();
        if (slides_.empty()) return;
    start_transition(current_, idx);
}

void Carousel::start_transition(int from, int to) {
    if (slides_.empty()) return;
    const int n = (int)slides_.size();
    from = ((from % n) + n) % n;
    to = ((to % n) + n) % n;
    trans_from_ = from;
    trans_to_ = to;
    trans_t_ = 0.f;
    // change 事件在过渡完成时触发
}

void Carousel::next() {
    if (slides_.empty()) return;
    start_transition(current_, current_ + 1);
}
void Carousel::prev() {
    if (slides_.empty()) return;
    start_transition(current_, current_ - 1);
}

void Carousel::advance(float dt) { on_advance(dt); }

void Carousel::on_advance(float dt) {
    if (slides_.empty()) return;
    // 过渡进度
    if (trans_t_ >= 0.f) {
        trans_t_ += dt / kTransDur;
        if (trans_t_ >= 1.f) {
            trans_t_ = -1.f;
            if (current_ != trans_to_) {
                current_ = trans_to_;
                if (on_change_) on_change_(current_);
            }
        }
    }
    // 自动轮播（毫秒 interval，转秒计时）
    if (autoplay_ && slides_.size() >= 2 && trans_t_ < 0.f) {
        timer_ += dt;
        if (timer_ >= interval_ / 1000.f) {
            timer_ = 0.f;
            start_transition(current_, current_ + 1);
        }
    }
}

void Carousel::on_layout() {}

bool Carousel::on_mouse_move(float x, float y) {
    if (slides_.empty()) return false;
    const bool over = x >= bounds_.x && x <= bounds_.x + bounds_.w &&
                      y >= bounds_.y && y <= bounds_.y + bounds_.h;
    hovered_ = over;
    return false;
}

bool Carousel::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    if (slides_.empty()) return true;
    // 左右箭头区域（始终/悬停 时响应）
    if (arrow_ != CarouselArrow::None) {
        const bool show_by = arrow_ == CarouselArrow::Always || hovered_;
        if (show_by) {
            if (x >= bounds_.x && x <= bounds_.x + 40.f) {
                prev();
                return true;
            }
            if (x >= bounds_.x + bounds_.w - 40.f && x <= bounds_.x + bounds_.w) {
                next();
                return true;
            }
        }
    }
    // 底部指示点（inside 时在容器内，outside 时在下方一行）
    if (indicator_ != CarouselIndicator::None) {
        const int n = (int)slides_.size();
        const float dot_w = 10.f * n + 6.f * (n - 1);
        const float dx = bounds_.x + bounds_.w / 2.f - dot_w / 2.f;
        float dy = bounds_.y + bounds_.h - 18.f;
        if (indicator_ == CarouselIndicator::Outside) dy = bounds_.y + bounds_.h + 8.f;
        if (y >= dy && y <= dy + 8.f) {
            for (int i = 0; i < n; ++i) {
                const float d0 = dx + i * 16.f;
                if (x >= d0 && x <= d0 + 10.f) {
                    set_index(i);
                    return true;
                }
            }
        }
    }
    return true;  // 轮播区点击不冒泡
}

void Carousel::on_draw(Renderer& renderer) {
    if (slides_.empty()) return;
    const int n = (int)slides_.size();
    const int show = (trans_t_ >= 0.f) ? trans_to_ : current_;
    const int prev = (trans_t_ >= 0.f) ? trans_from_ : current_;

    const auto draw_slide_rect = [&](int idx, float ox, float oy) {
        const Slide& s = slides_[(size_t)idx];
        const Rect r{bounds_.x + ox, bounds_.y + oy, bounds_.w, bounds_.h};
        renderer.draw_quad(r, s.bg);
        if (s.image) {
            renderer.draw_textured_quad(r, s.image->id(), Rect{0.f, 0.f, 1.f, 1.f},
                                        Vec4{1, 1, 1, 1});
        }
        if (font_ && !s.title.empty()) {
            const Vec4 text_color = s.bg.a > 0.6f ? Vec4{0.2f, 0.2f, 0.2f, 1.0f}
                                                  : Vec4{1, 1, 1, 1};
            const float cx = r.x + r.w / 2.f;
            const float cy = r.y + r.h / 2.f;
            const float tw = font_->measure_text(s.title.c_str());
            font_->draw_text(renderer, s.title.c_str(), cx - tw / 2.f,
                             cy - font_->line_height() / 2.f, text_color);
        }
    };

    // 过渡动画
    if (trans_t_ >= 0.f) {
        const float t = trans_t_;
        const bool forward = (trans_to_ == ((trans_from_ + 1) % n) &&
                              trans_from_ != ((trans_to_ + 1) % n));
        switch (anim_) {
            case CarouselAnim::Updown: {
                // 上→下翻页：新版从下往上推入
                const float off_old = -t * bounds_.h;
                const float off_new = (1.f - t) * bounds_.h;
                draw_slide_rect(forward ? prev : show, 0.f,
                                forward ? off_old : (show == trans_from_ ? off_new : 0.f));
                draw_slide_rect(forward ? show : prev, 0.f,
                                forward ? off_new : (prev == trans_from_ ? off_old : 0.f));
                break;
            }
            case CarouselAnim::Fade: {
                // 渐隐渐显：底层旧页完整绘制，上层新页以 alpha=t 叠加（混合开启）
                const auto draw_slide_alpha = [&](int idx, float alpha) {
                    const Slide& s = slides_[(size_t)idx];
                    const Rect r = bounds_;
                    renderer.push_scissor(bounds_);
                    renderer.draw_quad(r, Vec4{s.bg.r, s.bg.g, s.bg.b, alpha});
                    if (s.image)
                        renderer.draw_textured_quad(r, s.image->id(), Rect{0.f, 0.f, 1.f, 1.f},
                                                    Vec4{1, 1, 1, alpha});
                    if (font_ && !s.title.empty()) {
                        const Vec4 tc = s.bg.a >= 0.6f ? Vec4{0.2f, 0.2f, 0.2f, alpha}
                                                       : Vec4{1, 1, 1, alpha};
                        const float cx = r.x + r.w / 2.f;
                        const float cy = r.y + r.h / 2.f;
                        const float tw = font_->measure_text(s.title.c_str());
                        font_->draw_text(renderer, s.title.c_str(), cx - tw / 2.f,
                                         cy - font_->line_height() / 2.f, tc);
                    }
                    renderer.pop_scissor();
                };
                draw_slide_alpha(prev, 1.f);
                draw_slide_alpha(show, t);
                break;
            }
            case CarouselAnim::Default:
            default: {
                const float off_old = -t * bounds_.w;
                const float off_new = (1.f - t) * bounds_.w;
                draw_slide_rect(forward ? prev : show, forward ? off_old : 0.f, 0.f);
                draw_slide_rect(forward ? show : prev, forward ? off_new : off_old, 0.f);
                break;
            }
        }
    } else {
        draw_slide_rect(current_, 0.f, 0.f);
    }

    // 覆盖层（页码 / 箭头 / 指示点）
    if (!font_) return;
    {
        // 页码：右下角 "1/3"
        char page[16];
        std::snprintf(page, sizeof(page), "%d/%d", current_ + 1, n);
        const float pw = font_->measure_text(page);
        font_->draw_text(renderer, page, bounds_.x + bounds_.w - pw - 8.f,
                         bounds_.y + bounds_.h - 16.f, Vec4{0.3f, 0.3f, 0.3f, 0.6f});
    }
    // 左右箭头
    if (arrow_ != CarouselArrow::None) {
        const bool show = arrow_ == CarouselArrow::Always || hovered_;
        if (show) {
            const float ay = bounds_.y + bounds_.h / 2.f;
            Vec2 pts[3];
            pts[0] = Vec2{bounds_.x + 12.f, ay - 8.f};
            pts[1] = Vec2{bounds_.x + 12.f, ay + 8.f};
            pts[2] = Vec2{bounds_.x + 26.f, ay};
            renderer.draw_polygon(pts, 3, Vec4{1, 1, 1, 0.8f});
            pts[0] = Vec2{bounds_.x + bounds_.w - 26.f, ay - 8.f};
            pts[1] = Vec2{bounds_.x + bounds_.w - 26.f, ay + 8.f};
            pts[2] = Vec2{bounds_.x + bounds_.w - 12.f, ay};
            renderer.draw_polygon(pts, 3, Vec4{1, 1, 1, 0.8f});
        }
    }
    // 指示点
    if (indicator_ != CarouselIndicator::None) {
        const float dot_w = 10.f * n + 6.f * (n - 1);
        const float dx = bounds_.x + bounds_.w / 2.f - dot_w / 2.f;
        float dy = bounds_.y + bounds_.h - 16.f;
        if (indicator_ == CarouselIndicator::Outside) dy += 16.f;
        for (int i = 0; i < n; ++i) {
            const Rect r{dx + i * 16.f, dy, 10.f, 6.f};
            const Vec4 c = (i == current_) ? Vec4{0.09f, 0.72f, 0.47f, 0.95f}
                                           : Vec4{1, 1, 1, 0.5f};
            renderer.draw_rounded_rect(r, 3.f, c);
        }
    }
}

}  // namespace ui