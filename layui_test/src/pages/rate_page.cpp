#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/rate.h"
#include "ui/ui/scrollbox.h"
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

ui::Rate* make_rate(PageHost& host, float w, float h) {
    ui::Rate* r = new ui::Rate(0, 0, w, h);
    r->set_font(host.font);
    r->set_theme(host.theme);
    host.content->add_child(r);
    return r;
}

// ── 1. 基础效果（test1：默认 value0 length5）─────────────────────────────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "基础效果");
    make_rate(host, 260.f, 30.f);
}

// ── 2. 显示文字（test2：value2 + text）──────────────────────────────────────
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "显示文字");
    ui::Rate* r = make_rate(host, 260.f, 30.f);
    r->set_value(2);
    r->set_text(true);
}

// ── 3. 半星效果（test3：value2.5 + half；test4：value3.5 + half + text）──────
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "半星效果");
    ui::Rate* r3 = make_rate(host, 260.f, 30.f);
    r3->set_value(2.5f);
    r3->set_half(true);

    ui::Rate* r4 = make_rate(host, 260.f, 30.f);
    r4->set_value(3.5f);
    r4->set_half(true);
    r4->set_text(true);
}

// ── 4. 自定义文本（test5：value3 + text + setText；test6：value1.5 + half + text）─
void build_demo4(PageHost& host) {
    add_fieldset_title(host, "自定义内容");
    ui::Rate* r5 = make_rate(host, 260.f, 30.f);
    r5->set_value(3);
    r5->set_text(true);
    r5->set_text_callback([](float v) {
        const int i = (int)(v + 0.5f);
        switch (i) {
            case 1: return std::string("极差");
            case 2: return std::string("差");
            case 3: return std::string("中等");
            case 4: return std::string("好");
            case 5: return std::string("极好");
            default: return std::string("极好");
        }
    });

    ui::Rate* r6 = make_rate(host, 260.f, 30.f);
    r6->set_value(1.5f);
    r6->set_half(true);
    r6->set_text(true);
    r6->set_text_callback([](float v) {
        const int whole = (int)v;
        return std::to_string(whole) + (v - whole > 0.f ? ".5" : "");
    });
}

// ── 5. 自定义长度（test7：length3；test8：length10 value8）───────────────────
void build_demo5(PageHost& host) {
    add_fieldset_title(host, "自定义长度");
    ui::Rate* r7 = make_rate(host, 200.f, 30.f);
    r7->set_length(3);

    ui::Rate* r8 = make_rate(host, 560.f, 30.f);
    r8->set_length(10);
    r8->set_value(8);
}

// ── 6. 只读（test9：value4 + readonly）──────────────────────────────────────
void build_demo6(PageHost& host) {
    add_fieldset_title(host, "只读");
    ui::Rate* r9 = make_rate(host, 260.f, 30.f);
    r9->set_value(4);
    r9->set_readonly(true);
}

// ── 7. 自定义主题色（test10~14：theme）──────────────────────────────────────
void build_demo7(PageHost& host) {
    add_fieldset_title(host, "自定义主题色");
    const struct { float v; bool half; const char* hex; } cfg[] = {
        {3.f, false, "#FF8000"},
        {3.f, false, "#009688"},
        {2.5f, true, "#1E9FFF"},
        {2.5f, true, "#2F4056"},
        {2.5f, true, "#FE0000"},
    };
    for (const auto& c : cfg) {
        ui::Rate* r = make_rate(host, 260.f, 30.f);
        r->set_value(c.v);
        r->set_half(c.half);
        r->set_theme_color(ui::layui::hex(c.hex));
    }
}

void build_rate(PageHost& host) {
    build_demo1(host);
    build_demo2(host);
    build_demo3(host);
    build_demo4(host);
    build_demo5(host);
    build_demo6(host);
    build_demo7(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/rate_page.cpp — 一比一克隆 layui rate.html 全部 14 个 demo\n"
            "\n"
            "ui::Rate* r = new ui::Rate(0, 0, 260, 30);\n"
            "r->set_font(host.font); r->set_theme(host.theme);\n"
            "\n"
            "// 1. 基础效果（默认）\n"
            "// 2. 显示文字\n"
            "r->set_value(2); r->set_text(true);\n"
            "// 3. 半星效果\n"
            "r->set_value(2.5f); r->set_half(true); r->set_text(true);\n"
            "// 4. 自定义文本\n"
            "r->set_text_callback([](float v){ return std::string(\"好\"); });\n"
            "// 5. 自定义长度\n"
            "r->set_length(10); r->set_value(8);\n"
            "// 6. 只读\n"
            "r->set_value(4); r->set_readonly(true);\n"
            "// 7. 自定义主题色\n"
            "r->set_theme_color(ui::layui::hex(\"#1E9FFF\"));");
    }
    if (host.help) {
        const char* lines[] = {
            "评分（Rate）API — 一比一克隆 rate.html 全部 14 个 demo",
            "",
            "1. 基础效果：默认 value0 length5",
            "2. 显示文字：set_text(true) 星后显示数值",
            "3. 半星效果：set_half(true) + set_value(x.5) 允许半星",
            "4. 自定义文本：set_text_callback(fn) 对应 layui setText",
            "5. 自定义长度：set_length(n) 对应 layui length",
            "6. 只读：set_readonly(true) 不可交互",
            "7. 自定义主题色：set_theme_color(Vec4) 对应 layui theme",
            "通用：set_value / value() / value_f() / set_on_change(int)",
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
    build_rate(host);
}

static const PageDef kRatePage{57, "交互", "评分", "评分：基本/文字/半星/长度/只读/主题色", build, nullptr, false, nullptr, 0, "Rate"};

static const bool kRegistered = []() {
    page::register_page(kRatePage);
    return true;
}();
