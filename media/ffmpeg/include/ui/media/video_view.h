#pragma once

#include "ui/ui/widget.h"

#include <cstdint>

namespace ui {
class Renderer;
class Texture;
namespace media {
class MediaPipeline;
}

// 视频控件（M7：解码线程 + 播放控制）：后台线程持续解码，主线程逐帧上传纹理。
// UI-191：实现已整体迁移到媒体管线（media/core MediaPipeline + media/ffmpeg 节点）
// ——公共 API 与行为语义保持不动，FFmpeg 细节隔离在节点实现中（头文件不暴露 C 头）。
class VideoView : public Widget {
public:
    VideoView();
    ~VideoView() override;

    // 打开视频文件（mp4 等 FFmpeg 支持格式），失败返回 false
    bool open_source(const char* path);
    void close_source();
    bool has_video() const;

    // ---- 播放控制（M7）----
    void play();
    void pause();
    void set_playing(bool playing);
    bool playing() const;
    void set_loop(bool loop);
    bool loop() const;
    // 播放倍速（0.1..4，M13）；影响播放节奏与解码节流
    void set_speed(float speed);
    float speed() const;
    // 暂停并解码下一帧（M13 逐帧步进）
    void step_frame();
    // 跳到指定秒（解码线程处理，主线程不阻塞；关键帧后前进解码到精确位置）
    void seek(float seconds);
    double duration() const;
    float position() const;

    // ---- 音频（M14 遗留修复：winmm 输出 + 音量）----
    // 文件是否含可解码音频流
    bool has_audio() const;
    // 音量 0..1（默认 1），实时作用于解码出的 PCM
    void set_volume(float volume);
    float volume() const;

    // 每帧调用：推进播放并上传最新解码帧；返回是否有新帧上传（GL 调用必须在主线程）
    bool advance(float dt);

    void on_draw(Renderer& renderer) override;

private:
    media::MediaPipeline* pipe_ = nullptr;  // 管线持有解码线程与播放状态（open_source 建立）
    Texture* texture_ = nullptr;
    bool has_frame_ = false;
};

}  // namespace ui