#pragma once

// 媒体管线节点接口（UI-191）：Source / Decoder / AudioSink 三类可更换节点。
// 设计约束：media/core 不依赖 FFmpeg（无 extern "C"、无 C 头）；
// FFmpeg 具体实现（FfmpegSource/FfmpegVideoDecoder/FfmpegAudioSink）在 media/ffmpeg 提供并注册，
// TestPatternSource（合成源）在 media/core 内随库携带，保证纯 UI 工程也能演示/冒烟视频容器。
//
// 数据流：
//   Source (ReadPacket) --视频包--> VideoDecoder --RGBA 帧--> 主线程 FrameSink(Texture)
//                         --音频包--> AudioSink --winmm PCM--> 声卡
// 包内容为不透明句柄（FFmpeg 实现 = AVPacket*），头文件不暴露 C 类型。

#include "ui/core/math.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ui {
namespace media {

// 管线统一交换的 RGBA 帧（y 向下、每像素 4 字节 BGRA/RGBA，与 GL Texture 上传一致）
struct RgbaFrame {
    int width = 0;
    int height = 0;
    double position = 0.0;  // 帧时间（秒）；无 PTS 时由解码器按 1/fps 推算
    std::vector<unsigned char> data;  // width*height*4
};

// 压缩数据包句柄：opaque 由实现定义（FFmpeg 实现 = AVPacket*），不暴露 C 类型
struct PacketHandle {
    bool is_video = true;  // true=视频包进 VideoDecoder；false=音频包进 AudioSink
    void* opaque = nullptr;
    std::int64_t seq = 0;  // 帧序号（合成源用：解码器按 seq/fps 计算位置；FFmpeg 忽略）
};

// 媒体源节点：打开描述串 + 元信息 + 逐包读取 + 定位
class MediaSource {
public:
    virtual ~MediaSource() = default;
    // 打开描述串（如文件路径 / "synthetic:4"）；失败返回 false（可重复调用）
    virtual bool open(const char* descriptor) = 0;
    virtual void close() = 0;
    virtual bool has_video() const = 0;
    virtual bool has_audio() const = 0;
    virtual int width() const = 0;  // has_video() 时有效
    virtual int height() const = 0;
    virtual double fps() const = 0;
    virtual double duration() const = 0;
    // 读取下一包到 out；返回 false 表示源已结束（EOF）
    virtual bool read_packet(PacketHandle& out) = 0;
    // 定位到 seconds（秒，0..duration）；解码器需另行 flush()
    virtual void seek(double seconds) = 0;
    // 源是否已读到 EOF（供管线循环空转保护）
    virtual bool eof() const = 0;
};

// 视频解码节点：压缩视频包 -> RGBA 帧（decode 写入调用方提供缓冲，避免每帧分配）
class VideoDecoder {
public:
    virtual ~VideoDecoder() = default;
    // 绑定源（读元信息建立解码器）；失败返回 false
    virtual bool bind(MediaSource& src) = 0;
    // 推入一个视频包；内部处理解码器缓冲满（EAGAIN 语义，与既有 VideoView 一致）
    virtual void push_packet(const PacketHandle& pkt) = 0;
    // 尝试解出一帧写入 out（data 由调用方预分配真尺寸）；true 有帧 / false 无
    // （需继续推包、或源已 EOF 已 flush 仍未出帧——此时 eof() 判定结束）
    virtual bool pull_frame(RgbaFrame& out) = 0;
    // 源读完（EOF）后调用一次：冲刷解码器之内缓冲帧（等价既有 avcodec_send_packet(null)）
    virtual void finish_input() = 0;
    // seek/eof 循环时清解码器缓冲（等价既有 avcodec_flush_buffers）
    virtual void flush() = 0;
    // 是否已解码到末尾（pull_frame 不返回帧且内部 EOF 置位）
    virtual bool eof() const = 0;
};

// 音频输出节点：音频包解码 -> 16bit 交错立体声 PCM 写设备；设备能力不足/失败视为静音
class AudioSink {
public:
    virtual ~AudioSink() = default;
    // 绑定源（读音频流参数建立解码器+输出设备）；失败返回 false（调用方视为静音）
    virtual bool bind(MediaSource& src) = 0;
    virtual bool has_audio() const = 0;
    // 推入一个音频包；内部完成解码+重采样+写设备（含 EAGAIN 腾空，与既有 VideoView 一致）
    virtual void push_packet(const PacketHandle& pkt) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    // seek / eof 循环时复位：清解码器缓冲 + 清输出设备队列
    virtual void flush() = 0;
    virtual void close() = 0;
    // 音量注入（0..1）：解码线程写设备前实时读取应用
    virtual void set_volume_source(const std::atomic<float>* vol) = 0;
};

}  // namespace media
}  // namespace ui