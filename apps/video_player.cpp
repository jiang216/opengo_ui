// 视频播放器示例应用（M16 收官）：打开视频 / 播放控制 / 进度 / 音量 / 倍速。
// 用法：opengl_ui_video_player [视频路径]
//  - 打开：按钮或 Ctrl+O（Win32 文件对话框，宽字符路径）
//  - 播放/暂停、进度条 seek、循环开关、倍速 0.5x..2x、音量 0..1
//  - --smoke <秒>：运行指定秒后自动退出（CI/冒烟）

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include "ui/core/log.h"
#include "ui/core/version.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/media/video_view.h"
#include "ui/ui/button.h"
#include "ui/ui/checkbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scene.h"
#include "ui/ui/slider.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

std::string utf8_from_wchar(const std::wstring& w) {
    if (w.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

bool open_file_dialog(std::wstring& out) {
    wchar_t buf[MAX_PATH] = {0};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Videos\0*.mp4;*.mkv;*.avi;*.mov;*.webm;*.flv\0All\0*.*\0";
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return false;
    out = buf;
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    double smoke_seconds = 0.0;
    std::string file_arg;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc) smoke_seconds = std::atof(argv[++i]);
        else if (argv[i][0] != '-' && file_arg.empty()) file_arg = argv[i];
    }

    ui::log_info("opengl_ui %s video_player", ui::version_string());

    ui::Window window(960, 700, "video player (M16)");
    if (!window.handle()) return 1;
    ui::Renderer renderer;
    if (!renderer.init()) return 1;
    renderer.set_viewport(window.width(), window.height());

    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    font.load_cjk_font_auto();

    // ui::Scene：统一驱动生命周期；video_player 迁移前无标准键分发（仅 Space 自定义 + Ctrl+O）
    ui::Scene scene(window, renderer);
    scene.set_root_background(ui::Vec4{0.16f, 0.17f, 0.19f, 1.f});
    scene.dispatch_nav_keys = false;
    scene.dispatch_text = false;

    ui::Widget& root = scene.root();

    // 顶部工具条
    ui::Button open_btn(8, 8, 90, 30, "Open...");
    open_btn.set_font(&font);
    root.add_child(&open_btn);
    ui::Label status(110, 10, 840, 24, "Ctrl+O: open | play/pause | seek | volume | speed");
    status.set_font(&font);
    root.add_child(&status);

    ui::VideoView video;
    video.set_bounds(0, 46, (float)window.width(), (float)window.height() - 46.f - 150.f);
    root.add_child(&video);

    // 控制条：播放切换 / 循环 / 进度 / 时间 / 倍速 / 音量
    ui::Button toggle(8, 0, 90, 30, "Pause");
    toggle.set_font(&font);
    toggle.set_bounds(8, (float)window.height() - 140, 90, 30);
    root.add_child(&toggle);

    ui::CheckBox loop_cb(110, 0, 140, 24, "loop playback");
    loop_cb.set_font(&font);
    loop_cb.set_bounds(110, (float)window.height() - 136, 140, 24);
    loop_cb.set_checked(true);
    root.add_child(&loop_cb);

    ui::Slider seek(8, 0, 944, 24);
    seek.set_bounds(8, (float)window.height() - 108, 944, 24);
    seek.set_range(0.f, 1.f);
    root.add_child(&seek);

    ui::Label time_lb(8, 0, 300, 20, "0.0 / 0.0 s");
    time_lb.set_font(&font);
    time_lb.set_bounds(8, (float)window.height() - 80, 300, 20);
    root.add_child(&time_lb);

    ui::Label speed_lb(320, 0, 200, 20, "speed 1.00x");
    speed_lb.set_font(&font);
    speed_lb.set_bounds(320, (float)window.height() - 80, 200, 20);
    root.add_child(&speed_lb);

    ui::Slider speed(8, 0, 470, 20);
    speed.set_bounds(8, (float)window.height() - 56, 470, 20);
    speed.set_range(0.5f, 2.0f);
    speed.set_value_silent(1.0f);
    root.add_child(&speed);

    ui::Label vol_lb(480, 0, 200, 20, "volume 1.00");
    vol_lb.set_font(&font);
    vol_lb.set_bounds(480, (float)window.height() - 80, 200, 20);
    root.add_child(&vol_lb);

    ui::Slider vol(480, 0, 472, 20);
    vol.set_bounds(480, (float)window.height() - 56, 472, 20);
    vol.set_range(0.f, 1.f);
    vol.set_value_silent(1.0f);
    root.add_child(&vol);

    auto open_by_path = [&](const char* path) {
        if (video.open_source(path)) {
            video.play();
            toggle.set_text("Pause");
            status.set_text(path);
        } else {
            ui::log_error("video_player: cannot open %s", path ? path : "");
        }
    };
    open_btn.set_on_click([&]() {
        std::wstring w;
        if (open_file_dialog(w)) open_by_path(utf8_from_wchar(w).c_str());
    });

    toggle.set_on_click([&]() {
        if (!video.has_video()) return;
        if (video.playing()) {
            video.pause();
            toggle.set_text("Play");
        } else {
            video.play();
            toggle.set_text("Pause");
        }
    });
    loop_cb.set_on_change([&](bool v) { video.set_loop(v); });
    seek.set_on_change([&](float t) {
        if (video.has_video()) video.seek(t * (float)video.duration());
    });
    speed.set_on_change([&](float s) { video.set_speed(s); });
    vol.set_on_change([&](float v) { video.set_volume(v); });

    if (!file_arg.empty()) open_by_path(file_arg.c_str());

    // ---- Scene 生命周期订阅 ----
    // 帧前：视频推进 + 状态文本同步
    scene.subscribe(ui::ScenePhase::Frame, [&](ui::Scene&, float dt) {
        video.advance(dt);
        if (video.has_video()) {
            const float dur = (float)video.duration();
            const float pos = video.position();
            if (!seek.dragging()) seek.set_value_silent(dur > 0.f ? pos / dur : 0.f);
            char tbuf[48];
            std::snprintf(tbuf, sizeof(tbuf), "%.1f / %.1f s", pos, dur);
            time_lb.set_text(tbuf);
            char spbuf[32];
            std::snprintf(spbuf, sizeof(spbuf), "speed %.2fx", video.speed());
            speed_lb.set_text(spbuf);
            char vbuf[32];
            std::snprintf(vbuf, sizeof(vbuf), "volume %.2f", video.volume());
            vol_lb.set_text(vbuf);
            loop_cb.set_checked(video.loop());
        }
    });

    // 窗口尺寸变化：Scene 已更新视口+root；此处重排各控件（与旧主循环一致）
    scene.set_event_hook(ui::SceneEvent::Resized, [&](ui::Scene&) {
        video.set_bounds(0, 46, (float)window.width(), (float)window.height() - 46.f - 150.f);
        toggle.set_bounds(8, (float)window.height() - 140, 90, 30);
        loop_cb.set_bounds(110, (float)window.height() - 136, 140, 24);
        seek.set_bounds(8, (float)window.height() - 108, 944, 24);
        time_lb.set_bounds(8, (float)window.height() - 80, 300, 20);
        speed_lb.set_bounds(320, (float)window.height() - 80, 200, 20);
        speed.set_bounds(8, (float)window.height() - 56, 470, 20);
        vol_lb.set_bounds(480, (float)window.height() - 80, 200, 20);
        vol.set_bounds(480, (float)window.height() - 56, 472, 20);
        return true;
    });

    // 按键：Space=播放/暂停（自定义，消费）；Ctrl+O=打开
    scene.set_event_hook(ui::SceneEvent::Key, [&](ui::Scene&) {
        if (window.key_pressed(ui::KeySpace)) {
            if (video.has_video()) {
                if (video.playing()) {
                    video.pause();
                    toggle.set_text("Play");
                } else {
                    video.play();
                    toggle.set_text("Pause");
                }
            }
            return true;
        }
        if (window.key_pressed(ui::KeyO) && window.key_down(ui::KeyLeftControl)) {
            std::wstring w;
            if (open_file_dialog(w)) open_by_path(utf8_from_wchar(w).c_str());
            return true;
        }
        return true;
    });

    // 运行：--smoke 指定秒自动退出，否则直至窗口关闭
    if (smoke_seconds > 0.0) {
        scene.run_for(smoke_seconds);
    } else {
        scene.run();
    }
    return 0;
}
