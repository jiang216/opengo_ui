#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

struct AudioOutBlock;  // 完整定义在 cpp（winmm 结构不暴露给头文件）

// Windows winmm（系统库，无第三方依赖）16bit PCM 音频输出（M13 音量 / M14 遗留修复）。
// 由解码线程 write() 推 PCM，waveOut 回调线程负责播放完的块回收与续写。
// 仅支持 1/2 声道、16bit 交错 PCM。
class AudioOut {
public:
    AudioOut();
    ~AudioOut();

    AudioOut(const AudioOut&) = delete;
    AudioOut& operator=(const AudioOut&) = delete;

    // 打开默认输出设备；失败返回 false（调用方应视为静音）
    bool open(int sample_rate, int channels);
    void close();
    bool is_open() const { return opened_; }

    // 追加一段 16bit 交错 PCM；内部复制，异步播放
    void write(const std::int16_t* samples, int count);
    // 暂停 / 恢复 / 清空（seek 时调用 flush）
    void pause();
    void resume();
    void flush();

private:
    // waveOut 设备回调：播放完一块后把块从 in_flight_ 移到 retired_（回调线程内不调任何 winmm API，
    // 否则与写线程 waveOutWrite 并发会卡死 VoiceMeeter/winmm 驱动；unprepare 由写线程 pump 统一处理）。
    // 参数用 size_t/void*（与 DWORD_PTR/UINT_PTR 同宽）声明：若用 unsigned long long，
    // 在 x86（DWORD_PTR=4 字节）下栈上参数宽度不匹配，__stdcall 返回地址弹栈错位会破坏调用方栈
    // （回调被 winmm 触发即 0xC0000005 崩溃，UI-044 根因）。
    static void __stdcall WaveCallback(void* hwo, unsigned int msg, size_t user,
                                       size_t p1, size_t p2);
    // 泵送：锁内 unprepare retired_、摘取待播块并 prepare，锁外 waveOutWrite（设备忙时
    // waveOutWrite 会阻塞等回调，回调需持锁回收 -> 绝不能持锁调用写，否则死锁）；
    // 失败块移回 pending 队首重试
    void pump();

    bool opened_ = false;
    void* hwo_ = nullptr;                             // HWAVEOUT
    std::mutex mu_;
    std::deque<std::unique_ptr<AudioOutBlock>> pending_;    // 待播放
    std::deque<std::unique_ptr<AudioOutBlock>> in_flight_;  // 播放中（设备持有）
    std::deque<std::unique_ptr<AudioOutBlock>> retired_;    // 播放完待 unprepare（写线程统一回收）
};
