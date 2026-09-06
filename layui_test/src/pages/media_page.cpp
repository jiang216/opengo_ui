#include "../page_registry.h"

#include "ui/media/video_container.h"

#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

// 多媒体页（UI-191）：视频容器控件（MediaPipeline + 可换节点），合成源演示——
// 纯 UI 工程无需 FFmpeg 即可运行；容器自带底部播放控制条（播放/进度/音量/倍速/循环）。

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

void add_spacer(PageHost& host, float h) {
    ui::Label* pad = new ui::Label(0, 0, 10, h, "");
    pad->set_font(host.font);
    pad->set_theme(host.theme);
    host.content->add_child(pad);
}

void build_media(PageHost& host) {
    // 1. 单实例：合成源 8 秒（SMPTE 色条 + 进度条，帧内容仅依赖绝对位置 -> 截图可回归）
    add_section(host, "视频容器（合成源，无需 FFmpeg）");
    ui::VideoContainer* vc = new ui::VideoContainer(0, 0, 800, 360);
    vc->set_font(host.font);
    vc->set_theme(host.theme);
    vc->set_title("视频容器 · synthetic:8 ｜ 每实例独立解码线程与纹理");
    vc->open_source("synthetic:8");
    host.content->add_child(vc);
    add_spacer(host, 10.f);

    // 2. 程序式控制：播放/暂停切换 + 重新打开（演示公共 API；容器内部控制条亦可用）
    ui::HBox* row = new ui::HBox(0, 0, 800, 30.f);
    row->set_theme(host.theme);
    row->set_spacing(8.f);
    host.content->add_child(row);

    ui::Button* toggle = new ui::Button(0, 0, 110.f, 28.f, "播放/暂停");
    toggle->set_font(host.font);
    toggle->set_theme(host.theme);
    toggle->add_class("layui-btn layui-btn-normal");
    toggle->set_on_click([vc]() { vc->set_playing(!vc->playing()); });
    row->add_child(toggle);

    ui::Button* reopen = new ui::Button(0, 0, 110.f, 28.f, "重新打开");
    reopen->set_font(host.font);
    reopen->set_theme(host.theme);
    reopen->add_class("layui-btn layui-btn-normal");
    reopen->set_on_click([vc]() { vc->open_source("synthetic:8"); });
    row->add_child(reopen);

    ui::Button* stop = new ui::Button(0, 0, 110.f, 28.f, "关闭");
    stop->set_font(host.font);
    stop->set_theme(host.theme);
    stop->add_class("layui-btn layui-btn-normal");
    stop->set_on_click([vc]() { vc->close_source(); });
    row->add_child(stop);

    ui::Button* jump = new ui::Button(0, 0, 110.f, 28.f, "跳到 4.0s");
    jump->set_font(host.font);
    jump->set_theme(host.theme);
    jump->add_class("layui-btn layui-btn-normal");
    jump->set_on_click([vc]() { vc->seek(4.f); });
    row->add_child(jump);
    add_spacer(host, 10.f);

    // 3. 多实例同窗口共存（各自解码线程与纹理）
    add_section(host, "多实例共存（同窗口，两个容器独立推进）");
    ui::HBox* cols = new ui::HBox(0, 0, 800, 240.f);
    cols->set_theme(host.theme);
    cols->set_spacing(10.f);
    host.content->add_child(cols);
    ui::VideoContainer* vc_a = new ui::VideoContainer(0, 0, 395, 240);
    vc_a->set_font(host.font);
    vc_a->set_theme(host.theme);
    vc_a->set_title("容器 A · 4s");
    vc_a->open_source("synthetic:4");
    cols->add_child(vc_a);
    ui::VideoContainer* vc_b = new ui::VideoContainer(0, 0, 395, 240);
    vc_b->set_font(host.font);
    vc_b->set_theme(host.theme);
    vc_b->set_title("容器 B · 6s（320x180 保持宽高比居中）");
    // 不同尺寸：open 时按源分辨率重新建纹理，容器保持宽高比居中适配
    vc_b->open_source("synthetic:6");
    cols->add_child(vc_b);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// 视频容器（VideoContainer，UI-191）：MediaPipeline + 可换节点\n"
            "\n"
            "ui::VideoContainer* vc = new ui::VideoContainer(0, 0, 800, 360);\n"
            "vc->set_font(host.font);            // 标题/占位/控制条字体\n"
            "vc->set_theme(host.theme);          // world/menu 主主题\n"
            "vc->set_title(\"视频容器 · 合成源\");\n"
            "vc->open_source(\"synthetic:8\");   // 合成源（纯 CPU，无需 FFmpeg）\n"
            "// vc->open_source(\"assets/test.mp4\"); // 真实文件需 media/ffmpeg 提供 ffmpeg 节点\n"
            "host.content->add_child(vc);        // on_advance 每帧推进管线并上传纹理\n"
            "\n"
            "// 播放控制（同 VideoView）：\n"
            "vc->play(); vc->pause(); vc->seek(4.f);\n"
            "vc->set_loop(true); vc->set_speed(1.f); vc->set_volume(1.f);\n"
            "vc->has_video(); vc->has_audio(); vc->position(); vc->duration();");
    }
    if (host.help) {
        const char* lines[] = {
            "视频容器（VideoContainer）API",
            "  open_source(desc)：'synthetic[:秒]' 合成源（无需 FFmpeg）或文件路径",
            "  set_title(t) / has_video() / close_source()",
            "  播放控制：play/pause/set_playing/playing/set_loop/loop",
            "  定位：seek(秒) / position() / duration()；倍速 set_speed(0.1..4)",
            "  音频：has_audio() / set_volume(0..1)（FFmpeg 节点 + winmm 输出）",
            "  控制条按需显隐（UI-197）：鼠标悬停容器显示，移开隐藏；双击画面=播放/暂停",
            "  内部自带底部控制条：播放按钮/进度滑条/循环开关/音量/倍速",
            "  数据管线（MediaPipeline）：Source/Decoder/AudioSink 运行期按名更换",
            "  （MediaRegistry 注册表；纯合成源在 media/core，FFmpeg 节点在 media/ffmpeg）",
            "  约束：纹理归属所属窗口 GL 上下文（UI-146），每实例独立解码线程与纹理",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_media(host);
}

static const PageDef kMediaPage{62, "展示", "视频容器", "视频容器：合成源/多实例/播放控制（MediaPipeline 可换节点）", build, nullptr, false, nullptr, 0, "VideoContainer"};

static const bool kRegistered = []() {
    page::register_page(kMediaPage);
    return true;
}();