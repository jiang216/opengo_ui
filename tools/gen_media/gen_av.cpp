// 生成带音频的测试视频（M14 遗留修复：音频支持验证素材）。
// 用法: opengl_ui_gen_av <out.mp4>   （默认 test_av.mp4）
// 内容: 320x180 h264(yuv420p) 25fps 2s 渐变图案 + aac 44100 立体声 440Hz 正弦波
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

static int add_video_stream(AVFormatContext* oc, AVCodecContext** cc, AVStream** st, int w, int h,
                            int fps) {
    // 优先 libx265（用户指定 h265 编码）；回退 libx264 / mpeg4
    const AVCodec* codec = avcodec_find_encoder_by_name("libx265");
    if (!codec) codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) codec = avcodec_find_encoder_by_name("mpeg4");
    if (!codec) return -1;
    *st = avformat_new_stream(oc, codec);
    *cc = avcodec_alloc_context3(codec);
    (*cc)->width = w;
    (*cc)->height = h;
    (*cc)->time_base = AVRational{1, fps};
    (*st)->time_base = (*cc)->time_base;
    (*cc)->framerate = AVRational{fps, 1};
    (*cc)->pix_fmt = AV_PIX_FMT_YUV420P;
    (*cc)->gop_size = fps;
    (*cc)->max_b_frames = 0;
    av_opt_set((*cc)->priv_data, "preset", "ultrafast", 0);
    if (avcodec_open2(*cc, codec, nullptr) < 0) return -1;
    avcodec_parameters_from_context((*st)->codecpar, *cc);
    return 0;
}

static int add_audio_stream(AVFormatContext* oc, AVCodecContext** cc, AVStream** st, int rate) {
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) return -1;
    *st = avformat_new_stream(oc, codec);
    *cc = avcodec_alloc_context3(codec);
    (*cc)->sample_fmt = AV_SAMPLE_FMT_FLTP;
    (*cc)->sample_rate = rate;
    av_channel_layout_default(&(*cc)->ch_layout, 2);
    (*cc)->time_base = AVRational{1, rate};
    (*st)->time_base = (*cc)->time_base;
    (*cc)->bit_rate = 96000;
    if (avcodec_open2(*cc, codec, nullptr) < 0) return -1;
    avcodec_parameters_from_context((*st)->codecpar, *cc);
    return 0;
}

int main(int argc, char** argv) {
    const char* out = argc > 1 ? argv[1] : "test_av.mp4";
    const int w = 320, h = 180, fps = 25, rate = 44100;
    const int total_video = fps * 2;            // 2s
    const int frame_n = 1024;                   // AAC frame
    const int n_audio = (int)ceil(2.0 * rate / frame_n);

    AVFormatContext* oc = nullptr;
    if (avformat_alloc_output_context2(&oc, nullptr, "mp4", out) < 0) {
        std::fprintf(stderr, "gen_av: alloc output ctx failed\n");
        return 1;
    }
    AVCodecContext *vc = nullptr, *ac = nullptr;
    AVStream *vs = nullptr, *as = nullptr;
    if (add_video_stream(oc, &vc, &vs, w, h, fps) < 0 ||
        add_audio_stream(oc, &ac, &as, rate) < 0) {
        std::fprintf(stderr, "gen_av: encoder unavailable (need libx264 + aac)\n");
        return 1;
    }
    if (avio_open(&oc->pb, out, AVIO_FLAG_WRITE) < 0) {
        std::fprintf(stderr, "gen_av: cannot open %s\n", out);
        return 1;
    }
    std::fprintf(stderr, "gen_av: streams v=%d a=%d, enc=%s\n", (int)vs->index, (int)as->index,
                 avcodec_get_name(vc->codec_id));
    if (avformat_write_header(oc, nullptr) < 0) return 1;
    std::fprintf(stderr, "gen_av: header ok\n");

    SwsContext* sws = sws_getContext(w, h, AV_PIX_FMT_RGBA, w, h, AV_PIX_FMT_YUV420P,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
    AVFrame* rgb = av_frame_alloc();
    rgb->format = AV_PIX_FMT_RGBA;
    rgb->width = w;
    rgb->height = h;
    av_frame_get_buffer(rgb, 32);
    AVFrame* yuv = av_frame_alloc();
    yuv->format = AV_PIX_FMT_YUV420P;
    yuv->width = w;
    yuv->height = h;
    av_frame_get_buffer(yuv, 32);

    std::vector<AVPacket*> pkts;

    auto drain = [&](AVCodecContext* cc, AVRational from, AVRational to, int sidx) {
        AVPacket* pkt = av_packet_alloc();
        while (avcodec_receive_packet(cc, pkt) == 0) {
            pkt->stream_index = sidx;
            av_packet_rescale_ts(pkt, from, to);
            pkts.push_back(pkt);
            pkt = av_packet_alloc();
        }
        av_packet_free(&pkt);
    };

    for (int f = 0; f < total_video; ++f) {
        const double t = (double)f / fps;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                uint8_t* p = rgb->data[0] + (size_t)y * rgb->linesize[0] + (size_t)x * 4;
                p[0] = (uint8_t)(255 * (0.5 + 0.5 * std::sin(2 * 3.14159 * t + x * 0.05)));
                p[1] = (uint8_t)(255 * (0.5 + 0.5 * std::sin(2 * 3.14159 * t * 1.3 + y * 0.05)));
                p[2] = (uint8_t)(128 + 80 * std::sin(2 * 3.14159 * t * 2.0 + (x + y) * 0.03));
                p[3] = 255;
            }
        }
        rgb->pts = f;
        sws_scale(sws, rgb->data, rgb->linesize, 0, h, yuv->data, yuv->linesize);
        yuv->pts = f;
        avcodec_send_frame(vc, yuv);
        drain(vc, vc->time_base, vs->time_base, (int)vs->index);
    }
    avcodec_send_frame(vc, nullptr);  // flush video
    drain(vc, vc->time_base, vs->time_base, (int)vs->index);
    std::fprintf(stderr, "gen_av: video done (%zu pkts)\n", pkts.size());

    AVFrame* af = av_frame_alloc();
    af->format = AV_SAMPLE_FMT_FLTP;
    af->sample_rate = rate;
    af->nb_samples = frame_n;
    av_channel_layout_copy(&af->ch_layout, &ac->ch_layout);
    av_frame_get_buffer(af, 32);
    for (int i = 0; i < n_audio; ++i) {
        if (i % 10 == 0) std::fprintf(stderr, "gen_av: audio frame %d/%d\n", i, n_audio);
        const double t0 = (double)(i * frame_n) / rate;
        for (int s = 0; s < frame_n; ++s) {
            const double t = t0 + (double)s / rate;
            const float v = 0.3f * (float)std::sin(2 * 3.14159 * 440.0 * t);
            ((float*)af->data[0])[s] = v;
            ((float*)af->data[1])[s] = v;
        }
        af->nb_samples = frame_n;
        af->pts = (int64_t)(i * frame_n);
        const int sr = avcodec_send_frame(ac, af);
        if (sr < 0) std::fprintf(stderr, "gen_av: audio send %d -> %d\n", i, sr);
        drain(ac, ac->time_base, as->time_base, (int)as->index);
    }
    avcodec_send_frame(ac, nullptr);  // flush audio
    drain(ac, ac->time_base, as->time_base, (int)as->index);
    std::fprintf(stderr, "gen_av: audio done (%zu pkts total)\n", pkts.size());

    // 按 dts 排序后交错写入（音视频统一比较时间）
    const AVRational ms_tb = {1, 1000};
    std::sort(pkts.begin(), pkts.end(), [&](const AVPacket* a, const AVPacket* b) {
        const AVRational& ta = a->stream_index == (int)vs->index ? vs->time_base : as->time_base;
        const AVRational& tb = b->stream_index == (int)vs->index ? vs->time_base : as->time_base;
        return av_rescale_q(a->dts, ta, ms_tb) < av_rescale_q(b->dts, tb, ms_tb);
    });
    for (AVPacket* p : pkts) av_interleaved_write_frame(oc, p);
    for (AVPacket* p : pkts) av_packet_free(&p);

    av_write_trailer(oc);
    av_frame_free(&rgb);
    av_frame_free(&yuv);
    av_frame_free(&af);
    sws_freeContext(sws);
    avcodec_free_context(&vc);
    avcodec_free_context(&ac);
    avformat_free_context(oc);
    std::printf("gen_av: wrote %s (%d video pkts, %d audio pkts)\n", out, total_video,
                n_audio);
    return 0;
}
