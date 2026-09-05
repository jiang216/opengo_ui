#pragma once

// 媒体管线（UI-191）：Source/Decoder/AudioSink 节点组装 + 解码线程 + 播放控制。
// 语义与既有 VideoView（M7/M13/M14）逐条对齐：
//   - 解码线程持续解码，主线程 advance(dt) 弹最新帧（单槽交接，位置不跳变 UI-015）
//   - 解码节流：每帧按 1/(fps*speed) 推进（EOF/loop/seek/step/暂停语义平移至此）
//   - 纯 GL 无关：RGBA 帧交接为主，纹理上传由宿主（VideoView/VideoContainer）做
// 线程约束：open()/advance()/seek() 等由主线程调用；解码线程只访问解码器内部与源。

#include "ui/media/pipeline/node.h"
#include "ui/media/pipeline/registry.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ui {
namespace media {

class MediaPipeline {
public:
    MediaPipeline();
    ~MediaPipeline();

    MediaPipeline(const MediaPipeline&) = delete;
    MediaPipeline& operator=(const MediaPipeline&) = delete;

    // ---- 节点选择（open 之前调用；默认 ffmpeg/ffmpeg/winmm）----
    void set_source_name(const char* name);
    void set_decoder_name(const char* name);
    void set_audio_name(const char* name);
    const char* source_name() const { return source_name_.c_str(); }

    // 打开描述串（如文件路径 / "synthetic:4"）。descriptor 以 "synthetic" 开头时
    // 自动切换 test-pattern 源 + null 音频（合成源演示，不需 FFmpeg）。
    bool open(const char* descriptor);
    void close();
    bool has_video() const { return source_ != nullptr && source_->has_video(); }
    bool has_audio() const;

    // ---- 播放控制（同 VideoView）----
    void play();
    void pause();
    void set_playing(bool playing);
    bool playing() const { return playing_.load(); }
    void set_loop(bool loop) { loop_.store(loop); }
    bool loop() const { return loop_.load(); }
    void set_speed(float speed);
    float speed() const { return speed_.load(); }
    void step_frame();
    void seek(float seconds);
    double duration() const { return duration_; }
    float position() const { return (float)pos_seconds_.load(); }
    void set_volume(float volume);
    float volume() const { return volume_.load(); }

    // ---- 主线程逐帧推进 ----
    // 返回 true 表示有最新一帧已就位（latest_rgba() 有效，需上传纹理）。语义同 VideoView::advance。
    bool advance(float dt);
    const unsigned char* latest_rgba() const { return latest_frame_.empty() ? nullptr : latest_frame_.data(); }
    int latest_width() const { return latest_w_; }
    int latest_height() const { return latest_h_; }
    bool has_frame() const { return has_frame_; }

    // 解码器信息（open 后有效）
    double fps() const { return fps_.load(); }

private:
    void decode_loop();
    bool decode_one_frame();
    void do_seek(std::int64_t ms, float target);
    void do_loop_restart();
    void setup_default_nodes();
    void resolve_node_names(bool synthetic);

    MediaSource* source_ = nullptr;
    VideoDecoder* decoder_ = nullptr;
    AudioSink* audio_ = nullptr;
    std::string default_source_ = "ffmpeg";  // 用户通过 set_*_name 配置的默认节点
    std::string default_decoder_ = "ffmpeg";
    std::string default_audio_ = "winmm";
    std::string source_name_;  // 本次 open 实际使用的节点名
    std::string decoder_name_;
    std::string audio_name_;

    std::atomic<bool> running_{false};
    std::atomic<bool> playing_{true};
    std::atomic<bool> loop_{true};
    std::atomic<bool> eof_{false};
    std::atomic<std::int64_t> seek_ms_{-1};
    std::atomic<float> seek_target_{-1.f};
    std::atomic<float> speed_{1.f};
    std::atomic<float> volume_{1.f};
    std::atomic<bool> step_{false};
    std::atomic<double> pos_seconds_{0.0};
    std::atomic<double> fps_{25.0};

    std::thread decode_thread_;
    std::mutex frame_mutex_;
    std::condition_variable frame_cv_;

    std::vector<unsigned char> pending_frame_;   // decode -> main 交接缓冲
    std::vector<unsigned char> latest_frame_;    // main 线程最新可用帧（advance 成功时更新）
    int latest_w_ = 0;
    int latest_h_ = 0;
    bool has_frame_ = false;
    bool source_eof_ = false;

    float play_time_ = 0.f;
    double duration_ = 0.0;
    RgbaFrame frame_buf_;  // pull_frame 目标（data 在 open 时 resize 一次，解码线程专用）
};

}  // namespace media
}  // namespace ui