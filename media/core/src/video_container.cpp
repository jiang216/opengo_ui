#include "ui/media/video_container.h"

#include "ui/core/log.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/ui/button.h"
#include "ui/ui/iconlabel.h"
#include "ui/ui/label.h"
#include "ui/ui/slider.h"
#include "ui/ui/switch.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

namespace ui {

VideoContainer::VideoContainer(float x, float y, float w, float h)
    : Widget(x, y, w, h) {
    texture_ = new Texture;
}

VideoContainer::~VideoContainer() {
    close_source();
    delete texture_;
}

void VideoContainer::set_font(Font* font) {
    font_ = font;
    if (time_label_) time_label_->set_font(font);
    ensure_controls();
    refresh_controls();
}

void VideoContainer::set_theme(const Theme* theme) {
    theme_ = theme;
    ensure_controls();
    if (play_btn_) play_btn_->set_theme(theme);
    if (time_label_) time_label_->set_theme(theme);
    if (seek_slider_) seek_slider_->set_theme(theme);
    if (loop_switch_) loop_switch_->set_theme(theme);
    if (vol_slider_) vol_slider_->set_theme(theme);
    if (vol_label_) vol_label_->set_theme(theme);
    if (bright_slider_) bright_slider_->set_theme(theme);
    if (bright_label_) bright_label_->set_theme(theme);
    if (speed_down_btn_) speed_down_btn_->set_theme(theme);
    if (speed_label_) speed_label_->set_theme(theme);
    if (speed_up_btn_) speed_up_btn_->set_theme(theme);
}

void VideoContainer::set_title(const char* title) {
    title_ = title ? title : "";
}

bool VideoContainer::open_source(const char* descriptor) {
    close_source();
    synthetic_ = descriptor && std::strncmp(descriptor, "synthetic", 9) == 0;
    last_error_.clear();
    if (!pipe_.open(descriptor)) {
        last_error_ = "无法打开: ";
        last_error_ += descriptor ? descriptor : "";
        has_frame_ = false;
        refresh_controls();
        return false;
    }
    // 预创建纹理（GL 主线程）
    const int w = pipe_.latest_width();
    const int h = pipe_.latest_height();
    if (w > 0 && h > 0) {
        std::vector<unsigned char> zeros((size_t)w * h * 4, 0);
        texture_->create(w, h, zeros.data());
    }
    has_frame_ = false;
    refresh_controls();
    return true;
}

void VideoContainer::close_source() {
    pipe_.close();
    has_frame_ = false;
    if (texture_) texture_->create(0, 0, nullptr);  // 释放旧纹理，重新 open 时重建
}

bool VideoContainer::has_video() const { return pipe_.has_video(); }

void VideoContainer::play() {
    pipe_.play();
    refresh_controls();
}

void VideoContainer::pause() {
    pipe_.pause();
    refresh_controls();
}

void VideoContainer::set_playing(bool playing) {
    pipe_.set_playing(playing);
    refresh_controls();
}

bool VideoContainer::playing() const { return pipe_.playing(); }

void VideoContainer::set_loop(bool loop) {
    pipe_.set_loop(loop);
    refresh_controls();
}

bool VideoContainer::loop() const { return pipe_.loop(); }

void VideoContainer::set_speed(float speed) {
    pipe_.set_speed(speed);
}

float VideoContainer::speed() const { return pipe_.speed(); }

void VideoContainer::step_frame() { pipe_.step_frame(); }

void VideoContainer::seek(float seconds) {
    pipe_.seek(seconds);
}

double VideoContainer::duration() const { return pipe_.duration(); }

float VideoContainer::position() const { return pipe_.position(); }

void VideoContainer::set_volume(float volume) { pipe_.set_volume(volume); }

float VideoContainer::volume() const { return pipe_.volume(); }

void VideoContainer::set_brightness(float brightness) {
    const float v = std::clamp(brightness, kVideoContainerBrightMin, kVideoContainerBrightMax);
    if (std::fabs(v - brightness_) < 1e-4f) return;
    brightness_ = v;
    mark_dirty();
}

bool VideoContainer::has_audio() const { return pipe_.has_audio(); }

void VideoContainer::ensure_controls() {
    if (play_btn_) return;
    play_btn_ = new Button(0, 0, 26, 26, "");
    play_btn_->set_icon(pipe_.playing() ? "pause" : "play");
    play_btn_->set_on_click([this]() {
        if (!has_video()) return;
        if (pipe_.playing()) {
            pipe_.pause();
        } else {
            pipe_.play();
        }
        refresh_controls();
    });
    add_child(play_btn_);

    time_label_ = new Label(0, 0, 130, 18, "00:00 / 00:00");
    add_child(time_label_);

    seek_slider_ = new Slider(0, 0, 100, 16);
    seek_slider_->set_range(0.f, 1.f);
    seek_slider_->set_on_change([this](float t) {
        if (has_video() && pipe_.duration() > 0.0) pipe_.seek(t * (float)pipe_.duration());
    });
    add_child(seek_slider_);

    loop_switch_ = new Switch(0, 0, 40, 20);
    loop_switch_->set_on_change([this](bool v) { pipe_.set_loop(v); });
    add_child(loop_switch_);

    // 竖向音量滑块（UI-194）：视频区左缘
    vol_slider_ = new Slider(0, 0, 22, 120);
    vol_slider_->set_vertical(true);
    vol_slider_->set_range(0.f, 1.f);
    vol_slider_->set_value_silent(1.f);
    vol_slider_->set_tips(false);
    vol_slider_->set_on_change([this](float v) { pipe_.set_volume(v); });
    add_child(vol_slider_);

    // 音量标签（UI-194）：sound 图标 + 中文，放滑块正上方空档，纯白高亮
    vol_label_ = new IconLabel(0, 0, 22, 20, "音量");
    vol_label_->set_icon("sound");
    vol_label_->set_icon_size(14.f);
    vol_label_->set_icon_color({1.f, 1.f, 1.f, 1.f});
    vol_label_->set_text_color({1.f, 1.f, 1.f, 1.f});
    add_child(vol_label_);

    // 竖向亮度滑块（UI-194）：视频区右缘，只调画面（RGB 乘亮度系数）
    bright_slider_ = new Slider(0, 0, 22, 120);
    bright_slider_->set_vertical(true);
    bright_slider_->set_range(kVideoContainerBrightMin, kVideoContainerBrightMax);
    bright_slider_->set_value_silent(1.f);
    bright_slider_->set_tips(false);
    bright_slider_->set_on_change([this](float v) { set_brightness(v); });
    add_child(bright_slider_);

    // 亮度标签（UI-194）：light 图标 + 中文，放滑块正上方空档，纯白高亮
    bright_label_ = new IconLabel(0, 0, 22, 20, "亮度");
    bright_label_->set_icon("light");
    bright_label_->set_icon_size(14.f);
    bright_label_->set_icon_color({1.f, 1.f, 1.f, 1.f});
    bright_label_->set_text_color({1.f, 1.f, 1.f, 1.f});
    add_child(bright_label_);

    // 倍速箭头（UI-194）：▲ 值 ▼，步进 ±0.25 夹在 0.5..2x
    speed_down_btn_ = new Button(0, 0, 18, 18, "");
    speed_down_btn_->set_icon("down");
    speed_down_btn_->set_on_click([this]() {
        const float s = std::max(0.5f, pipe_.speed() - kVideoContainerSpeedStep);
        pipe_.set_speed(s);
        refresh_controls();
    });
    add_child(speed_down_btn_);

    speed_label_ = new Label(0, 0, 40, 14, "1.0x");
    add_child(speed_label_);

    speed_up_btn_ = new Button(0, 0, 18, 18, "");
    speed_up_btn_->set_icon("up");
    speed_up_btn_->set_on_click([this]() {
        const float s = std::min(2.0f, pipe_.speed() + kVideoContainerSpeedStep);
        pipe_.set_speed(s);
        refresh_controls();
    });
    add_child(speed_up_btn_);

    if (font_) {
        play_btn_->set_font(font_);
        time_label_->set_font(font_);
        seek_slider_->set_font(font_);
        loop_switch_->set_font(font_);
        vol_slider_->set_font(font_);
        vol_label_->set_font(font_);
        bright_slider_->set_font(font_);
        bright_label_->set_font(font_);
        speed_down_btn_->set_font(font_);
        speed_label_->set_font(font_);
        speed_up_btn_->set_font(font_);
    }
    if (theme_) {
        play_btn_->set_theme(theme_);
        time_label_->set_theme(theme_);
        seek_slider_->set_theme(theme_);
        loop_switch_->set_theme(theme_);
        vol_slider_->set_theme(theme_);
        vol_label_->set_theme(theme_);
        bright_slider_->set_theme(theme_);
        bright_label_->set_theme(theme_);
        speed_down_btn_->set_theme(theme_);
        speed_label_->set_theme(theme_);
        speed_up_btn_->set_theme(theme_);
    }
}

void VideoContainer::layout_controls() {
    const float x0 = bounds_.x + 8.f;
    const float x1 = bounds_.x + bounds_.w - 8.f;
    const float bottom = bounds_.y + bounds_.h - 6.f;
    const float title_h = title_.empty() ? 0.f : 26.f;

    // 底部控制条（叠在画面上）：seek 一行 + 控制一行
    seek_slider_->set_bounds(x0, bottom - 18.f, x1 - x0, 14.f);
    const float cy = bottom - 42.f;
    play_btn_->set_bounds(x0, cy, 26.f, 26.f);
    time_label_->set_bounds(x0 + 32.f, cy + 3.f, 130.f, 18.f);

    // 右侧成组（由右向左）：倍速 ▲ 值 ▼ + 循环开关
    const float right_x = x1 - 18.f;
    speed_up_btn_->set_bounds(right_x, cy + 1.f, 18.f, 18.f);
    speed_label_->set_bounds(right_x - 42.f, cy + 3.f, 42.f, 14.f);
    speed_down_btn_->set_bounds(right_x - 42.f - 20.f, cy + 1.f, 18.f, 18.f);
    loop_switch_->set_bounds(speed_down_btn_->bounds().x - 8.f - 40.f, cy + 2.f, 40.f, 20.f);

    // 竖向滑块：视频区左右边缘（避开标题条与控制面板，轨道长随 bounds_.h）
    const float top = bounds_.y + title_h + 8.f;
    const float vid_bottom = bottom - 60.f;
    const float vh = std::max(64.f, vid_bottom - top);
    const float vol_w = 22.f;
    // 标签带在滑块正上方（不重叠轨道/把手），纯白高亮 IconLabel
    const float lbl_top = top;
    const float lbl_h = 20.f;
    const float slider_top = top + lbl_h + 4.f;
    const float slider_h = std::max(64.f, vid_bottom - slider_top);
    vol_label_->set_bounds(bounds_.x + 6.f, lbl_top, vol_w, lbl_h);
    vol_slider_->set_bounds(bounds_.x + 6.f, slider_top, vol_w, slider_h);
    bright_label_->set_bounds(bounds_.x + bounds_.w - 6.f - vol_w, lbl_top, vol_w, lbl_h);
    bright_slider_->set_bounds(bounds_.x + bounds_.w - 6.f - vol_w, slider_top, vol_w, slider_h);
}

void VideoContainer::refresh_controls() {
    if (play_btn_) {
        play_btn_->set_icon(has_video() && pipe_.playing() ? "pause" : "play");
        play_btn_->set_text("");
    }
    if (loop_switch_ && has_video()) loop_switch_->set_checked(pipe_.loop());
    if (time_label_) {
        char buf[48];
        const double dur = pipe_.duration();
        const double pos = pipe_.position();
        const int pm = (int)(pos / 60), ps = (int)pos % 60;
        const int dm = (int)(dur / 60), ds = (int)dur % 60;
        std::snprintf(buf, sizeof(buf), "%02d:%02d / %02d:%02d", pm, ps, dm, ds);
        time_label_->set_text(buf);
    }
    if (seek_slider_ && has_video() && !seek_slider_->dragging() && pipe_.duration() > 0.0) {
        seek_slider_->set_value_silent((float)(pipe_.position() / pipe_.duration()));
    }
    if (vol_slider_ && !vol_slider_->dragging()) vol_slider_->set_value_silent(pipe_.volume());
    if (bright_slider_ && !bright_slider_->dragging()) bright_slider_->set_value_silent(brightness());
    if (speed_label_) {
        char sbuf[16];
        std::snprintf(sbuf, sizeof(sbuf), "%.1fx", pipe_.speed());
        speed_label_->set_text(sbuf);
    }
}

void VideoContainer::on_advance(float dt) {
    // 主线程：推进管线并上传新帧（GL 调用）
    if (pipe_.advance(dt)) {
        if (texture_ && pipe_.latest_rgba()) {
            texture_->update(pipe_.latest_rgba());
            has_frame_ = true;
        }
    }
    refresh_controls();
    Widget::on_advance(dt);
}

void VideoContainer::on_layout() {
    ensure_controls();
    layout_controls();
    Widget::on_layout();
}

void VideoContainer::on_draw(Renderer& renderer) {
    const Vec4 dark{0.10f, 0.10f, 0.11f, 1.0f};
    renderer.draw_quad(bounds_, dark);

    // 视频区（不含顶部标题条 26px）
    const float title_h = title_.empty() ? 0.f : 26.f;
    Rect video_rect = bounds_;
    video_rect.y += title_h;
    video_rect.h -= title_h;

    if (has_frame_ && texture_ && texture_->width() > 0 && texture_->height() > 0) {
        const float vw = (float)texture_->width();
        const float vh = (float)texture_->height();
        const float img_aspect = vw / vh;
        const float box_aspect = video_rect.w / video_rect.h;
        Rect r = video_rect;
        if (box_aspect > img_aspect) {
            const float w = video_rect.h * img_aspect;
            r.x = video_rect.x + (video_rect.w - w) * 0.5f;
            r.w = w;
        } else {
            const float h = video_rect.w / img_aspect;
            r.y = video_rect.y + (video_rect.h - h) * 0.5f;
            r.h = h;
        }
        renderer.draw_textured_quad(r, texture_->id(), Rect{0.f, 0.f, 1.f, 1.f},
                                    Vec4{brightness_, brightness_, brightness_, 1.f});
    } else if (font_) {
        // 占位 / 错误文案（居中）
        const char* msg = !last_error_.empty() ? last_error_.c_str()
                          : (has_video() ? "loading..." : "open_source(descriptor)");
        const float lh = font_->line_height();
        font_->draw_text(renderer, msg, video_rect.x + 12.f,
                         video_rect.y + (video_rect.h - lh) * 0.5f,
                         theme_ ? theme_->text_dim : Vec4{0.62f, 0.64f, 0.68f, 1.f});
    }

    // 标题条
    if (!title_.empty() && font_) {
        renderer.draw_quad({bounds_.x, bounds_.y, bounds_.w, title_h},
                           theme_ ? theme_->input_bg : Vec4{0.16f, 0.17f, 0.19f, 1.f});
        font_->draw_text(renderer, title_.c_str(), bounds_.x + 8.f,
                         bounds_.y + (title_h - font_->line_height()) * 0.5f,
                         theme_ ? theme_->text : Vec4{0.92f, 0.93f, 0.95f, 1.f});
    }

    // 底部控制条半透明面板（radius>0 允许半透明，防 UI-037 全屏灰蒙）
    if (font_) {
        const Rect bar{bounds_.x, bounds_.y + bounds_.h - 56.f, bounds_.w, 56.f};
        renderer.draw_rounded_rect(bar, 6.f, Vec4{0.f, 0.f, 0.f, 0.55f});
    }

    // 竖向滑块标识图标（UI-194）已改由 vol_label_/bright_label_（IconLabel 控件）在滑块上方绘制，
    // 不在此手绘（UI-194 修复：图标与轨道/把手零重叠）。

    // 边框（UI-053 圆角描边环）
    const float radius = theme_ ? theme_->radius : 6.f;
    const Vec4 border_color = theme_ ? theme_->input_border : Vec4{0.35f, 0.36f, 0.40f, 1.f};
    renderer.draw_rounded_rect_outline(bounds_, radius, 1.f, border_color);
}

}  // namespace ui