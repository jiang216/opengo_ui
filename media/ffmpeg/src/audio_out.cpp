#include "ui/media/audio_out.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <algorithm>

#pragma comment(lib, "winmm.lib")

namespace {

constexpr DWORD kBlockSamples = 4096;  // 每块样本数（16bit，块 ≈ 46ms@44.1k 双声道）
constexpr int kMaxInFlight = 6;        // 设备同时持有的块上限（≈ 0.3s 缓冲）

}  // namespace

// 待播放数据块（winmm 结构仅在此可见，头文件不暴露）
struct AudioOutBlock {
    WAVEHDR hdr = {};
    std::vector<std::int16_t> data;  // 待播放样本（count 个）
    int count = 0;                   // 有效样本数
};

void AudioOut::WaveCallback(void* /*hwo*/, unsigned int msg, size_t user,
                            size_t p1, size_t /*p2*/) {
    if (msg != WOM_DONE || !user) return;
    AudioOut* self = reinterpret_cast<AudioOut*>(user);
    WAVEHDR* hdr = reinterpret_cast<WAVEHDR*>(p1);
    AudioOutBlock* blk = reinterpret_cast<AudioOutBlock*>(hdr->dwUser);
    // 回调线程内绝不调用 winmm API（unprepare/write 与写线程并发会卡死驱动），
    // 只把块移入 retired_，由写线程 pump() 统一 unprepare。
    std::lock_guard<std::mutex> lk(self->mu_);
    auto& inflight = self->in_flight_;
    for (auto it = inflight.begin(); it != inflight.end(); ++it) {
        if ((*it).get() == blk) {
            self->retired_.push_back(std::move(*it));
            inflight.erase(it);
            break;
        }
    }
}

AudioOut::AudioOut() = default;

AudioOut::~AudioOut() { close(); }

bool AudioOut::open(int sample_rate, int channels) {
    close();
    if (sample_rate <= 0 || channels < 1 || channels > 2) return false;

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)channels;
    wfx.nSamplesPerSec = sample_rate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (WORD)(channels * 2);
    wfx.nAvgBytesPerSec = sample_rate * wfx.nBlockAlign;

    HWAVEOUT hwo = nullptr;
    if (waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR)&AudioOut::WaveCallback,
                    (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
        return false;
    hwo_ = hwo;
    opened_ = true;
    return true;
}

void AudioOut::close() {
    if (!opened_) return;
    HWAVEOUT hwo = static_cast<HWAVEOUT>(hwo_);
    // reset 会同步等待设备回调完成回收（回调把块移入 retired_，不调 winmm），
    // 回调需持锁，故不能持锁调用 reset（否则死锁）
    waveOutReset(hwo);
    {
        std::lock_guard<std::mutex> lk(mu_);
        for (auto& b : in_flight_) waveOutUnprepareHeader(hwo, &b->hdr, sizeof(WAVEHDR));
        in_flight_.clear();
        for (auto& b : retired_) waveOutUnprepareHeader(hwo, &b->hdr, sizeof(WAVEHDR));
        retired_.clear();
        pending_.clear();
    }
    waveOutClose(hwo);
    hwo_ = nullptr;
    opened_ = false;
}

void AudioOut::write(const std::int16_t* samples, int count) {
    if (!opened_ || !samples || count <= 0) return;
    std::unique_ptr<AudioOutBlock> blk(new AudioOutBlock);
    blk->data.assign(samples, samples + count);
    blk->count = count;
    {
        std::lock_guard<std::mutex> lk(mu_);
        pending_.push_back(std::move(blk));
    }
    pump();
}

void AudioOut::pause() {
    if (!opened_) return;
    waveOutPause(static_cast<HWAVEOUT>(hwo_));
}

void AudioOut::resume() {
    if (!opened_) return;
    waveOutRestart(static_cast<HWAVEOUT>(hwo_));
}

void AudioOut::flush() {
    if (!opened_) return;
    HWAVEOUT hwo = static_cast<HWAVEOUT>(hwo_);
    // 同 close：先解锁 reset（回调需持锁），再清空回收
    waveOutReset(hwo);
    std::lock_guard<std::mutex> lk(mu_);
    for (auto& b : in_flight_) waveOutUnprepareHeader(hwo, &b->hdr, sizeof(WAVEHDR));
    in_flight_.clear();
    for (auto& b : retired_) waveOutUnprepareHeader(hwo, &b->hdr, sizeof(WAVEHDR));
    retired_.clear();
    pending_.clear();
}

void AudioOut::pump() {
    if (!opened_) return;
    HWAVEOUT hwo = static_cast<HWAVEOUT>(hwo_);
    std::vector<AudioOutBlock*> to_write;
    {
        std::lock_guard<std::mutex> lk(mu_);
        // 回收上一批播放完的块（回调只搬运，unprepare 必须在写线程做）
        if (!retired_.empty()) {
            for (auto& b : retired_) waveOutUnprepareHeader(hwo, &b->hdr, sizeof(WAVEHDR));
            retired_.clear();
        }
        while (!pending_.empty() && (int)in_flight_.size() < kMaxInFlight) {
            std::unique_ptr<AudioOutBlock> blk = std::move(pending_.front());
            pending_.pop_front();
            blk->hdr.lpData = reinterpret_cast<LPSTR>(blk->data.data());
            blk->hdr.dwBufferLength = (DWORD)(blk->count * sizeof(std::int16_t));
            blk->hdr.dwFlags = 0;
            blk->hdr.dwUser = (DWORD_PTR)blk.get();
            if (waveOutPrepareHeader(hwo, &blk->hdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
                pending_.push_front(std::move(blk));
                break;
            }
            to_write.push_back(blk.get());
            in_flight_.push_back(std::move(blk));
        }
    }
    // waveOutWrite 可能在设备忙时阻塞等待回调腾空间，回调需持锁回收，
    // 因此写调用必须在锁外进行（否则回调锁死 -> 设备永不腾位 -> 死锁）。
    for (AudioOutBlock* b : to_write) {
        if (waveOutWrite(hwo, &b->hdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            // 提交失败（如设备忙）：该块及后续未提交块移回 pending 队首，下次 pump 重试
            std::lock_guard<std::mutex> lk(mu_);
            while (!in_flight_.empty() && in_flight_.back().get() != b) {
                auto blk = std::move(in_flight_.back());
                in_flight_.pop_back();
                pending_.push_front(std::move(blk));
            }
            if (!in_flight_.empty()) {
                auto blk = std::move(in_flight_.back());
                in_flight_.pop_back();
                pending_.push_front(std::move(blk));
            }
            break;
        }
    }
}
