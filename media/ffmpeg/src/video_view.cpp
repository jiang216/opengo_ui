#include "ui/media/video_view.h"

#include "ui/core/log.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/media/audio_out.h"

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

#include <algorithm>
#include <chrono>

namespace ui {

// FFmpeg 解码上下文（仅解码线程访问，实现细节隔离在 cpp）
struct VideoContext {
    AVFormatContext* fmt = nullptr;
    AVCodecContext* codec = nullptr;
    int stream = -1;
    int width = 0;
    int height = 0;
    double fps = 25.0;
    SwsContext* sws = nullptr;
    AVPixelFormat sws_src = AV_PIX_FMT_NONE;
    AVFrame* frame = nullptr;
    AVPacket* pkt = nullptr;
    bool eof = false;
};

// 音频解码上下文（M14 遗留修复：winmm 输出 + 音量；仅解码线程访问）
struct AudioContext {
    AVCodecContext* codec = nullptr;
    int stream = -1;
    int sample_rate = 0;
    int in_channels = 0;
    int out_channels = 0;
    SwrContext* swr = nullptr;
    AVFrame* frame = nullptr;
    uint8_t* pcm_buf = nullptr;
    int pcm_cap = 0;  // swr 输出缓冲样本容量
    AudioOut out;
};

namespace {

// 初始化音频解码 + 重采样（s16 立体声）+ winmm 输出；失败返回 false（静音继续）
bool audio_init_ctx(AudioContext* a, AVFormatContext* fmt) {
    const AVCodec* dec = nullptr;
    a->stream = av_find_best_stream(fmt, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (a->stream < 0 || !dec) return false;
    a->codec = avcodec_alloc_context3(dec);
    if (!a->codec) return false;
    if (avcodec_parameters_to_context(a->codec, fmt->streams[a->stream]->codecpar) < 0)
        return false;
    a->codec->thread_count = 1;  // 同视频：固定单线程避免 seek/flush 死锁
    if (avcodec_open2(a->codec, dec, nullptr) < 0) return false;
    if (a->codec->sample_fmt == AV_SAMPLE_FMT_NONE || a->codec->sample_rate <= 0) return false;

    a->sample_rate = a->codec->sample_rate;
    a->in_channels = a->codec->ch_layout.nb_channels > 0 ? a->codec->ch_layout.nb_channels : 2;
    AVChannelLayout src, dst;
    av_channel_layout_default(&src, a->in_channels);
    av_channel_layout_default(&dst, 2);  // 输出统一立体声 s16
    const int sr = swr_alloc_set_opts2(&a->swr, &dst, AV_SAMPLE_FMT_S16, a->sample_rate, &src,
                                       a->codec->sample_fmt, a->sample_rate, 0, nullptr);
    av_channel_layout_uninit(&src);
    av_channel_layout_uninit(&dst);
    if (sr < 0 || swr_init(a->swr) < 0) return false;

    a->frame = av_frame_alloc();
    if (!a->frame) return false;
    a->pcm_cap = 8192;
    int linesize = 0;
    if (av_samples_alloc(&a->pcm_buf, &linesize, 2, a->pcm_cap, AV_SAMPLE_FMT_S16, 0) < 0)
        return false;
    a->out_channels = 2;
    if (!a->out.open(a->sample_rate, 2)) log_info("VideoView: waveOut open failed, muted");
    log_info("VideoView: audio stream %d (%d Hz, %d ch)", a->stream, a->sample_rate,
             a->in_channels);
    return true;
}

void audio_free_ctx(AudioContext* a) {
    if (!a) return;
    a->out.flush();
    a->out.close();
    if (a->swr) swr_free(&a->swr);
    if (a->frame) av_frame_free(&a->frame);
    if (a->pcm_buf) av_freep(&a->pcm_buf);
    if (a->codec) avcodec_free_context(&a->codec);
    delete a;
}

}  // namespace

VideoView::VideoView() : Widget(0, 0, 320, 180) {
    texture_ = new Texture;
}

VideoView::~VideoView() {
    close_source();
    delete texture_;
}

void VideoView::close_source() {
    if (!ctx_ && !audio_ctx_) return;
    running_.store(false);
    frame_cv_.notify_all();
    if (decode_thread_.joinable()) {
        log_info("VideoView: joining decode thread...");
        decode_thread_.join();
        log_info("VideoView: decode thread joined");
    }
    decode_thread_ = std::thread();
    if (ctx_) {
        if (ctx_->sws) sws_freeContext(ctx_->sws);
        if (ctx_->frame) av_frame_free(&ctx_->frame);
        if (ctx_->pkt) av_packet_free(&ctx_->pkt);
        if (ctx_->codec) avcodec_free_context(&ctx_->codec);
        if (ctx_->fmt) avformat_close_input(&ctx_->fmt);
        delete ctx_;
        ctx_ = nullptr;
    }
    if (audio_ctx_) {
        audio_free_ctx(audio_ctx_);
        audio_ctx_ = nullptr;
    }
    has_frame_ = false;
}

bool VideoView::open_source(const char* path) {
    close_source();
    ctx_ = new VideoContext;

    if (avformat_open_input(&ctx_->fmt, path, nullptr, nullptr) < 0) {
        log_error("VideoView: cannot open %s", path ? path : "");
        close_source();
        return false;
    }
    if (avformat_find_stream_info(ctx_->fmt, nullptr) < 0) {
        log_error("VideoView: find_stream_info failed");
        close_source();
        return false;
    }

    const AVCodec* dec = nullptr;
    ctx_->stream = av_find_best_stream(ctx_->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ctx_->stream < 0 || !dec) {
        log_error("VideoView: no video stream in %s", path ? path : "");
        close_source();
        return false;
    }

    ctx_->codec = avcodec_alloc_context3(dec);
    if (!ctx_->codec) {
        log_error("VideoView: alloc codec context failed");
        close_source();
        return false;
    }
    avcodec_parameters_to_context(ctx_->codec, ctx_->fmt->streams[ctx_->stream]->codecpar);
    ctx_->codec->thread_count = 1;  // 多线程解码在精简版 DLL 上与 seek/flush 有死锁风险，固定单线程
    if (avcodec_open2(ctx_->codec, dec, nullptr) < 0) {
        log_error("VideoView: open codec failed");
        close_source();
        return false;
    }

    ctx_->width = ctx_->codec->width;
    ctx_->height = ctx_->codec->height;
    ctx_->frame = av_frame_alloc();
    ctx_->pkt = av_packet_alloc();

    const AVRational fr = ctx_->fmt->streams[ctx_->stream]->avg_frame_rate;
    if (fr.num > 0 && fr.den > 0) ctx_->fps = av_q2d(fr);
    if (ctx_->fps <= 0.0) ctx_->fps = 25.0;
    fps_ = ctx_->fps;

    if (ctx_->fmt->duration > 0)
        duration_ = (double)ctx_->fmt->duration / (double)AV_TIME_BASE;
    else {
        const AVStream* st = ctx_->fmt->streams[ctx_->stream];
        if (st->duration > 0 && st->time_base.den > 0)
            duration_ = (double)st->duration * av_q2d(st->time_base);
    }

    // 预创建纹理 + 各帧缓冲
    std::vector<unsigned char> zeros((size_t)ctx_->width * ctx_->height * 4, 0);
    texture_->create(ctx_->width, ctx_->height, zeros.data());
    decoded_rgba_.resize((size_t)ctx_->width * ctx_->height * 4);
    pending_frame_.clear();
    play_time_ = 0.f;
    pos_seconds_.store(0.0);
    seek_ms_.store(-1);
    seek_target_.store(-1.f);
    speed_.store(1.f);
    step_.store(false);
    playing_.store(true);
    loop_.store(true);
    eof_.store(false);

    running_.store(true);
    decode_thread_ = std::thread(&VideoView::decode_loop, this);

    // 音频初始化（M14 遗留修复）：失败仅静音，不影响视频
    audio_ctx_ = new AudioContext;
    if (!audio_init_ctx(audio_ctx_, ctx_->fmt)) {
        audio_free_ctx(audio_ctx_);
        audio_ctx_ = nullptr;
        log_info("VideoView: no audio stream, muted");
    }

    log_info("VideoView: opened %s (%dx%d @ %.2ffps, %.2fs)", path ? path : "", ctx_->width,
             ctx_->height, ctx_->fps, duration_);
    return true;
}

// 解出一帧写入 decoded_rgba_ 并更新 pos_seconds_；无帧可出返回 false（EOF 或出错）
bool VideoView::decode_one_frame() {
    for (;;) {
        int ret = avcodec_receive_frame(ctx_->codec, ctx_->frame);
        if (ret == 0) {
            if (!ctx_->sws || ctx_->sws_src != ctx_->frame->format) {
                if (ctx_->sws) sws_freeContext(ctx_->sws);
                ctx_->sws = sws_getContext(ctx_->width, ctx_->height,
                                           (AVPixelFormat)ctx_->frame->format, ctx_->width,
                                           ctx_->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr,
                                           nullptr, nullptr);
                ctx_->sws_src = (AVPixelFormat)ctx_->frame->format;
            }
            uint8_t* dst[4] = {decoded_rgba_.data(), nullptr, nullptr, nullptr};
            int dst_linesize[4] = {ctx_->width * 4, 0, 0, 0};
            sws_scale(ctx_->sws, ctx_->frame->data, ctx_->frame->linesize, 0, ctx_->height, dst,
                      dst_linesize);
            if (ctx_->frame->pts != AV_NOPTS_VALUE) {
                const AVRational tb = ctx_->fmt->streams[ctx_->stream]->time_base;
                pos_seconds_.store(av_q2d(tb) * (double)ctx_->frame->pts);
            } else {
                pos_seconds_.store(pos_seconds_.load() + 1.0 / ctx_->fps);
            }
            return true;
        }
        if (ret == AVERROR_EOF) {
            ctx_->eof = true;
            return false;
        }
        if (ret < 0 && ret != AVERROR(EAGAIN)) {
            ctx_->eof = true;
            return false;
        }
        int pr = av_read_frame(ctx_->fmt, ctx_->pkt);
        if (pr < 0) {
            // 文件读完：冲刷解码器以吐尽缓冲帧
            avcodec_send_packet(ctx_->codec, nullptr);
            av_packet_unref(ctx_->pkt);
            continue;
        }
        if (ctx_->pkt->stream_index == ctx_->stream) {
            avcodec_send_packet(ctx_->codec, ctx_->pkt);
        } else if (audio_ctx_ && ctx_->pkt->stream_index == audio_ctx_->stream)
            handle_audio_packet(ctx_->pkt);
        av_packet_unref(ctx_->pkt);
    }
}

// 解码一包音频：送解码器 → swr 重采样为 s16 立体声 → 应用音量 → 写 winmm 输出
void VideoView::handle_audio_packet(void* pkt_ptr) {
    AudioContext* a = audio_ctx_;
    if (!a || !a->codec || !a->swr) return;
    AVPacket* pkt = static_cast<AVPacket*>(pkt_ptr);
    avcodec_send_packet(a->codec, pkt);
    for (;;) {
        int ret = avcodec_receive_frame(a->codec, a->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) break;
        int out_n = swr_convert(a->swr, &a->pcm_buf, a->pcm_cap,
                                (const uint8_t**)a->frame->extended_data, a->frame->nb_samples);
        if (out_n <= 0) continue;
        const int total = out_n * a->out_channels;
        int16_t* s = (int16_t*)a->pcm_buf;
        const float g = volume_.load();
        if (g < 1.f) {
            for (int i = 0; i < total; ++i) s[i] = (int16_t)((float)s[i] * g);
        }
        const int chunk = 4096;
        int off = 0;
        while (off < total) {
            const int n = (total - off) > chunk ? chunk : (total - off);
            a->out.write(s + off, n);
            off += n;
        }
    }
}

void VideoView::decode_loop() {
    log_info("VideoView: decode thread start");
    // 解码节流：每帧按 1/(fps*speed) 推进，避免解码远快于播放时空转烧 CPU
    auto next_push = std::chrono::steady_clock::now();
    while (running_.load()) {
        // 暂停则等待（seek/step/play 会唤醒）
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
            const std::int64_t us = ms * 1000;  // stream=-1 时按 AV_TIME_BASE（微秒）
            av_seek_frame(ctx_->fmt, -1, us, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(ctx_->codec);
            if (audio_ctx_) {  // 音频解码器 + winmm 队列同步复位
                avcodec_flush_buffers(audio_ctx_->codec);
                audio_ctx_->out.flush();
            }
            ctx_->eof = false;
            eof_.store(false);
            next_push = std::chrono::steady_clock::now();  // seek 后立即刷新
            // 精确 seek：BACKWARD 定位到目标前的关键帧，前进解码到目标时刻
            int guard = 0;
            while (target >= 0.f && pos_seconds_.load() < (double)target - 0.02 &&
                   guard++ < 60) {
                if (!decode_one_frame()) break;
            }
            {
                std::lock_guard<std::mutex> lk(frame_mutex_);
                if (!decoded_rgba_.empty()) pending_frame_ = decoded_rgba_;
            }
            continue;
        }

        // 帧步进（M13）：暂停中精确解码下一帧供显示
        if (step_.exchange(false)) {
            if (decode_one_frame()) {
                std::lock_guard<std::mutex> lk(frame_mutex_);
                pending_frame_ = decoded_rgba_;
            }
            continue;
        }

        if (!decode_one_frame()) {
            if (ctx_->eof && loop_.load()) {
                av_seek_frame(ctx_->fmt, -1, 0, AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(ctx_->codec);
                if (audio_ctx_) {  // 音频解码器 + winmm 队列同步复位
                    avcodec_flush_buffers(audio_ctx_->codec);
                    audio_ctx_->out.flush();
                }
                ctx_->eof = false;
                eof_.store(false);
                next_push = std::chrono::steady_clock::now();
                continue;
            }
            if (ctx_->eof) {
                // 播完且不循环：置位并等待 play()/seek 重置
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
            pending_frame_ = decoded_rgba_;
        }
        const float speed = speed_.load();
        next_push += std::chrono::microseconds((std::int64_t)(1e6 / (fps_ * speed)));
        std::this_thread::sleep_until(next_push);
    }
    log_info("VideoView: decode thread exit");
}

void VideoView::play() {
    if (eof_.load()) seek(0.f);  // 播完后再播放：回 0 重新开始
    playing_.store(true);
    eof_.store(false);
    if (audio_ctx_) audio_ctx_->out.resume();
    frame_cv_.notify_all();
}

void VideoView::pause() {
    playing_.store(false);
    if (audio_ctx_) audio_ctx_->out.pause();
    frame_cv_.notify_all();
}

void VideoView::set_volume(float volume) {
    volume_.store(volume < 0.f ? 0.f : (volume > 1.f ? 1.f : volume));
}

void VideoView::set_playing(bool playing) {
    if (playing)
        play();
    else
        pause();
}

void VideoView::set_speed(float speed) {
    speed_.store(speed < 0.1f ? 0.1f : (speed > 4.f ? 4.f : speed));
    frame_cv_.notify_all();  // 唤醒解码线程应用新节奏
}

void VideoView::step_frame() {
    playing_.store(false);
    step_.store(true);
    frame_cv_.notify_all();
}

void VideoView::seek(float seconds) {
    if (seconds < 0.f) seconds = 0.f;
    if (duration_ > 0.0 && (double)seconds > duration_) seconds = (float)duration_;
    seek_ms_.store((std::int64_t)(seconds * 1000.f));
    seek_target_.store(seconds);
    play_time_ = seconds;
    frame_cv_.notify_all();
}

bool VideoView::advance(float dt) {
    if (!ctx_) return false;
    if (!playing_.load()) {
        // 暂停：仅上传解码线程最新帧（如 seek 后刷新显示）
        std::vector<unsigned char> frame;
        {
            std::lock_guard<std::mutex> lk(frame_mutex_);
            if (pending_frame_.empty()) return false;
            frame.swap(pending_frame_);
        }
        texture_->update(frame.data());
        has_frame_ = true;
        return true;
    }
    const float speed = speed_.load();
    if (speed <= 0.f) return false;
    play_time_ += dt;
    if (play_time_ < 1.0f / (float)(fps_ * speed)) return false;
    play_time_ -= 1.0f / (float)(fps_ * speed);
    std::vector<unsigned char> frame;
    {
        std::lock_guard<std::mutex> lk(frame_mutex_);
        if (pending_frame_.empty()) return false;
        frame.swap(pending_frame_);
    }
    texture_->update(frame.data());
    has_frame_ = true;
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
