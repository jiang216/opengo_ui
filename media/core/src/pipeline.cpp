#include "ui/media/pipeline/pipeline.h"

#include "ui/core/log.h"

#include <algorithm>
#include <chrono>
#include <cstring>

namespace ui {
namespace media {

MediaPipeline::MediaPipeline() {
    // 确保合成本地节点已注册（防静态链接裁剪未引用 TU；幂等）
    register_core_nodes();
}

MediaPipeline::~MediaPipeline() {
    close();
}

void MediaPipeline::set_source_name(const char* name) {
    if (name && *name) default_source_ = name;
}

void MediaPipeline::set_decoder_name(const char* name) {
    if (name && *name) default_decoder_ = name;
}

void MediaPipeline::set_audio_name(const char* name) {
    if (name && *name) default_audio_ = name;
}

bool MediaPipeline::has_audio() const {
    return audio_ && audio_->has_audio();
}

void MediaPipeline::resolve_node_names(bool synthetic) {
    if (synthetic) {
        source_name_ = "test-pattern";
        decoder_name_ = "test-pattern";
        audio_name_ = "null";
    } else {
        source_name_ = default_source_;
        decoder_name_ = default_decoder_;
        audio_name_ = default_audio_;
    }
}

void MediaPipeline::setup_default_nodes() {
    NodeRegistry& reg = node_registry();
    source_ = reg.make_source(source_name_.c_str());
    if (!source_) log_error("media: source node '%s' not registered", source_name_.c_str());
    decoder_ = reg.make_decoder(decoder_name_.c_str());
    if (!decoder_) log_error("media: decoder node '%s' not registered", decoder_name_.c_str());
    audio_ = reg.make_audio(audio_name_.c_str());
    if (!audio_) log_error("media: audio node '%s' not registered", audio_name_.c_str());
}

bool MediaPipeline::open(const char* descriptor) {
    close();
    if (!descriptor || !*descriptor) return false;

    // 合成源快速通道：descriptor 以 "synthetic" 开头时切换 test-pattern 节点（无需 FFmpeg）。
    // resolve_node_names 每次 open 按描述串独立解出本次节点名，避免先合成后真实文件互相污染。
    const bool synthetic = std::strncmp(descriptor, "synthetic", 9) == 0;
    resolve_node_names(synthetic);

    setup_default_nodes();
    if (!source_ || !decoder_) {
        log_error("media: pipeline nodes unavailable (source=%s decoder=%s)", source_name_.c_str(),
                  decoder_name_.c_str());
        close();
        return false;
    }

    if (!source_->open(descriptor)) {
        log_error("media: cannot open source '%s'", descriptor);
        close();
        return false;
    }
    if (!source_->has_video()) {
        log_error("media: no video stream in '%s'", descriptor);
        close();
        return false;
    }

    if (!decoder_->bind(*source_)) {
        log_error("media: decoder bind failed");
        close();
        return false;
    }
    if (audio_) {
        if (!audio_->bind(*source_)) {
            // 音频失败仅静音降级（同 VideoView M14 语义），音频节点继续持有（has_audio=false）
            audio_->close();
            log_info("media: no audio stream, muted");
        }
        audio_->set_volume_source(&volume_);
    }

    duration_ = source_->duration();
    fps_.store(source_->fps() > 0.0 ? source_->fps() : 25.0);
    latest_w_ = source_->width();
    latest_h_ = source_->height();
    frame_buf_.width = latest_w_;
    frame_buf_.height = latest_h_;
    frame_buf_.data.resize((size_t)latest_w_ * latest_h_ * 4);
    pending_frame_.clear();
    latest_frame_.clear();
    play_time_ = 0.f;
    pos_seconds_.store(0.0);
    seek_ms_.store(-1);
    seek_target_.store(-1.f);
    speed_.store(1.f);
    step_.store(false);
    playing_.store(true);
    loop_.store(true);
    eof_.store(false);
    source_eof_ = false;
    has_frame_ = false;

    running_.store(true);
    decode_thread_ = std::thread(&MediaPipeline::decode_loop, this);
    log_info("media: pipeline opened '%s' (%dx%d @ %.2ffps, %.2fs)", descriptor, latest_w_,
             latest_h_, fps_.load(), duration_);
    return true;
}

void MediaPipeline::close() {
    if (!source_ && !audio_) return;
    running_.store(false);
    frame_cv_.notify_all();
    if (decode_thread_.joinable()) {
        decode_thread_.join();
    }
    decode_thread_ = std::thread();
    if (decoder_) {
        delete decoder_;
        decoder_ = nullptr;
    }
    if (audio_) {
        audio_->close();
        delete audio_;
        audio_ = nullptr;
    }
    if (source_) {
        source_->close();
        delete source_;
        source_ = nullptr;
    }
    has_frame_ = false;
}

void MediaPipeline::play() {
    if (eof_.load()) seek(0.f);
    playing_.store(true);
    eof_.store(false);
    if (audio_) audio_->resume();
    frame_cv_.notify_all();
}

void MediaPipeline::pause() {
    playing_.store(false);
    if (audio_) audio_->pause();
    frame_cv_.notify_all();
}

void MediaPipeline::set_playing(bool playing) {
    if (playing)
        play();
    else
        pause();
}

void MediaPipeline::set_volume(float volume) {
    volume_.store(volume < 0.f ? 0.f : (volume > 1.f ? 1.f : volume));
}

void MediaPipeline::set_speed(float speed) {
    speed_.store(speed < 0.1f ? 0.1f : (speed > 4.f ? 4.f : speed));
    frame_cv_.notify_all();
}

void MediaPipeline::step_frame() {
    playing_.store(false);
    step_.store(true);
    frame_cv_.notify_all();
}

void MediaPipeline::seek(float seconds) {
    if (seconds < 0.f) seconds = 0.f;
    if (duration_ > 0.0 && (double)seconds > duration_) seconds = (float)duration_;
    pos_seconds_.store((double)seconds);  // UI-196：立即同步目标位置，UI 手柄/时间即时反映用户拖拽目标
    seek_ms_.store((std::int64_t)(seconds * 1000.f));
    seek_target_.store(seconds);
    play_time_ = seconds;
    frame_cv_.notify_all();
}

// 解码一帧：先收解码器缓冲（视频包来源=源 read_packet 推送），空则读源补包
bool MediaPipeline::decode_one_frame() {
    for (;;) {
        if (decoder_->pull_frame(frame_buf_)) {
            pos_seconds_.store(frame_buf_.position);
            return true;
        }
        if (decoder_->eof()) return false;

        if (!source_eof_) {
            PacketHandle pkt;
            if (source_->read_packet(pkt)) {
                if (pkt.is_video) {
                    decoder_->push_packet(pkt);
                } else if (audio_ && audio_->has_audio()) {
                    audio_->push_packet(pkt);
                }
                continue;
            }
            // 源读完：冲刷解码器吐尽缓冲帧
            source_eof_ = true;
            decoder_->finish_input();
            continue;
        }
        // 源已 EOF 且解码器缓冲已空但未置 EOF：按「无帧可出」处理（防死循环）
        return false;
    }
}

void MediaPipeline::do_seek(std::int64_t ms, float target) {
    const double sec = (double)ms / 1000.0;
    source_->seek(sec);
    decoder_->flush();
    if (audio_) audio_->flush();
    source_eof_ = false;
    eof_.store(false);
    // 精确 seek：BACKWARD 定位到目标前关键帧，前进解码到目标时刻（守卫同 VideoView UI-018）。
    // UI-196：守卫上限提高到 6s 帧数（合成源/常见素材从任意关键帧追回目标足够），
    // 解码不足时不中途退出——seek 后从目标时刻续播，位置以解码帧为准；
    // 耗尽仍不足目标时把 pos 补到目标（避免 UI 手柄落后于用户拖拽目标）。
    const int guard_max = 6 * (int)fps_.load() + 2;
    int guard = 0;
    while (pos_seconds_.load() < (double)target - 0.02 && guard++ < guard_max) {
        if (!decode_one_frame()) break;
    }
    {
        std::lock_guard<std::mutex> lk(frame_mutex_);
        if (!frame_buf_.data.empty()) pending_frame_ = frame_buf_.data;
    }
    // 目标时刻已到（或源无更多帧）：确保 pos 至少到目标，UI 进度条与目标一致
    if (target >= 0.f && pos_seconds_.load() < (double)target - 0.02) {
        pos_seconds_.store((double)target);
    }
}

void MediaPipeline::do_loop_restart() {
    source_->seek(0.0);
    decoder_->flush();
    if (audio_) audio_->flush();
    source_eof_ = false;
    eof_.store(false);
}

void MediaPipeline::decode_loop() {
    log_info("media: decode thread start");
    auto next_push = std::chrono::steady_clock::now();
    while (running_.load()) {
        {
            std::unique_lock<std::mutex> lk(frame_mutex_);
            frame_cv_.wait(lk, [this] {
                return !running_.load() || playing_.load() || seek_ms_.load() >= 0 ||
                       step_.load();
            });
        }
        if (!running_.load()) break;

        if (seek_ms_.load() >= 0) {
            const std::int64_t ms = seek_ms_.exchange(-1);
            const float target = seek_target_.exchange(-1.f);
            do_seek(ms, target);
            next_push = std::chrono::steady_clock::now();
            continue;
        }

        if (step_.exchange(false)) {
            if (decode_one_frame()) {
                std::lock_guard<std::mutex> lk(frame_mutex_);
                pending_frame_ = frame_buf_.data;
            }
            continue;
        }

        if (!decode_one_frame()) {
            if (eof_.load()) {
                // 播完且不循环：等待 play()/seek 重置
                std::unique_lock<std::mutex> lk(frame_mutex_);
                frame_cv_.wait(lk, [this] {
                    return !running_.load() || !eof_.load() || seek_ms_.load() >= 0;
                });
                continue;
            }
            if (source_eof_ && decoder_->eof()) {
                if (loop_.load()) {
                    do_loop_restart();
                    next_push = std::chrono::steady_clock::now();
                    continue;
                }
                eof_.store(true);
                std::unique_lock<std::mutex> lk(frame_mutex_);
                frame_cv_.wait(lk, [this] {
                    return !running_.load() || !eof_.load() || seek_ms_.load() >= 0;
                });
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        {
            std::lock_guard<std::mutex> lk(frame_mutex_);
            pending_frame_ = frame_buf_.data;
        }
        const float speed = speed_.load();
        next_push += std::chrono::microseconds((std::int64_t)(1e6 / (fps_.load() * speed)));
        std::this_thread::sleep_until(next_push);
    }
    log_info("media: decode thread exit");
}

bool MediaPipeline::advance(float dt) {
    if (!has_video()) return false;
    if (!playing_.load()) {
        // 暂停：仅上传解码线程最新帧（如 seek 后刷新显示）
        std::lock_guard<std::mutex> lk(frame_mutex_);
        if (pending_frame_.empty()) return false;
        latest_frame_.swap(pending_frame_);
        has_frame_ = true;
        return true;
    }
    const float speed = speed_.load();
    if (speed <= 0.f) return false;
    const float frame_interval = 1.0f / (float)(fps_.load() * speed);
    if (!has_frame_) {
        // 首帧即时放行：刚打开时立即显示第一帧，避免先闪占位/截图过早捕获空面
        std::lock_guard<std::mutex> lk(frame_mutex_);
        if (pending_frame_.empty()) return false;
        latest_frame_.swap(pending_frame_);
        has_frame_ = true;
        return true;
    }
    play_time_ += dt;
    if (play_time_ < frame_interval) return false;
    play_time_ -= frame_interval;
    std::lock_guard<std::mutex> lk(frame_mutex_);
    if (pending_frame_.empty()) return false;
    latest_frame_.swap(pending_frame_);
    has_frame_ = true;
    return true;
}

}  // namespace media
}  // namespace ui