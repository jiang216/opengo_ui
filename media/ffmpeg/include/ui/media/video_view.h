#pragma once

#include "ui/ui/widget.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

namespace ui {
class Renderer;
class Texture;

// 视频控件（M7：解码线程 + 播放控制）：后台线程持续解码，主线程逐帧上传纹理。
// FFmpeg 细节全部藏在 src（头文件不暴露 C 头，VideoContext/AudioContext 不透明指针）。
// M14 遗留修复：音频流解码 + winmm 输出 + 音量（无音频流时 has_audio()==false，其余功能不变）。
struct VideoContext;
struct AudioContext;

class VideoView : public Widget {
public:
    VideoView();
    ~VideoView() override;

    // 打开视频文件（mp4 等 FFmpeg 支持格式），失败返回 false
    bool open_source(const char* path);
    void close_source();
    bool has_video() const { return ctx_ != nullptr; }

    // ---- 播放控制（M7）----
    void play();
    void pause();
    void set_playing(bool playing);
    bool playing() const { return playing_.load(); }
    void set_loop(bool loop) { loop_.store(loop); }
    bool loop() const { return loop_.load(); }
    // 播放倍速（0.1..4，M13）；影响播放节奏与解码节流
    void set_speed(float speed);
    float speed() const { return speed_.load(); }
    // 暂停并解码下一帧（M13 逐帧步进）
    void step_frame();
    // 跳到指定秒（解码线程处理，主线程不阻塞；关键帧后前进解码到精确位置）
    void seek(float seconds);
    double duration() const { return duration_; }
    float position() const { return (float)pos_seconds_.load(); }

    // ---- 音频（M14 遗留修复：winmm 输出 + 音量）----
    // 文件是否含可解码音频流
    bool has_audio() const { return audio_ctx_ != nullptr; }
    // 音量 0..1（默认 1），实时作用于解码出的 PCM
    void set_volume(float volume);
    float volume() const { return volume_.load(); }

    // 每帧调用：推进播放并上传最新解码帧；返回是否有新帧上传（GL 调用必须在主线程）
    bool advance(float dt);

    void on_draw(Renderer& renderer) override;

private:
    void decode_loop();
    bool decode_one_frame();
    void handle_audio_packet(void* pkt);  // AVPacket*（不透明避免暴露 C 头）

    VideoContext* ctx_ = nullptr;  // 仅解码线程访问（open_source 建好并启动线程后）
    AudioContext* audio_ctx_ = nullptr;
    Texture* texture_ = nullptr;
    bool has_frame_ = false;

    std::atomic<bool> running_{false};
    std::atomic<bool> playing_{true};
    std::atomic<bool> loop_{true};
    std::atomic<bool> eof_{false};  // 已播到片尾（loop 关时置位，play() 据此回 0）
    std::atomic<std::int64_t> seek_ms_{-1};
    std::atomic<float> seek_target_{-1.f};  // 精确 seek 目标秒（解码线程用）
    std::atomic<float> speed_{1.f};
    std::atomic<float> volume_{1.f};
    std::atomic<bool> step_{false};  // 帧步进请求
    std::atomic<double> pos_seconds_{0.0};
    std::thread decode_thread_;
    std::mutex frame_mutex_;
    std::condition_variable frame_cv_;
    std::vector<unsigned char> pending_frame_;  // decode -> main 交接缓冲
    std::vector<unsigned char> decoded_rgba_;   // 解码线程专用（open 时 resize 一次）

    float play_time_ = 0.f;
    std::atomic<double> fps_{25.0};  // 主线程 open_source 写、解码线程 decode_loop 读（UI-158 数据竞争）
    double duration_ = 0.0;
};

}  // namespace ui
