#include "ui/media/video_view.h"

#include "ui/core/log.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/media/ffmpeg_nodes.h"
#include "ui/media/pipeline/pipeline.h"

#include <cstddef>
#include <vector>

namespace ui {

VideoView::VideoView() : Widget(0, 0, 320, 180) {
    texture_ = new Texture;
    pipe_ = new media::MediaPipeline;
    media::register_ffmpeg_nodes();  // 幂等：确保 FFmpeg 节点注册（即使 ffmpeg_nodes.cpp 未被直接引用）
}

VideoView::~VideoView() {
    close_source();
    delete pipe_;
    delete texture_;
}

void VideoView::close_source() {
    if (!pipe_) return;
    pipe_->close();
    has_frame_ = false;
}

bool VideoView::open_source(const char* path) {
    close_source();
    if (!pipe_->open(path)) {
        has_frame_ = false;
        return false;
    }
    const int w = pipe_->latest_width();
    const int h = pipe_->latest_height();
    if (w > 0 && h > 0) {
        // 预创建纹理（GL 主线程）
        std::vector<unsigned char> zeros((size_t)w * h * 4, 0);
        texture_->create(w, h, zeros.data());
    }
    has_frame_ = false;
    return true;
}

bool VideoView::has_video() const { return pipe_ != nullptr && pipe_->has_video(); }

void VideoView::play() {
    if (pipe_) pipe_->play();
}

void VideoView::pause() {
    if (pipe_) pipe_->pause();
}

void VideoView::set_playing(bool playing) {
    if (pipe_) pipe_->set_playing(playing);
}

bool VideoView::playing() const { return pipe_ != nullptr && pipe_->playing(); }

void VideoView::set_loop(bool loop) {
    if (pipe_) pipe_->set_loop(loop);
}

bool VideoView::loop() const { return pipe_ != nullptr && pipe_->loop(); }

void VideoView::set_speed(float speed) {
    if (pipe_) pipe_->set_speed(speed);
}

float VideoView::speed() const { return pipe_ ? pipe_->speed() : 1.f; }

void VideoView::step_frame() {
    if (pipe_) pipe_->step_frame();
}

void VideoView::seek(float seconds) {
    if (pipe_) pipe_->seek(seconds);
}

double VideoView::duration() const { return pipe_ ? pipe_->duration() : 0.0; }

float VideoView::position() const { return pipe_ ? pipe_->position() : 0.f; }

bool VideoView::has_audio() const { return pipe_ != nullptr && pipe_->has_audio(); }

void VideoView::set_volume(float volume) {
    if (pipe_) pipe_->set_volume(volume);
}

float VideoView::volume() const { return pipe_ ? pipe_->volume() : 1.f; }

bool VideoView::advance(float dt) {
    if (!pipe_ || !pipe_->has_video()) return false;
    if (!pipe_->advance(dt)) return false;
    const unsigned char* rgba = pipe_->latest_rgba();
    if (rgba) {
        texture_->update(rgba);
        has_frame_ = true;
    }
    return true;
}

void VideoView::on_draw(Renderer& renderer) {
    if (!has_frame_ || !texture_) return;
    const float vw = (float)texture_->width();
    const float vh = (float)texture_->height();
    if (vw <= 0.f || vh <= 0.f) return;

    // 保持宽高比，居中适配
    const float img_aspect = vw / vh;
    const float box_aspect = bounds_.w / bounds_.h;
    Rect r = bounds_;
    if (box_aspect > img_aspect) {
        const float w = bounds_.h * img_aspect;
        r.x = bounds_.x + (bounds_.w - w) * 0.5f;
        r.w = w;
    } else {
        const float h = bounds_.w / img_aspect;
        r.y = bounds_.y + (bounds_.h - h) * 0.5f;
        r.h = h;
    }
    renderer.draw_textured_quad(r, texture_->id(), Rect{0.f, 0.f, 1.f, 1.f},
                                Vec4{1.f, 1.f, 1.f, 1.f});
}

}  // namespace ui