#pragma once

// 视频容器控件（UI-193）：边框 + 标题条 + 底部控制条 + 状态文案，内部托管媒体管线。
// 数据：MediaPipeline（Source/Decoder/AudioSink 可换节点）+ 每实例自持 Texture（UI-146
// 每个窗口独立 GL 上下文，纹理归属所属上下文，主线程 advance/on_draw 才触碰 GL）。
// 用法：
//   VideoContainer c(0, 0, 560, 340);
//   c.set_font(&font); c.set_theme(&theme);
//   c.open_source("synthetic:4");      // 合成源（纯 UI 工程即可演示）
//   c.open_source("assets/test.mp4");  // 真实文件（需 media/ffmpeg 提供 ffmpeg 节点）
//   ->play() / pause() / seek() / set_loop() / set_speed() / set_volume()
// 每实例独立解码线程与纹理，同窗口可放多个容器。

#include "ui/core/math.h"
#include "ui/ui/widget.h"

#include "ui/media/pipeline/pipeline.h"

#include <algorithm>
#include <chrono>

namespace ui {
class Font;
class Renderer;
class Texture;
struct Theme;
class Button;
class Label;
class Slider;
class Switch;
class IconLabel;

// 画面亮度范围（UI-194）：0.2（明显调暗）.. 3.0（增亮至 3 倍），默认 1.0 = 原样
constexpr float kVideoContainerBrightMin = 0.2f;
constexpr float kVideoContainerBrightMax = 3.0f;
// 倍速箭头步进（UI-194）：▲/▼ 每点一次 ±kVideoContainerSpeedStep，夹在 0.5..2x
constexpr float kVideoContainerSpeedStep = 0.25f;

class VideoContainer : public Widget {
public:
    VideoContainer(float x, float y, float w, float h);
    ~VideoContainer() override;

    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_title(const char* title);
    const std::string& title() const { return title_; }

    // 打开描述串：文件路径或 "synthetic[:秒]"。失败返回 false（进入错误文案态，可重复 open）。
    bool open_source(const char* descriptor);
    void close_source();
    bool has_video() const;

    // ---- 播放控制（同 MediaPipeline / VideoView）----
    void play();
    void pause();
    void set_playing(bool playing);
    bool playing() const;
    void set_loop(bool loop);
    bool loop() const;
    void set_speed(float speed);
    float speed() const;
    void step_frame();
    void seek(float seconds);
    double duration() const;
    float position() const;
    void set_volume(float volume);
    float volume() const;
    bool has_audio() const;
    // 画面亮度（UI-194）：显示层效果，只调视频画面（RGB 乘亮度系数），范围 [0.2, 3.0]，默认 1.0
    void set_brightness(float brightness);
    float brightness() const { return std::clamp(brightness_, kVideoContainerBrightMin, kVideoContainerBrightMax); }

    // 合成源自动启用标志（已 open 合成源）
    bool synthetic() const { return synthetic_; }

    void on_advance(float dt) override;
    void on_draw(Renderer& renderer) override;
    void on_layout() override;

private:
    void ensure_controls();
    void layout_controls();
    void refresh_controls();
    void update_controls_visibility(bool on);  // UI-197：显隐底部控制条 + 左右音量/亮度竖滑块

    bool on_mouse_down(float x, float y, int button) override;  // UI-197：双击播放/暂停（右键 UI-198）
    bool on_mouse_move(float x, float y) override;              // UI-197：悬停显示 / 移开隐藏控制条

    media::MediaPipeline pipe_;
    Texture* texture_ = nullptr;
    bool has_frame_ = false;
    bool synthetic_ = false;
    std::string last_error_;
    float brightness_ = 1.0f;   // 画面亮度系数（UI-194），范围 [0.2, 3.0]

    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::string title_;

    Button* play_btn_ = nullptr;
    Label* time_label_ = nullptr;
    Slider* seek_slider_ = nullptr;
    Switch* loop_switch_ = nullptr;
    Slider* vol_slider_ = nullptr;      // 竖向音量滑块（UI-194）
    IconLabel* vol_label_ = nullptr;    // 音量标签（sound 图标 + 中文，UI-194）
    Label* vol_val_label_ = nullptr;    // 音量常显数值（"100%"，UI-196）
    Slider* bright_slider_ = nullptr;   // 竖向亮度滑块（UI-194）
    IconLabel* bright_label_ = nullptr; // 亮度标签（light 图标 + 中文，UI-194）
    Label* bright_val_label_ = nullptr; // 亮度常显数值（"1.0"，UI-196）

    float pending_seek_norm_ = -1.f;    // 用户拖 seek 的目标（归一化 0..1）；管线 position 未追上显示它（UI-196），已追上/-1 则回 position
    Button* speed_down_btn_ = nullptr;  // 倍速 ▼（UI-194）
    Label* speed_label_ = nullptr;      // 倍速值显示（UI-194）
    Button* speed_up_btn_ = nullptr;    // 倍速 ▲（UI-194）

    // UI-197：控制条悬停显隐——默认隐藏，鼠标在容器内才显示；播放源打开后若从未悬停也隐藏
    bool controls_visible_ = false;
    // UI-197：双击判定——左键时间窗 + 位置容差（双击切换播放/暂停）
    std::chrono::steady_clock::time_point last_left_down_{};
    float last_left_x_ = -1e9f;
    float last_left_y_ = -1e9f;
};

}  // namespace ui