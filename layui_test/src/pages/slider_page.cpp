#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/slider.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <string>

namespace {

void add_fieldset_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* ft = new ui::FieldsetTitle(800, 40.f, text);
    ft->set_font(host.font);
    ft->set_theme(host.theme);
    host.content->add_child(ft);
}

ui::Label* add_status(PageHost& host, const char* text) {
    ui::Label* status = new ui::Label(0, 4, 800, 0, text);
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    return status;
}

ui::Slider* make_slider(PageHost& host, float w, float h) {
    ui::Slider* s = new ui::Slider(0, 0, w, h);
    s->set_font(host.font);
    s->set_theme(host.theme);
    host.content->add_child(s);
    return s;
}

// ── 1. 基本滑块（slideTest1：默认）───────────────────────────────────────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "基本滑块");
    make_slider(host, 560.f, 30.f);
}

// ── 2. 定义初始值（slideTest2：value:20）─────────────────────────────────────
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "定义初始值");
    ui::Slider* s = make_slider(host, 560.f, 30.f);
    s->set_value(20.f);
}

// ── 3. 设置最大最小值（slideTest3：min:20 max:50）────────────────────────────
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "设置最大最小值");
    ui::Slider* s = make_slider(host, 560.f, 30.f);
    s->set_range(20.f, 50.f);
}

// ── 4. 设置步长（slideTest4 step:10 / slideTest5 step:10 + showstep）─────────
void build_demo4(PageHost& host) {
    add_fieldset_title(host, "设置步长");
    ui::Slider* s4 = make_slider(host, 560.f, 30.f);
    s4->set_step(10.f);

    ui::Slider* s5 = make_slider(host, 560.f, 30.f);
    s5->set_step(10.f);
    s5->set_showstep(true);
}

// ── 5. 设置提示文本（slideTest6 setTips / slideTest7 tips:false + change）────
void build_demo5(PageHost& host) {
    add_fieldset_title(host, "设置提示文本");
    ui::Slider* s6 = make_slider(host, 560.f, 30.f);
    s6->set_range(20.f, 1000.f);
    s6->set_tips_callback([](float v) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0fGB", v);
        return std::string(buf);
    });

    ui::Slider* s7 = make_slider(host, 560.f, 30.f);
    s7->set_tips(false);
    ui::Label* tips = add_status(host, "当前数值：0");
    s7->set_on_change([tips](float v) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "当前数值：%.0f", v);
        tips->set_text(buf);
    });
}

// ── 6. 开启输入框（slideTest8：input:true）──────────────────────────────────
void build_demo6(PageHost& host) {
    add_fieldset_title(host, "开启输入框");
    ui::Slider* s8 = make_slider(host, 560.f, 36.f);
    s8->set_input(true);
}

// ── 7. 开启范围选择（slideTest9 value:40 + range / slideTest10 [30,60]）──────
void build_demo7(PageHost& host) {
    add_fieldset_title(host, "开启范围选择");
    ui::Slider* s9 = make_slider(host, 560.f, 30.f);
    s9->set_range_enabled(true);
    s9->set_value(40.f);
    ui::Label* tip = add_status(host, "开始值：0、结尾值：40");
    s9->set_on_range_change([tip](float a, float b) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "开始值：%.0f、结尾值：%.0f", a, b);
        tip->set_text(buf);
    });

    ui::Slider* s10 = make_slider(host, 560.f, 30.f);
    s10->set_range_enabled(true);
    s10->set_range_value(30.f, 60.f);
}

// ── 8. 垂直滑块（slideTest11~14：value/range/input，同一卡片内并排）──────────
void build_demo8(PageHost& host) {
    add_fieldset_title(host, "垂直滑块");
    ui::HBox* row = new ui::HBox(0, 0, 560.f, 200.f);
    host.content->add_child(row);
    const auto add_v = [&](float w, float h) {
        ui::Slider* s = new ui::Slider(0, 0, w, h);
        s->set_font(host.font);
        s->set_theme(host.theme);
        row->add_child(s);
        return s;
    };
    ui::Slider* s11 = add_v(80.f, 200.f);
    s11->set_vertical(true);

    ui::Slider* s12 = add_v(80.f, 200.f);
    s12->set_vertical(true);
    s12->set_value(30.f);

    ui::Slider* s13 = add_v(90.f, 200.f);
    s13->set_vertical(true);
    s13->set_range_enabled(true);
    s13->set_range_value(30.f, 70.f);

    ui::Slider* s14 = add_v(140.f, 200.f);
    s14->set_vertical(true);
    s14->set_input(true);
    s14->set_value(80.f);
}

// ── 9. 自定义颜色（slideTest15~17：theme）────────────────────────────────────
void build_demo9(PageHost& host) {
    add_fieldset_title(host, "自定义颜色");
    ui::Slider* s15 = make_slider(host, 560.f, 30.f);
    s15->set_theme_color({0.12f, 0.62f, 1.0f, 1.f});   // #1E9FFF

    ui::Slider* s16 = make_slider(host, 560.f, 30.f);
    s16->set_theme_color({0.37f, 0.72f, 0.47f, 1.f});  // #5FB878
    s16->set_value(50.f);

    ui::Slider* s17 = make_slider(host, 560.f, 30.f);
    s17->set_theme_color({1.0f, 0.34f, 0.13f, 1.f});   // #FF5722
    s17->set_range_enabled(true);
    s17->set_range_value(30.f, 70.f);
}

// ── 10. 禁用滑块（slideTest18：value:35 + disabled）──────────────────────────
void build_demo10(PageHost& host) {
    add_fieldset_title(host, "禁用滑块");
    ui::Slider* s18 = make_slider(host, 560.f, 30.f);
    s18->set_value(35.f);
    s18->set_disabled(true);
}

void build_sliders(PageHost& host) {
    build_demo1(host);
    build_demo2(host);
    build_demo3(host);
    build_demo4(host);
    build_demo5(host);
    build_demo6(host);
    build_demo7(host);
    build_demo8(host);
    build_demo9(host);
    build_demo10(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/slider_page.cpp — 一比一克隆 layui slider.html 全部 18 个 demo\n"
            "\n"
            "ui::Slider* s = new ui::Slider(0, 0, 560, 30);\n"
            "s->set_font(host.font); s->set_theme(host.theme);\n"
            "\n"
            "// 1. 基本滑块（默认）\n"
            "// 2. 定义初始值\n"
            "s->set_value(20);\n"
            "// 3. 设置最大最小值\n"
            "s->set_range(20.f, 50.f);\n"
            "// 4. 设置步长 + 间断点\n"
            "s->set_step(10); s->set_showstep(true);\n"
            "// 5. 设置提示文本\n"
            "s->set_tips_callback([](float v){ return std::to_string((int)v)+\"GB\"; });\n"
            "s->set_tips(false);  s->set_on_change([](float v){});\n"
            "// 6. 开启输入框\n"
            "s->set_input(true);\n"
            "// 7. 开启范围选择\n"
            "s->set_range_enabled(true); s->set_range_value(30.f, 60.f);\n"
            "s->set_on_range_change([](float a, float b){});\n"
            "// 8. 垂直滑块\n"
            "s->set_vertical(true); s->set_value(30); s->set_input(true);\n"
            "// 9. 自定义颜色\n"
            "s->set_theme_color({0.12f,0.62f,1.f,1.f});\n"
            "// 10. 禁用滑块\n"
            "s->set_disabled(true);");
    }
    if (host.help) {
        const char* lines[] = {
            "滑块（Slider）API — 一比一克隆 slider.html 全部 18 个 demo",
            "",
            "1. 基本滑块：默认（min0 max100）",
            "2. 定义初始值：set_value(20)",
            "3. 设置最大最小值：set_range(min, max)",
            "4. 设置步长：set_step(10)；间断点 set_showstep(true)",
            "5. 设置提示文本：set_tips_callback(fn) 自定义；set_tips(false) 关闭 + set_on_change",
            "6. 开启输入框：set_input(true)（数值 + 上下箭头）",
            "7. 开启范围选择：set_range_enabled(true) + set_range_value(start,end) + set_on_range_change",
            "8. 垂直滑块：set_vertical(true) 高度 set_track_height / 实例化传 h",
            "9. 自定义颜色：set_theme_color(Vec4)",
            "10. 禁用滑块：set_disabled(true)",
            "通用：set_value / set_value_silent(不触发回调) / value() / value2()",
            "键盘：左右/上下方向键微调，Home/End 到边界",
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
    build_sliders(host);
}

static const PageDef kSliderPage{57, "交互", "滑块", "滑块：基本/步长/范围/垂直/颜色/禁用", build, nullptr, false, nullptr, 0, "Slider"};

static const bool kRegistered = []() {
    page::register_page(kSliderPage);
    return true;
}();