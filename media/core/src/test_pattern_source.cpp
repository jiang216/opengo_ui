#include "ui/media/pipeline/node.h"
#include "ui/media/pipeline/registry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace ui {
namespace media {

// ---------- TestPatternSource：纯 CPU 合成源（无需 FFmpeg / 无需素材） ----------
// 生成 SMPTE 色彩条 + 底部进度条，帧内容仅依赖绝对位置 -> --shot-page 截图确定性可回归。
// descriptor: "synthetic" 或 "synthetic:<秒>"（默认 4s，320x180 @ 25fps）。
// 同时实现 MediaSource 与 VideoDecoder：packet 携带 seq（帧序号），decoder 按 seq/fps
// 计算位置——seek 后 source 重置 seq，decoder 用最新 seq，无需共享计数器。

class TestPatternSource : public MediaSource {
public:
    bool open(const char* descriptor) override {
        close();
        double secs = 4.0;
        int n = 0;
        double v = 0.0;
        if (descriptor && std::sscanf(descriptor, "synthetic:%9lf%n", &v, &n) >= 1 && n > 0)
            secs = v;
        secs = std::max(secs, 0.5);
        duration_ = secs;
        total_frames_ = (std::int64_t)std::llround(secs * kFps);
        next_seq_ = 0;
        return true;
    }
    void close() override {
        duration_ = 0.0;
        total_frames_ = 0;
        next_seq_ = 0;
    }
    bool has_video() const override { return total_frames_ > 0; }
    bool has_audio() const override { return false; }
    int width() const override { return kWidth; }
    int height() const override { return kHeight; }
    double fps() const override { return kFps; }
    double duration() const override { return duration_; }

    bool read_packet(PacketHandle& out) override {
        if (next_seq_ >= total_frames_) return false;
        out.is_video = true;
        out.opaque = nullptr;
        out.seq = next_seq_++;
        return true;
    }

    void seek(double seconds) override {
        const double s = std::max(0.0, std::min(seconds, duration_));
        next_seq_ = std::clamp((std::int64_t)std::llround(s * kFps), (std::int64_t)0,
                               total_frames_);
    }

    bool eof() const override { return next_seq_ >= total_frames_; }

private:
    static constexpr int kWidth = 320;
    static constexpr int kHeight = 180;
    static constexpr double kFps = 25.0;
    static constexpr std::int64_t kPtsPerFrame = (std::int64_t)1000000 / (std::int64_t)kFps;

    double duration_ = 0.0;
    std::int64_t total_frames_ = 0;
    std::int64_t next_seq_ = 0;
};

namespace {

void put_pixel(unsigned char* dst, int cx, int cy, int width, int height, unsigned char r,
               unsigned char g, unsigned char b) {
    (void)width;
    (void)height;
    dst[(cy * width + cx) * 4 + 0] = r;
    dst[(cy * width + cx) * 4 + 1] = g;
    dst[(cy * width + cx) * 4 + 2] = b;
    dst[(cy * width + cx) * 4 + 3] = 255;
}

// SMPTE 七色条（RGB/255）
constexpr unsigned char kBars[7][3] = {
    {192, 192, 192},  // white
    {192, 192, 0},    // yellow
    {0, 192, 192},    // cyan
    {0, 192, 0},      // green
    {192, 0, 192},    // magenta
    {192, 0, 0},      // red
    {0, 0, 192},      // blue
};

}  // namespace

class TestPatternDecoder : public VideoDecoder {
public:
    bool bind(MediaSource& src) override {
        width_ = src.width();
        height_ = src.height();
        fps_ = src.fps() > 0.0 ? src.fps() : 25.0;
        duration_ = src.duration();
        return true;
    }
    void push_packet(const PacketHandle& pkt) override {
        have_pkt_ = true;
        seq_ = pkt.seq;
    }
    bool pull_frame(RgbaFrame& out) override {
        if (!have_pkt_ || eof_) return false;
        have_pkt_ = false;
        const double pos = (double)seq_ / fps_;
        fill_bars(out, pos);
        out.position = pos;
        return true;
    }
    void finish_input() override { eof_ = true; }
    void flush() override {
        have_pkt_ = false;
        eof_ = false;
    }
    bool eof() const override { return eof_; }

private:
    void fill_bars(RgbaFrame& out, double pos) {
        out.width = width_;
        out.height = height_;
        if (out.data.size() < (size_t)width_ * height_ * 4) return;
        const int bar_h = height_ * 4 / 5;
        const double prog = duration_ > 0.0 ? std::max(0.0, std::min(1.0, pos / duration_)) : 0.0;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (y < bar_h) {
                    const int band = std::min((int)(x * 7 / width_), 6);
                    put_pixel(out.data.data(), x, y, width_, height_, kBars[band][0],
                              kBars[band][1], kBars[band][2]);
                } else {
                    const unsigned char dark = 24;
                    const unsigned char fill =
                        (double)x / (double)width_ < prog ? 150 : dark;
                    put_pixel(out.data.data(), x, y, width_, height_, fill, fill, fill);
                }
            }
        }
    }

    int width_ = 0;
    int height_ = 0;
    double fps_ = 25.0;
    double duration_ = 0.0;
    std::int64_t seq_ = 0;
    bool have_pkt_ = false;
    bool eof_ = false;
};

// ---------- NullAudioSink：设备占位（合成源演示/无声环境） ----------
class NullAudioSink : public AudioSink {
public:
    bool bind(MediaSource&) override { return false; }
    bool has_audio() const override { return false; }
    void push_packet(const PacketHandle&) override {}
    void pause() override {}
    void resume() override {}
    void flush() override {}
    void close() override {}
    void set_volume_source(const std::atomic<float>*) override {}
};

namespace {

bool register_test_nodes() {
    NodeRegistry& r = node_registry();
    r.register_source("test-pattern", []() -> MediaSource* { return new TestPatternSource; });
    r.register_decoder("test-pattern",
                       []() -> VideoDecoder* { return new TestPatternDecoder; });
    r.register_audio("null", []() -> AudioSink* { return new NullAudioSink; });
    return true;
}

}  // namespace

// 显式注册入口：由 MediaPipeline 构造函数调用（见 pipeline.cpp），
// 保证被静态链接工具链裁掉本 TU 时合成源仍然可用（幂等）。
void register_core_nodes() {
    static const bool registered = register_test_nodes();
    (void)registered;
}

}  // namespace media
}  // namespace ui