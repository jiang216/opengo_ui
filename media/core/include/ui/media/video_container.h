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

namespace ui {
class Font;
class Renderer;
class Texture;
struct Theme;
class Button;
class Label;
class Slider;
class Switch;

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

    // 合成源自动启用标志（已 open 合成源）
    bool synthetic() const { return synthetic_; }

    void on_advance(float dt) override;
    void on_draw(Renderer& renderer) override;
    void on_layout() override;

private:
    void ensure_controls();
    void layout_controls();
    void refresh_controls();

    media::MediaPipeline pipe_;
    Texture* texture_ = nullptr;
    bool has_frame_ = false;
    bool synthetic_ = false;
    std::string last_error_;

    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::string title_;

    Button* play_btn_ = nullptr;
    Label* time_label_ = nullptr;
    Slider* seek_slider_ = nullptr;
    Switch* loop_switch_ = nullptr;
    Slider* vol_slider_ = nullptr;
    Slider* speed_slider_ = nullptr;
};

}  // namespace ui