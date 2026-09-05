#include "ui/media/ffmpeg_nodes.h"

#include "ui/core/log.h"
#include "ui/media/audio_out.h"
#include "ui/media/pipeline/node.h"
#include "ui/media/pipeline/pipeline.h"
#include "ui/media/pipeline/registry.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <atomic>
#include <cstdint>

namespace ui {
namespace media {

namespace {

// ---- FFmpeg 源节点：打开容器 + 读取压缩包（等价移自 VideoView::open_source / decode 读包）----
class FfmpegSource : public MediaSource {
public:
    bool open(const char* descriptor) override {
        close();
        if (!descriptor || !*descriptor) return false;

        if (avformat_open_input(&fmt_, descriptor, nullptr, nullptr) < 0) {
            log_error("media/ffmpeg: cannot open %s", descriptor);
            close();
            return false;
        }
        if (avformat_find_stream_info(fmt_, nullptr) < 0) {
            log_error("media/ffmpeg: find_stream_info failed");
            close();
            return false;
        }

        const AVCodec* vdec = nullptr;
        video_stream_ = av_find_best_stream(fmt_, AVMEDIA_TYPE_VIDEO, -1, -1, &vdec, 0);
        if (video_stream_ < 0 || !vdec) {
            log_error("media/ffmpeg: no video stream in %s", descriptor);
            close();
            return false;
        }
        audio_stream_ = av_find_best_stream(fmt_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

        const AVCodecParameters* vp = fmt_->streams[video_stream_]->codecpar;
        width_ = vp->width;
        height_ = vp->height;

        const AVRational fr = fmt_->streams[video_stream_]->avg_frame_rate;
        if (fr.num > 0 && fr.den > 0) fps_ = av_q2d(fr);
        if (fps_ <= 0.0) fps_ = 25.0;

        if (fmt_->duration > 0) {
            duration_ = (double)fmt_->duration / (double)AV_TIME_BASE;
        } else {
            const AVStream* st = fmt_->streams[video_stream_];
            if (st->duration > 0 && st->time_base.den > 0)
                duration_ = (double)st->duration * av_q2d(st->time_base);
        }

        pkt_ = av_packet_alloc();
        if (!pkt_) {
            close();
            return false;
        }
        log_info("media/ffmpeg: opened '%s' (%dx%d @ %.2ffps, %.2fs)", descriptor, width_, height_,
                 fps_, duration_);
        return true;
    }

    void close() override {
        if (pkt_) av_packet_free(&pkt_);
        if (fmt_) {
            avformat_close_input(&fmt_);
        }
        video_stream_ = -1;
        audio_stream_ = -1;
        width_ = height_ = 0;
        fps_ = 25.0;
        duration_ = 0.0;
        fmt_eof_ = false;
    }

    bool has_video() const override { return video_stream_ >= 0; }
    bool has_audio() const override { return audio_stream_ >= 0; }
    int width() const override { return width_; }
    int height() const override { return height_; }
    double fps() const override { return fps_; }
    double duration() const override { return duration_; }

    bool read_packet(PacketHandle& out) override {
        if (!fmt_) return false;
        for (;;) {
            av_packet_unref(pkt_);  // 每次读取前确保可复用（同上一个包是否已被消费）
            const int pr = av_read_frame(fmt_, pkt_);
            if (pr < 0) {
                fmt_eof_ = true;
                av_packet_unref(pkt_);
                return false;
            }
            if (pkt_->stream_index == video_stream_) {
                out.is_video = true;
                out.opaque = pkt_;
                return true;
            }
            if (pkt_->stream_index == audio_stream_) {
                out.is_video = false;
                out.opaque = pkt_;
                return true;
            }
            // 其余流（字幕等）：跳过，继续读下一包（同既有 VideoView 语义）
        }
    }

    void seek(double seconds) override {
        if (!fmt_) return;
        const std::int64_t us = (std::int64_t)(seconds * AV_TIME_BASE);
        av_seek_frame(fmt_, -1, us, AVSEEK_FLAG_BACKWARD);
        fmt_eof_ = false;
    }

    bool eof() const override { return fmt_eof_; }

    // 同一 TU 内供 FfmpegVideoDecoder / FfmpegAudioSink 使用（FFmpeg 类型不对外暴露）
    AVFormatContext* fmt() const { return fmt_; }
    int video_stream() const { return video_stream_; }
    int audio_stream() const { return audio_stream_; }
    AVStream* stream(int idx) const { return fmt_ ? fmt_->streams[idx] : nullptr; }

private:
    AVFormatContext* fmt_ = nullptr;
    AVPacket* pkt_ = nullptr;
    int video_stream_ = -1;
    int audio_stream_ = -1;
    int width_ = 0;
    int height_ = 0;
    double fps_ = 25.0;
    double duration_ = 0.0;
    bool fmt_eof_ = false;
};

// ---- FFmpeg 视频解码节点：压缩视频包 -> RGBA 帧（等价移自 VideoView 解码部分）----
class FfmpegVideoDecoder : public VideoDecoder {
public:
    ~FfmpegVideoDecoder() override { close(); }

    bool bind(MediaSource& src) override {
        close();
        auto* fs = dynamic_cast<FfmpegSource*>(&src);
        if (!fs) return false;
        fmt_ = fs->fmt();
        stream_ = fs->video_stream();
        if (!fmt_ || stream_ < 0) return false;

        const AVCodecParameters* cp = fmt_->streams[stream_]->codecpar;
        const AVCodec* dec = avcodec_find_decoder(cp->codec_id);
        if (!dec) return false;
        codec_ = avcodec_alloc_context3(dec);
        if (!codec_) return false;
        if (avcodec_parameters_to_context(codec_, cp) < 0) return false;
        codec_->thread_count = 1;  // 多线程解码在精简 DLL 上有死锁风险，固定单线程
        if (avcodec_open2(codec_, dec, nullptr) < 0) return false;

        width_ = codec_->width > 0 ? codec_->width : cp->width;
        height_ = codec_->height > 0 ? codec_->height : cp->height;
        frame_ = av_frame_alloc();
        if (!frame_) return false;
        eof_ = false;
        last_pos_ = 0.0;
        return true;
    }

    void close() {
        if (sws_) sws_freeContext(sws_);
        if (frame_) av_frame_free(&frame_);
        if (codec_) avcodec_free_context(&codec_);
        sws_ = nullptr;
        frame_ = nullptr;
        codec_ = nullptr;
        fmt_ = nullptr;
        stream_ = -1;
        width_ = height_ = 0;
        eof_ = false;
    }

    void push_packet(const PacketHandle& pkt) override {
        if (!codec_ || !pkt.opaque) return;
        AVPacket* p = static_cast<AVPacket*>(pkt.opaque);
        if (p->stream_index != stream_) return;  // 非本流数据包：路由错误时忽略

        // EAGAIN 表示解码器缓冲已满：外层已先 pull_frame 再推包，正常不会触发；
        // 若触发则静默丢弃（同既有 VideoView 处理），避免花屏。
        const int sret = avcodec_send_packet(codec_, p);
        if (sret == AVERROR(EAGAIN)) return;
        if (sret < 0) log_error("media/ffmpeg: video send_packet failed %d", sret);
    }

    bool pull_frame(RgbaFrame& out) override {
        if (!codec_) return false;
        for (;;) {
            const int ret = avcodec_receive_frame(codec_, frame_);
            if (ret == 0) {
                if (!sws_ || sws_src_ != frame_->format) {
                    if (sws_) sws_freeContext(sws_);
                    sws_ = sws_getContext(width_, height_, (AVPixelFormat)frame_->format, width_,
                                          height_, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr,
                                          nullptr);
                    sws_src_ = (AVPixelFormat)frame_->format;
                    if (!sws_) {  // 转换器创建失败：跳过本帧，不崩溃
                        log_error("media/ffmpeg: sws_getContext failed, skip frame");
                        return false;
                    }
                }
                if ((int)out.data.size() < width_ * height_ * 4) {
                    log_error("media/ffmpeg: frame buffer too small, skip frame");
                    return false;
                }
                uint8_t* dst[4] = {out.data.data(), nullptr, nullptr, nullptr};
                int dst_linesize[4] = {width_ * 4, 0, 0, 0};
                sws_scale(sws_, frame_->data, frame_->linesize, 0, height_, dst, dst_linesize);

                if (frame_->pts != AV_NOPTS_VALUE && fmt_ && stream_ >= 0) {
                    const AVRational tb = fmt_->streams[stream_]->time_base;
                    last_pos_ = av_q2d(tb) * (double)frame_->pts;
                } else {
                    last_pos_ += 1.0 / fps();
                }
                out.width = width_;
                out.height = height_;
                out.position = last_pos_;
                return true;
            }
            if (ret == AVERROR_EOF) {
                eof_ = true;
                return false;
            }
            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                eof_ = true;
                return false;
            }
            return false;  // EAGAIN：需更多包推进
        }
    }

    // 源读完（EOF）后冲刷解码器缓冲帧（等价 avcodec_send_packet(nullptr)）
    void finish_input() override {
        if (!codec_) return;
        const int sret = avcodec_send_packet(codec_, nullptr);
        if (sret < 0 && sret != AVERROR(EAGAIN))
            log_error("media/ffmpeg: flush send_packet failed %d", sret);
    }

    void flush() override {
        if (codec_) avcodec_flush_buffers(codec_);
        eof_ = false;
    }

    bool eof() const override { return eof_; }

private:
    double fps() const { return fmt_ && stream_ >= 0 ? av_q2d(fmt_->streams[stream_]->avg_frame_rate)
                                                     : 25.0; }
    AVCodecContext* codec_ = nullptr;
    AVFormatContext* fmt_ = nullptr;
    int stream_ = -1;
    int width_ = 0;
    int height_ = 0;
    SwsContext* sws_ = nullptr;
    AVPixelFormat sws_src_ = AV_PIX_FMT_NONE;
    AVFrame* frame_ = nullptr;
    bool eof_ = false;
    double last_pos_ = 0.0;
};

// ---- FFmpeg 音频输出节点：音频包 -> swr s16 立体声 -> winmm（等价移自 VideoView 音频部分）----
class FfmpegAudioSink : public AudioSink {
public:
    bool bind(MediaSource& src) override {
        close();
        auto* fs = dynamic_cast<FfmpegSource*>(&src);
        if (!fs) return false;
        AVFormatContext* fmt = fs->fmt();
        stream_ = fs->audio_stream();
        if (!fmt || stream_ < 0) return false;

        const AVCodecParameters* cp = fmt->streams[stream_]->codecpar;
        const AVCodec* dec = avcodec_find_decoder(cp->codec_id);
        if (!dec) return false;
        codec_ = avcodec_alloc_context3(dec);
        if (!codec_) return false;
        if (avcodec_parameters_to_context(codec_, cp) < 0) return false;
        codec_->thread_count = 1;  // 同视频：固定单线程避免 seek/flush 死锁
        if (avcodec_open2(codec_, dec, nullptr) < 0) return false;
        if (codec_->sample_fmt == AV_SAMPLE_FMT_NONE || codec_->sample_rate <= 0) return false;

        sample_rate_ = codec_->sample_rate;
        in_channels_ = codec_->ch_layout.nb_channels > 0 ? codec_->ch_layout.nb_channels : 2;
        AVChannelLayout src_layout, dst_layout;
        av_channel_layout_default(&src_layout, in_channels_);
        av_channel_layout_default(&dst_layout, 2);  // 输出统一立体声 s16
        const int sr = swr_alloc_set_opts2(&swr_, &dst_layout, AV_SAMPLE_FMT_S16, sample_rate_,
                                           &src_layout, codec_->sample_fmt, sample_rate_, 0,
                                           nullptr);
        av_channel_layout_uninit(&src_layout);
        av_channel_layout_uninit(&dst_layout);
        if (sr < 0 || swr_init(swr_) < 0) return false;

        frame_ = av_frame_alloc();
        if (!frame_) return false;
        pcm_cap_ = 8192;
        int linesize = 0;
        if (av_samples_alloc(&pcm_buf_, &linesize, 2, pcm_cap_, AV_SAMPLE_FMT_S16, 0) < 0)
            return false;
        out_channels_ = 2;
        ok_ = true;
        if (!out_.open(sample_rate_, 2)) log_info("media/ffmpeg: waveOut open failed, muted");
        log_info("media/ffmpeg: audio stream %d (%d Hz, %d ch)", stream_, sample_rate_,
                 in_channels_);
        return true;
    }

    bool has_audio() const override { return ok_; }

    void close() override {
        if (!ok_ && !codec_) return;
        out_.flush();
        out_.close();
        if (swr_) swr_free(&swr_);
        if (frame_) av_frame_free(&frame_);
        if (pcm_buf_) av_freep(&pcm_buf_);
        if (codec_) avcodec_free_context(&codec_);
        swr_ = nullptr;
        frame_ = nullptr;
        pcm_buf_ = nullptr;
        codec_ = nullptr;
        stream_ = -1;
        ok_ = false;
    }

    void push_packet(const PacketHandle& pkt) override {
        if (!ok_ || !codec_ || !swr_ || !pkt.opaque) return;
        AVPacket* p = static_cast<AVPacket*>(pkt.opaque);
        if (p->stream_index != stream_) return;
        // avcodec_send_packet 可能返回 EAGAIN：先消费缓冲内已解码帧腾位再重送（同既有语义）
        for (int attempt = 0; attempt < 3; ++attempt) {
            const int sret = avcodec_send_packet(codec_, p);
            if (sret != AVERROR(EAGAIN)) {
                if (sret < 0) log_error("media/ffmpeg: audio send_packet failed %d", sret);
                break;
            }
            drain_frames();
        }
        drain_frames();
    }

    void pause() override {
        if (ok_) out_.pause();
    }

    void resume() override {
        if (ok_) out_.resume();
    }

    void flush() override {
        if (codec_) avcodec_flush_buffers(codec_);
        if (ok_) out_.flush();
    }

    void set_volume_source(const std::atomic<float>* vol) override { volume_ = vol; }

private:
    // 消费音频解码缓冲：swr 重采样为 s16 立体声 -> 应用音量 -> 写 winmm 输出
    void drain_frames() {
        for (;;) {
            const int ret = avcodec_receive_frame(codec_, frame_);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) break;
            const int out_n = swr_convert(swr_, &pcm_buf_, pcm_cap_,
                                          (const uint8_t**)frame_->extended_data,
                                          frame_->nb_samples);
            if (out_n <= 0) continue;
            const int total = out_n * out_channels_;
            int16_t* s = (int16_t*)pcm_buf_;
            const float g = volume_ ? volume_->load() : 1.f;
            if (g < 1.f) {
                for (int i = 0; i < total; ++i) s[i] = (int16_t)((float)s[i] * g);
            }
            const int chunk = 4096;
            int off = 0;
            while (off < total) {
                const int n = (total - off) > chunk ? chunk : (total - off);
                out_.write(s + off, n);
                off += n;
            }
        }
    }

    AVCodecContext* codec_ = nullptr;
    int stream_ = -1;
    int sample_rate_ = 0;
    int in_channels_ = 0;
    int out_channels_ = 0;
    SwrContext* swr_ = nullptr;
    AVFrame* frame_ = nullptr;
    uint8_t* pcm_buf_ = nullptr;
    int pcm_cap_ = 0;
    AudioOut out_;
    const std::atomic<float>* volume_ = nullptr;
    bool ok_ = false;
};

}  // namespace

void register_ffmpeg_nodes() {
    static const bool registered = [] {
        NodeRegistry& reg = node_registry();
        reg.register_source("ffmpeg", [] { return new FfmpegSource; });
        reg.register_source("ffmpeg-source", [] { return new FfmpegSource; });
        reg.register_decoder("ffmpeg", [] { return new FfmpegVideoDecoder; });
        reg.register_audio("ffmpeg", [] { return new FfmpegAudioSink; });
        reg.register_audio("winmm", [] { return new FfmpegAudioSink; });
        log_info("media: ffmpeg nodes registered");
        return true;
    }();
    (void)registered;
}

}  // namespace media
}  // namespace ui