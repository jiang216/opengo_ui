#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/colorpicker.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
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

ui::Label* add_status(PageHost& host, const char* text) {
    ui::Label* status = new ui::Label(0, 4, 800, 0, text);
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    return status;
}

ui::ColorPicker* make_picker(PageHost& host, float w, float h) {
    ui::ColorPicker* p = new ui::ColorPicker(0, 0, w, h);
    p->set_font(host.font);
    p->set_theme(host.theme);
    host.content->add_child(p);
    return p;
}

// ── 1. 常规使用（test1 无默认色 / test2 带默认色 #2ec770 + done 回调）──────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "常规使用");
    ui::ColorPicker* p1 = make_picker(host, 300.f, 0.f);
    ui::Label* st1 = add_status(host, "选择了：" );
    p1->set_on_change([st1](const ui::Vec4& c) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "选择了：rgb(%d,%d,%d)",
                      (int)(c.r * 255.f), (int)(c.g * 255.f), (int)(c.b * 255.f));
        st1->set_text(buf);
    });

    ui::ColorPicker* p2 = make_picker(host, 300.f, 0.f);
    p2->set_color({0.18f, 0.78f, 0.44f, 1.0f});  // #2ec770
    ui::Label* st2 = add_status(host, "选择了：#2ec770");
    p2->set_on_done([st2](const ui::Vec4& c) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "选择了：#%02X%02X%02X",
                      (int)(c.r * 255.f), (int)(c.g * 255.f), (int)(c.b * 255.f));
        st2->set_text(buf);
    });
}

// ── 2. 表单赋值（test-form：color #1c97f5 + done 回调写回输入框）───────────────
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "表单赋值");
    ui::ColorPicker* p = make_picker(host, 300.f, 0.f);
    p->set_color({0.11f, 0.59f, 0.96f, 1.0f});  // #1c97f5
    ui::Label* st = add_status(host, "请选择颜色 → 确认后写回");
    p->set_on_done([st](const ui::Vec4& c) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "输入框已赋值：#%02X%02X%02X",
                      (int)(c.r * 255.f), (int)(c.g * 255.f), (int)(c.b * 255.f));
        st->set_text(buf);
    });
}

// ── 3. RGB / RGBA 色值（test3 rgb / test4 rgba+alpha）────────────────────────
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "RGB / RGBA 色值");
    ui::ColorPicker* p3 = make_picker(host, 300.f, 0.f);
    p3->set_color({0.27f, 0.26f, 0.26f, 1.0f});  // rgb(68,66,66)
    p3->set_format(ui::ColorFormat::Rgb);

    ui::ColorPicker* p4 = make_picker(host, 320.f, 0.f);
    p4->set_color({0.27f, 0.26f, 0.26f, 0.5f});  // rgba(68,66,66,0.5)
    p4->set_format(ui::ColorFormat::Rgb);
    p4->set_alpha(true);
}

// ── 4. 透明度选择（test5 hex+alpha / test6 rgba / test7 无初始色）──────────────
void build_demo4(PageHost& host) {
    add_fieldset_title(host, "透明度选择");
    ui::ColorPicker* p5 = make_picker(host, 300.f, 0.f);
    p5->set_color({0.0f, 0.59f, 0.53f, 1.0f});  // #009688
    p5->set_alpha(true);
    p5->set_format(ui::ColorFormat::Rgb);

    ui::ColorPicker* p6 = make_picker(host, 320.f, 0.f);
    p6->set_color({0.0f, 0.59f, 0.53f, 0.6f});  // rgb(0,150,136,0.6)
    p6->set_alpha(true);
    p6->set_format(ui::ColorFormat::Rgb);

    ui::ColorPicker* p7 = make_picker(host, 320.f, 0.f);
    p7->set_color({0.0f, 0.0f, 0.0f, 1.0f});  // 无初始色（默认黑色）
    p7->set_alpha(true);
    p7->set_format(ui::ColorFormat::Rgb);
}

// ── 5. 预定义颜色项（test8 predefine / test9 自定义 colors）──────────────────
void build_demo5(PageHost& host) {
    add_fieldset_title(host, "预定义颜色项");
    ui::ColorPicker* p8 = make_picker(host, 320.f, 0.f);
    p8->set_color({0.78f, 0.08f, 0.52f, 1.0f});  // #c71585
    p8->set_predefine(true);

    ui::ColorPicker* p9 = make_picker(host, 320.f, 0.f);
    p9->set_color({0.62f, 0.54f, 0.05f, 1.0f});  // #9d8a0e
    p9->set_predefine(true);
    p9->set_colors({{1.0f, 0.55f, 0.0f, 1.f},    // #ff8c00
                    {0.0f, 0.81f, 0.82f, 1.f},   // #00ced1
                    {0.62f, 0.54f, 0.05f, 1.f}});  // #9d8a0e
}

// ── 6. 全功能和回调（test-all：format rgb + predefine + alpha + done + change）──
void build_demo6(PageHost& host) {
    add_fieldset_title(host, "全功能和回调的使用");
    ui::ColorPicker* p = make_picker(host, 360.f, 0.f);
    p->set_color({0.03f, 0.61f, 0.55f, 1.0f});  // rgba(7,155,140,1)
    p->set_format(ui::ColorFormat::Rgb);
    p->set_predefine(true);
    p->set_alpha(true);
    // change 把主题色写回页面头部/状态
    ui::Label* st = add_status(host, "主题色：rgba(7,155,140,1)");
    p->set_on_change([st](const ui::Vec4& c) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "主题色：rgba(%d,%d,%d,%.2f)",
                      (int)(c.r * 255.f), (int)(c.g * 255.f), (int)(c.b * 255.f), c.a);
        st->set_text(buf);
    });
    // done 赋给隐藏域（展示）
    ui::Label* done = add_status(host, "隐藏域：#1c97f5（未确认）");
    p->set_on_done([done](const ui::Vec4& c) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "隐藏域：#%02X%02X%02X",
                      (int)(c.r * 255.f), (int)(c.g * 255.f), (int)(c.b * 255.f));
        done->set_text(buf);
    });
}

// ── 7. 颜色框尺寸（test10 lg / test11 sm / test12 xs）────────────────────────
void build_demo7(PageHost& host) {
    add_fieldset_title(host, "颜色框尺寸");
    ui::ColorPicker* p10 = make_picker(host, 300.f, 0.f);
    p10->set_size(ui::ColorPickerSize::Lg);

    ui::ColorPicker* p11 = make_picker(host, 300.f, 0.f);
    p11->set_size(ui::ColorPickerSize::Sm);

    ui::ColorPicker* p12 = make_picker(host, 260.f, 0.f);
    p12->set_size(ui::ColorPickerSize::Xs);
}

void build_colorpicker(PageHost& host) {
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
            "// pages/colorpicker_page.cpp — 一比一克隆 layui colorpicker.html 全部 7 组 demo\n"
            "\n"
            "ui::ColorPicker* p = new ui::ColorPicker(0, 0, 300, 0);\n"
            "p->set_font(host.font); p->set_theme(host.theme);\n"
            "\n"
            "// 1. 常规使用\n"
            "p->set_on_change([](const Vec4& c){ /* 取色中 */ });       // change\n"
            "p->set_on_done([](const Vec4& c){ /* 确认/清除 */ });      // done\n"
            "\n"
            "// 2. 表单赋值\n"
            "p->set_color({0.11f,0.59f,0.96f,1});   // #1c97f5\n"
            "\n"
            "// 3. RGB / RGBA 色值\n"
            "p->set_format(ColorFormat::Rgb);       // hex/rgb（alpha 时自动 rgba）\n"
            "p->set_alpha(true);                    // 开启透明度滑块\n"
            "\n"
            "// 4. 透明度选择\n"
            "p->set_color({0.0f,0.59f,0.53f,0.6f}); // 可带 alpha\n"
            "\n"
            "// 5. 预定义颜色项\n"
            "p->set_predefine(true);                // 开启预定义色\n"
            "p->set_colors({Vec4{1,0.55f,0,1}, ...}); // 自定义预定义色\n"
            "\n"
            "// 6. 全功能和回调\n"
            "p->set_format(Rgb); p->set_predefine(true); p->set_alpha(true);\n"
            "p->set_on_change(...); p->set_on_done(...);\n"
            "\n"
            "// 7. 颜色框尺寸\n"
            "p->set_size(ColorPickerSize::Lg / Sm / Xs);");
    }
    if (host.help) {
        const char* lines[] = {
            "颜色选择器（ColorPicker）API — 一比一克隆 colorpicker.html 全部 7 组 demo",
            "",
            "1. 常规使用",
            "   set_on_change(取色中 change) / set_on_done(确认/清除 done)",
            "2. 表单赋值",
            "   set_color(默认色) + done 回调写回输入框",
            "3. RGB / RGBA 色值",
            "   set_format(ColorFormat::Hex / Rgb) + set_alpha(true) → rgba",
            "4. 透明度选择",
            "   set_alpha(true) 显示透明度滑块；格式自动 rgba",
            "5. 预定义颜色项",
            "   set_predefine(true) 开预定义色；set_colors({...}) 自定义",
            "6. 全功能和回调",
            "   format rgb + predefine + alpha + change + done",
            "7. 颜色框尺寸",
            "   set_size(ColorPickerSize::Lg / Sm / Xs)",
            "结构：SV 色场 + 色相滑块 + 透明度滑块 + 格式文本 + 预定义色 + 确认/清除",
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
    build_colorpicker(host);
}

static const PageDef kColorPickerPage{54, "交互", "颜色选择器", "颜色选择：SV/色相/透明度/预定义色", build, nullptr, false, nullptr, 0, "Colorpicker"};

static const bool kRegistered = []() {
    page::register_page(kColorPickerPage);
    return true;
}();