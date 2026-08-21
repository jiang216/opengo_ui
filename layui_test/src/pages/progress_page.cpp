#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/progress.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <cstdio>

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

// add_progress(h, value, w, big, show_text, cls)：宽度默认 800（镜像含 300px 容器示例）
ui::Progress* add_progress(PageHost& host, float value, float w, bool big, bool show_text,
                           const char* cls = "", const char* text = nullptr) {
    ui::Progress* p = new ui::Progress(0, 0, w, big ? 18.f : 6.f);
    p->set_font(host.font);
    p->set_theme(host.theme);
    if (big) p->add_class("layui-progress-big");
    if (cls && *cls) p->add_class(cls);
    p->set_show_text(show_text);
    p->set_text_override(text);
    p->set_value(value);
    host.content->add_child(p);
    return p;
}

void add_spacer(PageHost& host, float h) {
    ui::Label* pad = new ui::Label(0, 0, 10, h, "");
    pad->set_font(host.font);
    pad->set_theme(host.theme);
    host.content->add_child(pad);
}

void build_progress(PageHost& host) {
    // 1. 默认风格的进度条
    add_section(host, "默认风格的进度条");
    add_progress(host, 0.4f, 800.f, false, true);
    add_spacer(host, 6.f);
    // 300px 父容器示例（宽度自适应父级）
    ui::Label* note = new ui::Label(0, 0, 800, 0, "宽度 100% 适配父级：下面进度条在 300px 容器中");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
    add_progress(host, 0.7f, 300.f, false, true);

    // 2. 更多颜色选取
    add_section(host, "更多颜色选取");
    add_progress(host, 0.2f, 800.f, false, true, "layui-bg-red");
    add_spacer(host, 6.f);
    add_progress(host, 0.3f, 800.f, false, true, "layui-bg-orange");
    add_spacer(host, 6.f);
    add_progress(host, 0.4f, 800.f, false, true, "layui-bg-green");
    add_spacer(host, 6.f);
    add_progress(host, 0.5f, 800.f, false, true, "layui-bg-blue");
    add_spacer(host, 6.f);
    add_progress(host, 0.6f, 800.f, false, true, "layui-bg-cyan");

    // 3. 大尺寸进度条（layui-progress-big）
    add_section(host, "大尺寸进度条");
    add_progress(host, 0.2f, 800.f, true, true);
    add_spacer(host, 6.f);
    add_progress(host, 0.35f, 800.f, true, true, "layui-bg-green");
    add_spacer(host, 6.f);
    add_progress(host, 0.75f, 800.f, true, true, "layui-bg-cyan");

    // 4. 显示进度比文本（含分数 "5 / 10"）
    add_section(host, "显示进度比文本");
    add_progress(host, 0.2f, 800.f, false, true);
    add_spacer(host, 6.f);
    add_progress(host, 0.5f, 800.f, false, true, "", "5 / 10");
    add_spacer(host, 6.f);
    add_progress(host, 0.7f, 800.f, true, true);

    // 5. 动态改变进度
    add_section(host, "动态改变进度");
    ui::Progress* dyn = add_progress(host, 0.f, 800.f, true, true, "layui-bg-red");
    ui::HBox* row = new ui::HBox(0, 0, 800, 30.f);
    row->set_theme(host.theme);
    row->set_spacing(8.f);
    host.content->add_child(row);
    const float btn_h = 28.f;
    ui::Button* set50 = new ui::Button(0, 0, 120.f, btn_h, "设置 50%");
    set50->set_font(host.font);
    set50->set_theme(host.theme);
    set50->add_class("layui-btn layui-btn-normal");
    set50->set_on_click([dyn]() { dyn->set_value(0.5f); });
    row->add_child(set50);
    ui::Button* loading = new ui::Button(0, 0, 140.f, btn_h, "模拟 loading");
    loading->set_font(host.font);
    loading->set_theme(host.theme);
    loading->add_class("layui-btn layui-btn-normal");
    // 模拟 loading：点击后分步推进至 100%
    loading->set_on_click([dyn]() {
        float v = dyn->value() + 0.35f;
        if (v >= 1.f) v = 1.f;
        dyn->set_value(v);
    });
    row->add_child(loading);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/progress_page.cpp（节选）：克隆 layui progress.html\n"
            "\n"
            "ui::Progress* p = new ui::Progress(0, 0, 800, 6.f);\n"
            "p->set_font(host.font);\n"
            "p->set_theme(host.theme);\n"
            "p->set_value(0.4f);                 // 进度 0..1，默认绘制百分比文本\n"
            "p->add_class(\"layui-bg-red\");     // 颜色类（red/orange/green/blue/cyan）\n"
            "p->add_class(\"layui-progress-big\"); // 大号（高 18px，内嵌白字）\n"
            "p->set_show_text(false);            // 隐藏右侧文本\n"
            "p->set_text_override(\"5 / 10\");   // 自定义显示文本（分数）\n"
            "host.content->add_child(p);");
    }
    if (host.help) {
        const char* lines[] = {
            "进度条（Progress）API",
            "  set_value(v)：进度 0..1（自动 clamp），默认绘制百分比文本",
            "  set_show_text(bool)：是否显示文本",
            "  set_text_override(\"5 / 10\")：自定义显示文本（分数），空则回退百分比",
            "  颜色类：layui-bg-red/orange/green/blue/cyan；默认填充 #5fb878",
            "  类名：layui-progress（默认 h6）/ layui-progress-big（高 18px）",
            "  轨道 #eee 圆角 20；宽度按父级 100% 适配",
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
    build_progress(host);
}

static const PageDef kProgressPage{41, "展示", "进度条", "进度条：默认/颜色/大尺寸/分数/动态", build, nullptr, false, nullptr, 0, "Progress"};

static const bool kRegistered = []() {
    page::register_page(kProgressPage);
    return true;
}();