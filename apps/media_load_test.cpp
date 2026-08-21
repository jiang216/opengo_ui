// 素材加载测试工具：遍历指定目录/文件，用 OpenCV / FFmpeg / AudioOut 逐一加载并报告。
// 用法：opengl_ui_media_load_test <路径> [--play <秒>]   （路径可为文件或目录）
//   --play <秒>：对音频流解码并经 winmm（AudioOut）真实播放该秒数后退出。
// 路径一律走 Win32 宽字符（FindFirstFileW/_wfopen/CreateFileW）：
//   1. 中文文件名可正常处理（cv::imread 的窄路径按 ANSI 代码页解析 UTF-8 会失败，
//      故图片用宽路径读字节 + imdecode）；
//   2. 含孤立代理项等畸形 UTF-16 的文件名（UTF-8 无法无损表示）也能打开。

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ui/core/log.h"
#include "ui/media/audio_out.h"

#include <opencv2/imgcodecs.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

static std::string utf8_from_wchar(const std::wstring& w) {
    if (w.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr,
                                      nullptr);
    std::string s((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

static bool ends_with_lower(const std::string& s, const std::vector<std::string>& exts) {
    const std::string e = lower(s);
    for (auto& x : exts) {
        if (e.size() >= x.size() && e.compare(e.size() - x.size(), x.size(), x) == 0) return true;
    }
    return false;
}

// Win32 宽字符递归遍历（std::filesystem 对含非法 UTF-8 的文件名会崩溃）
static void scan_dir(const std::wstring& dir, std::vector<std::wstring>& imgs,
                     std::vector<std::wstring>& avs) {
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (std::wcscmp(fd.cFileName, L".") != 0 && std::wcscmp(fd.cFileName, L"..") != 0)
                scan_dir(dir + L"\\" + fd.cFileName, imgs, avs);
            continue;
        }
        const std::wstring full = dir + L"\\" + fd.cFileName;
        const std::string utf8 = utf8_from_wchar(full);
        if (ends_with_lower(utf8, {".jpg", ".jpeg", ".png", ".bmp", ".webp", ".tif", ".tiff",
                                   ".hdr", ".exr", ".psd", ".gif"}))
            imgs.push_back(full);
        else if (ends_with_lower(utf8, {".mp4", ".mkv", ".avi", ".mov", ".webm", ".mp3", ".wav",
                                        ".flac", ".aac", ".m4a", ".ogg", ".wma"}))
            avs.push_back(full);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

// ---- 图片：宽路径读字节 -> imdecode（绕开 imread 窄路径的 ANSI 编码问题）----
static void test_image(const std::wstring& wpath, const std::string& utf8) {
    HANDLE h = CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        std::printf("[IMG] FAIL open  %s\n", utf8.c_str());
        return;
    }
    const DWORD sz = GetFileSize(h, nullptr);
    std::vector<unsigned char> buf(sz);
    DWORD rd = 0;
    ReadFile(h, buf.data(), sz, &rd, nullptr);
    CloseHandle(h);
    if (buf.empty()) {
        std::printf("[IMG] FAIL empty %s\n", utf8.c_str());
        return;
    }
    cv::Mat img = cv::imdecode(buf, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::printf("[IMG] FAIL decode %s\n", utf8.c_str());
        return;
    }
    std::printf("[IMG] OK %4dx%-4d ch=%d depth=%d  %s\n", img.cols, img.rows, img.channels(),
                img.depth(), utf8.c_str());
}

// ---- 音视频：自定义 AVIO（宽路径 _wfopen）----
struct IoBuf {
    std::FILE* f;
};

static int io_read(void* opaque, uint8_t* buf, int size) {
    return (int)std::fread(buf, 1, (size_t)size, static_cast<IoBuf*>(opaque)->f);
}

static int64_t io_seek(void* opaque, int64_t off, int whence) {
    IoBuf* io = static_cast<IoBuf*>(opaque);
    if (whence == AVSEEK_SIZE) {
        const long cur = std::ftell(io->f);
        std::fseek(io->f, 0, SEEK_END);
        const long end = std::ftell(io->f);
        std::fseek(io->f, cur, SEEK_SET);
        return end;
    }
    std::fseek(io->f, (long)off, (whence == SEEK_END) ? SEEK_END : SEEK_SET);
    return std::ftell(io->f);
}

struct MediaHandle {
    AVFormatContext* fmt = nullptr;
    AVIOContext* pb = nullptr;
    IoBuf* io = nullptr;
};

static MediaHandle open_media(const std::wstring& wpath) {
    MediaHandle mh;
    std::FILE* f = _wfopen(wpath.c_str(), L"rb");
    if (!f) return mh;
    mh.io = new IoBuf{f};
    unsigned char* iobuf = (unsigned char*)av_malloc(32768);
    mh.pb = avio_alloc_context(iobuf, 32768, 0, mh.io, io_read, nullptr, io_seek);
    mh.fmt = avformat_alloc_context();
    mh.fmt->pb = mh.pb;
    if (avformat_open_input(&mh.fmt, nullptr, nullptr, nullptr) < 0) {
        avformat_free_context(mh.fmt);
        avio_context_free(&mh.pb);
        std::fclose(f);
        delete mh.io;
        mh.fmt = nullptr;
        mh.pb = nullptr;
        mh.io = nullptr;
    }
    return mh;
}

static void close_media(MediaHandle& mh) {
    if (mh.fmt) avformat_close_input(&mh.fmt);
    if (mh.pb) avio_context_free(&mh.pb);
    if (mh.io) {
        std::fclose(mh.io->f);
        delete mh.io;
    }
}

// ---- 音频：解码 s16 立体声并送入 AudioOut 播放 sec 秒 ----
static void play_audio(AVFormatContext* fmt, int stream, double sec) {
    const AVCodec* dec = avcodec_find_decoder(fmt->streams[stream]->codecpar->codec_id);
    if (!dec) return;
    AVCodecContext* c = avcodec_alloc_context3(dec);
    if (!c) return;
    if (avcodec_parameters_to_context(c, fmt->streams[stream]->codecpar) < 0) {
        avcodec_free_context(&c);
        return;
    }
    if (avcodec_open2(c, dec, nullptr) < 0) {
        avcodec_free_context(&c);
        return;
    }
    const int in_ch = c->ch_layout.nb_channels > 0 ? c->ch_layout.nb_channels : 2;
    AVChannelLayout src, dst;
    av_channel_layout_default(&src, in_ch);
    av_channel_layout_default(&dst, 2);
    SwrContext* swr = nullptr;
    if (swr_alloc_set_opts2(&swr, &dst, AV_SAMPLE_FMT_S16, c->sample_rate, &src, c->sample_fmt,
                            c->sample_rate, 0, nullptr) < 0 ||
        swr_init(swr) < 0) {
        av_channel_layout_uninit(&src);
        av_channel_layout_uninit(&dst);
        avcodec_free_context(&c);
        return;
    }
    av_channel_layout_uninit(&src);
    av_channel_layout_uninit(&dst);

    AudioOut out;
    if (!out.open(c->sample_rate, 2)) {
        std::printf("    [audio] waveOut open failed, muted test\n");
        out.close();
        swr_free(&swr);
        avcodec_free_context(&c);
        return;
    }
    const long want = (long)(sec * c->sample_rate * 2);  // 立体声样本数
    long got = 0;
    AVPacket* pkt = av_packet_alloc();
    AVFrame* fr = av_frame_alloc();
    uint8_t* pcm = nullptr;
    int pcm_cap = 8192, ls = 0;
    av_samples_alloc(&pcm, &ls, 2, pcm_cap, AV_SAMPLE_FMT_S16, 0);
    av_seek_frame(fmt, -1, 0, AVSEEK_FLAG_BACKWARD);  // 探测阶段已读走包，回 0 重放
    for (;;) {
        if (av_read_frame(fmt, pkt) < 0) break;
        if (pkt->stream_index == stream) {
            avcodec_send_packet(c, pkt);
            for (;;) {
                int ret = avcodec_receive_frame(c, fr);
                if (ret != 0) break;
                int n = swr_convert(swr, &pcm, pcm_cap, (const uint8_t**)fr->extended_data,
                                    fr->nb_samples);
                if (n > 0) {
                    out.write((const std::int16_t*)pcm, n * 2);
                    got += n * 2;
                }
                if (got >= want) break;
            }
        }
        av_packet_unref(pkt);
        if (got >= want) break;
    }
    std::printf("    [audio] played %.2fs/%.2fs of %s\n", (double)got / (2.0 * c->sample_rate),
                sec, avcodec_get_name(fmt->streams[stream]->codecpar->codec_id));
    out.flush();
    out.close();
    av_freep(&pcm);
    av_frame_free(&fr);
    av_packet_free(&pkt);
    swr_free(&swr);
    avcodec_free_context(&c);
}

// ---- 音视频：探测流 + 视频解前若干帧 + 可选音频播放 ----
static void test_media(const std::wstring& wpath, const std::string& utf8, double play_sec) {
    MediaHandle mh = open_media(wpath);
    if (!mh.fmt) {
        std::printf("[AV]  FAIL open %s\n", utf8.c_str());
        return;
    }
    AVFormatContext* fmt = mh.fmt;
    avformat_find_stream_info(fmt, nullptr);
    int vs = -1, as = -1;
    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        AVCodecParameters* p = fmt->streams[i]->codecpar;
        if (p->codec_type == AVMEDIA_TYPE_VIDEO && vs < 0) vs = (int)i;
        if (p->codec_type == AVMEDIA_TYPE_AUDIO && as < 0) as = (int)i;
    }
    double dur = fmt->duration > 0 ? (double)fmt->duration / AV_TIME_BASE : 0.0;
    std::printf("[AV]  OK streams=%d duration=%.2fs  %s\n", (int)fmt->nb_streams, dur,
                utf8.c_str());
    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        AVCodecParameters* p = fmt->streams[i]->codecpar;
        if (p->codec_type == AVMEDIA_TYPE_VIDEO)
            std::printf("    [v] %s %dx%d fps?=%.1f\n", avcodec_get_name(p->codec_id), p->width,
                        p->height, av_q2d(fmt->streams[i]->avg_frame_rate));
        else if (p->codec_type == AVMEDIA_TYPE_AUDIO)
            std::printf("    [a] %s %dHz %dch %d-bit\n", avcodec_get_name(p->codec_id),
                        p->sample_rate, p->ch_layout.nb_channels, p->bits_per_coded_sample);
        else
            std::printf("    [o] %s\n", avcodec_get_name(p->codec_id));
    }

    // 视频流：解码前 12 帧验证解码链路
    if (vs >= 0) {
        const AVCodec* dec = avcodec_find_decoder(fmt->streams[vs]->codecpar->codec_id);
        AVCodecContext* c = avcodec_alloc_context3(dec);
        if (c && avcodec_parameters_to_context(c, fmt->streams[vs]->codecpar) >= 0 &&
            avcodec_open2(c, dec, nullptr) >= 0) {
            int n = 0;
            AVPacket* pkt = av_packet_alloc();
            AVFrame* fr = av_frame_alloc();
            for (unsigned guard = 0; n < 12 && guard < fmt->nb_streams * 8; ++guard) {
                if (av_read_frame(fmt, pkt) < 0) break;
                if (pkt->stream_index == vs && avcodec_send_packet(c, pkt) >= 0) {
                    while (n < 12 && avcodec_receive_frame(c, fr) == 0) ++n;
                }
                av_packet_unref(pkt);
            }
            std::printf("    [v] decoded_frames=%d\n", n);
            av_frame_free(&fr);
            av_packet_free(&pkt);
            avcodec_free_context(&c);
        }
    }
    if (as >= 0 && play_sec > 0) play_audio(fmt, as, play_sec);
    close_media(mh);
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        std::printf("usage: opengl_ui_media_load_test <path> [--play <sec>]\n");
        return 2;
    }
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    double play_sec = 0.0;
    std::wstring target = argv[1];
    for (int i = 2; i < argc; ++i) {
        if (std::wcscmp(argv[i], L"--play") == 0 && i + 1 < argc)
            play_sec = _wtof(argv[++i]);
    }
    const std::string utf8_target = utf8_from_wchar(target);

    std::vector<std::wstring> imgs, avs;
    const DWORD attr = GetFileAttributesW(target.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        scan_dir(target, imgs, avs);
        std::sort(imgs.begin(), imgs.end());
        std::sort(avs.begin(), avs.end());
    } else if (attr != INVALID_FILE_ATTRIBUTES) {
        (ends_with_lower(utf8_target, {".mp4", ".mkv", ".avi", ".mov", ".webm", ".mp3", ".wav",
                                       ".flac", ".aac", ".m4a", ".ogg", ".wma"})
             ? avs
             : imgs)
            .push_back(target);
    } else {
        std::printf("path not found: %s\n", utf8_target.c_str());
        return 2;
    }

    std::printf("---- images (%zu) ----\n", imgs.size());
    for (auto& w : imgs) test_image(w, utf8_from_wchar(w));
    std::printf("---- media (%zu) ----\n", avs.size());
    for (auto& w : avs) test_media(w, utf8_from_wchar(w), play_sec);
    std::printf("done: images=%zu media=%zu play_sec=%.0f\n", imgs.size(), avs.size(), play_sec);
    return 0;
}
